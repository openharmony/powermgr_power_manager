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

#include <common_event_subscriber.h>
#include <iremote_object.h>
#ifdef POWER_MANAGER_ENABLE_EXTERNAL_SCREEN_MANAGEMENT
#include <screen_manager_lite.h>
#endif
#include <system_ability.h>

#include "actions/idevice_power_action.h"
#include "ffrt_utils.h"
#include "ipower_mgr.h"
#include "power_mgr_notify.h"
#include "power_mgr_stub.h"
#include "power_mode_module.h"
#include "power_save_mode.h"
#include "power_state_machine.h"
#include "running_lock_mgr.h"
#include "screen_off_pre_controller.h"
#include "shutdown_controller.h"
#include "shutdown_dialog.h"
#include "sp_singleton.h"
#include "suspend_controller.h"
#include "wakeup_controller.h"
#ifdef POWER_MANAGER_WAKEUP_ACTION
#include "wakeup_action_controller.h"
#endif

namespace OHOS {
namespace PowerMgr {
#ifdef POWER_MANAGER_ENABLE_CHARGING_TYPE_SETTING
enum class PowerConnectStatus : int32_t {
    POWER_CONNECT_INVALID = -1,
    POWER_CONNECT_DC = 0, // DC for Direct Current, means battery supply
    POWER_CONNECT_AC = 1, // AC for Alternating Current, means charing supply
};
#endif

class RunningLockMgr;
class PowerMgrService final : public SystemAbility, public PowerMgrStub {
    DECLARE_SYSTEM_ABILITY(PowerMgrService)
    DECLARE_DELAYED_SP_SINGLETON(PowerMgrService);

public:
    static void RegisterSettingWakeupPickupGestureObserver();
    static void WakeupPickupGestureSettingUpdateFunc(const std::string& key);
    static void RegisterSettingWakeupDoubleClickObservers();
    static void WakeupDoubleClickSettingUpdateFunc(const std::string& key);
    static bool GetSettingWakeupDoubleClick(const std::string& key = SETTING_POWER_WAKEUP_DOUBLE_KEY);
    static void RegisterSettingPowerModeObservers();
    static void PowerModeSettingUpdateFunc(const std::string& key);
    static void RegisterSettingWakeUpLidObserver();
    static void WakeupLidSettingUpdateFunc(const std::string& key);

    virtual void OnStart() override;
    virtual void OnStop() override;
    virtual void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
    virtual void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
    int32_t Dump(int32_t fd, const std::vector<std::u16string>& args) override;
    virtual PowerErrors RebootDevice(const std::string& reason) override;
    virtual PowerErrors RebootDeviceForDeprecated(const std::string& reason) override;
    virtual PowerErrors ShutDownDevice(const std::string& reason) override;
    virtual PowerErrors SetSuspendTag(const std::string& tag) override;
    virtual PowerErrors SuspendDevice(int64_t callTimeMs, SuspendDeviceType reason, bool suspendImmed) override;
    virtual PowerErrors WakeupDevice(int64_t callTimeMs, WakeupDeviceType reason, const std::string& details) override;
    virtual void WakeupDeviceAsync(int64_t callTimeMs, WakeupDeviceType reason, const std::string& details) override {};
    virtual bool RefreshActivity(int64_t callTimeMs, UserActivityType type, bool needChangeBacklight) override;
    bool RefreshActivityInner(int64_t callTimeMs, UserActivityType type, bool needChangeBacklight);
    virtual PowerErrors OverrideScreenOffTime(int64_t timeout) override;
    virtual PowerErrors RestoreScreenOffTime() override;
    virtual PowerState GetState() override;
    virtual bool IsScreenOn(bool needPrintLog = true) override;
    virtual bool IsFoldScreenOn() override;
    virtual bool IsCollaborationScreenOn() override;
    virtual PowerErrors ForceSuspendDevice(int64_t callTimeMs) override;
    virtual PowerErrors Hibernate(bool clearMemory) override;
    virtual PowerErrors CreateRunningLock(
        const sptr<IRemoteObject>& remoteObj, const RunningLockInfo& runningLockInfo) override;
    virtual bool ReleaseRunningLock(const sptr<IRemoteObject>& remoteObj, const std::string& name = "") override;
    virtual bool IsRunningLockTypeSupported(RunningLockType type) override;
    virtual bool UpdateWorkSource(const sptr<IRemoteObject>& remoteObj,
        const std::map<int32_t, std::string>& workSources) override;
    virtual PowerErrors Lock(const sptr<IRemoteObject>& remoteObj, int32_t timeOutMs = -1) override;
    virtual PowerErrors UnLock(const sptr<IRemoteObject>& remoteObj, const std::string& name = "") override;
    virtual bool QueryRunningLockLists(std::map<std::string, RunningLockInfo>& runningLockLists) override;
    virtual void ForceUnLock(const sptr<IRemoteObject>& remoteObj);
    virtual bool IsUsed(const sptr<IRemoteObject>& remoteObj) override;
    virtual bool ProxyRunningLock(bool isProxied, pid_t pid, pid_t uid) override;
    virtual bool ProxyRunningLocks(bool isProxied,
        const std::vector<std::pair<pid_t, pid_t>>& processInfos) override;
    virtual bool ResetRunningLocks() override;
    virtual bool RegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback, bool isSync = true) override;
    virtual bool UnRegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback) override;

    virtual bool RegisterSyncSleepCallback(const sptr<ISyncSleepCallback>& callback, SleepPriority priority) override;
    virtual bool UnRegisterSyncSleepCallback(const sptr<ISyncSleepCallback>& callback) override;
    virtual bool RegisterSyncHibernateCallback(const sptr<ISyncHibernateCallback>& callback) override;
    virtual bool UnRegisterSyncHibernateCallback(const sptr<ISyncHibernateCallback>& callback) override;

    virtual bool RegisterPowerModeCallback(const sptr<IPowerModeCallback>& callback) override;
    virtual bool UnRegisterPowerModeCallback(const sptr<IPowerModeCallback>& callback) override;

    virtual bool RegisterScreenStateCallback(int32_t remainTime, const sptr<IScreenOffPreCallback>& callback) override;
    virtual bool UnRegisterScreenStateCallback(const sptr<IScreenOffPreCallback>& callback) override;

    virtual bool RegisterRunningLockCallback(const sptr<IPowerRunninglockCallback>& callback) override;
    virtual bool UnRegisterRunningLockCallback(const sptr<IPowerRunninglockCallback>& callback) override;
    virtual bool SetDisplaySuspend(bool enable) override;
    virtual PowerErrors SetDeviceMode(const PowerMode& mode) override;
    virtual PowerMode GetDeviceMode() override;
    virtual std::string ShellDump(const std::vector<std::string>& args, uint32_t argc) override;
    virtual PowerErrors IsStandby(bool& isStandby) override;
    virtual PowerErrors SetForceTimingOut(bool enabled, const sptr<IRemoteObject>& token) override;
    virtual PowerErrors LockScreenAfterTimingOut(
        bool enabledLockScreen, bool checkLock, bool sendScreenOffEvent, const sptr<IRemoteObject>& token) override;
    virtual PowerErrors IsRunningLockEnabled(const RunningLockType type, bool& result) override;

    void SetEnableDoze(bool enable);
    void RegisterShutdownCallback(const sptr<ITakeOverShutdownCallback>& callback, ShutdownPriority priority) override;
    void UnRegisterShutdownCallback(const sptr<ITakeOverShutdownCallback>& callback) override;

    void RegisterShutdownCallback(const sptr<IAsyncShutdownCallback>& callback, ShutdownPriority priority) override;
    void UnRegisterShutdownCallback(const sptr<IAsyncShutdownCallback>& callback) override;
    void RegisterShutdownCallback(const sptr<ISyncShutdownCallback>& callback, ShutdownPriority priority) override;
    void UnRegisterShutdownCallback(const sptr<ISyncShutdownCallback>& callback) override;

#ifdef POWER_MANAGER_ENABLE_EXTERNAL_SCREEN_MANAGEMENT
    void RegisterExternalScreenListener();
    void UnRegisterExternalScreenListener();
#endif
    void HandleKeyEvent(int32_t keyCode);
    void HandlePointEvent(int32_t type);
    void KeyMonitorCancel();
    void SwitchSubscriberInit();
    void SwitchSubscriberCancel();
    void HallSensorSubscriberInit();
    void HallSensorSubscriberCancel();
    bool CheckDialogFlag();
    void InputMonitorInit();
    void InputMonitorCancel();
    bool CheckDialogAndShuttingDown();
    void SuspendControllerInit();
    void WakeupControllerInit();
#ifdef POWER_MANAGER_POWER_ENABLE_S4
    void HibernateControllerInit();
#endif
    bool IsCollaborationState();
    void QueryRunningLockListsInner(std::map<std::string, RunningLockInfo>& runningLockLists);
#ifdef POWER_MANAGER_WAKEUP_ACTION
    void WakeupActionControllerInit();
#endif
    void VibratorInit();
    void Reset();
    void KeepScreenOnInit();
    void KeepScreenOn(bool isOpenOn);
    void UnregisterAllSettingObserver();
    void RegisterAllSettingObserver();
    int64_t GetSettingDisplayOffTime(int64_t defaultTime);
#ifdef POWER_MANAGER_ENABLE_CHARGING_TYPE_SETTING
    PowerConnectStatus GetPowerConnectStatus() const
    {
        return powerConnectStatus_;
    }
    void SetPowerConnectStatus(PowerConnectStatus status)
    {
        powerConnectStatus_ = status;
    }
    void PowerConnectStatusInit();
    bool IsPowerConnected();
    void UpdateSettingInvalidDisplayOffTime();
#endif
    void OnChargeStateChanged();

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
#ifdef POWER_MANAGER_POWER_ENABLE_S4
    std::shared_ptr<HibernateController> GetHibernateController() const
    {
        return hibernateController_;
    }
#endif
    std::shared_ptr<WakeupController> GetWakeupController() const
    {
        return wakeupController_;
    }
    std::shared_ptr<ScreenOffPreController> GetScreenOffPreController() const
    {
        return screenOffPreController_;
    }
#ifdef POWER_MANAGER_WAKEUP_ACTION
    std::shared_ptr<WakeupActionController> GetWakeupActionController() const
    {
        return wakeupActionController_;
    }
#endif
    ShutdownDialog& GetShutdownDialog()
    {
        return shutdownDialog_;
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
        POWER_HILOGI(LABEL_TEST, "Service EnableMock:%{public}d", mockCount_++);
        runningLockMgr_->EnableMock(lockAction);
        powerStateMachine_->EnableMock(powerState);
        shutdownController_->EnableMock(powerAction, shutdownState);
    }

    void EnableShutdownMock(ShutdownController* shutdownAction)
    {
        POWER_HILOGI(LABEL_TEST, "need to mock shutdownController");
        std::unique_ptr<ShutdownController> mock(shutdownAction);
        shutdownController_.reset();
        shutdownController_ = std::move(mock);
    }

    void MockProximity(uint32_t status)
    {
        POWER_HILOGI(LABEL_TEST, "MockProximity: fun is start");
#ifdef HAS_SENSORS_SENSOR_PART
        runningLockMgr_->SetProximity(status);
#endif
        POWER_HILOGI(LABEL_TEST, "MockProximity: fun is end");
    }
    void MockSystemWakeup()
    {
        PowerStateMachine::onWakeup();
    }
    std::shared_ptr<ShutdownController> GetShutdownController()
    {
        return shutdownController_;
    }
#ifdef HAS_SENSORS_SENSOR_PART
    static bool isInLidMode_;
#endif

private:
    static constexpr int32_t POWER_KEY_PRESS_DELAY_MS = 10000;
    static constexpr int32_t INIT_KEY_MONITOR_DELAY_MS = 1000;
    static constexpr int32_t HALL_REPORT_INTERVAL = 0;
    static constexpr uint32_t HALL_SAMPLING_RATE = 100000000;
    static constexpr const char* SETTING_POWER_WAKEUP_DOUBLE_KEY {"settings.power.wakeup_double_click"};
    static std::atomic_bool isBootCompleted_;

    static void RegisterBootCompletedCallback();
    static void PowerExternalAbilityInit();
    static bool IsDeveloperMode();
#ifdef HAS_SENSORS_SENSOR_PART
    static void HallSensorCallback(SensorEvent* event);
#endif

#ifdef POWER_MANAGER_ENABLE_EXTERNAL_SCREEN_MANAGEMENT
    class ExternalScreenListener : public Rosen::ScreenManagerLite::IScreenListener {
    public:
        virtual void OnConnect(uint64_t screenId) override;
        virtual void OnDisconnect(uint64_t screenId) override;
        virtual void OnChange(uint64_t screenId) override {}
    };
#endif
    class BackgroundRunningLock {
    public:
        BackgroundRunningLock(std::string name, int32_t timeOutMs);
        ~BackgroundRunningLock();
    private:
        sptr<IRemoteObject> token_ {nullptr};
    };

    class InvokerDeathRecipient : public DeathRecipient {
        using CallbackType = std::function<void(const sptr<PowerMgrService>&)>;

    public:
        InvokerDeathRecipient(std::string interfaceName, CallbackType callback)
            : interfaceName_(interfaceName),
              callback_(callback) {}
        virtual ~InvokerDeathRecipient() = default;
        virtual void OnRemoteDied(const wptr<IRemoteObject>& remote) override;

    private:
        std::string interfaceName_;
        CallbackType callback_;
    };

    inline PowerModeModule& GetPowerModeModule()
    {
        return powerModeModule_;
    }

    bool Init();
    bool PowerStateMachineInit();
    std::string GetBundleNameByUid(const int32_t uid);
    RunningLockParam FillRunningLockParam(const RunningLockInfo& info, const uint64_t lockid, int32_t timeOutMS = -1);
    void SubscribeCommonEvent();
#ifdef MSDP_MOVEMENT_ENABLE
    void RegisterMovementCallback();
    void UnRegisterMovementCallback();
    void ResetMovementState();
#endif
#ifdef HAS_SENSORS_SENSOR_PART
    bool IsSupportSensor(SensorTypeId);
#endif
    void RegisterExternalCallback();
    void UnregisterExternalCallback();

    bool ready_ {false};
    std::mutex wakeupMutex_;
    std::mutex suspendMutex_;
#ifdef POWER_MANAGER_POWER_ENABLE_S4
    std::mutex hibernateMutex_;
#endif
    std::mutex stateMutex_;
    std::mutex shutdownMutex_;
    std::mutex modeMutex_;
    std::mutex screenOffPreMutex_;
    std::mutex screenMutex_;
    std::mutex dumpMutex_;
    std::mutex lockMutex_;
    std::shared_ptr<RunningLockMgr> runningLockMgr_ {nullptr};
    std::shared_ptr<PowerStateMachine> powerStateMachine_ {nullptr};
    std::shared_ptr<PowerMgrNotify> powerMgrNotify_ {nullptr};
    std::shared_ptr<ShutdownController> shutdownController_ {nullptr};
    std::shared_ptr<FFRTTimer> ffrtTimer_ {nullptr};
    std::shared_ptr<EventFwk::CommonEventSubscriber> subscriberPtr_ {nullptr};
    std::shared_ptr<SuspendController> suspendController_ {nullptr};
    std::shared_ptr<WakeupController> wakeupController_ {nullptr};
    sptr<IRemoteObject> ptoken_ {nullptr};
#ifdef POWER_MANAGER_POWER_ENABLE_S4
    std::shared_ptr<HibernateController> hibernateController_ {nullptr};
#endif
    std::shared_ptr<ScreenOffPreController> screenOffPreController_ {nullptr};
#ifdef POWER_MANAGER_WAKEUP_ACTION
    std::shared_ptr<WakeupActionController> wakeupActionController_ {nullptr};
#endif
#ifdef POWER_MANAGER_ENABLE_EXTERNAL_SCREEN_MANAGEMENT
    sptr<Rosen::ScreenManagerLite::IScreenListener> externalScreenListener_ {nullptr};
#endif
    PowerModeModule powerModeModule_;
    ShutdownDialog shutdownDialog_;
    uint32_t mockCount_ {0};
    int32_t switchId_ {0};
    int32_t doubleClickId_ {0};
    int32_t monitorId_ {0};
    int32_t inputMonitorId_ {-1};
#ifdef POWER_MANAGER_ENABLE_CHARGING_TYPE_SETTING
    PowerConnectStatus powerConnectStatus_ {PowerConnectStatus::POWER_CONNECT_INVALID};
#endif
#ifdef HAS_SENSORS_SENSOR_PART
    SensorUser sensorUser_{};
#endif
};

#ifdef HAS_MULTIMODALINPUT_INPUT_PART
class PowerMgrInputMonitor : public IInputEventConsumer {
public:
    virtual void OnInputEvent(std::shared_ptr<KeyEvent> keyEvent) const;
    virtual void OnInputEvent(std::shared_ptr<PointerEvent> pointerEvent) const;
    virtual void OnInputEvent(std::shared_ptr<AxisEvent> axisEvent) const;
};
#endif

class PowerCommonEventSubscriber : public EventFwk::CommonEventSubscriber {
public:
    explicit PowerCommonEventSubscriber(const EventFwk::CommonEventSubscribeInfo& subscribeInfo)
        : EventFwk::CommonEventSubscriber(subscribeInfo) {}
    virtual ~PowerCommonEventSubscriber() {}
    void OnReceiveEvent(const EventFwk::CommonEventData &data) override;
private:
#ifdef POWER_MANAGER_ENABLE_CHARGING_TYPE_SETTING
    void OnPowerConnectStatusChanged(PowerConnectStatus status);
#endif
};

} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_POWER_MGR_SERVICE_H
