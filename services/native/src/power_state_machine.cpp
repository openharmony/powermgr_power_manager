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
PowerStateMachine::PowerStateMachine(const wptr<PowerMgrService>& pms) : pms_(pms)
{
    // NOTICE Need get screen state when device startup, rightnow we set screen is on as default
    mDeviceState_.screenState.state = ScreenStateType::SCREEN_ON;
    mDeviceState_.screenState.lastUpdateTime = GetTickCount();
    mDeviceState_.lastWakeupEventTime = 0;
    mDeviceState_.lastRefreshActivityTime = 0;
    mDeviceState_.lastWakeupDeviceTime = 0;
    mDeviceState_.lastSuspendDeviceTime = 0;
}

PowerStateMachine::~PowerStateMachine() {}

bool PowerStateMachine::Init()
{
    POWER_HILOGI(MODULE_SERVICE, "PowerStateMachine:: Init start");
    stateAction_ = PowerMgrFactory::GetDeviceStateAction();
    if (powerStateCBDeathRecipient_ == nullptr) {
        powerStateCBDeathRecipient_ = new PowerStateCallbackDeathRecipient();
    }

    if (!powerMgrMonitor_.Start()) {
        POWER_HILOGE(MODULE_SERVICE, "Failed to start monitor");
        return false;
    }
    POWER_HILOGI(MODULE_SERVICE, "PowerStateMachine:: Init success!");
    return true;
}

void PowerStateMachine::SuspendDeviceInner(pid_t pid, int64_t callTimeMs, SuspendDeviceType type, bool suspendImmed,
    bool ignoreScreenState)
{
    POWER_HILOGI(MODULE_SERVICE, "PID: %{public}d Try to Suspend Device!!", pid);
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
}

void PowerStateMachine::WakeupDeviceInner(pid_t pid, int64_t callTimeMs, WakeupDeviceType type,
    const std::string& details, const std::string& pkgName)
{
    POWER_HILOGI(MODULE_SERVICE, "PID: %{public}d Try to Wakeup Device!!", pid);
    // Check the screen state
    if (stateAction_ != nullptr) {
        stateAction_->Wakeup(callTimeMs, type, details, pkgName);
    }
    mDeviceState_.lastWakeupDeviceTime = callTimeMs;
    POWER_HILOGD(MODULE_SERVICE, "Wakeup Device Call Binder Success!!");
}

void PowerStateMachine::RefreshActivityInner(pid_t pid, int64_t callTimeMs, UserActivityType type,
    bool needChangeBacklight)
{
    POWER_HILOGI(MODULE_SERVICE, "PID: %{public}d Start to RefreshActivity!!", pid);
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
        }
        POWER_HILOGD(MODULE_SERVICE, "Refresh Activity Call Binder Success!!");
    } else {
        POWER_HILOGE(MODULE_SERVICE, "RefreshActivity Failed, Screen is Off!");
    }
}

bool PowerStateMachine::ForceSuspendDeviceInner(pid_t pid, int64_t callTimeMs)
{
    if (stateAction_ != nullptr) {
        stateAction_->ForceSuspend();
    }
    return true;
}

bool PowerStateMachine::IsScreenOn()
{
    std::lock_guard lock(mutex_);
    if (mDeviceState_.screenState.state == ScreenStateType::SCREEN_ON ||
        mDeviceState_.screenState.state == ScreenStateType::SCREEN_DIM) {
        POWER_HILOGI(MODULE_SERVICE, "Current Screen State: On!");
        return true;
    }
    POWER_HILOGI(MODULE_SERVICE, "Current Screen State: Off!");
    return false;
}

void PowerStateMachine::ReceiveScreenEvent(bool isScreenOn)
{
    std::lock_guard lock(mutex_);
    auto prestate = mDeviceState_.screenState.state;
    mDeviceState_.screenState.lastUpdateTime = GetTickCount();
    mDeviceState_.screenState.state = isScreenOn ? ScreenStateType::SCREEN_ON : ScreenStateType::SCREEN_OFF;
    if (prestate != mDeviceState_.screenState.state) {
        NotifyPowerStateChanged(isScreenOn ? PowerState::AWAKE : PowerState::INACTIVE);
    }
    POWER_HILOGI(MODULE_SERVICE, "receive new screen event, new state is %{public}d, at %{public}" PRId64 "",
        mDeviceState_.screenState.state, mDeviceState_.screenState.lastUpdateTime);
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
    POWER_HILOGI(MODULE_SERVICE, "%{public}s, object = %p, callback = %p, listeners.size = %d,"
        " insertOk = %d", __func__, object.GetRefPtr(), callback.GetRefPtr(),
        static_cast<unsigned int>(powerStateListeners_.size()),
        retIt.second);
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
    POWER_HILOGI(MODULE_SERVICE, "%{public}s, object = %p, callback = %p, listeners.size = %d,"
        " eraseNum = %zu", __func__, object.GetRefPtr(), callback.GetRefPtr(),
        static_cast<unsigned int>(powerStateListeners_.size()),
        eraseNum);
}

void PowerStateMachine::NotifyPowerStateChanged(PowerState state)
{
    POWER_HILOGI(MODULE_SERVICE, "%{public}s state = %u, listeners.size = %d", __func__,
        static_cast<uint32_t>(state), static_cast<unsigned int>(powerStateListeners_.size()));
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
    } else {
        notify->PublishScreenOffEvents(callTime);
    }
}

const std::string TASK_UNREG_POWER_STATE_CALLBACK = "PowerState_UnRegPowerStateCB";

void PowerStateMachine::PowerStateCallbackDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
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
    std::function<void()> unRegFunc = std::bind(&PowerMgrService::UnRegisterPowerStateCallback, pms, callback);
    handler->PostTask(unRegFunc, TASK_UNREG_POWER_STATE_CALLBACK);
}

void PowerStateMachine::SetDelayTimer(int64_t delayTime, int32_t event)
{
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
}

void PowerStateMachine::CancelDelayTimer(int32_t event)
{
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
    POWER_HILOGI(MODULE_SERVICE, "remove event success");
}

void PowerStateMachine::HandleDelayTimer()
{
    POWER_HILOGI(MODULE_SERVICE, "handle delay timer success");
}
} // namespace PowerMgr
} // namespace OHOS
