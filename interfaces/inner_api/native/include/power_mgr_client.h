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
#include "suspend/itake_over_suspend_callback.h"
#include "isync_sleep_callback.h"

namespace OHOS {
namespace PowerMgr {
class PowerMgrClient final {
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
     * @brief Forces the device to reboot.
     * Ignores registered takeOverShutdownCallback, set time limits for executing other callbacks.
     * Guarantees the invocation of function "DoRebootExt" provided by startup_init within 60 seconds.
     * A Timeout will trigger reboot in kernel if the caller has the permission to set reboot stage.
     *
     * @param reason The reason for rebooting the device. e.g.updater
     */
    PowerErrors ForceRebootDevice(const std::string& reason);

    /**
     * Shut down the device.
     *
     * @param reason The reason for shutting down the device.
     *
     */
    PowerErrors ShutDownDevice(const std::string& reason);

    /**
     * @brief Set suspend tag before suspend.
     * The special sleep mode supported by the kernel and hardware is triggered by setting a special
     * suspend tag and then triggering suspend. If the suspend tag is not set, the standard S3 sleep
     * mode is triggered when suspend.
     *
     * @param tag Suspend tag.
     */
    PowerErrors SetSuspendTag(const std::string& tag);

    /**
     * Suspend device and set screen off.
     *
     * @param reason The reason why will you suspend the device, such as timeout/powerkey/forcesuspend and so on.
     */
    PowerErrors SuspendDevice(SuspendDeviceType reason = SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION,
        bool suspendImmed = false, const std::string& apiVersion = "-1");

    /**
     * Wake up the device and set the screen on.
     *
     * @param reason The reason for waking up the device, such as powerkey/plugin/application.
     */
    PowerErrors WakeupDevice(WakeupDeviceType reason = WakeupDeviceType::WAKEUP_DEVICE_APPLICATION,
        const std::string& detail = std::string("app call"), const std::string& apiVersion = "-1");

    /**
     * Wake up the device and set the screen on async.
     *
     * @param reason The reason for waking up the device, such as powerkey/plugin/application.
     */
    void WakeupDeviceAsync(WakeupDeviceType reason = WakeupDeviceType::WAKEUP_DEVICE_APPLICATION,
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
     * Refresh the screentimeout time, and keep the screen on. RefreshActivity works only when the screen is on.
     *
     * @param type The RefreshActivity type, such as touch/button/accessibility and so on.
     * @param refreshReason The reason to refresh the screentimeout time.
     * @return PowerErrors::ERR_OK if the call success, otherwise return error code.
     */
    PowerErrors RefreshActivity(UserActivityType type, const std::string& refreshReason);

    /**
     * Windows overwrite timeout
     * @param timeout Specifies the timeout duration.
     */
    PowerErrors OverrideScreenOffTime(int64_t timeout, const std::string& apiVersion = "-1");

    /**
     * Windows restores timeout
     */
    PowerErrors RestoreScreenOffTime(const std::string& apiVersion = "-1");

    /**
     * Check whether the device screen is on. The result may be true or false, depending on the system state.
     */
    bool IsScreenOn(bool needPrintLog = true);

    /**
     * Check whether the fold device screen is on. The result may be true or false, depending on the system state.
     */
    bool IsFoldScreenOn();

    /**
     * Check whether the collaboration device screen is on.
     * The result may be true or false, depending on the system state.
     */
    bool IsCollaborationScreenOn();

    /**
     * Check whether the device is in force sleep. The result may be true or false, depending on the system state.
     */
    bool IsForceSleeping();

    /**
     * Get Power state. The result is PowerState type.
     */
    PowerState GetState();

    /**
     * Forcibly suspend the device into deepsleep, and return the suspend result.
     */
    PowerErrors ForceSuspendDevice(const std::string& apiVersion = "-1");

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
     * @param reasonn The reason for hibernate the device.
     */
    PowerErrors Hibernate(bool clearMemory, const std::string& reason = "", const std::string& apiVersion = "-1");

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

    /**
     * Force auto screen-off after the set timeout. That is, the RUNNINGLOCK_SCREEN (or KeepScreenOn window attribute) is
     * ignored.
     *
     * @param enabled Whether or not the feature is enabled: True to ignore RUNNINGLOCK_SCREEN. False to retrieve the
     * default behaviour.
     */
    PowerErrors SetForceTimingOut(bool enabled);

    /**
     * Controls the screen-lock behavior as well as enable or disable callbacks and common-events after an
     * auto-screen-off after the set timeout.
     * In case of multiple clients using this interface, the default value of a parameter will take effect if and only
     * if all of the clients have set that parameter to default. That is, if any of them set a non-default value, the
     * behaviour will be non-default
     *
     * @param enabledLockScreen determines whether or not to lock the screen after auto-screen-off.
     * True(Default): the screen CAN be locked after auto-screen-off;
     * False: the screen will not be locked after auto-screen-off.
     *
     * @param checkLock is only taken into account if enabledLockScreen is set to true(default).
     * False(Default): Always lock the screen after an auto-screen-off.
     * True: lock the screen after an auto-screen-off only if there are active RUNNINGLOCK_SCREEN(which would normally
     * prevent the auto-screen-off without pairing with SetForceTimingOut).
     *
     * @param sendScreenOffEvent determines whether or not to send callbacks and common events after an auto-screen-off
     * THAT DOES NOT LOCK THE SCREEN.
     * True(Default): statechange callbacks and common events are sent normally after a non-screen-locking
     * auto-screen-off.
     * False: if an auto-screen-off does not lock the screen, callbacks and common events are not sent either.
     */
    PowerErrors LockScreenAfterTimingOut(bool enabledLockScreen, bool checkLock, bool sendScreenOffEvent = true);

    /**
     * Similar to LockScreenAfterTimingOut. This interface should only be used by a bridge service providing TS
     * interfaces for apps which dont have direct access to native interfaces,
     *
     * @param appid indicates the user of this interface, normally a hap.
     * @param lockScreen determins whether or not to lock the screen after an auto-screen-off.
     * True: same as calling LockScreenAfterTimingOut(true, false, true).
     * False: same as calling LockScreenAfterTimingOut(false, false, false).
     */
    PowerErrors LockScreenAfterTimingOutWithAppid(pid_t appid, bool lockScreen);

    std::shared_ptr<RunningLock> CreateRunningLock(const std::string& name, RunningLockType type);
    bool ProxyRunningLock(bool isProxied, pid_t pid, pid_t uid);
    bool ProxyRunningLocks(bool isProxied, const std::vector<std::pair<pid_t, pid_t>>& processInfos);
    bool ResetRunningLocks();
    bool RegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback, bool isSync = true);
    bool UnRegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback);
    bool RegisterSyncHibernateCallback(const sptr<ISyncHibernateCallback>& callback);
    bool UnRegisterSyncHibernateCallback(const sptr<ISyncHibernateCallback>& callback);
    bool RegisterSyncSleepCallback(const sptr<ISyncSleepCallback>& callback, SleepPriority priority);
    bool UnRegisterSyncSleepCallback(const sptr<ISyncSleepCallback>& callback);
    bool RegisterSuspendTakeoverCallback(
        const sptr<ITakeOverSuspendCallback>& callback, TakeOverSuspendPriority priority);
    bool UnRegisterSuspendTakeoverCallback(const sptr<ITakeOverSuspendCallback>& callback);
    bool RegisterPowerModeCallback(const sptr<IPowerModeCallback>& callback);
    bool UnRegisterPowerModeCallback(const sptr<IPowerModeCallback>& callback);
    void RecoverRunningLocks();
    bool RegisterScreenStateCallback(int32_t remainTime, const sptr<IScreenOffPreCallback>& callback);
    bool UnRegisterScreenStateCallback(const sptr<IScreenOffPreCallback>& callback);
    bool RegisterRunningLockCallback(const sptr<IPowerRunninglockCallback>& callback);
    bool UnRegisterRunningLockCallback(const sptr<IPowerRunninglockCallback>& callback);
    std::string Dump(const std::vector<std::string>& args);
    PowerErrors GetError();

    /**
     * Check whether the type of running lock is enabled
     * @param type The type of running lock for query if it is enabled
     * @param result The result of whether the type of running lock is enabled
     * @return PowerErrors::ERR_OK if the call success, otherwise return error code
     */
    PowerErrors IsRunningLockEnabled(const RunningLockType type, bool& result);

    /**
     * Set the power key filtering strategy.
     * @param strategy The power key filtering strategy.
     * @return PowerErrors::ERR_OK if the call success, otherwise return error code
     */
    PowerErrors SetPowerKeyFilteringStrategy(PowerKeyFilteringStrategy strategy);

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
    sptr<IPowerMgr> GetPowerMgrProxy();
    void ResetProxy(const wptr<IRemoteObject>& remote);
    sptr<IPowerMgr> proxy_ {nullptr};
    sptr<IRemoteObject::DeathRecipient> deathRecipient_ {nullptr};
    std::mutex mutex_;
    static std::vector<std::weak_ptr<RunningLock>> runningLocks_;
    static std::mutex runningLocksMutex_;
    sptr<IRemoteObject> token_ {nullptr};
    PowerErrors error_ = PowerErrors::ERR_OK;

private:
    PowerMgrClient();
    DISALLOW_COPY_AND_MOVE(PowerMgrClient);
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_POWER_MGR_CLIENT_H
