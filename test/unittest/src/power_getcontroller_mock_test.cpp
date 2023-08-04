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
#include "power_getcontroller_mock_test.h"
#include <fstream>
#include <thread>
#include <unistd.h>

#ifdef POWERMGR_GTEST
#define private   public
#define protected public
#endif

#include <datetime_ex.h>
#include <input_manager.h>
#include <securec.h>

#include "power_state_callback_stub.h"
#include "power_state_machine.h"
#include "setting_helper.h"

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
    void OnStart() override;
    void OnStop() override;
    void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
    int32_t Dump(int32_t fd, const std::vector<std::u16string>& args) override;
    PowerErrors RebootDevice(const std::string& reason) override;
    PowerErrors RebootDeviceForDeprecated(const std::string& reason) override;
    PowerErrors ShutDownDevice(const std::string& reason) override;
    PowerErrors SuspendDevice(int64_t callTimeMs, SuspendDeviceType reason, bool suspendImmed) override;
    PowerErrors WakeupDevice(int64_t callTimeMs, WakeupDeviceType reason, const std::string& details) override;
    bool RefreshActivity(int64_t callTimeMs, UserActivityType type, bool needChangeBacklight) override;
    bool OverrideScreenOffTime(int64_t timeout) override;
    bool RestoreScreenOffTime() override;
    PowerState GetState() override;
    bool IsScreenOn() override;
    bool ForceSuspendDevice(int64_t callTimeMs) override;
    PowerErrors CreateRunningLock(
        const sptr<IRemoteObject>& remoteObj, const RunningLockInfo& runningLockInfo) override;
    bool ReleaseRunningLock(const sptr<IRemoteObject>& remoteObj) override;
    bool IsRunningLockTypeSupported(RunningLockType type) override;
    bool Lock(const sptr<IRemoteObject>& remoteObj, int32_t timeOutMS) override;
    bool UnLock(const sptr<IRemoteObject>& remoteObj) override;
    void ForceUnLock(const sptr<IRemoteObject>& remoteObj);
    bool IsUsed(const sptr<IRemoteObject>& remoteObj) override;
    bool ProxyRunningLock(bool isProxied, pid_t pid, pid_t uid) override;
    bool ProxyRunningLocks(bool isProxied, const std::vector<std::pair<pid_t, pid_t>>& processInfos) override;
    bool ResetRunningLocks() override;
    bool RegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback) override;
    bool UnRegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback) override;
    bool RegisterPowerModeCallback(const sptr<IPowerModeCallback>& callback) override;
    bool UnRegisterPowerModeCallback(const sptr<IPowerModeCallback>& callback) override;
    bool SetDisplaySuspend(bool enable) override;
    PowerErrors SetDeviceMode(const PowerMode& mode) override;
    PowerMode GetDeviceMode() override;
    std::string ShellDump(const std::vector<std::string>& args, uint32_t argc) override;
    PowerErrors IsStandby(bool& isStandby) override;

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
        return nullptr;
    }
    std::shared_ptr<WakeupController> GetWakeupController() const
    {
        return nullptr;
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
    void MockProximity(uint32_t status)
    {
        POWER_HILOGI(LABEL_TEST, "MockProximity: fun is start");
        runningLockMgr_->SetProximity(status);
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
        static const int32_t timeout = 10000; // 10seconds
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

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;
static sptr<PowerMgrService> g_service;
static constexpr int SLEEP_WAIT_TIME_S = 2;

class InputCallbackMock : public IInputEventConsumer {
public:
    virtual void OnInputEvent(std::shared_ptr<KeyEvent> keyEvent) const;
    virtual void OnInputEvent(std::shared_ptr<PointerEvent> pointerEvent) const;
    virtual void OnInputEvent(std::shared_ptr<AxisEvent> axisEvent) const;
};

void PowerGetControllerMockTest::SetUpTestCase(void)
{
    g_service = DelayedSpSingleton<PowerMgrService>::GetInstance();
    g_service->OnStart();
}

void PowerGetControllerMockTest::TearDownTestCase(void)
{
    g_service->OnStop();
    DelayedSpSingleton<PowerMgrService>::DestroyInstance();
}

namespace {
/**
 * @tc.name: PowerGetControllerMockTest001
 * @tc.desc: test GetSourceList(exception)
 * @tc.type: FUNC
 * @tc.require: issueI7G6OY
 */
HWTEST_F(PowerGetControllerMockTest, PowerGetControllerMockTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerGetControllerMockTest001: start";

    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pmsTest_ == nullptr) {
        GTEST_LOG_(INFO) << "PowerSuspendTest014: Failed to get PowerMgrService";
    }

    pmsTest_->Init();
    std::shared_ptr<PowerStateMachine> stateMachine = nullptr;
    std::shared_ptr<WakeupController> wakeupController_ = std::make_shared<WakeupController>(stateMachine);
    wakeupController_->Wakeup();
    EXPECT_TRUE(wakeupController_ != nullptr);

    InputCallback* callback = new InputCallback();
    InputCallbackMock* callback_mock = reinterpret_cast<InputCallbackMock*>(callback);
    std::shared_ptr<OHOS::MMI::KeyEvent> keyEvent = OHOS::MMI::KeyEvent::Create();
    keyEvent->SetKeyAction(MMI::KeyEvent::KEY_ACTION_DOWN);
    keyEvent->SetKeyCode(OHOS::MMI::KeyEvent::KEYCODE_F1);
    callback_mock->OnInputEvent(keyEvent);
    EXPECT_TRUE(callback_mock != nullptr);
    delete callback;
    sleep(SLEEP_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerGetControllerMockTest001:  end";
}
} // namespace