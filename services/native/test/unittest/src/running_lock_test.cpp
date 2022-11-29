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

#include "running_lock_test.h"

#include <ipc_skeleton.h>

#include "actions/irunning_lock_action.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

sptr<PowerMgrService> RunningLockTest::pmsTest_ = nullptr;
std::shared_ptr<RunningLockMgr> RunningLockTest::runningLockMgr_ = nullptr;

void RunningLockTest::SetUpTestCase(void)
{
}

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

    WorkTriggerList worklist;
    worklist.push_back(nullptr);
    worklist.push_back(nullptr);
    worklist.push_back(nullptr);
    runningLock1->SetWorkTriggerList(worklist);
    auto& list1 = runningLock1->GetWorkTriggerList();
    ASSERT_TRUE(list1.empty());
    runningLock1->Lock();
    runningLock1->SetWorkTriggerList(worklist);
    auto& list2 = runningLock1->GetWorkTriggerList();
    ASSERT_TRUE(list2.empty());
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnitTest::RunningLockInnerKit01 end.");
}

/**
 * @tc.name: RunningLockInnerKit001
 * @tc.desc: Test RunningLock proxy function.
 * @tc.type: FUNC
 */
HWTEST_F (RunningLockTest, RunningLockInnerKit001, TestSize.Level0)
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    auto runningLock = powerMgrClient.CreateRunningLock("runninglock", RunningLockType::RUNNINGLOCK_BACKGROUND);
    ASSERT_TRUE(runningLock != nullptr);
    std::shared_ptr<WorkTrigger> worker1 = std::make_shared<WorkTrigger>(1, "worker1");
    std::shared_ptr<WorkTrigger> worker2 = std::make_shared<WorkTrigger>(2, "worker2", 20);
    std::shared_ptr<WorkTrigger> worker3 = std::make_shared<WorkTrigger>(3, "worker3", 30);
    std::shared_ptr<WorkTrigger> worker4 = std::make_shared<WorkTrigger>();
    WorkTriggerList worklist;
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnitTest::RunningLockInnerKit02, 1 usecount = %ld", worker1.use_count());
    runningLock->Lock();

    worklist.push_back(worker1);
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnitTest::RunningLockInnerKit02, 2 usecount = %ld", worker1.use_count());
    worklist.push_back(worker2);
    worklist.push_back(worker3);
    runningLock->SetWorkTriggerList(worklist);
    {
        auto& list = runningLock->GetWorkTriggerList();
        for (auto& worker : list) {
            POWER_HILOGD(LABEL_TEST, "PowerMgrUnitTest::RunningLockInnerKit02, 3 usecount = %ld, name = %s, "
                "uid = %d, pid = %d, abilityid = %d", worker.use_count(), worker->GetName().c_str(),
                worker->GetUid(), worker->GetPid(), worker->GetAbilityId());
        }
    }
    worklist.remove(worker2);
    worklist.remove(worker3);
    runningLock->SetWorkTriggerList(worklist);
    runningLock->UnLock();
    {
        auto& list2 = runningLock->GetWorkTriggerList();
        for (auto& worker : list2) {
            POWER_HILOGD(LABEL_TEST, "PowerMgrUnitTest::RunningLockInnerKit02, 4 usecount = %ld, name = %s, "
                "uid = %d, pid = %d, abilityid = %d", worker.use_count(), worker->GetName().c_str(),
                worker->GetUid(), worker->GetPid(), worker->GetAbilityId());
        }
    }
    worklist.push_back(worker4);
    runningLock->SetWorkTriggerList(worklist);
    {
        auto& list2 = runningLock->GetWorkTriggerList();
        for (auto& worker : list2) {
            POWER_HILOGD(LABEL_TEST, "PowerMgrUnitTest::RunningLockInnerKit02, 5 usecount = %ld, name = %s,"
                "uid = %d, pid = %d, abilityid = %d", worker.use_count(), worker->GetName().c_str(),
                worker->GetUid(), worker->GetPid(), worker->GetAbilityId());
        }
    }
    runningLock->Lock();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnitTest::RunningLockInnerKit002 end.");
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
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnitTest::RunningLockInnerKit003 1.");
    // lock 50ms
    runningLock1->Lock(50);
    usleep(4000);
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnitTest::RunningLockInnerKit003 2.");
    ASSERT_TRUE(runningLock1->IsUsed()) << "runningLock1->IsUsed() != true";
    usleep(1000);
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnitTest::RunningLockInnerKit003 3.");
    ASSERT_TRUE(runningLock1->IsUsed()) << "runningLock1->IsUsed() != true";
    // wait 60ms
    usleep(60000);
    ASSERT_TRUE(!runningLock1->IsUsed()) << "runningLock1->IsUsed() != false";
    RunningLockInfo inInfo {"runninglockTest1", RunningLockType::RUNNINGLOCK_SCREEN};
    std::shared_ptr<WorkTrigger> worker1 = std::make_shared<WorkTrigger>(1, "worker1");
    std::shared_ptr<WorkTrigger> worker2 = std::make_shared<WorkTrigger>(2, "worker2", 20);
    auto& worklist = inInfo.workTriggerlist;
    worklist.push_back(worker1);
    worklist.push_back(worker2);
    Parcel data;
    inInfo.Marshalling(data);
    RunningLockInfo *outInfo = inInfo.Unmarshalling(data);
    ASSERT_TRUE(outInfo != nullptr) << "outInfo != nullptr";
    ASSERT_TRUE(outInfo->name == inInfo.name) << "outInfo->name == inInfo.name";
    ASSERT_TRUE(outInfo->type == inInfo.type) << "outInfo->name == inInfo.name";
    ASSERT_TRUE(outInfo->workTriggerlist.size() == inInfo.workTriggerlist.size()) <<
        "outInfo->workTriggerlist.size() == inInfo.workTriggerlist.size()";
    auto& list1 = inInfo.workTriggerlist;
    auto& list2 = outInfo->workTriggerlist;
    for (auto it1 = list1.begin(), it2 = list2.begin(); (it1 != list1.end()) && (it2 != list2.end());
        it1++, it2++) {
        ASSERT_TRUE((*it1)->GetUid() == (*it2)->GetUid()) << "it1->GetUid() == it2->GetUid()";
        ASSERT_TRUE((*it1)->GetPid() == (*it2)->GetPid()) << "it1->GetPid() == it2->GetPid()";
        ASSERT_TRUE((*it1)->GetAbilityId() == (*it2)->GetAbilityId()) << "it1->GetAbilityId() == it2->GetAbilityId()";
        ASSERT_TRUE((*it1)->GetName() == (*it2)->GetName()) << "it1->GetName() == it2->GetName()";
    }
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnitTest::RunningLockInnerKit003 end.");
}

/**
 * @tc.name: RunningLockInnerKit005
 * @tc.desc: Test RunningLock proxy function.
 * @tc.type: FUNC
 * @tc.require: issue I63PN9
 */
HWTEST_F (RunningLockTest, RunningLockInnerKit005, TestSize.Level0)
{
    std::shared_ptr<WorkTrigger> worker = std::make_shared<WorkTrigger>(1, "worker");
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnitTest::RunningLockInnerKit05, 1 usecount = %ld", worker.use_count());

    worker->SetPid(1);
    EXPECT_EQ(worker->GetUid(), 1);
    worker->SetAbilityId(1);
    EXPECT_EQ(worker->GetAbilityId(), 1);
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnitTest::RunningLockInnerKit005 end.");
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
        POWER_HILOGD(LABEL_TEST, "PowerMgrUnitTest::RunningLockInnerKit005 1.");
        usleep(50000);
        ASSERT_TRUE(runningLock1->IsUsed()) << "runningLock1->IsUsed() != true";
        usleep(50000);
        ASSERT_TRUE(!runningLock1->IsUsed()) << "runningLock1->IsUsed() != false";
        // no unlock
        POWER_HILOGD(LABEL_TEST, "PowerMgrUnitTest::RunningLockInnerKit005 2.");
        runningLock1->Lock(2);
        runningLock1->Lock(3);
        runningLock1->Lock();
        POWER_HILOGD(LABEL_TEST, "PowerMgrUnitTest::RunningLockInnerKit005 3.");
        usleep(8000);
        ASSERT_TRUE(runningLock1->IsUsed()) << "runningLock1->IsUsed() != true";
        // after 3ms unlock
        runningLock1->Lock(30);
        POWER_HILOGD(LABEL_TEST, "PowerMgrUnitTest::RunningLockInnerKit005 4.");
        usleep(50000);
        ASSERT_TRUE(!runningLock1->IsUsed()) << "runningLock1->IsUsed() != false";
        runningLock1->Lock(5);
        runningLock1->UnLock();
        ASSERT_TRUE(!runningLock1->IsUsed()) << "runningLock1->IsUsed() != false";
        POWER_HILOGD(LABEL_TEST, "PowerMgrUnitTest::RunningLockInnerKit005 5.");
    }
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
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnitTest::RunningLockInnerKit004 1.");
    usleep(50000);
    ASSERT_TRUE(runningLock1->IsUsed()) << "runningLock1->IsUsed() != true";
    usleep(50000);
    ASSERT_TRUE(!runningLock1->IsUsed()) << "runningLock1->IsUsed() != false";
    // no unlock
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnitTest::RunningLockInnerKit004 2.");
    runningLock1->Lock(2);
    runningLock1->Lock(3);
    runningLock1->Lock();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnitTest::RunningLockInnerKit004 3.");
    usleep(8000);
    ASSERT_TRUE(runningLock1->IsUsed()) << "runningLock1->IsUsed() != true";
    // after 3ms unlock
    runningLock1->Lock(30);
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnitTest::RunningLockInnerKit004 4.");
    usleep(50000);
    ASSERT_TRUE(!runningLock1->IsUsed()) << "runningLock1->IsUsed() != false";
    runningLock1->Lock(5);
    runningLock1->UnLock();
    ASSERT_TRUE(!runningLock1->IsUsed()) << "runningLock1->IsUsed() != false";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnitTest::RunningLockInnerKit004 5.");
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
    POWER_HILOGD(LABEL_TEST, "RunningLockTest::RunningLockMgr001 end.");
}

/**
 * @tc.name: RunningLockMgr002
 * @tc.desc: Test RunningLockMgr SetWorkerList function.
 * @tc.type: FUNC
 */
HWTEST_F (RunningLockTest, RunningLockMgr002, TestSize.Level0)
{
    sptr<IRemoteObject> token = new RunningLockTokenStub();
    ASSERT_TRUE(token != nullptr);
    RunningLockInfo runningLockInfo1 {"runninglockTest1", RunningLockType::RUNNINGLOCK_SCREEN};
    UserIPCInfo userIPCinfo {IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid()};
    {
        runningLockMgr_->Lock(token, runningLockInfo1, userIPCinfo);
        TestRunningLockInnerExisit(token, runningLockInfo1);
        ASSERT_TRUE(1 == runningLockMgr_->GetRunningLockNum(RunningLockType::RUNNINGLOCK_SCREEN));
        ASSERT_TRUE(0 == runningLockMgr_->GetRunningLockNum(RunningLockType::RUNNINGLOCK_BACKGROUND));
    }
    {
        std::shared_ptr<WorkTrigger> worker1 = std::make_shared<WorkTrigger>(1, "worker1");
        std::shared_ptr<WorkTrigger> worker2 = std::make_shared<WorkTrigger>(2, "worker2", 20);
        std::shared_ptr<WorkTrigger> worker3 = std::make_shared<WorkTrigger>(3, "worker3", 30);
        std::shared_ptr<WorkTrigger> worker4 = std::make_shared<WorkTrigger>();
        RunningLockInfo runningLockInfo1;
        auto& worklist = runningLockInfo1.workTriggerlist;
        worklist.push_back(worker1);
        worklist.push_back(worker2);
        worklist.push_back(worker3);
        runningLockMgr_->SetWorkTriggerList(token, worklist);
        worklist.remove(worker3);
        worklist.push_back(worker4);
        runningLockMgr_->SetWorkTriggerList(token, worklist);
    }
    {
        runningLockMgr_->UnLock(token);
        TestRunningLockInnerNoExisit(token);
        ASSERT_TRUE(0 == runningLockMgr_->GetRunningLockNum(RunningLockType::RUNNINGLOCK_SCREEN));
        ASSERT_TRUE(0 == runningLockMgr_->GetRunningLockNum(RunningLockType::RUNNINGLOCK_BACKGROUND));
    }
    POWER_HILOGD(LABEL_TEST, "RunningLockTest::RunningLockMgr002 end.");
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
    POWER_HILOGD(LABEL_TEST, "RunningLockTest::RunningLockMgr003 end.");
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
    POWER_HILOGD(LABEL_TEST, "RunningLockTest::RunningLockMgr004 end.");
}

/**
 * @tc.name: RunningLockMgr005
 * @tc.desc: Test RunningLockMgr Proxy function.
 * @tc.type: FUNC
 */
HWTEST_F (RunningLockTest, RunningLockMgr005, TestSize.Level0)
{
    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo runningLockInfo1 {"runninglocktest1", RunningLockType::RUNNINGLOCK_SCREEN};
    UserIPCInfo userIPCinfo1 {1, 1};
    runningLockMgr_->Lock(token, runningLockInfo1, userIPCinfo1);
    auto& proxymap = runningLockMgr_->GetRunningLockProxyMap();
    ASSERT_TRUE(proxymap.empty());
    {
        auto lockInner1 = runningLockMgr_->GetRunningLockInner(token);
        runningLockMgr_->ProxyRunningLock(true, userIPCinfo1.uid, userIPCinfo1.pid);
        ASSERT_TRUE(!lockInner1->GetReallyLocked() && lockInner1->GetDisabled());
        runningLockMgr_->ProxyRunningLock(false, userIPCinfo1.uid, userIPCinfo1.pid);
        ASSERT_TRUE(lockInner1->GetReallyLocked() && !lockInner1->GetDisabled());
        UserIPCInfo workeripc1 {10, 20};
        UserIPCInfo workeripc2 {20, 20};
        runningLockMgr_->ProxyRunningLock(true, workeripc1.uid, workeripc1.pid);
        runningLockMgr_->ProxyRunningLock(true, workeripc2.uid, workeripc2.pid);
        std::shared_ptr<WorkTrigger> worker1 = std::make_shared<WorkTrigger>(workeripc1.uid, "worker1", workeripc1.pid);
        std::shared_ptr<WorkTrigger> worker2 = std::make_shared<WorkTrigger>(workeripc2.uid, "worker2", workeripc2.pid);
        auto& worklist = runningLockInfo1.workTriggerlist;
        worklist.push_back(worker1);
        worklist.push_back(worker2);
        runningLockMgr_->SetWorkTriggerList(token, worklist);
        ASSERT_TRUE(!lockInner1->GetReallyLocked() && lockInner1->GetDisabled());
        worklist.remove(worker2);
        runningLockMgr_->SetWorkTriggerList(token, worklist);
        ASSERT_TRUE(!lockInner1->GetReallyLocked() && lockInner1->GetDisabled());
        runningLockMgr_->ProxyRunningLock(false, workeripc1.uid, workeripc1.pid);
        runningLockMgr_->ProxyRunningLock(false, workeripc2.uid, workeripc2.pid);
        ASSERT_TRUE(lockInner1->GetReallyLocked() && !lockInner1->GetDisabled());
        UserIPCInfo workeripc3 {10, 30};
        std::shared_ptr<WorkTrigger> worker3 = std::make_shared<WorkTrigger>(workeripc3.uid, "worker3", workeripc3.pid);
        runningLockMgr_->ProxyRunningLock(true, workeripc1.uid, workeripc1.pid);
        runningLockMgr_->ProxyRunningLock(true, workeripc3.uid, workeripc3.pid);
        runningLockMgr_->SetWorkTriggerList(token, worklist);
        ASSERT_TRUE(!lockInner1->GetReallyLocked() && lockInner1->GetDisabled());
        runningLockMgr_->ProxyRunningLock(false, workeripc3.uid, INVALID_PID);
        ASSERT_TRUE(lockInner1->GetReallyLocked() && !lockInner1->GetDisabled());
        ASSERT_TRUE(proxymap.empty());
        runningLockMgr_->ProxyRunningLock(true, workeripc1.uid, workeripc1.pid);
        runningLockMgr_->ProxyRunningLock(true, workeripc3.uid, workeripc3.pid);
        runningLockMgr_->ProxyRunningLock(true, workeripc1.uid, INVALID_PID);
        auto it = proxymap.find(workeripc1.uid);
        auto& pidset = it->second;
        ASSERT_TRUE((pidset.size() == 1) && (pidset.count(INVALID_PID) == 1));
        runningLockMgr_->ProxyRunningLock(false, workeripc1.uid, INVALID_PID);
        ASSERT_TRUE(proxymap.empty());
    }
    runningLockMgr_->UnLock(token);
    POWER_HILOGD(LABEL_TEST, "RunningLockTest::RunningLockMgr005 end.");
}
#endif // IPC_AVAILABLE
}
