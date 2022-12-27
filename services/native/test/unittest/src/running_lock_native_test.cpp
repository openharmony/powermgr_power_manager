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

#include "running_lock_native_test.h"

#include <ipc_skeleton.h>

#include "actions/irunning_lock_action.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

void RunningLockNativeTest::SetUpTestCase()
{
}

void PowerShutdownTest1Callback::ShutdownCallback()
{
    POWER_HILOGI(LABEL_TEST, "PowerShutdownTest1Callback::ShutdownCallback.");
}

namespace {
/**
 * @tc.name: RunningLockNative001
 * @tc.desc: test init in runningLockMgr
 * @tc.type: FUNC
 */
HWTEST_F (RunningLockNativeTest, RunningLockNative001, TestSize.Level0)
{
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    pmsTest->OnStart();
    auto stateMachine = std::make_shared<PowerStateMachine>(pmsTest);
    EXPECT_TRUE(stateMachine->Init());
    UserActivityType userActivityType = UserActivityType::USER_ACTIVITY_TYPE_ACCESSIBILITY;
    stateMachine->RefreshActivityInner(PID, CALLTIMEMS, userActivityType, true);
    int32_t powermsEvent = PowermsEventHandler::CHECK_USER_ACTIVITY_OFF_TIMEOUT_MSG;
    stateMachine->HandleDelayTimer(powermsEvent);

    auto runningLockMgr = std::make_shared<RunningLockMgr>(pmsTest);
    EXPECT_TRUE(runningLockMgr->Init());
    sptr<IRemoteObject> remoteObject = new RunningLockTokenStub();
    EXPECT_TRUE(runningLockMgr->GetRunningLockInner(remoteObject) == nullptr);
    sptr<IRemoteObject> remoteObj = new RunningLockTokenStub();
    RunningLockInfo runningLockInfo {"runninglockNativeTest1", RunningLockType::RUNNINGLOCK_SCREEN};
    UserIPCInfo userIPCinfo {IPCSkeleton::GetCallingUid(), IPCSkeleton::GetCallingPid()};
    EXPECT_TRUE(runningLockMgr->CreateRunningLock(remoteObj, runningLockInfo, userIPCinfo) != nullptr);
    EXPECT_FALSE(runningLockMgr->IsUsed(remoteObj));
    runningLockMgr->Lock(remoteObj, runningLockInfo, userIPCinfo, TIMEOUTMS);
    EXPECT_TRUE(runningLockMgr->GetRunningLockNum(RunningLockType::RUNNINGLOCK_SCREEN) == LOCKNUM_B);
    EXPECT_TRUE(runningLockMgr->GetRunningLockNum(RunningLockType::RUNNINGLOCK_BUTT) == LOCKNUM_B);
    EXPECT_TRUE(runningLockMgr->GetValidRunningLockNum(RunningLockType::RUNNINGLOCK_SCREEN) == LOCKNUM_B);
    EXPECT_TRUE(runningLockMgr->GetValidRunningLockNum(static_cast<RunningLockType>(MAXTYPE)) == LOCKNUM_A);
    EXPECT_TRUE(runningLockMgr->IsUsed(remoteObj));
    sptr<IRemoteObject> token = new RunningLockTokenStub();
    EXPECT_FALSE(runningLockMgr->IsUsed(token));
    runningLockMgr->Lock(token, runningLockInfo, userIPCinfo, TIMEOUTMS);
    runningLockMgr->UnLock(remoteObj);
    runningLockMgr->UnLock(token);

    EXPECT_FALSE(runningLockMgr->ReleaseLock(remoteObj));
    EXPECT_FALSE(runningLockMgr->ReleaseLock(token));

    POWER_HILOGI(LABEL_TEST, "RunningLockNative001 end.");
}

/**
 * @tc.name: RunningLockNative002
 * @tc.desc: test proxyRunningLock in runningLockMgr
 * @tc.type: FUNC
 */
HWTEST_F (RunningLockNativeTest, RunningLockNative002, TestSize.Level0)
{
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    pmsTest->OnStart();
    auto stateMachine = std::make_shared<PowerStateMachine>(pmsTest);
    EXPECT_TRUE(stateMachine->Init());
    UserActivityType userActivityType = UserActivityType::USER_ACTIVITY_TYPE_ACCESSIBILITY;
    stateMachine->RefreshActivityInner(UID, PID, userActivityType, true);
    int32_t powermsEvent = PowermsEventHandler::CHECK_USER_ACTIVITY_OFF_TIMEOUT_MSG;
    stateMachine->HandleDelayTimer(powermsEvent);

    auto runningLockMgr = std::make_shared<RunningLockMgr>(pmsTest);
    EXPECT_TRUE(runningLockMgr->Init());
    EXPECT_FALSE(runningLockMgr->ExistValidRunningLock());
    runningLockMgr->CheckOverTime();
    std::shared_ptr<WorkTrigger> worker = std::make_shared<WorkTrigger>();
    RunningLockInfo runningLockInfo {"runninglockNativeTest1", RunningLockType::RUNNINGLOCK_SCREEN};
    auto& worklist = runningLockInfo.workTriggerlist;
    sptr<IRemoteObject> token = new RunningLockTokenStub();
    runningLockMgr->SetWorkTriggerList(token, worklist);
    worklist.push_back(worker);
    sptr<IRemoteObject> remoteObj = new RunningLockTokenStub();
    UserIPCInfo userIPCinfo {IPCSkeleton::GetCallingUid(), IPCSkeleton::GetCallingPid()};
    EXPECT_TRUE(runningLockMgr->CreateRunningLock(remoteObj, runningLockInfo, userIPCinfo) != nullptr);
    EXPECT_FALSE(runningLockMgr->ExistValidRunningLock());
    runningLockMgr->Lock(remoteObj, runningLockInfo, userIPCinfo, TIMEOUTMS);
    EXPECT_TRUE(runningLockMgr->ExistValidRunningLock() == true);
    runningLockMgr->SetWorkTriggerList(remoteObj, worklist);
    runningLockMgr->CheckOverTime();
    runningLockMgr->CheckOverTime();

    UserIPCInfo userIPCinfo1 {UID, UNPID};
    runningLockMgr->ProxyRunningLock(false, userIPCinfo.uid, userIPCinfo.pid);
    runningLockMgr->ProxyRunningLock(true, userIPCinfo.uid, userIPCinfo.pid);
    runningLockMgr->ProxyRunningLock(true, userIPCinfo.uid, userIPCinfo.pid);
    runningLockMgr->ProxyRunningLock(true, userIPCinfo1.uid, userIPCinfo1.pid);
    runningLockMgr->ProxyRunningLock(true, userIPCinfo1.uid, userIPCinfo1.pid);
    runningLockMgr->ProxyRunningLock(false, userIPCinfo.uid, userIPCinfo.pid);
    runningLockMgr->ProxyRunningLock(false, userIPCinfo1.uid, userIPCinfo1.pid);
    runningLockMgr->UnLock(remoteObj);
    EXPECT_FALSE(runningLockMgr->ReleaseLock(remoteObj));
    POWER_HILOGI(LABEL_TEST, "RunningLockNative002 end.");
}

/**
 * @tc.name: RunningLockNative003
 * @tc.desc: test enableMock and dumpInfo in runningLockMgr
 * @tc.type: FUNC
 */
HWTEST_F (RunningLockNativeTest, RunningLockNative003, TestSize.Level0)
{
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    pmsTest->OnStart();
    auto stateMachine = std::make_shared<PowerStateMachine>(pmsTest);
    EXPECT_TRUE(stateMachine->Init());
    UserActivityType userActivityType = UserActivityType::USER_ACTIVITY_TYPE_ACCESSIBILITY;
    stateMachine->RefreshActivityInner(UID, PID, userActivityType, true);
    int32_t powermsEvent = PowermsEventHandler::CHECK_USER_ACTIVITY_OFF_TIMEOUT_MSG;
    stateMachine->HandleDelayTimer(powermsEvent);

    auto runningLockMgr = std::make_shared<RunningLockMgr>(pmsTest);
    EXPECT_TRUE(runningLockMgr->Init());
    IRunningLockAction *runLockAction = new RunningLockAction();
    runningLockMgr->EnableMock(runLockAction);
    std::string result;
    runningLockMgr->DumpInfo(result);
    RunningLockInfo runningLockInfo1 {"runninglockNativeTest1", RunningLockType::RUNNINGLOCK_BACKGROUND};
    UserIPCInfo userIPCinfo1 {IPCSkeleton::GetCallingUid(), IPCSkeleton::GetCallingPid()};
    sptr<IRemoteObject> remoteObj = new RunningLockTokenStub();
    EXPECT_TRUE(runningLockMgr->CreateRunningLock(remoteObj, runningLockInfo1, userIPCinfo1) != nullptr);
    runningLockMgr->DumpInfo(result);
    runningLockMgr->Lock(remoteObj, runningLockInfo1, userIPCinfo1, TIMEOUTMS);
    runningLockMgr->UnLock(remoteObj);

    RunningLockInfo runningLockInfo2 {"runninglockNativeTest2",
                                      RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL};
    UserIPCInfo userIPCinfo2 {UID, PID};
    sptr<IRemoteObject> token = new RunningLockTokenStub();
    EXPECT_TRUE(runningLockMgr->CreateRunningLock(token, runningLockInfo2, userIPCinfo2) != nullptr);
    runningLockMgr->DumpInfo(result);
    runningLockMgr->Lock(token, runningLockInfo2, userIPCinfo2, TIMEOUTMS);
    runningLockMgr->UnLock(token);

    EXPECT_FALSE(runningLockMgr->ReleaseLock(remoteObj));
    EXPECT_FALSE(runningLockMgr->ReleaseLock(token));
    POWER_HILOGI(LABEL_TEST, "RunningLockNative003 end.");
}

/**
 * @tc.name: RunningLockNative004
 * @tc.desc: test setProximity in runningLockMgr
 * @tc.type: FUNC
 */
HWTEST_F (RunningLockNativeTest, RunningLockNative004, TestSize.Level0)
{
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    pmsTest->OnStart();
    auto stateMachine = std::make_shared<PowerStateMachine>(pmsTest);
    EXPECT_TRUE(stateMachine->Init());
    UserActivityType userActivityType = UserActivityType::USER_ACTIVITY_TYPE_ACCESSIBILITY;
    stateMachine->RefreshActivityInner(PID, CALLTIMEMS, userActivityType, true);
    int32_t powermsEvent = PowermsEventHandler::CHECK_USER_ACTIVITY_OFF_TIMEOUT_MSG;
    stateMachine->HandleDelayTimer(powermsEvent);

    auto runningLockMgr = std::make_shared<RunningLockMgr>(pmsTest);
    EXPECT_TRUE(runningLockMgr->Init());
    runningLockMgr->SetProximity(RunningLockMgr::PROXIMITY_AWAY);
    runningLockMgr->SetProximity(RunningLockMgr::PROXIMITY_CLOSE);
    runningLockMgr->SetProximity(MAXTYPE);

    RunningLockInfo runningLockInfo {"runninglockNativeTest", RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL};
    UserIPCInfo userIPCinfo {UID, PID};
    sptr<IRemoteObject> token = new RunningLockTokenStub();
    EXPECT_TRUE(runningLockMgr->CreateRunningLock(token, runningLockInfo, userIPCinfo) != nullptr);
    runningLockMgr->Lock(token, runningLockInfo, userIPCinfo, TIMEOUTMS);
    runningLockMgr->SetProximity(RunningLockMgr::PROXIMITY_AWAY);
    runningLockMgr->SetProximity(RunningLockMgr::PROXIMITY_CLOSE);
    runningLockMgr->SetProximity(RunningLockMgr::PROXIMITY_CLOSE);
    runningLockMgr->SetProximity(RunningLockMgr::PROXIMITY_AWAY);
    runningLockMgr->UnLock(token);

    RunningLockInfo runningLockInfo2 {"runninglockNativeTest2", static_cast<RunningLockType>(7U)};
    UserIPCInfo userIPCinfo2 {UID_A, PID_A};
    sptr<IRemoteObject> remoteObj = new RunningLockTokenStub();
    EXPECT_TRUE(runningLockMgr->CreateRunningLock(remoteObj, runningLockInfo2, userIPCinfo2) != nullptr);
    runningLockMgr->Lock(remoteObj, runningLockInfo2, userIPCinfo2, TIMEOUTMS);
    runningLockMgr->UnLock(remoteObj);
    POWER_HILOGI(LABEL_TEST, "RunningLockNative004 end.");
}

/**
 * @tc.name: RunningLockNative005
 * @tc.desc: test lock and unlock in runningLockMgr
 * @tc.type: FUNC
 */
HWTEST_F (RunningLockNativeTest, RunningLockNative005, TestSize.Level0)
{
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    pmsTest->OnStart();
    auto stateMachine = std::make_shared<PowerStateMachine>(pmsTest);
    EXPECT_TRUE(stateMachine->Init());
    UserActivityType userActivityType = UserActivityType::USER_ACTIVITY_TYPE_ACCESSIBILITY;
    stateMachine->RefreshActivityInner(UID, PID, userActivityType, true);
    int32_t powermsEvent = PowermsEventHandler::CHECK_USER_ACTIVITY_OFF_TIMEOUT_MSG;
    stateMachine->HandleDelayTimer(powermsEvent);

    auto runningLockMgr = std::make_shared<RunningLockMgr>(pmsTest);
    EXPECT_TRUE(runningLockMgr->Init());
    RunningLockInfo runningLockInfo {"runninglockNativeTest1", RunningLockType::RUNNINGLOCK_SCREEN};
    auto& worklist = runningLockInfo.workTriggerlist;
    sptr<IRemoteObject> remoteObj = new RunningLockTokenStub();
    UserIPCInfo userIPCinfo {UNUID, UNPID};
    EXPECT_TRUE(runningLockMgr->CreateRunningLock(remoteObj, runningLockInfo, userIPCinfo) != nullptr);
    runningLockMgr->SetWorkTriggerList(remoteObj, worklist);
    runningLockMgr->Lock(remoteObj, runningLockInfo, userIPCinfo, TIMEOUTMS);
    runningLockMgr->Lock(remoteObj, runningLockInfo, userIPCinfo, TIMEOUTMS);
    RunningLockInfo runningLockInfo1 {"runninglockNativeTest2", static_cast<RunningLockType>(MAXTYPE)};
    UserIPCInfo userIPCinfo1 {UNUID, UNPID};
    sptr<IRemoteObject> token = new RunningLockTokenStub();
    EXPECT_TRUE(runningLockMgr->CreateRunningLock(token, runningLockInfo1, userIPCinfo1) != nullptr);
    runningLockMgr->Lock(token, runningLockInfo1, userIPCinfo1, TIMEOUTMS);
    runningLockMgr->UnLock(token);

    EXPECT_FALSE(runningLockMgr->ReleaseLock(remoteObj));
    EXPECT_FALSE(runningLockMgr->ReleaseLock(token));
    POWER_HILOGI(LABEL_TEST, "RunningLockNative005 end.");
}

/**
 * @tc.name: RunningLockNative006
 * @tc.desc: test callback in runningLockMgr
 * @tc.type: FUNC
 */
HWTEST_F (RunningLockNativeTest, RunningLockNative006, TestSize.Level0)
{
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    auto runningLockMgr = std::make_shared<RunningLockMgr>(pmsTest);
    EXPECT_TRUE(runningLockMgr->Init());
    RunningLockInfo runningLockInfo {"runninglockNativeTest1", RunningLockType::RUNNINGLOCK_SCREEN};
    sptr<IRemoteObject> remoteObj = new RunningLockTokenStub();
    UserIPCInfo userIPCinfo {UNUID, UNPID};
    EXPECT_TRUE(runningLockMgr->CreateRunningLock(remoteObj, runningLockInfo, userIPCinfo) != nullptr);
    runningLockMgr->Lock(remoteObj, runningLockInfo, userIPCinfo, TIMEOUTMS);

    RunningLockMgr::RunningLockChangedType type = RunningLockMgr::RunningLockChangedType::NOTIFY_RUNNINGLOCK_OVERTIME;
    auto lockInner = RunningLockInner::CreateRunningLockInner(runningLockInfo, userIPCinfo);
    runningLockMgr->NotifyRunningLockChanged(remoteObj, lockInner, type);
    type = RunningLockMgr::RunningLockChangedType::RUNNINGLOCK_CHANGED_BUTT;
    runningLockMgr->NotifyRunningLockChanged(remoteObj, lockInner, type);
    type = static_cast<RunningLockMgr::RunningLockChangedType>(UNTYPE);
    runningLockMgr->NotifyRunningLockChanged(remoteObj, lockInner, type);

    auto runningLockMgrController = std::make_shared<RunningLockMgr::ProximityController>();
    SensorEvent sensorEvent;
    ProximityData data;
    data.distance = RunningLockMgr::ProximityController::PROXIMITY_CLOSE_SCALAR;
    sensorEvent.sensorTypeId = SENSOR_TYPE_ID_PROXIMITY;
    sensorEvent.data = reinterpret_cast<uint8_t*>(&data);
    runningLockMgrController->RecordSensorCallback(&sensorEvent);
    data.distance = RunningLockMgr::ProximityController::PROXIMITY_AWAY_SCALAR;
    runningLockMgrController->RecordSensorCallback(&sensorEvent);
    data.distance = RunningLockMgr::ProximityController::SAMPLING_RATE;
    runningLockMgrController->RecordSensorCallback(&sensorEvent);
    sensorEvent.sensorTypeId = TIMEOUTMS;
    runningLockMgrController->RecordSensorCallback(&sensorEvent);
    runningLockMgrController->RecordSensorCallback(nullptr);
    runningLockMgrController->support_ = true;
    runningLockMgrController->Enable();
    runningLockMgrController->Disable();

    auto runningLockMgrRecipient = std::make_shared<RunningLockMgr::RunningLockDeathRecipient>();
    wptr<IRemoteObject> remoteObject = new PowerShutdownTest1Callback();
    runningLockMgrRecipient->OnRemoteDied(remoteObject);
    runningLockMgrRecipient->OnRemoteDied(nullptr);

    EXPECT_FALSE(runningLockMgr->ReleaseLock(remoteObj));
    POWER_HILOGI(LABEL_TEST, "RunningLockNative006 end.");
}
}
