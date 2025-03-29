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

#include "running_lock_native_test.h"

#include <ipc_skeleton.h>

#include "actions/irunning_lock_action.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

namespace {
constexpr int64_t CALLTIMEMS = 1;
constexpr pid_t PID = 1;
constexpr pid_t PID_A = 2;
constexpr pid_t UNPID = -1;
constexpr pid_t UID = 1;
constexpr pid_t UID_A = 2;
constexpr pid_t UNUID = -1;
constexpr uint32_t LOCKNUM_A = 0;
constexpr uint32_t LOCKNUM_B = 1;
constexpr int32_t TIMEOUTMS = 7;
constexpr uint32_t MAXTYPE = 77;
constexpr int32_t UNTYPE = -1;
} //namespace

void PowerRunningLockTestCallback::HandleRunningLockMessage(std::string message)
{
    POWER_HILOGI(LABEL_TEST,
        "PowerRunningLockTestCallback::HandleRunningLockMessage, %{public}s", message.c_str());
}
namespace {
/**
 * @tc.name: RunningLockNative001
 * @tc.desc: test init in runningLockMgr
 * @tc.type: FUNC
 */
HWTEST_F (RunningLockNativeTest, RunningLockNative001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockNative001 function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    pmsTest->OnStart();
    auto stateMachine = pmsTest->GetPowerStateMachine();
    UserActivityType userActivityType = UserActivityType::USER_ACTIVITY_TYPE_ACCESSIBILITY;
    stateMachine->RefreshActivityInner(PID, CALLTIMEMS, userActivityType, true);

    auto runningLockMgr = std::make_shared<RunningLockMgr>(pmsTest);
    EXPECT_TRUE(runningLockMgr->Init());
    sptr<IRemoteObject> remoteObject = new RunningLockTokenStub();
    EXPECT_TRUE(runningLockMgr->GetRunningLockInner(remoteObject) == nullptr);
    sptr<IRemoteObject> remoteObj = new RunningLockTokenStub();
    int32_t pid = IPCSkeleton::GetCallingPid();
    int32_t uid = IPCSkeleton::GetCallingUid();
    RunningLockParam runningLockParam {0,
        "runninglockNativeTest1", "", RunningLockType::RUNNINGLOCK_SCREEN, TIMEOUTMS, pid, uid};
    EXPECT_TRUE(runningLockMgr->CreateRunningLock(remoteObj, runningLockParam) != nullptr);
    EXPECT_FALSE(runningLockMgr->IsUsed(remoteObj));
    runningLockMgr->Lock(remoteObj);
    EXPECT_TRUE(runningLockMgr->GetRunningLockNum(RunningLockType::RUNNINGLOCK_SCREEN) == LOCKNUM_B);
    EXPECT_TRUE(runningLockMgr->GetRunningLockNum(RunningLockType::RUNNINGLOCK_BUTT) == LOCKNUM_B);
    EXPECT_TRUE(runningLockMgr->GetValidRunningLockNum(RunningLockType::RUNNINGLOCK_SCREEN) == LOCKNUM_B);
    EXPECT_TRUE(runningLockMgr->GetValidRunningLockNum(static_cast<RunningLockType>(MAXTYPE)) == LOCKNUM_A);
    EXPECT_TRUE(runningLockMgr->IsUsed(remoteObj));
    sptr<IRemoteObject> token = new RunningLockTokenStub();
    EXPECT_FALSE(runningLockMgr->IsUsed(token));
    runningLockMgr->Lock(token);
    runningLockMgr->UnLock(remoteObj);
    runningLockMgr->UnLock(token);

    EXPECT_FALSE(runningLockMgr->ReleaseLock(remoteObj));
    EXPECT_FALSE(runningLockMgr->ReleaseLock(token));

    POWER_HILOGI(LABEL_TEST, "RunningLockNative001 function end!");
}

/**
 * @tc.name: RunningLockNative002
 * @tc.desc: test proxyRunningLock in runningLockMgr
 * @tc.type: FUNC
 */
HWTEST_F (RunningLockNativeTest, RunningLockNative002, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockNative002 function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    pmsTest->OnStart();
    auto stateMachine = pmsTest->GetPowerStateMachine();
    UserActivityType userActivityType = UserActivityType::USER_ACTIVITY_TYPE_ACCESSIBILITY;
    stateMachine->RefreshActivityInner(UID, PID, userActivityType, true);

    auto runningLockMgr = std::make_shared<RunningLockMgr>(pmsTest);
    EXPECT_TRUE(runningLockMgr->Init());
    EXPECT_FALSE(runningLockMgr->ExistValidRunningLock());
    sptr<IRemoteObject> remoteObj = new RunningLockTokenStub();
    int32_t pid = IPCSkeleton::GetCallingPid();
    int32_t uid = IPCSkeleton::GetCallingUid();
    RunningLockParam runningLockParam {0,
        "runninglockNativeTest1", "", RunningLockType::RUNNINGLOCK_SCREEN, TIMEOUTMS, pid, uid};
    EXPECT_TRUE(runningLockMgr->CreateRunningLock(remoteObj, runningLockParam) != nullptr);
    EXPECT_FALSE(runningLockMgr->ExistValidRunningLock());
    runningLockMgr->Lock(remoteObj);
    EXPECT_TRUE(runningLockMgr->ExistValidRunningLock() == true);

    runningLockMgr->ProxyRunningLock(false, pid, uid);
    runningLockMgr->ProxyRunningLock(true, pid, uid);
    runningLockMgr->ProxyRunningLock(true, pid, uid);
    runningLockMgr->ProxyRunningLock(true, UNPID, UID);
    runningLockMgr->ProxyRunningLock(true, UNPID, UID);
    runningLockMgr->ProxyRunningLock(false, pid, uid);
    runningLockMgr->ProxyRunningLock(false, UNPID, UID);

    runningLockMgr->UnLock(remoteObj);
    EXPECT_FALSE(runningLockMgr->ReleaseLock(remoteObj));
    POWER_HILOGI(LABEL_TEST, "RunningLockNative002 function end!");
}

/**
 * @tc.name: RunningLockNative003
 * @tc.desc: test enableMock and dumpInfo in runningLockMgr
 * @tc.type: FUNC
 */
HWTEST_F (RunningLockNativeTest, RunningLockNative003, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockNative003 function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    pmsTest->OnStart();
    auto stateMachine = pmsTest->GetPowerStateMachine();
    UserActivityType userActivityType = UserActivityType::USER_ACTIVITY_TYPE_ACCESSIBILITY;
    stateMachine->RefreshActivityInner(UID, PID, userActivityType, true);

    auto runningLockMgr = std::make_shared<RunningLockMgr>(pmsTest);
    EXPECT_TRUE(runningLockMgr->Init());
    IRunningLockAction *runLockAction = new RunningLockAction();
    runningLockMgr->EnableMock(runLockAction);
    std::string result;
    runningLockMgr->DumpInfo(result);

#ifdef HAS_SENSORS_SENSOR_PART
    RunningLockParam runningLockParam {0,
        "runninglockNativeTest2", "", RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL, TIMEOUTMS, PID, UID};
    sptr<IRemoteObject> token = new RunningLockTokenStub();
    EXPECT_TRUE(runningLockMgr->CreateRunningLock(token, runningLockParam) != nullptr);
    runningLockMgr->DumpInfo(result);
    runningLockMgr->Lock(token);
    runningLockMgr->UnLock(token);

    EXPECT_FALSE(runningLockMgr->ReleaseLock(token));
#endif
    POWER_HILOGI(LABEL_TEST, "RunningLockNative003 function end!");
}

/**
 * @tc.name: RunningLockNative004
 * @tc.desc: test setProximity in runningLockMgr
 * @tc.type: FUNC
 */
HWTEST_F (RunningLockNativeTest, RunningLockNative004, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockNative004 function start!");
#ifdef HAS_SENSORS_SENSOR_PART
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    pmsTest->OnStart();
    auto stateMachine = pmsTest->GetPowerStateMachine();
    UserActivityType userActivityType = UserActivityType::USER_ACTIVITY_TYPE_ACCESSIBILITY;
    stateMachine->RefreshActivityInner(PID, CALLTIMEMS, userActivityType, true);

    auto runningLockMgr = std::make_shared<RunningLockMgr>(pmsTest);
    EXPECT_TRUE(runningLockMgr->Init());
    runningLockMgr->SetProximity(RunningLockMgr::PROXIMITY_AWAY);
    runningLockMgr->SetProximity(RunningLockMgr::PROXIMITY_CLOSE);
    runningLockMgr->SetProximity(MAXTYPE);

    RunningLockParam runningLockParam {0,
        "runninglockNativeTest", "", RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL, TIMEOUTMS, PID, UID};
    sptr<IRemoteObject> token = new RunningLockTokenStub();
    EXPECT_TRUE(runningLockMgr->CreateRunningLock(token, runningLockParam) != nullptr);
    runningLockMgr->Lock(token);
    runningLockMgr->SetProximity(RunningLockMgr::PROXIMITY_AWAY);
    runningLockMgr->SetProximity(RunningLockMgr::PROXIMITY_CLOSE);
    runningLockMgr->SetProximity(RunningLockMgr::PROXIMITY_CLOSE);
    runningLockMgr->SetProximity(RunningLockMgr::PROXIMITY_AWAY);
    runningLockMgr->UnLock(token);

    RunningLockParam runningLockParam2 {0,
        "runninglockNativeTest2", "", static_cast<RunningLockType>(7U), TIMEOUTMS, PID_A, UID_A};
    sptr<IRemoteObject> remoteObj = new RunningLockTokenStub();
    EXPECT_TRUE(runningLockMgr->CreateRunningLock(remoteObj, runningLockParam2) != nullptr);
    runningLockMgr->Lock(remoteObj);
    runningLockMgr->UnLock(remoteObj);
#endif
    POWER_HILOGI(LABEL_TEST, "RunningLockNative004 function end!");
}

/**
 * @tc.name: RunningLockNative005
 * @tc.desc: test lock and unlock in runningLockMgr
 * @tc.type: FUNC
 */
HWTEST_F (RunningLockNativeTest, RunningLockNative005, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockNative005 function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    pmsTest->OnStart();
    auto stateMachine = pmsTest->GetPowerStateMachine();
    UserActivityType userActivityType = UserActivityType::USER_ACTIVITY_TYPE_ACCESSIBILITY;
    stateMachine->RefreshActivityInner(UID, PID, userActivityType, true);

    auto runningLockMgr = std::make_shared<RunningLockMgr>(pmsTest);
    EXPECT_TRUE(runningLockMgr->Init());
    sptr<IRemoteObject> remoteObj = new RunningLockTokenStub();
    RunningLockParam runningLockParam {0,
        "runninglockNativeTest1", "", RunningLockType::RUNNINGLOCK_SCREEN, TIMEOUTMS, UNPID, UNUID};
    EXPECT_TRUE(runningLockMgr->CreateRunningLock(remoteObj, runningLockParam) != nullptr);
    runningLockMgr->Lock(remoteObj);
    runningLockMgr->Lock(remoteObj);
    RunningLockParam runningLockParam1 {0,
        "runninglockNativeTest2", "", static_cast<RunningLockType>(MAXTYPE), TIMEOUTMS, UNPID, UNUID};
    sptr<IRemoteObject> token = new RunningLockTokenStub();
    EXPECT_TRUE(runningLockMgr->CreateRunningLock(token, runningLockParam1) != nullptr);
    runningLockMgr->Lock(token);
    runningLockMgr->UnLock(token);

    EXPECT_FALSE(runningLockMgr->ReleaseLock(remoteObj));
    EXPECT_FALSE(runningLockMgr->ReleaseLock(token));
    POWER_HILOGI(LABEL_TEST, "RunningLockNative005 function end!");
}

/**
 * @tc.name: RunningLockNative006
 * @tc.desc: test callback in runningLockMgr
 * @tc.type: FUNC
 */
HWTEST_F (RunningLockNativeTest, RunningLockNative006, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockNative006 function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    auto runningLockMgr = std::make_shared<RunningLockMgr>(pmsTest);
    EXPECT_TRUE(runningLockMgr->Init());
    RunningLockParam runningLockParam {0,
        "runninglockNativeTest1", "", RunningLockType::RUNNINGLOCK_SCREEN, TIMEOUTMS, UNPID, UNUID};
    sptr<IRemoteObject> remoteObj = new RunningLockTokenStub();
    EXPECT_TRUE(runningLockMgr->CreateRunningLock(remoteObj, runningLockParam) != nullptr);
    runningLockMgr->Lock(remoteObj);

    runningLockMgr->NotifyRunningLockChanged(runningLockParam, "DUBAI_TAG_RUNNINGLOCK_REMOVE", "RE");
    runningLockMgr->NotifyRunningLockChanged(runningLockParam, "DUBAI_TAG_RUNNINGLOCK_ADD", "AD");
    runningLockMgr->NotifyRunningLockChanged(runningLockParam, "DUBAI_TAG_RUNNINGLOCK_REMOVE", "RE");

#ifdef HAS_SENSORS_SENSOR_PART
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
#endif

    EXPECT_FALSE(runningLockMgr->ReleaseLock(remoteObj));
    POWER_HILOGI(LABEL_TEST, "RunningLockNative006 function end!");
}

/**
 * @tc.name: RunningLockNative007
 * @tc.desc: test activate in lockCounters
 * @tc.type: FUNC
 * @tc.require: issueI7MNRN
 */
HWTEST_F(RunningLockNativeTest, RunningLockNative007, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockNative007 function start!");
    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    auto runningLockMgr = std::make_shared<RunningLockMgr>(pmsTest_);
    EXPECT_TRUE(runningLockMgr->Init());
    std::shared_ptr<RunningLockMgr::LockCounter> ptr1 =
        runningLockMgr->lockCounters_[RunningLockType::RUNNINGLOCK_SCREEN];
    RunningLockParam runningLockParam1;
    ptr1->activate_(true, runningLockParam1);
    ptr1->activate_(false, runningLockParam1);
    EXPECT_TRUE(runningLockMgr->lockCounters_.size() != 0);

    pmsTest_->powerStateMachine_->SetState(PowerState::INACTIVE, StateChangeReason::STATE_CHANGE_REASON_TIMEOUT);
    std::shared_ptr<RunningLockMgr::LockCounter> ptr2 =
        runningLockMgr->lockCounters_[RunningLockType::RUNNINGLOCK_BACKGROUND_TASK];
    RunningLockParam runningLockParam2 {0,
        "RunningLockNative007", "", RunningLockType::RUNNINGLOCK_BACKGROUND_TASK, -1, 0, 0};
    ptr2->activate_(true, runningLockParam2);
    ptr2->activate_(false, runningLockParam2);
    EXPECT_TRUE(runningLockMgr->lockCounters_.size() != 0);

    pmsTest_->powerStateMachine_->SetState(PowerState::FREEZE, StateChangeReason::STATE_CHANGE_REASON_TIMEOUT);
    RunningLockParam runningLockParam3;
    ptr2->activate_(true, runningLockParam3);
    ptr2->activate_(false, runningLockParam3);
    EXPECT_TRUE(runningLockMgr->lockCounters_.size() != 0);

    std::shared_ptr<RunningLockMgr::LockCounter> ptr3 =
        runningLockMgr->lockCounters_[RunningLockType::RUNNINGLOCK_COORDINATION];
    RunningLockParam runningLockParam4;
    ptr3->activate_(true, runningLockParam4);
    ptr3->activate_(false, runningLockParam4);
    EXPECT_TRUE(runningLockMgr->lockCounters_.size() != 0);

    auto stateMachine = pmsTest_->GetPowerStateMachine();
    pmsTest_->powerStateMachine_ = nullptr;
    RunningLockParam runningLockParam5;
    ptr3->activate_(true, runningLockParam5);
    pmsTest_->powerStateMachine_ = stateMachine;
    EXPECT_TRUE(pmsTest_->powerStateMachine_ != nullptr);

    POWER_HILOGI(LABEL_TEST, "RunningLockNative007 function end!");
}

/**
 * @tc.name: RunningLockNative008
 * @tc.desc: test Lock
 * @tc.type: FUNC
 * @tc.require: issueI7MNRN
 */
HWTEST_F(RunningLockNativeTest, RunningLockNative008, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockNative008 function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    auto runningLockMgr = std::make_shared<RunningLockMgr>(pmsTest);
    EXPECT_TRUE(runningLockMgr->Init());
    sptr<IRemoteObject> remoteObject = new RunningLockTokenStub();
    runningLockMgr->Lock(remoteObject);
    runningLockMgr->UnLock(remoteObject);
    EXPECT_TRUE(runningLockMgr != nullptr);
    POWER_HILOGI(LABEL_TEST, "RunningLockNative008 function end!");
}

/**
 * @tc.name: RunningLockNative009
 * @tc.desc: test Lock
 * @tc.type: FUNC
 * @tc.require: issueI7MNRN
 */
HWTEST_F(RunningLockNativeTest, RunningLockNative009, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockNative009 function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    auto runningLockMgr = std::make_shared<RunningLockMgr>(pmsTest);
    EXPECT_TRUE(runningLockMgr->Init());
    sptr<IRemoteObject> remoteObject = new RunningLockTokenStub();
    runningLockMgr->UnLock(remoteObject);
    EXPECT_TRUE(runningLockMgr != nullptr);
    runningLockMgr->lockCounters_.clear();
    runningLockMgr->UnLock(remoteObject);
    EXPECT_TRUE(runningLockMgr != nullptr);
    POWER_HILOGI(LABEL_TEST, "RunningLockNative009 function end!");
}

/**
 * @tc.name: RunningLockNative010
 * @tc.desc: test IsUsed
 * @tc.type: FUNC
 * @tc.require: issueI7MNRN
 */
HWTEST_F(RunningLockNativeTest, RunningLockNative010, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockNative010 function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    auto runningLockMgr = std::make_shared<RunningLockMgr>(pmsTest);
    EXPECT_TRUE(runningLockMgr->Init());
    sptr<IRemoteObject> remoteObject = new RunningLockTokenStub();
    runningLockMgr->IsUsed(remoteObject);
    EXPECT_TRUE(runningLockMgr != nullptr);
    POWER_HILOGI(LABEL_TEST, "RunningLockNative010 function end!");
}

/**
 * @tc.name: RunningLockNative011
 * @tc.desc: test GetRunningLockNum
 * @tc.type: FUNC
 * @tc.require: issueI7MNRN
 */
HWTEST_F(RunningLockNativeTest, RunningLockNative011, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockNative011 function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    auto runningLockMgr = std::make_shared<RunningLockMgr>(pmsTest);
    EXPECT_TRUE(runningLockMgr->Init());
    runningLockMgr->GetRunningLockNum(RunningLockType::RUNNINGLOCK_BUTT);
    EXPECT_TRUE(runningLockMgr != nullptr);
    POWER_HILOGI(LABEL_TEST, "RunningLockNative011 function end!");
}

/**
 * @tc.name: RunningLockNative012
 * @tc.desc: test GetValidRunningLockNum
 * @tc.type: FUNC
 * @tc.require: issueI7MNRN
 */
HWTEST_F(RunningLockNativeTest, RunningLockNative012, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockNative012 function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    auto runningLockMgr = std::make_shared<RunningLockMgr>(pmsTest);
    EXPECT_TRUE(runningLockMgr->Init());
    runningLockMgr->GetValidRunningLockNum(static_cast<RunningLockType>(-1));
    EXPECT_TRUE(runningLockMgr != nullptr);
    POWER_HILOGI(LABEL_TEST, "RunningLockNative012 function end!");
}

/**
 * @tc.name: RunningLockNative013
 * @tc.desc: test NotifyRunningLockChanged
 * @tc.type: FUNC
 * @tc.require: issueI7MNRN
 */
HWTEST_F(RunningLockNativeTest, RunningLockNative013, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockNative013 function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    auto runningLockMgr = std::make_shared<RunningLockMgr>(pmsTest);
    EXPECT_TRUE(runningLockMgr->Init());
    sptr<IRemoteObject> remoteObj = new RunningLockTokenStub();
    RunningLockParam runningLockParam {0,
        "runninglockNativeTest1", "", RunningLockType::RUNNINGLOCK_SCREEN, TIMEOUTMS, UNPID, UNUID};
    runningLockMgr->NotifyRunningLockChanged(runningLockParam, "DUBAI_TAG_RUNNINGLOCK_ADD", "AD");
    runningLockMgr->NotifyRunningLockChanged(runningLockParam, "DUBAI_TAG_RUNNINGLOCK_REMOVE", "RE");
    EXPECT_TRUE(runningLockMgr != nullptr);
    POWER_HILOGI(LABEL_TEST, "RunningLockNative013 function end!");
}

/**
 * @tc.name: RunningLockNative014
 * @tc.desc: test ProxyRunningLock
 * @tc.type: FUNC
 * @tc.require: issueI7MNRN
 */
HWTEST_F(RunningLockNativeTest, RunningLockNative014, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockNative014 function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    auto runningLockMgr = std::make_shared<RunningLockMgr>(pmsTest);
    EXPECT_TRUE(runningLockMgr->Init());

    pid_t pid = 1;
    pid_t uid = 0;
    EXPECT_TRUE(runningLockMgr->ProxyRunningLock(true, pid, uid) == true);
    EXPECT_TRUE(runningLockMgr->ProxyRunningLock(false, pid, uid) == true);
    EXPECT_TRUE(runningLockMgr->ProxyRunningLock(true, 0, uid) == false);
    POWER_HILOGI(LABEL_TEST, "RunningLockNative014 function end!");
}

/**
 * @tc.name: RunningLockNative015
 * @tc.desc: test ProxyRunningLockInner
 * @tc.type: FUNC
 * @tc.require: issueI7MNRN
 */
HWTEST_F(RunningLockNativeTest, RunningLockNative015, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockNative015 function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    auto runningLockMgr = std::make_shared<RunningLockMgr>(pmsTest);
    EXPECT_TRUE(runningLockMgr->Init());

    pid_t pid = 0;
    pid_t uid = 0;
    EXPECT_TRUE(runningLockMgr != nullptr);

    RunningLockParam runningLockParam {0,
        "runninglockNativeTest1", "", RunningLockType::RUNNINGLOCK_SCREEN, TIMEOUTMS, UNPID, UNUID};
    sptr<IRemoteObject> remoteObj = new RunningLockTokenStub();
    EXPECT_TRUE(runningLockMgr->CreateRunningLock(remoteObj, runningLockParam) != nullptr);
    EXPECT_TRUE(runningLockMgr != nullptr);
    EXPECT_TRUE(runningLockMgr != nullptr);
    runningLockMgr->runninglockProxy_->AddRunningLock(pid, uid, nullptr);
    EXPECT_TRUE(runningLockMgr != nullptr);
    POWER_HILOGI(LABEL_TEST, "RunningLockNative015 function end!");
}

/**
 * @tc.name: RunningLockNative016
 * @tc.desc: test DumpInfo
 * @tc.type: FUNC
 * @tc.require: issueI7MNRN
 */
HWTEST_F(RunningLockNativeTest, RunningLockNative016, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockNative016 function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    auto runningLockMgr = std::make_shared<RunningLockMgr>(pmsTest);
    EXPECT_TRUE(runningLockMgr->Init());
    std::string result;
    RunningLockParam runningLockParam {0,
        "runninglockNativeTest1", "", RunningLockType::RUNNINGLOCK_SCREEN, TIMEOUTMS, UNPID, UNUID};
    sptr<IRemoteObject> remoteObj = new RunningLockTokenStub();
    EXPECT_TRUE(runningLockMgr->CreateRunningLock(remoteObj, runningLockParam) != nullptr);
    runningLockMgr->DumpInfo(result);
    EXPECT_TRUE(runningLockMgr != nullptr);
    runningLockMgr->runningLocks_.clear();
    runningLockMgr->DumpInfo(result);
    EXPECT_TRUE(runningLockMgr != nullptr);

    POWER_HILOGI(LABEL_TEST, "RunningLockNative016 function end!");
}

/**
 * @tc.name: RunningLockNative017
 * @tc.desc: test Lock
 * @tc.type: FUNC
 * @tc.require: issueI7MNRN
 */
HWTEST_F(RunningLockNativeTest, RunningLockNative017, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockNative017 function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    auto runningLockMgr = std::make_shared<RunningLockMgr>(pmsTest);
    EXPECT_TRUE(runningLockMgr->Init());
    EXPECT_TRUE(runningLockMgr != nullptr);
    POWER_HILOGI(LABEL_TEST, "RunningLockNative017 function end!");
}

/**
 * @tc.name: RunningLockNative018
 * @tc.desc: test Unlock
 * @tc.type: FUNC
 * @tc.require: issueI7MNRN
 */
HWTEST_F(RunningLockNativeTest, RunningLockNative018, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockNative018 function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    auto runningLockMgr = std::make_shared<RunningLockMgr>(pmsTest);
    EXPECT_TRUE(runningLockMgr->Init());
    EXPECT_TRUE(runningLockMgr != nullptr);
    POWER_HILOGI(LABEL_TEST, "RunningLockNative018 function end!");
}

/**
 * @tc.name: RunningLockNative019
 * @tc.desc: test activate in lockCounters
 * @tc.type: FUNC
 * @tc.require: issueI7MNRN
 */
HWTEST_F(RunningLockNativeTest, RunningLockNative019, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockNative019 function start!");
    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    auto runningLockMgr = std::make_shared<RunningLockMgr>(pmsTest_);
    EXPECT_TRUE(runningLockMgr->Init());
#ifdef HAS_SENSORS_SENSOR_PART
    RunningLockParam runningLockParam {0,
        "runninglockNativeTest1", "", RunningLockType::RUNNINGLOCK_SCREEN, TIMEOUTMS, UNPID, UNUID};
    runningLockMgr->lockCounters_[RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL]->Increase(
        runningLockParam);
    EXPECT_TRUE(runningLockMgr != nullptr);
    runningLockMgr->lockCounters_[RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL]->Decrease(
        runningLockParam);
    EXPECT_TRUE(runningLockMgr != nullptr);
#endif
    POWER_HILOGI(LABEL_TEST, "RunningLockNative019 function end!");
}

/**
 * @tc.name: RunningLockNative020
 * @tc.desc: test callback in NotifyRunningLockChanged
 * @tc.type: FUNC
 * @tc.require: issueI9C4GG
 */
HWTEST_F(RunningLockNativeTest, RunningLockNative020, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockNative020 function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    auto runningLockMgr = std::make_shared<RunningLockMgr>(pmsTest);
    EXPECT_TRUE(runningLockMgr->Init());
    sptr<IPowerRunninglockCallback> callback1 = new PowerRunningLockTestCallback();
    runningLockMgr->RegisterRunningLockCallback(callback1);
    RunningLockParam runningLockParam1 {0,
        "runninglockNativeTest1", "", RunningLockType::RUNNINGLOCK_SCREEN, TIMEOUTMS, UNPID, UNUID};
    runningLockMgr->NotifyRunningLockChanged(runningLockParam1, "DUBAI_TAG_RUNNINGLOCK_ADD", "AD");
    runningLockMgr->NotifyRunningLockChanged(runningLockParam1, "DUBAI_TAG_RUNNINGLOCK_REMOVE", "RE");

    sptr<IPowerRunninglockCallback> callback2 =new PowerRunningLockTestCallback();
    runningLockMgr->RegisterRunningLockCallback(callback2);
#ifdef HAS_SENSORS_SENSOR_PART
    RunningLockParam runningLockParam2 {0, "runninglockNativeTest2", "",
        RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL, TIMEOUTMS, UNPID, UNUID};
    runningLockMgr->NotifyRunningLockChanged(runningLockParam2, "DUBAI_TAG_RUNNINGLOCK_ADD", "AD");
    runningLockMgr->NotifyRunningLockChanged(runningLockParam2, "DUBAI_TAG_RUNNINGLOCK_REMOVE", "RE");
#endif

    runningLockMgr->UnRegisterRunningLockCallback(callback2);
    RunningLockParam runningLockParam3 {0, "runninglockNativeTest3", "",
        RunningLockType::RUNNINGLOCK_BACKGROUND_TASK, TIMEOUTMS, UNPID, UNUID};
    runningLockMgr->NotifyRunningLockChanged(runningLockParam3, "DUBAI_TAG_RUNNINGLOCK_ADD", "AD");
    runningLockMgr->NotifyRunningLockChanged(runningLockParam3, "DUBAI_TAG_RUNNINGLOCK_REMOVE", "RE");
    EXPECT_TRUE(runningLockMgr != nullptr);
    POWER_HILOGI(LABEL_TEST, "RunningLockNative020 function end!");
}

/**
 * @tc.name: RunningLockNative021
 * @tc.desc: test the activation of screen-on-lock
 * @tc.type: FUNC
 */
HWTEST_F(RunningLockNativeTest, RunningLockNative021, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockNative021 function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    pmsTest->OnStart();
    auto runningLockMgr = pmsTest->GetRunningLockMgr();

    sptr<IRemoteObject> remoteObj = new RunningLockTokenStub();
    int32_t pid = IPCSkeleton::GetCallingPid();
    int32_t uid = IPCSkeleton::GetCallingUid();
    RunningLockParam runningLockParam {0,
        "runninglockNativeTest1", "", RunningLockType::RUNNINGLOCK_SCREEN, -1, pid, uid};
    EXPECT_TRUE(runningLockMgr->CreateRunningLock(remoteObj, runningLockParam) != nullptr);
    EXPECT_FALSE(runningLockMgr->IsUsed(remoteObj));

    pmsTest->OverrideScreenOffTime(5000);
    auto stateMachine = pmsTest->GetPowerStateMachine();

    stateMachine->SetState(PowerState::INACTIVE, StateChangeReason::STATE_CHANGE_REASON_APPLICATION);
    EXPECT_EQ(stateMachine->GetState(), PowerState::INACTIVE);
    runningLockMgr->Lock(remoteObj);
    // screen on lock should not turn screen on if it is already off
    EXPECT_EQ(stateMachine->GetState(), PowerState::INACTIVE);
    runningLockMgr->UnLock(remoteObj);


    stateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_APPLICATION);
    EXPECT_EQ(stateMachine->GetState(), PowerState::AWAKE);
    runningLockMgr->Lock(remoteObj);
    stateMachine->SetState(PowerState::DIM, StateChangeReason::STATE_CHANGE_REASON_APPLICATION);
    // after the activation of screen-on lock the screen should no longer be in DIM state
    EXPECT_EQ(stateMachine->GetState(), PowerState::AWAKE);
    runningLockMgr->UnLock(remoteObj);
    
    pmsTest->RestoreScreenOffTime();
    POWER_HILOGI(LABEL_TEST, "RunningLockNative021 function end!");
}

/**
 * @tc.name: RunningLockNative022
 * @tc.desc: test enableMock and dumpInfo in runningLockMgr
 * @tc.type: FUNC
 */
HWTEST_F (RunningLockNativeTest, RunningLockNative022, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockNative022 function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    pmsTest->OnStart();
    auto stateMachine = pmsTest->GetPowerStateMachine();
    UserActivityType userActivityType = UserActivityType::USER_ACTIVITY_TYPE_ACCESSIBILITY;
    stateMachine->RefreshActivityInner(UID, PID, userActivityType, true);

    auto runningLockMgr = std::make_shared<RunningLockMgr>(pmsTest);
    EXPECT_TRUE(runningLockMgr->Init());
    IRunningLockAction *runLockAction = new RunningLockAction();
    runningLockMgr->EnableMock(runLockAction);
    std::string result;
    runningLockMgr->DumpInfo(result);

    RunningLockParam runningLockParam {0,
        "runninglockNativeTest022", "", RunningLockType::RUNNINGLOCK_BACKGROUND_AUDIO, TIMEOUTMS, PID, UID};
    sptr<IRemoteObject> token = new RunningLockTokenStub();
    EXPECT_TRUE(runningLockMgr->CreateRunningLock(token, runningLockParam) != nullptr);
    runningLockMgr->Lock(token);
    runningLockMgr->UpdateWorkSource(token, {{0, ""}});
    runningLockMgr->DumpInfo(result);
    runningLockMgr->UnLock(token);

    EXPECT_FALSE(runningLockMgr->ReleaseLock(token));
    POWER_HILOGI(LABEL_TEST, "RunningLockNative022 function end!");
}

/**
 * @tc.name: RunningLockNative023
 * @tc.desc: test NeedNotify
 * @tc.type: FUNC
 */
HWTEST_F (RunningLockNativeTest, RunningLockNative023, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockNative023 function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    pmsTest->OnStart();
    auto stateMachine = pmsTest->GetPowerStateMachine();
    UserActivityType userActivityType = UserActivityType::USER_ACTIVITY_TYPE_ACCESSIBILITY;
    stateMachine->RefreshActivityInner(UID, PID, userActivityType, true);

    auto runningLockMgr = std::make_shared<RunningLockMgr>(pmsTest);
    EXPECT_TRUE(runningLockMgr->Init());
    IRunningLockAction *runLockAction = new RunningLockAction();

    RunningLockParam runningLockParam0 {0,
        "runninglockNativeTest023_1", "", RunningLockType::RUNNINGLOCK_SCREEN, TIMEOUTMS, PID, UID};
    sptr<IRemoteObject> token0 = new RunningLockTokenStub();
    EXPECT_TRUE(runningLockMgr->CreateRunningLock(token0, runningLockParam0) != nullptr);
    runningLockMgr->Lock(token0);
    EXPECT_TRUE(runningLockMgr->NeedNotify(RunningLockType::RUNNINGLOCK_SCREEN));
    runningLockMgr->UnLock(token0);
    EXPECT_FALSE(runningLockMgr->ReleaseLock(token0));

    RunningLockParam runningLockParam1 {0, "runninglockNativeTest023_2", "",
        RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL, TIMEOUTMS, PID, UID};
    sptr<IRemoteObject> token1 = new RunningLockTokenStub();
    EXPECT_TRUE(runningLockMgr->CreateRunningLock(token1, runningLockParam1) != nullptr);
    runningLockMgr->Lock(token1);
    EXPECT_TRUE(runningLockMgr->NeedNotify(RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL));
    runningLockMgr->UnLock(token1);
    EXPECT_FALSE(runningLockMgr->ReleaseLock(token1));
    
    POWER_HILOGI(LABEL_TEST, "RunningLockNative023 function end!");
}
} // namespace
