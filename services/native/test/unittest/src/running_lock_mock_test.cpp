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

#include "running_lock_mock_test.h"

#include "mock_lock_action.h"
#include "mock_power_action.h"
#include "mock_state_action.h"
#include "power_common.h"
#include "power_mgr_service.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;
using ::testing::_;

namespace {
const std::string RUNNINGLOCK_BACKGROUND_NAME = "OHOS.RunningLock.Background";
constexpr int32_t RUNNINGLOCKPARAM_TIMEOUTMS_DEF = -1;
} // namespace
static sptr<PowerMgrService> g_powerService;
static MockStateAction* g_powerStateAction;
static MockStateAction* g_shutdownStateAction;
static MockPowerAction* g_powerAction;
static MockLockAction* g_lockAction;

static void ResetMockAction()
{
    g_powerStateAction = new MockStateAction();
    g_shutdownStateAction = new MockStateAction();
    g_powerAction = new MockPowerAction();
    g_lockAction = new MockLockAction();
    g_powerService->EnableMock(g_powerStateAction, g_shutdownStateAction, g_powerAction, g_lockAction);
}

void RunningLockMockTest::SetUpTestCase(void)
{
    // create singleton service object at the beginning
    g_powerService = DelayedSpSingleton<PowerMgrService>::GetInstance();
    g_powerService->OnStart();
}

void RunningLockMockTest::TearDownTestCase(void)
{
    g_powerService->OnStop();
    DelayedSpSingleton<PowerMgrService>::DestroyInstance();
}

void RunningLockMockTest::SetUp(void)
{
    ResetMockAction();
}

namespace {
/**
 * @tc.name: RunningLockMockTest001
 * @tc.desc: test proximity screen control runninglock by mock
 * @tc.type: FUNC
 * @tc.require: issueI6LPK9
 */
HWTEST_F (RunningLockMockTest, RunningLockMockTest001, TestSize.Level2)
{
    ASSERT_NE(g_powerService, nullptr);
    ASSERT_NE(g_lockAction, nullptr);

    RunningLockInfo runninglockInfo(
        "RunningLockMockProximity3.1", RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL);
    RunningLockInfo runninglockInfo2(
        "RunningLockMockProximity4.2", RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL);
    int32_t timeoutMs = 100;
    auto runningLockMgr = g_powerService->GetRunningLockMgr();
    uint32_t lockActionCount = 0;
    uint32_t unlockActionCount = 0;

    EXPECT_CALL(*g_lockAction, Lock(_)).WillRepeatedly([&](const RunningLockParam& param) {
            EXPECT_EQ(param.name, RUNNINGLOCK_BACKGROUND_NAME);
            EXPECT_EQ(param.type, RunningLockType::RUNNINGLOCK_BACKGROUND);
            EXPECT_EQ(param.timeoutMs, RUNNINGLOCKPARAM_TIMEOUTMS_DEF);
            lockActionCount++;
        });
    EXPECT_CALL(*g_lockAction, Unlock(_)).WillRepeatedly([&](const RunningLockParam& param) {
            EXPECT_EQ(param.name, RUNNINGLOCK_BACKGROUND_NAME);
            EXPECT_EQ(param.type, RunningLockType::RUNNINGLOCK_BACKGROUND);
            unlockActionCount++;
        });

    sptr<IRemoteObject> runninglockToken = new RunningLockTokenStub();
    sptr<IRemoteObject> runninglockToken2 = new RunningLockTokenStub();
    EXPECT_EQ(PowerErrors::ERR_OK, g_powerService->CreateRunningLock(runninglockToken, runninglockInfo));
    EXPECT_EQ(PowerErrors::ERR_OK, g_powerService->CreateRunningLock(runninglockToken2, runninglockInfo2));

    g_powerService->Lock(runninglockToken, timeoutMs);
    EXPECT_EQ(1, runningLockMgr->GetValidRunningLockNum(runninglockInfo.type));
    g_powerService->Lock(runninglockToken2, timeoutMs);
    EXPECT_EQ(2, runningLockMgr->GetValidRunningLockNum(runninglockInfo2.type));

    g_powerService->UnLock(runninglockToken);
    EXPECT_EQ(1, runningLockMgr->GetValidRunningLockNum(runninglockInfo.type));
    g_powerService->UnLock(runninglockToken2);
    EXPECT_EQ(0, runningLockMgr->GetValidRunningLockNum(runninglockInfo2.type));

    g_powerService->ReleaseRunningLock(runninglockToken);
    g_powerService->ReleaseRunningLock(runninglockToken2);

    EXPECT_EQ(lockActionCount, 1);
    EXPECT_EQ(unlockActionCount, 1);
}

/**
 * @tc.name: RunningLockMockTest002
 * @tc.desc: test proximity screen control runninglock release function by mock
 * @tc.type: FUNC
 * @tc.require: issueI6LPK9
 */
HWTEST_F (RunningLockMockTest, RunningLockMockTest002, TestSize.Level2)
{
    ASSERT_NE(g_powerService, nullptr);
    ASSERT_NE(g_lockAction, nullptr);

    RunningLockInfo runninglockInfo(
        "RunningLockMockProximity4.1", RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL);
    int32_t timeoutMs = 100;
    auto runningLockMgr = g_powerService->GetRunningLockMgr();
    uint32_t lockActionCount = 0;
    uint32_t unlockActionCount = 0;

    EXPECT_CALL(*g_lockAction, Lock(_)).WillRepeatedly([&](const RunningLockParam& param) {
            EXPECT_EQ(param.name, RUNNINGLOCK_BACKGROUND_NAME);
            EXPECT_EQ(param.type, RunningLockType::RUNNINGLOCK_BACKGROUND);
            EXPECT_EQ(param.timeoutMs, RUNNINGLOCKPARAM_TIMEOUTMS_DEF);
            lockActionCount++;
        });
    EXPECT_CALL(*g_lockAction, Unlock(_)).WillRepeatedly([&](const RunningLockParam& param) {
            EXPECT_EQ(param.name, RUNNINGLOCK_BACKGROUND_NAME);
            EXPECT_EQ(param.type, RunningLockType::RUNNINGLOCK_BACKGROUND);
            unlockActionCount++;
        });

    sptr<IRemoteObject> runninglockToken = new RunningLockTokenStub();
    EXPECT_EQ(PowerErrors::ERR_OK, g_powerService->CreateRunningLock(runninglockToken, runninglockInfo));

    g_powerService->Lock(runninglockToken, timeoutMs);
    EXPECT_EQ(1, runningLockMgr->GetValidRunningLockNum(runninglockInfo.type));
    g_powerService->ReleaseRunningLock(runninglockToken);

    EXPECT_EQ(lockActionCount, 1);
    EXPECT_EQ(unlockActionCount, 1);
}

/**
 * @tc.name: RunningLockMockTest003
 * @tc.desc: test scene runninglock by mock
 * @tc.type: FUNC
 * @tc.require: issueI6LPK9
 */
HWTEST_F (RunningLockMockTest, RunningLockMockTest003, TestSize.Level2)
{
    ASSERT_NE(g_powerService, nullptr);
    ASSERT_NE(g_lockAction, nullptr);

    RunningLockInfo runninglockPhone("RunningLockMockPhone5.1", RunningLockType::RUNNINGLOCK_BACKGROUND_PHONE);
    RunningLockInfo runninglockNotify("RunningLockMockNotify5.1", RunningLockType::RUNNINGLOCK_BACKGROUND_NOTIFICATION);
    int32_t timeoutMs = 100;
    uint32_t lockActionCount = 0;
    uint32_t unlockActionCount = 0;

    auto GetRunningLockInfo = [&](RunningLockType type) {
        if (type == RunningLockType::RUNNINGLOCK_BACKGROUND_PHONE) {
            return runninglockPhone;
        }
        return runninglockNotify;
    };

    EXPECT_CALL(*g_lockAction, Lock(_)).WillRepeatedly([&](const RunningLockParam& param) {
            RunningLockInfo runninglockInfo = GetRunningLockInfo(param.type);
            EXPECT_EQ(param.name, runninglockInfo.name);
            EXPECT_EQ(param.type, runninglockInfo.type);
            EXPECT_EQ(param.timeoutMs, timeoutMs);
            lockActionCount++;
        });
    EXPECT_CALL(*g_lockAction, Unlock(_)).WillRepeatedly([&](const RunningLockParam& param) {
            RunningLockInfo runninglockInfo = GetRunningLockInfo(param.type);
            EXPECT_EQ(param.name, runninglockInfo.name);
            EXPECT_EQ(param.type, runninglockInfo.type);
            unlockActionCount++;
        });

    sptr<IRemoteObject> phoneToken = new RunningLockTokenStub();
    sptr<IRemoteObject> notifyToken = new RunningLockTokenStub();
    EXPECT_EQ(PowerErrors::ERR_OK, g_powerService->CreateRunningLock(phoneToken, runninglockPhone));
    EXPECT_EQ(PowerErrors::ERR_OK, g_powerService->CreateRunningLock(notifyToken, runninglockNotify));

    g_powerService->Lock(phoneToken, timeoutMs);
    g_powerService->Lock(notifyToken, timeoutMs);
    g_powerService->UnLock(phoneToken);
    g_powerService->UnLock(notifyToken);

    g_powerService->ReleaseRunningLock(phoneToken);
    g_powerService->ReleaseRunningLock(notifyToken);

    EXPECT_EQ(lockActionCount, 2);
    EXPECT_EQ(unlockActionCount, 4);
}

/**
 * @tc.name: RunningLockMockTest004
 * @tc.desc: test scene runninglock by mock
 * @tc.type: FUNC
 * @tc.require: issueI6LPK9
 */
HWTEST_F (RunningLockMockTest, RunningLockMockTest004, TestSize.Level2)
{
    ASSERT_NE(g_powerService, nullptr);
    ASSERT_NE(g_lockAction, nullptr);

    RunningLockInfo runninglockAudio("RunningLockMockAudio6.1", RunningLockType::RUNNINGLOCK_BACKGROUND_AUDIO);
    RunningLockInfo runninglockSport("RunningLockMockSport6.1", RunningLockType::RUNNINGLOCK_BACKGROUND_SPORT);
    int32_t timeoutMs = 100;
    uint32_t lockActionCount = 0;
    uint32_t unlockActionCount = 0;

    auto GetRunningLockInfo = [&](RunningLockType type) {
        if (type == RunningLockType::RUNNINGLOCK_BACKGROUND_AUDIO) {
            return runninglockAudio;
        }
        return runninglockSport;
    };

    EXPECT_CALL(*g_lockAction, Lock(_)).WillRepeatedly([&](const RunningLockParam& param) {
            RunningLockInfo runninglockInfo = GetRunningLockInfo(param.type);
            EXPECT_EQ(param.name, runninglockInfo.name);
            EXPECT_EQ(param.type, runninglockInfo.type);
            EXPECT_EQ(param.timeoutMs, timeoutMs);
            lockActionCount++;
        });
    EXPECT_CALL(*g_lockAction, Unlock(_)).WillRepeatedly([&](const RunningLockParam& param) {
            RunningLockInfo runninglockInfo = GetRunningLockInfo(param.type);
            EXPECT_EQ(param.name, runninglockInfo.name);
            EXPECT_EQ(param.type, runninglockInfo.type);
            unlockActionCount++;
        });

    sptr<IRemoteObject> audioToken = new RunningLockTokenStub();
    sptr<IRemoteObject> sportToken = new RunningLockTokenStub();
    EXPECT_EQ(PowerErrors::ERR_OK, g_powerService->CreateRunningLock(audioToken, runninglockAudio));
    EXPECT_EQ(PowerErrors::ERR_OK, g_powerService->CreateRunningLock(sportToken, runninglockSport));

    g_powerService->Lock(audioToken, timeoutMs);
    g_powerService->Lock(sportToken, timeoutMs);
    g_powerService->UnLock(audioToken);
    g_powerService->UnLock(sportToken);

    g_powerService->ReleaseRunningLock(audioToken);
    g_powerService->ReleaseRunningLock(sportToken);

    EXPECT_EQ(lockActionCount, 2);
    EXPECT_EQ(unlockActionCount, 4);
}

/**
 * @tc.name: RunningLockMockTest005
 * @tc.desc: test scene runninglock by mock
 * @tc.type: FUNC
 * @tc.require: issueI6LPK9
 */
HWTEST_F (RunningLockMockTest, RunningLockMockTest005, TestSize.Level2)
{
    ASSERT_NE(g_powerService, nullptr);
    ASSERT_NE(g_lockAction, nullptr);

    RunningLockInfo runninglockNavi("RunningLockMockNavi7.1", RunningLockType::RUNNINGLOCK_BACKGROUND_NAVIGATION);
    RunningLockInfo runninglockTask("RunningLockMockTask7.1", RunningLockType::RUNNINGLOCK_BACKGROUND_TASK);
    int32_t timeoutMs = 100;
    uint32_t lockActionCount = 0;
    uint32_t unlockActionCount = 0;

    auto GetRunningLockInfo = [&](RunningLockType type) {
        if (type == RunningLockType::RUNNINGLOCK_BACKGROUND_NAVIGATION) {
            return runninglockNavi;
        }
        return runninglockTask;
    };

    EXPECT_CALL(*g_lockAction, Lock(_)).WillRepeatedly([&](const RunningLockParam& param) {
            auto runninglockInfo = GetRunningLockInfo(param.type);
            EXPECT_EQ(param.name, runninglockInfo.name);
            EXPECT_EQ(param.type, runninglockInfo.type);
            EXPECT_EQ(param.timeoutMs, timeoutMs);
            lockActionCount++;
        });
    EXPECT_CALL(*g_lockAction, Unlock(_)).WillRepeatedly([&](const RunningLockParam& param) {
            auto runninglockInfo = GetRunningLockInfo(param.type);
            EXPECT_EQ(param.name, runninglockInfo.name);
            EXPECT_EQ(param.type, runninglockInfo.type);
            unlockActionCount++;
        });

    sptr<IRemoteObject> naviToken = new RunningLockTokenStub();
    sptr<IRemoteObject> taskToken = new RunningLockTokenStub();

    EXPECT_EQ(PowerErrors::ERR_OK, g_powerService->CreateRunningLock(naviToken, runninglockNavi));
    EXPECT_EQ(PowerErrors::ERR_OK, g_powerService->CreateRunningLock(taskToken, runninglockTask));

    g_powerService->Lock(naviToken, timeoutMs);
    g_powerService->Lock(taskToken, timeoutMs);
    g_powerService->UnLock(naviToken);
    g_powerService->UnLock(taskToken);

    g_powerService->ReleaseRunningLock(naviToken);
    g_powerService->ReleaseRunningLock(taskToken);

    EXPECT_EQ(lockActionCount, 2);
    EXPECT_EQ(unlockActionCount, 4);
}

/**
 * @tc.name: RunningLockMockTest006
 * @tc.desc: test scene runninglock release function by mock
 * @tc.type: FUNC
 * @tc.require: issueI6LPK9
 */
HWTEST_F (RunningLockMockTest, RunningLockMockTest006, TestSize.Level2)
{
    ASSERT_NE(g_powerService, nullptr);
    ASSERT_NE(g_lockAction, nullptr);

    RunningLockInfo runninglockPhone("RunningLockMockPhone8.1", RunningLockType::RUNNINGLOCK_BACKGROUND_PHONE);
    RunningLockInfo runninglockNotify("RunningLockMockNotify8.1", RunningLockType::RUNNINGLOCK_BACKGROUND_NOTIFICATION);
    RunningLockInfo runninglockAudio("RunningLockMockAudio8.1", RunningLockType::RUNNINGLOCK_BACKGROUND_AUDIO);
    int32_t timeoutMs = 100;
    uint32_t lockActionCount = 0;
    uint32_t unlockActionCount = 0;

    auto GetRunningLockInfo = [&](RunningLockType type) {
        RunningLockInfo lockInfo {};
        switch (type) {
            case RunningLockType::RUNNINGLOCK_BACKGROUND_PHONE:
                return runninglockPhone;
            case RunningLockType::RUNNINGLOCK_BACKGROUND_NOTIFICATION:
                return runninglockNotify;
            case RunningLockType::RUNNINGLOCK_BACKGROUND_AUDIO:
                return runninglockAudio;
            default:
                return lockInfo;
        }
    };

    EXPECT_CALL(*g_lockAction, Lock(_)).WillRepeatedly([&](const RunningLockParam& param) {
            lockActionCount++;
        });
    EXPECT_CALL(*g_lockAction, Unlock(_)).WillRepeatedly([&](const RunningLockParam& param) {
            RunningLockInfo runninglockInfo = GetRunningLockInfo(param.type);
            EXPECT_EQ(param.name, runninglockInfo.name);
            EXPECT_EQ(param.type, runninglockInfo.type);
            unlockActionCount++;
        });

    sptr<IRemoteObject> phoneToken = new RunningLockTokenStub();
    sptr<IRemoteObject> notifyToken = new RunningLockTokenStub();
    sptr<IRemoteObject> audioToken = new RunningLockTokenStub();

    EXPECT_EQ(PowerErrors::ERR_OK, g_powerService->CreateRunningLock(phoneToken, runninglockPhone));
    EXPECT_EQ(PowerErrors::ERR_OK, g_powerService->CreateRunningLock(notifyToken, runninglockNotify));
    EXPECT_EQ(PowerErrors::ERR_OK, g_powerService->CreateRunningLock(audioToken, runninglockAudio));

    g_powerService->Lock(phoneToken, timeoutMs);
    g_powerService->Lock(notifyToken, timeoutMs);
    g_powerService->Lock(audioToken, timeoutMs);

    g_powerService->ReleaseRunningLock(phoneToken);
    g_powerService->ReleaseRunningLock(notifyToken);
    g_powerService->ReleaseRunningLock(audioToken);

    EXPECT_EQ(lockActionCount, 3);
    EXPECT_EQ(unlockActionCount, 3);
}

/**
 * @tc.name: RunningLockMockTest007
 * @tc.desc: test scene runninglock release function by mock
 * @tc.type: FUNC
 * @tc.require: issueI6LPK9
 */
HWTEST_F (RunningLockMockTest, RunningLockMockTest007, TestSize.Level2)
{
    ASSERT_NE(g_powerService, nullptr);
    ASSERT_NE(g_lockAction, nullptr);

    RunningLockInfo runninglockSport("RunningLockMockSport9.1", RunningLockType::RUNNINGLOCK_BACKGROUND_SPORT);
    RunningLockInfo runninglockNavi("RunningLockMockNavi9.1", RunningLockType::RUNNINGLOCK_BACKGROUND_NAVIGATION);
    RunningLockInfo runninglockTask("RunningLockMockTask9.1", RunningLockType::RUNNINGLOCK_BACKGROUND_TASK);
    int32_t timeoutMs = 100;
    uint32_t lockActionCount = 0;
    uint32_t unlockActionCount = 0;
    
    auto GetRunningLockInfo = [&](RunningLockType type) {
        RunningLockInfo lockInfo {};
        switch (type) {
            case RunningLockType::RUNNINGLOCK_BACKGROUND_SPORT:
                return runninglockSport;
            case RunningLockType::RUNNINGLOCK_BACKGROUND_NAVIGATION:
                return runninglockNavi;
            case RunningLockType::RUNNINGLOCK_BACKGROUND_TASK:
                return runninglockTask;
            default:
                return lockInfo;
        }
    };

    EXPECT_CALL(*g_lockAction, Lock(_)).WillRepeatedly([&](const RunningLockParam& param) {
            lockActionCount++;
        });
    EXPECT_CALL(*g_lockAction, Unlock(_)).WillRepeatedly([&](const RunningLockParam& param) {
            RunningLockInfo runninglockInfo = GetRunningLockInfo(param.type);
            EXPECT_EQ(param.name, runninglockInfo.name);
            EXPECT_EQ(param.type, runninglockInfo.type);
            unlockActionCount++;
        });

    sptr<IRemoteObject> sportToken = new RunningLockTokenStub();
    sptr<IRemoteObject> naviToken = new RunningLockTokenStub();
    sptr<IRemoteObject> taskToken = new RunningLockTokenStub();

    EXPECT_EQ(PowerErrors::ERR_OK, g_powerService->CreateRunningLock(sportToken, runninglockSport));
    EXPECT_EQ(PowerErrors::ERR_OK, g_powerService->CreateRunningLock(naviToken, runninglockNavi));
    EXPECT_EQ(PowerErrors::ERR_OK, g_powerService->CreateRunningLock(taskToken, runninglockTask));

    g_powerService->Lock(sportToken, timeoutMs);
    g_powerService->Lock(naviToken, timeoutMs);
    g_powerService->Lock(taskToken, timeoutMs);

    g_powerService->ReleaseRunningLock(sportToken);
    g_powerService->ReleaseRunningLock(naviToken);
    g_powerService->ReleaseRunningLock(taskToken);

    EXPECT_EQ(lockActionCount, 3);
    EXPECT_EQ(unlockActionCount, 3);
}
}
