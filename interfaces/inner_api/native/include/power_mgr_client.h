/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
class PowerMgrClient final {
private:
    PowerMgrClient();
    DISALLOW_COPY_AND_MOVE(PowerMgrClient);

public:
    static PowerMgrClient& GetInstance();
    virtual ~PowerMgrClient();
    static const uint32_t CONNECT_RETRY_COUNT = 5;
    static const uint32_t CONNECT_RETRY_MS = 800000;
    /**
     * Reboot the device.
     *
     * @param reason The reason for rebooting the device. e.g.updater
     */
    PowerErrors RebootDevice(const std::string& reason);
    PowerErrors RebootDeviceForDeprecated(const std::string& reason);

    /**
     * Shut down the device.
     *
     * @param reason The reason for shutting down the device.
     *
     */
    PowerErrors ShutDownDevice(const std::string& reason);

    /**
     * Suspend device and set screen off.
     *
     * @param reason The reason why will you suspend the device, such as timeout/powerkey/forcesuspend and so on.
     */
    PowerErrors SuspendDevice(SuspendDeviceType reason = SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION,
        bool suspendImmed = false);

    /**
     * Wake up the device and set the screen on.
     *
     * @param reason The reason for waking up the device, such as powerkey/plugin/application.
     */
    PowerErrors WakeupDevice(WakeupDeviceType reason = WakeupDeviceType::WAKEUP_DEVICE_APPLICATION,
        const std::string& detail = std::string("app call"));

    /**
     * Refresh the screentimeout time, and keep the screen on. RefreshActivity works only when the screen is on.
     *
     * @param type The RefreshActivity type, such as touch/button/accessibility and so on.
     * @param needChangeBacklight Whether to change the backlight state, for example, from DIM to BRIGHT.
     *                            Set it to false if you don't want to change the backlight state.
     */
    bool RefreshActivity(UserActivityType type = UserActivityType::USER_ACTIVITY_TYPE_OTHER);

    /**
     * Windows overwrite timeout
     * @param timeout Specifies the timeout duration.
     */
    bool OverrideScreenOffTime(int64_t timeout);

    /**
     * Windows restores timeout
     */
    bool RestoreScreenOffTime();

    /**
     * Check whether the device screen is on. The result may be true or false, depending on the system state.
     */
    bool IsScreenOn();

    /**
     * Get Power state. The result is PowerState type.
     */
    PowerState GetState();

    /**
     * Forcibly suspend the device into deepsleep, and return the suspend result.
     */
    bool ForceSuspendDevice();

    /**
     * Check whether the type of running lock is supported
     */
    bool IsRunningLockTypeSupported(RunningLockType type);

    /**
     * Enable/disable display suspend state
     */
    bool SetDisplaySuspend(bool enable);

    /**
     * Hibernate the device.
     * @param clearMemory Indicates whether to clear the memory before the device hibernates.
     */
    bool Hibernate(bool clearMemory);

    /* Set the device mode.
     *
     * @param set The mode the device.
     */
    PowerErrors SetDeviceMode(const PowerMode mode);

    /**
     * Get the device mode.
     *
     * @param Get The mode the device.
     */
    PowerMode GetDeviceMode();

    /**
     * Check if the device has entered standby mode.
     */
    PowerErrors IsStandby(bool& isStandby);

    /**
     * Query the list of lock information.
     */
    bool QueryRunningLockLists(std::map<std::string, RunningLockInfo>& runningLockLists);

    PowerErrors SetForceTimingOut(bool enabled);
    PowerErrors LockScreenAfterTimingOut(bool enabledLockScreen, bool checkLock);

    std::shared_ptr<RunningLock> CreateRunningLock(const std::string& name, RunningLockType type);
    bool ProxyRunningLock(bool isProxied, pid_t pid, pid_t uid);
    bool ProxyRunningLocks(bool isProxied, const std::vector<std::pair<pid_t, pid_t>>& processInfos);
    bool ResetRunningLocks();
    bool RegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback);
    bool UnRegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback);
    bool RegisterSyncSleepCallback(const sptr<ISyncSleepCallback>& callback, SleepPriority priority);
    bool UnRegisterSyncSleepCallback(const sptr<ISyncSleepCallback>& callback);
    bool RegisterPowerModeCallback(const sptr<IPowerModeCallback>& callback);
    bool UnRegisterPowerModeCallback(const sptr<IPowerModeCallback>& callback);
    bool RegisterScreenStateCallback(int32_t remainTime, const sptr<IScreenOffPreCallback>& callback);
    bool UnRegisterScreenStateCallback(const sptr<IScreenOffPreCallback>& callback);
    bool RegisterRunningLockCallback(const sptr<IPowerRunninglockCallback>& callback);
    bool UnRegisterRunningLockCallback(const sptr<IPowerRunninglockCallback>& callback);
    void RecoverRunningLocks();
    std::string Dump(const std::vector<std::string>& args);
    PowerErrors GetError();

#ifndef POWERMGR_SERVICE_DEATH_UT
private:
#endif
    class PowerMgrDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        explicit PowerMgrDeathRecipient(PowerMgrClient& client) : client_(client) {}
        ~PowerMgrDeathRecipient() = default;
        void OnRemoteDied(const wptr<IRemoteObject>& remote);

    private:
        DISALLOW_COPY_AND_MOVE(PowerMgrDeathRecipient);
        PowerMgrClient& client_;
    };

    ErrCode Connect();
    void ResetProxy(const wptr<IRemoteObject>& remote);
    sptr<IPowerMgr> proxy_ {nullptr};
    sptr<IRemoteObject::DeathRecipient> deathRecipient_ {nullptr};
    std::mutex mutex_;
    static std::vector<std::weak_ptr<RunningLock>> runningLocks_;
    static std::mutex runningLocksMutex_;
    PowerErrors error_ = PowerErrors::ERR_OK;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_POWER_MGR_CLIENT_H
