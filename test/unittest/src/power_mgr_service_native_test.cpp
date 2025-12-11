/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "power_mgr_service_native_test.h"

#ifdef THERMAL_GTEST
#define private   public
#define protected public
#endif

#include <csignal>
#include <iostream>

#ifdef HAS_MULTIMODALINPUT_INPUT_PART
#include "key_event.h"
#include "pointer_event.h"
#endif
#include "permission.h"
#include "power_common.h"
#include "power_mgr_service.h"
#include "async_ulsr_callback_stub.h"
#include "setting_helper.h"
#include "shutdown/async_shutdown_callback_stub.h"
using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

namespace {
constexpr int32_t TIMEOUTMS = 7;
constexpr int64_t CALLTIMEMS = 1;
constexpr int64_t SUSCALLTIMEMS = 3;
constexpr pid_t PID = 1;
constexpr pid_t UID = 1;
constexpr int32_t UNCANCELID = -1;
constexpr int32_t INVALID_CODE = -1;
constexpr int32_t TRY_TIMES = 2;
sptr<PowerMgrService> g_pmsTest;
bool g_isSystem = true;
bool g_isPermissionGranted = true;
} // namespace

bool Permission::IsSystem()
{
    return g_isSystem;
}

bool Permission::IsPermissionGranted(const std::string& perm)
{
    return g_isPermissionGranted;
}

void PowerMgrServiceNativeTest::SetUp()
{
    g_pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    EXPECT_TRUE(g_pmsTest != nullptr) << "PowerMgrService02 fail to get PowerMgrService";
    g_pmsTest->OnStart();

    auto stateMachine = std::make_shared<PowerStateMachine>(g_pmsTest);
    EXPECT_TRUE(stateMachine->Init());
    UserActivityType userActivityType = UserActivityType::USER_ACTIVITY_TYPE_ACCESSIBILITY;
    stateMachine->RefreshActivityInner(PID, CALLTIMEMS, userActivityType, true);
    auto runningLockMgr = std::make_shared<RunningLockMgr>(g_pmsTest);
    EXPECT_TRUE(runningLockMgr->Init());
    sptr<IRemoteObject> remoteObj = new RunningLockTokenStub();
    RunningLockParam runningLockParam {0,
        "runninglockNativeTest1", "", RunningLockType::RUNNINGLOCK_SCREEN, TIMEOUTMS, PID, UID};
    EXPECT_TRUE(runningLockMgr->CreateRunningLock(remoteObj, runningLockParam) != nullptr);
    runningLockMgr->Lock(remoteObj);
    runningLockMgr->UnLock(remoteObj);
    EXPECT_FALSE(runningLockMgr->ReleaseLock(remoteObj));
}

void PowerMgrServiceNativeTest::TearDown()
{
    g_pmsTest->OnStop();
}

void PowerStateTestCallback::OnPowerStateChanged(PowerState state)
{
    POWER_HILOGI(
        LABEL_TEST, "PowerStateTestCallback::OnPowerStateChanged state = %{public}u.", static_cast<uint32_t>(state));
}

namespace {
/**
 * @tc.name: PowerMgrServiceNative001
 * @tc.desc: test init in powerMgrService
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PowerMgrServiceNative001 start.";
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative001 function start!");
    g_pmsTest->HallSensorSubscriberInit();
    g_pmsTest->HallSensorSubscriberCancel();
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
    int32_t keyCode = OHOS::MMI::KeyEvent::KEYCODE_F1;
    g_pmsTest->HandleKeyEvent(keyCode);
    int32_t type = OHOS::MMI::PointerEvent::SOURCE_TYPE_MOUSE;
    g_pmsTest->HandlePointEvent(type);

    EXPECT_EQ(g_pmsTest->OverrideScreenOffTime(TIMEOUTMS), PowerErrors::ERR_OK);
    EXPECT_EQ(g_pmsTest->RestoreScreenOffTime(), PowerErrors::ERR_OK);

    g_pmsTest->doubleClickId_ = UNCANCELID;
    g_pmsTest->monitorId_ = UNCANCELID;
#endif
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative001 function end!");
    GTEST_LOG_(INFO) << "PowerMgrServiceNative001 end.";
}

/**
 * @tc.name: PowerMgrServiceNative002
 * @tc.desc: test handleKeyEvent in powerMgrService
 * @tc.type: FUNC
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PowerMgrServiceNative002 start.";
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative002 function start!");
    int32_t keyCode = OHOS::MMI::KeyEvent::KEYCODE_F1;
    g_pmsTest->HandleKeyEvent(keyCode);
    keyCode = OHOS::MMI::KeyEvent::KEYCODE_F2;
    g_pmsTest->HandleKeyEvent(keyCode);
    keyCode = OHOS::MMI::KeyEvent::KEYCODE_VIRTUAL_MULTITASK;
    g_pmsTest->HandleKeyEvent(keyCode);
    keyCode = OHOS::MMI::KeyEvent::KEYCODE_WEAR_1;
    g_pmsTest->HandleKeyEvent(keyCode);
    int32_t type = OHOS::MMI::PointerEvent::SOURCE_TYPE_MOUSE;
    g_pmsTest->HandlePointEvent(type);
    type = OHOS::MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN;
    g_pmsTest->HandlePointEvent(type);

    EXPECT_TRUE(g_pmsTest->ShutDownDevice(SHUTDOWN_FAST_REASON) == PowerErrors::ERR_OK);
    SuspendDeviceType reasonSDT = SuspendDeviceType::SUSPEND_DEVICE_REASON_DEVICE_ADMIN;
    EXPECT_TRUE(g_pmsTest->SuspendDevice(SUSCALLTIMEMS, reasonSDT, false) == PowerErrors::ERR_OK);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative002 function end!");
    GTEST_LOG_(INFO) << "PowerMgrServiceNative002 end.";
}
#endif

/**
 * @tc.name: PowerMgrServiceNative003
 * @tc.desc: test pickup register
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PowerMgrServiceNative003 start.";
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative003 function start!");
#ifdef POWER_PICKUP_ENABLE
    g_pmsTest->RegisterSettingWakeupPickupGestureObserver();
    g_pmsTest->RegisterSettingWakeupPickupGestureObserver();
    EXPECT_TRUE(SettingHelper::pickUpObserver_ != nullptr);
    SettingHelper::UnregisterSettingWakeupPickupObserver();
    SettingHelper::UnregisterSettingWakeupPickupObserver();
    EXPECT_TRUE(SettingHelper::pickUpObserver_ == nullptr);
#endif
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative003 function end!");
    GTEST_LOG_(INFO) << "PowerMgrServiceNative003 end.";
}

/**
 * @tc.name: PowerMgrServiceNative004
 * @tc.desc: test autobrightness register
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PowerMgrServiceNative004 start.";
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative004 function start!");
    shared_ptr<PowerModeModule> powerModeModuleTest = make_shared<PowerModeModule>();
    EXPECT_TRUE(powerModeModuleTest != nullptr);
    auto flag = SettingHelper::IsAutoAdjustBrightnessSettingValid();
    powerModeModuleTest->RegisterAutoAdjustBrightnessObserver();
    SettingHelper::SetSettingAutoAdjustBrightness(SettingHelper::SwitchStatus::INVALID);
    auto ret = SettingHelper::GetSettingAutoAdjustBrightness(INVALID_CODE);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative004 function end!");
    GTEST_LOG_(INFO) << "PowerMgrServiceNative004 end.";
}

/**
 * @tc.name: PowerMgrServiceNative005
 * @tc.desc: test brightness register
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PowerMgrServiceNative005 start.";
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative005 function start!");
    shared_ptr<PowerModeModule> powerModeModuleTest = make_shared<PowerModeModule>();
    EXPECT_TRUE(powerModeModuleTest != nullptr);
    powerModeModuleTest->RegisterAutoAdjustBrightnessObserver();
    auto flag = SettingHelper::IsBrightnessSettingValid();
    SettingHelper::SetSettingBrightness(INVALID_CODE);
    auto ret = SettingHelper::GetSettingAutoAdjustBrightness(INVALID_CODE);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative005 function end!");
    GTEST_LOG_(INFO) << "PowerMgrServiceNative005 end.";
}

/**
 * @tc.name: PowerMgrServiceNative006
 * @tc.desc: test vibrateState register
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PowerMgrServiceNative006 start.";
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative006 function start!");
    shared_ptr<PowerModeModule> powerModeModuleTest = make_shared<PowerModeModule>();
    EXPECT_TRUE(powerModeModuleTest != nullptr);
    powerModeModuleTest->RegisterVibrateStateObserver();
    auto flag = SettingHelper::IsVibrationSettingValid();
    SettingHelper::SetSettingVibration(SettingHelper::SwitchStatus::INVALID);
    auto ret = SettingHelper::GetSettingVibration(INVALID_CODE);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative006 function end!");
    GTEST_LOG_(INFO) << "PowerMgrServiceNative006 end.";
}


/**
 * @tc.name: PowerMgrServiceNative007
 * @tc.desc: test autoWindowRotation register
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PowerMgrServiceNative007 start.";
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative007 function start!");
    shared_ptr<PowerModeModule> powerModeModuleTest = make_shared<PowerModeModule>();
    EXPECT_TRUE(powerModeModuleTest != nullptr);
    powerModeModuleTest->RegisterAutoWindowRotationObserver();
    auto flag = SettingHelper::IsWindowRotationSettingValid();
    SettingHelper::SetSettingWindowRotation(SettingHelper::SwitchStatus::INVALID);
    auto ret = SettingHelper::GetSettingWindowRotation(INVALID_CODE);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative007 function end!");
    GTEST_LOG_(INFO) << "PowerMgrServiceNative007 end.";
}

/**
 * @tc.name: PowerMgrServiceNative008
 * @tc.desc: test intellVoiceSetting register
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PowerMgrServiceNative008 start.";
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative008 function start!");
    shared_ptr<PowerModeModule> powerModeModuleTest = make_shared<PowerModeModule>();
    EXPECT_TRUE(powerModeModuleTest != nullptr);
    powerModeModuleTest->RegisterIntellVoiceObserver();
    auto flag = SettingHelper::IsIntellVoiceSettingValid();
    SettingHelper::SetSettingIntellVoice(SettingHelper::SwitchStatus::INVALID);
    auto ret = SettingHelper::GetSettingIntellVoice(INVALID_CODE);
    powerModeModuleTest->UnregisterSaveModeObserver();
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative008 function end!");
    GTEST_LOG_(INFO) << "PowerMgrServiceNative008 end.";
}

/**
 * @tc.name: PowerMgrServiceNative009
 * @tc.desc: test settingPowerMode register
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PowerMgrServiceNative009 start.";
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative009 function start!");
    g_pmsTest->RegisterSettingPowerModeObservers();
    g_pmsTest->RegisterSettingPowerModeObservers();
    EXPECT_TRUE(SettingHelper::powerModeObserver_ != nullptr);
    SettingHelper::SaveCurrentMode(INVALID_CODE);
    SettingHelper::UnRegisterSettingPowerModeObserver();
    EXPECT_TRUE(SettingHelper::powerModeObserver_ == nullptr);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative009 function end!");
    GTEST_LOG_(INFO) << "PowerMgrServiceNative009 end.";
}

/**
 * @tc.name: PowerMgrServiceNative010
 * @tc.desc: test settingWakeUpLid register
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PowerMgrServiceNative010 start.";
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative010 function start!");
    g_pmsTest->RegisterSettingWakeUpLidObserver();
    SettingHelper::SetSettingWakeupLid(true);
    bool ret = SettingHelper::GetSettingWakeupLid();
    SettingHelper::UnRegisterSettingWakeupLidObserver();
    EXPECT_TRUE(SettingHelper::lidObserver_ == nullptr);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative010 function end!");
    GTEST_LOG_(INFO) << "PowerMgrServiceNative010 end.";
}

/**
 * @tc.name: PowerMgrServiceNative011
 * @tc.desc: test RegisterSettingWakeupDoubleClickObservers
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative011, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PowerMgrServiceNative011 start.";
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative011 function start!");
#ifdef POWER_DOUBLECLICK_ENABLE
    g_pmsTest->RegisterSettingWakeupDoubleClickObservers();
    g_pmsTest->RegisterSettingWakeupDoubleClickObservers();
    SettingHelper::IsWakeupDoubleSettingValid();
    SettingHelper::UnregisterSettingWakeupDoubleObserver();
    SettingHelper::UnregisterSettingWakeupDoubleObserver();
    EXPECT_TRUE(SettingHelper::doubleClickObserver_ == nullptr);
#endif
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative011 function end!");
    GTEST_LOG_(INFO) << "PowerMgrServiceNative011 end.";
}

/**
 * @tc.name: PowerMgrServiceNative012
 * @tc.desc: test RegisterMovementCallback
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative012, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PowerMgrServiceNative012 start.";
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative012 function start!");
#ifdef MSDP_MOVEMENT_ENABLE
    auto stateMachine = std::make_shared<PowerStateMachine>(g_pmsTest);
    g_pmsTest->RegisterMovementCallback();
    g_pmsTest->UnRegisterMovementCallback();
    g_pmsTest->ResetMovementState();
    bool ret =  stateMachine->IsMovementStateOn();
    EXPECT_TRUE(ret == false);
#endif
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative012 function end!");
    GTEST_LOG_(INFO) << "PowerMgrServiceNative012 end.";
}

/**
 * @tc.name: PowerMgrServiceNative013
 * @tc.desc: test RegisterExternalScreenListener
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative013, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PowerMgrServiceNative013 start.";
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative013 function start!");
#ifdef POWER_MANAGER_ENABLE_EXTERNAL_SCREEN_MANAGEMENT
    g_pmsTest->RegisterExternalScreenListener();
    EXPECT_TRUE(g_pmsTest->externalScreenListener_ != nullptr);
    g_pmsTest->UnRegisterExternalScreenListener();
    EXPECT_TRUE(g_pmsTest->externalScreenListener_ == nullptr);
#endif
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative013 function end!");
    GTEST_LOG_(INFO) << "PowerMgrServiceNative013 end.";
}

/**
 * @tc.name: PowerMgrServiceNative014
 * @tc.desc: test ExternalScreenListener OnConnect
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative014, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PowerMgrServiceNative014 start.";
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative014 function start!");
#ifdef POWER_MANAGER_ENABLE_EXTERNAL_SCREEN_MANAGEMENT
    g_pmsTest->SuspendControllerInit();
    g_pmsTest->WakeupControllerInit();
    // Register screen listener
    g_pmsTest->RegisterExternalScreenListener();
    EXPECT_TRUE(g_pmsTest->externalScreenListener_ != nullptr);

    auto powerStateMachine = g_pmsTest->GetPowerStateMachine();
    constexpr uint64_t SCREEN_A_ID = 10001;
    // Mock open switch to wakeup deivce
    // case 1: isSwitchOpen:true, isScreenOn:true
    powerStateMachine->SetSwitchState(true);
    g_pmsTest->WakeupDevice(
        static_cast<int64_t>(time(nullptr)), WakeupDeviceType::WAKEUP_DEVICE_SWITCH, "PowerMgrServiceNative014");
    EXPECT_TRUE(powerStateMachine->IsScreenOn());
    g_pmsTest->externalScreenListener_->OnConnect(SCREEN_A_ID);
    EXPECT_TRUE(powerStateMachine->IsScreenOn());
    powerStateMachine->SetExternalScreenNumber(0);

    // case 2: isSwitchOpen:true, isScreenOn:false
    g_pmsTest->SuspendDevice(GetTickCount(), SuspendDevice::SUSPEND_DEVICE_REASON_POWER_KEY, false);
    EXPECT_FALSE(powerStateMachine->IsScreenOn());
    g_pmsTest->externalScreenListener_->OnConnect(SCREEN_A_ID);
    EXPECT_TRUE(powerStateMachine->IsScreenOn());
    powerStateMachine->SetExternalScreenNumber(0);

    // case 3: isSwitchOpen:false, isScreenOn:false
    powerStateMachine->SetSwitchState(false);
    g_pmsTest->SuspendDevice(GetTickCount(), SuspendDevice::SUSPEND_DEVICE_REASON_SWITCH, false);
    EXPECT_FALSE(powerStateMachine->IsScreenOn());
    g_pmsTest->externalScreenListener_->OnConnect(SCREEN_A_ID);
    EXPECT_FALSE(powerStateMachine->IsScreenOn());
    powerStateMachine->SetExternalScreenNumber(0);

    // Unregister screen listener
    g_pmsTest->UnRegisterExternalScreenListener();
    EXPECT_TRUE(g_pmsTest->externalScreenListener_ == nullptr);
#endif
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative014 function end!");
    GTEST_LOG_(INFO) << "PowerMgrServiceNative014 end.";
}

/**
 * @tc.name: PowerMgrServiceNative015
 * @tc.desc: test ExternalScreenListener OnDisconnect
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative015, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PowerMgrServiceNative015 start.";
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative015 function start!");
#ifdef POWER_MANAGER_ENABLE_EXTERNAL_SCREEN_MANAGEMENT
    g_pmsTest->SuspendControllerInit();
    g_pmsTest->WakeupControllerInit();
    // Register screen listener
    g_pmsTest->RegisterExternalScreenListener();
    EXPECT_TRUE(g_pmsTest->externalScreenListener_ != nullptr);

    auto powerStateMachine = g_pmsTest->GetPowerStateMachine();
    constexpr uint64_t SCREEN_A_ID = 10001;
    // Mock open switch to wakeup deivce
    // case 1: isSwitchOpen:true, isScreenOn:true
    powerStateMachine->SetSwitchState(true);
    g_pmsTest->WakeupDevice(
        static_cast<int64_t>(time(nullptr)), WakeupDeviceType::WAKEUP_DEVICE_SWITCH, "PowerMgrServiceNative015");
    powerStateMachine->SetExternalScreenNumber(1);
    EXPECT_TRUE(powerStateMachine->IsScreenOn());
    g_pmsTest->externalScreenListener_->OnDisconnect(SCREEN_A_ID);
    EXPECT_TRUE(powerStateMachine->IsScreenOn());

    // case 2: isSwitchOpen:true, isScreenOn:false
    g_pmsTest->SuspendDevice(GetTickCount(), SuspendDevice::SUSPEND_DEVICE_REASON_POWER_KEY, false);
    EXPECT_FALSE(powerStateMachine->IsScreenOn());
    powerStateMachine->SetExternalScreenNumber(1);
    g_pmsTest->externalScreenListener_->OnDisconnect(SCREEN_A_ID);
    EXPECT_FALSE(powerStateMachine->IsScreenOn());

    // case 3: isSwitchOpen:false, isScreenOn:true, and there's only 1 external screen
    // Mock open switch to wakeup deivce
    powerStateMachine->SetSwitchState(true);
    g_service->WakeupDevice(
        static_cast<int64_t>(time(nullptr)), WakeupDeviceType::WAKEUP_DEVICE_SWITCH, "PowerMgrServiceNative015");
    EXPECT_TRUE(powerStateMachine->IsScreenOn());
    powerStateMachine->SetExternalScreenNumber(1);
    // Mock close switch to suspend device when switch's action is configured as ACTION_NONE
    powerStateMachine->SetSwitchState(false);
    auto suspendController = g_service->GetSuspendController();
    suspendController->ControlListener(SuspendDeviceType::SUSPEND_DEVICE_REASON_SWITCH, SuspendAction::ACTION_NONE, 0);
    EXPECT_TRUE(powerStateMachine->IsScreenOn());
    g_pmsTest->externalScreenListener_->OnDisconnect(SCREEN_A_ID);
    EXPECT_FALSE(powerStateMachine->IsScreenOn());

    // Unregister screen listener
    g_pmsTest->UnRegisterExternalScreenListener();
    EXPECT_TRUE(g_pmsTest->externalScreenListener_ == nullptr);
#endif
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative015 function end!");
    GTEST_LOG_(INFO) << "PowerMgrServiceNative015 end.";
}

/**
 * @tc.name: PowerMgrServiceNative016
 * @tc.desc: test RegisterSettingDuringCallObservers
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative016, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PowerMgrServiceNative016 start.";
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative016 function start!");
    g_pmsTest->isDuringCallStateEnable_ = true;
    EXPECT_TRUE(g_pmsTest->IsDuringCallStateEnable());
    g_pmsTest->RegisterSettingDuringCallObservers();
    EXPECT_TRUE(SettingHelper::duringCallObserver_ != nullptr);
    g_pmsTest->RegisterSettingDuringCallObservers();
    EXPECT_TRUE(SettingHelper::duringCallObserver_ != nullptr);
    SettingHelper::UnRegisterSettingDuringCallObserver();
    EXPECT_TRUE(SettingHelper::duringCallObserver_ == nullptr);
    SettingHelper::UnRegisterSettingDuringCallObserver();
    EXPECT_TRUE(SettingHelper::duringCallObserver_ == nullptr);
    g_pmsTest->DuringCallSettingUpdateFunc(SettingHelper::SETTING_DURING_CALL_STATE_KEY);
    auto stateMachine = g_pmsTest->GetPowerStateMachine();
    EXPECT_TRUE(stateMachine != nullptr && !stateMachine->isDuringCall_);
    g_pmsTest->powerStateMachine_ = nullptr;
    EXPECT_TRUE(g_pmsTest->GetPowerStateMachine() == nullptr);
    g_pmsTest->DuringCallSettingUpdateFunc(SettingHelper::SETTING_DURING_CALL_STATE_KEY);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative016 function end!");
    GTEST_LOG_(INFO) << "PowerMgrServiceNative016 end.";
}

/**
 * @tc.name: PowerMgrServiceNative017
 * @tc.desc: test RegisterSettingDuringCallObservers
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative017, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PowerMgrServiceNative017 start.";
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative017 function start!");
    g_pmsTest->isDuringCallStateEnable_ = false;
    EXPECT_FALSE(g_pmsTest->IsDuringCallStateEnable());
    g_pmsTest->RegisterSettingDuringCallObservers();
    EXPECT_TRUE(SettingHelper::duringCallObserver_ == nullptr);
    SettingHelper::UnRegisterSettingDuringCallObserver();
    EXPECT_TRUE(SettingHelper::duringCallObserver_ == nullptr);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative017 function end!");
    GTEST_LOG_(INFO) << "PowerMgrServiceNative017 end.";
}

/**
 * @tc.name: PowerMgrServiceNative018
 * @tc.desc: Test GetShutdownReason
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative018, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative018 function start!");
    auto g_pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    EXPECT_TRUE(g_pmsTest != nullptr);
    g_pmsTest->OnStart();
    std::string setReasonFirst = "reasonfirst";
    std::string setReasonSecond =
        "reasonfirstreasonfirstreasonfirstreasonfirstreasonfirstreasonfirstreasonfirstreasonfirstreasonfirst";
    g_isSystem = true;
    PowerErrors ret = g_pmsTest->GetShutdownReason(setReasonFirst);
    EXPECT_TRUE(ret == PowerErrors::ERR_OK);
    ret = g_pmsTest->GetShutdownReason(setReasonSecond);
    EXPECT_TRUE(ret == PowerErrors::ERR_OK);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative018 function end!");
}

/**
 * @tc.name: PowerMgrServiceNative019
 * @tc.desc: Test GetShutdownReason
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative019, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative019 function start!");
    auto g_pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    EXPECT_TRUE(g_pmsTest != nullptr);
    g_pmsTest->OnStart();
    std::string setReasonFirst = "reasonfirst";
    std::string setReasonSecond =
        "reasonfirstreasonfirstreasonfirstreasonfirstreasonfirstreasonfirstreasonfirstreasonfirstreasonfirst";
    g_isSystem = false;
    PowerErrors ret = g_pmsTest->GetShutdownReason(setReasonFirst);
    EXPECT_TRUE(ret != PowerErrors::ERR_PERMISSION_DENIED);
    ret = g_pmsTest->GetShutdownReason(setReasonSecond);
    EXPECT_TRUE(ret != PowerErrors::ERR_PERMISSION_DENIED);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative019 function end!");
}

class TestAsyncUlsrCallback : public AsyncUlsrCallbackStub {
public:
    TestAsyncUlsrCallback() = default;
    virtual ~TestAsyncUlsrCallback() = default;

    void OnAsyncWakeup() override
    {
        POWER_HILOGI(LABEL_TEST, "TestAsyncUlsrCallback OnAsyncWakeup!");
    }
};

class TestAsyncShutdownCallback : public AsyncShutdownCallbackStub {
public:
    TestAsyncShutdownCallback() = default;
    virtual ~TestAsyncShutdownCallback() = default;

    void OnAsyncShutdownOrReboot(bool isReboot) override
    {
        POWER_HILOGI(LABEL_TEST, "TestAsyncShutdownCallback OnAsyncShutdownOrReboot");
    }
};

/**
 * @tc.name: PowerMgrServiceNative020
 * @tc.desc: Test RegisterAsyncUlsrCallback && UnRegisterAsyncUlsrCallback
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative020, TestSize.Level2) {
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest::PowerMgrServiceNative020 start!");
    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    PowerErrors ret;
    
    // Test case 1: No system permission
    g_isSystem = false;
    sptr<TestAsyncUlsrCallback> callback = new TestAsyncUlsrCallback();
    ret = pmsTest_->RegisterUlsrCallback(callback);
    EXPECT_EQ(ret, PowerErrors::ERR_SYSTEM_API_DENIED) << "Test case 1 failed";
    ret = pmsTest_->UnRegisterUlsrCallback(callback);
    EXPECT_EQ(ret, PowerErrors::ERR_SYSTEM_API_DENIED) << "Test case 1 failed";

    // Test case 2: DEFAULT
#ifdef POWER_MANAGER_ENABLE_SUSPEND_WITH_TAG
    g_isSystem = true;
    ret = pmsTest_->RegisterUlsrCallback(callback);
    EXPECT_EQ(ret, PowerErrors::ERR_OK) << "Test case 2 failed";
    ret = pmsTest_->UnRegisterUlsrCallback(callback);
    EXPECT_EQ(ret, PowerErrors::ERR_OK) << "Test case 2 failed";
#else
    g_isSystem = true;
    ret = pmsTest_->RegisterUlsrCallback(callback);
    EXPECT_EQ(ret, PowerErrors::ERR_PARAM_INVALID) << "Test case 2 failed";
    ret = pmsTest_->UnRegisterUlsrCallback(callback);
    EXPECT_EQ(ret, PowerErrors::ERR_PARAM_INVALID) << "Test case 2 failed";
#endif
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest::PowerMgrServiceNative020 end!");
}

#ifdef POWER_MANAGER_ENABLE_SUSPEND_WITH_TAG
/**
 * @tc.name: PowerMgrServiceNative021
 * @tc.desc: Test TriggerUlsrWakeupCallback
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative021, TestSize.Level2) {
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest::PowerMgrServiceNative021 start!");
    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    PowerErrors ret;
    g_isSystem = true;
    ret = pmsTest_->RegisterUlsrCallback(nullptr);
    EXPECT_EQ(ret, PowerErrors::ERR_OK) << "Test case 2 failed";
    sptr<TestAsyncUlsrCallback> callback = new TestAsyncUlsrCallback();
    ret = pmsTest_->RegisterUlsrCallback(callback);
    EXPECT_EQ(ret, PowerErrors::ERR_OK) << "Test case 2 failed";
    EXPECT_TRUE(pmsTest_->ulsrCallbackHolder_ != nullptr);
    pmsTest_->TriggerUlsrWakeupCallback();
    ret = pmsTest_->UnRegisterUlsrCallback(callback);
    EXPECT_EQ(ret, PowerErrors::ERR_OK) << "Test case 2 failed";
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest::PowerMgrServiceNative021 end!");
}
#endif

/**
 * @tc.name: PowerMgrServiceNative022
 * @tc.desc: Test RegisterAsyncShutdownCallback && UnRegisterAsyncShutdownCallback
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative022, TestSize.Level2) {
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest::PowerMgrServiceNative022 start!");
    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    PowerErrors ret;

    // Test case 0: nullptr
    ret = pmsTest_->RegisterAsyncShutdownCallback(nullptr, ShutdownPriority::DEFAULT);
    EXPECT_EQ(ret, PowerErrors::ERR_PARAM_INVALID) << "Test case 0 failed";
    ret = pmsTest_->UnRegisterAsyncShutdownCallback(nullptr);
    EXPECT_EQ(ret, PowerErrors::ERR_PARAM_INVALID) << "Test case 0 failed";

    // Test case 1: No system permission
    g_isSystem = false;
    g_isPermissionGranted = true;
    sptr<IAsyncShutdownCallback> callback0 = new TestAsyncShutdownCallback();
    ret = pmsTest_->RegisterAsyncShutdownCallback(callback0, ShutdownPriority::DEFAULT);
    EXPECT_EQ(ret, PowerErrors::ERR_SYSTEM_API_DENIED) << "Test case 1 failed";
    ret = pmsTest_->UnRegisterAsyncShutdownCallback(callback0);
    EXPECT_EQ(ret, PowerErrors::ERR_SYSTEM_API_DENIED) << "Test case 1 failed";

    // Test case 2: No POWER_MANAGER permission
    g_isSystem = true;
    g_isPermissionGranted = false;
    ret = pmsTest_->RegisterAsyncShutdownCallback(callback0, ShutdownPriority::DEFAULT);
    EXPECT_EQ(ret, PowerErrors::ERR_PERMISSION_DENIED) << "Test case 2 failed";
    ret = pmsTest_->UnRegisterAsyncShutdownCallback(callback0);
    EXPECT_EQ(ret, PowerErrors::ERR_PERMISSION_DENIED) << "Test case 2 failed";

    // Test case 3: DEFAULT
    g_isSystem = true;
    g_isPermissionGranted = true;
    ret = pmsTest_->RegisterAsyncShutdownCallback(callback0, ShutdownPriority::DEFAULT);
    EXPECT_EQ(ret, PowerErrors::ERR_OK) << "Test case 3 failed";
    ret = pmsTest_->UnRegisterAsyncShutdownCallback(callback0);
    EXPECT_EQ(ret, PowerErrors::ERR_OK) << "Test case 3 failed";

    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest::PowerMgrServiceNative022 end!");
}
} // namespace
