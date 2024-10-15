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
    POWER_HILOGD(LABEL_TEST, "ResetMockAction:Start");
    g_stateAction = new MockStateAction();
    g_shutdownState = new MockStateAction();
    g_powerAction = new MockPowerAction();
    g_lockAction = new MockLockAction();
    g_service->EnableMock(g_stateAction, g_shutdownState, g_powerAction, g_lockAction);
    POWER_HILOGD(LABEL_TEST, "ResetMockAction:End");
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
    POWER_HILOGD(LABEL_TEST, "PowerMgrMock001:Start");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock001: Failed to get PowerMgrService";
    }

    EXPECT_CALL(*g_powerAction, Reboot(std::string("test"))).Times(1);
    pms->RebootDevice(std::string("test"));

    POWER_HILOGD(LABEL_TEST, "PowerMgrMock001:End");
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
    POWER_HILOGD(LABEL_TEST, "PowerMgrMock002:Start");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock002: Failed to get PowerMgrService";
    }

    EXPECT_CALL(*g_powerAction, Shutdown(std::string("test"))).Times(1);
    pms->ShutDownDevice(std::string("test"));

    POWER_HILOGD(LABEL_TEST, "PowerMgrMock002:End");
    GTEST_LOG_(INFO) << "PowerMgrMock002: end";
    usleep(SLEEP_WAIT_TIME_MS * TRANSFER_NS_TO_MS);
}

/**
 * @tc.name: PowerMgrMock003
 * @tc.desc: test ForceSuspendDevice by mock
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrSTMockTest, PowerMgrMock003, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerMgrMock003: start";
    POWER_HILOGD(LABEL_TEST, "PowerMgrMock003:Start");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock003: Failed to get PowerMgrService";
    }

    POWER_HILOGD(LABEL_TEST, "PowerMgrMock003:forcesuspend");
    EXPECT_EQ(pms->ForceSuspendDevice(0), PowerErrors::ERR_OK);

    POWER_HILOGD(LABEL_TEST, "PowerMgrMock003:End");
    GTEST_LOG_(INFO) << "PowerMgrMock003: end";
}

/**
 * @tc.name: PowerMgrMock004
 * @tc.desc: test RunningLock by mock
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrSTMockTest, PowerMgrMock004, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerMgrMock004: start";
    POWER_HILOGD(LABEL_TEST, "PowerMgrMock004:Start");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock004: Failed to get PowerMgrService";
    }

    pms->SetDisplayOffTime(SET_DISPLAY_OFF_TIME_MS);
    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_SCREEN);
    pms->CreateRunningLock(token, info);
    pms->Lock(token);
    EXPECT_EQ(pms->IsUsed(token), true);
    pms->UnLock(token);
    EXPECT_EQ(pms->IsUsed(token), false);

    pms->SetDisplayOffTime(PowerStateMachine::DEFAULT_DISPLAY_OFF_TIME_MS);
    POWER_HILOGD(LABEL_TEST, "PowerMgrMock004:End");
    GTEST_LOG_(INFO) << "PowerMgrMock004: end";
}

/**
 * @tc.name: PowerMgrMock005
 * @tc.desc: test proximity RunningLock by mock
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrSTMockTest, PowerMgrMock005, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerMgrMock005: start";
    POWER_HILOGD(LABEL_TEST, "PowerMgrMock005:Start");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock005: Failed to get PowerMgrService";
    }

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL);
    pms->CreateRunningLock(token, info);

    pms->Lock(token);
    EXPECT_EQ(pms->IsUsed(token), true);
    sleep(SCREEN_DIM_WAIT_TIME_S + ONE_SECOND);

    pms->UnLock(token);
    EXPECT_EQ(pms->IsUsed(token), false);

    POWER_HILOGD(LABEL_TEST, "PowerMgrMock005:End");
    GTEST_LOG_(INFO) << "PowerMgrMock005: end";
}

/**
 * @tc.name: PowerMgrMock006
 * @tc.desc: test ForceSuspendDevice by mock during Inactive
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrSTMockTest, PowerMgrMock006, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerMgrMock006: start";
    POWER_HILOGD(LABEL_TEST, "PowerMgrMock006:Start");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock006: Failed to get PowerMgrService";
    }

    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    pms->ForceSuspendDevice(0);
    sleep(SLEEP_WAIT_TIME_S + ONE_SECOND);

    POWER_HILOGD(LABEL_TEST, "PowerMgrMock006:End");
    GTEST_LOG_(INFO) << "PowerMgrMock006: end";
}

/**
 * @tc.name: PowerMgrMock007
 * @tc.desc: test Screen RunningLock by mock
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrSTMockTest, PowerMgrMock007, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerMgrMock007: start";
    POWER_HILOGD(LABEL_TEST, "PowerMgrMock007:Start");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock007: Failed to get PowerMgrService";
    }

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_SCREEN);
    pms->CreateRunningLock(token, info);

    pms->Lock(token);
    EXPECT_EQ(pms->IsUsed(token), true);

    EXPECT_EQ(pms->ForceSuspendDevice(0), PowerErrors::ERR_OK);
    sleep(SLEEP_WAIT_TIME_S + ONE_SECOND);

    pms->UnLock(token);
    EXPECT_EQ(pms->IsUsed(token), false);

    POWER_HILOGD(LABEL_TEST, "PowerMgrMock007:End");
    GTEST_LOG_(INFO) << "PowerMgrMock007: end";
}

/**
 * @tc.name: PowerMgrMock008
 * @tc.desc: test proximity screen control RunningLock by mock
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrSTMockTest, PowerMgrMock008, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerMgrMock008: start";
    POWER_HILOGD(LABEL_TEST, "PowerMgrMock008:Start");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock008: Failed to get PowerMgrService";
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
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    sleep(REFRESHACTIVITY_WAIT_TIME_S + ONE_SECOND);
    pms->Lock(token);
    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_APPLICATION, std::string("test"));
    EXPECT_EQ(pms->IsUsed(token), true);

    pms->UnLock(token);
    EXPECT_EQ(pms->IsUsed(token), false);

    pms->SetDisplayOffTime(PowerStateMachine::DEFAULT_DISPLAY_OFF_TIME_MS);
    POWER_HILOGD(LABEL_TEST, "PowerMgrMock008:End");
    GTEST_LOG_(INFO) << "PowerMgrMock008: end";
}

/**
 * @tc.name: PowerMgrMock009
 * @tc.desc: test proximity screen control RunningLock by mock
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrSTMockTest, PowerMgrMock009, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerMgrMock009: start";
    POWER_HILOGD(LABEL_TEST, "PowerMgrMock009:Start");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock009: Failed to get PowerMgrService";
    }

    pms->SetDisplayOffTime(SET_DISPLAY_OFF_TIME_MS);
    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL);
    pms->CreateRunningLock(token, info);
    EXPECT_EQ(pms->ForceSuspendDevice(0), PowerErrors::ERR_OK);
    EXPECT_CALL(*g_stateAction, GetDisplayState())
        .Times(::testing::AtLeast(1))
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_OFF));
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_ON, ::testing::_))
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    sleep(REFRESHACTIVITY_WAIT_TIME_S + ONE_SECOND);
    pms->Lock(token);
    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_APPLICATION, std::string("test"));
    EXPECT_EQ(pms->IsUsed(token), true);
    pms->UnLock(token);
    EXPECT_EQ(pms->IsUsed(token), false);

    pms->SetDisplayOffTime(PowerStateMachine::DEFAULT_DISPLAY_OFF_TIME_MS);
    POWER_HILOGD(LABEL_TEST, "PowerMgrMock009:End");
    GTEST_LOG_(INFO) << "PowerMgrMock009: end";
}

/**
 * @tc.name: PowerMgrMock010
 * @tc.desc: test proximity RunningLock by mock
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrSTMockTest, PowerMgrMock010, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerMgrMock010: start";
    POWER_HILOGD(LABEL_TEST, "PowerMgrMock010:Start");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock010: Failed to get PowerMgrService";
    }

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL);
    pms->CreateRunningLock(token, info);

    pms->Lock(token);
    EXPECT_EQ(pms->IsUsed(token), true);
    pms->MockProximity(RunningLockMgr::PROXIMITY_CLOSE);
    pms->UnLock(token);

    POWER_HILOGD(LABEL_TEST, "PowerMgrMock010:End");
    GTEST_LOG_(INFO) << "PowerMgrMock010: end";
}

/**
 * @tc.name: PowerMgrMock011
 * @tc.desc: test proximity RunningLock by mock
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrSTMockTest, PowerMgrMock011, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerMgrMock011: start";
    POWER_HILOGD(LABEL_TEST, "PowerMgrMock011:Start");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock011: Failed to get PowerMgrService";
    }

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL);
    pms->CreateRunningLock(token, info);
    pms->Lock(token);
    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_APPLICATION, std::string("test"));
    EXPECT_EQ(pms->IsUsed(token), true);
    EXPECT_EQ(PowerState::AWAKE, pms->GetState());
    pms->MockProximity(RunningLockMgr::PROXIMITY_CLOSE);
    pms->UnLock(token);

    POWER_HILOGD(LABEL_TEST, "PowerMgrMock011:End");
    GTEST_LOG_(INFO) << "PowerMgrMock011: end";
}

/**
 * @tc.name: PowerMgrMock012
 * @tc.desc: test proximity RunningLock by mock
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrSTMockTest, PowerMgrMock012, TestSize.Level2)
{
    int i;
    GTEST_LOG_(INFO) << "PowerMgrMock012: start";
    POWER_HILOGD(LABEL_TEST, "PowerMgrMock012:Start");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock012: Failed to get PowerMgrService";
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

    pms->SetDisplayOffTime(PowerStateMachine::DEFAULT_DISPLAY_OFF_TIME_MS);
    POWER_HILOGD(LABEL_TEST, "PowerMgrMock012:End");
    GTEST_LOG_(INFO) << "PowerMgrMock012: end";
}

/**
 * @tc.name: PowerMgrMock013
 * @tc.desc: test proximity RunningLock by mock
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrSTMockTest, PowerMgrMock013, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerMgrMock013: start";
    POWER_HILOGD(LABEL_TEST, "PowerMgrMock013:Start");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock013: Failed to get PowerMgrService";
    }

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL);
    pms->CreateRunningLock(token, info);
    pms->Lock(token);
    EXPECT_EQ(pms->IsUsed(token), true);

    pms->ForceSuspendDevice(0);

    pms->UnLock(token);
    EXPECT_EQ(pms->IsUsed(token), false);

    POWER_HILOGD(LABEL_TEST, "PowerMgrMock013:End");
    GTEST_LOG_(INFO) << "PowerMgrMock013: end";
}

/**
 * @tc.name: PowerMgrMock014
 * @tc.desc: test proximity RunningLock by mock
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrSTMockTest, PowerMgrMock014, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerMgrMock014: start";
    POWER_HILOGD(LABEL_TEST, "PowerMgrMock014:Start");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock014: Failed to get PowerMgrService";
    }

    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL);
    pms->CreateRunningLock(token, info);

    pms->Lock(token);
    EXPECT_EQ(pms->IsUsed(token), true);

    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN, std::string("test"));
    EXPECT_EQ(PowerState::AWAKE, pms->GetState());

    POWER_HILOGD(LABEL_TEST, "PowerMgrMock014:End");
    GTEST_LOG_(INFO) << "PowerMgrMock014: end";
}

/**
 * @tc.name: PowerMgrMock015
 * @tc.desc: test The display status and power status are inconsistent SuspendDevice
 * @tc.type: FUNC
 * @tc.require: issueI66HYP
 */
HWTEST_F(PowerMgrSTMockTest, PowerMgrMock015, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerMgrMock015: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrMock015:Start.");

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

    POWER_HILOGD(LABEL_TEST, "PowerMgrMock015:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock015: end.";
}

/**
 * @tc.name: PowerMgrMock016
 * @tc.desc: test The display status and power status are inconsistent WakeupDevice
 * @tc.type: FUNC
 * @tc.require: issueI66HYP
 */
HWTEST_F(PowerMgrSTMockTest, PowerMgrMock016, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerMgrMock016: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrMock016:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    ASSERT_TRUE(pms != nullptr);

    // Set the power status to AWAKE
    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON, std::string("PowerMgrMock016"));
    EXPECT_TRUE(pms->GetState() == PowerState::AWAKE);

    // Analog display return DISPLAY_OFF
    EXPECT_CALL(*g_stateAction, GetDisplayState())
        .Times(::testing::AtLeast(1))
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_OFF));
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_ON, ::testing::_))
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    // The status is changed to display status OFF, and the screen is off again
    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON, std::string("PowerMgrMock016_Again"));
    EXPECT_TRUE(pms->GetState() == PowerState::AWAKE);

    POWER_HILOGD(LABEL_TEST, "PowerMgrMock016:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock016: end.";
}

/**
 * @tc.name: PowerMgrMock017
 * @tc.desc: test SetState will block transit from AWAKE to SLEEP/HIBERNATE
 * @tc.type: FUNC
 * @tc.require: issueI9AMZT
 */
HWTEST_F(PowerMgrSTMockTest, PowerMgrMock017, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerMgrMock017: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrMock017:Start.");

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

    POWER_HILOGD(LABEL_TEST, "PowerMgrMock017:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock017: end.";
}
} // namespace
