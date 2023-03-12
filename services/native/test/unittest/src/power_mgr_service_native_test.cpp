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

#include "power_mgr_service_native_test.h"

#include <csignal>
#include <iostream>

#include "power_mgr_service.h"
#include "key_event.h"
#include "pointer_event.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

sptr<PowerMgrService> g_pmsTest;

void PowerMgrServiceNativeTest::SetUpTestCase()
{
}

void PowerMgrServiceNativeTest::TearDownTestCase()
{
}

void PowerMgrServiceNativeTest::SetUp()
{
    g_pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    EXPECT_TRUE(g_pmsTest != nullptr) << "PowerMgrService02 fail to get PowerMgrService";
    g_pmsTest->OnStart();

    auto stateMachine = std::make_shared<PowerStateMachine>(g_pmsTest);
    EXPECT_TRUE(stateMachine->Init());
    UserActivityType userActivityType = UserActivityType::USER_ACTIVITY_TYPE_ACCESSIBILITY;
    stateMachine->RefreshActivityInner(PID, CALLTIMEMS, userActivityType, true);
    int32_t powermsEvent = PowermsEventHandler::CHECK_USER_ACTIVITY_OFF_TIMEOUT_MSG;
    stateMachine->HandleDelayTimer(powermsEvent);
    auto runningLockMgr = std::make_shared<RunningLockMgr>(g_pmsTest);
    EXPECT_TRUE(runningLockMgr->Init());
    sptr<IRemoteObject> remoteObj = new RunningLockTokenStub();
    RunningLockInfo runningLockInfo {"runninglockNativeTest1", RunningLockType::RUNNINGLOCK_SCREEN};
    UserIPCInfo userIPCinfo {UID, PID};
    EXPECT_TRUE(runningLockMgr->CreateRunningLock(remoteObj, runningLockInfo, userIPCinfo) != nullptr);
    runningLockMgr->Lock(remoteObj, runningLockInfo, userIPCinfo, TIMEOUTMS);
    EXPECT_FALSE(runningLockMgr->ReleaseLock(remoteObj));
}

void PowerMgrServiceNativeTest::TearDown()
{
    g_pmsTest->OnStop();
}

void PowerStateTestCallback::OnPowerStateChanged(PowerState state)
{
    POWER_HILOGI(LABEL_TEST, "PowerStateTestCallback::OnPowerStateChanged state = %u.",
        static_cast<uint32_t>(state));
}

/**
 * @tc.name: PowerMgrServiceNative001
 * @tc.desc: test init in powerMgrService
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceNativeTest, PowerMgrServiceNative001, TestSize.Level0)
{
    g_pmsTest->KeyMonitorInit();
    g_pmsTest->KeyMonitorCancel();

    g_pmsTest->HallSensorSubscriberInit();
    g_pmsTest->HallSensorSubscriberCancel();

    g_pmsTest->HandleShutdownRequest();
    g_pmsTest->HandleScreenOnTimeout();
    int32_t keyCode = OHOS::MMI::KeyEvent::KEYCODE_F1;
    g_pmsTest->HandleKeyEvent(keyCode);
    int32_t type = OHOS::MMI::PointerEvent::SOURCE_TYPE_MOUSE;
    g_pmsTest->HandlePointEvent(type);
    g_pmsTest->HandleScreenOnTimeout();

    EXPECT_TRUE(g_pmsTest->OverrideScreenOffTime(TIMEOUTMS));
    EXPECT_TRUE(g_pmsTest->RestoreScreenOffTime());

    g_pmsTest->powerkeyLongPressId_ = UNCANCELID;
    g_pmsTest->powerkeyShortPressId_ = UNCANCELID;
    g_pmsTest->powerkeyReleaseId_ = UNCANCELID;
    g_pmsTest->doubleClickId_ = UNCANCELID;
    g_pmsTest->monitorId_ = UNCANCELID;
    g_pmsTest->KeyMonitorCancel();
    g_pmsTest->KeyMonitorInit();
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative001 end.");
}

/**
 * @tc.name: PowerMgrServiceNative002
 * @tc.desc: test handleKeyEvent in powerMgrService
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceNativeTest, PowerMgrServiceNative002, TestSize.Level0)
{
    int32_t keyCode = OHOS::MMI::KeyEvent::KEYCODE_F1;
    g_pmsTest->HandleKeyEvent(keyCode);
    keyCode = OHOS::MMI::KeyEvent::KEYCODE_F2;
    g_pmsTest->HandleKeyEvent(keyCode);
    keyCode = OHOS::MMI::KeyEvent::KEYCODE_VIRTUAL_MULTITASK;
    g_pmsTest->HandleKeyEvent(keyCode);
    keyCode = OHOS::MMI::KeyEvent::KEYCODE_WEAR_1;
    g_pmsTest->HandleKeyEvent(keyCode);
    int32_t type = OHOS::MMI::PointerEvent::SOURCE_TYPE_MOUSE;
    g_pmsTest->HandlePointEvent(type);
    type = OHOS::MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN;
    g_pmsTest->HandlePointEvent(type);

    EXPECT_TRUE(g_pmsTest->ShutDownDevice(SHUTDOWN_FAST_REASON) == PowerErrors::ERR_OK);
    SuspendDeviceType reasonSDT = SuspendDeviceType::SUSPEND_DEVICE_REASON_DEVICE_ADMIN;
    EXPECT_TRUE(g_pmsTest->SuspendDevice(SUSCALLTIMEMS, reasonSDT, false) == PowerErrors::ERR_OK);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative002 end.");
}

/**
 * @tc.name: PowerMgrServiceNative003
 * @tc.desc: test lock and unlock in powerMgrService
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceNativeTest, PowerMgrServiceNative003, TestSize.Level0)
{
    EXPECT_TRUE(g_pmsTest->OverrideScreenOffTime(TIMEOUTMS));
    EXPECT_TRUE(g_pmsTest->RestoreScreenOffTime());
    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo runningLockInfo1 {"runninglockNativeTest2", RunningLockType::RUNNINGLOCK_BACKGROUND};
    EXPECT_TRUE(g_pmsTest->CreateRunningLock(token, runningLockInfo1) == PowerErrors::ERR_OK);
    EXPECT_TRUE(g_pmsTest->IsRunningLockTypeSupported(RunningLockType::RUNNINGLOCK_SCREEN));
    EXPECT_FALSE(g_pmsTest->IsRunningLockTypeSupported(RunningLockType::RUNNINGLOCK_BUTT));
    EXPECT_TRUE(g_pmsTest->Lock(token, runningLockInfo1, SUSCALLTIMEMS));
    EXPECT_TRUE(g_pmsTest->ProxyRunningLock(token, UID, PID));
    EXPECT_TRUE(g_pmsTest->UnLock(token));
    EXPECT_FALSE(g_pmsTest->ReleaseRunningLock(token));

    g_pmsTest->shutdownService_.started_ = true;
    g_pmsTest->RebootDevice("-717");
    g_pmsTest->ForceSuspendDevice(UNCALLTIMEMS);
    EXPECT_TRUE(g_pmsTest->ShutDownDevice("st") == PowerErrors::ERR_OK);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative003 end.");
}

/**
 * @tc.name: PowerMgrServiceNative004
 * @tc.desc: test callback in powerMgrService
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceNativeTest, PowerMgrServiceNative004, TestSize.Level0)
{
    sptr<IPowerStateCallback> powerStatCb = new PowerStateTestCallback();
    EXPECT_TRUE(g_pmsTest->RegisterPowerStateCallback(powerStatCb));
    EXPECT_TRUE(g_pmsTest->UnRegisterPowerStateCallback(powerStatCb));

    EXPECT_TRUE(g_pmsTest->SetDisplaySuspend(true));
    PowerMode mode = PowerMode::POWER_SAVE_MODE;
    EXPECT_TRUE(g_pmsTest->SetDeviceMode(mode) == PowerErrors::ERR_OK);
    g_pmsTest->NotifyRunningLockChanged(true);
    g_pmsTest->NotifyRunningLockChanged(false);
    g_pmsTest->powerkeyPressed_ = true;
    g_pmsTest->HandleScreenOnTimeout();
    g_pmsTest->HandlePowerKeyUp();
    g_pmsTest->HandlePowerKeyUp();

    SensorEvent sensorEvent;
    HallData data;
    data.status = LID_CLOSED_HALL_FLAG;
    sensorEvent.sensorTypeId = SENSOR_TYPE_ID_HALL;
    sensorEvent.data = reinterpret_cast<uint8_t*>(&data);
    g_pmsTest->HallSensorCallback(&sensorEvent);
    data.status = LID_CLOSED_HALL_FLAG_B;
    g_pmsTest->HallSensorCallback(&sensorEvent);
    sensorEvent.data = nullptr;
    g_pmsTest->HallSensorCallback(&sensorEvent);
    sensorEvent.sensorTypeId = TIMEOUTMS;
    g_pmsTest->HallSensorCallback(&sensorEvent);
    g_pmsTest->HallSensorCallback(nullptr);

    g_pmsTest->ready_ = true;
    g_pmsTest->OnStart();
    g_pmsTest->OnStop();
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative004 end.");
}
