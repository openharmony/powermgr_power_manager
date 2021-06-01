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

#ifndef POWERMGR_IPOWER_MANAGER_H
#define POWERMGR_IPOWER_MANAGER_H

#include <string>

#include <iremote_broker.h>
#include <iremote_object.h>

#include "ipower_state_callback.h"
#include "ishutdown_callback.h"
#include "power_state_machine_info.h"
#include "running_lock_info.h"

namespace OHOS {
namespace PowerMgr {
class IPowerMgr : public IRemoteBroker {
public:
    enum {
        RUNNINGLOCK_LOCK = 0,
        RUNNINGLOCK_UNLOCK,
        RUNNINGLOCK_SET_WORK_TRIGGER_LIST,
        RUNNINGLOCK_ISUSED,
        PROXY_RUNNINGLOCK,
        WAKEUP_DEVICE,
        SUSPEND_DEVICE,
        REFRESH_ACTIVITY,
        IS_SCREEN_ON,
        FORCE_DEVICE_SUSPEND,
        REBOOT_DEVICE,
        SHUTDOWN_DEVICE,
        REG_POWER_STATE_CALLBACK,
        UNREG_POWER_STATE_CALLBACK,
        REG_SHUTDOWN_CALLBACK,
        UNREG_SHUTDOWN_CALLBACK
    };

    virtual void Lock(const sptr<IRemoteObject>& token, const RunningLockInfo& runningLockInfo,
        uint32_t timeOutMs) = 0;
    virtual void UnLock(const sptr<IRemoteObject>& token) = 0;
    virtual bool IsUsed(const sptr<IRemoteObject>& token) = 0;
    virtual void SetWorkTriggerList(const sptr<IRemoteObject>& token, const WorkTriggerList& workTriggerList) = 0;
    virtual void ProxyRunningLock(bool proxyLock, pid_t uid, pid_t pid) = 0;

    // Used for power state machine.
    virtual void RebootDevice(const std::string& reason) = 0;
    virtual void ShutDownDevice(const std::string& reason) = 0;
    virtual void SuspendDevice(int64_t callTimeMs, SuspendDeviceType reason, bool suspendImmed) = 0;
    virtual void WakeupDevice(int64_t callTimeMs, WakeupDeviceType reason, const std::string& details) = 0;
    virtual void RefreshActivity(int64_t callTimeMs, UserActivityType type, bool needChangeBacklight) = 0;
    virtual bool IsScreenOn() = 0;
    virtual bool ForceSuspendDevice(int64_t callTimeMs) = 0;
    virtual void RegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback) = 0;
    virtual void UnRegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback) = 0;

    // Used for callback registration upon shutdown.
    virtual void RegisterShutdownCallback(const sptr<IShutdownCallback>& callback) = 0;
    virtual void UnRegisterShutdownCallback(const sptr<IShutdownCallback>& callback) = 0;

    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.powermgr.IPowerMgr");
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_IPOWER_MANAGER_H
