/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "power_mgr_client_test.h"

#include <iostream>

#include <datetime_ex.h>
#include <gtest/gtest.h>
#include <if_system_ability_manager.h>
#include <ipc_skeleton.h>
#include <string_ex.h>
#ifdef HAS_DISPLAY_MANAGER_PART
#include "display_power_mgr_client.h"
#endif
#include "power_common.h"
#include "power_mgr_client.h"
#include "power_mgr_service.h"
#include "power_state_machine.h"
#include "running_lock_token_stub.h"
#include "mock_power_mgr_client.h"
#include "mock_power_remote_object.h"
#include "takeover_suspend_callback_proxy.h"
#include "takeover_suspend_callback_stub.h"
#include "async_ulsr_callback_stub.h"
#include "shutdown/async_shutdown_callback_stub.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

void PowerMgrClientTest::SetUpTestCase(void)
{
    stub_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    stub_->OnStart();
}

void PowerMgrClientTest::TearDownTestCase(void) {}

void PowerMgrClientTest::SetUp(void)
{
    if (!stateActionMock || !shutdownStateActionMock || !powerActionMock || !lockActionMock) {
        stateActionMock = new NiceMock<MockStateAction>;
        shutdownStateActionMock = new NiceMock<MockStateAction>;
        powerActionMock = new NiceMock<MockPowerAction>;
        lockActionMock = new NiceMock<MockLockAction>;
        stub_->EnableMock(stateActionMock, shutdownStateActionMock, powerActionMock, lockActionMock);
    }
}

void PowerMgrClientTest::TearDown(void)
{
    stub_->EnableMock(nullptr, nullptr, nullptr, nullptr);
    // EnableMock for PowerStateMachine ignores nullptr, reset it manually
    auto& stateAction =
        const_cast<std::shared_ptr<IDeviceStateAction>&>(stub_->GetPowerStateMachine()->GetStateAction());
    stateAction.reset();
    stateActionMock = nullptr;
    shutdownStateActionMock = nullptr;
    powerActionMock = nullptr;
    lockActionMock = nullptr;
}

sptr<IPowerMgr> PowerMgrClient::GetPowerMgrProxy()
{
    return PowerMgrClientTest::stub_;
}

bool PowerMgr::Permission::IsSystem()
{
    return true;
}

bool PowerMgr::Permission::IsPermissionGranted(const std::string&)
{
    return true;
}

namespace {
constexpr int SLEEP_AFTER_LOCK_TIME_US = 1000 * 1000;

/**
 * @tc.name: PowerMgrClient001
 * @tc.desc: test RefreshActivity
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient001, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient001 function start!");
    uint32_t PARM_TWO = 2;
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    powerMgrClient.SuspendDevice();
    // Wakeup Device before test
    powerMgrClient.WakeupDevice();
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient001: Prepare Fail, Screen is OFF.";
    powerMgrClient.OverrideScreenOffTime(SCREEN_OFF_WAIT_TIME_MS);
    sleep(SCREEN_OFF_WAIT_TIME_S / PARM_TWO);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient001: Prepare Fail, Screen is OFF.";
    powerMgrClient.RefreshActivity();
    sleep(SCREEN_OFF_WAIT_TIME_S / PARM_TWO);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);

    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient001: RefreshActivity Device Fail, Screen is OFF";
    powerMgrClient.OverrideScreenOffTime(PowerStateMachine::DEFAULT_SLEEP_TIME_MS);
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient001 function end!");
}

/**
 * @tc.name: PowerMgrClient002
 * @tc.desc: test RefreshActivity
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient002, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient002 function start!");
    uint32_t PARM_TWO = 2;
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    powerMgrClient.SuspendDevice();
    // Wakeup Device before test
    powerMgrClient.WakeupDevice();
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient002: Prepare Fail, Screen is OFF.";
    powerMgrClient.OverrideScreenOffTime(SCREEN_OFF_WAIT_TIME_MS);
    sleep(SCREEN_OFF_WAIT_TIME_S / PARM_TWO);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient002: Prepare Fail, Screen is OFF.";
    powerMgrClient.RefreshActivity(UserActivityType::USER_ACTIVITY_TYPE_BUTTON);
    sleep(SCREEN_OFF_WAIT_TIME_S / PARM_TWO);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);

    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient002: RefreshActivity Device Fail, Screen is OFF";
    powerMgrClient.OverrideScreenOffTime(PowerStateMachine::DEFAULT_SLEEP_TIME_MS);
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient002 function end!");
}

/**
 * @tc.name: PowerMgrClient003
 * @tc.desc: test RefreshActivity
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient003, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient003 function start!");
    uint32_t PARM_TWO = 2;
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    powerMgrClient.SuspendDevice();
    // Wakeup Device before test
    powerMgrClient.WakeupDevice();
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient003: Prepare Fail, Screen is OFF.";
    powerMgrClient.OverrideScreenOffTime(SCREEN_OFF_WAIT_TIME_MS);
    sleep(SCREEN_OFF_WAIT_TIME_S / PARM_TWO);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient003: Prepare Fail, Screen is OFF.";
    powerMgrClient.RefreshActivity(UserActivityType::USER_ACTIVITY_TYPE_TOUCH);
    sleep(SCREEN_OFF_WAIT_TIME_S / PARM_TWO);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);

    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient003: RefreshActivity Device Fail, Screen is OFF";
    powerMgrClient.OverrideScreenOffTime(PowerStateMachine::DEFAULT_SLEEP_TIME_MS);
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient003 function end!");
}

/**
 * @tc.name: PowerMgrClient004
 * @tc.desc: test RefreshActivity
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient004, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient004 function start!");
    uint32_t PARM_TWO = 2;
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    powerMgrClient.SuspendDevice();
    // Wakeup Device before test
    powerMgrClient.WakeupDevice();
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient004: Prepare Fail, Screen is OFF.";
    powerMgrClient.OverrideScreenOffTime(SCREEN_OFF_WAIT_TIME_MS);
    sleep(SCREEN_OFF_WAIT_TIME_S / PARM_TWO);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient004: Prepare Fail, Screen is OFF.";
    powerMgrClient.RefreshActivity(UserActivityType::USER_ACTIVITY_TYPE_ACCESSIBILITY);
    sleep(SCREEN_OFF_WAIT_TIME_S / PARM_TWO);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);

    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient004: RefreshActivity Device Fail, Screen is OFF";
    powerMgrClient.OverrideScreenOffTime(PowerStateMachine::DEFAULT_SLEEP_TIME_MS);
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient004 function end!");
}

/**
 * @tc.name: PowerMgrClient005
 * @tc.desc: test RefreshActivity
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient005, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient005 function start!");
    uint32_t PARM_TWO = 2;
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    powerMgrClient.SuspendDevice();
    // Wakeup Device before test
    powerMgrClient.WakeupDevice();
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient005: Prepare Fail, Screen is OFF.";
    powerMgrClient.OverrideScreenOffTime(SCREEN_OFF_WAIT_TIME_MS);
    sleep(SCREEN_OFF_WAIT_TIME_S / PARM_TWO);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient005: Prepare Fail, Screen is OFF.";
    powerMgrClient.RefreshActivity(UserActivityType::USER_ACTIVITY_TYPE_ATTENTION);
    sleep(SCREEN_OFF_WAIT_TIME_S / PARM_TWO);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);

    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient005: RefreshActivity Device Fail, Screen is OFF";
    powerMgrClient.OverrideScreenOffTime(PowerStateMachine::DEFAULT_SLEEP_TIME_MS);
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient005 function end!");
}

/**
 * @tc.name: PowerMgrClient006
 * @tc.desc: test RefreshActivity
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient006, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient006 function start!");
    uint32_t PARM_TWO = 2;
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    powerMgrClient.SuspendDevice();
    // Wakeup Device before test
    powerMgrClient.WakeupDevice();
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient006: Prepare Fail, Screen is OFF.";
    powerMgrClient.OverrideScreenOffTime(SCREEN_OFF_WAIT_TIME_MS);
    sleep(SCREEN_OFF_WAIT_TIME_S / PARM_TWO);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient006: Prepare Fail, Screen is OFF.";
    powerMgrClient.RefreshActivity(UserActivityType::USER_ACTIVITY_TYPE_SOFTWARE);
    sleep(SCREEN_OFF_WAIT_TIME_S / PARM_TWO);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);

    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient006: RefreshActivity Device Fail, Screen is OFF";
    powerMgrClient.OverrideScreenOffTime(PowerStateMachine::DEFAULT_SLEEP_TIME_MS);
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient006 function end!");
}

/**
 * @tc.name: PowerMgrClient007
 * @tc.desc: test RefreshActivity
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient007, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient007 function start!");
    uint32_t PARM_TWO = 2;
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    UserActivityType abnormaltype = UserActivityType(9);

    powerMgrClient.SuspendDevice();
    // Wakeup Device before test
    powerMgrClient.WakeupDevice();
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient007: Prepare Fail, Screen is OFF.";
    powerMgrClient.OverrideScreenOffTime(SCREEN_OFF_WAIT_TIME_MS);
    usleep(SCREEN_OFF_WAIT_TIME_S * TRANSFER_NS_TO_MS / PARM_TWO);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient007: Prepare Fail, Screen is OFF.";
    powerMgrClient.RefreshActivity(abnormaltype);
    usleep(SCREEN_OFF_WAIT_TIME_S * TRANSFER_NS_TO_MS / PARM_TWO + WAIT_SUSPEND_TIME_MS * TRANSFER_MS_TO_S);
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_OFF));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerMgrClient007: RefreshActivity Device Fail, Screen is OFF";
    powerMgrClient.OverrideScreenOffTime(PowerStateMachine::DEFAULT_SLEEP_TIME_MS);
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient007 function end!");
}

#ifdef HAS_DISPLAY_MANAGER_PART
/**
 * @tc.name: PowerMgrClient008
 * @tc.desc: test DISPLAY_DIM
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient008, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient008 function start!");
    int64_t time = SCREEN_OFF_WAIT_TIME_MS;
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    auto& displayMgrClient = OHOS::DisplayPowerMgr::DisplayPowerMgrClient::GetInstance();

    powerMgrClient.OverrideScreenOffTime(time);
    powerMgrClient.SuspendDevice();
    // Wakeup Device before test
    powerMgrClient.WakeupDevice();
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient008: Prepare Fail, Screen is OFF.";
    usleep(time * TRANSFER_MS_TO_S * DIM_RATE);
    // waiting for GetDimState time
    usleep(DIM_STATE_WAIT_TIME_MS * TRANSFER_MS_TO_S);
    displayMgrClient.GetDisplayState();

    powerMgrClient.OverrideScreenOffTime(PowerStateMachine::DEFAULT_SLEEP_TIME_MS);
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient008 function end!");
}
#endif

/**
 * @tc.name: PowerMgrClient009
 * @tc.desc: test WakeupDevice
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient009, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient009 function start!");
    int32_t PARM_ONE = 1;
    int32_t wakeupReason = (static_cast<int32_t>(WakeupDeviceType::WAKEUP_DEVICE_MAX)) + PARM_ONE;
    WakeupDeviceType abnormaltype = WakeupDeviceType(wakeupReason);
    auto& powerMgrClient = MockPowerMgrClient::GetInstance();

    powerMgrClient.WakeupDevice();
    // Suspend Device before test
    powerMgrClient.SuspendDevice();
    sleep(SLEEP_WAIT_TIME_S);
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_OFF));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerMgrClient009: Prepare Fail, Screen is On.";
    powerMgrClient.WakeupDevice(abnormaltype);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerMgrClient009: Wakeup Device Fail, Screen is Off";

    POWER_HILOGI(LABEL_TEST, "PowerMgrClient009 function end!");
}

/**
 * @tc.name: PowerMgrClient010
 * @tc.desc: test Suspend Device
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient010, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient010 function start!");
    int32_t PARM_ONE = 1;
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    int32_t suspendReason = (static_cast<int32_t>(SuspendDeviceType::SUSPEND_DEVICE_REASON_MAX)) + PARM_ONE;
    SuspendDeviceType abnormaltype = SuspendDeviceType(suspendReason);

    powerMgrClient.SuspendDevice();
    // Wakeup Device before test
    powerMgrClient.WakeupDevice();
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient010: Prepare Fail, Screen is OFF.";
    powerMgrClient.SuspendDevice(abnormaltype, false);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);

    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient010: Suspend Device Fail, Screen is On";
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient010 function end!");
}

#ifdef HAS_DISPLAY_MANAGER_PART
/**
 * @tc.name: PowerMgrClient011
 * @tc.desc: test  auto suspend
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient011, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient011 function start!");
    int32_t time = SET_DISPLAY_OFF_TIME_MS;
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    auto& displayMgrClient = OHOS::DisplayPowerMgr::DisplayPowerMgrClient::GetInstance();

    powerMgrClient.OverrideScreenOffTime(time);
    powerMgrClient.SuspendDevice();
    // Wakeup Device before test
    powerMgrClient.WakeupDevice();
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient011: Prepare Fail, Screen is OFF.";
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient011::1!");
    usleep(time * TRANSFER_MS_TO_S * DIM_RATE);
    // waiting for GetDimState time
    usleep(DIM_STATE_WAIT_TIME_MS * TRANSFER_MS_TO_S);
    displayMgrClient.GetDisplayState();

    POWER_HILOGI(LABEL_TEST, "PowerMgrClient011::3!");
    usleep(time * TRANSFER_MS_TO_S * DIM_RATE + WAIT_SUSPEND_TIME_MS * TRANSFER_MS_TO_S);
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient011::8!");
    displayMgrClient.GetDisplayState();


    powerMgrClient.OverrideScreenOffTime(PowerStateMachine::DEFAULT_SLEEP_TIME_MS);
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient011 function end!");
}
#endif

/**
 * @tc.name: PowerMgrClient012
 * @tc.desc: test SCREEN_ON RunningLock
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient012, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient012 function start!");
    int32_t time = SET_DISPLAY_OFF_TIME_MS;
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.WakeupDevice();
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient0012: Prepare Fail, Screen is OFF.";
    auto runningLock = powerMgrClient.CreateRunningLock("runninglock", RunningLockType::RUNNINGLOCK_SCREEN);
    powerMgrClient.OverrideScreenOffTime(time);
    runningLock->Lock();
    usleep(SLEEP_AFTER_LOCK_TIME_US);
    EXPECT_EQ(runningLock->IsUsed(), true);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true);
    usleep(time * TRANSFER_MS_TO_S);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);
    runningLock->UnLock();
    EXPECT_EQ(runningLock->IsUsed(), false);
    powerMgrClient.OverrideScreenOffTime(PowerStateMachine::DEFAULT_SLEEP_TIME_MS);
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient012 function end!");
}

/**
 * @tc.name: PowerMgrClient013
 * @tc.desc: test SCREEN_ON RunningLock
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient013, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient013 function start!");

    int32_t time = SET_DISPLAY_OFF_TIME_MS;
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.WakeupDevice();
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient0013: Prepare Fail, Screen is OFF.";
    auto runningLock = powerMgrClient.CreateRunningLock("runninglock", RunningLockType::RUNNINGLOCK_SCREEN);
    powerMgrClient.OverrideScreenOffTime(time);

    runningLock->Lock();
    usleep(SLEEP_AFTER_LOCK_TIME_US);
    EXPECT_EQ(runningLock->IsUsed(), true);

    runningLock->UnLock();
    EXPECT_EQ(runningLock->IsUsed(), false);
    usleep(time * TRANSFER_MS_TO_S + WAIT_SUSPEND_TIME_MS * TRANSFER_MS_TO_S);

    powerMgrClient.OverrideScreenOffTime(PowerStateMachine::DEFAULT_SLEEP_TIME_MS);
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient013 function end!");
}

/**
 * @tc.name: PowerMgrClient014
 * @tc.desc: test SCREEN_ON RunningLock
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient014, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient014 function start!");

    int32_t time = SET_DISPLAY_OFF_TIME_MS;
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.WakeupDevice();
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient0014: Prepare Fail, Screen is OFF.";
    auto runningLock = powerMgrClient.CreateRunningLock("runninglock", RunningLockType::RUNNINGLOCK_SCREEN);
    powerMgrClient.OverrideScreenOffTime(time);

    runningLock->Lock();
    usleep(SLEEP_AFTER_LOCK_TIME_US);
    EXPECT_EQ(runningLock->IsUsed(), true);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true);
    usleep(time * TRANSFER_MS_TO_S * DOUBLE_TIMES);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);

    runningLock->UnLock();
    EXPECT_EQ(runningLock->IsUsed(), false);
    powerMgrClient.OverrideScreenOffTime(PowerStateMachine::DEFAULT_SLEEP_TIME_MS);
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient014 function end!");
}

/**
 * @tc.name: PowerMgrClient015
 * @tc.desc: test SCREEN_ON RunningLock
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient015, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient015 function start!");

    int32_t time = SET_DISPLAY_OFF_TIME_MS;
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.WakeupDevice();
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient0015: Prepare Fail, Screen is OFF.";
    auto runningLock = powerMgrClient.CreateRunningLock("runninglock", RunningLockType::RUNNINGLOCK_SCREEN);

    powerMgrClient.OverrideScreenOffTime(time);

    runningLock->Lock();
    usleep(SLEEP_AFTER_LOCK_TIME_US);
    EXPECT_EQ(runningLock->IsUsed(), true);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true);
    powerMgrClient.SuspendDevice();
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_OFF));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false);

    runningLock->UnLock();
    EXPECT_EQ(runningLock->IsUsed(), false);
    powerMgrClient.OverrideScreenOffTime(PowerStateMachine::DEFAULT_SLEEP_TIME_MS);
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient015 function end!");
}

/**
 * @tc.name: PowerMgrClient016
 * @tc.desc: test SCREEN_ON RunningLock
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient016, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient016 function start!");

    int32_t time = SET_DISPLAY_OFF_TIME_MS;
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.WakeupDevice();
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient0016: Prepare Fail, Screen is OFF.";
    auto runningLock = powerMgrClient.CreateRunningLock("runninglock", RunningLockType::RUNNINGLOCK_SCREEN);

    powerMgrClient.OverrideScreenOffTime(time);

    runningLock->Lock();
    usleep(SLEEP_AFTER_LOCK_TIME_US + SLEEP_AFTER_LOCK_TIME_US);
    EXPECT_EQ(runningLock->IsUsed(), true);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true);
    usleep(time * TRANSFER_MS_TO_S);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);

    runningLock->UnLock();
    EXPECT_EQ(runningLock->IsUsed(), false);
    powerMgrClient.OverrideScreenOffTime(PowerStateMachine::DEFAULT_SLEEP_TIME_MS);
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient016 function end!");
}

/**
 * @tc.name: PowerMgrClient017
 * @tc.desc: test SetDisplaySuspend
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient017, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient017 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    powerMgrClient.WakeupDevice();
    powerMgrClient.SuspendDevice();
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_OFF));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false);
    powerMgrClient.SetDisplaySuspend(true);

    EXPECT_EQ(powerMgrClient.IsScreenOn(), false);
    powerMgrClient.SetDisplaySuspend(false);
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient017 function end!");
}

/**
 * @tc.name: PowerMgrClient018
 * @tc.desc: test Suspend Device in proxy
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient018, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient018 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    powerMgrClient.SuspendDevice();
    // Wakeup Device before test
    powerMgrClient.WakeupDevice();
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient018: Prepare Fail, Screen is OFF.";

    powerMgrClient.SuspendDevice();
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_OFF));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerMgrClient018: Suspend Device Fail, Screen is On";

    POWER_HILOGI(LABEL_TEST, "PowerMgrClient018 function end!");
}

/**
 * @tc.name: PowerMgrClient019
 * @tc.desc: test Suspend Device
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient019, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient019 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    powerMgrClient.SuspendDevice();
    // Wakeup Device before test
    powerMgrClient.WakeupDevice();
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient019: Prepare Fail, Screen is OFF.";

    powerMgrClient.SuspendDevice(SuspendDeviceType::SUSPEND_DEVICE_REASON_DEVICE_ADMIN, false);
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_OFF));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerMgrClient019: Suspend Device Fail, Screen is On";

    POWER_HILOGI(LABEL_TEST, "PowerMgrClient019 function end!");
}

/**
 * @tc.name: PowerMgrClient020
 * @tc.desc: test Suspend Device
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient020, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient020 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    powerMgrClient.SuspendDevice();
    // Wakeup Device before test
    powerMgrClient.WakeupDevice();
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient020: Prepare Fail, Screen is OFF.";

    powerMgrClient.SuspendDevice(SuspendDeviceType::SUSPEND_DEVICE_REASON_TIMEOUT, false);
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_OFF));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerMgrClient020: Suspend Device Fail, Screen is On";

    POWER_HILOGI(LABEL_TEST, "PowerMgrClient020 function end!");
}

/**
 * @tc.name: PowerMgrClient021
 * @tc.desc: test Suspend Device
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient021, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient021 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    powerMgrClient.SuspendDevice();
    // Wakeup Device before test
    powerMgrClient.WakeupDevice();
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient021: Prepare Fail, Screen is OFF.";

    powerMgrClient.SuspendDevice(SuspendDeviceType::SUSPEND_DEVICE_REASON_LID, false);
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_OFF));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerMgrClient021: Suspend Device Fail, Screen is On";

    POWER_HILOGI(LABEL_TEST, "PowerMgrClient021 function end!");
}

/**
 * @tc.name: PowerMgrClient022
 * @tc.desc: test Suspend Device
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient022, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient022 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    powerMgrClient.SuspendDevice();
    // Wakeup Device before test
    powerMgrClient.WakeupDevice();
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient022: Prepare Fail, Screen is OFF.";
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_OFF));
    powerMgrClient.SuspendDevice(SuspendDeviceType::SUSPEND_DEVICE_REASON_POWER_KEY, false);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerMgrClient022: Suspend Device Fail, Screen is On";

    POWER_HILOGI(LABEL_TEST, "PowerMgrClient022 function end!");
}

/**
 * @tc.name: PowerMgrClient023
 * @tc.desc: test Suspend Device
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient023, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient023 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    powerMgrClient.SuspendDevice();
    // Wakeup Device before test
    powerMgrClient.WakeupDevice();
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient023: Prepare Fail, Screen is OFF.";
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_OFF));
    powerMgrClient.SuspendDevice(SuspendDeviceType::SUSPEND_DEVICE_REASON_HDMI, false);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerMgrClient023: Suspend Device Fail, Screen is On";

    POWER_HILOGI(LABEL_TEST, "PowerMgrClient023 function end!");
}

/**
 * @tc.name: PowerMgrClient024
 * @tc.desc: test Suspend Device
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient024, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient024 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    powerMgrClient.SuspendDevice();
    // Wakeup Device before test
    powerMgrClient.WakeupDevice();
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient024: Prepare Fail, Screen is OFF.";
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_OFF));
    powerMgrClient.SuspendDevice(SuspendDeviceType::SUSPEND_DEVICE_REASON_SLEEP_KEY, false);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerMgrClient024: Suspend Device Fail, Screen is On";

    POWER_HILOGI(LABEL_TEST, "PowerMgrClient024 function end!");
}

/**
 * @tc.name: PowerMgrClient025
 * @tc.desc: test Suspend Device
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient025, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient025 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    powerMgrClient.SuspendDevice();
    // Wakeup Device before test
    powerMgrClient.WakeupDevice();
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient025: Prepare Fail, Screen is OFF.";
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_OFF));
    powerMgrClient.SuspendDevice(SuspendDeviceType::SUSPEND_DEVICE_REASON_ACCESSIBILITY, false);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerMgrClient025: Suspend Device Fail, Screen is On";

    POWER_HILOGI(LABEL_TEST, "PowerMgrClient025 function end!");
}

/**
 * @tc.name: PowerMgrClient026
 * @tc.desc: test Suspend Device
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient026, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient026 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    powerMgrClient.SuspendDevice();
    // Wakeup Device before test
    powerMgrClient.WakeupDevice();
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient026: Prepare Fail, Screen is OFF.";
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_OFF));
    powerMgrClient.SuspendDevice(SuspendDeviceType::SUSPEND_DEVICE_REASON_FORCE_SUSPEND, false);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerMgrClient026: Suspend Device Fail, Screen is On";

    POWER_HILOGI(LABEL_TEST, "PowerMgrClient026 function end!");
}

/**
 * @tc.name: PowerMgrClient027
 * @tc.desc: test WakeupDevice(int64_t timeMs) in proxy
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient027, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient027 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    powerMgrClient.WakeupDevice();
    // Suspend Device before test
    powerMgrClient.SuspendDevice();
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_OFF));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerMgrClient027: Prepare Fail, Screen is On.";

    powerMgrClient.WakeupDevice();
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient027: Wakeup Device Fail, Screen is Off";

    POWER_HILOGI(LABEL_TEST, "PowerMgrClient027 function end!");
}

/**
 * @tc.name: PowerMgrClient028
 * @tc.desc: test WakeupDevice
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient028, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient028 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    powerMgrClient.WakeupDevice();
    // Suspend Device before test
    powerMgrClient.SuspendDevice();
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_OFF));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerMgrClient028: Prepare Fail, Screen is On.";

    powerMgrClient.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient028: Wakeup Device Fail, Screen is Off";

    POWER_HILOGI(LABEL_TEST, "PowerMgrClient028 function end!");
}

/**
 * @tc.name: PowerMgrClient029
 * @tc.desc: test WakeupDevice
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient029, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient029 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    powerMgrClient.WakeupDevice();
    // Suspend Device before test
    powerMgrClient.SuspendDevice();
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_OFF));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerMgrClient029: Prepare Fail, Screen is On.";

    powerMgrClient.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient029: Wakeup Device Fail, Screen is Off";

    POWER_HILOGI(LABEL_TEST, "PowerMgrClient029 function end!");
}

/**
 * @tc.name: PowerMgrClient030
 * @tc.desc: test WakeupDevice
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient030, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient030 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    powerMgrClient.WakeupDevice();
    // Suspend Device before test
    powerMgrClient.SuspendDevice();
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_OFF));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerMgrClient030: Prepare Fail, Screen is On.";

    powerMgrClient.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_PLUGGED_IN);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient030: Wakeup Device Fail, Screen is Off";

    POWER_HILOGI(LABEL_TEST, "PowerMgrClient030 function end!");
}

/**
 * @tc.name: PowerMgrClient031
 * @tc.desc: test WakeupDevice
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient031, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient031 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    powerMgrClient.WakeupDevice();
    // Suspend Device before test
    powerMgrClient.SuspendDevice();
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_OFF));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerMgrClient031: Prepare Fail, Screen is On.";

    powerMgrClient.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_GESTURE);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient031: Wakeup Device Fail, Screen is Off";

    POWER_HILOGI(LABEL_TEST, "PowerMgrClient031 function end!");
}

/**
 * @tc.name: PowerMgrClient032
 * @tc.desc: test WakeupDevice
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient032, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient032 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    powerMgrClient.WakeupDevice();
    // Suspend Device before test
    powerMgrClient.SuspendDevice();
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_OFF));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerMgrClient032: Prepare Fail, Screen is On.";

    powerMgrClient.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_CAMERA_LAUNCH);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient032: Wakeup Device Fail, Screen is Off";

    POWER_HILOGI(LABEL_TEST, "PowerMgrClient032 function end!");
}

/**
 * @tc.name: PowerMgrClient033
 * @tc.desc: test WakeupDevice
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient033, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient033 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    powerMgrClient.WakeupDevice();
    // Suspend Device before test
    powerMgrClient.SuspendDevice();
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_OFF));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerMgrClient033: Prepare Fail, Screen is On.";

    powerMgrClient.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_WAKE_KEY);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient033: Wakeup Device Fail, Screen is Off";

    POWER_HILOGI(LABEL_TEST, "PowerMgrClient033 function end!");
}

/**
 * @tc.name: PowerMgrClient034
 * @tc.desc: test WakeupDevice
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient034, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient034 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    powerMgrClient.WakeupDevice();
    // Suspend Device before test
    powerMgrClient.SuspendDevice();
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_OFF));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerMgrClient034: Prepare Fail, Screen is On.";

    powerMgrClient.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_WAKE_MOTION);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient034: Wakeup Device Fail, Screen is Off";

    POWER_HILOGI(LABEL_TEST, "PowerMgrClient034 function end!");
}

/**
 * @tc.name: PowerMgrClient035
 * @tc.desc: test WakeupDevice
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient035, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient035 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    powerMgrClient.WakeupDevice();
    // Suspend Device before test
    powerMgrClient.SuspendDevice();
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_OFF));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerMgrClient035: Prepare Fail, Screen is On.";

    powerMgrClient.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_HDMI);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient035: Wakeup Device Fail, Screen is Off";

    POWER_HILOGI(LABEL_TEST, "PowerMgrClient035 function end!");
}

/**
 * @tc.name: PowerMgrClient036
 * @tc.desc: test WakeupDevice
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient036, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient036 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    powerMgrClient.WakeupDevice();
    // Suspend Device before test
    powerMgrClient.SuspendDevice();
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_OFF));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerMgrClient036: Prepare Fail, Screen is On.";

    powerMgrClient.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_LID);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient036: Wakeup Device Fail, Screen is Off";

    POWER_HILOGI(LABEL_TEST, "PowerMgrClient036 function end!");
}

/**
 * @tc.name: PowerMgrClient037
 * @tc.desc: test WakeupDevice
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient037, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient037 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    powerMgrClient.WakeupDevice();
    // Suspend Device before test
    powerMgrClient.SuspendDevice();
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_OFF));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerMgrClient037: Prepare Fail, Screen is On.";

    powerMgrClient.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_DOUBLE_CLICK);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient037: Wakeup Device Fail, Screen is Off";

    POWER_HILOGI(LABEL_TEST, "PowerMgrClient037 function end!");
}

/**
 * @tc.name: PowerMgrClient038
 * @tc.desc: test WakeupDevice
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient038, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient038 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    powerMgrClient.WakeupDevice();
    // Suspend Device before test
    powerMgrClient.SuspendDevice();
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_OFF));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerMgrClient038: Prepare Fail, Screen is On.";

    powerMgrClient.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_KEYBOARD);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient038: Wakeup Device Fail, Screen is Off";

    POWER_HILOGI(LABEL_TEST, "PowerMgrClient038 function end!");
}

/**
 * @tc.name: PowerMgrClient039
 * @tc.desc: test WakeupDevice
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient039, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient039 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    powerMgrClient.WakeupDevice();
    // Suspend Device before test
    powerMgrClient.SuspendDevice();
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_OFF));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerMgrClient039: Prepare Fail, Screen is On.";

    powerMgrClient.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_MOUSE);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient039: Wakeup Device Fail, Screen is Off";

    POWER_HILOGI(LABEL_TEST, "PowerMgrClient039 function end!");
}

/**
 * @tc.name: PowerMgrClient040
 * @tc.desc: test IsRunningLockTypeSupported
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient040, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient040 function start!");

    auto& powerMgrClient = PowerMgrClient::GetInstance();
    auto ret = powerMgrClient.IsRunningLockTypeSupported(RunningLockType::RUNNINGLOCK_BUTT);
    EXPECT_EQ(ret, false);
    ret = powerMgrClient.IsRunningLockTypeSupported(RunningLockType::RUNNINGLOCK_SCREEN);
    EXPECT_EQ(ret, true);
    ret = powerMgrClient.IsRunningLockTypeSupported(RunningLockType::RUNNINGLOCK_BACKGROUND);
    EXPECT_EQ(ret, true);
#ifdef HAS_SENSORS_SENSOR_PART
    ret = powerMgrClient.IsRunningLockTypeSupported(RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL);
    EXPECT_EQ(ret, true);
#endif
    powerMgrClient.WakeupDevice();
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true);
    powerMgrClient.SuspendDevice();
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_OFF));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false);

    POWER_HILOGI(LABEL_TEST, "PowerMgrClient040 function end!");
}

/**
 * @tc.name: PowerMgrClient041
 * @tc.desc: Test GetError
 * @tc.type: FUNC
 * @tc.require: issue I5YZQR
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient041, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient041 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    PowerErrors error = powerMgrClient.GetError();
    POWER_HILOGI(LABEL_TEST, "get error %{public}d", static_cast<int32_t>(error));
    EXPECT_TRUE(error == PowerErrors::ERR_OK);
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient041 function end!");
}

/**
 * @tc.name: PowerMgrClient042
 * @tc.desc: test IsRunningLockTypeSupported for BACKGROUND lock types
 * @tc.type: FUNC
 * @tc.require: issueI6FMHX
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient042, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient042 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    auto ret = powerMgrClient.IsRunningLockTypeSupported(RunningLockType::RUNNINGLOCK_BACKGROUND_PHONE);
    EXPECT_EQ(ret, true);
    ret = powerMgrClient.IsRunningLockTypeSupported(RunningLockType::RUNNINGLOCK_BACKGROUND_NOTIFICATION);
    EXPECT_EQ(ret, true);
    ret = powerMgrClient.IsRunningLockTypeSupported(RunningLockType::RUNNINGLOCK_BACKGROUND_AUDIO);
    EXPECT_EQ(ret, true);
    ret = powerMgrClient.IsRunningLockTypeSupported(RunningLockType::RUNNINGLOCK_BACKGROUND_SPORT);
    EXPECT_EQ(ret, true);
    ret = powerMgrClient.IsRunningLockTypeSupported(RunningLockType::RUNNINGLOCK_BACKGROUND_NAVIGATION);
    EXPECT_EQ(ret, true);
    ret = powerMgrClient.IsRunningLockTypeSupported(RunningLockType::RUNNINGLOCK_BACKGROUND_TASK);
    EXPECT_EQ(ret, true);
    ret = powerMgrClient.IsRunningLockTypeSupported(RunningLockType::RUNNINGLOCK_BUTT);
    EXPECT_EQ(ret, false);
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient042 function end!");
}

/**
 * @tc.name: PowerMgrClient043
 * @tc.desc: test WakeupDevice
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient043, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient043 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    powerMgrClient.WakeupDevice();
    // Suspend Device before test
    powerMgrClient.SuspendDevice();
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_OFF));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerMgrClient043: Prepare Fail, Screen is On.";

    powerMgrClient.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_APPLICATION);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient043: Wakeup Device Fail, Screen is Off";

    POWER_HILOGI(LABEL_TEST, "PowerMgrClient043 function end!");
}

/**
 * @tc.name: PowerMgrClient044
 * @tc.desc: test WakeupDevice
 * @tc.type: FUNC
 * @tc.require: #I9G5XH
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient044, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient044 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    powerMgrClient.WakeupDevice();
    // Suspend Device before test
    powerMgrClient.SuspendDevice();
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_OFF));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerMgrClient044: Prepare Fail, Screen is On.";

    powerMgrClient.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_AOD_SLIDING);
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient044: Wakeup Device Fail, Screen is Off";

    POWER_HILOGI(LABEL_TEST, "PowerMgrClient044 function end!");
}

/**
 * @tc.name: PowerMgrClient045
 * @tc.desc: test SetIgnoreScreenOnLock
 * @tc.type: FUNC
 * @tc.require: issueI96FJ5
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient045, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient045 function start!");
    uint32_t PARM_ONE = 1;
    int32_t time = SET_DISPLAY_OFF_TIME_MS;
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    //the last testcase aka WAKEUP_DEVICE_AOD_SLIDING unlocks screen as well.
    //thus the screen off time will be restored thereafter.
    //in case the screen turns off cause of time-out, we don't want screen-lock to override screen-off-time
    auto ret = powerMgrClient.LockScreenAfterTimingOut(false, false);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    sleep(PARM_ONE); // wait for screen lock to restore the overrriden screen-off time after AOD_SLIDING.
    //everything should be fine now, no more interference from screen-lock
    powerMgrClient.WakeupDevice();
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient045: Prepare Fail, Screen is OFF.";
    auto runningLock = powerMgrClient.CreateRunningLock("runninglock", RunningLockType::RUNNINGLOCK_SCREEN);
    powerMgrClient.OverrideScreenOffTime(time);
    runningLock->Lock();
    usleep(time * TRANSFER_MS_TO_S + WAIT_SUSPEND_TIME_MS * TRANSFER_MS_TO_S);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true);
    ret = powerMgrClient.SetForceTimingOut(true);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    usleep(time * TRANSFER_MS_TO_S + WAIT_SUSPEND_TIME_MS * TRANSFER_MS_TO_S);
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_OFF));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false);
    powerMgrClient.SetForceTimingOut(false);
    powerMgrClient.WakeupDevice();
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true);
    usleep(time * TRANSFER_MS_TO_S + WAIT_SUSPEND_TIME_MS * TRANSFER_MS_TO_S);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true);
    ret = powerMgrClient.LockScreenAfterTimingOut(true, false); // reset
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient045 function end!");
}

/**
 * @tc.name: PowerMgrClient046
 * @tc.desc: test hibernate
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
#ifndef POWER_MANAGER_POWER_ENABLE_S4
HWTEST_F(PowerMgrClientTest, PowerMgrClient046, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient046 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    PowerErrors ret = powerMgrClient.Hibernate(true);
    EXPECT_NE(ret, PowerErrors::ERR_OK);
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient046 function end!");
}

/**
 * @tc.name: PowerMgrClient047
 * @tc.desc: test hibernate
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient047, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient047 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    PowerErrors ret = powerMgrClient.Hibernate(false);
    EXPECT_NE(ret, PowerErrors::ERR_OK);
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient047 function end!");
}
#endif

/**
 * @tc.name: PowerMgrClient048
 * @tc.desc: test for coverage
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient048, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient048 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    std::shared_ptr<RunningLock> testLock =
        powerMgrClient.CreateRunningLock("testLock", RunningLockType::RUNNINGLOCK_SCREEN);
    ASSERT_TRUE(!testLock->IsUsed());
    testLock->Lock();
    ASSERT_TRUE(testLock->IsUsed());
    pid_t curUid = getuid();
    pid_t curPid = getpid();
    EXPECT_TRUE(powerMgrClient.ProxyRunningLock(true, curPid, curUid));
    ASSERT_TRUE(!testLock->IsUsed());
    EXPECT_TRUE(powerMgrClient.ProxyRunningLock(false, curPid, curUid));
    ASSERT_TRUE(testLock->IsUsed());
    testLock->UnLock();
    testLock->Recover(nullptr);
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient048 function end!");
}

/**
 * @tc.name: PowerMgrClient049
 * @tc.desc: test WakeupDevice
 * @tc.type: FUNC
 * @tc.require: #I9O7I2
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient049, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient049 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    powerMgrClient.WakeupDevice();
    // Suspend Device before test
    powerMgrClient.SuspendDevice();
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_OFF));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerMgrClient049: Prepare Fail, Screen is On.";

    powerMgrClient.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_PEN);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient049: Wakeup Device Fail, Screen is Off";

    POWER_HILOGI(LABEL_TEST, "PowerMgrClient049 function end!");
}

/**
 * @tc.name: PowerMgrClient050
 * @tc.desc: test SetSuspendTag
 * @tc.type: FUNC
 * @tc.require: #I9TKSX
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient050, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient050 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    EXPECT_EQ(powerMgrClient.SetSuspendTag("ulsr"), PowerErrors::ERR_OK);
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient050 function end!");
}

/**
 * @tc.name: PowerMgrClient051
 * @tc.desc: test IsFoldScreenOn
 * @tc.type: FUNC
 * @tc.require: #I9UWD0
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient051, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient051 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    powerMgrClient.WakeupDevice();

    // Suspend Device before test
    powerMgrClient.SuspendDevice();

    powerMgrClient.WakeupDevice();
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_OFF));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false);
    powerMgrClient.IsFoldScreenOn();

    POWER_HILOGI(LABEL_TEST, "PowerMgrClient051 function end!");
}

/**
 * @tc.name: PowerMgrClient052
 * @tc.desc: test RegisterSyncHibernateCallback
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient052, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient052 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    bool ret = powerMgrClient.RegisterSyncHibernateCallback(nullptr);
    // parameter is nullptr
    EXPECT_FALSE(ret);
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient052 function end!");
}

/**
 * @tc.name: PowerMgrClient053
 * @tc.desc: test unRegisterSyncHibernateCallback
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient053, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient053 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    bool ret = powerMgrClient.UnRegisterSyncHibernateCallback(nullptr);
    EXPECT_FALSE(ret);
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient053 function end!");
}

/**
 * @tc.name: PowerMgrClient054
 * @tc.desc: test IsCollaborationScreenOn
 * @tc.type: FUNC
 * @tc.require: #IAN4ZA
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient054, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient054 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    powerMgrClient.WakeupDevice();

    // Suspend Device before test
    powerMgrClient.SuspendDevice();

    powerMgrClient.WakeupDevice();
    EXPECT_EQ(powerMgrClient.IsCollaborationScreenOn(), true) << "PowerMgrClient054: Screen is On";

    POWER_HILOGI(LABEL_TEST, "PowerMgrClient054 function end!");
}

/**
 * @tc.name: PowerMgrClient055
 * @tc.desc: test WakeupDevice
 * @tc.type: FUNC
 * @tc.require: #IAXR0O
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient055, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient055 function start!");
    uint32_t PARM_ONE = 1;
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    powerMgrClient.WakeupDevice();
    // Suspend Device before test
    powerMgrClient.SuspendDevice();
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_OFF));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerMgrClient055: Prepare Fail, Screen is On.";

    powerMgrClient.WakeupDeviceAsync(WakeupDeviceType::WAKEUP_DEVICE_PEN);
    sleep(PARM_ONE);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient055: Wakeup Device Async Fail, Screen is Off";

    POWER_HILOGI(LABEL_TEST, "PowerMgrClient055 function end!");
}

/**
 * @tc.name: PowerMgrClient056
 * @tc.desc: test IsRunningLockEnabled
 * @tc.type: FUNC
 * @tc.require: #IBADHF
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient056, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient056 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    auto runningLock = powerMgrClient.CreateRunningLock("runninglockScreen", RunningLockType::RUNNINGLOCK_SCREEN);
    runningLock->Lock();
    usleep(SLEEP_AFTER_LOCK_TIME_US);
    bool result = false;
    PowerErrors error = powerMgrClient.IsRunningLockEnabled(RunningLockType::RUNNINGLOCK_SCREEN, result);
    POWER_HILOGI(LABEL_TEST, "IsRunningLockEnabled error %{public}d", static_cast<int32_t>(error));
    EXPECT_TRUE(result) << "PowerMgrClient056: IsRunningLockEnabled Fail, result is false" ;
    runningLock->UnLock();
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient056 function end!");
}

/**
 * @tc.name: PowerMgrClient057
 * @tc.desc: test IsForceSleeping
 * @tc.type: FUNC
 * @tc.require: issueICE3O4
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient057, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient057 function start!");

    stub_->suspendController_ =
        std::make_shared<SuspendController>(stub_->shutdownController_, stub_->powerStateMachine_, stub_->ffrtTimer_);
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    int32_t wakeupReason = (static_cast<int32_t>(WakeupDeviceType::WAKEUP_DEVICE_MAX)) + 1;
    WakeupDeviceType abnormaltype = WakeupDeviceType(wakeupReason);
    powerMgrClient.WakeupDevice();
    sleep(NEXT_WAIT_TIME_S);
    EXPECT_EQ(powerMgrClient.IsForceSleeping(), false);
    powerMgrClient.SuspendDevice();
    sleep(NEXT_WAIT_TIME_S);
    EXPECT_EQ(powerMgrClient.IsForceSleeping(), false);

    powerMgrClient.WakeupDevice();
    sleep(NEXT_WAIT_TIME_S);
    EXPECT_EQ(powerMgrClient.IsForceSleeping(), false);
    powerMgrClient.ForceSuspendDevice();
    sleep(NEXT_WAIT_TIME_S);
#ifdef POWER_MANAGER_ENABLE_FORCE_SLEEP_BROADCAST
    EXPECT_EQ(powerMgrClient.IsForceSleeping(), true);
#else
    EXPECT_EQ(powerMgrClient.IsForceSleeping(), false);
#endif
    powerMgrClient.WakeupDevice(abnormaltype);
    sleep(NEXT_WAIT_TIME_S);
#ifdef POWER_MANAGER_ENABLE_FORCE_SLEEP_BROADCAST
    EXPECT_EQ(powerMgrClient.IsForceSleeping(), true);
#else
    EXPECT_EQ(powerMgrClient.IsForceSleeping(), false);
#endif
    powerMgrClient.WakeupDevice();
    sleep(NEXT_WAIT_TIME_S);
    EXPECT_EQ(powerMgrClient.IsForceSleeping(), false);

    // try to wait ffrt tasks to end
    auto ffrtTimer = stub_->ffrtTimer_;
    if (ffrtTimer) {
        ffrtTimer->Clear();
    }
    ffrt::wait();
    stub_->suspendController_ = nullptr;
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient057 function end!");
}

/**
 * @tc.name: PowerMgrClient058
 * @tc.desc: test RefreshActivity
 * @tc.type: FUNC
 * @tc.require: issueICPVG3
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient058, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient058 function start!");
    uint32_t PARM_TWO = 2;
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    powerMgrClient.SuspendDevice();
    // Wakeup Device before test
    powerMgrClient.WakeupDevice();
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient058: Prepare Fail, Screen is OFF.";
    powerMgrClient.OverrideScreenOffTime(SCREEN_OFF_WAIT_TIME_MS);
    usleep(SCREEN_OFF_WAIT_TIME_S * TRANSFER_NS_TO_MS / PARM_TWO);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient058: Prepare Fail, Screen is OFF.";
    powerMgrClient.RefreshActivity(UserActivityType::USER_ACTIVITY_TYPE_APPLICATION, "PowerMgrClient058");
    usleep(SCREEN_OFF_WAIT_TIME_S * TRANSFER_NS_TO_MS / PARM_TWO + WAIT_SUSPEND_TIME_MS * TRANSFER_MS_TO_S);
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_OFF));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerMgrClient058: RefreshActivity Device Fail, Screen is OFF";
    powerMgrClient.OverrideScreenOffTime(PowerStateMachine::DEFAULT_SLEEP_TIME_MS);
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient058 function end!");
}

class TestTakeOverSuspendCallback : public TakeOverSuspendCallbackStub {
    public:
        TestTakeOverSuspendCallback() = default;
        virtual ~TestTakeOverSuspendCallback() = default;

        bool OnTakeOverSuspend(SuspendDeviceType type) override
        {
            return false;
        }
};

#ifdef POWER_MANAGER_TAKEOVER_SUSPEND
/**
 * @tc.name: PowerMgrClient059
 * @tc.desc: test RegisterSuspendTakeoverCallback
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient059, TestSize.Level0) {
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient059 function start!");
    auto& powerMgrClinet = PowerMgrClient::GetInstance();
    sptr<TestTakeOverSuspendCallback> callback = nullptr;
    TakeOverSuspendPriority priority = TakeOverSuspendPriority::LOW;
    bool ret = powerMgrClinet.RegisterSuspendTakeoverCallback(callback, priority);
    EXPECT_FALSE(ret);
    callback = new TestTakeOverSuspendCallback();
    ret = powerMgrClinet.RegisterSuspendTakeoverCallback(callback, priority);
    EXPECT_TRUE(ret);
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient059 function end!");
}

/**
 * @tc.name: PowerMgrClient060
 * @tc.desc: test UnRegisterSuspendTakeoverCallback
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient060, TestSize.Level0) {
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient060 function start!");
    auto& powerMgrClinet = PowerMgrClient::GetInstance();
    sptr<TestTakeOverSuspendCallback> callback = nullptr;
    TakeOverSuspendPriority priority = TakeOverSuspendPriority::HIGH;
    bool ret = powerMgrClinet.UnRegisterSuspendTakeoverCallback(callback);
    EXPECT_FALSE(ret);
    callback = new TestTakeOverSuspendCallback();
    // first register callback
    ret = powerMgrClinet.RegisterSuspendTakeoverCallback(callback, priority);
    EXPECT_TRUE(ret);
    // then test unregister callback
    ret = powerMgrClinet.UnRegisterSuspendTakeoverCallback(callback);
    EXPECT_TRUE(ret);
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient060 function end!");
}
#else
/**
 * @tc.name: PowerMgrClient059
 * @tc.desc: test RegisterSuspendTakeoverCallback
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient059, TestSize.Level0) {
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient059 function start!");
    auto& powerMgrClinet = PowerMgrClient::GetInstance();
    sptr<TestTakeOverSuspendCallback> callback = nullptr;
    TakeOverSuspendPriority priority = TakeOverSuspendPriority::LOW;
    bool ret = powerMgrClinet.RegisterSuspendTakeoverCallback(callback, priority);
    EXPECT_FALSE(ret);
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient059 function end!");
}

/**
 * @tc.name: PowerMgrClient060
 * @tc.desc: test UnRegisterSuspendTakeoverCallback
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient060, TestSize.Level0) {
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient060 function start!");
    auto& powerMgrClinet = PowerMgrClient::GetInstance();
    sptr<TestTakeOverSuspendCallback> callback = nullptr;
    TakeOverSuspendPriority priority = TakeOverSuspendPriority::HIGH;
    bool ret = powerMgrClinet.UnRegisterSuspendTakeoverCallback(callback);
    EXPECT_FALSE(ret);
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient060 function end!");
}
#endif

class TestAsyncUlsrCallback : public AsyncUlsrCallbackStub {
public:
    TestAsyncUlsrCallback() = default;
    virtual ~TestAsyncUlsrCallback() = default;

    void OnAsyncWakeup() override
    {
        POWER_HILOGI(LABEL_TEST, "TestAsyncUlsrCallback OnAsyncWakeup!");
    }
};

/**
 * @tc.name: PowerMgrClient062
 * @tc.desc: test RegisterUlsrCallback && UnRegisterUlsrCallback
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrClientTest, AsyncUlsrCallbackTest001, TestSize.Level0) {
    POWER_HILOGI(LABEL_TEST, "AsyncUlsrCallbackTest001 function start!");
    auto& powerMgrClinet = PowerMgrClient::GetInstance();
    PowerErrors ret = PowerErrors::ERR_OK;

    sptr<TestAsyncUlsrCallback> callback0 = nullptr;
    ret = powerMgrClinet.RegisterUlsrCallback(callback0);
    EXPECT_EQ(ret, PowerErrors::ERR_PARAM_INVALID);

    ret = powerMgrClinet.UnRegisterUlsrCallback(callback0);
    EXPECT_EQ(ret, PowerErrors::ERR_PARAM_INVALID);

    sptr<TestAsyncUlsrCallback> callback1 = new TestAsyncUlsrCallback();
    ret = powerMgrClinet.RegisterUlsrCallback(callback1);
#ifdef POWER_MANAGER_ENABLE_SUSPEND_WITH_TAG
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
#else
    EXPECT_NE(ret, PowerErrors::ERR_OK);
#endif
    ret = powerMgrClinet.UnRegisterUlsrCallback(callback1);
#ifdef POWER_MANAGER_ENABLE_SUSPEND_WITH_TAG
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
#else
    EXPECT_NE(ret, PowerErrors::ERR_OK);
#endif
    POWER_HILOGI(LABEL_TEST, "AsyncUlsrCallbackTest001 function end!");
}


/**
 * @tc.name: PowerMgrClient061
 * @tc.desc: Test all PowerKeyFilteringStrategy enum values
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient061, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient061 start!");
    auto& client = PowerMgrClient::GetInstance();
    PowerErrors ret;

    // Test DISABLE_LONG_PRESS_FILTERING
    ret = client.SetPowerKeyFilteringStrategy(PowerKeyFilteringStrategy::DISABLE_LONG_PRESS_FILTERING);
    EXPECT_EQ(ret, PowerErrors::ERR_OK) << "DISABLE_LONG_PRESS_FILTERING failed";

    // Test LONG_PRESS_FILTERING_ONCE
    ret = client.SetPowerKeyFilteringStrategy(PowerKeyFilteringStrategy::LONG_PRESS_FILTERING_ONCE);
    EXPECT_EQ(ret, PowerErrors::ERR_OK) << "LONG_PRESS_FILTERING_ONCE failed";

    // Test invalid STRATEGY_MAX
    ret = client.SetPowerKeyFilteringStrategy(PowerKeyFilteringStrategy::STRATEGY_MAX);
    EXPECT_EQ(ret, PowerErrors::ERR_OK) << "STRATEGY_MAX should return error";

    POWER_HILOGI(LABEL_TEST, "PowerMgrClient061 end!");
}

class TestAsyncShutdownCallback : public AsyncShutdownCallbackStub {
public:
    TestAsyncShutdownCallback() = default;
    virtual ~TestAsyncShutdownCallback() = default;

    void OnAsyncShutdownOrReboot(bool isReboot) override
    {
        POWER_HILOGI(LABEL_TEST, "TestAsyncShutdownCallback OnAsyncShutdownOrReboot");
    }
};

/**
 * @tc.name: PowerMgrClient062
 * @tc.desc: test RegisterAsyncShutdownCallback && UnRegisterAsyncShutdownCallback
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient062, TestSize.Level0) {
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient062 function start!");
    auto& powerMgrClinet = PowerMgrClient::GetInstance();
    PowerErrors ret;

    sptr<IAsyncShutdownCallback> callback0 = nullptr;
    ret = powerMgrClinet.RegisterAsyncShutdownCallback(callback0, ShutdownPriority::DEFAULT);
    EXPECT_EQ(ret, PowerErrors::ERR_PARAM_INVALID);

    ret = powerMgrClinet.UnRegisterAsyncShutdownCallback(callback0);
    EXPECT_EQ(ret, PowerErrors::ERR_PARAM_INVALID);

    sptr<IAsyncShutdownCallback> callback1 = new TestAsyncShutdownCallback();
    ret = powerMgrClinet.RegisterAsyncShutdownCallback(callback1, ShutdownPriority::DEFAULT);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);

    ret = powerMgrClinet.UnRegisterAsyncShutdownCallback(callback1);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient062 function end!");
}

} // namespace