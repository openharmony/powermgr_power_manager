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

#ifndef POWERMGR_SERVICES_IPOWER_MANAGER_PROXY_H
#define POWERMGR_SERVICES_IPOWER_MANAGER_PROXY_H

#include <iremote_proxy.h>
#include <nocopyable.h>

#include "ipower_mgr.h"
#include "power_state_machine_info.h"

namespace OHOS {
namespace PowerMgr {
class PowerMgrProxy : public IRemoteProxy<IPowerMgr> {
public:
    explicit PowerMgrProxy(const sptr<IRemoteObject>& impl)
        : IRemoteProxy<IPowerMgr>(impl) {}
    ~PowerMgrProxy() = default;
    DISALLOW_COPY_AND_MOVE(PowerMgrProxy);

    virtual void CreateRunningLock(const sptr<IRemoteObject>& remoteObj,
        const RunningLockInfo& runningLockInfo) override;
    virtual void ReleaseRunningLock(const sptr<IRemoteObject>& remoteObj) override;
    virtual bool IsRunningLockTypeSupported(uint32_t type) override;
    virtual void Lock(const sptr<IRemoteObject>& remoteObj, const RunningLockInfo& runningLockInfo,
        uint32_t timeOutMs) override;
    virtual void UnLock(const sptr<IRemoteObject>& remoteObj) override;
    virtual void SetWorkTriggerList(const sptr<IRemoteObject>& remoteObj,
        const WorkTriggerList& workTriggerList) override;
    virtual void ProxyRunningLock(bool proxyLock, pid_t uid, pid_t pid) override;
    virtual bool IsUsed(const sptr<IRemoteObject>& remoteObj) override;
    // Use for PowerStateMachine
    virtual void SuspendDevice(int64_t callTimeMs, SuspendDeviceType reason, bool suspendImmed) override;
    virtual void WakeupDevice(int64_t callTimeMs, WakeupDeviceType reason, const std::string& details) override;
    virtual void RefreshActivity(int64_t callTimeMs, UserActivityType type, bool needChangeBacklight) override;
    virtual bool OverrideScreenOffTime(int64_t timeout) override;
    virtual bool RestoreScreenOffTime() override;
    virtual PowerState GetState() override;
    virtual bool IsScreenOn() override;
    virtual bool ForceSuspendDevice(int64_t callTimeMs) override;
    virtual void RebootDevice(const std::string& reason) override;
    virtual void ShutDownDevice(const std::string& reason) override;
    virtual void RegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback) override;
    virtual void UnRegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback) override;
    virtual void RegisterShutdownCallback(IShutdownCallback::ShutdownPriority priority,
        const sptr<IShutdownCallback>& callback) override;
    virtual void UnRegisterShutdownCallback(const sptr<IShutdownCallback>& callback) override;
    virtual void RegisterPowerModeCallback(const sptr<IPowerModeCallback>& callback) override;
    virtual void UnRegisterPowerModeCallback(const sptr<IPowerModeCallback>& callback) override;
    virtual void SetDisplaySuspend(bool enable) override;
    virtual void SetDeviceMode(const uint32_t& mode) override;
    virtual uint32_t GetDeviceMode() override;
    virtual std::string ShellDump(const std::vector<std::string>& args, uint32_t argc) override;
private:
    static inline BrokerDelegator<PowerMgrProxy> delegator_;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_SERVICES_IPOWER_MANAGER_PROXY_H
