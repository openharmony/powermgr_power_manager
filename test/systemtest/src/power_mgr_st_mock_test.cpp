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

#include "power_mgr_st_mock_test.h"
#include "power_mgr_client.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;
using ::testing::_;

static sptr<PowerMgrService> g_service;
static MockStateAction* g_shutdownState;
static MockStateAction* g_stateAction;
static MockPowerAction* g_powerAction;
static MockLockAction* g_lockAction;

static void ResetMockAction()
{
    POWER_HILOGI(LABEL_TEST, "ResetMockAction:Start");
    g_stateAction = new MockStateAction();
    g_shutdownState = new MockStateAction();
    g_powerAction = new MockPowerAction();
    g_lockAction = new MockLockAction();
    g_service->EnableMock(g_stateAction, g_shutdownState, g_powerAction, g_lockAction);
}

void PowerMgrSTMockTest::SetUpTestCase(void)
{
    // create singleton service object at the beginning
    g_service = DelayedSpSingleton<PowerMgrService>::GetInstance();
    g_service->OnStart();
    ResetMockAction();
}

void PowerMgrSTMockTest::TearDownTestCase(void)
{
    g_service->OnStop();
    DelayedSpSingleton<PowerMgrService>::DestroyInstance();
}

void PowerMgrSTMockTest::SetUp(void)
{
    sleep(SLEEP_WAIT_TIME_S + ONE_SECOND);
}

void PowerMgrSTMockTest::TearDown(void)
{
    ResetMockAction();
}

namespace {
constexpr int32_t TEST_TIMES = 1000;
/**
 * @tc.name: PowerMgrMock001
 * @tc.desc: test RebootDevice by mock
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrSTMockTest, PowerMgrMock001, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerMgrMock001: start";
    POWER_HILOGI(LABEL_TEST, "PowerMgrMock001:Start");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock001: Failed to get PowerMgrService";
    }

    EXPECT_CALL(*g_powerAction, Reboot(std::string("test"))).Times(1);
    pms->RebootDevice(std::string("test"));

    POWER_HILOGI(LABEL_TEST, "PowerMgrMock001:End");
    GTEST_LOG_(INFO) << "PowerMgrMock001: end";
    usleep(SLEEP_WAIT_TIME_MS * TRANSFER_NS_TO_MS);
}

/**
 * @tc.name: PowerMgrMock002
 * @tc.desc: test ShutDownDevice by mock
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrSTMockTest, PowerMgrMock002, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerMgrMock002: start";
    POWER_HILOGI(LABEL_TEST, "PowerMgrMock002:Start");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock002: Failed to get PowerMgrService";
    }

    EXPECT_CALL(*g_powerAction, Shutdown(std::string("test"))).Times(1);
    pms->ShutDownDevice(std::string("test"));

    POWER_HILOGI(LABEL_TEST, "PowerMgrMock002:End");
    GTEST_LOG_(INFO) << "PowerMgrMock002: end";
    usleep(SLEEP_WAIT_TIME_MS * TRANSFER_NS_TO_MS);
}

/**
 * @tc.name: PowerMgrMock029
 * @tc.desc: test ForceSuspendDevice by mock
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrSTMockTest, PowerMgrMock029, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerMgrMock029: start";
    POWER_HILOGI(LABEL_TEST, "PowerMgrMock029:Start");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock029: Failed to get PowerMgrService";
    }

    POWER_HILOGI(LABEL_TEST, "PowerMgrMock029:forcesuspend");
    EXPECT_EQ(pms->ForceSuspendDevice(0), true);

    POWER_HILOGI(LABEL_TEST, "PowerMgrMock029:End");
    GTEST_LOG_(INFO) << "PowerMgrMock029: end";
}

/**
 * @tc.name: PowerMgrMock030
 * @tc.desc: test RunningLock by mock
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrSTMockTest, PowerMgrMock030, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerMgrMock030: start";
    POWER_HILOGI(LABEL_TEST, "PowerMgrMock030:Start");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock030: Failed to get PowerMgrService";
    }

    pms->SetDisplayOffTime(SET_DISPLAY_OFF_TIME_MS);
    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_SCREEN);
    pms->CreateRunningLock(token, info);
    pms->Lock(token);
    EXPECT_EQ(pms->IsUsed(token), true);
    pms->UnLock(token);
    EXPECT_EQ(pms->IsUsed(token), false);

    pms->SetDisplayOffTime(DEFAULT_DISPLAY_OFF_TIME);
    POWER_HILOGI(LABEL_TEST, "PowerMgrMock030:End");
    GTEST_LOG_(INFO) << "PowerMgrMock030: end";
}

/**
 * @tc.name: PowerMgrMock032
 * @tc.desc: test proximity RunningLock by mock
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrSTMockTest, PowerMgrMock032, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerMgrMock032: start";
    POWER_HILOGI(LABEL_TEST, "PowerMgrMock032:Start");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock032: Failed to get PowerMgrService";
    }

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL);
    pms->CreateRunningLock(token, info);

    pms->Lock(token);
    EXPECT_EQ(pms->IsUsed(token), true);
    sleep(SCREEN_DIM_WAIT_TIME_S + ONE_SECOND);

    pms->UnLock(token);
    EXPECT_EQ(pms->IsUsed(token), false);

    POWER_HILOGI(LABEL_TEST, "PowerMgrMock032:End");
    GTEST_LOG_(INFO) << "PowerMgrMock032: end";
}

/**
 * @tc.name: PowerMgrMock054
 * @tc.desc: test ForceSuspendDevice by mock during Inactive
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrSTMockTest, PowerMgrMock054, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerMgrMock054: start";
    POWER_HILOGI(LABEL_TEST, "PowerMgrMock054:Start");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock054: Failed to get PowerMgrService";
    }

    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    pms->ForceSuspendDevice(0);
    sleep(SLEEP_WAIT_TIME_S + ONE_SECOND);

    POWER_HILOGI(LABEL_TEST, "PowerMgrMock054:End");
    GTEST_LOG_(INFO) << "PowerMgrMock054: end";
}

/**
 * @tc.name: PowerMgrMock063
 * @tc.desc: test Screen RunningLock by mock
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrSTMockTest, PowerMgrMock063, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerMgrMock063: start";
    POWER_HILOGI(LABEL_TEST, "PowerMgrMock063:Start");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock063: Failed to get PowerMgrService";
    }

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_SCREEN);
    pms->CreateRunningLock(token, info);

    pms->Lock(token);
    EXPECT_EQ(pms->IsUsed(token), true);

    EXPECT_EQ(pms->ForceSuspendDevice(0), true);
    sleep(SLEEP_WAIT_TIME_S + ONE_SECOND);

    pms->UnLock(token);
    EXPECT_EQ(pms->IsUsed(token), false);

    POWER_HILOGI(LABEL_TEST, "PowerMgrMock063:End");
    GTEST_LOG_(INFO) << "PowerMgrMock063: end";
}

/**
 * @tc.name: PowerMgrMock071
 * @tc.desc: test proximity screen control RunningLock by mock
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrSTMockTest, PowerMgrMock071, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerMgrMock071: start";
    POWER_HILOGI(LABEL_TEST, "PowerMgrMock071:Start");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock071: Failed to get PowerMgrService";
    }

    pms->SetDisplayOffTime(SET_DISPLAY_OFF_TIME_MS);
    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL);
    pms->CreateRunningLock(token, info);
    EXPECT_CALL(*g_stateAction, GetDisplayState())
        .Times(::testing::AtLeast(1))
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    EXPECT_EQ(PowerState::INACTIVE, pms->GetState());
    EXPECT_CALL(*g_stateAction, GetDisplayState())
        .Times(::testing::AtLeast(1))
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_OFF));
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_ON, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    sleep(REFRESHACTIVITY_WAIT_TIME_S + ONE_SECOND);
    pms->Lock(token);
    EXPECT_EQ(pms->IsUsed(token), true);

    pms->UnLock(token);
    EXPECT_EQ(pms->IsUsed(token), false);

    pms->SetDisplayOffTime(DEFAULT_DISPLAY_OFF_TIME);
    POWER_HILOGI(LABEL_TEST, "PowerMgrMock071:End");
    GTEST_LOG_(INFO) << "PowerMgrMock071: end";
}

/**
 * @tc.name: PowerMgrMock072
 * @tc.desc: test proximity screen control RunningLock by mock
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrSTMockTest, PowerMgrMock072, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerMgrMock072: start";
    POWER_HILOGI(LABEL_TEST, "PowerMgrMock072:Start");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock072: Failed to get PowerMgrService";
    }

    pms->SetDisplayOffTime(SET_DISPLAY_OFF_TIME_MS);
    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL);
    pms->CreateRunningLock(token, info);
    EXPECT_EQ(pms->ForceSuspendDevice(0), true);
    EXPECT_CALL(*g_stateAction, GetDisplayState())
        .Times(::testing::AtLeast(1))
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_OFF));
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_ON, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    sleep(REFRESHACTIVITY_WAIT_TIME_S + ONE_SECOND);
    pms->Lock(token);
    EXPECT_EQ(pms->IsUsed(token), true);
    pms->UnLock(token);
    EXPECT_EQ(pms->IsUsed(token), false);

    pms->SetDisplayOffTime(DEFAULT_DISPLAY_OFF_TIME);
    POWER_HILOGI(LABEL_TEST, "PowerMgrMock072:End");
    GTEST_LOG_(INFO) << "PowerMgrMock072: end";
}

/**
 * @tc.name: PowerMgrMock074
 * @tc.desc: test proximity RunningLock by mock
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrSTMockTest, PowerMgrMock074, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerMgrMock074: start";
    POWER_HILOGI(LABEL_TEST, "PowerMgrMock074:Start");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock074: Failed to get PowerMgrService";
    }

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL);
    pms->CreateRunningLock(token, info);

    pms->Lock(token);
    EXPECT_EQ(pms->IsUsed(token), true);
    pms->MockProximity(RunningLockMgr::PROXIMITY_CLOSE);
    pms->UnLock(token);

    POWER_HILOGI(LABEL_TEST, "PowerMgrMock074:End");
    GTEST_LOG_(INFO) << "PowerMgrMock074: end";
}

/**
 * @tc.name: PowerMgrMock075
 * @tc.desc: test proximity RunningLock by mock
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrSTMockTest, PowerMgrMock075, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerMgrMock075: start";
    POWER_HILOGI(LABEL_TEST, "PowerMgrMock075:Start");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock075: Failed to get PowerMgrService";
    }

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL);
    pms->CreateRunningLock(token, info);
    pms->Lock(token);
    EXPECT_EQ(pms->IsUsed(token), true);
    EXPECT_EQ(PowerState::AWAKE, pms->GetState());
    pms->MockProximity(RunningLockMgr::PROXIMITY_CLOSE);
    pms->UnLock(token);

    POWER_HILOGI(LABEL_TEST, "PowerMgrMock075:End");
    GTEST_LOG_(INFO) << "PowerMgrMock075: end";
}

/**
 * @tc.name: PowerMgrMock076
 * @tc.desc: test proximity RunningLock by mock
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrSTMockTest, PowerMgrMock076, TestSize.Level2)
{
    int i;
    GTEST_LOG_(INFO) << "PowerMgrMock076: start";
    POWER_HILOGI(LABEL_TEST, "PowerMgrMock076:Start");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock076: Failed to get PowerMgrService";
    }

    pms->SetDisplayOffTime(ASYNC_WAIT_TIME_S * TEST_TIMES);
    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL);
    pms->CreateRunningLock(token, info);
    for (i = 0; i < 10; i++) {
        pms->Lock(token);
        EXPECT_EQ(pms->IsUsed(token), true);
        pms->UnLock(token);
        EXPECT_EQ(pms->IsUsed(token), false);
    }
    pms->Lock(token);
    EXPECT_EQ(pms->IsUsed(token), true);
    pms->UnLock(token);

    pms->SetDisplayOffTime(DEFAULT_DISPLAY_OFF_TIME);
    POWER_HILOGI(LABEL_TEST, "PowerMgrMock076:End");
    GTEST_LOG_(INFO) << "PowerMgrMock076: end";
}

/**
 * @tc.name: PowerMgrMock078
 * @tc.desc: test proximity RunningLock by mock
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrSTMockTest, PowerMgrMock078, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerMgrMock078: start";
    POWER_HILOGI(LABEL_TEST, "PowerMgrMock078:Start");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock078: Failed to get PowerMgrService";
    }

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL);
    pms->CreateRunningLock(token, info);
    pms->Lock(token);
    EXPECT_EQ(pms->IsUsed(token), true);

    pms->ForceSuspendDevice(0);

    pms->UnLock(token);
    EXPECT_EQ(pms->IsUsed(token), false);

    POWER_HILOGI(LABEL_TEST, "PowerMgrMock078:End");
    GTEST_LOG_(INFO) << "PowerMgrMock078: end";
}

/**
 * @tc.name: PowerMgrMock079
 * @tc.desc: test proximity RunningLock by mock
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrSTMockTest, PowerMgrMock079, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerMgrMock079: start";
    POWER_HILOGI(LABEL_TEST, "PowerMgrMock079:Start");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock079: Failed to get PowerMgrService";
    }

    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL);
    pms->CreateRunningLock(token, info);

    pms->Lock(token);
    EXPECT_EQ(pms->IsUsed(token), true);

    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN, std::string("test"));
    EXPECT_EQ(PowerState::AWAKE, pms->GetState());

    POWER_HILOGI(LABEL_TEST, "PowerMgrMock079:End");
    GTEST_LOG_(INFO) << "PowerMgrMock079: end";
}

/**
 * @tc.name: PowerMgrMock081
 * @tc.desc: test The display status and power status are inconsistent SuspendDevice
 * @tc.type: FUNC
 * @tc.require: issueI66HYP
 */
HWTEST_F(PowerMgrSTMockTest, PowerMgrMock081, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerMgrMock081: start.";
    POWER_HILOGI(LABEL_TEST, "PowerMgrMock081:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    ASSERT_TRUE(pms != nullptr);

    // Set the power status to INACTIVE
    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    EXPECT_TRUE(pms->GetState() == PowerState::INACTIVE || pms->GetState() == PowerState::SLEEP);

    // Analog display return DISPLAY_ON
    EXPECT_CALL(*g_stateAction, GetDisplayState())
        .Times(::testing::AtLeast(1))
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    // The status is changed to display status ON, and the screen is off again
    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    EXPECT_TRUE(pms->GetState() == PowerState::INACTIVE || pms->GetState() == PowerState::SLEEP);

    POWER_HILOGI(LABEL_TEST, "PowerMgrMock081:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock081: end.";
}

/**
 * @tc.name: PowerMgrMock082
 * @tc.desc: test The display status and power status are inconsistent WakeupDevice
 * @tc.type: FUNC
 * @tc.require: issueI66HYP
 */
HWTEST_F(PowerMgrSTMockTest, PowerMgrMock082, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerMgrMock082: start.";
    POWER_HILOGI(LABEL_TEST, "PowerMgrMock082:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    ASSERT_TRUE(pms != nullptr);

    // Set the power status to AWAKE
    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON, std::string("PowerMgrMock082"));
    EXPECT_TRUE(pms->GetState() == PowerState::AWAKE);

    // Analog display return DISPLAY_OFF
    EXPECT_CALL(*g_stateAction, GetDisplayState())
        .Times(::testing::AtLeast(1))
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_OFF));
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_ON, ::testing::_))
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    // The status is changed to display status OFF, and the screen is off again
    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON, std::string("PowerMgrMock082_Again"));
    EXPECT_TRUE(pms->GetState() == PowerState::AWAKE);

    POWER_HILOGI(LABEL_TEST, "PowerMgrMock082:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock082: end.";
}

/**
 * @tc.name: PowerMgrMock083
 * @tc.desc: test SetState will block transit from AWAKE to SLEEP/HIBERNATE
 * @tc.type: FUNC
 * @tc.require: issueI9AMZT
 */
HWTEST_F(PowerMgrSTMockTest, PowerMgrMock083, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerMgrMock083: start.";
    POWER_HILOGI(LABEL_TEST, "PowerMgrMock083:Start.");

    vector<PowerState> sleepStates { PowerState::SLEEP, PowerState::HIBERNATE };

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    ASSERT_TRUE(pms != nullptr);
    std::shared_ptr<PowerStateMachine> stateMachine = pms->GetPowerStateMachine();
    ASSERT_TRUE(stateMachine != nullptr);

    // Analog display return DISPLAY_ON
    EXPECT_CALL(*g_stateAction, GetDisplayState())
    .Times(::testing::AtLeast(1))
    .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));

    // Set the power state to AWAKE
    auto ret = stateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_INIT);
    EXPECT_TRUE(stateMachine->GetState() == PowerState::AWAKE);

    for (auto targetState : sleepStates) {
        // Set the power state to target state
        auto ret = stateMachine->SetState(targetState, StateChangeReason::STATE_CHANGE_REASON_SYSTEM);
        // SetState will block transit from AWAKE to these states, expect SetState fail
        EXPECT_FALSE(ret);
        EXPECT_TRUE(stateMachine->GetState() == PowerState::AWAKE);
    }

    POWER_HILOGI(LABEL_TEST, "PowerMgrMock083:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock083: end.";
}
} // namespace
