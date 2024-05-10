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

#ifndef POWERMGR_SERVICES_IPOWER_MANAGER_PROXY_H
#define POWERMGR_SERVICES_IPOWER_MANAGER_PROXY_H

#include <cstdint>
#include <sys/types.h>
#include <functional>
#include <iosfwd>
#include <vector>
#include <iremote_proxy.h>
#include <nocopyable.h>
#include "refbase.h"
#include "iremote_broker.h"
#include "iremote_object.h"
#include "ipower_mode_callback.h"
#include "ipower_state_callback.h"
#include "ipower_runninglock_callback.h"
#include "ipower_mgr.h"
#include "iscreen_off_pre_callback.h"
#include "running_lock_info.h"
#include "power_state_machine_info.h"
#include "power_errors.h"

namespace OHOS {
namespace PowerMgr {
class PowerMgrProxy : public IRemoteProxy<IPowerMgr> {
public:
    explicit PowerMgrProxy(const sptr<IRemoteObject>& impl)
        : IRemoteProxy<IPowerMgr>(impl) {}
    ~PowerMgrProxy() = default;
    DISALLOW_COPY_AND_MOVE(PowerMgrProxy);

    virtual PowerErrors CreateRunningLock(const sptr<IRemoteObject>& remoteObj,
        const RunningLockInfo& runningLockInfo) override;
    virtual bool ReleaseRunningLock(const sptr<IRemoteObject>& remoteObj) override;
    virtual bool IsRunningLockTypeSupported(RunningLockType type) override;
    virtual bool Lock(const sptr<IRemoteObject>& remoteObj, int32_t timeOutMs = -1) override;
    virtual bool UnLock(const sptr<IRemoteObject>& remoteObj) override;
    virtual bool QueryRunningLockLists(std::map<std::string, RunningLockInfo>& runningLockLists) override;
    virtual bool ProxyRunningLock(bool isProxied, pid_t pid, pid_t uid) override;
    virtual bool ProxyRunningLocks(bool isProxied,
        const std::vector<std::pair<pid_t, pid_t>>& processInfos) override;
    virtual bool ResetRunningLocks() override;
    virtual bool IsUsed(const sptr<IRemoteObject>& remoteObj) override;
    // Use for PowerStateMachine
    virtual PowerErrors SuspendDevice(int64_t callTimeMs, SuspendDeviceType reason, bool suspendImmed) override;
    virtual PowerErrors WakeupDevice(int64_t callTimeMs, WakeupDeviceType reason, const std::string& details) override;
    virtual bool RefreshActivity(int64_t callTimeMs, UserActivityType type, bool needChangeBacklight) override;
    virtual bool OverrideScreenOffTime(int64_t timeout) override;
    virtual bool RestoreScreenOffTime() override;
    virtual PowerState GetState() override;
    virtual bool IsScreenOn() override;
    virtual bool ForceSuspendDevice(int64_t callTimeMs) override;
    virtual PowerErrors RebootDevice(const std::string& reason) override;
    virtual PowerErrors RebootDeviceForDeprecated(const std::string& reason) override;
    virtual PowerErrors ShutDownDevice(const std::string& reason) override;
    virtual bool RegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback) override;
    virtual bool UnRegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback) override;
    virtual bool RegisterPowerModeCallback(const sptr<IPowerModeCallback>& callback) override;
    virtual bool UnRegisterPowerModeCallback(const sptr<IPowerModeCallback>& callback) override;
    virtual bool RegisterRunningLockCallback(const sptr<IPowerRunninglockCallback>& callback) override;
    virtual bool UnRegisterRunningLockCallback(const sptr<IPowerRunninglockCallback>& callback) override;
    virtual bool RegisterScreenStateCallback(int32_t remainTime, const sptr<IScreenOffPreCallback>& callback) override;
    virtual bool UnRegisterScreenStateCallback(const sptr<IScreenOffPreCallback>& callback) override;
    virtual bool SetDisplaySuspend(bool enable) override;
    virtual bool Hibernate(bool clearMemory) override;
    virtual PowerErrors SetDeviceMode(const PowerMode& mode) override;
    virtual PowerMode GetDeviceMode() override;
    virtual std::string ShellDump(const std::vector<std::string>& args, uint32_t argc) override;
    virtual PowerErrors IsStandby(bool& isStandby) override;
    virtual PowerErrors SetForceTimingOut(bool enabled) override;
    virtual PowerErrors LockScreenAfterTimingOut(bool enabledLockScreen, bool checkLock) override;

    void RegisterShutdownCallback(const sptr<ITakeOverShutdownCallback>& callback, ShutdownPriority priority) override;
    void UnRegisterShutdownCallback(const sptr<ITakeOverShutdownCallback>& callback) override;
    void RegisterShutdownCallback(const sptr<IAsyncShutdownCallback>& callback, ShutdownPriority priority) override;
    void UnRegisterShutdownCallback(const sptr<IAsyncShutdownCallback>& callback) override;
    void RegisterShutdownCallback(const sptr<ISyncShutdownCallback>& callback, ShutdownPriority priority) override;
    void UnRegisterShutdownCallback(const sptr<ISyncShutdownCallback>& callback) override;

    virtual bool RegisterSyncSleepCallback(const sptr<ISyncSleepCallback>& callback, SleepPriority priority) override;
    virtual bool UnRegisterSyncSleepCallback(const sptr<ISyncSleepCallback>& callback) override;

private:
    static inline BrokerDelegator<PowerMgrProxy> delegator_;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_SERVICES_IPOWER_MANAGER_PROXY_H
