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
    POWER_HILOGD(LABEL_TEST, "ResetMockAction:Start.");
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
}

void PowerMgrSTMockTest::TearDownTestCase(void)
{
    g_service->OnStop();
    DelayedSpSingleton<PowerMgrService>::DestroyInstance();
}

void PowerMgrSTMockTest::SetUp(void)
{
    ResetMockAction();
}

void PowerMgrSTMockTest::TearDown(void)
{
}

namespace {
/**
 * @tc.name: PowerMgrMock001
 * @tc.desc: test RebootDevice by mock
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrSTMockTest, PowerMgrMock001, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerMgrMock001: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrMock001:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock001: Failed to get PowerMgrService";
    }

    EXPECT_CALL(*g_powerAction, Reboot(std::string("test"))).Times(1);
    pms->RebootDevice(std::string("test"));

    POWER_HILOGD(LABEL_TEST, "PowerMgrMock001:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock001: end.";
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
    GTEST_LOG_(INFO) << "PowerMgrMock002: start.";
    POWER_HILOGI(LABEL_TEST, "PowerMgrMock002:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock002: Failed to get PowerMgrService";
    }

    EXPECT_CALL(*g_powerAction, Shutdown(std::string("test"))).Times(1);
    pms->ShutDownDevice(std::string("test"));

    POWER_HILOGI(LABEL_TEST, "PowerMgrMock002:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock002: end.";
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
    GTEST_LOG_(INFO) << "PowerMgrMock029: start.";
    POWER_HILOGI(LABEL_TEST, "PowerMgrMock029:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock029: Failed to get PowerMgrService";
    }

    EXPECT_CALL(*g_stateAction, GoToSleep(_, _, true)).Times(1).WillOnce(::testing::Return(ActionResult::SUCCESS));
    POWER_HILOGI(LABEL_TEST, "PowerMgrMock029:forcesuspend.");
    EXPECT_EQ(pms->ForceSuspendDevice(0), true);

    POWER_HILOGI(LABEL_TEST, "PowerMgrMock029:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock029: end.";
}

/**
 * @tc.name: PowerMgrMock030
 * @tc.desc: test Screen on RunningLock by mock
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrSTMockTest, PowerMgrMock030, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerMgrMock030: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrMock030:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock030: Failed to get PowerMgrService";
    }

    pms->SetDisplayOffTime(SET_DISPLAY_OFF_TIME_MS);
    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_SCREEN);
    pms->CreateRunningLock(token, info);
    pms->Lock(token, info, 0);
    EXPECT_EQ(pms->IsUsed(token), true);

    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_DIM, ::testing::_)).Times(0);
    sleep((REFRESHACTIVITY_WAIT_TIME_S / TEST_RATE) + 1);

    pms->UnLock(token);
    EXPECT_EQ(pms->IsUsed(token), false);

    pms->SetDisplayOffTime(DEFAULT_DISPLAY_OFF_TIME);
    POWER_HILOGD(LABEL_TEST, "PowerMgrMock030:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock030: end.";
}

/**
 * @tc.name: PowerMgrMock031
 * @tc.desc: test background RunningLock by mock
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrSTMockTest, PowerMgrMock031, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerMgrMock031: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrMock031:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock031: Failed to get PowerMgrService";
    }

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_BACKGROUND);
    pms->CreateRunningLock(token, info);

    EXPECT_CALL(*g_lockAction, Lock(_, _)).Times(1);
    pms->Lock(token, info, 0);
    EXPECT_EQ(pms->IsUsed(token), true);

    EXPECT_CALL(*g_stateAction, GoToSleep(_, _, _)).Times(0);
    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    sleep(SLEEP_WAIT_TIME_S + ONE_SECOND);

    EXPECT_CALL(*g_lockAction, Unlock(_, _)).Times(1);
    pms->UnLock(token);
    EXPECT_EQ(pms->IsUsed(token), false);

    POWER_HILOGD(LABEL_TEST, "PowerMgrMock031:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock031: end.";
}

/**
 * @tc.name: PowerMgrMock032
 * @tc.desc: test proximity RunningLock by mock
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrSTMockTest, PowerMgrMock032, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerMgrMock032: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrMock032:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock032: Failed to get PowerMgrService";
    }

    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_DIM, ::testing::_)).Times(0);
    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL);
    pms->CreateRunningLock(token, info);

    EXPECT_CALL(*g_lockAction, Lock(_, _)).Times(1);
    pms->Lock(token, info, 0);
    EXPECT_EQ(pms->IsUsed(token), true);

    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_DIM, ::testing::_)).Times(0);
    sleep(SCREEN_DIM_WAIT_TIME_S + ONE_SECOND);

    EXPECT_CALL(*g_lockAction, Unlock(_, _)).Times(1);
    pms->UnLock(token);
    EXPECT_EQ(pms->IsUsed(token), false);

    POWER_HILOGD(LABEL_TEST, "PowerMgrMock032:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock032: end.";
}

/**
 * @tc.name: PowerMgrMock033
 * @tc.desc: test RunningLock release by mock
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrSTMockTest, PowerMgrMock033, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerMgrMock033: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrMock033:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock033: Failed to get PowerMgrService";
    }

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_BACKGROUND);
    pms->CreateRunningLock(token, info);
    pms->ReleaseRunningLock(token);

    EXPECT_CALL(*g_lockAction, Lock(_, _)).Times(0);
    pms->Lock(token, info, 0);
    EXPECT_EQ(pms->IsUsed(token), false);

    POWER_HILOGD(LABEL_TEST, "PowerMgrMock033:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock033: end.";
}

/**
 * @tc.name: PowerMgrMock048
 * @tc.desc: test SuspendDevice by mock
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrSTMockTest, PowerMgrMock048, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerMgrMock048: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrMock048:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock048: Failed to get PowerMgrService";
    }

    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON, std::string("test"));
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction, Suspend(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false));
    EXPECT_CALL(*g_stateAction, GoToSleep(_, _, _)).Times(1).WillOnce(::testing::Return(ActionResult::SUCCESS));
    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    EXPECT_EQ(PowerState::INACTIVE, pms->GetState());
    sleep(SLEEP_WAIT_TIME_S + ONE_SECOND);
    EXPECT_EQ(PowerState::SLEEP, pms->GetState());

    POWER_HILOGD(LABEL_TEST, "PowerMgrMock048:End");
    GTEST_LOG_(INFO) << "PowerMgrMock048: end.";
}

/**
 * @tc.name: PowerMgrMock054
 * @tc.desc: test ForceSuspendDevice by mock during Inactive
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrSTMockTest, PowerMgrMock054, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerMgrMock054: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrMock054:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock054: Failed to get PowerMgrService";
    }

    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    EXPECT_CALL(*g_stateAction, GoToSleep(_, _, _))
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    pms->ForceSuspendDevice(0);
    sleep(SLEEP_WAIT_TIME_S + ONE_SECOND);
    EXPECT_EQ(PowerState::SLEEP, pms->GetState());

    POWER_HILOGD(LABEL_TEST, "PowerMgrMock054:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock054: end.";
}

/**
 * @tc.name: PowerMgrMock062
 * @tc.desc: test Screen RunningLock by mock
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrSTMockTest, PowerMgrMock062, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerMgrMock062: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrMock062:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock062: Failed to get PowerMgrService";
    }

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_SCREEN);
    pms->CreateRunningLock(token, info);

    pms->Lock(token, info, 0);
    EXPECT_EQ(pms->IsUsed(token), true);

    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(ActionResult::SUCCESS));

    EXPECT_CALL(*g_stateAction, GoToSleep(_, _, _)).Times(1);
    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    sleep(SLEEP_WAIT_TIME_S + ONE_SECOND);

    pms->UnLock(token);
    EXPECT_EQ(pms->IsUsed(token), false);
    EXPECT_EQ(PowerState::SLEEP, pms->GetState());

    POWER_HILOGD(LABEL_TEST, "PowerMgrMock062:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock062: end.";
}

/**
 * @tc.name: PowerMgrMock063
 * @tc.desc: test Screen RunningLock by mock
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrSTMockTest, PowerMgrMock063, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerMgrMock063: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrMock063:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock063: Failed to get PowerMgrService";
    }

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_SCREEN);
    pms->CreateRunningLock(token, info);

    pms->Lock(token, info, 0);
    EXPECT_EQ(pms->IsUsed(token), true);

    EXPECT_CALL(*g_stateAction, GoToSleep(_, _, true)).Times(1).WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_EQ(pms->ForceSuspendDevice(0), true);
    sleep(SLEEP_WAIT_TIME_S + ONE_SECOND);

    pms->UnLock(token);
    EXPECT_EQ(pms->IsUsed(token), false);
    EXPECT_EQ(PowerState::SLEEP, pms->GetState());

    POWER_HILOGD(LABEL_TEST, "PowerMgrMock063:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock063: end.";
}

/**
 * @tc.name: PowerMgrMock066
 * @tc.desc: test background RunningLock by mock
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrSTMockTest, PowerMgrMock066, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerMgrMock066: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrMock066:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock066: Failed to get PowerMgrService";
    }

    pms->SetDisplayOffTime(SET_DISPLAY_OFF_TIME_MS);
    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_BACKGROUND);
    pms->CreateRunningLock(token, info);

    EXPECT_CALL(*g_lockAction, Lock(_, _)).Times(1);
    pms->Lock(token, info, 0);
    EXPECT_EQ(pms->IsUsed(token), true);

    EXPECT_CALL(*g_lockAction, Unlock(_, _)).Times(1);
    pms->UnLock(token);
    ON_CALL(*g_stateAction, GetDisplayState()).WillByDefault(::testing::Return(DisplayState::DISPLAY_ON));
    sleep((REFRESHACTIVITY_WAIT_TIME_S * DOUBLE_TIMES / TEST_RATE) + 1);
    ON_CALL(*g_stateAction, GetDisplayState()).WillByDefault(::testing::Return(DisplayState::DISPLAY_DIM));
    sleep((REFRESHACTIVITY_WAIT_TIME_S / TEST_RATE) + 1);
    ON_CALL(*g_stateAction, GetDisplayState()).WillByDefault(::testing::Return(DisplayState::DISPLAY_OFF));
    EXPECT_EQ(pms->IsUsed(token), false);
    EXPECT_CALL(*g_stateAction, GoToSleep(_, _, _)).Times(1);
    sleep(SLEEP_WAIT_TIME_S + ONE_SECOND);
    EXPECT_EQ(PowerState::SLEEP, pms->GetState());

    pms->SetDisplayOffTime(DEFAULT_DISPLAY_OFF_TIME);
    POWER_HILOGD(LABEL_TEST, "PowerMgrMock066:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock066: end.";
}

/**
 * @tc.name: PowerMgrMock070
 * @tc.desc: test background RunningLock by mock
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrSTMockTest, PowerMgrMock070, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerMgrMock070: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrMock070:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock070: Failed to get PowerMgrService";
    }

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_BACKGROUND);
    pms->CreateRunningLock(token, info);

    EXPECT_CALL(*g_lockAction, Lock(_, _)).Times(1);
    pms->Lock(token, info, 0);
    EXPECT_EQ(pms->IsUsed(token), true);

    sleep(SLEEP_WAIT_TIME_S + ONE_SECOND);
    EXPECT_CALL(*g_stateAction, GoToSleep(_, _, true)).Times(1).WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_EQ(pms->ForceSuspendDevice(0), true);
    sleep(SLEEP_WAIT_TIME_S + ONE_SECOND);

    EXPECT_CALL(*g_lockAction, Unlock(_, _)).Times(1);
    pms->UnLock(token);
    EXPECT_EQ(pms->IsUsed(token), false);
    EXPECT_EQ(PowerState::SLEEP, pms->GetState());

    POWER_HILOGD(LABEL_TEST, "PowerMgrMock070:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock070: end.";
}

/**
 * @tc.name: PowerMgrMock071
 * @tc.desc: test proximity screen control RunningLock by mock
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrSTMockTest, PowerMgrMock071, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerMgrMock071: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrMock071:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock071: Failed to get PowerMgrService";
    }

    pms->SetDisplayOffTime(SET_DISPLAY_OFF_TIME_MS);
    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL);
    pms->CreateRunningLock(token, info);
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    EXPECT_EQ(PowerState::INACTIVE, pms->GetState());
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_ON, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    sleep(REFRESHACTIVITY_WAIT_TIME_S + ONE_SECOND);
    pms->Lock(token, info, 0);
    EXPECT_EQ(pms->IsUsed(token), true);
    EXPECT_EQ(PowerState::AWAKE, pms->GetState());

    pms->UnLock(token);
    EXPECT_EQ(pms->IsUsed(token), false);

    pms->SetDisplayOffTime(DEFAULT_DISPLAY_OFF_TIME);
    POWER_HILOGD(LABEL_TEST, "PowerMgrMock071:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock071: end.";
}

/**
 * @tc.name: PowerMgrMock072
 * @tc.desc: test proximity screen control RunningLock by mock
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrSTMockTest, PowerMgrMock072, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerMgrMock072: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrMock072:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock072: Failed to get PowerMgrService";
    }

    pms->SetDisplayOffTime(SET_DISPLAY_OFF_TIME_MS);
    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL);
    pms->CreateRunningLock(token, info);
    EXPECT_CALL(*g_stateAction, GoToSleep(_, _, true)).Times(1).WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_EQ(pms->ForceSuspendDevice(0), true);
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_ON, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    sleep(REFRESHACTIVITY_WAIT_TIME_S + ONE_SECOND);
    EXPECT_EQ(PowerState::SLEEP, pms->GetState());
    pms->Lock(token, info, 0);
    EXPECT_EQ(pms->IsUsed(token), true);
    EXPECT_EQ(PowerState::AWAKE, pms->GetState());
    pms->UnLock(token);
    EXPECT_EQ(pms->IsUsed(token), false);

    pms->SetDisplayOffTime(DEFAULT_DISPLAY_OFF_TIME);
    POWER_HILOGD(LABEL_TEST, "PowerMgrMock072:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock072: end.";
}

/**
 * @tc.name: PowerMgrMock073
 * @tc.desc: test proximity screen control RunningLock by mock
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrSTMockTest, PowerMgrMock073, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerMgrMock073: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrMock073:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock073: Failed to get PowerMgrService";
    }

    pms->SetDisplayOffTime(SET_DISPLAY_OFF_TIME_MS);
    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL);
    pms->CreateRunningLock(token, info);
    pms->Lock(token, info, 0);
    EXPECT_EQ(pms->IsUsed(token), true);
    sleep(SLEEP_WAIT_TIME_S);

    pms->UnLock(token);
    EXPECT_EQ(pms->IsUsed(token), false);
    ON_CALL(*g_stateAction, GetDisplayState()).WillByDefault(::testing::Return(DisplayState::DISPLAY_ON));
    sleep((REFRESHACTIVITY_WAIT_TIME_S * DOUBLE_TIMES / TEST_RATE) + 1);
    ON_CALL(*g_stateAction, GetDisplayState()).WillByDefault(::testing::Return(DisplayState::DISPLAY_DIM));
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    sleep((REFRESHACTIVITY_WAIT_TIME_S / TEST_RATE) + ONE_SECOND);
    EXPECT_EQ(PowerState::INACTIVE, pms->GetState());
    EXPECT_CALL(*g_stateAction, GoToSleep(_, _, false)).Times(1).WillOnce(::testing::Return(ActionResult::SUCCESS));
    ON_CALL(*g_stateAction, GetDisplayState()).WillByDefault(::testing::Return(DisplayState::DISPLAY_OFF));
    sleep(SLEEP_WAIT_TIME_S + ONE_SECOND);
    EXPECT_EQ(PowerState::SLEEP, pms->GetState());

    pms->SetDisplayOffTime(DEFAULT_DISPLAY_OFF_TIME);
    POWER_HILOGD(LABEL_TEST, "PowerMgrMock073:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock073: end.";
}

/**
 * @tc.name: PowerMgrMock074
 * @tc.desc: test proximity RunningLock by mock
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrSTMockTest, PowerMgrMock074, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerMgrMock074: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrMock074:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock074: Failed to get PowerMgrService";
    }

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL);
    pms->CreateRunningLock(token, info);

    EXPECT_CALL(*g_lockAction, Lock(_, _)).Times(1);
    pms->Lock(token, info, 0);
    EXPECT_EQ(pms->IsUsed(token), true);
    pms->MockProximity(RunningLockMgr::PROXIMITY_CLOSE);

    POWER_HILOGD(LABEL_TEST, "PowerMgrMock074:End");
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
    GTEST_LOG_(INFO) << "PowerMgrMock075: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrMock075:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock075: Failed to get PowerMgrService";
    }

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL);
    pms->CreateRunningLock(token, info);
    pms->Lock(token, info, 0);
    EXPECT_EQ(pms->IsUsed(token), true);
    EXPECT_EQ(PowerState::AWAKE, pms->GetState());
    pms->MockProximity(RunningLockMgr::PROXIMITY_CLOSE);

    POWER_HILOGD(LABEL_TEST, "PowerMgrMock075:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock075: end.";
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
    GTEST_LOG_(INFO) << "PowerMgrMock076: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrMock076:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock076: Failed to get PowerMgrService";
    }

    pms->SetDisplayOffTime(ASYNC_WAIT_TIME_S * 1000);
    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL);
    pms->CreateRunningLock(token, info);
    for (i = 0; i < 10; i++) {
        pms->Lock(token, info, 0);
        EXPECT_EQ(pms->IsUsed(token), true);
        pms->UnLock(token);
        EXPECT_EQ(pms->IsUsed(token), false);
    }
    pms->Lock(token, info, 0);
    EXPECT_EQ(pms->IsUsed(token), true);
    pms->MockProximity(RunningLockMgr::PROXIMITY_CLOSE);
    EXPECT_EQ(PowerState::AWAKE, pms->GetState());

    pms->SetDisplayOffTime(DEFAULT_DISPLAY_OFF_TIME);
    POWER_HILOGD(LABEL_TEST, "PowerMgrMock076:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock076: end.";
}

/**
 * @tc.name: PowerMgrMock077
 * @tc.desc: test proximity RunningLock by mock
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrSTMockTest, PowerMgrMock077, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerMgrMock077: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrMock077:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock077: Failed to get PowerMgrService";
    }

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL);
    pms->CreateRunningLock(token, info);
    EXPECT_CALL(*g_lockAction, Lock(_, _)).Times(1);
    pms->Lock(token, info, 0);
    EXPECT_EQ(pms->IsUsed(token), true);
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction, GoToSleep(_, _, false)).Times(1).WillOnce(::testing::Return(ActionResult::SUCCESS));
    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    sleep(SLEEP_WAIT_TIME_S + 1);
    ON_CALL(*g_stateAction, GetDisplayState()).WillByDefault(::testing::Return(DisplayState::DISPLAY_ON));
    sleep((SCREEN_OFF_WAIT_TIME_S * DOUBLE_TIMES / TEST_RATE) + ONE_SECOND);
    ON_CALL(*g_stateAction, GetDisplayState()).WillByDefault(::testing::Return(DisplayState::DISPLAY_DIM));
    sleep((SCREEN_OFF_WAIT_TIME_S / TEST_RATE) + ONE_SECOND);
    ON_CALL(*g_stateAction, GetDisplayState()).WillByDefault(::testing::Return(DisplayState::DISPLAY_OFF));
    EXPECT_EQ(PowerState::SLEEP, pms->GetState());

    POWER_HILOGD(LABEL_TEST, "PowerMgrMock077:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock077: end.";
}

/**
 * @tc.name: PowerMgrMock078
 * @tc.desc: test proximity RunningLock by mock
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrSTMockTest, PowerMgrMock078, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerMgrMock078: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrMock078:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock078: Failed to get PowerMgrService";
    }

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL);
    pms->CreateRunningLock(token, info);
    EXPECT_CALL(*g_lockAction, Lock(_, _)).Times(1);
    pms->Lock(token, info, 0);
    EXPECT_EQ(pms->IsUsed(token), true);

    EXPECT_CALL(*g_stateAction, GoToSleep(_, _, true)).Times(1).WillOnce(::testing::Return(ActionResult::SUCCESS));
    pms->ForceSuspendDevice(0);
    EXPECT_EQ(PowerState::SLEEP, pms->GetState());

    EXPECT_CALL(*g_lockAction, Unlock(_, _)).Times(1);
    pms->UnLock(token);
    EXPECT_EQ(pms->IsUsed(token), false);

    POWER_HILOGD(LABEL_TEST, "PowerMgrMock078:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock078: end.";
}

/**
 * @tc.name: PowerMgrMock079
 * @tc.desc: test proximity RunningLock by mock
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrSTMockTest, PowerMgrMock079, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerMgrMock079: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrMock079:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock079: Failed to get PowerMgrService";
    }

    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL);
    pms->CreateRunningLock(token, info);

    EXPECT_CALL(*g_lockAction, Lock(_, _)).Times(1);
    pms->Lock(token, info, 0);
    EXPECT_EQ(pms->IsUsed(token), true);

    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN, std::string("test"));
    EXPECT_EQ(PowerState::AWAKE, pms->GetState());

    POWER_HILOGD(LABEL_TEST, "PowerMgrMock079:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock079: end.";
}

/**
 * @tc.name: PowerMgrMock080
 * @tc.desc: test Auto SuspendDevice by mock after 5s
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrSTMockTest, PowerMgrMock080, TestSize.Level2)
{
    int64_t time = 50;
    GTEST_LOG_(INFO) << "PowerMgrMock080: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrMock080:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock080: Failed to get PowerMgrService";
    }

    pms->SetDisplayOffTime(time);
    ON_CALL(*g_stateAction, GetDisplayState()).WillByDefault(::testing::Return(DisplayState::DISPLAY_ON));
    sleep(((time / TRANSFER_MS_TO_S) * DOUBLE_TIMES / TEST_RATE) + ONE_SECOND);
    ON_CALL(*g_stateAction, GetDisplayState()).WillByDefault(::testing::Return(DisplayState::DISPLAY_DIM));
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    sleep(time / TRANSFER_MS_TO_S / TEST_RATE + ONE_SECOND);
    EXPECT_CALL(*g_stateAction, GoToSleep(_, _, false)).Times(1).WillOnce(::testing::Return(ActionResult::SUCCESS));
    ON_CALL(*g_stateAction, GetDisplayState()).WillByDefault(::testing::Return(DisplayState::DISPLAY_OFF));
    sleep(SLEEP_WAIT_TIME_S + ONE_SECOND);

    POWER_HILOGD(LABEL_TEST, "PowerMgrMock080:End.");
    pms->SetDisplayOffTime(DEFAULT_DISPLAY_OFF_TIME);
    GTEST_LOG_(INFO) << "PowerMgrMock080: end.";
}
}
