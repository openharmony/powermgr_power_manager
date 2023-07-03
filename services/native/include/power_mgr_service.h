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
#include "power_mgr_notify.h"
#include "power_mgr_stub.h"
#include "power_mode_module.h"
#include "power_save_mode.h"
#include "power_state_machine.h"
#include "running_lock_mgr.h"
#include "shutdown_controller.h"
#include "sp_singleton.h"
#include "suspend_controller.h"
#include "wakeup_controller.h"

namespace OHOS {
namespace PowerMgr {
class RunningLockMgr;
class PowerMgrService final : public SystemAbility, public PowerMgrStub {
    DECLARE_SYSTEM_ABILITY(PowerMgrService)
    DECLARE_DELAYED_SP_SINGLETON(PowerMgrService);

public:
    virtual void OnStart() override;
    virtual void OnStop() override;
    virtual void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
    int32_t Dump(int32_t fd, const std::vector<std::u16string>& args) override;
    virtual PowerErrors RebootDevice(const std::string& reason) override;
    virtual PowerErrors RebootDeviceForDeprecated(const std::string& reason) override;
    virtual PowerErrors ShutDownDevice(const std::string& reason) override;
    virtual PowerErrors SuspendDevice(int64_t callTimeMs, SuspendDeviceType reason, bool suspendImmed) override;
    virtual PowerErrors WakeupDevice(int64_t callTimeMs, WakeupDeviceType reason, const std::string& details) override;
    virtual bool RefreshActivity(int64_t callTimeMs, UserActivityType type, bool needChangeBacklight) override;
    virtual bool OverrideScreenOffTime(int64_t timeout) override;
    virtual bool RestoreScreenOffTime() override;
    virtual PowerState GetState() override;
    virtual bool IsScreenOn() override;
    virtual bool ForceSuspendDevice(int64_t callTimeMs) override;
    virtual PowerErrors CreateRunningLock(
        const sptr<IRemoteObject>& remoteObj, const RunningLockInfo& runningLockInfo) override;
    virtual bool ReleaseRunningLock(const sptr<IRemoteObject>& remoteObj) override;
    virtual bool IsRunningLockTypeSupported(RunningLockType type) override;
    virtual bool Lock(const sptr<IRemoteObject>& remoteObj, int32_t timeOutMS) override;
    virtual bool UnLock(const sptr<IRemoteObject>& remoteObj) override;
    virtual void ForceUnLock(const sptr<IRemoteObject>& remoteObj);
    virtual bool IsUsed(const sptr<IRemoteObject>& remoteObj) override;
    virtual bool ProxyRunningLock(bool isProxied, pid_t pid, pid_t uid) override;
    virtual bool RegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback) override;
    virtual bool UnRegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback) override;
    virtual bool RegisterPowerModeCallback(const sptr<IPowerModeCallback>& callback) override;
    virtual bool UnRegisterPowerModeCallback(const sptr<IPowerModeCallback>& callback) override;
    virtual bool SetDisplaySuspend(bool enable) override;
    virtual PowerErrors SetDeviceMode(const PowerMode& mode) override;
    virtual PowerMode GetDeviceMode() override;
    virtual std::string ShellDump(const std::vector<std::string>& args, uint32_t argc) override;

    void RegisterShutdownCallback(const sptr<ITakeOverShutdownCallback>& callback, ShutdownPriority priority) override;
    void UnRegisterShutdownCallback(const sptr<ITakeOverShutdownCallback>& callback) override;

    void RegisterShutdownCallback(const sptr<IAsyncShutdownCallback>& callback, ShutdownPriority priority) override;
    void UnRegisterShutdownCallback(const sptr<IAsyncShutdownCallback>& callback) override;
    void RegisterShutdownCallback(const sptr<ISyncShutdownCallback>& callback, ShutdownPriority priority) override;
    void UnRegisterShutdownCallback(const sptr<ISyncShutdownCallback>& callback) override;

    void HandleShutdownRequest();
    void HandleKeyEvent(int32_t keyCode);
    void HandlePointEvent(int32_t type);
    void KeyMonitorInit();
    void KeyMonitorCancel();
    void SwitchSubscriberInit();
    void SwitchSubscriberCancel();
    void HallSensorSubscriberInit();
    void HallSensorSubscriberCancel();
    bool ShowPowerDialog();
    bool CheckDialogAndShuttingDown();
    void SuspendControllerInit();
    void WakeupControllerInit();

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

    std::shared_ptr<SuspendController> GetSuspendController() const
    {
        return suspendController_;
    }
    std::shared_ptr<WakeupController> GetWakeupController() const
    {
        return wakeupController_;
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

    void EnableMock(IDeviceStateAction* powerState, IDeviceStateAction* shutdownState, IDevicePowerAction* powerAction,
        IRunningLockAction* lockAction)
    {
        POWER_HILOGE(LABEL_TEST, "Service EnableMock:%{public}d", mockCount_++);
        runningLockMgr_->EnableMock(lockAction);
        powerStateMachine_->EnableMock(powerState);
        shutdownController_->EnableMock(powerAction, shutdownState);
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
    std::shared_ptr<ShutdownController> GetShutdownController()
    {
        return shutdownController_;
    }

    std::shared_ptr<SuspendController> suspendController_ = nullptr;
    std::shared_ptr<WakeupController> wakeupController_ = nullptr;

private:
    class WakeupRunningLock {
    public:
        static void Create();
        static void Lock();
        static void Unlock();

    private:
        WakeupRunningLock() = default;
        ~WakeupRunningLock() = default;

        static sptr<RunningLockTokenStub> token_;
        static const int32_t TIMEOUT = 10000; // 10seconds
    };

    static constexpr int32_t LONG_PRESS_DELAY_MS = 3000;
    static constexpr int32_t POWER_KEY_PRESS_DELAY_MS = 10000;
    static constexpr int32_t INIT_KEY_MONITOR_DELAY_MS = 1000;
    static constexpr int32_t HALL_REPORT_INTERVAL = 0;
    static constexpr uint32_t HALL_SAMPLING_RATE = 100000000;
    bool Init();
    bool PowerStateMachineInit();
    void NotifyRunningLockChanged(bool isUnLock);
    RunningLockParam FillRunningLockParam(const RunningLockInfo& info, int32_t timeOutMS = -1);
    bool IsSupportSensor(SensorTypeId);
    static void HallSensorCallback(SensorEvent* event);
    static void RegisterBootCompletedCallback();

    inline PowerModeModule& GetPowerModeModule()
    {
        return powerModeModule_;
    }

    bool ready_ {false};
    static std::atomic_bool isBootCompleted_;
    std::mutex wakeupMutex_;
    std::mutex suspendMutex_;
    std::mutex stateMutex_;
    std::mutex shutdownMutex_;
    std::mutex modeMutex_;
    std::mutex screenMutex_;
    std::mutex dumpMutex_;
    std::mutex lockMutex_;
    std::shared_ptr<RunningLockMgr> runningLockMgr_;
    std::shared_ptr<PowerStateMachine> powerStateMachine_;
    std::shared_ptr<PowerMgrNotify> powerMgrNotify_;
    std::shared_ptr<ShutdownController> shutdownController_;
    PowerModeModule powerModeModule_;
    uint32_t mockCount_ {0};
    bool isDialogShown_ {false};
    int32_t powerkeyLongPressId_ {0};
    int32_t switchId_ {0};
    int32_t doubleClickId_ {0};
    int32_t monitorId_ {0};
    SensorUser sensorUser_;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_POWER_MGR_SERVICE_H
