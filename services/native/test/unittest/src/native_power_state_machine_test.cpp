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

#include "native_power_state_machine_test.h"

#include <ipc_skeleton.h>

#include "actions/irunning_lock_action.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

void NativePowerStateMachineTest::SetUpTestCase()
{
}

void PowerStateTest1Callback::OnPowerStateChanged(PowerState state)
{
    POWER_HILOGI(LABEL_TEST, "PowerStateTest1Callback::OnPowerStateChanged state = %u.",
        static_cast<uint32_t>(state));
}

void PowerStateTest2Callback::OnPowerStateChanged(PowerState state)
{
    POWER_HILOGI(LABEL_TEST, "PowerStateTest1Callback::OnPowerStateChanged state = %u.",
        static_cast<uint32_t>(state));
}

namespace {
TransitResult TransitResultToStateChangeReason(StateChangeReason trigger)
{
    return TransitResult::ALREADY_IN_STATE;
}

/**
 * @tc.name: NativePowerStateMachine001
 * @tc.desc: test init in powerStateMachine
 * @tc.type: FUNC
 */
HWTEST_F (NativePowerStateMachineTest, NativePowerStateMachine001, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "NativePowerStateMachine001::fun is start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    auto stateMachine = std::make_shared<PowerStateMachine>(pmsTest);
    EXPECT_TRUE(stateMachine->Init());
    EXPECT_TRUE(stateMachine->Init());
    stateMachine->InitState();
    EXPECT_FALSE(stateMachine->CheckRefreshTime());
    EXPECT_TRUE(stateMachine->CheckRefreshTime());
    EXPECT_FALSE(stateMachine->RestoreScreenOffTimeInner());
    EXPECT_TRUE(stateMachine->OverrideScreenOffTimeInner(7));
    EXPECT_TRUE(stateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_BATTERY, true));
    stateMachine->SetDisplayOffTime(TIME, true);
    stateMachine->ResetInactiveTimer();
    std::string result;
    stateMachine->DumpInfo(result);
    EXPECT_TRUE(stateMachine->RestoreScreenOffTimeInner());
    stateMachine->onWakeup();
    EXPECT_TRUE(stateMachine->ForceSuspendDeviceInner(PID, CALLTIMEMS));
    stateMachine->SetSleepTime(TIME);
    stateMachine->ResetSleepTimer();

    bool ret = stateMachine->SetState(PowerState::UNKNOWN, StateChangeReason::STATE_CHANGE_REASON_BATTERY, true);
    EXPECT_FALSE(ret);
    stateMachine->SetDisplaySuspend(true);
    stateMachine->SetDisplaySuspend(false);

    POWER_HILOGI(LABEL_TEST, "NativePowerStateMachine001::fun is end!");
    GTEST_LOG_(INFO) << "NativePowerStateMachine001: Suspend Device end.";
}

/**
 * @tc.name: NativePowerStateMachine002
 * @tc.desc: test callback in powerStateMachine
 * @tc.type: FUNC
 */
HWTEST_F (NativePowerStateMachineTest, NativePowerStateMachine002, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "NativePowerStateMachine002::fun is start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    auto stateMachine = std::make_shared<PowerStateMachine>(pmsTest);
    EXPECT_TRUE(stateMachine->Init());
    stateMachine->ReceiveScreenEvent(true);
    stateMachine->ReceiveScreenEvent(false);
    sptr<IPowerStateCallback> cb = new PowerStateTest1Callback();
    stateMachine->RegisterPowerStateCallback(cb);
    EXPECT_TRUE(stateMachine->SetState(PowerState::AWAKE,
                  StateChangeReason::STATE_CHANGE_REASON_RUNNING_LOCK, true));
    stateMachine->UnRegisterPowerStateCallback(cb);
    sptr<IPowerStateCallback> cb1 = new PowerStateTest2Callback();
    stateMachine->UnRegisterPowerStateCallback(cb1);
    sptr<IPowerStateCallback> cb2 = nullptr;
    stateMachine->RegisterPowerStateCallback(cb2);
    stateMachine->UnRegisterPowerStateCallback(cb2);

    EXPECT_TRUE(stateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_TIMEOUT, true));
    int32_t powermsEvent = PowermsEventHandler::CHECK_USER_ACTIVITY_TIMEOUT_MSG;
    stateMachine->HandleDelayTimer(powermsEvent);
    powermsEvent = PowermsEventHandler::CHECK_USER_ACTIVITY_OFF_TIMEOUT_MSG;
    stateMachine->HandleDelayTimer(powermsEvent);
    powermsEvent = PowermsEventHandler::CHECK_USER_ACTIVITY_SLEEP_TIMEOUT_MSG;
    stateMachine->HandleDelayTimer(powermsEvent);

    POWER_HILOGI(LABEL_TEST, "NativePowerStateMachine002::fun is end!");
    GTEST_LOG_(INFO) << "NativePowerStateMachine002: Suspend Device end.";
}

/**
 * @tc.name: NativePowerStateMachine003
 * @tc.desc: test suspendDeviceInner in powerStateMachine
 * @tc.type: FUNC
 */
HWTEST_F (NativePowerStateMachineTest, NativePowerStateMachine003, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "NativePowerStateMachine003::fun is start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    auto stateMachine = std::make_shared<PowerStateMachine>(pmsTest);
    EXPECT_TRUE(stateMachine->Init());
    int32_t powermsEvent = PowermsEventHandler::CHECK_USER_ACTIVITY_OFF_TIMEOUT_MSG;
    stateMachine->SetDelayTimer(stateMachine->GetDisplayOffTime() / THREE, powermsEvent);

    SuspendDeviceType suspendDeviceType = SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION;
    stateMachine->SuspendDeviceInner(PID, CALLTIMEMS, suspendDeviceType, true, false);
    suspendDeviceType = SuspendDeviceType::SUSPEND_DEVICE_REASON_DEVICE_ADMIN;
    stateMachine->SuspendDeviceInner(PID, CALLTIMEMS, suspendDeviceType, true, true);
    suspendDeviceType = SuspendDeviceType::SUSPEND_DEVICE_REASON_TIMEOUT;
    stateMachine->SuspendDeviceInner(PID, CALLTIMEMS, suspendDeviceType, true, false);
    suspendDeviceType = SuspendDeviceType::SUSPEND_DEVICE_REASON_LID_SWITCH;
    stateMachine->SuspendDeviceInner(PID, CALLTIMEMS, suspendDeviceType, true, false);
    suspendDeviceType = SuspendDeviceType::SUSPEND_DEVICE_REASON_SLEEP_BUTTON;
    stateMachine->SuspendDeviceInner(PID, CALLTIMEMS, suspendDeviceType, true, false);
    suspendDeviceType = SuspendDeviceType::SUSPEND_DEVICE_REASON_HDMI;
    stateMachine->SuspendDeviceInner(PID, CALLTIMEMS, suspendDeviceType, true, false);
    suspendDeviceType = SuspendDeviceType::SUSPEND_DEVICE_REASON_ACCESSIBILITY;
    stateMachine->SuspendDeviceInner(PID, CALLTIMEMS, suspendDeviceType, true, false);
    suspendDeviceType = static_cast<SuspendDeviceType>(MAXTYPE);
    stateMachine->SuspendDeviceInner(PID, CALLTIMEMS, suspendDeviceType, true, false);

    stateMachine->RegisterDisplayOffTimeObserver();
    stateMachine->RegisterDisplayOffTimeObserver();
    stateMachine->UnregisterDisplayOffTimeObserver();
    stateMachine->UnregisterDisplayOffTimeObserver();

    std::string result;
    stateMachine->DumpInfo(result);

    POWER_HILOGI(LABEL_TEST, "NativePowerStateMachine003::fun is end!");
    GTEST_LOG_(INFO) << "NativePowerStateMachine003: Suspend Device end.";
}

/**
 * @tc.name: NativePowerStateMachine004
 * @tc.desc: test setDisplaySuspend and handleDelayTimer in powerMgrService
 * @tc.type: FUNC
 */
HWTEST_F (NativePowerStateMachineTest, NativePowerStateMachine004, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "NativePowerStateMachine004::fun is start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    pmsTest->OnStart();
    auto stateMachine = std::make_shared<PowerStateMachine>(pmsTest);
    int32_t powermsEvent = PowermsEventHandler::CHECK_USER_ACTIVITY_SLEEP_TIMEOUT_MSG;
    EXPECT_TRUE(stateMachine->Init());
    stateMachine->SetDisplaySuspend(false);
    stateMachine->InitState();
    stateMachine->SetDisplaySuspend(true);
    UserActivityType userActivityType = UserActivityType::USER_ACTIVITY_TYPE_ACCESSIBILITY;
    stateMachine->RefreshActivityInner(PID, CALLTIMEMS, userActivityType, true);
    EXPECT_TRUE(stateMachine->SetState(PowerState::INACTIVE, StateChangeReason::STATE_CHANGE_REASON_WORK, true));
    EXPECT_TRUE(stateMachine->SetState(PowerState::SLEEP, StateChangeReason::STATE_CHANGE_REASON_THERMAL, true));
    stateMachine->SetDisplaySuspend(false);
    EXPECT_TRUE(stateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_INIT, true));
    stateMachine->HandleDelayTimer(powermsEvent);
    powermsEvent = PowermsEventHandler::CHECK_USER_ACTIVITY_OFF_TIMEOUT_MSG;
    stateMachine->HandleDelayTimer(powermsEvent);
    powermsEvent = PowermsEventHandler::CHECK_USER_ACTIVITY_TIMEOUT_MSG;
    stateMachine->HandleDelayTimer(powermsEvent);
    powermsEvent = MAXTYPE;
    stateMachine->HandleDelayTimer(powermsEvent);
    powermsEvent = PowermsEventHandler::SYSTEM_WAKE_UP_MSG;
    stateMachine->HandleDelayTimer(powermsEvent);

    bool ret = stateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_SETTINGS, true);
    EXPECT_TRUE(ret);
    stateMachine->InitState();
    stateMachine->HandleDelayTimer(powermsEvent);

    POWER_HILOGI(LABEL_TEST, "NativePowerStateMachine004::fun is end!");
    GTEST_LOG_(INFO) << "NativePowerStateMachine004: Suspend Device end.";
}

/**
 * @tc.name: NativePowerStateMachine005
 * @tc.desc: test refreshActivityInner and wakeupDeviceInner in powerMgrService
 * @tc.type: FUNC
 */
HWTEST_F (NativePowerStateMachineTest, NativePowerStateMachine005, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "NativePowerStateMachine005::fun is start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    UserActivityType userActivityType = UserActivityType::USER_ACTIVITY_TYPE_OTHER;
    auto stateMachine = std::make_shared<PowerStateMachine>(pmsTest);
    EXPECT_TRUE(stateMachine->Init());
    stateMachine->RefreshActivityInner(PID, CALLTIMEMS, userActivityType, true);
    stateMachine->RefreshActivityInner(PID, CALLTIMEMS, static_cast<UserActivityType>(MAXTYPE), true);
    bool ret = stateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_TIMEOUT, true);
    EXPECT_TRUE(ret);
    stateMachine->RefreshActivityInner(PID, CALLTIMEMS, userActivityType, true);
    userActivityType = UserActivityType::USER_ACTIVITY_TYPE_TOUCH;
    stateMachine->RefreshActivityInner(PID, CALLTIMEMS, userActivityType, true);
    userActivityType = UserActivityType::USER_ACTIVITY_TYPE_SOFTWARE;
    stateMachine->RefreshActivityInner(PID, CALLTIMEMS, userActivityType, true);
    userActivityType = UserActivityType::USER_ACTIVITY_TYPE_BUTTON;
    stateMachine->RefreshActivityInner(PID, CALLTIMEMS, userActivityType, true);

    WakeupDeviceType type = WakeupDeviceType::WAKEUP_DEVICE_APPLICATION;
    stateMachine->WakeupDeviceInner(PID, CALLTIMEMS, type, "7", "7");
    type = WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON;
    stateMachine->WakeupDeviceInner(PID, CALLTIMEMS, type, "7", "7");
    type = WakeupDeviceType::WAKEUP_DEVICE_WAKE_KEY;
    stateMachine->WakeupDeviceInner(PID, CALLTIMEMS, type, "7", "7");
    type = WakeupDeviceType::WAKEUP_DEVICE_HDMI;
    stateMachine->WakeupDeviceInner(PID, CALLTIMEMS, type, "7", "7");
    type = WakeupDeviceType::WAKEUP_DEVICE_GESTURE;
    stateMachine->WakeupDeviceInner(PID, CALLTIMEMS, type, "7", "7");
    type = WakeupDeviceType::WAKEUP_DEVICE_CAMERA_LAUNCH;
    stateMachine->WakeupDeviceInner(PID, CALLTIMEMS, type, "7", "7");
    type = WakeupDeviceType::WAKEUP_DEVICE_WAKE_MOTION;
    stateMachine->WakeupDeviceInner(PID, CALLTIMEMS, type, "7", "7");
    type = WakeupDeviceType::WAKEUP_DEVICE_LID;
    stateMachine->WakeupDeviceInner(PID, CALLTIMEMS, type, "7", "7");
    type = WakeupDeviceType::WAKEUP_DEVICE_DOUBLE_CLICK;
    stateMachine->WakeupDeviceInner(PID, CALLTIMEMS, type, "7", "7");
    type = WakeupDeviceType::WAKEUP_DEVICE_KEYBOARD;
    stateMachine->WakeupDeviceInner(PID, CALLTIMEMS, type, "7", "7");
    type = WakeupDeviceType::WAKEUP_DEVICE_MOUSE;
    stateMachine->WakeupDeviceInner(PID, CALLTIMEMS, type, "7", "7");
    type = WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN;
    stateMachine->WakeupDeviceInner(PID, CALLTIMEMS, type, "7", "7");
    stateMachine->WakeupDeviceInner(PID, CALLTIMEMS, static_cast<WakeupDeviceType>(MAXTYPE), "7", "7");

    POWER_HILOGI(LABEL_TEST, "NativePowerStateMachine005::fun is end!");
    GTEST_LOG_(INFO) << "NativePowerStateMachine005: Suspend Device end.";
}

/**
 * @tc.name: NativePowerStateMachine006
 * @tc.desc: test recordFailure in powerStateMachine
 * @tc.type: FUNC
 */
HWTEST_F (NativePowerStateMachineTest, NativePowerStateMachine006, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "NativePowerStateMachine006::fun is start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();

    auto stateMachine = std::make_shared<PowerStateMachine>(pmsTest);
    EXPECT_TRUE(stateMachine->Init());

    auto stateMachineController = std::make_shared<PowerStateMachine::StateController>(PowerState::INACTIVE,
                                    stateMachine, TransitResultToStateChangeReason);
    StateChangeReason trigger = StateChangeReason::STATE_CHANGE_REASON_BATTERY;
    TransitResult failReason = TransitResult::LOCKING;
    stateMachineController->RecordFailure(PowerState::INACTIVE, trigger, failReason);
    failReason = TransitResult::HDI_ERR;
    stateMachineController->RecordFailure(PowerState::INACTIVE, trigger, failReason);
    failReason = TransitResult::DISPLAY_ON_ERR;
    stateMachineController->RecordFailure(PowerState::INACTIVE, trigger, failReason);
    failReason = TransitResult::DISPLAY_OFF_ERR;
    stateMachineController->RecordFailure(PowerState::INACTIVE, trigger, failReason);
    failReason = TransitResult::OTHER_ERR;
    stateMachineController->RecordFailure(PowerState::INACTIVE, trigger, failReason);

    EXPECT_TRUE(stateMachineController->TransitTo(trigger, false) == TransitResult::ALREADY_IN_STATE);
    stateMachine->SetDisplayOffTime(TIME, false);

    POWER_HILOGI(LABEL_TEST, "NativePowerStateMachine006::fun is end!");
    GTEST_LOG_(INFO) << "NativePowerStateMachine006: Suspend Device end.";
}
}
