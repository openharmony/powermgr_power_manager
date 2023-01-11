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

#include <iremote_broker.h>
#include <iremote_object.h>

#include "ipower_mode_callback.h"
#include "ipower_state_callback.h"
#include "ishutdown_callback.h"
#include "power_errors.h"
#include "power_state_machine_info.h"
#include "running_lock_info.h"

namespace OHOS {
namespace PowerMgr {
class IPowerMgr : public IRemoteBroker {
public:
    enum {
        CREATE_RUNNINGLOCK = 0,
        RELEASE_RUNNINGLOCK,
        IS_RUNNINGLOCK_TYPE_SUPPORTED,
        RUNNINGLOCK_LOCK,
        RUNNINGLOCK_UNLOCK,
        RUNNINGLOCK_SET_WORK_TRIGGER_LIST,
        RUNNINGLOCK_ISUSED,
        PROXY_RUNNINGLOCK,
        WAKEUP_DEVICE,
        SUSPEND_DEVICE,
        REFRESH_ACTIVITY,
        OVERRIDE_DISPLAY_OFF_TIME,
        RESTORE_DISPLAY_OFF_TIME,
        GET_STATE,
        IS_SCREEN_ON,
        FORCE_DEVICE_SUSPEND,
        REBOOT_DEVICE,
        REBOOT_DEVICE_FOR_DEPRECATED,
        SHUTDOWN_DEVICE,
        REG_POWER_STATE_CALLBACK,
        UNREG_POWER_STATE_CALLBACK,
        REG_SHUTDOWN_CALLBACK,
        UNREG_SHUTDOWN_CALLBACK,
        REG_POWER_MODE_CALLBACK,
        UNREG_POWER_MODE_CALLBACK,
        SET_DISPLAY_SUSPEND,
        SETMODE_DEVICE,
        GETMODE_DEVICE,
        SHELL_DUMP
    };

    virtual PowerErrors CreateRunningLock(const sptr<IRemoteObject>& remoteObj,
        const RunningLockInfo& runningLockInfo) = 0;
    virtual bool ReleaseRunningLock(const sptr<IRemoteObject>& remoteObj) = 0;
    virtual bool IsRunningLockTypeSupported(uint32_t type) = 0;
    virtual bool Lock(const sptr<IRemoteObject>& remoteObj, const RunningLockInfo& runningLockInfo,
        uint32_t timeOutMs) = 0;
    virtual bool UnLock(const sptr<IRemoteObject>& remoteObj) = 0;
    virtual bool IsUsed(const sptr<IRemoteObject>& remoteObj) = 0;
    virtual bool SetWorkTriggerList(const sptr<IRemoteObject>& remoteObj, const WorkTriggerList& workTriggerList) = 0;
    virtual bool ProxyRunningLock(bool proxyLock, pid_t uid, pid_t pid) = 0;

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

    // Used for callback registration upon shutdown.
    virtual bool RegisterShutdownCallback(IShutdownCallback::ShutdownPriority priority,
        const sptr<IShutdownCallback>& callback) = 0;
    virtual bool UnRegisterShutdownCallback(const sptr<IShutdownCallback>& callback) = 0;

    // Used for callback registration upon power mode.
    virtual bool RegisterPowerModeCallback(const sptr<IPowerModeCallback>& callback) = 0;
    virtual bool UnRegisterPowerModeCallback(const sptr<IPowerModeCallback>& callback) = 0;

    virtual bool SetDisplaySuspend(bool enable) = 0;
    virtual PowerErrors SetDeviceMode(const PowerMode& mode) = 0;
    virtual PowerMode GetDeviceMode() = 0;
    virtual std::string ShellDump(const std::vector<std::string>& args, uint32_t argc) = 0;

    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.powermgr.IPowerMgr");
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_IPOWER_MANAGER_H
