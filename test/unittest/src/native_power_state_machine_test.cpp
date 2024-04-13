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

#include "native_power_state_machine_test.h"

#include <ipc_skeleton.h>

#include "actions/irunning_lock_action.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

void NativePowerStateMachineTest::SetUpTestCase() {}

void PowerStateTest1Callback::OnPowerStateChanged(PowerState state)
{
    POWER_HILOGI(LABEL_TEST, "PowerStateTest1Callback::OnPowerStateChanged state = %u.", static_cast<uint32_t>(state));
}

void PowerStateTest2Callback::OnPowerStateChanged(PowerState state)
{
    POWER_HILOGI(LABEL_TEST, "PowerStateTest1Callback::OnPowerStateChanged state = %u.", static_cast<uint32_t>(state));
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
HWTEST_F(NativePowerStateMachineTest, NativePowerStateMachine001, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "NativePowerStateMachine001::fun is start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    pmsTest->OnStart();
    auto stateMachine = std::make_shared<PowerStateMachine>(pmsTest);
    EXPECT_TRUE(stateMachine->Init());
    EXPECT_TRUE(stateMachine->Init());
    stateMachine->InitState();
    EXPECT_FALSE(stateMachine->CheckRefreshTime());
    EXPECT_TRUE(stateMachine->CheckRefreshTime());
    EXPECT_FALSE(stateMachine->RestoreScreenOffTimeInner());
    EXPECT_TRUE(stateMachine->OverrideScreenOffTimeInner(TIMEOUT));
    EXPECT_TRUE(stateMachine->OverrideScreenOffTimeInner(TIMEOUT));
    stateMachine->SetDisplayOffTime(TIME, true);
    stateMachine->ResetInactiveTimer();
    std::string result;
    stateMachine->DumpInfo(result);
    EXPECT_TRUE(stateMachine->RestoreScreenOffTimeInner());
    stateMachine->onWakeup();
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
HWTEST_F(NativePowerStateMachineTest, NativePowerStateMachine002, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "NativePowerStateMachine002::fun is start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    auto stateMachine = std::make_shared<PowerStateMachine>(pmsTest);
    EXPECT_TRUE(stateMachine->Init());
    stateMachine->ReceiveScreenEvent(true);
    stateMachine->ReceiveScreenEvent(false);
    sptr<IPowerStateCallback> cb = new PowerStateTest1Callback();
    stateMachine->RegisterPowerStateCallback(cb);
    stateMachine->SetState(PowerState::INACTIVE, StateChangeReason::STATE_CHANGE_REASON_RUNNING_LOCK, true);
    EXPECT_TRUE(stateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_RUNNING_LOCK, true));
    stateMachine->UnRegisterPowerStateCallback(cb);
    sptr<IPowerStateCallback> cb1 = new PowerStateTest2Callback();
    stateMachine->UnRegisterPowerStateCallback(cb1);
    sptr<IPowerStateCallback> cb2 = nullptr;
    stateMachine->RegisterPowerStateCallback(cb2);
    stateMachine->UnRegisterPowerStateCallback(cb2);
    stateMachine->SetState(PowerState::INACTIVE, StateChangeReason::STATE_CHANGE_REASON_TIMEOUT, true);
    EXPECT_TRUE(stateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_RUNNING_LOCK, true));

    POWER_HILOGI(LABEL_TEST, "NativePowerStateMachine002::fun is end!");
    GTEST_LOG_(INFO) << "NativePowerStateMachine002: Suspend Device end.";
}

/**
 * @tc.name: NativePowerStateMachine003
 * @tc.desc: test suspendDeviceInner in powerStateMachine
 * @tc.type: FUNC
 */
HWTEST_F(NativePowerStateMachineTest, NativePowerStateMachine003, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "NativePowerStateMachine003::fun is start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    pmsTest->OnStart();
    auto stateMachine = std::make_shared<PowerStateMachine>(pmsTest);
    EXPECT_TRUE(stateMachine->Init());
    int32_t powermsEvent = PowerStateMachine::CHECK_USER_ACTIVITY_OFF_TIMEOUT_MSG;
    stateMachine->SetDelayTimer(stateMachine->GetDisplayOffTime() / THREE, powermsEvent);

    SuspendDeviceType suspendDeviceType = SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION;
    stateMachine->SuspendDeviceInner(PID, CALLTIMEMS, suspendDeviceType, true, false);
    suspendDeviceType = SuspendDeviceType::SUSPEND_DEVICE_REASON_DEVICE_ADMIN;
    stateMachine->SuspendDeviceInner(PID, CALLTIMEMS, suspendDeviceType, true, true);
    suspendDeviceType = SuspendDeviceType::SUSPEND_DEVICE_REASON_TIMEOUT;
    stateMachine->SuspendDeviceInner(PID, CALLTIMEMS, suspendDeviceType, true, false);
    suspendDeviceType = SuspendDeviceType::SUSPEND_DEVICE_REASON_LID;
    stateMachine->SuspendDeviceInner(PID, CALLTIMEMS, suspendDeviceType, true, false);
    suspendDeviceType = SuspendDeviceType::SUSPEND_DEVICE_REASON_SLEEP_KEY;
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
 * @tc.name: NativePowerStateMachine005
 * @tc.desc: test refreshActivityInner and wakeupDeviceInner in powerMgrService
 * @tc.type: FUNC
 */
HWTEST_F(NativePowerStateMachineTest, NativePowerStateMachine005, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "NativePowerStateMachine005::fun is start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    UserActivityType userActivityType = UserActivityType::USER_ACTIVITY_TYPE_OTHER;
    auto stateMachine = std::make_shared<PowerStateMachine>(pmsTest);
    EXPECT_TRUE(stateMachine->Init());
    stateMachine->RefreshActivityInner(PID, CALLTIMEMS, userActivityType, true);
    stateMachine->RefreshActivityInner(PID, CALLTIMEMS, static_cast<UserActivityType>(MAXTYPE), true);
    bool ret = stateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_RUNNING_LOCK, true);
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
HWTEST_F(NativePowerStateMachineTest, NativePowerStateMachine006, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "NativePowerStateMachine006::fun is start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();

    auto stateMachine = std::make_shared<PowerStateMachine>(pmsTest);
    EXPECT_TRUE(stateMachine->Init());

    auto stateMachineController = std::make_shared<PowerStateMachine::StateController>(
        PowerState::INACTIVE, stateMachine, TransitResultToStateChangeReason);
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

    auto stateMachineController2 = std::make_shared<PowerStateMachine::StateController>(
        PowerState::INACTIVE, nullptr, TransitResultToStateChangeReason);
    EXPECT_TRUE(stateMachineController2->TransitTo(trigger, false) == TransitResult::OTHER_ERR);
    EXPECT_FALSE(stateMachineController2->CheckState());

    POWER_HILOGI(LABEL_TEST, "NativePowerStateMachine006::fun is end!");
    GTEST_LOG_(INFO) << "NativePowerStateMachine006: Suspend Device end.";
}

/**
 * @tc.name: NativePowerStateMachine007
 * @tc.desc: test recordFailure in powerStateMachine
 * @tc.type: FUNC
 */
HWTEST_F(NativePowerStateMachineTest, NativePowerStateMachine007, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "NativePowerStateMachine007::fun is start!");
    auto stateMachine = std::make_shared<PowerStateMachine>(nullptr);
    EXPECT_TRUE(stateMachine->Init());
    EXPECT_FALSE(stateMachine->CheckRunningLock(PowerState::INACTIVE));

    auto stateMachineController = std::make_shared<PowerStateMachine::StateController>(
        PowerState::INACTIVE, stateMachine, TransitResultToStateChangeReason);
    StateChangeReason trigger = StateChangeReason::STATE_CHANGE_REASON_BATTERY;
    EXPECT_TRUE(stateMachineController->TransitTo(trigger, false) == TransitResult::LOCKING);
    stateMachine->currentState_ = PowerState::INACTIVE;
    stateMachine->SetDisplaySuspend(false);
    stateMachine->SetSleepTime(TIME);
    SuspendDeviceType suspendDeviceType = SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION;
    stateMachine->SuspendDeviceInner(PID, CALLTIMEMS, suspendDeviceType, true, false);

    POWER_HILOGI(LABEL_TEST, "NativePowerStateMachine007::fun is end!");
    GTEST_LOG_(INFO) << "NativePowerStateMachine007: Suspend Device end.";
}

/**
 * @tc.name: NativePowerStateMachine008
 * @tc.desc: test init in powerStateMachine
 * @tc.type: FUNC
 */
HWTEST_F(NativePowerStateMachineTest, NativePowerStateMachine008, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "NativePowerStateMachine008::fun is start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    pmsTest->OnStart();
    auto stateMachine = std::make_shared<PowerStateMachine>(pmsTest);
    EXPECT_TRUE(stateMachine->Init());

    bool ret = stateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_BATTERY, true);
    EXPECT_TRUE(ret);
    stateMachine->HandleSystemWakeup();
    stateMachine->HandleActivitySleepTimeout();
    ret = stateMachine->SetState(PowerState::INACTIVE, StateChangeReason::STATE_CHANGE_REASON_BATTERY, true);
    EXPECT_TRUE(ret);
    stateMachine->HandleSystemWakeup();

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo infoInactive("test1", RunningLockType::RUNNINGLOCK_SCREEN);
    pmsTest->CreateRunningLock(token, infoInactive);
    pmsTest->Lock(token);
    EXPECT_EQ(pmsTest->IsUsed(token), true);
    stateMachine->HandleActivityTimeout();

    pmsTest->UnLock(token);
    EXPECT_EQ(pmsTest->IsUsed(token), false);

    pmsTest->UnLock(token);
    EXPECT_EQ(pmsTest->IsUsed(token), false);
    POWER_HILOGI(LABEL_TEST, "NativePowerStateMachine008::fun is end!");
    GTEST_LOG_(INFO) << "NativePowerStateMachine008: Suspend Device end.";
}
/**
 * @tc.name: NativePowerStateMachine009
 * @tc.desc: test duration of DIM state
 * @tc.type: FUNC
 */
HWTEST_F(NativePowerStateMachineTest, NativePowerStateMachine009, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "NativePowerStateMachine009: func started!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    pmsTest->OnStart();
    auto stateMachine = pmsTest->GetPowerStateMachine();
    stateMachine->OverrideScreenOffTimeInner(10 * 1000);
    auto displayOffTime = stateMachine->GetDisplayOffTime();
    EXPECT_EQ(displayOffTime, 10 * 1000);
    EXPECT_EQ(stateMachine->GetDimTime(displayOffTime), displayOffTime / PowerStateMachine::OFF_TIMEOUT_FACTOR);

    stateMachine->OverrideScreenOffTimeInner(60 * 1000);
    displayOffTime = stateMachine->GetDisplayOffTime();
    EXPECT_EQ(displayOffTime, 60 * 1000);
    EXPECT_EQ(stateMachine->GetDimTime(displayOffTime), PowerStateMachine::MAX_DIM_TIME_MS);

    POWER_HILOGI(LABEL_TEST, "NativePowerStateMachine009: func ended!");
}
} // namespace
