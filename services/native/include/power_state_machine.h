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

#ifndef POWERMGR_POWER_STATE_MACHINE_H
#define POWERMGR_POWER_STATE_MACHINE_H

#include <set>

#include <singleton.h>

#include "actions/idevice_state_action.h"
#include "ipower_state_callback.h"
#include "power_common.h"
#include "power_mgr_monitor.h"
#include "power_state_machine_info.h"

namespace OHOS {
namespace PowerMgr {
class RunningLockMgr;
class PowerMgrService;

enum class ScreenStateType {
    SCREEN_OFF = 0,
    SCREEN_ON = 1,
    SCREEN_DIM = 2,
};

struct ScreenState {
    ScreenStateType state;
    int64_t lastUpdateTime;
};

struct DevicePowerState {
    ScreenState screenState;
    // record the last time when get wakeup event from A side
    int64_t lastWakeupEventTime;
    // record the last time when calling func RefreshActivityInner
    int64_t lastRefreshActivityTime;
    // record the last time when calling func WakeupDeviceInner
    int64_t lastWakeupDeviceTime;
    // record the last time when calling func SuspendDeviceInner
    int64_t lastSuspendDeviceTime;
};

class PowerStateMachine {
public:
    explicit PowerStateMachine(const wptr<PowerMgrService>& pms);
    ~PowerStateMachine();

    bool Init();
    void SuspendDeviceInner(pid_t pid, int64_t callTimeMs, SuspendDeviceType type, bool suspendImmed,
        bool ignoreScreenState = false);
    void WakeupDeviceInner(pid_t pid, int64_t callTimeMs, WakeupDeviceType type, const std::string& details,
        const std::string& pkgName);
    void RefreshActivityInner(pid_t pid, int64_t callTimeMs, UserActivityType type, bool needChangeBacklight);
    void ReceiveScreenEvent(bool isScreenOn);
    bool IsScreenOn();
    bool ForceSuspendDeviceInner(pid_t pid, int64_t callTimeMs);
    void RegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback);
    void UnRegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback);
    void SetDelayTimer(int64_t delayTime, int32_t event);
    void CancelDelayTimer(int32_t event);
    void HandleDelayTimer();
    // only use for test
    int64_t GetLastSuspendDeviceTime() const
    {
        return mDeviceState_.lastSuspendDeviceTime;
    }
    int64_t GetLastWakeupDeviceTime() const
    {
        return mDeviceState_.lastWakeupDeviceTime;
    }
    int64_t GetLastRefreshActivityTime() const
    {
        return mDeviceState_.lastRefreshActivityTime;
    }
    int64_t GetLastWakeupEventTime() const
    {
        return mDeviceState_.lastWakeupEventTime;
    }
    class PowerStateCallbackDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        PowerStateCallbackDeathRecipient() = default;
        virtual void OnRemoteDied(const wptr<IRemoteObject>& remote);
        virtual ~PowerStateCallbackDeathRecipient() = default;
    };

private:
    struct classcomp {
        bool operator() (const sptr<IPowerStateCallback>& l, const sptr<IPowerStateCallback>& r) const
        {
            return l->AsObject() < r->AsObject();
        }
    };

    void NotifyPowerStateChanged(PowerState state);
    void SendEventToPowerMgrNotify(PowerState state, int64_t callTime);

    const wptr<PowerMgrService> pms_;
    PowerMgrMonitor powerMgrMonitor_;
    std::mutex mutex_;
    DevicePowerState mDeviceState_;
    sptr<IRemoteObject::DeathRecipient> powerStateCBDeathRecipient_;
    std::set<const sptr<IPowerStateCallback>, classcomp> powerStateListeners_;
    std::unique_ptr<IDeviceStateAction> stateAction_;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_POWER_STATE_MACHINE_H
