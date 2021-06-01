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

#ifndef POWERMGR_POWER_MGR_SERVICE_H
#define POWERMGR_POWER_MGR_SERVICE_H

#include <iremote_object.h>
#include <system_ability.h>

#include "actions/idevice_power_action.h"
#include "ipower_mgr.h"
#include "powerms_event_handler.h"
#include "power_mgr_notify.h"
#include "power_mgr_stub.h"
#include "power_state_machine.h"
#include "running_lock_mgr.h"
#include "shutdown_service.h"
#include "sp_singleton.h"

namespace OHOS {
namespace PowerMgr {
class RunningLockMgr;
class PowerMgrService final : public SystemAbility, public PowerMgrStub {
    DECLARE_SYSTEM_ABILITY(PowerMgrService)
    DECLARE_DELAYED_SP_SINGLETON(PowerMgrService);

public:
    virtual void OnStart() override;
    virtual void OnStop() override;
    int32_t Dump(int32_t fd, const std::vector<std::u16string>& args) override;
    virtual void RebootDevice(const std::string& reason) override;
    virtual void ShutDownDevice(const std::string& reason) override;
    virtual void SuspendDevice(int64_t callTimeMs, SuspendDeviceType reason, bool suspendImmed) override;
    virtual void WakeupDevice(int64_t callTimeMs, WakeupDeviceType reason, const std::string& details) override;
    virtual void RefreshActivity(int64_t callTimeMs, UserActivityType type, bool needChangeBacklight) override;
    virtual bool IsScreenOn() override;
    virtual bool ForceSuspendDevice(int64_t callTimeMs) override;
    void Lock(const sptr<IRemoteObject>& token, const RunningLockInfo& runningLockInfo, uint32_t timeOutMS) override;
    void UnLock(const sptr<IRemoteObject>& token) override;
    void ForceUnLock(const sptr<IRemoteObject>& token);
    bool IsUsed(const sptr<IRemoteObject>& token) override;
    void SetWorkTriggerList(const sptr<IRemoteObject>& token, const WorkTriggerList& workTriggerList) override;
    void ProxyRunningLock(bool proxyLock, pid_t uid, pid_t pid) override;
    void RegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback) override;
    void UnRegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback) override;
    void RegisterShutdownCallback(const sptr<IShutdownCallback>& callback) override;
    void UnRegisterShutdownCallback(const sptr<IShutdownCallback>& callback) override;

    std::shared_ptr<PowermsEventHandler> GetHandler() const
    {
        return handler_;
    }

    std::shared_ptr<RunningLockMgr> GetRunningLockMgr() const
    {
        return runningLockMgr_;
    }

    std::shared_ptr<PowerStateMachine> GetPowerStateMachine() const
    {
        return powerStateMachine_;
    }

    std::shared_ptr<PowerMgrNotify> GetPowerMgrNotify() const
    {
        return powerMgrNotify_;
    }

    bool IsServiceReady() const
    {
        return ready_;
    }

private:
    bool Init();
    bool PowerStateMachineInit();
    void NotifyRunningLockChanged(bool isUnLock);
    void FillUserIPCInfo(UserIPCInfo &userIPCinfo);
    bool ready_ {false};
    std::mutex mutex_;
    std::shared_ptr<RunningLockMgr> runningLockMgr_;
    std::shared_ptr<AppExecFwk::EventRunner> eventRunner_;
    std::shared_ptr<PowermsEventHandler> handler_;
    std::shared_ptr<PowerStateMachine> powerStateMachine_;
    std::shared_ptr<PowerMgrNotify> powerMgrNotify_;
    ShutdownService shutdownService_;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_POWER_MGR_SERVICE_H
