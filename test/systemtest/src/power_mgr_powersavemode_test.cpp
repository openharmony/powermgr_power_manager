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

#include <iostream>
#include <datetime_ex.h>
#include <gtest/gtest.h>
#include <if_system_ability_manager.h>
#include <ipc_skeleton.h>
#include <bundle_mgr_proxy.h>
#include <string_ex.h>

#include "power_common.h"
#include "power_mgr_client.h"
#include "power_mgr_service.h"
#include "power_state_machine.h"
#include "power_state_machine_info.h"
#include "running_lock_info.h"
#include "running_lock.h"
#include "ipower_mode_callback.h"
#include "common_event_manager.h"
#include "power_mgr_powersavemode_test.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS::EventFwk;
using namespace OHOS;
using namespace std;

void PowerMgrPowerSavemodeTest::PowerModeTest1Callback::PowerModeCallback()
{
    POWER_HILOGD(LABEL_TEST, "PowerModeTest1Callback::PowerModeCallback.");
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
 */
HWTEST_F (PowerMgrPowerSavemodeTest, PowerSavemode_001, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerSavemode_001: SetDeviceMode start.";

    auto& powerMgrClient = PowerMgrClient::GetInstance();
    uint32_t mode = 601;
    uint32_t mode1 = 601;
    powerMgrClient.SetDeviceMode(mode);
    sleep(SLEEP_WAIT_TIME_S);
    mode = powerMgrClient.GetDeviceMode();
    EXPECT_EQ(mode, mode1) << "PowerSavemode_001 fail to SetDeviceMode";
    GTEST_LOG_(INFO) << "PowerSavemode_001: SetDeviceMode end.";
}


/**
 * @tc.name: PowerSavemode_002
 * @tc.desc: test SetDeviceMode in proxy
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrPowerSavemodeTest, PowerSavemode_002, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerSavemode_002: SetDeviceMode start.";

    auto& powerMgrClient = PowerMgrClient::GetInstance();
    uint32_t mode = 602;
    uint32_t mode1 = 602;
    powerMgrClient.SetDeviceMode(mode);
    sleep(SLEEP_WAIT_TIME_S);
    mode = powerMgrClient.GetDeviceMode();
    EXPECT_EQ(mode, mode1) << "PowerSavemode_002 fail to SetDeviceMode";
    GTEST_LOG_(INFO) << "PowerSavemode_002: SetDeviceMode end." << mode;
}

/**
 * @tc.name: PowerSavemode_003
 * @tc.desc: test SetDeviceMode in proxy
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrPowerSavemodeTest, PowerSavemode_003, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerSavemode_003: SetDeviceMode start.";

    auto& powerMgrClient = PowerMgrClient::GetInstance();
    uint32_t mode = 603;
    uint32_t mode1 = 603;
    powerMgrClient.SetDeviceMode(mode);
    mode = powerMgrClient.GetDeviceMode();
    EXPECT_EQ(mode, mode1) << "PowerSavemode_003 fail to SetDeviceMode";
    GTEST_LOG_(INFO) << "PowerSavemode_003: SetDeviceMode end.";
}

/**
 * @tc.name: PowerSavemode_028
 * @tc.desc: test SetDeviceMode in proxy
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrPowerSavemodeTest, PowerSavemode_028, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerSavemode_028: SetDeviceMode start.";

    auto& powerMgrClient = PowerMgrClient::GetInstance();
    uint32_t mode = 1;
    uint32_t mode1 = 1;
    powerMgrClient.SetDeviceMode(mode);
    mode = powerMgrClient.GetDeviceMode();
    EXPECT_NE(mode, mode1) << "PowerSavemode_028 fail to SetDeviceMode abnormal";
    GTEST_LOG_(INFO) << "PowerSavemode_028: SetDeviceMode end.";
}

/**
 * @tc.name: PowerSavemode_029
 * @tc.desc: test SetDeviceMode in proxy
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrPowerSavemodeTest, PowerSavemode_029, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerSavemode_029: SetDeviceMode start.";

    auto& powerMgrClient = PowerMgrClient::GetInstance();
    uint32_t mode = 603;
    uint32_t mode1 = 603;
    for (int i=0; i<100; i++)
    {
        powerMgrClient.SetDeviceMode(mode);
    }
    mode = powerMgrClient.GetDeviceMode();
    EXPECT_EQ(mode, mode1) << "PowerSavemode_029 fail to SetDeviceMode";
    GTEST_LOG_(INFO) << "PowerSavemode_029: SetDeviceMode end.";
}

/**
 * @tc.name: PowerSavemode_004
 * @tc.desc: test GetDeviceMode in proxy
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrPowerSavemodeTest, PowerSavemode_004, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerSavemode_004: GetDeviceMode start.";

    auto& powerMgrClient = PowerMgrClient::GetInstance();
    uint32_t mode = 601;
    uint32_t mode1 = 601;
    powerMgrClient.SetDeviceMode(mode);
    mode = powerMgrClient.GetDeviceMode();
    EXPECT_EQ(mode, mode1) << "PowerSavemode_004 fail to GetDeviceMode";
    GTEST_LOG_(INFO) << "PowerSavemode_004: GetDeviceMode end. mode == " << mode;
}

/**
 * @tc.name: PowerSavemode_005
 * @tc.desc: test GetDeviceMode in proxy
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrPowerSavemodeTest, PowerSavemode_005, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerSavemode_005: GetDeviceMode start.";

    auto& powerMgrClient = PowerMgrClient::GetInstance();
    uint32_t mode = 602;
    uint32_t mode1 = 602;
    powerMgrClient.SetDeviceMode(mode);
    mode = powerMgrClient.GetDeviceMode();
    EXPECT_EQ(mode, mode1) << "PowerSavemode_005 fail to GetDeviceMode";
    GTEST_LOG_(INFO) << "PowerSavemode_005: GetDeviceMode end. mode == " << mode;
}

/**
 * @tc.name: PowerSavemode_006
 * @tc.desc: test GetDeviceMode in proxy
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrPowerSavemodeTest, PowerSavemode_006, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerSavemode_006: GetDeviceMode start.";

    auto& powerMgrClient = PowerMgrClient::GetInstance();
    uint32_t mode = 603;
    uint32_t mode1 = 603;
    powerMgrClient.SetDeviceMode(mode);
    mode = powerMgrClient.GetDeviceMode();
    EXPECT_EQ(mode, mode1) << "PowerSavemode_006 fail to GetDeviceMode";
    GTEST_LOG_(INFO) << "PowerSavemode_006: GetDeviceMode end. mode == " << mode;
}

/**
 * @tc.name: PowerSavemode_030
 * @tc.desc: test GetDeviceMode in proxy
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrPowerSavemodeTest, PowerSavemode_030, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerSavemode_030: GetDeviceMode start.";

    auto& powerMgrClient = PowerMgrClient::GetInstance();
    uint32_t mode = 0;
    uint32_t mode1 = 601;
    mode = powerMgrClient.GetDeviceMode();
    EXPECT_NE(mode, mode1) << "PowerSavemode_030 fail to GetDeviceMode";
    GTEST_LOG_(INFO) << "PowerSavemode_030: GetDeviceMode end. mode == " << mode;
}

/**
 * @tc.name: PowerSavemode_031
 * @tc.desc: test GetDeviceMode in proxy
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrPowerSavemodeTest, PowerSavemode_031, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerSavemode_031: GetDeviceMode start.";

    auto& powerMgrClient = PowerMgrClient::GetInstance();
    uint32_t mode = 601;
    uint32_t mode1 = 601;
    powerMgrClient.SetDeviceMode(mode);
    for (int i=0; i<100; i++)
    {
        mode = powerMgrClient.GetDeviceMode();
    }
    EXPECT_EQ(mode, mode1) << "PowerSavemode_031 fail to GetDeviceMode";
    GTEST_LOG_(INFO) << "PowerSavemode_031: GetDeviceMode end. mode == " << mode;
}

/**
 * @tc.name: PowerSavemode_032
 * @tc.desc: test PowerModeCallback
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrPowerSavemodeTest, PowerSavemode_032, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerSavemode_032: RegisterPowerModeCallback start.";

    auto& powerMgrClient = PowerMgrClient::GetInstance();
    const sptr<IPowerModeCallback> cb1 = new PowerModeTest1Callback();
    powerMgrClient.RegisterPowerModeCallback(cb1);
    sleep(SLEEP_WAIT_TIME_S);
    uint32_t mode = 601;
    uint32_t mode1 = 601;
    powerMgrClient.SetDeviceMode(mode);
    mode = powerMgrClient.GetDeviceMode();
    EXPECT_EQ(mode, mode1) << "PowerSavemode_032 fail to PowerModeCallback";

    POWER_HILOGD(LABEL_TEST, "PowerSavemode_032 1.");
}

/**
 * @tc.name: PowerSavemode_033
 * @tc.desc: test PowerModeCallback
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrPowerSavemodeTest, PowerSavemode_033, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerSavemode_033: RegisterPowerModeCallback start.";

    auto& powerMgrClient = PowerMgrClient::GetInstance();
    POWER_HILOGD(LABEL_TEST, "PowerSavemode_033 Start.");
    const sptr<IPowerModeCallback> cb1 = new PowerModeTest1Callback();
    powerMgrClient.RegisterPowerModeCallback(cb1);
    sleep(SLEEP_WAIT_TIME_S);
    uint32_t mode = 1;
    uint32_t mode1 = 1;
    powerMgrClient.SetDeviceMode(mode);
    mode = powerMgrClient.GetDeviceMode();
    EXPECT_NE(mode, mode1) << "PowerSavemode_033 fail to PowerModeCallback";

    POWER_HILOGD(LABEL_TEST, "PowerSavemode_033 1.");
}

/**
 * @tc.name: PowerSavemode_034
 * @tc.desc: test PowerModeCallback
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrPowerSavemodeTest, PowerSavemode_034, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerSavemode_034: RegisterPowerModeCallback start.";

    auto& powerMgrClient = PowerMgrClient::GetInstance();
    const sptr<IPowerModeCallback> cb1 = new PowerModeTest1Callback();
    powerMgrClient.RegisterPowerModeCallback(cb1);
    sleep(SLEEP_WAIT_TIME_S);
    uint32_t mode = 601;
    uint32_t mode1 = 601;
    powerMgrClient.SetDeviceMode(mode);
    mode = powerMgrClient.GetDeviceMode();
    EXPECT_EQ(mode, mode1) << "PowerSavemode_034 fail to PowerModeCallback";

    POWER_HILOGD(LABEL_TEST, "PowerSavemode_034 1.");
}

/**
 * @tc.name: PowerSavemode_035
 * @tc.desc: test PowerModeCallback
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrPowerSavemodeTest, PowerSavemode_035, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerSavemode_035: RegisterPowerModeCallback start.";

    auto& powerMgrClient = PowerMgrClient::GetInstance();
    const sptr<IPowerModeCallback> cb1 = new PowerModeTest1Callback();
    for (int i=0; i<100; i++)
    {
        powerMgrClient.RegisterPowerModeCallback(cb1);
    }
    sleep(SLEEP_WAIT_TIME_S);
    uint32_t mode = 601;
    uint32_t mode1 = 601;
    powerMgrClient.SetDeviceMode(mode);
    mode = powerMgrClient.GetDeviceMode();
    EXPECT_EQ(mode, mode1) << "PowerSavemode_035 fail to PowerModeCallback";

    POWER_HILOGD(LABEL_TEST, "PowerSavemode_035 1.");
}

/**
 * @tc.name: PowerSavemode_036
 * @tc.desc: test PowerModeCallback
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrPowerSavemodeTest, PowerSavemode_036, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerSavemode_036: UnRegisterPowerModeCallback start.";

    auto& powerMgrClient = PowerMgrClient::GetInstance();
    const sptr<IPowerModeCallback> cb1 = new PowerModeTest1Callback();
    powerMgrClient.UnRegisterPowerModeCallback(cb1);
    sleep(SLEEP_WAIT_TIME_S);
    uint32_t mode = 601;
    uint32_t mode1 = 601;
    powerMgrClient.SetDeviceMode(mode);
    mode = powerMgrClient.GetDeviceMode();
    EXPECT_EQ(mode, mode1) << "PowerSavemode_036 fail to PowerModeCallback";

    POWER_HILOGD(LABEL_TEST, "PowerSavemode_036 1.");
}

/**
 * @tc.name: PowerSavemode_037
 * @tc.desc: test PowerModeCallback
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrPowerSavemodeTest, PowerSavemode_037, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerSavemode_037: UnRegisterPowerModeCallback start.";

    auto& powerMgrClient = PowerMgrClient::GetInstance();
    const sptr<IPowerModeCallback> cb1 = new PowerModeTest1Callback();
    powerMgrClient.UnRegisterPowerModeCallback(cb1);
    sleep(SLEEP_WAIT_TIME_S);
    uint32_t mode = 1;
    uint32_t mode1 = 1;
    powerMgrClient.SetDeviceMode(mode);
    mode = powerMgrClient.GetDeviceMode();
    EXPECT_NE(mode, mode1) << "PowerSavemode_036 fail to PowerModeCallback";

    POWER_HILOGD(LABEL_TEST, "PowerSavemode_037 1.");
}

/**
 * @tc.name: PowerSavemode_038
 * @tc.desc: test PowerModeCallback
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrPowerSavemodeTest, PowerSavemode_038, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerSavemode_038: UnRegisterPowerModeCallback start.";

    auto& powerMgrClient = PowerMgrClient::GetInstance();
    const sptr<IPowerModeCallback> cb1 = new PowerModeTest1Callback();
    powerMgrClient.UnRegisterPowerModeCallback(cb1);
    sleep(SLEEP_WAIT_TIME_S);
    uint32_t mode = 601;
    uint32_t mode1 = 601;
    powerMgrClient.SetDeviceMode(mode);
    mode = powerMgrClient.GetDeviceMode();
    EXPECT_EQ(mode, mode1) << "PowerSavemode_036 fail to PowerModeCallback";

    POWER_HILOGD(LABEL_TEST, "PowerSavemode_038 1.");
}

/**
 * @tc.name: PowerSavemode_039
 * @tc.desc: test PowerModeCallback
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrPowerSavemodeTest, PowerSavemode_039, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerSavemode_039: UnRegisterPowerModeCallback start.";

    auto& powerMgrClient = PowerMgrClient::GetInstance();
    const sptr<IPowerModeCallback> cb1 = new PowerModeTest1Callback();
    for (int i=0; i<100; i++)
    {
        powerMgrClient.UnRegisterPowerModeCallback(cb1);
    }
    sleep(SLEEP_WAIT_TIME_S);
    uint32_t mode = 601;
    uint32_t mode1 = 601;
    powerMgrClient.SetDeviceMode(mode);
    mode = powerMgrClient.GetDeviceMode();
    EXPECT_EQ(mode, mode1) << "PowerSavemode_036 fail to PowerModeCallback";

    POWER_HILOGD(LABEL_TEST, "PowerSavemode_039 1.");
}
}

PowerMgrPowerSavemodeTest::CommonEventServiCesSystemTest::CommonEventServiCesSystemTest(
    const CommonEventSubscribeInfo &subscriberInfo)
    : CommonEventSubscriber(subscriberInfo)
{}

static uint32_t g_i = 0;
static int g_judgeNum = 2;

void PowerMgrPowerSavemodeTest::CommonEventServiCesSystemTest::OnReceiveEvent(const CommonEventData &data)
{
    std::string action = data.GetWant().GetAction();
    if (action == CommonEventSupport::COMMON_EVENT_DEVICE_IDLE_MODE_CHANGED) {
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
 */
HWTEST_F(PowerMgrPowerSavemodeTest, PowerSavemode_022, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerSavemode_022: UnRegisterPowerModeCallback start.";

    bool result = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_DEVICE_IDLE_MODE_CHANGED);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServiCesSystemTest>(subscribeInfo);
    result = CommonEventManager::SubscribeCommonEvent(subscriberPtr);
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "PowerSavemode_022: ShutDownDevice start.";
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    uint32_t mode = 602;
    powerMgrClient.SetDeviceMode(mode);
    sleep(SLEEP_WAIT_TIME_S);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);

    POWER_HILOGD(LABEL_TEST, "PowerSavemode_022 1.");
}

/**
 * @tc.name: PowerSavemode_023
 * @tc.desc: ReceiveEvent
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrPowerSavemodeTest, PowerSavemode_023, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerSavemode_023: UnRegisterPowerModeCallback start.";

    bool result = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_DEVICE_IDLE_MODE_CHANGED);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServiCesSystemTest>(subscribeInfo);
    result = CommonEventManager::SubscribeCommonEvent(subscriberPtr);
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "PowerSavemode_023: ShutDownDevice start.";
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    uint32_t mode = 603;
    powerMgrClient.SetDeviceMode(mode);
    sleep(SLEEP_WAIT_TIME_S);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);

    POWER_HILOGD(LABEL_TEST, "PowerSavemode_023 1.");
}

/**
 * @tc.name: PowerSavemode_024
 * @tc.desc: ReceiveEvent
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrPowerSavemodeTest, PowerSavemode_024, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerSavemode_024: UnRegisterPowerModeCallback start.";

    bool result = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_DEVICE_IDLE_MODE_CHANGED);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServiCesSystemTest>(subscribeInfo);
    result = CommonEventManager::SubscribeCommonEvent(subscriberPtr);
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "PowerSavemode_024: ShutDownDevice start.";
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    uint32_t mode = 602;
    powerMgrClient.SetDeviceMode(mode);
    sleep(SLEEP_WAIT_TIME_S);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);

    POWER_HILOGD(LABEL_TEST, "PowerSavemode_024 1.");
}

/**
 * @tc.name: PowerSavemode_025
 * @tc.desc: ReceiveEvent
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrPowerSavemodeTest, PowerSavemode_025, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerSavemode_025: UnRegisterPowerModeCallback start.";

    bool result = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_DEVICE_IDLE_MODE_CHANGED);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServiCesSystemTest>(subscribeInfo);
    result = CommonEventManager::SubscribeCommonEvent(subscriberPtr);
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "PowerSavemode_025: ShutDownDevice start.";
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    uint32_t mode = 603;
    powerMgrClient.SetDeviceMode(mode);
    sleep(SLEEP_WAIT_TIME_S);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);

    POWER_HILOGD(LABEL_TEST, "PowerSavemode_025 1.");
}
}