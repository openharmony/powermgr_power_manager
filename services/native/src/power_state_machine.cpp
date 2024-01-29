/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "power_state_machine.h"

#include <cinttypes>
#include <datetime_ex.h>
#include <hisysevent.h>

#include "power_hitrace.h"
#include "power_mgr_factory.h"
#include "power_mgr_service.h"
#include "power_utils.h"
#include "setting_helper.h"

namespace OHOS {
namespace PowerMgr {
namespace {
sptr<SettingObserver> g_displayOffTimeObserver;
constexpr int64_t COORDINATED_STATE_SCREEN_OFF_TIME_MS = 10000;
}
PowerStateMachine::PowerStateMachine(const wptr<PowerMgrService>& pms) : pms_(pms), currentState_(PowerState::UNKNOWN)
{
    POWER_HILOGD(FEATURE_POWER_STATE, "Instance start");
    // NOTICE Need get screen state when device startup,
    // rightnow we set screen is on as default
    mDeviceState_.screenState.lastOnTime = GetTickCount();
    mDeviceState_.screenState.lastOffTime = 0;
    mDeviceState_.lastWakeupEventTime = 0;
    mDeviceState_.lastRefreshActivityTime = 0;
    mDeviceState_.lastWakeupDeviceTime = 0;
    mDeviceState_.lastSuspendDeviceTime = 0;

    // init lock map which will block state transit
    std::vector<RunningLockType> awakeBlocker {};
    std::vector<RunningLockType> freezeBlocker {};
    std::vector<RunningLockType> inactiveBlocker {RunningLockType::RUNNINGLOCK_SCREEN};
    std::vector<RunningLockType> standByBlocker {};
    std::vector<RunningLockType> dozeBlocker {};
    std::vector<RunningLockType> sleepBlocker {
        RunningLockType::RUNNINGLOCK_COORDINATION
    };
    std::vector<RunningLockType> hibernateBlocker {};
    std::vector<RunningLockType> shutdownBlocker {};

    lockMap_.emplace(PowerState::AWAKE, std::make_shared<std::vector<RunningLockType>>(awakeBlocker));
    lockMap_.emplace(PowerState::FREEZE, std::make_shared<std::vector<RunningLockType>>(freezeBlocker));
    lockMap_.emplace(PowerState::INACTIVE, std::make_shared<std::vector<RunningLockType>>(inactiveBlocker));
    lockMap_.emplace(PowerState::STAND_BY, std::make_shared<std::vector<RunningLockType>>(standByBlocker));
    lockMap_.emplace(PowerState::DOZE, std::make_shared<std::vector<RunningLockType>>(dozeBlocker));
    lockMap_.emplace(PowerState::SLEEP, std::make_shared<std::vector<RunningLockType>>(sleepBlocker));
    lockMap_.emplace(PowerState::HIBERNATE, std::make_shared<std::vector<RunningLockType>>(hibernateBlocker));
    lockMap_.emplace(PowerState::SHUTDOWN, std::make_shared<std::vector<RunningLockType>>(shutdownBlocker));

    POWER_HILOGD(FEATURE_POWER_STATE, "Instance end");
}

PowerStateMachine::~PowerStateMachine() {}

bool PowerStateMachine::Init()
{
    POWER_HILOGD(FEATURE_POWER_STATE, "Start init");
    queue_ = std::make_shared<FFRTQueue>("power_state_machine");
    if (queue_ == nullptr) {
        return false;
    }
    stateAction_ = PowerMgrFactory::GetDeviceStateAction();
    InitStateMap();

    if (powerStateCBDeathRecipient_ == nullptr) {
        powerStateCBDeathRecipient_ = new PowerStateCallbackDeathRecipient();
    }
    POWER_HILOGD(FEATURE_POWER_STATE, "Init success");
    return true;
}

void PowerStateMachine::InitState()
{
    POWER_HILOGD(FEATURE_POWER_STATE, "Init power state");
    if (IsScreenOn()) {
        SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_INIT, true);
    } else {
        SetState(PowerState::INACTIVE, StateChangeReason::STATE_CHANGE_REASON_INIT, true);
    }
}

void PowerStateMachine::EmplaceAwake()
{
    controllerMap_.emplace(PowerState::AWAKE,
        std::make_shared<StateController>(PowerState::AWAKE, shared_from_this(), [this](StateChangeReason reason) {
            POWER_HILOGI(FEATURE_POWER_STATE, "StateController_AWAKE lambda start");
            mDeviceState_.screenState.lastOnTime = GetTickCount();
            uint32_t ret = this->stateAction_->SetDisplayState(DisplayState::DISPLAY_ON, reason);
            if (ret != ActionResult::SUCCESS) {
                POWER_HILOGE(FEATURE_POWER_STATE, "Failed to go to AWAKE, display error, ret: %{public}u", ret);
                return TransitResult::DISPLAY_ON_ERR;
            }
            ResetInactiveTimer();
            return TransitResult::SUCCESS;
        }));
}

void PowerStateMachine::EmplaceFreeze()
{
    controllerMap_.emplace(PowerState::FREEZE,
        std::make_shared<StateController>(PowerState::FREEZE, shared_from_this(), [this](StateChangeReason reason) {
            POWER_HILOGI(FEATURE_POWER_STATE, "StateController_FREEZE lambda start");
            // Subsequent added functions
            return TransitResult::SUCCESS;
        }));
}

void PowerStateMachine::EmplaceInactive()
{
    controllerMap_.emplace(PowerState::INACTIVE,
        std::make_shared<StateController>(PowerState::INACTIVE, shared_from_this(), [this](StateChangeReason reason) {
            POWER_HILOGI(FEATURE_POWER_STATE, "StateController_INACTIVE lambda start");
            mDeviceState_.screenState.lastOffTime = GetTickCount();
            DisplayState state = DisplayState::DISPLAY_OFF;
            if (enableDisplaySuspend_) {
                POWER_HILOGI(FEATURE_POWER_STATE, "Display suspend enabled");
                state = DisplayState::DISPLAY_SUSPEND;
            }
            uint32_t ret = this->stateAction_->SetDisplayState(state, reason);
            if (ret != ActionResult::SUCCESS) {
                POWER_HILOGE(FEATURE_POWER_STATE, "Failed to go to INACTIVE, display error, ret: %{public}u", ret);
                return TransitResult::DISPLAY_OFF_ERR;
            }
            return TransitResult::SUCCESS;
        }));
}

void PowerStateMachine::EmplaceStandBy()
{
    controllerMap_.emplace(PowerState::STAND_BY,
        std::make_shared<StateController>(PowerState::STAND_BY, shared_from_this(), [this](StateChangeReason reason) {
            POWER_HILOGI(FEATURE_POWER_STATE, "StateController_STAND_BY lambda start");
            mDeviceState_.screenState.lastOffTime = GetTickCount();
            // Subsequent added functions
            return TransitResult::SUCCESS;
        }));
}

void PowerStateMachine::EmplaceDoze()
{
    controllerMap_.emplace(PowerState::DOZE,
        std::make_shared<StateController>(PowerState::DOZE, shared_from_this(), [this](StateChangeReason reason) {
            POWER_HILOGI(FEATURE_POWER_STATE, "StateController_DOZE lambda start");
            mDeviceState_.screenState.lastOffTime = GetTickCount();
            // Subsequent added functions
            return TransitResult::SUCCESS;
        }));
}

void PowerStateMachine::EmplaceSleep()
{
    controllerMap_.emplace(PowerState::SLEEP,
        std::make_shared<StateController>(PowerState::SLEEP, shared_from_this(), [this](StateChangeReason reason) {
            POWER_HILOGI(FEATURE_POWER_STATE, "StateController_SLEEP lambda start");
            return TransitResult::SUCCESS;
        }));
}

void PowerStateMachine::EmplaceHibernate()
{
    controllerMap_.emplace(PowerState::HIBERNATE,
        std::make_shared<StateController>(PowerState::HIBERNATE, shared_from_this(), [this](StateChangeReason reason) {
            POWER_HILOGI(FEATURE_POWER_STATE, "StateController_HIBERNATE lambda start");
            // Subsequent added functions
            return TransitResult::SUCCESS;
        }));
}

void PowerStateMachine::EmplaceShutdown()
{
    controllerMap_.emplace(PowerState::SHUTDOWN,
        std::make_shared<StateController>(PowerState::SHUTDOWN, shared_from_this(), [this](StateChangeReason reason) {
            POWER_HILOGI(FEATURE_POWER_STATE, "StateController_SHUTDOWN lambda start");
            // Subsequent added functions
            return TransitResult::SUCCESS;
        }));
}

void PowerStateMachine::InitStateMap()
{
    EmplaceAwake();
    EmplaceFreeze();
    EmplaceInactive();
    EmplaceStandBy();
    EmplaceDoze();
    EmplaceSleep();
    EmplaceHibernate();
    EmplaceShutdown();
}

void PowerStateMachine::onSuspend()
{
    POWER_HILOGI(FEATURE_SUSPEND, "System is suspending");
}

void PowerStateMachine::onWakeup()
{
    POWER_HILOGI(FEATURE_WAKEUP, "System is awaking");
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        return;
    }
    FFRTTask task = [&pms] {
        pms->GetPowerStateMachine()->HandleSystemWakeup();
    };
}

void PowerStateMachine::SuspendDeviceInner(
    pid_t pid, int64_t callTimeMs, SuspendDeviceType type, bool suspendImmed, bool ignoreScreenState)
{
    PowerHitrace powerHitrace("SuspendDevice");
    if (type > SuspendDeviceType::SUSPEND_DEVICE_REASON_MAX) {
        POWER_HILOGW(FEATURE_SUSPEND, "Invalid type: %{public}d", type);
        return;
    }
    // Check the screen state
    if (!ignoreScreenState) {
        if (stateAction_ != nullptr) {
            stateAction_->Suspend(
                callTimeMs, type, suspendImmed ? SUSPEND_DEVICE_IMMEDIATELY : SUSPEND_DEVICE_NEED_DOZE);
        }
        mDeviceState_.lastSuspendDeviceTime = callTimeMs;
        POWER_HILOGD(FEATURE_SUSPEND, "Suspend device success");
    } else {
        POWER_HILOGD(FEATURE_SUSPEND, "Do not suspend device, screen state is ignored");
    }

    if (SetState(PowerState::INACTIVE, GetReasionBySuspendType(type), true)) {
        uint32_t delay = 0;
        SetAutoSuspend(type, delay);
    }
    POWER_HILOGD(FEATURE_SUSPEND, "Suspend device finish");
}

void PowerStateMachine::WakeupDeviceInner(
    pid_t pid, int64_t callTimeMs, WakeupDeviceType type, const std::string& details, const std::string& pkgName)
{
    PowerHitrace powerHitrace("WakeupDevice");
    if (type > WakeupDeviceType::WAKEUP_DEVICE_MAX) {
        POWER_HILOGW(FEATURE_WAKEUP, "Invalid type: %{public}d", type);
        return;
    }

    // Call legacy wakeup, Check the screen state
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    auto suspendController = pms->GetSuspendController();
    if (suspendController != nullptr) {
        POWER_HILOGI(FEATURE_WAKEUP, "Stop sleep ffrt task");
        suspendController->StopSleep();
    }

    if (stateAction_ != nullptr) {
        stateAction_->Wakeup(callTimeMs, type, details, pkgName);
    }
    mDeviceState_.lastWakeupDeviceTime = callTimeMs;

    ResetInactiveTimer();
    SetState(PowerState::AWAKE, GetReasonByWakeType(type), true);

    if (suspendController != nullptr) {
        POWER_HILOGI(FEATURE_WAKEUP, "WakeupDeviceInner. TriggerSyncSleepCallback start.");
        suspendController->TriggerSyncSleepCallback(true);
    } else {
        POWER_HILOGD(FEATURE_WAKEUP, "WakeupDeviceInner. suspendController is nullptr");
    }

    POWER_HILOGD(FEATURE_WAKEUP, "Wakeup device finish");
}

void PowerStateMachine::RefreshActivityInner(
    pid_t pid, int64_t callTimeMs, UserActivityType type, bool needChangeBacklight)
{
    if (type > UserActivityType::USER_ACTIVITY_TYPE_MAX) {
        POWER_HILOGW(FEATURE_ACTIVITY, "Invalid type: %{public}d", type);
        return;
    }
    // Check the screen state
    if (IsScreenOn()) {
        if (stateAction_ != nullptr) {
            stateAction_->RefreshActivity(callTimeMs, type,
                needChangeBacklight ? REFRESH_ACTIVITY_NEED_CHANGE_LIGHTS : REFRESH_ACTIVITY_NO_CHANGE_LIGHTS);
            mDeviceState_.screenState.lastOnTime = GetTickCount();
        }
        // reset timer
        ResetInactiveTimer();
    } else {
        POWER_HILOGD(FEATURE_ACTIVITY, "Ignore refresh activity, screen is off");
    }
}

bool PowerStateMachine::CheckRefreshTime()
{
    // The minimum refreshactivity interval is 100ms!!
    int64_t now = GetTickCount();
    if ((mDeviceState_.lastRefreshActivityTime + MIN_TIME_MS_BETWEEN_USERACTIVITIES) > now) {
        return true;
    }
    mDeviceState_.lastRefreshActivityTime = now;
    return false;
}

bool PowerStateMachine::OverrideScreenOffTimeInner(int64_t timeout)
{
    POWER_HILOGI(COMP_SVC, "Override screenOffTime, timeout=%{public}" PRId64 "", timeout);
    if (!isScreenOffTimeOverride_) {
        int64_t beforeOverrideTime = this->GetDisplayOffTime();
        isScreenOffTimeOverride_ = true;
        beforeOverrideTime_ = beforeOverrideTime;
    }
    this->SetDisplayOffTime(timeout, false);
    POWER_HILOGD(COMP_SVC, "Override screenOffTime finish");
    return true;
}

bool PowerStateMachine::RestoreScreenOffTimeInner()
{
    if (!isScreenOffTimeOverride_) {
        POWER_HILOGD(COMP_SVC, "RestoreScreenOffTime is not override, no need to restore");
        return false;
    }
    this->SetDisplayOffTime(beforeOverrideTime_, false);
    isScreenOffTimeOverride_ = false;
    POWER_HILOGD(COMP_SVC, "Restore screenOffTime finish");
    return true;
}

void PowerStateMachine::OverrideScreenOffTimeCoordinated()
{
    if (isCoordinatedOverride_ || !IsRunningLockEnabled(RunningLockType::RUNNINGLOCK_COORDINATION)) {
        POWER_HILOGD(FEATURE_POWER_STATE,
            "Coordianted state override screen off time failed, override flag=%{public}d", isCoordinatedOverride_);
        return;
    }
    OverrideScreenOffTimeInner(COORDINATED_STATE_SCREEN_OFF_TIME_MS);
    isCoordinatedOverride_ = true;
}

void PowerStateMachine::RestoreScreenOffTimeCoordinated()
{
    if (!isCoordinatedOverride_) {
        POWER_HILOGI(FEATURE_POWER_STATE,
            "Coordianted state restore screen off time failed, override flag=%{public}d", isCoordinatedOverride_);
        return;
    }
    RestoreScreenOffTimeInner();
    isCoordinatedOverride_ = false;
}

bool PowerStateMachine::ForceSuspendDeviceInner(pid_t pid, int64_t callTimeMs)
{
    SetState(
        PowerState::INACTIVE, GetReasionBySuspendType(SuspendDeviceType::SUSPEND_DEVICE_REASON_FORCE_SUSPEND), true);
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    auto suspendController = pms->GetSuspendController();
    if (suspendController != nullptr) {
        POWER_HILOGI(FEATURE_SUSPEND, "ForceSuspendDeviceInner StartSleepTimer start.");
        suspendController->StartSleepTimer(
            SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION,
            static_cast<uint32_t>(SuspendAction::ACTION_FORCE_SUSPEND), 0);
    }

    POWER_HILOGI(FEATURE_SUSPEND, "Force suspend finish");
    return true;
}

bool PowerStateMachine::IsScreenOn()
{
    DisplayState state = stateAction_->GetDisplayState();
    if (state == DisplayState::DISPLAY_ON || state == DisplayState::DISPLAY_DIM) {
        POWER_HILOGD(FEATURE_POWER_STATE, "Current screen is on, state: %{public}u", state);
        return true;
    }
    POWER_HILOGD(FEATURE_POWER_STATE, "Current screen is off, state: %{public}u", state);
    return false;
}

void PowerStateMachine::ReceiveScreenEvent(bool isScreenOn)
{
    POWER_HILOGD(FEATURE_POWER_STATE, "Enter");
    std::lock_guard lock(mutex_);
    auto prestate = mDeviceState_.screenState.state;
    if (isScreenOn) {
        mDeviceState_.screenState.lastOnTime = GetTickCount();
    } else {
        mDeviceState_.screenState.lastOffTime = GetTickCount();
    }
    if (prestate != mDeviceState_.screenState.state) {
        NotifyPowerStateChanged(isScreenOn ? PowerState::AWAKE : PowerState::INACTIVE);
    }
}

void PowerStateMachine::RegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback)
{
    std::lock_guard lock(mutex_);
    RETURN_IF(callback == nullptr);
    auto object = callback->AsObject();
    RETURN_IF(object == nullptr);
    auto retIt = powerStateListeners_.insert(callback);
    if (retIt.second) {
        object->AddDeathRecipient(powerStateCBDeathRecipient_);
    }
    POWER_HILOGD(FEATURE_POWER_STATE, "listeners.size = %{public}d, insertOk = %{public}d",
        static_cast<unsigned int>(powerStateListeners_.size()), retIt.second);
}

void PowerStateMachine::UnRegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback)
{
    std::lock_guard lock(mutex_);
    RETURN_IF(callback == nullptr);
    auto object = callback->AsObject();
    RETURN_IF(object == nullptr);
    size_t eraseNum = powerStateListeners_.erase(callback);
    if (eraseNum != 0) {
        object->RemoveDeathRecipient(powerStateCBDeathRecipient_);
    }
    POWER_HILOGD(FEATURE_POWER_STATE, "listeners.size = %{public}d, eraseNum = %{public}zu",
        static_cast<unsigned int>(powerStateListeners_.size()), eraseNum);
}

void PowerStateMachine::EnableMock(IDeviceStateAction* mockAction)
{
    std::lock_guard lock(mutex_);
    // reset to awake state when mock and default off/sleep time
    currentState_ = PowerState::AWAKE;
    displayOffTime_ = DEFAULT_DISPLAY_OFF_TIME;
    sleepTime_ = DEFAULT_SLEEP_TIME;
    ResetInactiveTimer();

    std::unique_ptr<IDeviceStateAction> mock(mockAction);
    stateAction_.reset();
    stateAction_ = std::move(mock);
}

void PowerStateMachine::NotifyPowerStateChanged(PowerState state)
{
    if (GetState() == PowerState::INACTIVE && IsRunningLockEnabled(RunningLockType::RUNNINGLOCK_COORDINATION)) {
        POWER_HILOGI(FEATURE_POWER_STATE, "Coordination is enabled, not notify power state");
        return;
    }
    POWER_HILOGD(
        FEATURE_POWER_STATE, "state=%{public}u, listeners.size=%{public}zu", state, powerStateListeners_.size());
    HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::POWER, "STATE", HiviewDFX::HiSysEvent::EventType::STATISTIC,
        "STATE", static_cast<uint32_t>(state));
    std::lock_guard lock(mutex_);
    int64_t now = GetTickCount();
    // Send Notification event
    SendEventToPowerMgrNotify(state, now);

    // Call back all native function
    for (auto& listener : powerStateListeners_) {
        listener->OnPowerStateChanged(state);
    }
}

void PowerStateMachine::SendEventToPowerMgrNotify(PowerState state, int64_t callTime)
{
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        POWER_HILOGE(FEATURE_POWER_STATE, "Pms is nullptr");
        return;
    }
    auto notify = pms->GetPowerMgrNotify();
    if (notify == nullptr) {
        POWER_HILOGE(FEATURE_POWER_STATE, "Notify is null");
        return;
    }
    if (state == PowerState::AWAKE) {
        notify->PublishScreenOnEvents(callTime);
    } else if (state == PowerState::INACTIVE) {
        notify->PublishScreenOffEvents(callTime);
    } else {
        POWER_HILOGI(FEATURE_POWER_STATE, "No need to publish event, state:%{public}u", state);
    }
}

void PowerStateMachine::PowerStateCallbackDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    if (remote == nullptr || remote.promote() == nullptr) {
        return;
    }
    sptr<IPowerStateCallback> callback = iface_cast<IPowerStateCallback>(remote.promote());
    FFRTTask unRegFunc = std::bind([](const sptr<IPowerStateCallback>& cb) {
        auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
        if (pms == nullptr) {
            POWER_HILOGE(FEATURE_POWER_STATE, "Pms is nullptr");
            return;
        }
        pms->UnRegisterPowerStateCallback(cb);
    }, callback);
    FFRTUtils::SubmitTask(unRegFunc);
}

void PowerStateMachine::SetDelayTimer(int64_t delayTime, int32_t event)
{
    POWER_HILOGD(FEATURE_ACTIVITY, "Set delay timer, delayTime=%{public}s, event=%{public}d",
        std::to_string(delayTime).c_str(), event);
    switch (event) {
        case CHECK_USER_ACTIVITY_TIMEOUT_MSG: {
            std::lock_guard lock(ffrtMutex_);
            FFRTTask task = std::bind(&PowerStateMachine::HandleActivityTimeout, this);
            userActivityTimeoutHandle_ = FFRTUtils::SubmitDelayTask(task, delayTime, queue_);
            break;
        }
        case CHECK_USER_ACTIVITY_OFF_TIMEOUT_MSG: {
            auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
            auto suspendController = pms->GetSuspendController();
            if (suspendController == nullptr) {
                POWER_HILOGW(FEATURE_ACTIVITY, "suspendController is nullptr");
                return;
            }
            suspendController->HandleEvent(delayTime);
            break;
        }
        default: {
            break;
        }
    }
}

void PowerStateMachine::CancelDelayTimer(int32_t event)
{
    POWER_HILOGD(FEATURE_ACTIVITY, "Cancel delay timer, event: %{public}d", event);
    switch (event) {
        case CHECK_USER_ACTIVITY_TIMEOUT_MSG: {
            std::lock_guard lock(ffrtMutex_);
            FFRTUtils::CancelTask(userActivityTimeoutHandle_, queue_);
            break;
        }
        case CHECK_USER_ACTIVITY_OFF_TIMEOUT_MSG: {
            auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
            auto suspendController = pms->GetSuspendController();
            if (suspendController == nullptr) {
                POWER_HILOGW(FEATURE_ACTIVITY, "suspendController is nullptr");
                return;
            }
            suspendController->CancelEvent();
            break;
        }
        default: {
            break;
        }
    }
}

void PowerStateMachine::ResetInactiveTimer()
{
    CancelDelayTimer(PowerStateMachine::CHECK_USER_ACTIVITY_TIMEOUT_MSG);
    CancelDelayTimer(PowerStateMachine::CHECK_USER_ACTIVITY_OFF_TIMEOUT_MSG);
    if (this->GetDisplayOffTime() < 0) {
        POWER_HILOGD(FEATURE_ACTIVITY, "Auto display off is disabled");
        return;
    }

    if (this->CheckRunningLock(PowerState::INACTIVE)) {
        const double DIMTIMERATE = 2.0/3;
        this->SetDelayTimer(
            this->GetDisplayOffTime() * DIMTIMERATE, PowerStateMachine::CHECK_USER_ACTIVITY_TIMEOUT_MSG);
    }
}

void PowerStateMachine::ResetSleepTimer()
{
    CancelDelayTimer(PowerStateMachine::CHECK_USER_ACTIVITY_TIMEOUT_MSG);
    CancelDelayTimer(PowerStateMachine::CHECK_USER_ACTIVITY_OFF_TIMEOUT_MSG);
    if (this->GetSleepTime() < 0) {
        POWER_HILOGD(FEATURE_ACTIVITY, "Auto sleep is disabled");
        return;
    }
}

void PowerStateMachine::SetAutoSuspend(SuspendDeviceType type, uint32_t delay)
{
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        POWER_HILOGE(FEATURE_SUSPEND, "Pms is nullptr");
        return;
    }
    auto suspendController = pms->GetSuspendController();
    if (suspendController == nullptr) {
        POWER_HILOGE(FEATURE_SUSPEND, "Suspend controller is nullptr");
        return;
    }
    suspendController->StartSleepTimer(type, static_cast<uint32_t>(SuspendAction::ACTION_AUTO_SUSPEND), delay);
    POWER_HILOGD(FEATURE_SUSPEND, "Set auto suspend finish");
}

void PowerStateMachine::HandleActivityTimeout()
{
    POWER_HILOGD(FEATURE_ACTIVITY, "Enter, displayState = %{public}d", stateAction_->GetDisplayState());
    DisplayState dispState = stateAction_->GetDisplayState();
    const uint32_t THREE = 3;
    if (!this->CheckRunningLock(PowerState::INACTIVE)) {
        POWER_HILOGI(FEATURE_ACTIVITY, "RunningLock is blocking to transit to INACTIVE");
        return;
    }
    if (dispState == DisplayState::DISPLAY_ON) {
        stateAction_->SetDisplayState(DisplayState::DISPLAY_DIM, StateChangeReason::STATE_CHANGE_REASON_TIMEOUT);
        if (this->GetDisplayOffTime() < 0) {
            POWER_HILOGD(FEATURE_ACTIVITY, "Auto display off is disabled");
            return;
        } else {
            SetDelayTimer(GetDisplayOffTime() / THREE, PowerStateMachine::CHECK_USER_ACTIVITY_OFF_TIMEOUT_MSG);
        }
    } else {
        if (this->GetDisplayOffTime() < 0) {
            POWER_HILOGD(FEATURE_ACTIVITY, "Auto display off is disabled");
            return;
        }
        CancelDelayTimer(PowerStateMachine::CHECK_USER_ACTIVITY_OFF_TIMEOUT_MSG);
        SetDelayTimer(GetDisplayOffTime() / THREE, PowerStateMachine::CHECK_USER_ACTIVITY_OFF_TIMEOUT_MSG);
    }
}

void PowerStateMachine::HandleActivitySleepTimeout()
{
    POWER_HILOGD(FEATURE_ACTIVITY, "Enter, displayState = %{public}d", stateAction_->GetDisplayState());
    if (!this->CheckRunningLock(PowerState::SLEEP)) {
        POWER_HILOGW(FEATURE_POWER_STATE, "RunningLock is blocking to transit to SLEEP");
        return;
    }
    DisplayState dispState = stateAction_->GetDisplayState();
    if (dispState == DisplayState::DISPLAY_OFF) {
        SetState(PowerState::SLEEP, StateChangeReason::STATE_CHANGE_REASON_TIMEOUT);
    } else {
        POWER_HILOGW(FEATURE_ACTIVITY, "Display is on, ignore activity sleep timeout, state = %{public}d", dispState);
    }
}

void PowerStateMachine::HandleSystemWakeup()
{
    POWER_HILOGD(FEATURE_WAKEUP, "Enter, displayState = %{public}d", stateAction_->GetDisplayState());
    if (IsScreenOn()) {
        SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_SYSTEM, true);
    } else {
        SetState(PowerState::INACTIVE, StateChangeReason::STATE_CHANGE_REASON_SYSTEM, true);
    }
}

bool PowerStateMachine::CheckRunningLock(PowerState state)
{
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "Enter, state = %{public}u", state);
    auto pms = pms_.promote();
    if (pms == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Pms is nullptr");
        return false;
    }
    auto runningLockMgr = pms->GetRunningLockMgr();
    if (runningLockMgr == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "RunningLockMgr is nullptr");
        return false;
    }
    auto iterator = lockMap_.find(state);
    if (iterator == lockMap_.end()) {
        POWER_HILOGI(FEATURE_RUNNING_LOCK, "No specific lock in lockMap_ for state: %{public}u", state);
        return true;
    }

    std::shared_ptr<std::vector<RunningLockType>> pLock = iterator->second;
    for (std::vector<RunningLockType>::const_iterator iter = pLock->begin(); iter != pLock->end(); ++iter) {
        uint32_t count = runningLockMgr->GetValidRunningLockNum(*iter);
        if (count > 0) {
            POWER_HILOGD(FEATURE_POWER_STATE,
                "RunningLock %{public}s is locking (count=%{public}d), blocking %{public}s",
                PowerUtils::GetRunningLockTypeString(*iter).c_str(), count,
                PowerUtils::GetPowerStateString(state).c_str());
            return false;
        }
    }

    POWER_HILOGI(FEATURE_RUNNING_LOCK, "No specific lock for state: %{public}u", state);
    return true;
}

bool PowerStateMachine::IsRunningLockEnabled(RunningLockType type)
{
    auto pms = pms_.promote();
    if (pms == nullptr) {
        POWER_HILOGE(FEATURE_POWER_STATE, "Pms is nullptr");
        return false;
    }
    auto runningLockMgr = pms->GetRunningLockMgr();
    if (runningLockMgr == nullptr) {
        POWER_HILOGE(FEATURE_POWER_STATE, "RunningLockMgr is nullptr");
        return false;
    }
    if (runningLockMgr->GetValidRunningLockNum(type) > 0) {
        return true;
    }
    return false;
}

void PowerStateMachine::SetDisplayOffTime(int64_t time, bool needUpdateSetting)
{
    POWER_HILOGI(FEATURE_POWER_STATE, "set display off time %{public}" PRId64 " -> %{public}" PRId64 "",
        displayOffTime_.load(), time);
    displayOffTime_ = time;
    if (currentState_ == PowerState::AWAKE) {
        ResetInactiveTimer();
    }
    if (needUpdateSetting) {
        SettingHelper::SetSettingDisplayOffTime(displayOffTime_);
    }
}

static void DisplayOffTimeUpdateFunc()
{
    auto stateMachine = DelayedSpSingleton<PowerMgrService>::GetInstance()->GetPowerStateMachine();
    int64_t systemTime = stateMachine->GetDisplayOffTime();
    auto settingTime = SettingHelper::GetSettingDisplayOffTime(systemTime);
    if (settingTime == systemTime) {
        return;
    }
    POWER_HILOGD(FEATURE_POWER_STATE, "setting update display off time %{public}" PRId64 " -> %{public}" PRId64 "",
        systemTime, settingTime);
    stateMachine->SetDisplayOffTime(settingTime, false);
}

void PowerStateMachine::RegisterDisplayOffTimeObserver()
{
    if (g_displayOffTimeObserver) {
        POWER_HILOGI(FEATURE_POWER_STATE, "setting display off time observer is already registered");
        return;
    }
    DisplayOffTimeUpdateFunc();
    SettingObserver::UpdateFunc updateFunc = [&](const std::string&) {
        DisplayOffTimeUpdateFunc();
    };
    g_displayOffTimeObserver = SettingHelper::RegisterSettingDisplayOffTimeObserver(updateFunc);
}

void PowerStateMachine::UnregisterDisplayOffTimeObserver()
{
    if (g_displayOffTimeObserver == nullptr) {
        POWER_HILOGD(FEATURE_POWER_STATE, "g_displayOffTimeObserver is nullptr, no need to unregister");
        return;
    }
    SettingHelper::UnregisterSettingDisplayOffTimeObserver(g_displayOffTimeObserver);
    g_displayOffTimeObserver = nullptr;
}

void PowerStateMachine::SetSleepTime(int64_t time)
{
    sleepTime_ = time;
}

int64_t PowerStateMachine::GetDisplayOffTime()
{
    return displayOffTime_;
}

int64_t PowerStateMachine::GetSleepTime()
{
    return sleepTime_;
}

bool PowerStateMachine::SetState(PowerState state, StateChangeReason reason, bool force)
{
    POWER_HILOGD(FEATURE_POWER_STATE, "state=%{public}d, reason=%{public}d, force=%{public}d", state, reason, force);
    auto iterator = controllerMap_.find(state);
    if (iterator == controllerMap_.end()) {
        return false;
    }
    std::shared_ptr<StateController> pController = iterator->second;
    if (pController == nullptr) {
        POWER_HILOGW(FEATURE_POWER_STATE, "StateController is not init");
        return false;
    }
    TransitResult ret = pController->TransitTo(reason, force);
    POWER_HILOGI(FEATURE_POWER_STATE, "StateController::TransitTo ret: %{public}d", ret);
    return (ret == TransitResult::SUCCESS || ret == TransitResult::ALREADY_IN_STATE);
}

void PowerStateMachine::SetDisplaySuspend(bool enable)
{
    POWER_HILOGD(FEATURE_POWER_STATE, "enable: %{public}d", enable);
    enableDisplaySuspend_ = enable;
    if (GetState() == PowerState::INACTIVE) {
        POWER_HILOGI(FEATURE_POWER_STATE, "Change display state");
        if (enable) {
            stateAction_->SetDisplayState(
                DisplayState::DISPLAY_SUSPEND, StateChangeReason::STATE_CHANGE_REASON_APPLICATION);
        } else {
            stateAction_->SetDisplayState(
                DisplayState::DISPLAY_OFF, StateChangeReason::STATE_CHANGE_REASON_APPLICATION);
        }
    }
}

StateChangeReason PowerStateMachine::GetReasonByUserActivity(UserActivityType type)
{
    StateChangeReason ret = StateChangeReason::STATE_CHANGE_REASON_UNKNOWN;
    switch (type) {
        case UserActivityType::USER_ACTIVITY_TYPE_BUTTON:
            ret = StateChangeReason::STATE_CHANGE_REASON_HARD_KEY;
            break;
        case UserActivityType::USER_ACTIVITY_TYPE_TOUCH:
            ret = StateChangeReason::STATE_CHANGE_REASON_TOUCH;
            break;
        case UserActivityType::USER_ACTIVITY_TYPE_ACCESSIBILITY:
            ret = StateChangeReason::STATE_CHANGE_REASON_ACCESSIBILITY;
            break;
        case UserActivityType::USER_ACTIVITY_TYPE_SOFTWARE:
            ret = StateChangeReason::STATE_CHANGE_REASON_APPLICATION;
            break;
        case UserActivityType::USER_ACTIVITY_TYPE_ATTENTION: // fail through
        case UserActivityType::USER_ACTIVITY_TYPE_OTHER:     // fail through
        default:
            break;
    }
    return ret;
}

StateChangeReason PowerStateMachine::GetReasonByWakeType(WakeupDeviceType type)
{
    StateChangeReason ret = StateChangeReason::STATE_CHANGE_REASON_UNKNOWN;
    switch (type) {
        case WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON:
            ret = StateChangeReason::STATE_CHANGE_REASON_POWER_KEY;
            break;
        case WakeupDeviceType::WAKEUP_DEVICE_WAKE_KEY:
            ret = StateChangeReason::STATE_CHANGE_REASON_HARD_KEY;
            break;
        case WakeupDeviceType::WAKEUP_DEVICE_APPLICATION:
            ret = StateChangeReason::STATE_CHANGE_REASON_APPLICATION;
            break;
        case WakeupDeviceType::WAKEUP_DEVICE_PLUGGED_IN: // fall through
        case WakeupDeviceType::WAKEUP_DEVICE_HDMI:
            ret = StateChangeReason::STATE_CHANGE_REASON_CABLE;
            break;
        case WakeupDeviceType::WAKEUP_DEVICE_GESTURE:
            ret = StateChangeReason::STATE_CHANGE_REASON_TOUCH;
            break;
        case WakeupDeviceType::WAKEUP_DEVICE_CAMERA_LAUNCH:
            ret = StateChangeReason::STATE_CHANGE_REASON_CAMERA;
            break;
        case WakeupDeviceType::WAKEUP_DEVICE_WAKE_MOTION:
            ret = StateChangeReason::STATE_CHANGE_REASON_SENSOR;
            break;
        case WakeupDeviceType::WAKEUP_DEVICE_LID:
            ret = StateChangeReason::STATE_CHANGE_REASON_LID;
            break;
        case WakeupDeviceType::WAKEUP_DEVICE_DOUBLE_CLICK:
            ret = StateChangeReason::STATE_CHANGE_REASON_DOUBLE_CLICK;
            break;
        case WakeupDeviceType::WAKEUP_DEVICE_KEYBOARD:
            ret = StateChangeReason::STATE_CHANGE_REASON_KEYBOARD;
            break;
        case WakeupDeviceType::WAKEUP_DEVICE_MOUSE:
            ret = StateChangeReason::STATE_CHANGE_REASON_MOUSE;
            break;
        case WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN: // fail through
        default:
            break;
    }
    return ret;
}

StateChangeReason PowerStateMachine::GetReasionBySuspendType(SuspendDeviceType type)
{
    POWER_HILOGD(FEATURE_SUSPEND, "SuspendDeviceType: %{public}u", type);
    StateChangeReason ret = StateChangeReason::STATE_CHANGE_REASON_UNKNOWN;
    switch (type) {
        case SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION:
            ret = StateChangeReason::STATE_CHANGE_REASON_APPLICATION;
            break;
        case SuspendDeviceType::SUSPEND_DEVICE_REASON_DEVICE_ADMIN:
            ret = StateChangeReason::STATE_CHANGE_REASON_REMOTE;
            break;
        case SuspendDeviceType::SUSPEND_DEVICE_REASON_TIMEOUT:
            ret = StateChangeReason::STATE_CHANGE_REASON_TIMEOUT;
            break;
        case SuspendDeviceType::SUSPEND_DEVICE_REASON_LID:
            ret = StateChangeReason::STATE_CHANGE_REASON_LID;
            break;
        case SuspendDeviceType::SUSPEND_DEVICE_REASON_POWER_KEY: // fall through
        case SuspendDeviceType::SUSPEND_DEVICE_REASON_SLEEP_KEY:
            ret = StateChangeReason::STATE_CHANGE_REASON_HARD_KEY;
            break;
        case SuspendDeviceType::SUSPEND_DEVICE_REASON_HDMI:
            ret = StateChangeReason::STATE_CHANGE_REASON_CABLE;
            break;
        case SuspendDeviceType::SUSPEND_DEVICE_REASON_ACCESSIBILITY:
            ret = StateChangeReason::STATE_CHANGE_REASON_ACCESSIBILITY;
            break;
        case SuspendDeviceType::SUSPEND_DEVICE_REASON_FORCE_SUSPEND:
            ret = StateChangeReason::STATE_CHANGE_REASON_SYSTEM;
            break;
        default:
            break;
    }
    POWER_HILOGD(FEATURE_SUSPEND, "StateChangeReason: %{public}u", ret);
    return ret;
}

void PowerStateMachine::AppendDumpInfo(std::string& result, std::string& reason, std::string& time)
{
    result.append("POWER STATE DUMP:\n");
    result.append("Current State: ")
        .append(PowerUtils::GetPowerStateString(GetState()))
        .append("  Reason: ")
        .append(reason)
        .append("  Time: ")
        .append(time)
        .append("\n");

    result.append("ScreenOffTime: Timeout=");
    if (isScreenOffTimeOverride_) {
        result.append((ToString(beforeOverrideTime_)))
            .append("ms  OverrideTimeout=")
            .append((ToString(GetDisplayOffTime())))
            .append("ms\n");
    } else {
        result.append((ToString(GetDisplayOffTime()))).append("ms\n");
    }

    result.append("DUMP DETAILS:\n");
    result.append("Last Screen On: ").append(ToString(mDeviceState_.screenState.lastOnTime)).append("\n");
    result.append("Last Screen Off: ").append(ToString(mDeviceState_.screenState.lastOffTime)).append("\n");
    result.append("Last SuspendDevice: ").append(ToString(mDeviceState_.lastSuspendDeviceTime)).append("\n");
    result.append("Last WakeupDevice: ").append(ToString(mDeviceState_.lastWakeupDeviceTime)).append("\n");
    result.append("Last Refresh: ").append(ToString(mDeviceState_.lastRefreshActivityTime)).append("\n");

    result.append("DUMP EACH STATES:\n");
    for (auto it = controllerMap_.begin(); it != controllerMap_.end(); it++) {
        result.append("State: ")
            .append(PowerUtils::GetPowerStateString(it->second->GetState()))
            .append("   Reason: ")
            .append(PowerUtils::GetReasonTypeString(it->second->lastReason_).c_str())
            .append("   Time: ")
            .append(ToString(it->second->lastTime_))
            .append("\n")
            .append("   Failure: ")
            .append(PowerUtils::GetReasonTypeString(it->second->failTrigger_).c_str())
            .append("   Reason: ")
            .append(it->second->failReasion_)
            .append("   From: ")
            .append(PowerUtils::GetPowerStateString(it->second->failFrom_))
            .append("   Time: ")
            .append(ToString(it->second->failTime_))
            .append("\n\n");
    }
}

void PowerStateMachine::DumpInfo(std::string& result)
{
    std::string reason = "UNKNOWN";
    std::string time = "UNKNOWN";
    auto it = controllerMap_.find(GetState());
    if (it != controllerMap_.end() && it->second != nullptr) {
        reason = ToString(static_cast<uint32_t>(it->second->lastReason_));
        time = ToString(it->second->lastTime_);
    }
    AppendDumpInfo(result, reason, time);
}

TransitResult PowerStateMachine::StateController::TransitTo(StateChangeReason reason, bool ignoreLock)
{
    POWER_HILOGD(FEATURE_POWER_STATE, "Start");
    std::shared_ptr<PowerStateMachine> owner = owner_.lock();
    if (owner == nullptr) {
        POWER_HILOGW(FEATURE_POWER_STATE, "owner is nullptr");
        return TransitResult::OTHER_ERR;
    }
    POWER_HILOGI(FEATURE_POWER_STATE, "Transit from %{public}s to %{public}s for %{public}s ignoreLock=%{public}d",
        PowerUtils::GetPowerStateString(owner->currentState_).c_str(),
        PowerUtils::GetPowerStateString(this->state_).c_str(),
        PowerUtils::GetReasonTypeString(reason).c_str(), ignoreLock);
    MatchState(owner->currentState_, owner->stateAction_->GetDisplayState());
    if (!CheckState()) {
        POWER_HILOGD(FEATURE_POWER_STATE, "Already in state: %{public}d", owner->currentState_);
        RecordFailure(owner->currentState_, reason, TransitResult::ALREADY_IN_STATE);
        return TransitResult::ALREADY_IN_STATE;
    }
    if (!ignoreLock && !owner->CheckRunningLock(GetState())) {
        POWER_HILOGD(FEATURE_POWER_STATE, "Running lock block");
        RecordFailure(owner->currentState_, reason, TransitResult::LOCKING);
        return TransitResult::LOCKING;
    }
    TransitResult ret = action_(reason);
    if (ret == TransitResult::SUCCESS) {
        lastReason_ = reason;
        lastTime_ = GetTickCount();
        owner->currentState_ = GetState();
        owner->NotifyPowerStateChanged(owner->currentState_);
    } else {
        RecordFailure(owner->currentState_, reason, ret);
    }

    POWER_HILOGD(FEATURE_POWER_STATE, "Finish, result: %{public}d", ret);
    return ret;
}

bool PowerStateMachine::StateController::CheckState()
{
    std::shared_ptr<PowerStateMachine> owner = owner_.lock();
    if (owner == nullptr) {
        POWER_HILOGW(FEATURE_POWER_STATE, "Owner is nullptr");
        return false;
    }
    auto state = GetState();
    POWER_HILOGD(FEATURE_POWER_STATE, "state: %{public}u, currentState_: %{public}u", state, owner->currentState_);
    return state != owner->currentState_;
}

void PowerStateMachine::StateController::CorrectState(
    PowerState& currentState, PowerState correctState, DisplayState state)
{
    std::string msg = "Correct power state errors from";
    msg.append(PowerUtils::GetPowerStateString(currentState))
        .append(" to ")
        .append(PowerUtils::GetPowerStateString(correctState))
        .append(" due to cuurent display state is ")
        .append(PowerUtils::GetDisplayStateString(state));
    POWER_HILOGW(FEATURE_POWER_STATE, "%{public}s", msg.c_str());
    HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::POWER, "STATE_CORRECTION", HiviewDFX::HiSysEvent::EventType::FAULT,
        "ERROR_STATE", static_cast<uint32_t>(currentState), "CORRECTION_STATE", static_cast<uint32_t>(correctState),
        "DISPLAY_STATE", static_cast<uint32_t>(state), "MSG", msg);
    currentState = correctState;
}

void PowerStateMachine::StateController::MatchState(PowerState& currentState, DisplayState state)
{
    if (GetState() == PowerState::SLEEP || currentState == PowerState::SLEEP || GetState() == PowerState::HIBERNATE ||
        currentState == PowerState::HIBERNATE || GetState() == PowerState::SHUTDOWN ||
        currentState == PowerState::SHUTDOWN) {
        return;
    }

    // Keep the state of display consistent with the state of power
    switch (state) {
        case DisplayState::DISPLAY_OFF:
            if (currentState == PowerState::AWAKE || currentState == PowerState::FREEZE) {
                CorrectState(currentState, PowerState::INACTIVE, state);
            }
            break;
        case DisplayState::DISPLAY_DIM:
        case DisplayState::DISPLAY_ON:
            if (currentState == PowerState::INACTIVE || currentState == PowerState::STAND_BY ||
                currentState == PowerState::DOZE) {
                CorrectState(currentState, PowerState::AWAKE, state);
            }
            break;
        case DisplayState::DISPLAY_SUSPEND:
        case DisplayState::DISPLAY_UNKNOWN:
        default:
            break;
    }
}

void PowerStateMachine::Reset()
{
    queue_.reset();
}

void PowerStateMachine::StateController::RecordFailure(
    PowerState from, StateChangeReason trigger, TransitResult failReason)
{
    failFrom_ = from;
    failTrigger_ = trigger;
    failTime_ = GetTickCount();
    switch (failReason) {
        case TransitResult::ALREADY_IN_STATE:
            failReasion_ = "Already in the state";
            break;
        case TransitResult::LOCKING:
            failReasion_ = "Blocked by running lock";
            break;
        case TransitResult::HDI_ERR:
            failReasion_ = "Power HDI error";
            break;
        case TransitResult::DISPLAY_ON_ERR:
            failReasion_ = "SetDisplayState(ON) error";
            break;
        case TransitResult::DISPLAY_OFF_ERR:
            failReasion_ = "SetDisplayState(OFF) error";
            break;
        case TransitResult::OTHER_ERR:
        default:
            failReasion_ = "Unknown Error";
            break;
    }
    std::string message = "State Transit Failed from ";
    message.append(PowerUtils::GetPowerStateString(failFrom_))
        .append(" to ")
        .append(PowerUtils::GetPowerStateString(GetState()))
        .append(" by ")
        .append(PowerUtils::GetReasonTypeString(failTrigger_).c_str())
        .append("   Reason:")
        .append(failReasion_)
        .append("   Time:")
        .append(ToString(failTime_))
        .append("\n");
    const int logLevel = 2;
    const std::string tag = "TAG_POWER";
    HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::POWER, "SCREEN", HiviewDFX::HiSysEvent::EventType::FAULT,
        "LOG_LEVEL", logLevel, "TAG", tag, "MESSAGE", message);
    POWER_HILOGI(FEATURE_POWER_STATE, "RecordFailure: %{public}s", message.c_str());
}
} // namespace PowerMgr
} // namespace OHOS
