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

#include "power_mgr_mock_system_test.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;
using ::testing::_;

static sptr<PowerMgrService> service;
static MockStateAction* stateAction;
static MockPowerAction* powerAction;
static MockLockAction* lockAction;

static void ResetMockAction()
{
    POWER_HILOGD(MODULE_SERVICE, "ResetMockAction:Start.");
    stateAction = new MockStateAction();
    powerAction = new MockPowerAction();
    lockAction = new MockLockAction();
    service->EnableMock(stateAction, powerAction, lockAction);
}

void PowerMgrMockSystemTest::SetUpTestCase(void)
{
    // create singleton service object at the beginning
    service = DelayedSpSingleton<PowerMgrService>::GetInstance();
    service->OnStart();
    ResetMockAction();
}

void PowerMgrMockSystemTest::TearDownTestCase(void)
{
    service->OnStop();
    DelayedSpSingleton<PowerMgrService>::DestroyInstance();
    delete stateAction;
    delete powerAction;
    delete lockAction;
}

void PowerMgrMockSystemTest::SetUp(void)
{
}

void PowerMgrMockSystemTest::TearDown(void)
{
}

namespace {
/**
 * @tc.name: PowerMgrMock102
 * @tc.desc: test SuspendDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockSystemTest, PowerMgrMock102, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock102: start.";

    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock102:Start.");
    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock102: Failed to get PowerMgrService";
    }

    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock102:Start mock.");
    EXPECT_CALL(*stateAction, SetDisplayState(DisplayState::DISPLAY_SUSPEND))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*stateAction,
        Suspend(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false));
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock102:Start suspend.");
    pms->SetDisplaySuspend(true);
    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock102:end.");
    GTEST_LOG_(INFO) << "PowerMgrMock102: end.";
}

/**
 * @tc.name: PowerMgrMock103
 * @tc.desc: test SuspendDevice and auto sleep by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockSystemTest, PowerMgrMock103, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock103: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock103:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock022: Failed to get PowerMgrService";
    }

    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN, std::string("test"));
    EXPECT_CALL(*stateAction, SetDisplayState(DisplayState::DISPLAY_SUSPEND))
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*stateAction,
        Suspend(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false));
    pms->SetDisplaySuspend(true);
    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    sleep(SLEEP_WAIT_TIME_S + 1);

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock103:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock103: end.";
}

/**
 * @tc.name: PowerMgrMock104
 * @tc.desc: test WakeupDevice and auto suspend  by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockSystemTest, PowerMgrMock104, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock104: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock104:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock104: Failed to get PowerMgrService";
    }

    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN, std::string("test"));
    pms->SetDisplaySuspend(true);
    ON_CALL(*stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_ON));
    sleep((SCREEN_OFF_WAIT_TIME_S*2/3) + 1);
    ON_CALL(*stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_DIM));
    EXPECT_CALL(*stateAction, SetDisplayState(DisplayState::DISPLAY_SUSPEND))
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    sleep((SCREEN_OFF_WAIT_TIME_S*1/3) + 1);

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock104:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock104: end.";
}

/**
 * @tc.name: PowerMgrMock105
 * @tc.desc: test Auto SuspendDevice by mock after 30s
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockSystemTest, PowerMgrMock105, TestSize.Level2)
{
    int64_t time =30000;
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock105: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock105:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock105: Failed to get PowerMgrService";
    }

    pms->SetDisplayOffTime(time);
    pms->SetDisplaySuspend(true);
    ON_CALL(*stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_ON));
    sleep(((time/1000)*2/3)+1);
    ON_CALL(*stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_DIM));
    EXPECT_CALL(*stateAction, SetDisplayState(DisplayState::DISPLAY_SUSPEND))
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    sleep(((time/1000)*1/3)+1);


    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock105:End");
    pms->SetDisplayOffTime(DEFAULT_DISPLAY_OFF_TIME);
    GTEST_LOG_(INFO) << "PowerMgrMock105: end.";
}

/**
 * @tc.name: PowerMgrMock106
 * @tc.desc: test proximity RunningLock by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockSystemTest, PowerMgrMock106, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock106: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock106:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock106: Failed to get PowerMgrService";
    }

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL);
    pms->CreateRunningLock(token, info);
    pms->SetDisplaySuspend(true);
    pms->Lock(token, info, 0);
    EXPECT_EQ(pms->IsUsed(token), true);
    sleep(SLEEP_WAIT_TIME_S*10);
    EXPECT_EQ(PowerState::AWAKE, pms->GetState());
    EXPECT_CALL(*stateAction, SetDisplayState(DisplayState::DISPLAY_SUSPEND))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*stateAction, GoToSleep(_, _, _)).Times(0);
    pms->MockProximity(RunningLockMgr::PROXIMITY_CLOSE);
    EXPECT_EQ(PowerState::INACTIVE, pms->GetState());

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock106:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock106: end.";
}

/**
 * @tc.name: PowerMgrMock107
 * @tc.desc: test background RunningLock by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockSystemTest, PowerMgrMock107, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock107: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock107:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock068: Failed to get PowerMgrService";
    }

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_BACKGROUND);
    ON_CALL(*stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_ON));
    sleep((SCREEN_OFF_WAIT_TIME_S*2/3) + 1);
    ON_CALL(*stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_DIM));
    pms->SetDisplaySuspend(true);
    pms->CreateRunningLock(token, info);
    EXPECT_CALL(*stateAction, SetDisplayState(DisplayState::DISPLAY_SUSPEND))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*stateAction, GoToSleep(_, _, _)).Times(0);
    pms->Lock(token, info, 0);
    EXPECT_EQ(pms->IsUsed(token), true);
    sleep((SCREEN_OFF_WAIT_TIME_S*1/3) + 1);
    pms->UnLock(token);

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock107:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock107: end.";
}
}