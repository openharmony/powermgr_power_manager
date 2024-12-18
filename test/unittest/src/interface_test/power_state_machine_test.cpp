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

#include "power_state_machine_test.h"

#include <iostream>

#include <datetime_ex.h>
#include <gtest/gtest.h>
#include <if_system_ability_manager.h>
#include <ipc_skeleton.h>
#include <string_ex.h>

#include "power_common.h"
#include "power_mgr_client.h"
#include "power_mgr_service.h"
#include "power_state_machine.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

void PowerStateMachineTest::SetUpTestCase(void)
{
}

void PowerStateMachineTest::TearDownTestCase(void)
{
}

void PowerStateMachineTest::SetUp(void)
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.OverrideScreenOffTime(SCREEN_OFF_WAIT_TIME_MS);
}

void PowerStateMachineTest::TearDown(void)
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.RestoreScreenOffTime();
}

namespace {
/**
 * @tc.name: PowerStateMachine001
 * @tc.desc: test Suspend Device in proxy
 * @tc.type: FUNC
 */
HWTEST_F (PowerStateMachineTest, PowerStateMachine001, TestSize.Level0)
{
    POWER_HILOGD(LABEL_TEST, "PowerStateMachine001::fun is start!");
    sleep(SLEEP_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerStateMachine001: Suspend Device start.";
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    // Wakeup Device before test
    GTEST_LOG_(INFO) << "PowerStateMachine001: Wakeup Device before test.";
    powerMgrClient.WakeupDevice();
    usleep(SLEEP_WAIT_TIME_MS);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerStateMachine001: Prepare Fail, Screen is OFF.";
    GTEST_LOG_(INFO) << "PowerStateMachine001: Screen is On, Begin to Suspend Device!";

    powerMgrClient.SuspendDevice();

    sleep(REFRESHACTIVITY_WAIT_TIME_S);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerStateMachine001: Suspend Device Fail, Screen is On";

    POWER_HILOGD(LABEL_TEST, "PowerStateMachine001::fun is end!");
    GTEST_LOG_(INFO) << "PowerStateMachine001: Suspend Device end.";
}

/**
 * @tc.name: PowerStateMachine002
 * @tc.desc: test WakeupDevice(int64_t timeMs) in proxy
 * @tc.type: FUNC
 */
HWTEST_F (PowerStateMachineTest, PowerStateMachine002, TestSize.Level0)
{
    POWER_HILOGD(LABEL_TEST, "PowerStateMachine002::fun is start!");
    sleep(SLEEP_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerStateMachine002: Wakeup Device start.";
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    // Suspend Device before test
    GTEST_LOG_(INFO) << "PowerStateMachine002: Suspend Device before test.";
    powerMgrClient.SuspendDevice();
    usleep(SLEEP_WAIT_TIME_MS);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerStateMachine002: Prepare Fail, Screen is On.";
    GTEST_LOG_(INFO) << "PowerStateMachine002: Screen is Off, Begin to Wakeup Device!";

    powerMgrClient.WakeupDevice();

    usleep(SLEEP_WAIT_TIME_MS);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerStateMachine002: Wakeup Device Fail, Screen is Off";

    POWER_HILOGD(LABEL_TEST, "PowerStateMachine002::fun is end!");
    GTEST_LOG_(INFO) << "PowerStateMachine002: Wakeup Device end.";
}

/**
 * @tc.name: PowerStateMachine003
 * @tc.desc: test IsScreenOn in proxy
 * @tc.type: FUNC
 */
HWTEST_F (PowerStateMachineTest, PowerStateMachine003, TestSize.Level0)
{
    sleep(SLEEP_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerStateMachine003: IsScreenOn start.";
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    for (int i = 0; i < 3; i++) {
        if (powerMgrClient.IsScreenOn()) {
            powerMgrClient.SuspendDevice();
            usleep(SLEEP_WAIT_TIME_MS);
            EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerStateMachine003" << i
                << ": Suspend Device Fail, Screen is On";
        } else {
            powerMgrClient.WakeupDevice();
            usleep(SLEEP_WAIT_TIME_MS);
            EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerStateMachine003" << i
                << ": Wakeup Device Fail, Screen is Off";
        }
    }

    GTEST_LOG_(INFO) << "PowerStateMachine003: IsScreenOn end.";
}

/**
 * @tc.name: PowerStateMachine004
 * @tc.desc: test WakeupDevice(int64_t timeMs, const WakeupDeviceType reason, const std::string& details) in proxy
 * @tc.type: FUNC
 */
HWTEST_F (PowerStateMachineTest, PowerStateMachine004, TestSize.Level0)
{
    sleep(SLEEP_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerStateMachine004: Wakeup Device start.";
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    // Suspend Device before test
    GTEST_LOG_(INFO) << "PowerStateMachine004: Suspend Device before test.";
    powerMgrClient.SuspendDevice();
    usleep(SLEEP_WAIT_TIME_MS);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerStateMachine004: Prepare Fail, Screen is On.";
    GTEST_LOG_(INFO) << "PowerStateMachine004: Screen is Off, Begin to Wakeup Device!";

    // Check illegal para details
    GTEST_LOG_(INFO) << "PowerStateMachine004: Check illegal para details Begin!";
    powerMgrClient.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_APPLICATION);
    usleep(SLEEP_WAIT_TIME_MS);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerStateMachine004: Check details test Fail, Screen is Off.";

    // Suspend Device before test
    GTEST_LOG_(INFO) << "PowerStateMachine004: Suspend Device before real test.";
    powerMgrClient.SuspendDevice();
    usleep(SLEEP_WAIT_TIME_MS);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerStateMachine004: Real test tprepare Fail, Screen is On.";
    GTEST_LOG_(INFO) << "PowerStateMachine004: Screen is Off, Begin to Real Wakeup Device!";

    powerMgrClient.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_APPLICATION);

    usleep(SLEEP_WAIT_TIME_MS);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerStateMachine004: Real Wakeup Device Fail, Screen is Off";

    GTEST_LOG_(INFO) << "PowerStateMachine004: Wakeup Device end.";
}

/**
 * @tc.name: PowerStateMachine005
 * @tc.desc: test Suspend Device in proxy
 * @tc.type: FUNC
 * @tc.require: issueI6MZ3M
 */
HWTEST_F (PowerStateMachineTest, PowerStateMachine005, TestSize.Level0)
{
    POWER_HILOGD(LABEL_TEST, "PowerStateMachine005::fun is start!");
    sleep(SLEEP_WAIT_TIME_S);
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    EXPECT_EQ(powerMgrClient.RestoreScreenOffTime(), PowerErrors::ERR_OK);
    POWER_HILOGD(LABEL_TEST, "PowerStateMachine005::fun is end!");
}
}

void PowerStateMachineTest::PowerStateTest1Callback::OnPowerStateChanged(PowerState state)
{
    POWER_HILOGD(
        LABEL_TEST, "PowerStateTest1Callback::OnPowerStateChanged state = %{public}u.", static_cast<uint32_t>(state));
}

void PowerStateMachineTest::PowerStateTest1Callback::OnAsyncPowerStateChanged(PowerState state)
{
    POWER_HILOGD(LABEL_TEST, "PowerStateTest1Callback::OnAsyncPowerStateChanged state = %{public}u.",
        static_cast<uint32_t>(state));
}

void PowerStateMachineTest::PowerStateTest2Callback::OnPowerStateChanged(PowerState state)
{
    POWER_HILOGD(
        LABEL_TEST, "PowerStateTest2Callback::OnPowerStateChanged state = %{public}u.", static_cast<uint32_t>(state));
}

void PowerStateMachineTest::PowerStateTest2Callback::OnAsyncPowerStateChanged(PowerState state)
{
    POWER_HILOGD(LABEL_TEST, "PowerStateTest2Callback::OnAsyncPowerStateChanged state = %{public}u.",
        static_cast<uint32_t>(state));
}

namespace {
/**
 * @tc.name: PowerStateCallback001
 * @tc.desc: test PowerStateCallback
 * @tc.type: FUNC
 */
HWTEST_F (PowerStateMachineTest, PowerStateCallback001, TestSize.Level0)
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    sptr<IPowerStateCallback> cb1 = new PowerStateTest1Callback();
    powerMgrClient.RegisterPowerStateCallback(cb1);
    POWER_HILOGD(LABEL_TEST, "PowerStateCallback001 1.");
    {
        sptr<IPowerStateCallback> cb2 = new PowerStateTest2Callback();
        powerMgrClient.UnRegisterPowerStateCallback(cb2);
        POWER_HILOGD(LABEL_TEST, "PowerStateCallback001 2.");
        powerMgrClient.RegisterPowerStateCallback(cb2);
        POWER_HILOGD(LABEL_TEST, "PowerStateCallback001 3.");
        powerMgrClient.RegisterPowerStateCallback(cb2);
        POWER_HILOGD(LABEL_TEST, "PowerStateCallback001 4.");
    }
    EXPECT_TRUE(powerMgrClient.UnRegisterPowerStateCallback(cb1));
    POWER_HILOGD(LABEL_TEST, "PowerStateTestCallback::PowerStateCallback001 end.");
}

/**
 * @tc.name: PowerStateCallback002
 * @tc.desc: test PowerStateCallback
 * @tc.type: FUNC
 */
HWTEST_F (PowerStateMachineTest, PowerStateCallback002, TestSize.Level0)
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    sptr<IPowerStateCallback> cb1 = new PowerStateTest1Callback();
    powerMgrClient.RegisterPowerStateCallback(cb1, false);
    POWER_HILOGD(LABEL_TEST, "PowerStateCallback002 1.");
    {
        sptr<IPowerStateCallback> cb2 = new PowerStateTest2Callback();
        powerMgrClient.UnRegisterPowerStateCallback(cb2);
        POWER_HILOGD(LABEL_TEST, "PowerStateCallback002 2.");
        powerMgrClient.RegisterPowerStateCallback(cb2, false);
        POWER_HILOGD(LABEL_TEST, "PowerStateCallback002 3.");
        powerMgrClient.RegisterPowerStateCallback(cb2, false);
        POWER_HILOGD(LABEL_TEST, "PowerStateCallback002 4.");
    }
    EXPECT_TRUE(powerMgrClient.UnRegisterPowerStateCallback(cb1));
    POWER_HILOGD(LABEL_TEST, "PowerStateTestCallback::PowerStateCallback002 end.");
}
}

void PowerStateMachineTest::WakeUpthread()
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.WakeupDevice();
}

void PowerStateMachineTest::Suspendthread()
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.SuspendDevice();
}

void PowerStateMachineTest::Rebootthread()
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    if (false) {
        powerMgrClient.RebootDevice(string("RebootDeviceTestThread"));
    }
}

void PowerStateMachineTest::Shutdownthread()
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    if (false) {
        powerMgrClient.ShutDownDevice(string("ShutDownDeviceTestThread"));
    }
}
