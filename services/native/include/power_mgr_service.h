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
#include "power_mode_module.h"
#include "power_save_mode.h"

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
    virtual PowerErrors RebootDevice(const std::string& reason) override;
    virtual PowerErrors RebootDeviceForDeprecated(const std::string& reason) override;
    virtual PowerErrors ShutDownDevice(const std::string& reason) override;
    virtual PowerErrors SuspendDevice(int64_t callTimeMs, SuspendDeviceType reason,
        bool suspendImmed) override;
    virtual PowerErrors WakeupDevice(int64_t callTimeMs, WakeupDeviceType reason,
        const std::string& details) override;
    virtual bool RefreshActivity(int64_t callTimeMs, UserActivityType type,
        bool needChangeBacklight) override;
    virtual bool OverrideScreenOffTime(int64_t timeout) override;
    virtual bool RestoreScreenOffTime() override;
    virtual PowerState GetState() override;
    virtual bool IsScreenOn() override;
    virtual bool ForceSuspendDevice(int64_t callTimeMs) override;
    virtual PowerErrors CreateRunningLock(const sptr<IRemoteObject>& remoteObj,
        const RunningLockInfo& runningLockInfo) override;
    virtual bool ReleaseRunningLock(const sptr<IRemoteObject>& remoteObj) override;
    virtual bool IsRunningLockTypeSupported(uint32_t type) override;
    virtual bool Lock(const sptr<IRemoteObject>& remoteObj,
        const RunningLockInfo& runningLockInfo, uint32_t timeOutMS) override;
    virtual bool UnLock(const sptr<IRemoteObject>& remoteObj) override;
    virtual void ForceUnLock(const sptr<IRemoteObject>& remoteObj);
    virtual bool IsUsed(const sptr<IRemoteObject>& remoteObj) override;
    virtual bool SetWorkTriggerList(const sptr<IRemoteObject>& remoteObj,
        const WorkTriggerList& workTriggerList) override;
    virtual bool ProxyRunningLock(bool proxyLock, pid_t uid, pid_t pid) override;
    virtual bool RegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback) override;
    virtual bool UnRegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback) override;
    virtual bool RegisterShutdownCallback(IShutdownCallback::ShutdownPriority priority,
        const sptr<IShutdownCallback>& callback) override;
    virtual bool UnRegisterShutdownCallback(const sptr<IShutdownCallback>& callback) override;
    virtual bool RegisterPowerModeCallback(const sptr<IPowerModeCallback>& callback) override;
    virtual bool UnRegisterPowerModeCallback(const sptr<IPowerModeCallback>& callback) override;
    virtual bool SetDisplaySuspend(bool enable) override;
    virtual PowerErrors SetDeviceMode(const PowerMode& mode) override;
    virtual PowerMode GetDeviceMode() override;
    virtual std::string ShellDump(const std::vector<std::string>& args, uint32_t argc) override;

    void HandleShutdownRequest();
    void HandleKeyEvent(int32_t keyCode);
    void HandlePointEvent(int32_t type);
    void NotifyDisplayActionDone(uint32_t event);
    void KeyMonitorInit();
    void KeyMonitorCancel();
    void HallSensorSubscriberInit();
    void HallSensorSubscriberCancel();
    bool ShowPowerDialog();
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
    void SetDisplayOffTime(int64_t time)
    {
        powerStateMachine_->SetDisplayOffTime(time);
    }
    void SetSleepTime(int64_t time)
    {
        powerStateMachine_->SetSleepTime(time);
    }
    void HandleScreenOnTimeout();

    void EnableMock(IDeviceStateAction* powerState, IDeviceStateAction* shutdownState,
        IDevicePowerAction* powerAction, IRunningLockAction* lockAction)
    {
        POWER_HILOGE(LABEL_TEST, "Service EnableMock:%{public}d", mockCount_++);
        runningLockMgr_->EnableMock(lockAction);
        powerStateMachine_->EnableMock(powerState);
        shutdownService_.EnableMock(powerAction, shutdownState);
    }
    void MockProximity(uint32_t status)
    {
        POWER_HILOGE(LABEL_TEST, "MockProximity: fun is start");
        runningLockMgr_->SetProximity(status);
        POWER_HILOGE(LABEL_TEST, "MockProximity: fun is end");
    }
    void MockSystemWakeup()
    {
        PowerStateMachine::onWakeup();
    }
private:
    static constexpr int32_t LONG_PRESS_DELAY_MS = 3000;
    static constexpr int32_t POWER_KEY_PRESS_DELAY_MS = 10000;
    static constexpr int32_t INIT_KEY_MONITOR_DELAY_MS = 1000;
    static constexpr int32_t HALL_REPORT_INTERVAL = 0;
    static constexpr uint32_t HALL_SAMPLING_RATE = 100000000;
    bool Init();
    bool PowerStateMachineInit();
    void HandlePowerKeyUp();
    void NotifyRunningLockChanged(bool isUnLock);
    void FillUserIPCInfo(UserIPCInfo &userIPCinfo);
    bool IsSupportSensor(SensorTypeId);
    static void HallSensorCallback(SensorEvent* event);
    static void RegisterBootCompletedCallback();

    inline PowerModeModule& GetPowerModeModule()
    {
        return powerModeModule_;
    }

    bool ready_ {false};
    std::mutex mutex_;
    std::mutex lockMutex_;
    std::shared_ptr<RunningLockMgr> runningLockMgr_;
    std::shared_ptr<AppExecFwk::EventRunner> eventRunner_;
    std::shared_ptr<PowermsEventHandler> handler_;
    std::shared_ptr<PowerStateMachine> powerStateMachine_;
    std::shared_ptr<PowerMgrNotify> powerMgrNotify_;
    ShutdownService shutdownService_;
    PowerModeModule powerModeModule_;
    bool powerkeyPressed_ {false};
    uint32_t mockCount_ {0};
    bool isDialogstatus_ {false};
    int32_t powerkeyLongPressId_ {0};
    int32_t powerkeyShortPressId_ {0};
    int32_t powerkeyReleaseId_ {0};
    int32_t doubleClickId_ {0};
    int32_t monitorId_ {0};
    SensorUser sensorUser_;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_POWER_MGR_SERVICE_H
