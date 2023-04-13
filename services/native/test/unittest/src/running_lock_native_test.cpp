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

void PowerShutdownTest1Callback::ShutdownCallback()
{
    POWER_HILOGI(LABEL_TEST, "PowerShutdownTest1Callback::ShutdownCallback");
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
    int32_t pid = IPCSkeleton::GetCallingPid();
    int32_t uid = IPCSkeleton::GetCallingUid();
    RunningLockParam runningLockParam {
        "runninglockNativeTest1", RunningLockType::RUNNINGLOCK_SCREEN, TIMEOUTMS, pid, uid};
    EXPECT_TRUE(runningLockMgr->CreateRunningLock(remoteObj, runningLockParam) != nullptr);
    EXPECT_FALSE(runningLockMgr->IsUsed(remoteObj));
    runningLockMgr->Lock(remoteObj, TIMEOUTMS);
    EXPECT_TRUE(runningLockMgr->GetRunningLockNum(RunningLockType::RUNNINGLOCK_SCREEN) == LOCKNUM_B);
    EXPECT_TRUE(runningLockMgr->GetRunningLockNum(RunningLockType::RUNNINGLOCK_BUTT) == LOCKNUM_B);
    EXPECT_TRUE(runningLockMgr->GetValidRunningLockNum(RunningLockType::RUNNINGLOCK_SCREEN) == LOCKNUM_B);
    EXPECT_TRUE(runningLockMgr->GetValidRunningLockNum(static_cast<RunningLockType>(MAXTYPE)) == LOCKNUM_A);
    EXPECT_TRUE(runningLockMgr->IsUsed(remoteObj));
    sptr<IRemoteObject> token = new RunningLockTokenStub();
    EXPECT_FALSE(runningLockMgr->IsUsed(token));
    runningLockMgr->Lock(token, TIMEOUTMS);
    runningLockMgr->UnLock(remoteObj);
    runningLockMgr->UnLock(token);

    EXPECT_FALSE(runningLockMgr->ReleaseLock(remoteObj));
    EXPECT_FALSE(runningLockMgr->ReleaseLock(token));

    POWER_HILOGI(LABEL_TEST, "RunningLockNative001 end");
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
    sptr<IRemoteObject> remoteObj = new RunningLockTokenStub();
    int32_t pid = IPCSkeleton::GetCallingPid();
    int32_t uid = IPCSkeleton::GetCallingUid();
    RunningLockParam runningLockParam {
        "runninglockNativeTest1", RunningLockType::RUNNINGLOCK_SCREEN, TIMEOUTMS, pid, uid};
    EXPECT_TRUE(runningLockMgr->CreateRunningLock(remoteObj, runningLockParam) != nullptr);
    EXPECT_FALSE(runningLockMgr->ExistValidRunningLock());
    runningLockMgr->Lock(remoteObj, TIMEOUTMS);
    EXPECT_TRUE(runningLockMgr->ExistValidRunningLock() == true);
    runningLockMgr->CheckOverTime();
    runningLockMgr->CheckOverTime();

    runningLockMgr->ProxyRunningLock(false, pid, uid);
    runningLockMgr->ProxyRunningLock(true, pid, uid);
    runningLockMgr->ProxyRunningLock(true, pid, uid);
    runningLockMgr->ProxyRunningLock(true, UNPID, UID);
    runningLockMgr->ProxyRunningLock(true, UNPID, UID);
    runningLockMgr->ProxyRunningLock(false, pid, uid);
    runningLockMgr->ProxyRunningLock(false, UNPID, UID);
    runningLockMgr->UnLock(remoteObj);
    EXPECT_FALSE(runningLockMgr->ReleaseLock(remoteObj));
    POWER_HILOGI(LABEL_TEST, "RunningLockNative002 end");
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

    RunningLockParam runningLockParam {
        "runninglockNativeTest2", RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL, TIMEOUTMS, PID, UID};
    sptr<IRemoteObject> token = new RunningLockTokenStub();
    EXPECT_TRUE(runningLockMgr->CreateRunningLock(token, runningLockParam) != nullptr);
    runningLockMgr->DumpInfo(result);
    runningLockMgr->Lock(token, TIMEOUTMS);
    runningLockMgr->UnLock(token);

    EXPECT_FALSE(runningLockMgr->ReleaseLock(token));
    POWER_HILOGI(LABEL_TEST, "RunningLockNative003 end");
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

    RunningLockParam runningLockParam {
        "runninglockNativeTest", RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL, TIMEOUTMS, PID, UID};
    sptr<IRemoteObject> token = new RunningLockTokenStub();
    EXPECT_TRUE(runningLockMgr->CreateRunningLock(token, runningLockParam) != nullptr);
    runningLockMgr->Lock(token, TIMEOUTMS);
    runningLockMgr->SetProximity(RunningLockMgr::PROXIMITY_AWAY);
    runningLockMgr->SetProximity(RunningLockMgr::PROXIMITY_CLOSE);
    runningLockMgr->SetProximity(RunningLockMgr::PROXIMITY_CLOSE);
    runningLockMgr->SetProximity(RunningLockMgr::PROXIMITY_AWAY);
    runningLockMgr->UnLock(token);

    RunningLockParam runningLockParam2 {
        "runninglockNativeTest2", static_cast<RunningLockType>(7U), TIMEOUTMS, PID_A, UID_A};
    sptr<IRemoteObject> remoteObj = new RunningLockTokenStub();
    EXPECT_TRUE(runningLockMgr->CreateRunningLock(remoteObj, runningLockParam2) != nullptr);
    runningLockMgr->Lock(remoteObj, TIMEOUTMS);
    runningLockMgr->UnLock(remoteObj);
    POWER_HILOGI(LABEL_TEST, "RunningLockNative004 end");
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
    sptr<IRemoteObject> remoteObj = new RunningLockTokenStub();
    RunningLockParam runningLockParam {
        "runninglockNativeTest1", RunningLockType::RUNNINGLOCK_SCREEN, TIMEOUTMS, UNPID, UNUID};
    EXPECT_TRUE(runningLockMgr->CreateRunningLock(remoteObj, runningLockParam) != nullptr);
    runningLockMgr->Lock(remoteObj, TIMEOUTMS);
    runningLockMgr->Lock(remoteObj, TIMEOUTMS);
    RunningLockParam runningLockParam1 {
        "runninglockNativeTest2", static_cast<RunningLockType>(MAXTYPE), TIMEOUTMS, UNPID, UNUID};
    sptr<IRemoteObject> token = new RunningLockTokenStub();
    EXPECT_TRUE(runningLockMgr->CreateRunningLock(token, runningLockParam1) != nullptr);
    runningLockMgr->Lock(token, TIMEOUTMS);
    runningLockMgr->UnLock(token);

    EXPECT_FALSE(runningLockMgr->ReleaseLock(remoteObj));
    EXPECT_FALSE(runningLockMgr->ReleaseLock(token));
    POWER_HILOGI(LABEL_TEST, "RunningLockNative005 end");
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
    RunningLockParam runningLockParam {
        "runninglockNativeTest1", RunningLockType::RUNNINGLOCK_SCREEN, TIMEOUTMS, UNPID, UNUID};
    sptr<IRemoteObject> remoteObj = new RunningLockTokenStub();
    EXPECT_TRUE(runningLockMgr->CreateRunningLock(remoteObj, runningLockParam) != nullptr);
    runningLockMgr->Lock(remoteObj, TIMEOUTMS);

    RunningLockMgr::RunningLockChangedType type = RunningLockMgr::RunningLockChangedType::NOTIFY_RUNNINGLOCK_OVERTIME;
    auto lockInner = RunningLockInner::CreateRunningLockInner(runningLockParam);
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
    POWER_HILOGI(LABEL_TEST, "RunningLockNative006 end");
}

/**
 * @tc.name: RunningLockNative007
 * @tc.desc: test CheckOverTime in runningLockMgr
 * @tc.type: FUNC
 */
HWTEST_F (RunningLockNativeTest, RunningLockNative007, TestSize.Level0)
{
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    auto runningLockMgr = std::make_shared<RunningLockMgr>(pmsTest);
    EXPECT_TRUE(runningLockMgr->Init());
    sptr<IRemoteObject> remoteObj = new RunningLockTokenStub();
    std::weak_ptr<PowermsEventHandler> powermsEventHandler;
    runningLockMgr->handler_ = powermsEventHandler;
    runningLockMgr->CheckOverTime();
    runningLockMgr->RemoveAndPostUnlockTask(remoteObj);
    runningLockMgr->SendCheckOverTimeMsg(CALLTIMEMS);
    POWER_HILOGI(LABEL_TEST, "RunningLockNative007 end");
}
}
