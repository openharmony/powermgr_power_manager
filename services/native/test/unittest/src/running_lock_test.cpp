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

#include "running_lock_test.h"

#include <ipc_skeleton.h>

#include "actions/irunning_lock_action.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;
namespace {
constexpr int32_t US_PER_MS = 1000;
} // namespace
sptr<PowerMgrService> RunningLockTest::pmsTest_ = nullptr;
std::shared_ptr<RunningLockMgr> RunningLockTest::runningLockMgr_ = nullptr;

namespace {
/**
 * @tc.name: RunningLockInnerKit000
 * @tc.desc: Test RunningLockInnerKit function, connect PowerMgrService and call member function.
 * @tc.type: FUNC
 */
HWTEST_F (RunningLockTest, RunningLockInnerKit000, TestSize.Level0)
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    auto runningLock1 = powerMgrClient.CreateRunningLock("runninglock1", RunningLockType::RUNNINGLOCK_SCREEN);
    ASSERT_TRUE(runningLock1 != nullptr);

    ASSERT_TRUE(!runningLock1->IsUsed()) << "runningLock1->IsUsed() != false";
    runningLock1->Lock();
    ASSERT_TRUE(runningLock1->IsUsed()) << "runningLock1->IsUsed() != true";

    runningLock1->UnLock();
    ASSERT_TRUE(!runningLock1->IsUsed()) << "runningLock1->IsUsed() != false";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnitTest::RunningLockInnerKit01 end");
}

/**
 * @tc.name: RunningLockInnerKit002
 * @tc.desc: Test RunningLockInnerKit function, timeout lock.
 * @tc.type: FUNC
 */
HWTEST_F (RunningLockTest, RunningLockInnerKit002, TestSize.Level1)
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    auto runningLock1 = powerMgrClient.CreateRunningLock("runninglock1", RunningLockType::RUNNINGLOCK_SCREEN);
    ASSERT_TRUE(runningLock1 != nullptr);
    ASSERT_TRUE(!runningLock1->IsUsed()) << "runningLock1->IsUsed() != false";
    runningLock1->Lock();
    ASSERT_TRUE(runningLock1->IsUsed()) << "runningLock1->IsUsed() != true";
    runningLock1->UnLock();
    ASSERT_TRUE(!runningLock1->IsUsed()) << "runningLock1->IsUsed() != false";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnitTest::RunningLockInnerKit003 1");
    // lock 50ms
    runningLock1->Lock(50);
    usleep(4000);
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnitTest::RunningLockInnerKit003 2");
    ASSERT_TRUE(runningLock1->IsUsed()) << "runningLock1->IsUsed() != true";
    usleep(1000);
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnitTest::RunningLockInnerKit003 3");
    ASSERT_TRUE(runningLock1->IsUsed()) << "runningLock1->IsUsed() != true";
    // wait 60ms
    usleep(60000);
    ASSERT_TRUE(!runningLock1->IsUsed()) << "runningLock1->IsUsed() != false";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnitTest::RunningLockInnerKit003 end");
}

/**
 * @tc.name: RunningLockInnerKit004
 * @tc.desc: Test RunningLockInnerKit function, timeout lock.
 * @tc.type: FUNC
 */
HWTEST_F (RunningLockTest, RunningLockInnerKit004, TestSize.Level1)
{
    if (false) {
        auto& powerMgrClient = PowerMgrClient::GetInstance();
        auto runningLock1 = powerMgrClient.CreateRunningLock("runninglock005",
            RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL);
        ASSERT_TRUE(runningLock1 != nullptr);
        ASSERT_TRUE(!runningLock1->IsUsed()) << "runningLock1->IsUsed() != false";
        // after 8ms unlock
        runningLock1->Lock(30);
        runningLock1->Lock(80);
        POWER_HILOGD(LABEL_TEST, "PowerMgrUnitTest::RunningLockInnerKit005 1");
        usleep(50000);
        ASSERT_TRUE(runningLock1->IsUsed()) << "runningLock1->IsUsed() != true";
        usleep(50000);
        ASSERT_TRUE(!runningLock1->IsUsed()) << "runningLock1->IsUsed() != false";
        // no unlock
        POWER_HILOGD(LABEL_TEST, "PowerMgrUnitTest::RunningLockInnerKit005 2");
        runningLock1->Lock(2);
        runningLock1->Lock(3);
        runningLock1->Lock();
        POWER_HILOGD(LABEL_TEST, "PowerMgrUnitTest::RunningLockInnerKit005 3");
        usleep(8000);
        ASSERT_TRUE(runningLock1->IsUsed()) << "runningLock1->IsUsed() != true";
        // after 3ms unlock
        runningLock1->Lock(30);
        POWER_HILOGD(LABEL_TEST, "PowerMgrUnitTest::RunningLockInnerKit005 4");
        usleep(50000);
        ASSERT_TRUE(!runningLock1->IsUsed()) << "runningLock1->IsUsed() != false";
        runningLock1->Lock(5);
        runningLock1->UnLock();
        ASSERT_TRUE(!runningLock1->IsUsed()) << "runningLock1->IsUsed() != false";
        POWER_HILOGD(LABEL_TEST, "PowerMgrUnitTest::RunningLockInnerKit005 5");
    }
}

/**
 * @tc.name: RunningLockInnerKit006
 * @tc.desc: Test RunningLock function, running lock type is phone
 * @tc.type: FUNC
 * @tc.require: issueI6IU1G
 */
HWTEST_F (RunningLockTest, RunningLockInnerKit006, TestSize.Level0)
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    std::string runningLockName = "running.lock.phone";
    uint32_t timeOutMs = 30;
    uint32_t waitTimeOutMs = timeOutMs + 10;
    auto runningLock = powerMgrClient.CreateRunningLock
        (runningLockName, RunningLockType::RUNNINGLOCK_BACKGROUND_PHONE);
    ASSERT_TRUE(runningLock != nullptr);
    EXPECT_FALSE(runningLock->IsUsed());

    runningLock->Lock();
    EXPECT_TRUE(runningLock->IsUsed());

    runningLock->UnLock();
    EXPECT_FALSE(runningLock->IsUsed());

    runningLock->Lock(timeOutMs);
    EXPECT_TRUE(runningLock->IsUsed());

    usleep(waitTimeOutMs * US_PER_MS);

    EXPECT_FALSE(runningLock->IsUsed());
}

/**
 * @tc.name: RunningLockInnerKit007
 * @tc.desc: Test RunningLock function, running lock type is notification
 * @tc.type: FUNC
 * @tc.require: issueI6IU1G
 */
HWTEST_F (RunningLockTest, RunningLockInnerKit007, TestSize.Level0)
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    std::string runningLockName = "running.lock.notification";
    uint32_t timeOutMs = 30;
    uint32_t waitTimeOutMs = timeOutMs + 10;
    auto runningLock = powerMgrClient.CreateRunningLock
        (runningLockName, RunningLockType::RUNNINGLOCK_BACKGROUND_NOTIFICATION);
    ASSERT_TRUE(runningLock != nullptr);
    EXPECT_FALSE(runningLock->IsUsed());

    runningLock->Lock();
    EXPECT_TRUE(runningLock->IsUsed());

    runningLock->UnLock();
    EXPECT_FALSE(runningLock->IsUsed());

    runningLock->Lock(timeOutMs);
    EXPECT_TRUE(runningLock->IsUsed());

    usleep(waitTimeOutMs * US_PER_MS);

    EXPECT_FALSE(runningLock->IsUsed());
}

/**
 * @tc.name: RunningLockInnerKit008
 * @tc.desc: Test RunningLock function, running lock type is audio
 * @tc.type: FUNC
 * @tc.require: issueI6IU1G
 */
HWTEST_F (RunningLockTest, RunningLockInnerKit008, TestSize.Level0)
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    std::string runningLockName = "running.lock.audio";
    uint32_t timeOutMs = 30;
    uint32_t waitTimeOutMs = timeOutMs + 10;
    auto runningLock = powerMgrClient.CreateRunningLock
        (runningLockName, RunningLockType::RUNNINGLOCK_BACKGROUND_AUDIO);
    ASSERT_TRUE(runningLock != nullptr);
    EXPECT_FALSE(runningLock->IsUsed());

    runningLock->Lock();
    EXPECT_TRUE(runningLock->IsUsed());

    runningLock->UnLock();
    EXPECT_FALSE(runningLock->IsUsed());

    runningLock->Lock(timeOutMs);
    EXPECT_TRUE(runningLock->IsUsed());

    usleep(waitTimeOutMs * US_PER_MS);

    EXPECT_FALSE(runningLock->IsUsed());
}

/**
 * @tc.name: RunningLockInnerKit009
 * @tc.desc: Test RunningLock function, running lock type is sport
 * @tc.type: FUNC
 * @tc.require: issueI6IU1G
 */
HWTEST_F (RunningLockTest, RunningLockInnerKit009, TestSize.Level0)
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    std::string runningLockName = "running.lock.sport";
    uint32_t timeOutMs = 30;
    uint32_t waitTimeOutMs = timeOutMs + 10;
    auto runningLock = powerMgrClient.CreateRunningLock
        (runningLockName, RunningLockType::RUNNINGLOCK_BACKGROUND_SPORT);
    ASSERT_TRUE(runningLock != nullptr);
    EXPECT_FALSE(runningLock->IsUsed());

    runningLock->Lock();
    EXPECT_TRUE(runningLock->IsUsed());

    runningLock->UnLock();
    EXPECT_FALSE(runningLock->IsUsed());

    runningLock->Lock(timeOutMs);
    EXPECT_TRUE(runningLock->IsUsed());

    usleep(waitTimeOutMs * US_PER_MS);

    EXPECT_FALSE(runningLock->IsUsed());
}

/**
 * @tc.name: RunningLockInnerKit010
 * @tc.desc: Test RunningLock function, running lock type is navigation
 * @tc.type: FUNC
 * @tc.require: issueI6IU1G
 */
HWTEST_F (RunningLockTest, RunningLockInnerKit010, TestSize.Level0)
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    std::string runningLockName = "running.lock.navigation";
    uint32_t timeOutMs = 30;
    uint32_t waitTimeOutMs = timeOutMs + 10;
    auto runningLock = powerMgrClient.CreateRunningLock
        (runningLockName, RunningLockType::RUNNINGLOCK_BACKGROUND_NAVIGATION);
    ASSERT_TRUE(runningLock != nullptr);
    EXPECT_FALSE(runningLock->IsUsed());

    runningLock->Lock();
    EXPECT_TRUE(runningLock->IsUsed());

    runningLock->UnLock();
    EXPECT_FALSE(runningLock->IsUsed());

    runningLock->Lock(timeOutMs);
    EXPECT_TRUE(runningLock->IsUsed());

    usleep(waitTimeOutMs * US_PER_MS);

    EXPECT_FALSE(runningLock->IsUsed());
}

/**
 * @tc.name: RunningLockInnerKit011
 * @tc.desc: Test RunningLock function, running lock type is navigation
 * @tc.type: FUNC
 * @tc.require: issueI6IU1G
 */
HWTEST_F (RunningLockTest, RunningLockInnerKit011, TestSize.Level0)
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    std::string runningLockName = "running.lock.task";
    uint32_t timeOutMs = 30;
    uint32_t waitTimeOutMs = timeOutMs + 10;
    auto runningLock = powerMgrClient.CreateRunningLock
        (runningLockName, RunningLockType::RUNNINGLOCK_BACKGROUND_TASK);
    ASSERT_TRUE(runningLock != nullptr);
    EXPECT_FALSE(runningLock->IsUsed());

    runningLock->Lock();
    EXPECT_TRUE(runningLock->IsUsed());

    runningLock->UnLock();
    EXPECT_FALSE(runningLock->IsUsed());

    runningLock->Lock(timeOutMs);
    EXPECT_TRUE(runningLock->IsUsed());

    usleep(waitTimeOutMs * US_PER_MS);

    EXPECT_FALSE(runningLock->IsUsed());
}

#ifdef IPC_AVAILABLE
/**
 * @tc.name: RunningLockInnerKit003
 * @tc.desc: Test RunningLockInnerKit function, timeout lock.
 * @tc.type: FUNC
 */
HWTEST_F (RunningLockTest, RunningLockInnerKit003, TestSize.Level0)
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    auto runningLock1 = powerMgrClient.CreateRunningLock("runninglock2", RunningLockType::RUNNINGLOCK_SCREEN);
    ASSERT_TRUE(runningLock1 != nullptr);
    ASSERT_TRUE(!runningLock1->IsUsed()) << "runningLock1->IsUsed() != false";
    // after 8ms unlock
    runningLock1->Lock(30);
    runningLock1->Lock(80);
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnitTest::RunningLockInnerKit004 1");
    usleep(50000);
    ASSERT_TRUE(runningLock1->IsUsed()) << "runningLock1->IsUsed() != true";
    usleep(50000);
    ASSERT_TRUE(!runningLock1->IsUsed()) << "runningLock1->IsUsed() != false";
    // no unlock
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnitTest::RunningLockInnerKit004 2");
    runningLock1->Lock(2);
    runningLock1->Lock(3);
    runningLock1->Lock();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnitTest::RunningLockInnerKit004 3");
    usleep(8000);
    ASSERT_TRUE(runningLock1->IsUsed()) << "runningLock1->IsUsed() != true";
    // after 3ms unlock
    runningLock1->Lock(30);
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnitTest::RunningLockInnerKit004 4");
    usleep(50000);
    ASSERT_TRUE(!runningLock1->IsUsed()) << "runningLock1->IsUsed() != false";
    runningLock1->Lock(5);
    runningLock1->UnLock();
    ASSERT_TRUE(!runningLock1->IsUsed()) << "runningLock1->IsUsed() != false";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnitTest::RunningLockInnerKit004 5");
}

/**
 * @tc.name: RunningLockMgr001
 * @tc.desc: Test RunningLockMgr function, connect PowerMgrService and call member function.
 * @tc.type: FUNC
 */
HWTEST_F (RunningLockTest, RunningLockMgr001, TestSize.Level0)
{
    sptr<IRemoteObject> token = new RunningLockTokenStub();
    ASSERT_TRUE(token != nullptr);
    sptr<IRemoteObject> token2 = new RunningLockTokenStub();
    ASSERT_TRUE(token2 != nullptr);
    RunningLockInfo runningLockInfo1 {"runninglockTest1", RunningLockType::RUNNINGLOCK_SCREEN};
    UserIPCInfo userIPCinfo {IPCSkeleton::GetCallingUid(), IPCSkeleton::GetCallingPid()};
    {
        runningLockMgr_->Lock(token, runningLockInfo1, userIPCinfo);
        TestRunningLockInnerExisit(token, runningLockInfo1);
        ASSERT_TRUE(1 == runningLockMgr_->GetRunningLockNum(RunningLockType::RUNNINGLOCK_SCREEN));
        ASSERT_TRUE(0 == runningLockMgr_->GetRunningLockNum(RunningLockType::RUNNINGLOCK_BACKGROUND));
    }
    RunningLockInfo runningLockInfo2 {"runninglockTest2", RunningLockType::RUNNINGLOCK_BACKGROUND};
    {
        runningLockMgr_->Lock(token2, runningLockInfo2, userIPCinfo);
        TestRunningLockInnerExisit(token2, runningLockInfo2);
        ASSERT_TRUE(1 == runningLockMgr_->GetRunningLockNum(RunningLockType::RUNNINGLOCK_SCREEN));
        ASSERT_TRUE(1 == runningLockMgr_->GetRunningLockNum(RunningLockType::RUNNINGLOCK_BACKGROUND));
    }
    {
        runningLockMgr_->UnLock(token);
        TestRunningLockInnerNoExisit(token);
        ASSERT_TRUE(0 == runningLockMgr_->GetRunningLockNum(RunningLockType::RUNNINGLOCK_SCREEN));
        ASSERT_TRUE(1 == runningLockMgr_->GetRunningLockNum(RunningLockType::RUNNINGLOCK_BACKGROUND));
    }
    {
        runningLockMgr_->UnLock(token2);
        TestRunningLockInnerNoExisit(token2);
        ASSERT_TRUE(0 == runningLockMgr_->GetRunningLockNum(RunningLockType::RUNNINGLOCK_SCREEN));
        ASSERT_TRUE(0 == runningLockMgr_->GetRunningLockNum(RunningLockType::RUNNINGLOCK_BACKGROUND));
    }
    POWER_HILOGD(LABEL_TEST, "RunningLockTest::RunningLockMgr001 end");
}

/**
 * @tc.name: RunningLockMgr003
 * @tc.desc: Test RunningLockMgr Proxy function.
 * @tc.type: FUNC
 */
HWTEST_F (RunningLockTest, RunningLockMgr003, TestSize.Level0)
{
    sptr<IRemoteObject> token1 = new RunningLockTokenStub();
    sptr<IRemoteObject> token2 = new RunningLockTokenStub();
    RunningLockInfo runningLockInfo1 {"runninglocktest1", RunningLockType::RUNNINGLOCK_SCREEN};
    RunningLockInfo runningLockInfo2 {"runninglocktest2", RunningLockType::RUNNINGLOCK_SCREEN};
    UserIPCInfo userIPCinfo1 {1, 1};
    UserIPCInfo userIPCinfo2 {2, 2};
    runningLockMgr_->Lock(token1, runningLockInfo1, userIPCinfo1);
    runningLockMgr_->Lock(token2, runningLockInfo2, userIPCinfo2);
    TestRunningLockInnerExisit(token1, runningLockInfo1);
    TestRunningLockInnerExisit(token2, runningLockInfo2);
    ASSERT_TRUE(2 == runningLockMgr_->GetRunningLockNum(RunningLockType::RUNNINGLOCK_SCREEN));
    ASSERT_TRUE(2 == runningLockMgr_->GetValidRunningLockNum(RunningLockType::RUNNINGLOCK_SCREEN));
    auto& proxymap = runningLockMgr_->GetRunningLockProxyMap();
    ASSERT_TRUE(proxymap.empty());
    {
        // proxy by userIPCinfo1, lockinner1 disabled
        runningLockMgr_->ProxyRunningLock(true, userIPCinfo1.uid, userIPCinfo1.pid);
        auto lockInner1 = runningLockMgr_->GetRunningLockInner(token1);
        ASSERT_TRUE(!lockInner1->GetReallyLocked());
        ASSERT_TRUE(lockInner1->GetDisabled());
        ASSERT_TRUE(!proxymap.empty() && (proxymap.count(userIPCinfo1.uid) > 0));

        runningLockMgr_->ProxyRunningLock(true, userIPCinfo2.uid, userIPCinfo2.pid);
        auto lockInner2 = runningLockMgr_->GetRunningLockInner(token2);
        ASSERT_TRUE(!lockInner2->GetReallyLocked());
        ASSERT_TRUE(lockInner2->GetDisabled());
        ASSERT_TRUE(!proxymap.empty() && (proxymap.count(userIPCinfo2.uid) > 0));
        ASSERT_TRUE(2 == runningLockMgr_->GetRunningLockNum(RunningLockType::RUNNINGLOCK_SCREEN));
        ASSERT_TRUE(0 == runningLockMgr_->GetValidRunningLockNum(RunningLockType::RUNNINGLOCK_SCREEN));

        runningLockMgr_->ProxyRunningLock(false, userIPCinfo1.uid, userIPCinfo1.pid);
        ASSERT_TRUE(lockInner1->GetReallyLocked());
        ASSERT_TRUE(!lockInner1->GetDisabled());
        ASSERT_TRUE(!proxymap.empty() && (proxymap.count(userIPCinfo1.uid) == 0));

        runningLockMgr_->ProxyRunningLock(false, userIPCinfo2.uid, userIPCinfo2.pid);
        ASSERT_TRUE(lockInner2->GetReallyLocked());
        ASSERT_TRUE(!lockInner2->GetDisabled());
        ASSERT_TRUE(proxymap.empty());
        ASSERT_TRUE(2 == runningLockMgr_->GetRunningLockNum(RunningLockType::RUNNINGLOCK_SCREEN));
        ASSERT_TRUE(2 == runningLockMgr_->GetValidRunningLockNum(RunningLockType::RUNNINGLOCK_SCREEN));
    }
    runningLockMgr_->UnLock(token1);
    runningLockMgr_->UnLock(token2);
    POWER_HILOGD(LABEL_TEST, "RunningLockTest::RunningLockMgr003 end");
}

/**
 * @tc.name: RunningLockMgr004
 * @tc.desc: Test RunningLockMgr Proxy function.
 * @tc.type: FUNC
 */
HWTEST_F (RunningLockTest, RunningLockMgr004, TestSize.Level0)
{
    sptr<IRemoteObject> token3 = new RunningLockTokenStub();
    sptr<IRemoteObject> token4 = new RunningLockTokenStub();
    RunningLockInfo runningLockInfo3 {"runninglocktest3", RunningLockType::RUNNINGLOCK_SCREEN};
    RunningLockInfo runningLockInfo4 {"runninglocktest4", RunningLockType::RUNNINGLOCK_SCREEN};
    UserIPCInfo userIPCinfo3 {3, 3};
    UserIPCInfo userIPCinfo4 {3, 4};
    runningLockMgr_->Lock(token3, runningLockInfo3, userIPCinfo3);
    runningLockMgr_->Lock(token4, runningLockInfo4, userIPCinfo4);
    TestRunningLockInnerExisit(token3, runningLockInfo3);
    TestRunningLockInnerExisit(token4, runningLockInfo4);
    ASSERT_TRUE(2 == runningLockMgr_->GetValidRunningLockNum(RunningLockType::RUNNINGLOCK_SCREEN));
    auto& proxymap = runningLockMgr_->GetRunningLockProxyMap();
    ASSERT_TRUE(proxymap.empty());
    {
        // lockinner3 and lockinner4 have same pid, save uid key, pid set {uid3,{pid3, pid4}}
        runningLockMgr_->ProxyRunningLock(true, userIPCinfo3.uid, userIPCinfo3.pid);
        runningLockMgr_->ProxyRunningLock(true, userIPCinfo4.uid, userIPCinfo4.pid);
        auto lockInner3 = runningLockMgr_->GetRunningLockInner(token3);
        ASSERT_TRUE(!lockInner3->GetReallyLocked());
        ASSERT_TRUE(lockInner3->GetDisabled());
        ASSERT_TRUE(!proxymap.empty() && (proxymap.count(userIPCinfo3.uid) > 0));
        {
            auto it = proxymap.find(userIPCinfo3.uid);
            ASSERT_TRUE(it != proxymap.end());
            auto& pidset = it->second;
            ASSERT_TRUE(pidset.count(userIPCinfo3.pid) == 1);
        }
        auto lockInner4 = runningLockMgr_->GetRunningLockInner(token4);
        ASSERT_TRUE(lockInner4 != nullptr);
        ASSERT_TRUE(!lockInner4->GetReallyLocked());
        ASSERT_TRUE(lockInner4->GetDisabled());
        ASSERT_TRUE(!proxymap.empty() && (proxymap.count(userIPCinfo4.uid) > 0));
        {
            auto it = proxymap.find(userIPCinfo4.uid);
            ASSERT_TRUE(it != proxymap.end());
            auto& pidset = it->second;
            ASSERT_TRUE(pidset.count(userIPCinfo4.pid) == 1);
        }
        ASSERT_TRUE(0 == runningLockMgr_->GetValidRunningLockNum(RunningLockType::RUNNINGLOCK_SCREEN));
        runningLockMgr_->ProxyRunningLock(false, userIPCinfo3.uid, INVALID_PID);
        ASSERT_TRUE(proxymap.empty());
        ASSERT_TRUE(lockInner3->GetReallyLocked() && !lockInner3->GetDisabled());
        ASSERT_TRUE(lockInner4->GetReallyLocked() && !lockInner4->GetDisabled());
        ASSERT_TRUE(2 == runningLockMgr_->GetValidRunningLockNum(RunningLockType::RUNNINGLOCK_SCREEN));
    }
    runningLockMgr_->UnLock(token3);
    runningLockMgr_->UnLock(token4);
    POWER_HILOGD(LABEL_TEST, "RunningLockTest::RunningLockMgr004 end");
}
#endif // IPC_AVAILABLE
}
