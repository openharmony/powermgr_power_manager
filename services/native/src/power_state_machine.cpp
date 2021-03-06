/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
    std::vector<RunningLockType> inactiveBlocker {RunningLockType::RUNNINGLOCK_SCREEN,
        RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL};
    std::vector<RunningLockType> sleepBlocker {RunningLockType::RUNNINGLOCK_BACKGROUND};
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

void PowerStateMachine::InitStateMap()
{
    // init state controller map
    controllerMap_.emplace(PowerState::AWAKE,
        std::make_shared<StateController>(PowerState::AWAKE, shared_from_this(), [this] {
            mDeviceState_.screenState.lastOnTime = GetTickCount();
            uint32_t ret = this->stateAction_->SetDisplayState(DisplayState::DISPLAY_ON);
            if (ret != ActionResult::SUCCESS) {
                POWER_HILOGE(MODULE_SERVICE, "Failed to go to AWAKE, Display Err");
                return TransitResult::HDI_ERR;
            }
            ResetInactiveTimer();
            return TransitResult::SUCCESS;
        })
    );
    controllerMap_.emplace(PowerState::INACTIVE,
        std::make_shared<StateController>(PowerState::INACTIVE, shared_from_this(), [this] {
            POWER_HILOGI(MODULE_SERVICE, "StateController_INACTIVE: func is Start.");
            mDeviceState_.screenState.lastOffTime = GetTickCount();
            DisplayState state = DisplayState::DISPLAY_OFF;
            if (enableDisplaySuspend_) {
                POWER_HILOGI(MODULE_SERVICE, "display suspend enabled");
                state = DisplayState::DISPLAY_SUSPEND;
            }
            uint32_t ret = this->stateAction_->SetDisplayState(state);
            if (ret != ActionResult::SUCCESS) {
                POWER_HILOGE(MODULE_SERVICE, "Failed to go to INACTIVE, Display Err");
                return TransitResult::HDI_ERR;
            }
            ResetSleepTimer();
            return TransitResult::SUCCESS;
        })
    );
    controllerMap_.emplace(PowerState::SLEEP,
        std::make_shared<StateController>(PowerState::SLEEP, shared_from_this(), [this] {
            DisplayState state = DisplayState::DISPLAY_OFF;
            if (enableDisplaySuspend_) {
                POWER_HILOGI(MODULE_SERVICE, "display suspend enabled");
                state = DisplayState::DISPLAY_SUSPEND;
            }
            uint32_t ret = this->stateAction_->SetDisplayState(state);
            if (ret != ActionResult::SUCCESS) {
                POWER_HILOGE(MODULE_SERVICE, "Failed to go to SLEEP, Display Err");
                return TransitResult::HDI_ERR;
            }
            ret = this->stateAction_->GoToSleep(onSuspend, onWakeup, false);
            if (ret != ActionResult::SUCCESS) {
                POWER_HILOGE(MODULE_SERVICE, "Failed to go to SLEEP, Sleep Err");
                return TransitResult::HDI_ERR;
            }
            return TransitResult::SUCCESS;
        })
    );
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
        POWER_HILOGE(MODULE_SERVICE, "RefreshActivity Failed, refresh too fast!");
        return;
    }
    mDeviceState_.lastRefreshActivityTime = now;
    // Check the screen state
    if (IsScreenOn()) {
        if (stateAction_ != nullptr) {
            stateAction_->RefreshActivity(callTimeMs, type, needChangeBacklight ?
                REFRESH_ACTIVITY_NEED_CHANGE_LIGHTS : REFRESH_ACTIVITY_NO_CHANGE_LIGHTS);
            mDeviceState_.screenState.lastOnTime = GetTickCount();
            stateAction_->SetDisplayState(DisplayState::DISPLAY_ON);
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
    POWER_HILOGI(MODULE_SERVICE, "IsScreenOn: fun is Start!");
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

static const char* GetReasonTypeString(StateChangeReason type)
{
    switch (type) {
        case StateChangeReason::STATE_CHANGE_REASON_INIT:
            return "INIT";
        case StateChangeReason::STATE_CHANGE_REASON_TIMEOUT:
            return "TIMEOUT";
        case StateChangeReason::STATE_CHANGE_REASON_RUNNING_LOCK:
            return "RUNNING_LOCK";
        case StateChangeReason::STATE_CHANGE_REASON_BATTERY:
            return "BATTERY";
        case StateChangeReason::STATE_CHANGE_REASON_THERMAL:
            return "THERMAL";
        case StateChangeReason::STATE_CHANGE_REASON_WORK:
            return "WORK";
        case StateChangeReason::STATE_CHANGE_REASON_SYSTEM:
            return "SYSTEM";
        case StateChangeReason::STATE_CHANGE_REASON_APPLICATION:
            return "APPLICATION";
        case StateChangeReason::STATE_CHANGE_REASON_SETTINGS:
            return "SETTINGS";
        case StateChangeReason::STATE_CHANGE_REASON_HARD_KEY:
            return "HARD_KEY";
        case StateChangeReason::STATE_CHANGE_REASON_TOUCH:
            return "TOUCH";
        case StateChangeReason::STATE_CHANGE_REASON_CABLE:
            return "CABLE";
        case StateChangeReason::STATE_CHANGE_REASON_SENSOR:
            return "SENSOR";
        case StateChangeReason::STATE_CHANGE_REASON_LID:
            return "LID";
        case StateChangeReason::STATE_CHANGE_REASON_CAMERA:
            return "CAMERA";
        case StateChangeReason::STATE_CHANGE_REASON_ACCESSIBILITY:
            return "ACCESSIBILITY";
        case StateChangeReason::STATE_CHANGE_REASON_REMOTE:
            return "REMOTE";
        case StateChangeReason::STATE_CHANGE_REASON_UNKNOWN:
            return "UNKNOWN";
        default:
            break;
    }

    return "UNKNOWN";
}

static const char* GetPowerStateString(PowerState state)
{
    switch (state) {
        case PowerState::AWAKE:
            return "AWAKE";
        case PowerState::INACTIVE:
            return "INACTIVE";
        case PowerState::SLEEP:
            return "SLEEP";
        case PowerState::UNKNOWN:
            return "UNKNOWN";
        default:
            break;
    }

    return "UNKNOWN";
}

static const char* GetRunningLockTypeString(RunningLockType type)
{
    switch (type) {
        case RunningLockType::RUNNINGLOCK_SCREEN:
            return "SCREEN";
        case RunningLockType::RUNNINGLOCK_BACKGROUND:
            return "BACKGROUND";
        case RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL:
            return "PROXIMITY_SCREEN_CONTROL";
        case RunningLockType::RUNNINGLOCK_BUTT:
            return "BUTT";
        default:
            break;
    }

    return "UNKNOWN";
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
        stateAction_->SetDisplayState(DisplayState::DISPLAY_DIM);
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
                GetRunningLockTypeString(*iter),
                count,
                GetPowerStateString(state));
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
            stateAction_->SetDisplayState(DisplayState::DISPLAY_SUSPEND);
        } else {
            stateAction_->SetDisplayState(DisplayState::DISPLAY_OFF);
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
        case WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON: // fall through
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
            .append(GetReasonTypeString(it->second->lastReason_))
            .append("   Time:")
            .append(ToString(it->second->lastTime_))
            .append("\n");
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
        GetPowerStateString(owner->currentState_),
        GetPowerStateString(this->state_),
        GetReasonTypeString(reason),
        ignoreLock);
    TransitResult ret = TransitResult::OTHER_ERR;
    if (!CheckState()) {
        POWER_HILOGE(MODULE_SERVICE, "TransitTo: already in %{public}d",
            owner->currentState_);
        return TransitResult::ALREADY_IN_STATE;
    }
    if (!ignoreLock && !owner->CheckRunningLock(GetState())) {
        POWER_HILOGE(MODULE_SERVICE, "TransitTo: running lock block");
        return TransitResult::LOCKING;
    }
    ret = action_();
    if (ret == TransitResult::SUCCESS) {
        lastReason_ = reason;
        lastTime_ = GetTickCount();
        owner->currentState_ = GetState();
        owner->NotifyPowerStateChanged(owner->currentState_);
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
} // namespace PowerMgr
} // namespace OHOS
