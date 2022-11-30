/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "display_power_mgr_client.h"
#include "running_lock_token_stub.h"
#include "power_common.h"
#include "power_mgr_client.h"
#include "power_mgr_service.h"
#include "power_state_machine.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

namespace {
/**
 * @tc.name: PowerMgrClient001
 * @tc.desc: test RefreshActivity
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient001, TestSize.Level2)
{
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient001::fun is start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    // Wakeup Device before test
    powerMgrClient.WakeupDevice();
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient001: Prepare Fail, Screen is OFF.";
    powerMgrClient.OverrideScreenOffTime(SCREEN_OFF_WAIT_TIME_MS);
    sleep(SCREEN_OFF_WAIT_TIME_S / 2);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient001: Prepare Fail, Screen is OFF.";
    powerMgrClient.RefreshActivity();
    sleep(SCREEN_OFF_WAIT_TIME_S / 2);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);

    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient001: RefreshActivity Device Fail, Screen is OFF";
    powerMgrClient.OverrideScreenOffTime(DEFAULT_SLEEP_TIME);
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient001::fun is end!");
}

/**
 * @tc.name: PowerMgrClient002
 * @tc.desc: test RefreshActivity
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient002, TestSize.Level2)
{
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient002::fun is start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    // Wakeup Device before test
    powerMgrClient.WakeupDevice();
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient002: Prepare Fail, Screen is OFF.";
    powerMgrClient.OverrideScreenOffTime(SCREEN_OFF_WAIT_TIME_MS);
    sleep(SCREEN_OFF_WAIT_TIME_S / 2);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient002: Prepare Fail, Screen is OFF.";
    powerMgrClient.RefreshActivity(UserActivityType::USER_ACTIVITY_TYPE_BUTTON);
    sleep(SCREEN_OFF_WAIT_TIME_S / 2);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);

    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient002: RefreshActivity Device Fail, Screen is OFF";
    powerMgrClient.OverrideScreenOffTime(DEFAULT_SLEEP_TIME);
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient002::fun is end!");
}

/**
 * @tc.name: PowerMgrClient003
 * @tc.desc: test RefreshActivity
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient003, TestSize.Level2)
{
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient003::fun is start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    // Wakeup Device before test
    powerMgrClient.WakeupDevice();
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient003: Prepare Fail, Screen is OFF.";
    powerMgrClient.OverrideScreenOffTime(SCREEN_OFF_WAIT_TIME_MS);
    sleep(SCREEN_OFF_WAIT_TIME_S / 2);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient003: Prepare Fail, Screen is OFF.";
    powerMgrClient.RefreshActivity(UserActivityType::USER_ACTIVITY_TYPE_TOUCH);
    sleep(SCREEN_OFF_WAIT_TIME_S / 2);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);

    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient003: RefreshActivity Device Fail, Screen is OFF";
    powerMgrClient.OverrideScreenOffTime(DEFAULT_SLEEP_TIME);
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient003::fun is end!");
}

/**
 * @tc.name: PowerMgrClient004
 * @tc.desc: test RefreshActivity
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient004, TestSize.Level2)
{
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient004::fun is start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    // Wakeup Device before test
    powerMgrClient.WakeupDevice();
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient004: Prepare Fail, Screen is OFF.";
    powerMgrClient.OverrideScreenOffTime(SCREEN_OFF_WAIT_TIME_MS);
    sleep(SCREEN_OFF_WAIT_TIME_S / 2);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient004: Prepare Fail, Screen is OFF.";
    powerMgrClient.RefreshActivity(UserActivityType::USER_ACTIVITY_TYPE_ACCESSIBILITY);
    sleep(SCREEN_OFF_WAIT_TIME_S / 2);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);

    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient004: RefreshActivity Device Fail, Screen is OFF";
    powerMgrClient.OverrideScreenOffTime(DEFAULT_SLEEP_TIME);
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient004::fun is end!");
}

/**
 * @tc.name: PowerMgrClient005
 * @tc.desc: test RefreshActivity
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient005, TestSize.Level2)
{
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient005::fun is start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    // Wakeup Device before test
    powerMgrClient.WakeupDevice();
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient005: Prepare Fail, Screen is OFF.";
    powerMgrClient.OverrideScreenOffTime(SCREEN_OFF_WAIT_TIME_MS);
    sleep(SCREEN_OFF_WAIT_TIME_S / 2);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient005: Prepare Fail, Screen is OFF.";
    powerMgrClient.RefreshActivity(UserActivityType::USER_ACTIVITY_TYPE_ATTENTION);
    sleep(SCREEN_OFF_WAIT_TIME_S / 2);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);

    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient005: RefreshActivity Device Fail, Screen is OFF";
    powerMgrClient.OverrideScreenOffTime(DEFAULT_SLEEP_TIME);
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient005::fun is end!");
}

/**
 * @tc.name: PowerMgrClient006
 * @tc.desc: test RefreshActivity
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient006, TestSize.Level2)
{
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient006::fun is start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    // Wakeup Device before test
    powerMgrClient.WakeupDevice();
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient006: Prepare Fail, Screen is OFF.";
    powerMgrClient.OverrideScreenOffTime(SCREEN_OFF_WAIT_TIME_MS);
    sleep(SCREEN_OFF_WAIT_TIME_S / 2);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient006: Prepare Fail, Screen is OFF.";
    powerMgrClient.RefreshActivity(UserActivityType::USER_ACTIVITY_TYPE_SOFTWARE);
    sleep(SCREEN_OFF_WAIT_TIME_S / 2);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);

    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient006: RefreshActivity Device Fail, Screen is OFF";
    powerMgrClient.OverrideScreenOffTime(DEFAULT_SLEEP_TIME);
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient006::fun is end!");
}

/**
 * @tc.name: PowerMgrClient007
 * @tc.desc: test RefreshActivity
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient007, TestSize.Level2)
{
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient007::fun is start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    UserActivityType abnormaltype = UserActivityType(9);

    // Wakeup Device before test
    powerMgrClient.WakeupDevice();
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient007: Prepare Fail, Screen is OFF.";
    powerMgrClient.OverrideScreenOffTime(SCREEN_OFF_WAIT_TIME_MS);
    usleep(SCREEN_OFF_WAIT_TIME_S * TRANSFER_NS_TO_MS / 2);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient007: Prepare Fail, Screen is OFF.";
    powerMgrClient.RefreshActivity(abnormaltype);
    usleep(SCREEN_OFF_WAIT_TIME_S * TRANSFER_NS_TO_MS / 2 + TRANSFER_NS_TO_MS);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::INACTIVE);

    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerMgrClient007: RefreshActivity Device Fail, Screen is OFF";
    powerMgrClient.OverrideScreenOffTime(DEFAULT_SLEEP_TIME);
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient007::fun is end!");
}

/**
 * @tc.name: PowerMgrClient008
 * @tc.desc: test DISPLAY_DIM
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient008, TestSize.Level2)
{
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient008::fun is start!");
    int64_t time = SCREEN_OFF_WAIT_TIME_MS;
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    auto& displayMgrClient = OHOS::DisplayPowerMgr::DisplayPowerMgrClient::GetInstance();

    powerMgrClient.OverrideScreenOffTime(time);
    // Wakeup Device before test
    powerMgrClient.WakeupDevice();
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient008: Prepare Fail, Screen is OFF.";
    usleep(time * TRANSFER_MS_TO_S * DIM_RATE);
    auto ret = displayMgrClient.GetDisplayState();
    EXPECT_EQ(ret, DisplayPowerMgr::DisplayState::DISPLAY_DIM);

    powerMgrClient.OverrideScreenOffTime(DEFAULT_SLEEP_TIME);
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient008::fun is end!");
}

/**
 * @tc.name: PowerMgrClient009
 * @tc.desc: test WakeupDevice
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient009, TestSize.Level0)
{
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient009::fun is start!");
    int32_t wakeupReason = (static_cast<int32_t>(WakeupDeviceType::WAKEUP_DEVICE_MAX)) + 1;
    WakeupDeviceType abnormaltype = WakeupDeviceType(wakeupReason);
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    // Suspend Device before test
    powerMgrClient.SuspendDevice();
    sleep(SLEEP_WAIT_TIME_S);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerMgrClient009: Prepare Fail, Screen is On.";
    powerMgrClient.WakeupDevice(abnormaltype);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::INACTIVE);

    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerMgrClient009: Wakeup Device Fail, Screen is Off";

    POWER_HILOGD(LABEL_TEST, "PowerMgrClient009::fun is end!");
}

/**
 * @tc.name: PowerMgrClient010
 * @tc.desc: test Suspend Device
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient010, TestSize.Level0)
{
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient010::fun is start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    int32_t suspendReason = (static_cast<int32_t>(SuspendDeviceType::SUSPEND_DEVICE_REASON_MAX)) + 1;
    SuspendDeviceType abnormaltype = SuspendDeviceType(suspendReason);

    // Wakeup Device before test
    powerMgrClient.WakeupDevice();
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient010: Prepare Fail, Screen is OFF.";
    powerMgrClient.SuspendDevice(abnormaltype, false);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);

    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient010: Suspend Device Fail, Screen is On";
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient010::fun is end!");
}

/**
 * @tc.name: PowerMgrClient011
 * @tc.desc: test  auto suspend
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient011, TestSize.Level2)
{
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient011::fun is start!");
    int32_t time = SLEEP_WAIT_TIME_MS;
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    auto& displayMgrClient = OHOS::DisplayPowerMgr::DisplayPowerMgrClient::GetInstance();

    powerMgrClient.OverrideScreenOffTime(time);
    // Wakeup Device before test
    powerMgrClient.WakeupDevice();
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient011: Prepare Fail, Screen is OFF.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient011::1!");
    usleep(time * TRANSFER_MS_TO_S * DIM_RATE);
    auto ret = displayMgrClient.GetDisplayState();
    EXPECT_EQ(ret, DisplayPowerMgr::DisplayState::DISPLAY_DIM);
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient011::3!");
    usleep(time * TRANSFER_MS_TO_S * DIM_RATE);
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient011::8!");
    ret = displayMgrClient.GetDisplayState();
    EXPECT_EQ(ret, DisplayPowerMgr::DisplayState::DISPLAY_OFF);

    powerMgrClient.OverrideScreenOffTime(DEFAULT_SLEEP_TIME);
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient011::fun is end!");
}

/**
 * @tc.name: PowerMgrClient012
 * @tc.desc: test SCREEN_ON RunningLock
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient012, TestSize.Level2)
{
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient012:Start.");
    int32_t time = SLEEP_WAIT_TIME_MS;
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    auto runningLock = powerMgrClient.CreateRunningLock("runninglock", RunningLockType::RUNNINGLOCK_SCREEN);
    powerMgrClient.OverrideScreenOffTime(time);
    runningLock->Lock();
    EXPECT_EQ(runningLock->IsUsed(), true);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true);
    usleep(time * TRANSFER_MS_TO_S);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);
    runningLock->UnLock();
    EXPECT_EQ(runningLock->IsUsed(), false);
    powerMgrClient.OverrideScreenOffTime(DEFAULT_SLEEP_TIME);
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient012:End.");
}

/**
 * @tc.name: PowerMgrClient013
 * @tc.desc: test SCREEN_ON RunningLock
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient013, TestSize.Level2)
{
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient013:Start.");

    int32_t time = SLEEP_WAIT_TIME_MS;
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    auto runningLock = powerMgrClient.CreateRunningLock("runninglock", RunningLockType::RUNNINGLOCK_SCREEN);
    powerMgrClient.OverrideScreenOffTime(time);

    runningLock->Lock();
    EXPECT_EQ(runningLock->IsUsed(), true);

    runningLock->UnLock();
    EXPECT_EQ(runningLock->IsUsed(), false);
    usleep(time * TRANSFER_MS_TO_S * DOUBLE_TIMES);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::INACTIVE);
    powerMgrClient.OverrideScreenOffTime(DEFAULT_SLEEP_TIME);
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient013:End.");
}

/**
 * @tc.name: PowerMgrClient014
 * @tc.desc: test SCREEN_ON RunningLock
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient014, TestSize.Level2)
{
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient014:Start.");

    int32_t time = SLEEP_WAIT_TIME_MS;
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    auto runningLock = powerMgrClient.CreateRunningLock("runninglock", RunningLockType::RUNNINGLOCK_SCREEN);
    powerMgrClient.OverrideScreenOffTime(time);

    runningLock->Lock();
    EXPECT_EQ(runningLock->IsUsed(), true);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true);
    usleep(time * TRANSFER_MS_TO_S * DOUBLE_TIMES);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);

    runningLock->UnLock();
    EXPECT_EQ(runningLock->IsUsed(), false);
    powerMgrClient.OverrideScreenOffTime(DEFAULT_SLEEP_TIME);
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient014:End.");
}

/**
 * @tc.name: PowerMgrClient015
 * @tc.desc: test SCREEN_ON RunningLock
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient015, TestSize.Level2)
{
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient015:Start.");

    int32_t time = SLEEP_WAIT_TIME_MS;
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    auto runningLock = powerMgrClient.CreateRunningLock("runninglock", RunningLockType::RUNNINGLOCK_SCREEN);

    powerMgrClient.OverrideScreenOffTime(time);

    runningLock->Lock();
    EXPECT_EQ(runningLock->IsUsed(), true);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true);
    powerMgrClient.SuspendDevice();
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::INACTIVE);

    runningLock->UnLock();
    EXPECT_EQ(runningLock->IsUsed(), false);
    powerMgrClient.OverrideScreenOffTime(DEFAULT_SLEEP_TIME);
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient015:End.");
}

/**
 * @tc.name: PowerMgrClient016
 * @tc.desc: test SCREEN_ON RunningLock
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient016, TestSize.Level2)
{
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient016:Start.");

    int32_t time = SLEEP_WAIT_TIME_MS;
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    auto runningLock = powerMgrClient.CreateRunningLock("runninglock", RunningLockType::RUNNINGLOCK_SCREEN);

    powerMgrClient.OverrideScreenOffTime(time);

    powerMgrClient.SuspendDevice();
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::INACTIVE);
    runningLock->Lock();
    EXPECT_EQ(runningLock->IsUsed(), true);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true);
    usleep(time * TRANSFER_MS_TO_S);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);

    runningLock->UnLock();
    EXPECT_EQ(runningLock->IsUsed(), false);
    powerMgrClient.OverrideScreenOffTime(DEFAULT_SLEEP_TIME);
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient016:End.");
}

/**
 * @tc.name: PowerMgrClient017
 * @tc.desc: test background RunningLock
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient017, TestSize.Level2)
{
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient017:Start.");

    int32_t time = SLEEP_WAIT_TIME_MS;
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    auto runningLock = powerMgrClient.CreateRunningLock("runninglock", RunningLockType::RUNNINGLOCK_BACKGROUND);

    powerMgrClient.OverrideScreenOffTime(time);

    powerMgrClient.SuspendDevice();
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::INACTIVE);
    runningLock->Lock();
    EXPECT_EQ(runningLock->IsUsed(), true);
    sleep(5);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::INACTIVE);

    runningLock->UnLock();
    EXPECT_EQ(runningLock->IsUsed(), false);
    powerMgrClient.OverrideScreenOffTime(DEFAULT_SLEEP_TIME);
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient017:End.");
}

/**
 * @tc.name: PowerMgrClient018
 * @tc.desc: test background RunningLock
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient018, TestSize.Level2)
{
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient018:Start.");

    auto& powerMgrClient = PowerMgrClient::GetInstance();
    auto runningLock = powerMgrClient.CreateRunningLock("runninglock", RunningLockType::RUNNINGLOCK_BACKGROUND);

    runningLock->Lock();
    EXPECT_EQ(runningLock->IsUsed(), true);
    powerMgrClient.WakeupDevice();
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);

    runningLock->UnLock();
    EXPECT_EQ(runningLock->IsUsed(), false);
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient018:End.");
}

/**
 * @tc.name: PowerMgrClient019
 * @tc.desc: test background RunningLock
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient019, TestSize.Level2)
{
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient019:Start.");

    int32_t time = SLEEP_WAIT_TIME_MS;
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    auto runningLock = powerMgrClient.CreateRunningLock("runninglock", RunningLockType::RUNNINGLOCK_BACKGROUND);

    powerMgrClient.OverrideScreenOffTime(time);

    powerMgrClient.WakeupDevice();
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);
    runningLock->Lock();
    EXPECT_EQ(runningLock->IsUsed(), true);
    usleep(time * TRANSFER_MS_TO_S * DOUBLE_TIMES);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::INACTIVE);
    runningLock->UnLock();
    EXPECT_EQ(runningLock->IsUsed(), false);
    powerMgrClient.OverrideScreenOffTime(DEFAULT_SLEEP_TIME);
    powerMgrClient.RestoreScreenOffTime();
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient019:End.");
}

/**
 * @tc.name: PowerMgrClient020
 * @tc.desc: test SetDisplaySuspend
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient020, TestSize.Level2)
{
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient020::fun is start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    powerMgrClient.SuspendDevice();
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false);
    powerMgrClient.SetDisplaySuspend(true);

    EXPECT_EQ(powerMgrClient.GetState(), PowerState::INACTIVE);

    EXPECT_EQ(powerMgrClient.IsScreenOn(), false);
    powerMgrClient.SetDisplaySuspend(false);
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient020::fun is end!");
}

/**
 * @tc.name: PowerMgrClient022
 * @tc.desc: test Suspend Device in proxy
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient022, TestSize.Level0)
{
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient022::fun is start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    // Wakeup Device before test
    powerMgrClient.WakeupDevice();
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient022: Prepare Fail, Screen is OFF.";

    powerMgrClient.SuspendDevice();
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::INACTIVE);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerMgrClient022: Suspend Device Fail, Screen is On";

    POWER_HILOGD(LABEL_TEST, "PowerMgrClient022::fun is end!");
}

/**
 * @tc.name: PowerMgrClient023
 * @tc.desc: test Suspend Device
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient023, TestSize.Level0)
{
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient023::fun is start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    // Wakeup Device before test
    powerMgrClient.WakeupDevice();
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient023: Prepare Fail, Screen is OFF.";

    powerMgrClient.SuspendDevice(SuspendDeviceType::SUSPEND_DEVICE_REASON_DEVICE_ADMIN, false);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::INACTIVE);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerMgrClient023: Suspend Device Fail, Screen is On";

    POWER_HILOGD(LABEL_TEST, "PowerMgrClient023::fun is end!");
}

/**
 * @tc.name: PowerMgrClient024
 * @tc.desc: test Suspend Device
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient024, TestSize.Level0)
{
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient024::fun is start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    // Wakeup Device before test
    powerMgrClient.WakeupDevice();
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient024: Prepare Fail, Screen is OFF.";

    powerMgrClient.SuspendDevice(SuspendDeviceType::SUSPEND_DEVICE_REASON_TIMEOUT, false);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::INACTIVE);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerMgrClient024: Suspend Device Fail, Screen is On";

    POWER_HILOGD(LABEL_TEST, "PowerMgrClient024::fun is end!");
}

/**
 * @tc.name: PowerMgrClient025
 * @tc.desc: test Suspend Device
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient025, TestSize.Level0)
{
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient025::fun is start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    // Wakeup Device before test
    powerMgrClient.WakeupDevice();
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient025: Prepare Fail, Screen is OFF.";

    powerMgrClient.SuspendDevice(SuspendDeviceType::SUSPEND_DEVICE_REASON_LID_SWITCH, false);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::INACTIVE);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerMgrClient025: Suspend Device Fail, Screen is On";

    POWER_HILOGD(LABEL_TEST, "PowerMgrClient025::fun is end!");
}

/**
 * @tc.name: PowerMgrClient026
 * @tc.desc: test Suspend Device
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient026, TestSize.Level0)
{
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient026::fun is start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    // Wakeup Device before test
    powerMgrClient.WakeupDevice();
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient026: Prepare Fail, Screen is OFF.";

    powerMgrClient.SuspendDevice(SuspendDeviceType::SUSPEND_DEVICE_REASON_POWER_BUTTON, false);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::INACTIVE);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerMgrClient026: Suspend Device Fail, Screen is On";

    POWER_HILOGD(LABEL_TEST, "PowerMgrClient026::fun is end!");
}

/**
 * @tc.name: PowerMgrClient027
 * @tc.desc: test Suspend Device
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient027, TestSize.Level0)
{
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient027::fun is start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    // Wakeup Device before test
    powerMgrClient.WakeupDevice();
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient027: Prepare Fail, Screen is OFF.";

    powerMgrClient.SuspendDevice(SuspendDeviceType::SUSPEND_DEVICE_REASON_HDMI, false);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::INACTIVE);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerMgrClient027: Suspend Device Fail, Screen is On";

    POWER_HILOGD(LABEL_TEST, "PowerMgrClient027::fun is end!");
}

/**
 * @tc.name: PowerMgrClient028
 * @tc.desc: test Suspend Device
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient028, TestSize.Level0)
{
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient028::fun is start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    // Wakeup Device before test
    powerMgrClient.WakeupDevice();
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient028: Prepare Fail, Screen is OFF.";

    powerMgrClient.SuspendDevice(SuspendDeviceType::SUSPEND_DEVICE_REASON_SLEEP_BUTTON, false);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::INACTIVE);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerMgrClient028: Suspend Device Fail, Screen is On";

    POWER_HILOGD(LABEL_TEST, "PowerMgrClient028::fun is end!");
}

/**
 * @tc.name: PowerMgrClient029
 * @tc.desc: test Suspend Device
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient029, TestSize.Level0)
{
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient029::fun is start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    // Wakeup Device before test
    powerMgrClient.WakeupDevice();
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient029: Prepare Fail, Screen is OFF.";

    powerMgrClient.SuspendDevice(SuspendDeviceType::SUSPEND_DEVICE_REASON_ACCESSIBILITY, false);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::INACTIVE);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerMgrClient029: Suspend Device Fail, Screen is On";

    POWER_HILOGD(LABEL_TEST, "PowerMgrClient029::fun is end!");
}

/**
 * @tc.name: PowerMgrClient030
 * @tc.desc: test Suspend Device
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient030, TestSize.Level0)
{
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient030::fun is start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    // Wakeup Device before test
    powerMgrClient.WakeupDevice();
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient030: Prepare Fail, Screen is OFF.";

    powerMgrClient.SuspendDevice(SuspendDeviceType::SUSPEND_DEVICE_REASON_FORCE_SUSPEND, false);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::INACTIVE);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerMgrClient030: Suspend Device Fail, Screen is On";

    POWER_HILOGD(LABEL_TEST, "PowerMgrClient030::fun is end!");
}

/**
 * @tc.name: PowerMgrClient031
 * @tc.desc: test WakeupDevice(int64_t timeMs) in proxy
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient031, TestSize.Level0)
{
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient031::fun is start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    // Suspend Device before test
    powerMgrClient.SuspendDevice();
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerMgrClient031: Prepare Fail, Screen is On.";

    powerMgrClient.WakeupDevice();
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient031: Wakeup Device Fail, Screen is Off";

    POWER_HILOGD(LABEL_TEST, "PowerMgrClient031::fun is end!");
}

/**
 * @tc.name: PowerMgrClient032
 * @tc.desc: test WakeupDevice
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient032, TestSize.Level0)
{
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient032::fun is start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    // Suspend Device before test
    powerMgrClient.SuspendDevice();
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerMgrClient032: Prepare Fail, Screen is On.";

    powerMgrClient.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient032: Wakeup Device Fail, Screen is Off";

    POWER_HILOGD(LABEL_TEST, "PowerMgrClient032::fun is end!");
}

/**
 * @tc.name: PowerMgrClient033
 * @tc.desc: test WakeupDevice
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient033, TestSize.Level0)
{
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient033::fun is start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    // Suspend Device before test
    powerMgrClient.SuspendDevice();
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerMgrClient033: Prepare Fail, Screen is On.";

    powerMgrClient.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient033: Wakeup Device Fail, Screen is Off";

    POWER_HILOGD(LABEL_TEST, "PowerMgrClient033::fun is end!");
}

/**
 * @tc.name: PowerMgrClient034
 * @tc.desc: test WakeupDevice
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient034, TestSize.Level0)
{
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient034::fun is start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    // Suspend Device before test
    powerMgrClient.SuspendDevice();
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerMgrClient034: Prepare Fail, Screen is On.";

    powerMgrClient.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_PLUGGED_IN);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient034: Wakeup Device Fail, Screen is Off";

    POWER_HILOGD(LABEL_TEST, "PowerMgrClient034::fun is end!");
}

/**
 * @tc.name: PowerMgrClient035
 * @tc.desc: test WakeupDevice
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient035, TestSize.Level0)
{
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient035::fun is start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    // Suspend Device before test
    powerMgrClient.SuspendDevice();
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerMgrClient035: Prepare Fail, Screen is On.";

    powerMgrClient.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_GESTURE);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient035: Wakeup Device Fail, Screen is Off";

    POWER_HILOGD(LABEL_TEST, "PowerMgrClient035::fun is end!");
}

/**
 * @tc.name: PowerMgrClient036
 * @tc.desc: test WakeupDevice
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient036, TestSize.Level0)
{
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient036::fun is start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    // Suspend Device before test
    powerMgrClient.SuspendDevice();
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerMgrClient036: Prepare Fail, Screen is On.";

    powerMgrClient.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_CAMERA_LAUNCH);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient036: Wakeup Device Fail, Screen is Off";

    POWER_HILOGD(LABEL_TEST, "PowerMgrClient036::fun is end!");
}

/**
 * @tc.name: PowerMgrClient037
 * @tc.desc: test WakeupDevice
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient037, TestSize.Level0)
{
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient037::fun is start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    // Suspend Device before test
    powerMgrClient.SuspendDevice();
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerMgrClient037: Prepare Fail, Screen is On.";

    powerMgrClient.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_WAKE_KEY);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient037: Wakeup Device Fail, Screen is Off";

    POWER_HILOGD(LABEL_TEST, "PowerMgrClient037::fun is end!");
}

/**
 * @tc.name: PowerMgrClient038
 * @tc.desc: test WakeupDevice
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient038, TestSize.Level0)
{
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient038::fun is start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    // Suspend Device before test
    powerMgrClient.SuspendDevice();
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerMgrClient038: Prepare Fail, Screen is On.";

    powerMgrClient.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_WAKE_MOTION);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient038: Wakeup Device Fail, Screen is Off";

    POWER_HILOGD(LABEL_TEST, "PowerMgrClient038::fun is end!");
}

/**
 * @tc.name: PowerMgrClient039
 * @tc.desc: test WakeupDevice
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient039, TestSize.Level0)
{
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient039::fun is start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    // Suspend Device before test
    powerMgrClient.SuspendDevice();
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerMgrClient039: Prepare Fail, Screen is On.";

    powerMgrClient.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_HDMI);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient039: Wakeup Device Fail, Screen is Off";

    POWER_HILOGD(LABEL_TEST, "PowerMgrClient039::fun is end!");
}

/**
 * @tc.name: PowerMgrClient040
 * @tc.desc: test WakeupDevice
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient040, TestSize.Level0)
{
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient040::fun is start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    // Suspend Device before test
    powerMgrClient.SuspendDevice();
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerMgrClient040: Prepare Fail, Screen is On.";

    powerMgrClient.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_LID);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient040: Wakeup Device Fail, Screen is Off";

    POWER_HILOGD(LABEL_TEST, "PowerMgrClient040::fun is end!");
}

/**
 * @tc.name: PowerMgrClient041
 * @tc.desc: test WakeupDevice
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient041, TestSize.Level0)
{
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient041::fun is start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    // Suspend Device before test
    powerMgrClient.SuspendDevice();
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerMgrClient041: Prepare Fail, Screen is On.";

    powerMgrClient.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_DOUBLE_CLICK);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient041: Wakeup Device Fail, Screen is Off";

    POWER_HILOGD(LABEL_TEST, "PowerMgrClient041::fun is end!");
}

/**
 * @tc.name: PowerMgrClient042
 * @tc.desc: test WakeupDevice
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient042, TestSize.Level0)
{
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient042::fun is start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    // Suspend Device before test
    powerMgrClient.SuspendDevice();
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerMgrClient042: Prepare Fail, Screen is On.";

    powerMgrClient.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_KEYBOARD);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient042: Wakeup Device Fail, Screen is Off";

    POWER_HILOGD(LABEL_TEST, "PowerMgrClient042::fun is end!");
}

/**
 * @tc.name: PowerMgrClient043
 * @tc.desc: test WakeupDevice
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient043, TestSize.Level0)
{
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient043::fun is start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    // Suspend Device before test
    powerMgrClient.SuspendDevice();
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerMgrClient043: Prepare Fail, Screen is On.";

    powerMgrClient.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_MOUSE);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrClient043: Wakeup Device Fail, Screen is Off";

    POWER_HILOGD(LABEL_TEST, "PowerMgrClient043::fun is end!");
}

/**
 * @tc.name: PowerMgrClient021
 * @tc.desc: test IsRunningLockTypeSupported
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient021, TestSize.Level2)
{
    POWER_HILOGD(LABEL_TEST, "PowerMgrClient021:Start.");

    auto& powerMgrClient = PowerMgrClient::GetInstance();

    auto type = static_cast<uint32_t>(RunningLockType::RUNNINGLOCK_BUTT);
    auto ret = powerMgrClient.IsRunningLockTypeSupported(type);
    EXPECT_EQ(ret, false);
    type = static_cast<uint32_t>(RunningLockType::RUNNINGLOCK_SCREEN);
    ret = powerMgrClient.IsRunningLockTypeSupported(type);
    EXPECT_EQ(ret, true);
    type = static_cast<uint32_t>(RunningLockType::RUNNINGLOCK_BACKGROUND);
    ret = powerMgrClient.IsRunningLockTypeSupported(type);
    EXPECT_EQ(ret, true);
    type = static_cast<uint32_t>(RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL);
    ret = powerMgrClient.IsRunningLockTypeSupported(type);
    EXPECT_EQ(ret, true);
    powerMgrClient.WakeupDevice();
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true);
    powerMgrClient.SuspendDevice();
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false);

    POWER_HILOGD(LABEL_TEST, "PowerMgrClient021:End.");
}

/**
 * @tc.name: PowerMgrClient023
 * @tc.desc: Test GetError
 * @tc.type: FUNC
 * @tc.require: issue I63PN9
 */
HWTEST_F(PowerMgrClientTest, PowerMgrClient044, TestSize.Level2)
{
    POWER_HILOGD(LABEL_TEST, "fun is start");
    PowerErrors error = PowerMgrClient::GetInstance().GetError();
    POWER_HILOGD(LABEL_TEST, "get error %{public}d", static_cast<int32_t>(error));
    POWER_HILOGD(error == PowerErrors::ERR_OK);
}
}