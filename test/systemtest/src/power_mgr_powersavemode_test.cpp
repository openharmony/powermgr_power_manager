/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include <bundle_mgr_proxy.h>
#include <datetime_ex.h>
#include <gtest/gtest.h>
#include <if_system_ability_manager.h>
#include <iostream>
#include <ipc_skeleton.h>
#include <string_ex.h>

#include "common_event_manager.h"
#include "ipower_mode_callback.h"
#include "power_common.h"
#include "power_mgr_client.h"
#include "power_mgr_powersavemode_test.h"
#include "power_mgr_service.h"
#include "power_state_machine.h"
#include "power_state_machine_info.h"
#include "running_lock.h"
#include "running_lock_info.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS::EventFwk;
using namespace OHOS;
using namespace std;

void PowerMgrPowerSavemodeTest::PowerModeTest1Callback::OnPowerModeChanged(PowerMode mode)
{
    POWER_HILOGD(LABEL_TEST, "PowerModeTest1Callback::OnPowerModeChanged.");
}

void PowerMgrPowerSavemodeTest::SetUpTestCase(void)
{
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    pms->OnStart();
    SystemAbility::MakeAndRegisterAbility(pms.GetRefPtr());
}

void PowerMgrPowerSavemodeTest::TearDownTestCase(void)
{
}

void PowerMgrPowerSavemodeTest::SetUp(void)
{
}

void PowerMgrPowerSavemodeTest::TearDown(void)
{
}

namespace {
/**
 * @tc.name: PowerSavemode_001
 * @tc.desc: test SetDeviceMode in proxy
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrPowerSavemodeTest, PowerSavemode_001, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerSavemode_001: SetDeviceMode start.";

    auto& powerMgrClient = PowerMgrClient::GetInstance();
    PowerMode mode = PowerMode::POWER_SAVE_MODE;
    PowerMode mode1 = PowerMode::POWER_SAVE_MODE;
    powerMgrClient.SetDeviceMode(mode);
    mode = powerMgrClient.GetDeviceMode();
    EXPECT_EQ(mode, mode1) << "PowerSavemode_001 fail to SetDeviceMode";
    GTEST_LOG_(INFO) << "PowerSavemode_001: SetDeviceMode end.";
}

/**
 * @tc.name: PowerSavemode_002
 * @tc.desc: test SetDeviceMode in proxy
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrPowerSavemodeTest, PowerSavemode_002, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerSavemode_002: SetDeviceMode start.";

    auto& powerMgrClient = PowerMgrClient::GetInstance();
    PowerMode mode = PowerMode::PERFORMANCE_MODE;
    PowerMode mode1 = PowerMode::PERFORMANCE_MODE;
    powerMgrClient.SetDeviceMode(mode);
    mode = powerMgrClient.GetDeviceMode();
    EXPECT_EQ(mode, mode1) << "PowerSavemode_002 fail to SetDeviceMode";
}

/**
 * @tc.name: PowerSavemode_003
 * @tc.desc: test SetDeviceMode in proxy
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrPowerSavemodeTest, PowerSavemode_003, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerSavemode_003: SetDeviceMode start.";

    auto& powerMgrClient = PowerMgrClient::GetInstance();
    PowerMode mode = PowerMode::EXTREME_POWER_SAVE_MODE;
    PowerMode mode1 = PowerMode::EXTREME_POWER_SAVE_MODE;
    powerMgrClient.SetDeviceMode(mode);
    mode = powerMgrClient.GetDeviceMode();
    EXPECT_EQ(mode, mode1) << "PowerSavemode_003 fail to SetDeviceMode";
    GTEST_LOG_(INFO) << "PowerSavemode_003: SetDeviceMode end.";
}

/**
 * @tc.name: PowerSavemode_028
 * @tc.desc: test SetDeviceMode in proxy
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrPowerSavemodeTest, PowerSavemode_028, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerSavemode_028: SetDeviceMode start.";

    auto& powerMgrClient = PowerMgrClient::GetInstance();
    PowerMode mode = static_cast<PowerMode>(1);
    PowerMode mode1 = static_cast<PowerMode>(1);
    powerMgrClient.SetDeviceMode(mode);
    mode = powerMgrClient.GetDeviceMode();
    EXPECT_NE(mode, mode1) << "PowerSavemode_028 fail to SetDeviceMode abnormal";
    GTEST_LOG_(INFO) << "PowerSavemode_028: SetDeviceMode end.";
}

/**
 * @tc.name: PowerSavemode_029
 * @tc.desc: test SetDeviceMode in proxy
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrPowerSavemodeTest, PowerSavemode_029, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerSavemode_029: SetDeviceMode start.";

    auto& powerMgrClient = PowerMgrClient::GetInstance();
    PowerMode mode = PowerMode::EXTREME_POWER_SAVE_MODE;
    PowerMode mode1 = PowerMode::EXTREME_POWER_SAVE_MODE;
    for (int i = 0; i < 100; i++) {
        powerMgrClient.SetDeviceMode(mode);
    }
    mode = powerMgrClient.GetDeviceMode();
    EXPECT_EQ(mode, mode1) << "PowerSavemode_029 fail to SetDeviceMode";
    GTEST_LOG_(INFO) << "PowerSavemode_029: SetDeviceMode end.";
}

/**
 * @tc.name: PowerSavemode_030
 * @tc.desc: test GetDeviceMode in proxy
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrPowerSavemodeTest, PowerSavemode_030, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerSavemode_030: GetDeviceMode start.";

    auto& powerMgrClient = PowerMgrClient::GetInstance();
    PowerMode mode = static_cast<PowerMode>(0);
    PowerMode mode1 = PowerMode::NORMAL_MODE;
    mode = powerMgrClient.GetDeviceMode();
    EXPECT_NE(mode, mode1) << "PowerSavemode_030 fail to GetDeviceMode";
}

/**
 * @tc.name: PowerSavemode_031
 * @tc.desc: test GetDeviceMode in proxy
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrPowerSavemodeTest, PowerSavemode_031, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerSavemode_031: GetDeviceMode start.";

    auto& powerMgrClient = PowerMgrClient::GetInstance();
    PowerMode mode = PowerMode::POWER_SAVE_MODE;
    PowerMode mode1 = PowerMode::POWER_SAVE_MODE;
    powerMgrClient.SetDeviceMode(mode);
    for (int i = 0; i < 100; i++) {
        mode = powerMgrClient.GetDeviceMode();
    }
    EXPECT_EQ(mode, mode1) << "PowerSavemode_031 fail to GetDeviceMode";
}

/**
 * @tc.name: PowerSavemode_032
 * @tc.desc: test PowerModeCallback
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrPowerSavemodeTest, PowerSavemode_032, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerSavemode_032: RegisterPowerModeCallback start.";

    auto& powerMgrClient = PowerMgrClient::GetInstance();
    const sptr<IPowerModeCallback> cb1 = new PowerModeTest1Callback();
    powerMgrClient.RegisterPowerModeCallback(cb1);
    PowerMode mode = PowerMode::POWER_SAVE_MODE;
    PowerMode mode1 = PowerMode::POWER_SAVE_MODE;
    powerMgrClient.SetDeviceMode(mode);
    mode = powerMgrClient.GetDeviceMode();
    EXPECT_EQ(mode, mode1) << "PowerSavemode_032 fail to PowerModeCallback";

    POWER_HILOGD(LABEL_TEST, "PowerSavemode_032 end.");
}

/**
 * @tc.name: PowerSavemode_033
 * @tc.desc: test PowerModeCallback
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrPowerSavemodeTest, PowerSavemode_033, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerSavemode_033: RegisterPowerModeCallback start.";

    auto& powerMgrClient = PowerMgrClient::GetInstance();
    POWER_HILOGD(LABEL_TEST, "PowerSavemode_033 Start.");
    const sptr<IPowerModeCallback> cb1 = new PowerModeTest1Callback();
    powerMgrClient.RegisterPowerModeCallback(cb1);
    PowerMode mode = static_cast<PowerMode>(1);
    PowerMode mode1 = static_cast<PowerMode>(1);
    powerMgrClient.SetDeviceMode(mode);
    mode = powerMgrClient.GetDeviceMode();
    EXPECT_NE(mode, mode1) << "PowerSavemode_033 fail to PowerModeCallback";

    POWER_HILOGD(LABEL_TEST, "PowerSavemode_033 end.");
}

/**
 * @tc.name: PowerSavemode_035
 * @tc.desc: test PowerModeCallback
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrPowerSavemodeTest, PowerSavemode_035, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerSavemode_035: RegisterPowerModeCallback start.";

    auto& powerMgrClient = PowerMgrClient::GetInstance();
    const sptr<IPowerModeCallback> cb1 = new PowerModeTest1Callback();
    for (int i = 0; i < 100; i++) {
        powerMgrClient.RegisterPowerModeCallback(cb1);
    }
    PowerMode mode = PowerMode::POWER_SAVE_MODE;
    PowerMode mode1 = PowerMode::POWER_SAVE_MODE;
    powerMgrClient.SetDeviceMode(mode);
    mode = powerMgrClient.GetDeviceMode();
    EXPECT_EQ(mode, mode1) << "PowerSavemode_035 fail to PowerModeCallback";

    POWER_HILOGD(LABEL_TEST, "PowerSavemode_035 end.");
}

/**
 * @tc.name: PowerSavemode_036
 * @tc.desc: test PowerModeCallback
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrPowerSavemodeTest, PowerSavemode_036, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerSavemode_036: UnRegisterPowerModeCallback start.";

    auto& powerMgrClient = PowerMgrClient::GetInstance();
    const sptr<IPowerModeCallback> cb1 = new PowerModeTest1Callback();
    powerMgrClient.UnRegisterPowerModeCallback(cb1);
    PowerMode mode = PowerMode::POWER_SAVE_MODE;
    PowerMode mode1 = PowerMode::POWER_SAVE_MODE;
    powerMgrClient.SetDeviceMode(mode);
    mode = powerMgrClient.GetDeviceMode();
    EXPECT_EQ(mode, mode1) << "PowerSavemode_036 fail to PowerModeCallback";

    POWER_HILOGD(LABEL_TEST, "PowerSavemode_036 end.");
}

/**
 * @tc.name: PowerSavemode_037
 * @tc.desc: test PowerModeCallback
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrPowerSavemodeTest, PowerSavemode_037, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerSavemode_037: UnRegisterPowerModeCallback start.";

    auto& powerMgrClient = PowerMgrClient::GetInstance();
    const sptr<IPowerModeCallback> cb1 = new PowerModeTest1Callback();
    powerMgrClient.UnRegisterPowerModeCallback(cb1);
    PowerMode mode = static_cast<PowerMode>(1);
    PowerMode mode1 = static_cast<PowerMode>(1);
    powerMgrClient.SetDeviceMode(mode);
    mode = powerMgrClient.GetDeviceMode();
    EXPECT_NE(mode, mode1) << "PowerSavemode_037 fail to PowerModeCallback";

    POWER_HILOGD(LABEL_TEST, "PowerSavemode_037 UnRegisterPowerModeCallback end.");
}

/**
 * @tc.name: PowerSavemode_039
 * @tc.desc: test PowerModeCallback
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrPowerSavemodeTest, PowerSavemode_039, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerSavemode_039: UnRegisterPowerModeCallback start.";

    auto& powerMgrClient = PowerMgrClient::GetInstance();
    const sptr<IPowerModeCallback> cb1 = new PowerModeTest1Callback();
    for (int i = 0; i < 100; i++) {
        powerMgrClient.UnRegisterPowerModeCallback(cb1);
    }
    sleep(SLEEP_WAIT_TIME_S);
    PowerMode mode = PowerMode::POWER_SAVE_MODE;
    PowerMode mode1 = PowerMode::POWER_SAVE_MODE;
    powerMgrClient.SetDeviceMode(mode);
    mode = powerMgrClient.GetDeviceMode();
    EXPECT_EQ(mode, mode1) << "PowerSavemode_036 fail to PowerModeCallback";

    POWER_HILOGD(LABEL_TEST, "PowerSavemode_039 end.");
}
} // namespace

PowerMgrPowerSavemodeTest::CommonEventServiCesSystemTest::CommonEventServiCesSystemTest(
    const CommonEventSubscribeInfo& subscriberInfo) : CommonEventSubscriber(subscriberInfo)
{
    POWER_HILOGD(LABEL_TEST, "subscribe.");
}

static uint32_t g_i = 0;
static int g_judgeNum = 2;

void PowerMgrPowerSavemodeTest::CommonEventServiCesSystemTest::OnReceiveEvent(const CommonEventData& data)
{
    std::string action = data.GetWant().GetAction();
    if (action == CommonEventSupport::COMMON_EVENT_POWER_SAVE_MODE_CHANGED) {
        POWER_HILOGD(LABEL_TEST, "CommonEventServiCesSystemTest::OnReceiveEvent.");
        g_i = g_judgeNum;
    }
    uint32_t j = 2;
    EXPECT_EQ(g_i, j) << "PowerSavemode_022 fail to PowerModeCallback";
    POWER_HILOGD(LABEL_TEST, "CommonEventServiCesSystemTest::OnReceiveEvent other.");
}

namespace {
/**
 * @tc.name: PowerSavemode_022
 * @tc.desc: ReceiveEvent
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrPowerSavemodeTest, PowerSavemode_022, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerSavemode_022: UnRegisterPowerModeCallback start.";

    bool result = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_POWER_SAVE_MODE_CHANGED);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServiCesSystemTest>(subscribeInfo);
    result = CommonEventManager::SubscribeCommonEvent(subscriberPtr);
    EXPECT_TRUE(result);
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    PowerMode mode = PowerMode::PERFORMANCE_MODE;
    powerMgrClient.SetDeviceMode(mode);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);

    POWER_HILOGD(LABEL_TEST, "PowerSavemode_022 UnRegisterPowerModeCallback end.");
}

/**
 * @tc.name: PowerSavemode_023
 * @tc.desc: ReceiveEvent
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrPowerSavemodeTest, PowerSavemode_023, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerSavemode_023: UnRegisterPowerModeCallback start.";

    bool result = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_POWER_SAVE_MODE_CHANGED);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServiCesSystemTest>(subscribeInfo);
    result = CommonEventManager::SubscribeCommonEvent(subscriberPtr);
    EXPECT_TRUE(result);
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    PowerMode mode = PowerMode::EXTREME_POWER_SAVE_MODE;
    powerMgrClient.SetDeviceMode(mode);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);

    POWER_HILOGD(LABEL_TEST, "PowerSavemode_023 end.");
}
} // namespace
