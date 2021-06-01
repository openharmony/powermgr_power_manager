/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef POWERMGR_POWER_MGR_CLIENT_H
#define POWERMGR_POWER_MGR_CLIENT_H

#include <string>

#include <singleton.h>

#include "power_state_machine_info.h"
#include "running_lock.h"

namespace OHOS {
namespace PowerMgr {
class PowerMgrClient final : public DelayedRefSingleton<PowerMgrClient> {
    DECLARE_DELAYED_REF_SINGLETON(PowerMgrClient)

public:
    DISALLOW_COPY_AND_MOVE(PowerMgrClient);
    /**
     * Reboot the device.
     *
     * @param reason The reason for rebooting the device.
     */
    void RebootDevice(const std::string& reason);

    /**
     * Shut down the device.
     *
     * @param reason The reason for shutting down the device. e.g.recovery.
     *
     */
    void ShutDownDevice(const std::string& reason);

    /**
     * Suspend device and set screen off.
     *
     * @param reason The reason why will you suspend the device, such as timeout/powerkey/forcesuspend and so on.
     * @param suspendImmed The flag indicating whether the system will go to sleep immediately.
     */
    void SuspendDevice(SuspendDeviceType reason = SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION,
                       bool suspendImmed = true);

    /**
     * Wake up the device and set the screen on.
     *
     * @param reason The reason for waking up the device, such as powerkey/plugin/application.
     * @param details Details of the wakeup reason.
     */
    void WakeupDevice(WakeupDeviceType reason = WakeupDeviceType::WAKEUP_DEVICE_APPLICATION,
                      const std::string& details = "SoftWare Wakeup");

    /**
     * Refresh the screentimeout time, and keep the screen on. RefreshActivity works only when the screen is on.
     *
     * @param type The RefreshActivity type, such as touch/button/accessibility and so on.
     * @param needChangeBacklight Whether to change the backlight state, for example, from DIM to BRIGHT.
     *                            Set it to false if you don't want to change the backlight state.
     */
    void RefreshActivity(UserActivityType type = UserActivityType::USER_ACTIVITY_TYPE_OTHER,
                         bool needChangeBacklight = true);

    /**
     * Check whether the device screen is on. The result may be true or false, depending on the system state.
     */
    bool IsScreenOn();

    /**
     * Forcibly suspend the device into deepsleep, and return the suspend result.
     */
    bool ForceSuspendDevice();

    std::shared_ptr<RunningLock> CreateRunningLock(const std::string& name, RunningLockType type,
                                                   const int screenOnFlag = 0);
    void ProxyRunningLock(bool proxyLock, pid_t uid, pid_t pid = INVALID_PID);
    void RegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback);
    void UnRegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback);
    void RegisterShutdownCallback(const sptr<IShutdownCallback>& callback);
    void UnRegisterShutdownCallback(const sptr<IShutdownCallback>& callback);

private:
    class PowerMgrDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        PowerMgrDeathRecipient() = default;
        ~PowerMgrDeathRecipient() = default;
        void OnRemoteDied(const wptr<IRemoteObject>& remote);
    private:
        DISALLOW_COPY_AND_MOVE(PowerMgrDeathRecipient);
    };

    ErrCode Connect();
    sptr<IPowerMgr> proxy_ {nullptr};
    sptr<IRemoteObject::DeathRecipient> deathRecipient_ {nullptr};
    void ResetProxy(const wptr<IRemoteObject>& remote);
    std::mutex mutex_;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_POWER_MGR_CLIENT_H
