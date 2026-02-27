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
#include "power_utils.h"

#include "actions/irunning_lock_action.h"
#include "running_lock_changed_callback_stub.h"
#include "death_recipient_manager.h"

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

bool PowerUtils::IsForegroundApplication(const std::set<std::string>& appNames)
{
    return true;
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
    runningLockMgr->SetProximity(IProximityController::PROXIMITY_AWAY);
    runningLockMgr->SetProximity(IProximityController::PROXIMITY_CLOSE);
    runningLockMgr->SetProximity(MAXTYPE);

    RunningLockParam runningLockParam {0,
        "runninglockNativeTest", "", RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL, TIMEOUTMS, PID, UID};
    sptr<IRemoteObject> token = new RunningLockTokenStub();
    EXPECT_TRUE(runningLockMgr->CreateRunningLock(token, runningLockParam) != nullptr);
    runningLockMgr->Lock(token);
    runningLockMgr->SetProximity(IProximityController::PROXIMITY_AWAY);
    runningLockMgr->SetProximity(IProximityController::PROXIMITY_CLOSE);
    runningLockMgr->SetProximity(IProximityController::PROXIMITY_CLOSE);
    runningLockMgr->SetProximity(IProximityController::PROXIMITY_AWAY);
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
    runningLockMgr->Lock(remoteObj);
    // screen on lock should not turn screen on if it is already off

    runningLockMgr->UnLock(remoteObj);

    stateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_APPLICATION);

    runningLockMgr->Lock(remoteObj);
    stateMachine->SetState(PowerState::DIM, StateChangeReason::STATE_CHANGE_REASON_APPLICATION);
    // after the activation of screen-on lock the screen should no longer be in DIM state

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

/**
 * @tc.name: RunningLockNative024
 * @tc.desc: test IsExistAudioStream GetRunningLockMgr is nullptr
 * @tc.type: FUNC
 */
HWTEST_F (RunningLockNativeTest, RunningLockNative024, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockNative024 function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();

    auto runningLockMgr = std::make_shared<RunningLockMgr>(pmsTest);
    EXPECT_TRUE(runningLockMgr->Init());
    bool result = runningLockMgr->runninglockProxy_->IsExistAudioStream(UID);
    EXPECT_FALSE(result);
    POWER_HILOGI(LABEL_TEST, "RunningLockNative024 function end!");
}

/**
 * @tc.name: RunningLockNative025
 * @tc.desc: test IsExistAudioStream
 * @tc.type: FUNC
 */
HWTEST_F (RunningLockNativeTest, RunningLockNative025, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockNative025 function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    pmsTest->OnStart();
    auto runningLockMgr = pmsTest->GetRunningLockMgr();
    EXPECT_TRUE(runningLockMgr->Init());
    sptr<IRemoteObject> token0 = new RunningLockTokenStub();
    sptr<IRemoteObject> token1 = new RunningLockTokenStub();
    sptr<IRemoteObject> token2 = new RunningLockTokenStub();
    RunningLockParam runningLockParam0 {
        0, "runningLockNativeTest025_1", "", RunningLockType::RUNNINGLOCK_BACKGROUND_AUDIO, TIMEOUTMS, PID, UID};
    RunningLockParam runningLockParam1 {
        1, "runningLockNativeTest025_1", "", RunningLockType::RUNNINGLOCK_BACKGROUND_SPORT, TIMEOUTMS, PID, UID};
    EXPECT_TRUE(runningLockMgr->CreateRunningLock(token0, runningLockParam0) != nullptr);
    EXPECT_TRUE(runningLockMgr->CreateRunningLock(token1, runningLockParam1) != nullptr);
    runningLockMgr->runninglockProxy_->AddRunningLock(PID, UID, token2);
    runningLockMgr->Lock(token0);
    EXPECT_TRUE(runningLockMgr->NeedNotify(RunningLockType::RUNNINGLOCK_BACKGROUND_AUDIO));
    bool result = runningLockMgr->runninglockProxy_->IsExistAudioStream(UID);
    EXPECT_FALSE(result);
    std::map<int32_t, std::string> wks{{UID, ""}};
    runningLockMgr->runninglockProxy_->UpdateWorkSource(PID, UID, token0, wks);
    result = runningLockMgr->runninglockProxy_->IsExistAudioStream(UID);
    EXPECT_TRUE(result);
    runningLockMgr->UnLock(token0);
    EXPECT_FALSE(runningLockMgr->ReleaseLock(token0));
    POWER_HILOGI(LABEL_TEST, "RunningLockNative025 function end!");
}

/**
 * @tc.name: RunningLockNative026
 * @tc.desc: test IsVoiceAppForeground
 * @tc.type: FUNC
 * @tc.require: issueI7MNRN
 */
HWTEST_F(RunningLockNativeTest, RunningLockNative026, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockNative026 function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    auto runningLockMgr = std::make_shared<RunningLockMgr>(pmsTest);
    std::string str = "app.bundlename1;app.bundlename2";
    auto appList = PowerUtils::Split(str, ';');

    EXPECT_TRUE(runningLockMgr->Init());
    sptr<IRemoteObject> remoteObject = new RunningLockTokenStub();
    bool ret = runningLockMgr->IsVoiceAppForeground();
    EXPECT_TRUE(ret);
    POWER_HILOGI(LABEL_TEST, "RunningLockNative026 function end!");
}

/**
 * @tc.name: RunningLockNative027
 * @tc.desc: test ForceSleepReleaseLock
 * @tc.type: FUNC
 * @tc.require: issues/1585
 */
HWTEST_F(RunningLockNativeTest, RunningLockNative027, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockNative027 function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    pmsTest->OnStart();
    auto runningLockMgr = pmsTest->GetRunningLockMgr();
    sptr<IRemoteObject> remoteObj = new RunningLockTokenStub();
    sptr<IRemoteObject> remoteObjToken = new RunningLockTokenStub();
    int32_t pid = IPCSkeleton::GetCallingPid();
    int32_t uid = IPCSkeleton::GetCallingUid();
    RunningLockParam runningLockParam0 {0,
        "RunningLockNative027_1", "", RunningLockType::RUNNINGLOCK_BACKGROUND_USER_IDLE, TIMEOUTMS, pid, uid};
    RunningLockParam runningLockParam1 {0,
        "RunningLockNative027_2", "", RunningLockType::RUNNINGLOCK_BACKGROUND_TASK, TIMEOUTMS, pid, uid};
    EXPECT_TRUE(runningLockMgr->CreateRunningLock(remoteObj, runningLockParam0) != nullptr);
    EXPECT_TRUE(runningLockMgr->CreateRunningLock(remoteObjToken, runningLockParam1) != nullptr);
    runningLockMgr->Lock(remoteObj);
    runningLockMgr->Lock(remoteObjToken);
#ifdef POWER_MANAGER_ENABLE_FORCE_SLEEP_BROADCAST
    if (runningLockMgr->subscriberPtr_ != nullptr) {
        runningLockMgr->subscriberPtr_ =
            std::make_shared<RunningLockCommonEventSubscriber>(subscribeInfo, runningLockMgr);
    }
    EventFwk::CommonEventData data {};
    runningLockMgr->subscriberPtr_->OnReceiveEvent(data);
    EXPECT_TRUE(runningLockMgr->IsUsed(remoteObj));
    EventFwk::Want want;
    want.SetAction(OHOS::EventFwk::CommonEventSupport::COMMON_EVENT_ENTER_FORCE_SLEEP);
    data.SetWant(want);
    runningLockMgr->subscriberPtr_->OnReceiveEvent(data);
    EXPECT_TRUE(runningLockMgr->ffrtTimer_->get_task_cnt() != 0);
    want.SetAction(OHOS::EventFwk::CommonEventSupport::COMMON_EVENT_EXIT_FORCE_SLEEP);
    data.SetWant(want);
    runningLockMgr->subscriberPtr_->OnReceiveEvent(data);
    EXPECT_TRUE(runningLockMgr->IsUsed(remoteObj));
#endif
    runningLockMgr->ForceSleepReleaseLock();
    EXPECT_FALSE(runningLockMgr->IsUsed(remoteObj));
    EXPECT_TRUE(runningLockMgr->IsUsed(remoteObjToken));
    runningLockMgr->UnLock(remoteObjToken);
    EXPECT_TRUE(runningLockMgr->ForceSleepReleaseLock());
    POWER_HILOGI(LABEL_TEST, "RunningLockNative027 function end!");
}

/**
 * @tc.name: RunningLockNative028
 * @tc.desc: test GetEnabledRunningLocksByType
 * @tc.type: FUNC
 * @tc.require: issues/1585
 */
HWTEST_F(RunningLockNativeTest, RunningLockNative028, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockNative028 function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    auto runningLockMgr = std::make_shared<RunningLockMgr>(pmsTest);
    EXPECT_TRUE(runningLockMgr->Init());
    sptr<IRemoteObject> remoteObj = new RunningLockTokenStub();
    int32_t pid = IPCSkeleton::GetCallingPid();
    int32_t uid = IPCSkeleton::GetCallingUid();
    RunningLockParam runningLockParam {0,
        "RunningLockNative028_1", "", RunningLockType::RUNNINGLOCK_BACKGROUND_USER_IDLE, TIMEOUTMS, pid, uid};
    EXPECT_TRUE(runningLockMgr->CreateRunningLock(remoteObj, runningLockParam) != nullptr);
    auto locks = runningLockMgr->GetEnabledRunningLocksByType(RunningLockType::RUNNINGLOCK_BACKGROUND_USER_IDLE);
    EXPECT_TRUE(locks.size() == 0);
    runningLockMgr->Lock(remoteObj);
    locks = runningLockMgr->GetEnabledRunningLocksByType(RunningLockType::RUNNINGLOCK_BACKGROUND_USER_IDLE);
    int32_t lockSize = static_cast<int32_t>(locks.size());
    EXPECT_TRUE(lockSize != 0);
    sptr<IRemoteObject> remoteObjTokenInvalid = new RunningLockTokenStub();
    runningLockMgr->runningLocks_.emplace(remoteObjTokenInvalid, nullptr);
    locks = runningLockMgr->GetEnabledRunningLocksByType(RunningLockType::RUNNINGLOCK_BACKGROUND_USER_IDLE);
    EXPECT_TRUE(lockSize == locks.size());
    sptr<IRemoteObject> remoteObjTokenOne = new RunningLockTokenStub();
    RunningLockParam runningLockParamOne {0,
        "RunningLockNative028_2", "", RunningLockType::RUNNINGLOCK_BACKGROUND_TASK, TIMEOUTMS, pid, uid};
    EXPECT_TRUE(runningLockMgr->CreateRunningLock(remoteObjTokenOne, runningLockParamOne) != nullptr);
    runningLockMgr->Lock(remoteObjTokenOne);
    locks = runningLockMgr->GetEnabledRunningLocksByType(RunningLockType::RUNNINGLOCK_BACKGROUND_USER_IDLE);
    EXPECT_TRUE(lockSize == locks.size());
    runningLockMgr->UnLock(remoteObj);
    runningLockMgr->UnLock(remoteObjTokenOne);
    POWER_HILOGI(LABEL_TEST, "RunningLockNative028 function end!");
}

/**
 * @tc.name: RunningLockNative029
 * @tc.desc: test ForceUnlockWriteHiSysEvent
 * @tc.type: FUNC
 * @tc.require: issues/1585
 */
HWTEST_F(RunningLockNativeTest, RunningLockNative029, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockNative029 function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    auto runningLockMgr = std::make_shared<RunningLockMgr>(pmsTest);
    EXPECT_TRUE(runningLockMgr->Init());
    sptr<IRemoteObject> remoteObj = new RunningLockTokenStub();
    int32_t pid = IPCSkeleton::GetCallingPid();
    int32_t uid = IPCSkeleton::GetCallingUid();
    RunningLockParam runningLockParam {0,
        "RunningLockNative029", "", RunningLockType::RUNNINGLOCK_BACKGROUND_USER_IDLE, TIMEOUTMS, pid, uid};
    EXPECT_TRUE(runningLockMgr->CreateRunningLock(remoteObj, runningLockParam) != nullptr);
    runningLockMgr->Lock(remoteObj);
    EXPECT_FALSE(runningLockMgr->ForceUnlockWriteHiSysEvent(nullptr, ""));
    EXPECT_TRUE(runningLockMgr->ForceUnlockWriteHiSysEvent(nullptr, "RunningLockNative029"));
    EXPECT_TRUE(runningLockMgr->ForceUnlockWriteHiSysEvent(remoteObj, "RunningLockNative029"));
    runningLockMgr->UnLock(remoteObj);
    POWER_HILOGI(LABEL_TEST, "RunningLockNative029 function end!");
}

#ifdef POWER_MANAGER_ENABLE_MONITOR_RUNNING_LOCK_CHANGE
/**
 * @tc.name: RunningLockNative030
 * @tc.desc: test RegisterRunningLockChangedCallback and UnRegisterRunningLockChangedCallback
 * @tc.type: FUNC
 */
HWTEST_F(RunningLockNativeTest, RunningLockNative030, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockNative030 function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    pmsTest->OnStart();
    auto runningLockMgr = pmsTest->GetRunningLockMgr();
    
    class TestRunningLockChangedCallback : public RunningLockChangedCallbackStub {
    public:
        TestRunningLockChangedCallback() : stateReceived_(false) {}
        virtual ~TestRunningLockChangedCallback() = default;
        void OnAsyncScreenRunningLockChanged(RunningLockChangeState state) override
        {
            stateReceived_ = true;
            lastState_ = state;
        }
        bool stateReceived_;
        RunningLockChangeState lastState_;
    };
    
    sptr<TestRunningLockChangedCallback> callback = new TestRunningLockChangedCallback();
    int32_t pid = IPCSkeleton::GetCallingPid();
    int32_t uid = IPCSkeleton::GetCallingUid();
    
    runningLockMgr->RegisterRunningLockChangedCallback(callback->AsObject(), pid, uid);
    EXPECT_EQ(runningLockMgr->runningLockChangedCallbacks_.size(), 1);
    
    runningLockMgr->UnRegisterRunningLockChangedCallback(callback->AsObject());
    EXPECT_EQ(runningLockMgr->runningLockChangedCallbacks_.size(), 0);
    
    POWER_HILOGI(LABEL_TEST, "RunningLockNative030 function end!");
}

/**
 * @tc.name: RunningLockNative031
 * @tc.desc: test RegisterRunningLockChangedCallback with null callback
 * @tc.type: FUNC
 */
HWTEST_F(RunningLockNativeTest, RunningLockNative031, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockNative031 function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    pmsTest->OnStart();
    auto runningLockMgr = pmsTest->GetRunningLockMgr();
    
    int32_t pid = IPCSkeleton::GetCallingPid();
    int32_t uid = IPCSkeleton::GetCallingUid();
    size_t initialSize = runningLockMgr->runningLockChangedCallbacks_.size();
    
    runningLockMgr->RegisterRunningLockChangedCallback(nullptr, pid, uid);
    EXPECT_EQ(runningLockMgr->runningLockChangedCallbacks_.size(), initialSize);
    
    POWER_HILOGI(LABEL_TEST, "RunningLockNative031 function end!");
}

/**
 * @tc.name: RunningLockNative032
 * @tc.desc: test UnRegisterRunningLockChangedCallback with null callback
 * @tc.type: FUNC
 */
HWTEST_F(RunningLockNativeTest, RunningLockNative032, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockNative032 function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    pmsTest->OnStart();
    auto runningLockMgr = pmsTest->GetRunningLockMgr();
    size_t initialSize = runningLockMgr->runningLockChangedCallbacks_.size();
    
    runningLockMgr->UnRegisterRunningLockChangedCallback(nullptr);
    EXPECT_EQ(runningLockMgr->runningLockChangedCallbacks_.size(), initialSize);
    
    POWER_HILOGI(LABEL_TEST, "RunningLockNative032 function end!");
}

/**
 * @tc.name: RunningLockNative033
 * @tc.desc: test NotifyRunningLockChanged
 * @tc.type: FUNC
 */
HWTEST_F(RunningLockNativeTest, RunningLockNative033, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockNative033 function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    pmsTest->OnStart();
    auto runningLockMgr = pmsTest->GetRunningLockMgr();
    
    class TestRunningLockChangedCallback : public RunningLockChangedCallbackStub {
    public:
        TestRunningLockChangedCallback() : stateReceived_(false) {}
        virtual ~TestRunningLockChangedCallback() = default;
        void OnAsyncScreenRunningLockChanged(RunningLockChangeState state) override
        {
            stateReceived_ = true;
            lastState_ = state;
        }
        bool stateReceived_;
        RunningLockChangeState lastState_;
    };
    
    sptr<TestRunningLockChangedCallback> callback = new TestRunningLockChangedCallback();
    int32_t pid = IPCSkeleton::GetCallingPid();
    int32_t uid = IPCSkeleton::GetCallingUid();
    
    runningLockMgr->RegisterRunningLockChangedCallback(callback->AsObject(), pid, uid);
    
    runningLockMgr->NotifyScreenRunningLockChanged(RunningLockChangeState::RUNNINGLOCK_STATE_LOCKED);
    EXPECT_TRUE(callback->stateReceived_);
    EXPECT_EQ(callback->lastState_, RunningLockChangeState::RUNNINGLOCK_STATE_LOCKED);
    
    callback->stateReceived_ = false;
    runningLockMgr->NotifyScreenRunningLockChanged(RunningLockChangeState::RUNNINGLOCK_STATE_UNLOCKED);
    EXPECT_TRUE(callback->stateReceived_);
    EXPECT_EQ(callback->lastState_, RunningLockChangeState::RUNNINGLOCK_STATE_UNLOCKED);
    
    runningLockMgr->UnRegisterRunningLockChangedCallback(callback->AsObject());
    
    POWER_HILOGI(LABEL_TEST, "RunningLockNative033 function end!");
}

/**
 * @tc.name: RunningLockNative034
 * @tc.desc: test NotifyRunningLockChanged with lock active
 * @tc.type: FUNC
 */
HWTEST_F(RunningLockNativeTest, RunningLockNative034, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockNative034 function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    pmsTest->OnStart();
    auto runningLockMgr = pmsTest->GetRunningLockMgr();
    
    class TestRunningLockChangedCallback : public RunningLockChangedCallbackStub {
    public:
        TestRunningLockChangedCallback() : stateReceived_(false) {}
        virtual ~TestRunningLockChangedCallback() = default;
        void OnAsyncScreenRunningLockChanged(RunningLockChangeState state) override
        {
            stateReceived_ = true;
            lastState_ = state;
        }
        bool stateReceived_;
        RunningLockChangeState lastState_;
    };
    
    sptr<TestRunningLockChangedCallback> callback = new TestRunningLockChangedCallback();
    int32_t pid = IPCSkeleton::GetCallingPid();
    int32_t uid = IPCSkeleton::GetCallingUid();
    
    runningLockMgr->RegisterRunningLockChangedCallback(callback->AsObject(), pid, uid);
    
    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockParam runningLockParam {0,
        "RunningLockNative034", "", RunningLockType::RUNNINGLOCK_SCREEN, TIMEOUTMS, pid, uid};
    EXPECT_TRUE(runningLockMgr->CreateRunningLock(token, runningLockParam) != nullptr);
    runningLockMgr->Lock(token);
    
    EXPECT_TRUE(callback->stateReceived_);
    EXPECT_EQ(callback->lastState_, RunningLockChangeState::RUNNINGLOCK_STATE_LOCKED);
    
    runningLockMgr->UnLock(token);
    runningLockMgr->ReleaseLock(token);
    runningLockMgr->UnRegisterRunningLockChangedCallback(callback->AsObject());
    
    POWER_HILOGI(LABEL_TEST, "RunningLockNative034 function end!");
}

/**
 * @tc.name: RunningLockNative035
 * @tc.desc: test NotifyRunningLockChanged with lock inactive
 * @tc.type: FUNC
 */
HWTEST_F(RunningLockNativeTest, RunningLockNative035, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockNative035 function function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    pmsTest->OnStart();
    auto runningLockMgr = pmsTest->GetRunningLockMgr();
    
    class TestRunningLockChangedCallback : public RunningLockChangedCallbackStub {
    public:
        TestRunningLockChangedCallback() : stateReceived_(false) {}
        virtual ~TestRunningLockChangedCallback() = default;
        void OnAsyncScreenRunningLockChanged(RunningLockChangeState state) override
        {
            stateReceived_ = true;
            lastState_ = state;
        }
        bool stateReceived_;
        RunningLockChangeState lastState_;
    };
    
    sptr<TestRunningLockChangedCallback> callback = new TestRunningLockChangedCallback();
    int32_t pid = IPCSkeleton::GetCallingPid();
    int32_t uid = IPCSkeleton::GetCallingUid();
    
    runningLockMgr->RegisterRunningLockChangedCallback(callback->AsObject(), pid, uid);
    
    EXPECT_FALSE(callback->stateReceived_);
    
    runningLockMgr->UnRegisterRunningLockChangedCallback(callback->AsObject());
    
    EXPECT_TRUE(runningLockMgr != nullptr);
    
    POWER_HILOGI(LABEL_TEST, "RunningLockNative035 function end!");
}

/**
 * @tc.name: RunningLockNative036
 * @tc.desc: test RunningLockMgr is not null
 * @tc.type: FUNC
 */
HWTEST_F(RunningLockNativeTest, RunningLockNative036, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockNative036 function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    pmsTest->OnStart();
    auto runningLockMgr = pmsTest->GetRunningLockMgr();
    
    EXPECT_TRUE(runningLockMgr != nullptr);
    
    POWER_HILOGI(LABEL_TEST, "RunningLockNative036 function end!");
}

/**
 * @tc.name: RunningLockNative037
 * @tc.desc: test RegisterRunningLockChangedCallback duplicate
 * @tc.type: FUNC
 */
HWTEST_F(RunningLockNativeTest, RunningLockNative037, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockNative037 function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    pmsTest->OnStart();
    auto runningLockMgr = pmsTest->GetRunningLockMgr();
    
    class TestRunningLockChangedCallback : public RunningLockChangedCallbackStub {
    public:
        TestRunningLockChangedCallback() = default;
        virtual ~TestRunningLockChangedCallback() = default;
    };
    
    sptr<TestRunningLockChangedCallback> callback = new TestRunningLockChangedCallback();
    int32_t pid = IPCSkeleton::GetCallingPid();
    int32_t uid = IPCSkeleton::GetCallingUid();
    
    runningLockMgr->RegisterRunningLockChangedCallback(callback->AsObject(), pid, uid);
    size_t sizeAfterFirstRegister = runningLockMgr->runningLockChangedCallbacks_.size();
    
    runningLockMgr->RegisterRunningLockChangedCallback(callback->AsObject(), pid, uid);
    size_t sizeAfterSecondRegister = runningLockMgr->runningLockChangedCallbacks_.size();
    
    EXPECT_EQ(sizeAfterFirstRegister, sizeAfterSecondRegister);
    
    runningLockMgr->UnRegisterRunningLockChangedCallback(callback->AsObject());
    EXPECT_EQ(runningLockMgr->runningLockChangedCallbacks_.size(), 0);
    
    POWER_HILOGI(LABEL_TEST, "RunningLockNative037 function end!");
}

/**
 * @tc.name: RunningLockNative038
 * @tc.desc: test UnRegisterRunningLockChangedCallback not exists
 * @tc.type: FUNC
 */
HWTEST_F(RunningLockNativeTest, RunningLockNative038, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockNative038 function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    pmsTest->OnStart();
    auto runningLockMgr = pmsTest->GetRunningLockMgr();
    
    class TestRunningLockChangedCallback : public RunningLockChangedCallbackStub {
    public:
        TestRunningLockChangedCallback() = default;
        virtual ~TestRunningLockChangedCallback() = default;
    };
    
    sptr<TestRunningLockChangedCallback> callback = new TestRunningLockChangedCallback();
    size_t initialSize = runningLockMgr->runningLockChangedCallbacks_.size();
    
    runningLockMgr->UnRegisterRunningLockChangedCallback(callback->AsObject());
    EXPECT_EQ(runningLockMgr->runningLockChangedCallbacks_.size(), initialSize);
    
    POWER_HILOGI(LABEL_TEST, "RunningLockNative038 function end!");
}

/**
 * @tc.name: RunningLockNative039
 * @tc.desc: test NotifyRunningLockChanged with multiple callbacks
 * @tc.type: FUNC
 */
HWTEST_F(RunningLockNativeTest, RunningLockNative039, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockNative039 function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    pmsTest->OnStart();
    auto runningLockMgr = pmsTest->GetRunningLockMgr();
    
    class TestRunningLockChangedCallback : public RunningLockChangedCallbackStub {
    public:
        TestRunningLockChangedCallback() : stateReceived_(false) {}
        virtual ~TestRunningLockChangedCallback() = default;
        void OnAsyncScreenRunningLockChanged(RunningLockChangeState state) override
        {
            stateReceived_ = true;
            lastState_ = state;
        }
        bool stateReceived_;
        RunningLockChangeState lastState_;
    };
    
    sptr<TestRunningLockChangedCallback> callback1 = new TestRunningLockChangedCallback();
    sptr<TestRunningLockChangedCallback> callback2 = new TestRunningLockChangedCallback();
    int32_t pid = IPCSkeleton::GetCallingPid();
    int32_t uid = IPCSkeleton::GetCallingUid();
    
    runningLockMgr->RegisterRunningLockChangedCallback(callback1->AsObject(), pid, uid);
    runningLockMgr->RegisterRunningLockChangedCallback(callback2->AsObject(), pid, uid);
    
    runningLockMgr->NotifyScreenRunningLockChanged(RunningLockChangeState::RUNNINGLOCK_STATE_LOCKED);
    EXPECT_TRUE(callback1->stateReceived_);
    EXPECT_TRUE(callback2->stateReceived_);
    
    runningLockMgr->UnRegisterRunningLockChangedCallback(callback1->AsObject());
    runningLockMgr->UnRegisterRunningLockChangedCallback(callback2->AsObject());
    
    POWER_HILOGI(LABEL_TEST, "RunningLockNative039 function end!");
}

/**
 * @tc.name: RunningLockNative040
 * @tc.desc: test NotifyRunningLockChanged with no callbacks
 * @tc.type: FUNC
 */
HWTEST_F(RunningLockNativeTest, RunningLockNative040, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockNative040 function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    pmsTest->OnStart();
    auto runningLockMgr = pmsTest->GetRunningLockMgr();
    
    runningLockMgr->NotifyScreenRunningLockChanged(RunningLockChangeState::RUNNINGLOCK_STATE_LOCKED);
    
    EXPECT_TRUE(runningLockMgr != nullptr);
    
    POWER_HILOGI(LABEL_TEST, "RunningLockNative040 function end!");
}

/**
 * @tc.name: RunningLockNative041
 * @tc.desc: test PowerMgrService RegisterRunningLockChangedCallback
 * @tc.type: FUNC
 */
HWTEST_F(RunningLockNativeTest, RunningLockNative041, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockNative041 function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    pmsTest->OnStart();
    
    class TestRunningLockChangedCallback : public RunningLockChangedCallbackStub {
    public:
        TestRunningLockChangedCallback() = default;
        virtual ~TestRunningLockChangedCallback() = default;
    };
    
    sptr<TestRunningLockChangedCallback> callback = new TestRunningLockChangedCallback();
    
    PowerErrors ret = pmsTest->RegisterRunningLockChangedCallback(callback);
    EXPECT_TRUE(ret == PowerErrors::ERR_OK);
    
    ret = pmsTest->UnRegisterRunningLockChangedCallback(callback);
    EXPECT_TRUE(ret == PowerErrors::ERR_OK);
    
    POWER_HILOGI(LABEL_TEST, "RunningLockNative041 function end!");
}

/**
 * @tc.name: RunningLockNative042
 * @tc.desc: test PowerMgrService RegisterRunningLockChangedCallback with null callback
 * @tc.type: FUNC
 */
HWTEST_F(RunningLockNativeTest, RunningLockNative042, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockNative042 function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    pmsTest->OnStart();
    
    PowerErrors ret = pmsTest->RegisterRunningLockChangedCallback(nullptr);
    EXPECT_TRUE(ret == PowerErrors::ERR_PARAM_INVALID);
    
    POWER_HILOGI(LABEL_TEST, "RunningLockNative042 function end!");
}

/**
 * @tc.name: RunningLockNative043
 * @tc.desc: test PowerMgrService UnRegisterRunningLockChangedCallback with null callback
 * @tc.type: FUNC
 */
HWTEST_F(RunningLockNativeTest, RunningLockNative043, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockNative043 function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    pmsTest->OnStart();
    
    PowerErrors ret = pmsTest->UnRegisterRunningLockChangedCallback(nullptr);
    EXPECT_TRUE(ret == PowerErrors::ERR_PARAM_INVALID);
    
    POWER_HILOGI(LABEL_TEST, "RunningLockNative043 function end!");
}

/**
 * @tc.name: RunningLockNative044
 * @tc.desc: test DeathRecipient RemoveDeathRecipientObj
 * @tc.type: FUNC
 */
HWTEST_F(RunningLockNativeTest, RunningLockNative044, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockNative044 function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    pmsTest->OnStart();
    
    class TestRunningLockChangedCallback : public RunningLockChangedCallbackStub {
    public:
        TestRunningLockChangedCallback() = default;
        virtual ~TestRunningLockChangedCallback() = default;
    };
    
    sptr<TestRunningLockChangedCallback> callback = new TestRunningLockChangedCallback();
    
    PowerErrors ret = pmsTest->RegisterRunningLockChangedCallback(callback);
    EXPECT_TRUE(ret == PowerErrors::ERR_OK);
    
    DeathRecipientManager::GetInstance().RemoveDeathRecipientObj(callback->AsObject());
    
    ret = pmsTest->UnRegisterRunningLockChangedCallback(callback);
    
    POWER_HILOGI(LABEL_TEST, "RunningLockNative044 function end!");
}

/**
 * @tc.name: RunningLockNative045
 * @tc.desc: test DeathRecipient RemoveDeathRecipientObj with non-existent callback
 * @tc.type: FUNC
 */
HWTEST_F(RunningLockNativeTest, RunningLockNative045, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockNative045 function start!");
    
    class TestRunningLockChangedCallback : public RunningLockChangedCallbackStub {
    public:
        TestRunningLockChangedCallback() = default;
        virtual ~TestRunningLockChangedCallback() = default;
    };
    
    sptr<TestRunningLockChangedCallback> callback = new TestRunningLockChangedCallback();
    
    DeathRecipientManager::GetInstance().RemoveDeathRecipientObj(callback->AsObject());
    
    EXPECT_TRUE(callback != nullptr);
    
    POWER_HILOGI(LABEL_TEST, "RunningLockNative045 function end!");
}

/**
 * @tc.name: RunningLockNative046
 * @tc.desc: test DeathRecipient RemoveDeathRecipientObj with null callback
 * @tc.type: FUNC
 */
HWTEST_F(RunningLockNativeTest, RunningLockNative046, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockNative046 function start!");
    
    DeathRecipientManager::GetInstance().RemoveDeathRecipientObj(nullptr);
    
    EXPECT_TRUE(true);
    
    POWER_HILOGI(LABEL_TEST, "RunningLockNative046 function end!");
}
#endif
} // namespace
