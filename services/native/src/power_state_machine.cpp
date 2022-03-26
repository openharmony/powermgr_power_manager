/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include <file_ex.h>
#include <hisysevent.h>
#include <pubdef.h>
#include <sys/eventfd.h>
#include <unistd.h>

#include "powerms_event_handler.h"
#include "power_mgr_client.h"
#include "power_mgr_factory.h"
#include "power_mgr_service.h"

namespace OHOS {
namespace PowerMgr {
PowerStateMachine::PowerStateMachine(const wptr<PowerMgrService>& pms)
    : pms_(pms), currentState_(PowerState::UNKNOWN)
{
    POWER_HILOGI(MODULE_SERVICE, "PowerStateMachine_currentState: func is Start.");
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
    std::vector<RunningLockType> inactiveBlocker {
        RunningLockType::RUNNINGLOCK_SCREEN,
        RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL
    };
    std::vector<RunningLockType> sleepBlocker {
        RunningLockType::RUNNINGLOCK_BACKGROUND
    };
    lockMap_.emplace(PowerState::AWAKE,
        std::make_shared<std::vector<RunningLockType>>(awakeBlocker));
    lockMap_.emplace(PowerState::INACTIVE,
        std::make_shared<std::vector<RunningLockType>>(inactiveBlocker));
    lockMap_.emplace(PowerState::SLEEP,
        std::make_shared<std::vector<RunningLockType>>(sleepBlocker));

    POWER_HILOGI(MODULE_SERVICE, "PowerStateMachine_currentState: func is End.");
}

PowerStateMachine::~PowerStateMachine() {}

bool PowerStateMachine::Init()
{
    POWER_HILOGI(MODULE_SERVICE, "PowerStateMachine:: Init start");

    stateAction_ = PowerMgrFactory::GetDeviceStateAction();
    std::function<void(uint32_t)> callback = std::bind(&PowerStateMachine::ActionCallback,
        this, std::placeholders::_1);
    stateAction_->RegisterCallback(callback);
    InitStateMap();

    if (powerStateCBDeathRecipient_ == nullptr) {
        powerStateCBDeathRecipient_ = new PowerStateCallbackDeathRecipient();
    }

    if (!powerMgrMonitor_.Start()) {
        POWER_HILOGE(MODULE_SERVICE, "Failed to start monitor");
        return false;
    }

    if (IsScreenOn()) {
        SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_INIT, true);
    } else {
        SetState(PowerState::INACTIVE, StateChangeReason::STATE_CHANGE_REASON_INIT, true);
    }
    POWER_HILOGI(MODULE_SERVICE, "PowerStateMachine:: Init success!");
    return true;
}

void PowerStateMachine::EmplaceAwake()
{
    controllerMap_.emplace(PowerState::AWAKE,
        std::make_shared<StateController>(PowerState::AWAKE, shared_from_this(),
        [this](StateChangeReason reason) {
            mDeviceState_.screenState.lastOnTime = GetTickCount();
            uint32_t ret = this->stateAction_->SetDisplayState(DisplayState::DISPLAY_ON, reason);
            // Display power service maybe not ready when init
            if (ret != ActionResult::SUCCESS
                && reason != StateChangeReason::STATE_CHANGE_REASON_INIT) {
                POWER_HILOGE(MODULE_SERVICE, "Failed to go to AWAKE, Display Err");
                return TransitResult::DISPLAY_ON_ERR;
            }
            ResetInactiveTimer();
            return TransitResult::SUCCESS;
        })
    );
}
void PowerStateMachine::EmplaceInactive()
{
    controllerMap_.emplace(PowerState::INACTIVE,
        std::make_shared<StateController>(PowerState::INACTIVE, shared_from_this(),
        [this](StateChangeReason reason) {
            POWER_HILOGI(MODULE_SERVICE, "StateController_INACTIVE: func is Start.");
            mDeviceState_.screenState.lastOffTime = GetTickCount();
            DisplayState state = DisplayState::DISPLAY_OFF;
            if (enableDisplaySuspend_) {
                POWER_HILOGI(MODULE_SERVICE, "display suspend enabled");
                state = DisplayState::DISPLAY_SUSPEND;
            }
            uint32_t ret = this->stateAction_->SetDisplayState(state, reason);
            // Display power service maybe not ready when init
            if (ret != ActionResult::SUCCESS
                && reason != StateChangeReason::STATE_CHANGE_REASON_INIT) {
                POWER_HILOGE(MODULE_SERVICE, "Failed to go to INACTIVE, Display Err");
                return TransitResult::DISPLAY_OFF_ERR;
            }
            ResetSleepTimer();
            return TransitResult::SUCCESS;
        })
    );
}
void PowerStateMachine::EmplaceSleep()
{
    controllerMap_.emplace(PowerState::SLEEP,
        std::make_shared<StateController>(PowerState::SLEEP, shared_from_this(),
        [this](StateChangeReason reason) {
            DisplayState state = DisplayState::DISPLAY_OFF;
            if (enableDisplaySuspend_) {
                POWER_HILOGI(MODULE_SERVICE, "display suspend enabled");
                state = DisplayState::DISPLAY_SUSPEND;
            }
            uint32_t ret = this->stateAction_->GoToSleep(onSuspend, onWakeup, false);
            if (ret != ActionResult::SUCCESS) {
                POWER_HILOGE(MODULE_SERVICE, "Failed to go to SLEEP, Sleep Err");
                return TransitResult::HDI_ERR;
            }
            return TransitResult::SUCCESS;
        })
    );
}
void PowerStateMachine::InitStateMap()
{
    EmplaceAwake();
    EmplaceInactive();
    EmplaceSleep();
}

void PowerStateMachine::ActionCallback(uint32_t event)
{
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        return;
    }
    pms->NotifyDisplayActionDone(event);
}

void PowerStateMachine::onSuspend()
{
    POWER_HILOGI(MODULE_SERVICE, "System is suspending");
}

void PowerStateMachine::onWakeup()
{
    POWER_HILOGI(MODULE_SERVICE, "System is awake");
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        return;
    }
    auto handler = pms->GetHandler();
    if (handler == nullptr) {
        POWER_HILOGE(MODULE_SERVICE, "SetDelayTimer handler is null");
        return;
    }
    handler->SendEvent(PowermsEventHandler::SYSTEM_WAKE_UP_MSG, 0, 0);
}

void PowerStateMachine::SuspendDeviceInner(pid_t pid,
    int64_t callTimeMs,
    SuspendDeviceType type,
    bool suspendImmed,
    bool ignoreScreenState)
{
    POWER_HILOGI(MODULE_SERVICE, "PID: %{public}d Try to Suspend Device!!", pid);
    if (type > SuspendDeviceType::SUSPEND_DEVICE_REASON_MAX) {
        POWER_HILOGE(MODULE_SERVICE, "Invalid type: %{public}d", type);
        return;
    }
    // Check the screen state
    if (!ignoreScreenState) {
        if (stateAction_ != nullptr) {
            stateAction_->Suspend(callTimeMs, type, suspendImmed ?
                SUSPEND_DEVICE_IMMEDIATELY : SUSPEND_DEVICE_NEED_DOZE);
        }
        mDeviceState_.lastSuspendDeviceTime = callTimeMs;
        POWER_HILOGD(MODULE_SERVICE, "Suspend Device Call Binder Success!!");
    } else {
        POWER_HILOGE(MODULE_SERVICE, "Suspend Device Failed, Screen State is ignored!");
    }

    SetState(PowerState::INACTIVE, GetReasionBySuspendType(type), true);
    POWER_HILOGI(MODULE_SERVICE, "SuspendDeviceInner: fun is End!");
}

void PowerStateMachine::WakeupDeviceInner(pid_t pid,
    int64_t callTimeMs,
    WakeupDeviceType type,
    const std::string& details,
    const std::string& pkgName)
{
    POWER_HILOGI(MODULE_SERVICE, "PID: %{public}d Try to Wakeup Device!!", pid);
    if (type > WakeupDeviceType::WAKEUP_DEVICE_MAX) {
        POWER_HILOGE(MODULE_SERVICE, "Invalid type: %{public}d", type);
        return;
    }
    // Call legacy wakeup, Check the screen state
    if (stateAction_ != nullptr) {
        stateAction_->Wakeup(callTimeMs, type, details, pkgName);
    }
    mDeviceState_.lastWakeupDeviceTime = callTimeMs;

    ResetInactiveTimer();
    SetState(PowerState::AWAKE, GetReasonByWakeType(type), true);

    POWER_HILOGD(MODULE_SERVICE, "Wakeup Device Call");
}

void PowerStateMachine::RefreshActivityInner(pid_t pid,
    int64_t callTimeMs,
    UserActivityType type,
    bool needChangeBacklight)
{
    POWER_HILOGI(MODULE_SERVICE, "PID: %{public}d Start to RefreshActivity!!", pid);
    if (type > UserActivityType::USER_ACTIVITY_TYPE_MAX) {
        POWER_HILOGE(MODULE_SERVICE, "Invalid type: %{public}d", type);
        return;
    }
    // The minimum refreshactivity interval is 100ms!!
    int64_t now = GetTickCount();
    if ((mDeviceState_.lastRefreshActivityTime + MIN_TIME_MS_BETWEEN_USERACTIVITIES) > now) {
        POWER_HILOGW(MODULE_SERVICE, "RefreshActivity Failed, refresh too fast!");
        return;
    }
    mDeviceState_.lastRefreshActivityTime = now;
    // Check the screen state
    if (IsScreenOn()) {
        if (stateAction_ != nullptr) {
            stateAction_->RefreshActivity(callTimeMs, type, needChangeBacklight ?
                REFRESH_ACTIVITY_NEED_CHANGE_LIGHTS : REFRESH_ACTIVITY_NO_CHANGE_LIGHTS);
            mDeviceState_.screenState.lastOnTime = GetTickCount();
            stateAction_->SetDisplayState(DisplayState::DISPLAY_ON, GetReasonByUserActivity(type));
        }
        // reset timer
        ResetInactiveTimer();
        POWER_HILOGD(MODULE_SERVICE, "Refresh Activity Call Binder Success!!");
    } else {
        POWER_HILOGE(MODULE_SERVICE, "RefreshActivity Failed, Screen is Off!");
    }
    POWER_HILOGI(MODULE_SERVICE, "RefreshActivityInner: fun is End!");
}

bool PowerStateMachine::ForceSuspendDeviceInner(pid_t pid, int64_t callTimeMs)
{
    POWER_HILOGI(MODULE_SERVICE, "ForceSuspendDeviceInner: fun is Start!");
    if (stateAction_ != nullptr) {
        currentState_ = PowerState::SLEEP;
        stateAction_->GoToSleep(onSuspend, onWakeup, true);
    }

    POWER_HILOGI(MODULE_SERVICE, "ForceSuspendDeviceInner: fun is End!");
    return true;
}

bool PowerStateMachine::IsScreenOn()
{
    DisplayState state = stateAction_->GetDisplayState();
    if (state == DisplayState::DISPLAY_ON ||
        state == DisplayState::DISPLAY_DIM) {
        POWER_HILOGI(MODULE_SERVICE, "Current Screen State: On!");
        return true;
    }
    POWER_HILOGI(MODULE_SERVICE, "Current Screen State: Off!");
    return false;
}

void PowerStateMachine::ReceiveScreenEvent(bool isScreenOn)
{
    POWER_HILOGI(MODULE_SERVICE, "ReceiveScreenEvent: fun is Start!");
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
    POWER_HILOGI(MODULE_SERVICE, "RegisterPowerStateCallback: fun is Start!");
    std::lock_guard lock(mutex_);
    RETURN_IF(callback == nullptr);
    auto object = callback->AsObject();
    RETURN_IF(object == nullptr);
    auto retIt = powerStateListeners_.insert(callback);
    if (retIt.second) {
        object->AddDeathRecipient(powerStateCBDeathRecipient_);
    }
    POWER_HILOGI(MODULE_SERVICE,
        "%{public}s, object = %{public}p, callback = %{public}p, listeners.size = %{public}d,"
        " insertOk = %{public}d",
        __func__,
        object.GetRefPtr(),
        callback.GetRefPtr(),
        static_cast<unsigned int>(powerStateListeners_.size()),
        retIt.second);
}

void PowerStateMachine::UnRegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback)
{
    POWER_HILOGI(MODULE_SERVICE, "UnRegisterPowerStateCallback: fun is Start!");
    std::lock_guard lock(mutex_);
    RETURN_IF(callback == nullptr);
    auto object = callback->AsObject();
    RETURN_IF(object == nullptr);
    size_t eraseNum = powerStateListeners_.erase(callback);
    if (eraseNum != 0) {
        object->RemoveDeathRecipient(powerStateCBDeathRecipient_);
    }
    POWER_HILOGI(MODULE_SERVICE,
        "%{public}s, object = %{public}p, callback = %{public}p, listeners.size = %{public}d,"
        " eraseNum = %zu",
        __func__,
        object.GetRefPtr(),
        callback.GetRefPtr(),
        static_cast<unsigned int>(powerStateListeners_.size()),
        eraseNum);
}

static const std::string GetReasonTypeString(StateChangeReason type)
{
    switch (type) {
        case StateChangeReason::STATE_CHANGE_REASON_INIT:
            return std::string("INIT");
        case StateChangeReason::STATE_CHANGE_REASON_TIMEOUT:
            return std::string("TIMEOUT");
        case StateChangeReason::STATE_CHANGE_REASON_RUNNING_LOCK:
            return std::string("RUNNING_LOCK");
        case StateChangeReason::STATE_CHANGE_REASON_BATTERY:
            return std::string("BATTERY");
        case StateChangeReason::STATE_CHANGE_REASON_THERMAL:
            return std::string("THERMAL");
        case StateChangeReason::STATE_CHANGE_REASON_WORK:
            return std::string("WORK");
        case StateChangeReason::STATE_CHANGE_REASON_SYSTEM:
            return std::string("SYSTEM");
        case StateChangeReason::STATE_CHANGE_REASON_APPLICATION:
            return std::string("APPLICATION");
        case StateChangeReason::STATE_CHANGE_REASON_SETTINGS:
            return std::string("SETTINGS");
        case StateChangeReason::STATE_CHANGE_REASON_HARD_KEY:
            return std::string("HARD_KEY");
        case StateChangeReason::STATE_CHANGE_REASON_TOUCH:
            return std::string("TOUCH");
        case StateChangeReason::STATE_CHANGE_REASON_CABLE:
            return std::string("CABLE");
        case StateChangeReason::STATE_CHANGE_REASON_SENSOR:
            return std::string("SENSOR");
        case StateChangeReason::STATE_CHANGE_REASON_LID:
            return std::string("LID");
        case StateChangeReason::STATE_CHANGE_REASON_CAMERA:
            return std::string("CAMERA");
        case StateChangeReason::STATE_CHANGE_REASON_ACCESSIBILITY:
            return std::string("ACCESS");
        case StateChangeReason::STATE_CHANGE_REASON_POWER_KEY:
            return std::string("POWER_KEY");
        case StateChangeReason::STATE_CHANGE_REASON_KEYBOARD:
            return std::string("KEYBOARD");
        case StateChangeReason::STATE_CHANGE_REASON_MOUSE:
            return std::string("MOUSE");
        case StateChangeReason::STATE_CHANGE_REASON_REMOTE:
            return std::string("REMOTE");
        case StateChangeReason::STATE_CHANGE_REASON_UNKNOWN:
            return std::string("UNKNOWN");
        default:
            break;
    }

    return std::string("UNKNOWN");
}

static const std::string GetPowerStateString(PowerState state)
{
    switch (state) {
        case PowerState::AWAKE:
            return std::string("AWAKE");
        case PowerState::INACTIVE:
            return std::string("INACTIVE");
        case PowerState::SLEEP:
            return std::string("SLEEP");
        case PowerState::UNKNOWN:
            return std::string("UNKNOWN");
        default:
            break;
    }

    return std::string("UNKNOWN");
}

static const std::string GetRunningLockTypeString(RunningLockType type)
{
    switch (type) {
        case RunningLockType::RUNNINGLOCK_SCREEN:
            return std::string("SCREEN");
        case RunningLockType::RUNNINGLOCK_BACKGROUND:
            return std::string("BACKGROUND");
        case RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL:
            return std::string("PROXIMITY_SCREEN_CONTROL");
        case RunningLockType::RUNNINGLOCK_BUTT:
            return std::string("BUTT");
        default:
            break;
    }

    return std::string("UNKNOWN");
}

void PowerStateMachine::EnableMock(IDeviceStateAction* mockAction)
{
    POWER_HILOGI(MODULE_SERVICE, "enableMock: fun is Start!");
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
    POWER_HILOGI(MODULE_SERVICE,
        "%{public}s state = %u, listeners.size = %{public}d",
        __func__,
        static_cast<uint32_t>(state),
        static_cast<unsigned int>(powerStateListeners_.size()));
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
    POWER_HILOGD(MODULE_SERVICE, "SendEventToPowerMgrNotify: fun is Start!");
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        POWER_HILOGE(MODULE_SERVICE, "SendEventToPowerMgrNotify pms is Null, Fail!!");
        return;
    }
    auto notify = pms->GetPowerMgrNotify();
    if (notify == nullptr) {
        POWER_HILOGE(MODULE_SERVICE, "SendEventToPowerMgrNotify notify is Null, Fail!!");
        return;
    }
    if (state == PowerState::AWAKE) {
        notify->PublishScreenOnEvents(callTime);
    } else if (state == PowerState::INACTIVE) {
        notify->PublishScreenOffEvents(callTime);
    } else {
        POWER_HILOGI(MODULE_SERVICE, "No need to publish event, state:%{public}d", state);
    }
    POWER_HILOGD(MODULE_SERVICE, "SendEventToPowerMgrNotify: fun is End!");
}

const std::string TASK_UNREG_POWER_STATE_CALLBACK = "PowerState_UnRegPowerStateCB";

void PowerStateMachine::PowerStateCallbackDeathRecipient::OnRemoteDied(
    const wptr<IRemoteObject>& remote)
{
    POWER_HILOGD(MODULE_SERVICE, "OnRemoteDied: fun is Start!");
    if (remote == nullptr || remote.promote() == nullptr) {
        return;
    }
    POWER_HILOGI(MODULE_SERVICE, "PowerStateMachine::%{public}s remote = %p", __func__,
        remote.promote().GetRefPtr());
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        return;
    }
    auto handler = pms->GetHandler();
    if (handler == nullptr) {
        return;
    }
    sptr<IPowerStateCallback> callback = iface_cast<IPowerStateCallback>(remote.promote());
    std::function<void()> unRegFunc =
        std::bind(&PowerMgrService::UnRegisterPowerStateCallback, pms, callback);
    handler->PostTask(unRegFunc, TASK_UNREG_POWER_STATE_CALLBACK);
    POWER_HILOGD(MODULE_SERVICE, "OnRemoteDied: fun is End!");
}

void PowerStateMachine::SetDelayTimer(int64_t delayTime, int32_t event)
{
    POWER_HILOGD(MODULE_SERVICE, "SetDelayTimer: fun is Start!");
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        return;
    }
    auto handler = pms->GetHandler();
    if (handler == nullptr) {
        POWER_HILOGE(MODULE_SERVICE, "SetDelayTimer handler is null");
        return;
    }
    handler->SendEvent(event, 0, delayTime);
    POWER_HILOGD(MODULE_SERVICE, "SetDelayTimer: fun is End!");
}

void PowerStateMachine::CancelDelayTimer(int32_t event)
{
    POWER_HILOGD(MODULE_SERVICE, "CancelDelayTimer (%{public}d): fun is Start!", event);
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        return;
    }
    auto handler = pms->GetHandler();
    if (handler == nullptr) {
        POWER_HILOGE(MODULE_SERVICE, "CancelDelayTimer handler is null");
        return;
    }
    handler->RemoveEvent(event);
    POWER_HILOGD(MODULE_SERVICE, "CancelDelayTimer: fun is End!");
}

void PowerStateMachine::ResetInactiveTimer()
{
    POWER_HILOGD(MODULE_SERVICE, "ResetInactiveTimer: fun is Start!");
    CancelDelayTimer(PowermsEventHandler::CHECK_USER_ACTIVITY_TIMEOUT_MSG);
    CancelDelayTimer(PowermsEventHandler::CHECK_USER_ACTIVITY_OFF_TIMEOUT_MSG);
    CancelDelayTimer(PowermsEventHandler::CHECK_USER_ACTIVITY_SLEEP_TIMEOUT_MSG);
    if (this->GetDisplayOffTime() < 0) {
        POWER_HILOGI(MODULE_SERVICE, "Display Auto OFF is disabled");
        return;
    }

    if (this->CheckRunningLock(PowerState::INACTIVE)) {
        const uint32_t TWO = 2;
        const uint32_t THREE = 3;
        this->SetDelayTimer(this->GetDisplayOffTime() * TWO / THREE,
            PowermsEventHandler::CHECK_USER_ACTIVITY_TIMEOUT_MSG);
    }
    POWER_HILOGD(MODULE_SERVICE, "ResetInactiveTimer: fun is End!");
}

void PowerStateMachine::ResetSleepTimer()
{
    POWER_HILOGD(MODULE_SERVICE, "ResetSleepTimer: fun is Start!");
    CancelDelayTimer(PowermsEventHandler::CHECK_USER_ACTIVITY_TIMEOUT_MSG);
    CancelDelayTimer(PowermsEventHandler::CHECK_USER_ACTIVITY_OFF_TIMEOUT_MSG);
    CancelDelayTimer(PowermsEventHandler::CHECK_USER_ACTIVITY_SLEEP_TIMEOUT_MSG);
    if (this->GetSleepTime() < 0) {
        POWER_HILOGI(MODULE_SERVICE, "Auto Sleep is disabled");
        return;
    }

    if (this->CheckRunningLock(PowerState::SLEEP)) {
        this->SetDelayTimer(this->GetSleepTime(),
            PowermsEventHandler::CHECK_USER_ACTIVITY_SLEEP_TIMEOUT_MSG);
    }
    POWER_HILOGD(MODULE_SERVICE, "ResetSleepTimer: fun is End!");
}

void PowerStateMachine::HandleDelayTimer(int32_t event)
{
    POWER_HILOGD(MODULE_SERVICE, "handle delay timer: (%{public}d)", event);
    switch (event) {
        case PowermsEventHandler::CHECK_USER_ACTIVITY_TIMEOUT_MSG:
            HandleActivityTimeout();
            break;
        case PowermsEventHandler::CHECK_USER_ACTIVITY_OFF_TIMEOUT_MSG:
            HandleActivityOffTimeout();
            break;
        case PowermsEventHandler::CHECK_USER_ACTIVITY_SLEEP_TIMEOUT_MSG:
            HandleActivitySleepTimeout();
            break;
        case PowermsEventHandler::SYSTEM_WAKE_UP_MSG:
            HandleSystemWakeup();
            break;
        default:
            break;
    }
    POWER_HILOGD(MODULE_SERVICE, "ResetSleepTimer: fun is End!");
}

void PowerStateMachine::HandleActivityTimeout()
{
    POWER_HILOGI(MODULE_SERVICE, "HandleActivityTimeout (%{public}d)",
        stateAction_->GetDisplayState());
    DisplayState dispState = stateAction_->GetDisplayState();
    const uint32_t THREE = 3;
    if (!this->CheckRunningLock(PowerState::INACTIVE)) {
        POWER_HILOGW(MODULE_SERVICE, "RunningLock is blocking to transit to INACTIVE");
        return;
    }
    if (dispState == DisplayState::DISPLAY_ON) {
        stateAction_->SetDisplayState(DisplayState::DISPLAY_DIM,
            StateChangeReason::STATE_CHANGE_REASON_TIMEOUT);
        if (this->GetDisplayOffTime() < 0) {
            POWER_HILOGI(MODULE_SERVICE, "Display Auto OFF is disabled");
            return;
        } else {
            SetDelayTimer(GetDisplayOffTime() / THREE,
                PowermsEventHandler::CHECK_USER_ACTIVITY_OFF_TIMEOUT_MSG);
        }
    } else {
        POWER_HILOGW(MODULE_SERVICE,
            "HandleActivityTimeout when display: %{public}d", dispState);
    }
    POWER_HILOGI(MODULE_SERVICE, "HandleActivityTimeout: fun is End!");
}

void PowerStateMachine::HandleActivityOffTimeout()
{
    POWER_HILOGI(MODULE_SERVICE, "HandleActivityOffTimeout (%{public}d)",
        stateAction_->GetDisplayState());
    if (!this->CheckRunningLock(PowerState::INACTIVE)) {
        POWER_HILOGW(MODULE_SERVICE, "RunningLock is blocking to transit to INACTIVE");
        return;
    }
    DisplayState dispState = stateAction_->GetDisplayState();
    // Also transit state when ON if system not support DIM
    if (dispState == DisplayState::DISPLAY_ON
        || dispState == DisplayState::DISPLAY_DIM) {
        SetState(PowerState::INACTIVE, StateChangeReason::STATE_CHANGE_REASON_TIMEOUT);
    } else {
        POWER_HILOGW(MODULE_SERVICE,
            "HandleActivityOffTimeout when display: %{public}d", dispState);
    }
    POWER_HILOGI(MODULE_SERVICE, "HandleActivityOffTimeOut: fun is End!");
}

void PowerStateMachine::HandleActivitySleepTimeout()
{
    POWER_HILOGI(MODULE_SERVICE, "HandleActivitySleepTimeout (%{public}d)",
        stateAction_->GetDisplayState());
    if (!this->CheckRunningLock(PowerState::SLEEP)) {
        POWER_HILOGW(MODULE_SERVICE, "RunningLock is blocking to transit to SLEEP");
        return;
    }
    DisplayState dispState = stateAction_->GetDisplayState();
    if (dispState == DisplayState::DISPLAY_OFF) {
        SetState(PowerState::SLEEP, StateChangeReason::STATE_CHANGE_REASON_TIMEOUT);
    } else {
        POWER_HILOGW(MODULE_SERVICE,
            "HandleActivitySleepTimeout when display: %{public}d", dispState);
    }
    POWER_HILOGI(MODULE_SERVICE, "HandleActivitySleepTimeout: fun is End!");
}

void PowerStateMachine::HandleSystemWakeup()
{
    POWER_HILOGI(MODULE_SERVICE, "HandleSystemWakeup (%{public}d)",
        stateAction_->GetDisplayState());
    if (IsScreenOn()) {
        SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_SYSTEM, true);
    } else {
        SetState(PowerState::INACTIVE, StateChangeReason::STATE_CHANGE_REASON_SYSTEM, true);
    }
}

bool PowerStateMachine::CheckRunningLock(PowerState state)
{
    POWER_HILOGI(MODULE_SERVICE, "CheckRunningLock: fun is Start!");
    auto pms = pms_.promote();
    if (pms == nullptr) {
        POWER_HILOGE(MODULE_SERVICE, "CheckRunningLock: promote failed!");
        return false;
    }
    auto runningLockMgr = pms->GetRunningLockMgr();
    if (runningLockMgr == nullptr) {
        POWER_HILOGE(MODULE_SERVICE, "CheckRunningLock: GetRunningLockMgr failed!");
        return false;
    }
    auto iterator = lockMap_.find(state);
    if (iterator == lockMap_.end()) {
        POWER_HILOGE(MODULE_SERVICE, "CheckRunningLock: map find failed!");
        return false;
    }

    std::shared_ptr<std::vector<RunningLockType>> pLock = iterator->second;
    for (std::vector<RunningLockType>::const_iterator iter = pLock->begin();
        iter != pLock->end(); ++iter) {
        uint32_t count = runningLockMgr->GetValidRunningLockNum(*iter);
        if (count > 0) {
            POWER_HILOGE(MODULE_SERVICE,
                "RunningLock %{public}s is locking (count=%{public}d), blocking %{public}s",
                GetRunningLockTypeString(*iter).c_str(),
                count,
                GetPowerStateString(state).c_str());
            return false;
        }
    }

    POWER_HILOGI(MODULE_SERVICE, "No RunningLock block for state (%{public}d)", state);
    return true;
}

void PowerStateMachine::SetDisplayOffTime(int64_t time)
{
    displayOffTime_ = time;
    if (currentState_ == PowerState::AWAKE) {
        ResetInactiveTimer();
    }
}

void PowerStateMachine::SetSleepTime(int64_t time)
{
    sleepTime_ = time;
    if (currentState_ == PowerState::INACTIVE) {
        ResetSleepTimer();
    }
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
    POWER_HILOGI(MODULE_SERVICE,
        "SetState state=%{public}d, reason=%{public}d, force=%{public}d",
        state, reason, force);
    auto iterator = controllerMap_.find(state);
    if (iterator == controllerMap_.end()) {
        return false;
    }
    std::shared_ptr<StateController> pController = iterator->second;
    if (pController == nullptr) {
        POWER_HILOGE(MODULE_SERVICE, "AWAKE State not initiated");
        return false;
    }
    TransitResult ret = pController->TransitTo(reason, true);
    POWER_HILOGI(MODULE_SERVICE, "SetState: fun is End!");
    return (ret == TransitResult::SUCCESS || ret == TransitResult::ALREADY_IN_STATE);
}

void PowerStateMachine::SetDisplaySuspend(bool enable)
{
    POWER_HILOGI(MODULE_SERVICE, "SetDisplaySuspend:%{public}d", enable);
    enableDisplaySuspend_ = enable;
    if (GetState() == PowerState::INACTIVE) {
        POWER_HILOGI(MODULE_SERVICE, "Change display state");
        if (enable) {
            stateAction_->SetDisplayState(DisplayState::DISPLAY_SUSPEND,
                StateChangeReason::STATE_CHANGE_REASON_APPLICATION);
        } else {
            stateAction_->SetDisplayState(DisplayState::DISPLAY_OFF,
                StateChangeReason::STATE_CHANGE_REASON_APPLICATION);
        }
    }
}

StateChangeReason PowerStateMachine::GetReasonByUserActivity(UserActivityType type)
{
    POWER_HILOGI(MODULE_SERVICE, "GetReasonByUserActivity Start:%{public}d", type);
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
        case UserActivityType::USER_ACTIVITY_TYPE_OTHER: // fail through
        default:
            break;
    }
    POWER_HILOGI(MODULE_SERVICE, "GetReasonByUserActivity: fun is End!");
    return ret;
}

StateChangeReason PowerStateMachine::GetReasonByWakeType(WakeupDeviceType type)
{
    POWER_HILOGI(MODULE_SERVICE, "GetReasonByWakeType Start:%{public}d", type);
    StateChangeReason ret = StateChangeReason::STATE_CHANGE_REASON_UNKNOWN;
    switch (type) {
        case WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON:
            ret =  StateChangeReason::STATE_CHANGE_REASON_POWER_KEY;
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
            ret =  StateChangeReason::STATE_CHANGE_REASON_DOUBLE_CLICK;
            break;
        case WakeupDeviceType::WAKEUP_DEVICE_KEYBOARD:
            ret =  StateChangeReason::STATE_CHANGE_REASON_KEYBOARD;
            break;
        case WakeupDeviceType::WAKEUP_DEVICE_MOUSE:
            ret =  StateChangeReason::STATE_CHANGE_REASON_MOUSE;
            break;
        case WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN: // fail through
        default:
            break;
    }
    POWER_HILOGI(MODULE_SERVICE, "GetReasonByWakeType: fun is End!");
    return ret;
}

StateChangeReason PowerStateMachine::GetReasionBySuspendType(SuspendDeviceType type)
{
    POWER_HILOGI(MODULE_SERVICE, "GetReasionBySuspendType Start:%{public}d", type);
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
        case SuspendDeviceType::SUSPEND_DEVICE_REASON_LID_SWITCH:
            ret = StateChangeReason::STATE_CHANGE_REASON_LID;
            break;
        case SuspendDeviceType::SUSPEND_DEVICE_REASON_POWER_BUTTON: // fall through
        case SuspendDeviceType::SUSPEND_DEVICE_REASON_SLEEP_BUTTON:
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
    POWER_HILOGI(MODULE_SERVICE, "GetReasionBySuspendType: fun is End!");
    return ret;
}

void PowerStateMachine::DumpInfo(std::string& result)
{
    result.append("POWER MANAGER DUMP (hidumper -PowerStateMachine):\n");
    result.append("Current State: ")
            .append(GetPowerStateString(GetState()))
            .append("  Reasion: ")
            .append(ToString(static_cast<uint32_t>(
                controllerMap_.find(GetState())->second->lastReason_)))
            .append("  Time: ")
            .append(ToString(controllerMap_.find(GetState())->second->lastTime_))
            .append("\n");

    result.append("DUMP DETAILS:\n");
    result.append("Last Screen On: ")
            .append(ToString(mDeviceState_.screenState.lastOnTime))
            .append("\n");
    result.append("Last Screen Off: ")
            .append(ToString(mDeviceState_.screenState.lastOffTime))
            .append("\n");
    result.append("Last SuspendDevice: ")
            .append(ToString(mDeviceState_.lastSuspendDeviceTime))
            .append("\n");
    result.append("Last WakeupDevice: ")
            .append(ToString(mDeviceState_.lastWakeupDeviceTime))
            .append("\n");
    result.append("Last Refresh: ")
            .append(ToString(mDeviceState_.lastRefreshActivityTime))
            .append("\n");

    result.append("DUMP EACH STATES:\n");
    for (auto it = controllerMap_.begin(); it != controllerMap_.end(); it++) {
        result.append("State: ")
            .append(GetPowerStateString(it->second->GetState()))
            .append("   Reason:")
            .append(GetReasonTypeString(it->second->lastReason_).c_str())
            .append("   Time:")
            .append(ToString(it->second->lastTime_))
            .append("\n")
            .append("   Failure: ")
            .append(GetReasonTypeString(it->second->failTrigger_).c_str())
            .append("   Reason:")
            .append(it->second->failReasion_)
            .append("   From:")
            .append(GetPowerStateString(it->second->failFrom_))
            .append("   Time:")
            .append(ToString(it->second->failTime_))
            .append("\n\n");
    }
}

TransitResult PowerStateMachine::StateController::TransitTo(
    StateChangeReason reason,
    bool ignoreLock)
{
    POWER_HILOGI(MODULE_SERVICE, "TransitTo start");
    std::shared_ptr<PowerStateMachine> owner = owner_.lock();
    if (owner == nullptr) {
        POWER_HILOGE(MODULE_SERVICE, "TransitTo: no owner");
        return TransitResult::OTHER_ERR;
    }
    POWER_HILOGI(MODULE_SERVICE,
        "Transit from %{public}s to %{public}s for %{public}s ignoreLock=%{public}d",
        GetPowerStateString(owner->currentState_).c_str(),
        GetPowerStateString(this->state_).c_str(),
        GetReasonTypeString(reason).c_str(),
        ignoreLock);
    TransitResult ret = TransitResult::OTHER_ERR;
    if (!CheckState()) {
        POWER_HILOGE(MODULE_SERVICE, "TransitTo: already in %{public}d",
            owner->currentState_);
        RecordFailure(owner->currentState_, reason, TransitResult::ALREADY_IN_STATE);
        return TransitResult::ALREADY_IN_STATE;
    }
    if (!ignoreLock && !owner->CheckRunningLock(GetState())) {
        POWER_HILOGE(MODULE_SERVICE, "TransitTo: running lock block");
        RecordFailure(owner->currentState_, reason, TransitResult::LOCKING);
        return TransitResult::LOCKING;
    }
    ret = action_(reason);
    if (ret == TransitResult::SUCCESS) {
        lastReason_ = reason;
        lastTime_ = GetTickCount();
        owner->currentState_ = GetState();
        owner->NotifyPowerStateChanged(owner->currentState_);
    } else {
        RecordFailure(owner->currentState_, reason, ret);
    }

    POWER_HILOGI(MODULE_SERVICE, "Transit End, result=%{public}d", ret);
    return ret;
}

bool PowerStateMachine::StateController::CheckState()
{
    POWER_HILOGI(MODULE_SERVICE, "CheckState: fun is Start!");
    std::shared_ptr<PowerStateMachine> owner = owner_.lock();
    if (owner == nullptr) {
        return false;
    }
    POWER_HILOGI(MODULE_SERVICE, "CheckState: fun is End!");
    return !(GetState() == owner->currentState_);
}

void PowerStateMachine::StateController::RecordFailure(PowerState from,
    StateChangeReason trigger, TransitResult failReason)
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
    message.append(GetPowerStateString(failFrom_))
        .append(" to ")
        .append(GetPowerStateString(GetState()))
        .append(" by ")
        .append(GetReasonTypeString(failTrigger_).c_str())
        .append("   Reason:")
        .append(failReasion_)
        .append("   Time:")
        .append(ToString(failTime_))
        .append("\n");
    const int logLevel = 2;
    const std::string tag = "TAG_POWER";
    HiviewDFX::HiSysEvent::Write(HiviewDFX::HiSysEvent::Domain::POWERMGR, "POWER_SCREEN",
        HiviewDFX::HiSysEvent::EventType::FAULT,
        "LOG_LEVEL",
        logLevel,
        "TAG",
        tag,
        "MESSAGE",
        message);
    POWER_HILOGI(MODULE_SERVICE, "RecordFailure: %{public}s", message.c_str());
}
} // namespace PowerMgr
} // namespace OHOS
