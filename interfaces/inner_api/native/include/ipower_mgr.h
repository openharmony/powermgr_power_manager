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

#ifndef POWERMGR_IPOWER_MANAGER_H
#define POWERMGR_IPOWER_MANAGER_H

#include <string>
#include <vector>
#include <map>

#include <iremote_broker.h>
#include <iremote_object.h>

#include "ipower_mode_callback.h"
#include "ipower_runninglock_callback.h"
#include "ipower_state_callback.h"
#include "iscreen_off_pre_callback.h"
#include "power_errors.h"
#include "power_state_machine_info.h"
#include "running_lock_info.h"
#include "shutdown/ishutdown_client.h"
#include "suspend/isync_sleep_callback.h"

namespace OHOS {
namespace PowerMgr {
class IPowerMgr : public IShutdownClient, public IRemoteBroker {
public:
    virtual PowerErrors CreateRunningLock(const sptr<IRemoteObject>& remoteObj,
        const RunningLockInfo& runningLockInfo) = 0;
    virtual bool ReleaseRunningLock(const sptr<IRemoteObject>& remoteObj) = 0;
    virtual bool IsRunningLockTypeSupported(RunningLockType type) = 0;
    virtual bool Lock(const sptr<IRemoteObject>& remoteObj, int32_t timeOutMs = -1) = 0;
    virtual bool UnLock(const sptr<IRemoteObject>& remoteObj) = 0;
    virtual bool QueryRunningLockLists(std::map<std::string, RunningLockInfo>& runningLockLists) = 0;
    virtual bool IsUsed(const sptr<IRemoteObject>& remoteObj) = 0;
    virtual bool ProxyRunningLock(bool isProxied, pid_t pid, pid_t uid) = 0;
    virtual bool ProxyRunningLocks(bool isProxied, const std::vector<std::pair<pid_t, pid_t>>& processInfos) = 0;
    virtual bool ResetRunningLocks() = 0;

    // Used for power state machine.
    virtual PowerErrors RebootDevice(const std::string& reason) = 0;
    virtual PowerErrors RebootDeviceForDeprecated(const std::string& reason) = 0;
    virtual PowerErrors ShutDownDevice(const std::string& reason) = 0;
    virtual PowerErrors SuspendDevice(int64_t callTimeMs, SuspendDeviceType reason, bool suspendImmed) = 0;
    virtual PowerErrors WakeupDevice(int64_t callTimeMs, WakeupDeviceType reason, const std::string& details) = 0;
    virtual bool RefreshActivity(int64_t callTimeMs, UserActivityType type, bool needChangeBacklight) = 0;
    virtual bool OverrideScreenOffTime(int64_t timeout) = 0;
    virtual bool RestoreScreenOffTime() = 0;
    virtual PowerState GetState() = 0;
    virtual bool IsScreenOn() = 0;
    virtual bool ForceSuspendDevice(int64_t callTimeMs) = 0;
    virtual bool RegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback) = 0;
    virtual bool UnRegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback) = 0;

    virtual bool RegisterSyncSleepCallback(const sptr<ISyncSleepCallback>& callback,
        SleepPriority priority = SleepPriority::DEFAULT) = 0;
    virtual bool UnRegisterSyncSleepCallback(const sptr<ISyncSleepCallback>& callback) = 0;

    // Used for callback registration upon power mode.
    virtual bool RegisterPowerModeCallback(const sptr<IPowerModeCallback>& callback) = 0;
    virtual bool UnRegisterPowerModeCallback(const sptr<IPowerModeCallback>& callback) = 0;

    virtual bool RegisterScreenStateCallback(int32_t remainTime, const sptr<IScreenOffPreCallback>& callback) = 0;
    virtual bool UnRegisterScreenStateCallback(const sptr<IScreenOffPreCallback>& callback) = 0;

    virtual bool SetDisplaySuspend(bool enable) = 0;
    virtual bool Hibernate(bool clearMemory) = 0;
    virtual PowerErrors SetDeviceMode(const PowerMode& mode) = 0;
    virtual PowerMode GetDeviceMode() = 0;
    virtual std::string ShellDump(const std::vector<std::string>& args, uint32_t argc) = 0;
    virtual PowerErrors IsStandby(bool& isStandby) = 0;
    virtual PowerErrors SetForceTimingOut(bool enabled) = 0;
    virtual PowerErrors LockScreenAfterTimingOut(bool enabledLockScreen, bool checkLock) = 0;

    virtual void RegisterShutdownCallback(
        const sptr<ITakeOverShutdownCallback>& callback, ShutdownPriority priority) = 0;
    virtual void UnRegisterShutdownCallback(const sptr<ITakeOverShutdownCallback>& callback) = 0;

    virtual void RegisterShutdownCallback(
        const sptr<IAsyncShutdownCallback>& callback, ShutdownPriority priority) = 0;
    virtual void UnRegisterShutdownCallback(const sptr<IAsyncShutdownCallback>& callback) = 0;
    virtual void RegisterShutdownCallback(
        const sptr<ISyncShutdownCallback>& callback, ShutdownPriority priority) = 0;
    virtual void UnRegisterShutdownCallback(const sptr<ISyncShutdownCallback>& callback) = 0;

    virtual bool RegisterRunningLockCallback(const sptr<IPowerRunninglockCallback>& callback) = 0;
    virtual bool UnRegisterRunningLockCallback(const sptr<IPowerRunninglockCallback>& callback) = 0;

    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.powermgr.IPowerMgr");
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_IPOWER_MANAGER_H
