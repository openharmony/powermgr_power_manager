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

#include "power_mgr_service_native_test.h"

#ifdef THERMAL_GTEST
#define private   public
#define protected public
#endif

#include <csignal>
#include <iostream>

#include "key_event.h"
#include "pointer_event.h"
#include "power_common.h"
#include "power_mgr_service.h"
#include "setting_helper.h"
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
sptr<PowerMgrService> g_pmsTest;
} // namespace

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
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative001, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative001 begin.");
    g_pmsTest->HallSensorSubscriberInit();
    g_pmsTest->HallSensorSubscriberCancel();

    int32_t keyCode = OHOS::MMI::KeyEvent::KEYCODE_F1;
    g_pmsTest->HandleKeyEvent(keyCode);
    int32_t type = OHOS::MMI::PointerEvent::SOURCE_TYPE_MOUSE;
    g_pmsTest->HandlePointEvent(type);

    EXPECT_EQ(g_pmsTest->OverrideScreenOffTime(TIMEOUTMS), PowerErrors::ERR_OK);
    EXPECT_EQ(g_pmsTest->RestoreScreenOffTime(), PowerErrors::ERR_OK);

    g_pmsTest->doubleClickId_ = UNCANCELID;
    g_pmsTest->monitorId_ = UNCANCELID;
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative001 end.");
}

/**
 * @tc.name: PowerMgrServiceNative002
 * @tc.desc: test handleKeyEvent in powerMgrService
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative002, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative002 begin.");
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
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative002 end.");
}

/**
 * @tc.name: PowerMgrServiceNative003
 * @tc.desc: test pickup register
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative003, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative003 begin.");
    g_pmsTest->RegisterSettingWakeupPickupGestureObserver();
    g_pmsTest->RegisterSettingWakeupPickupGestureObserver();
    EXPECT_TRUE(SettingHelper::pickUpObserver_ != nullptr);
    SettingHelper::UnregisterSettingWakeupPickupObserver();
    SettingHelper::UnregisterSettingWakeupPickupObserver();
    EXPECT_TRUE(SettingHelper::pickUpObserver_ == nullptr);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative003 end.");
}

/**
 * @tc.name: PowerMgrServiceNative004
 * @tc.desc: test autobrightness register
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative004, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative004 begin.");
    shared_ptr<PowerModeModule> powerModeModuleTest = make_shared<PowerModeModule>();
    EXPECT_TRUE(powerModeModuleTest != nullptr);
    auto flag = SettingHelper::IsAutoAdjustBrightnessSettingValid();
    powerModeModuleTest->RegisterAutoAdjustBrightnessObserver();
    SettingHelper::SetSettingAutoAdjustBrightness(SettingHelper::SwitchStatus::INVALID);
    auto ret = SettingHelper::GetSettingAutoAdjustBrightness(INVALID_CODE);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative004 end.");
}

/**
 * @tc.name: PowerMgrServiceNative005
 * @tc.desc: test brightness register
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative005, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative005 begin.");
    shared_ptr<PowerModeModule> powerModeModuleTest = make_shared<PowerModeModule>();
    EXPECT_TRUE(powerModeModuleTest != nullptr);
    powerModeModuleTest->RegisterAutoAdjustBrightnessObserver();
    auto flag = SettingHelper::IsBrightnessSettingValid();
    SettingHelper::SetSettingBrightness(INVALID_CODE);
    auto ret = SettingHelper::GetSettingAutoAdjustBrightness(INVALID_CODE);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative005 end.");
}

/**
 * @tc.name: PowerMgrServiceNative006
 * @tc.desc: test vibrateState register
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative006, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative006 begin.");
    shared_ptr<PowerModeModule> powerModeModuleTest = make_shared<PowerModeModule>();
    EXPECT_TRUE(powerModeModuleTest != nullptr);
    powerModeModuleTest->RegisterVibrateStateObserver();
    auto flag = SettingHelper::IsVibrationSettingValid();
    SettingHelper::SetSettingVibration(SettingHelper::SwitchStatus::INVALID);
    auto ret = SettingHelper::GetSettingVibration(INVALID_CODE);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative006 end.");
}


/**
 * @tc.name: PowerMgrServiceNative007
 * @tc.desc: test autoWindowRotation register
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative007, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative007 begin.");
    shared_ptr<PowerModeModule> powerModeModuleTest = make_shared<PowerModeModule>();
    EXPECT_TRUE(powerModeModuleTest != nullptr);
    powerModeModuleTest->RegisterAutoWindowRotationObserver();
    auto flag = SettingHelper::IsWindowRotationSettingValid();
    SettingHelper::SetSettingWindowRotation(SettingHelper::SwitchStatus::INVALID);
    auto ret = SettingHelper::GetSettingWindowRotation(INVALID_CODE);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative007 end.");
}

/**
 * @tc.name: PowerMgrServiceNative008
 * @tc.desc: test intellVoiceSetting register
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative008, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative008 begin.");
    shared_ptr<PowerModeModule> powerModeModuleTest = make_shared<PowerModeModule>();
    EXPECT_TRUE(powerModeModuleTest != nullptr);
    powerModeModuleTest->RegisterIntellVoiceObserver();
    auto flag = SettingHelper::IsIntellVoiceSettingValid();
    SettingHelper::SetSettingIntellVoice(SettingHelper::SwitchStatus::INVALID);
    auto ret = SettingHelper::GetSettingIntellVoice(INVALID_CODE);
    powerModeModuleTest->UnregisterSaveModeObserver();
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative008 end.");
}

/**
 * @tc.name: PowerMgrServiceNative009
 * @tc.desc: test settingPowerMode register
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative009, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative009 begin.");
    g_pmsTest->RegisterSettingPowerModeObservers();
    g_pmsTest->RegisterSettingPowerModeObservers();
    EXPECT_TRUE(SettingHelper::powerModeObserver_ != nullptr);
    SettingHelper::SaveCurrentMode(INVALID_CODE);
    SettingHelper::UnRegisterSettingPowerModeObserver();
    EXPECT_TRUE(SettingHelper::powerModeObserver_ == nullptr);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative009 end.");
}

/**
 * @tc.name: PowerMgrServiceNative010
 * @tc.desc: test settingWakeUpLid register
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative010, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative010 begin.");
    g_pmsTest->RegisterSettingWakeUpLidObserver();
    SettingHelper::SetSettingWakeupLid(true);
    bool ret = SettingHelper::GetSettingWakeupLid();
    SettingHelper::UnRegisterSettingWakeupLidObserver();
    EXPECT_TRUE(SettingHelper::lidObserver_ == nullptr);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative010 end.");
}

/**
 * @tc.name: PowerMgrServiceNative011
 * @tc.desc: test RegisterSettingWakeupDoubleClickObservers
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative011, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative011 begin.");
    g_pmsTest->RegisterSettingWakeupDoubleClickObservers();
    g_pmsTest->RegisterSettingWakeupDoubleClickObservers();
    SettingHelper::IsWakeupPickupSettingValid();
    SettingHelper::UnregisterSettingWakeupDoubleObserver();
    SettingHelper::UnregisterSettingWakeupDoubleObserver();
    EXPECT_TRUE(SettingHelper::doubleClickObserver_ == nullptr);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative011 end.");
}

} // namespace
