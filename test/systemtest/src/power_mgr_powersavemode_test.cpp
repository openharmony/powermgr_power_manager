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

#include <bundle_mgr_proxy.h>
#include <condition_variable>
#include <datetime_ex.h>
#include <gtest/gtest.h>
#include <if_system_ability_manager.h>
#include <iostream>
#include <ipc_skeleton.h>
#include <mutex>
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

namespace {
std::condition_variable g_cv;
std::mutex g_mtx;
std::string g_action = "";
constexpr int64_t TIME_OUT = 1;
} // namespace

static uint32_t g_i = 0;
static int g_judgeNum = 2;

void PowerMgrPowerSavemodeTest::PowerModeTest1Callback::OnPowerModeChanged(PowerMode mode)
{
    POWER_HILOGI(LABEL_TEST, "PowerModeTest1Callback::OnPowerModeChanged.");
}

void PowerMgrPowerSavemodeTest::SetUpTestCase(void)
{
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    pms->OnStart();
    SystemAbility::MakeAndRegisterAbility(pms.GetRefPtr());
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
    POWER_HILOGI(LABEL_TEST, "PowerSavemode_001 start.");

    auto& powerMgrClient = PowerMgrClient::GetInstance();
    PowerMode modeFirst = PowerMode::POWER_SAVE_MODE;
    PowerMode modeSecond = PowerMode::POWER_SAVE_MODE;
    powerMgrClient.SetDeviceMode(modeFirst);
    modeFirst = powerMgrClient.GetDeviceMode();
    EXPECT_EQ(modeFirst, modeSecond) << "PowerSavemode_001 fail to SetDeviceMode";
    POWER_HILOGI(LABEL_TEST, "PowerSavemode_001 end.");
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
    POWER_HILOGI(LABEL_TEST, "PowerSavemode_002 start.");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    PowerMode modeFirst = PowerMode::PERFORMANCE_MODE;
    PowerMode modeSecond = PowerMode::PERFORMANCE_MODE;
    powerMgrClient.SetDeviceMode(modeFirst);
    modeFirst = powerMgrClient.GetDeviceMode();
    EXPECT_EQ(modeFirst, modeSecond) << "PowerSavemode_002 fail to SetDeviceMode";
    POWER_HILOGI(LABEL_TEST, "PowerSavemode_002 end.");
    GTEST_LOG_(INFO) << "PowerSavemode_002: SetDeviceMode end.";
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
    POWER_HILOGI(LABEL_TEST, "PowerSavemode_003 start.");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    PowerMode modeFirst = PowerMode::EXTREME_POWER_SAVE_MODE;
    PowerMode modeSecond = PowerMode::EXTREME_POWER_SAVE_MODE;
    powerMgrClient.SetDeviceMode(modeFirst);
    modeFirst = powerMgrClient.GetDeviceMode();
    EXPECT_EQ(modeFirst, modeSecond) << "PowerSavemode_003 fail to SetDeviceMode";
    POWER_HILOGI(LABEL_TEST, "PowerSavemode_003 end.");
    GTEST_LOG_(INFO) << "PowerSavemode_003: SetDeviceMode end.";
}

/**
 * @tc.name: PowerSavemode_004
 * @tc.desc: test SetDeviceMode in proxy
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrPowerSavemodeTest, PowerSavemode_004, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerSavemode_004: SetDeviceMode start.";
    POWER_HILOGI(LABEL_TEST, "PowerSavemode_004 start.");
    int PARM_ONE = 1;
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    PowerMode modeFirst = static_cast<PowerMode>(PARM_ONE);
    PowerMode modeSecond = static_cast<PowerMode>(PARM_ONE);
    powerMgrClient.SetDeviceMode(modeFirst);
    modeFirst = powerMgrClient.GetDeviceMode();
    EXPECT_NE(modeFirst, modeSecond) << "PowerSavemode_004 fail to SetDeviceMode abnormal";
    POWER_HILOGI(LABEL_TEST, "PowerSavemode_004 end.");
    GTEST_LOG_(INFO) << "PowerSavemode_004: SetDeviceMode end.";
}

/**
 * @tc.name: PowerSavemode_005
 * @tc.desc: test SetDeviceMode in proxy
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrPowerSavemodeTest, PowerSavemode_005, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerSavemode_005: SetDeviceMode start.";
    POWER_HILOGI(LABEL_TEST, "PowerSavemode_005 start.");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    PowerMode modeFirst = PowerMode::EXTREME_POWER_SAVE_MODE;
    PowerMode modeSecond = PowerMode::EXTREME_POWER_SAVE_MODE;
    for (int i = 0; i < 100; i++) {
        powerMgrClient.SetDeviceMode(modeFirst);
    }
    modeFirst = powerMgrClient.GetDeviceMode();
    EXPECT_EQ(modeFirst, modeSecond) << "PowerSavemode_005 fail to SetDeviceMode";
    POWER_HILOGI(LABEL_TEST, "PowerSavemode_005 end.");
    GTEST_LOG_(INFO) << "PowerSavemode_005: SetDeviceMode end.";
}

/**
 * @tc.name: PowerSavemode_006
 * @tc.desc: test GetDeviceMode in proxy
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrPowerSavemodeTest, PowerSavemode_006, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerSavemode_006: GetDeviceMode start.";
    POWER_HILOGI(LABEL_TEST, "PowerSavemode_006 start.");
    int PARM_ZERO = 0;
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    PowerMode modeFirst = static_cast<PowerMode>(PARM_ZERO);
    PowerMode modeSecond = PowerMode::NORMAL_MODE;
    modeFirst = powerMgrClient.GetDeviceMode();
    EXPECT_NE(modeFirst, modeSecond) << "PowerSavemode_006 fail to GetDeviceMode";
    POWER_HILOGI(LABEL_TEST, "PowerSavemode_006 end.");
    GTEST_LOG_(INFO) << "PowerSavemode_006: GetDeviceMode end.";
}

/**
 * @tc.name: PowerSavemode_007
 * @tc.desc: test GetDeviceMode in proxy
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrPowerSavemodeTest, PowerSavemode_007, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerSavemode_007: GetDeviceMode start.";
    POWER_HILOGI(LABEL_TEST, "PowerSavemode_007 start.");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    PowerMode modeFirst = PowerMode::POWER_SAVE_MODE;
    PowerMode modeSecond = PowerMode::POWER_SAVE_MODE;
    powerMgrClient.SetDeviceMode(modeFirst);
    for (int i = 0; i < 100; i++) {
        modeFirst = powerMgrClient.GetDeviceMode();
    }
    EXPECT_EQ(modeFirst, modeSecond) << "PowerSavemode_007 fail to GetDeviceMode";
    POWER_HILOGI(LABEL_TEST, "PowerSavemode_007 start.");
    GTEST_LOG_(INFO) << "PowerSavemode_007: GetDeviceMode end.";
}

/**
 * @tc.name: PowerSavemode_008
 * @tc.desc: test PowerModeCallback
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrPowerSavemodeTest, PowerSavemode_008, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerSavemode_008: RegisterPowerModeCallback start.";
    POWER_HILOGI(LABEL_TEST, "PowerSavemode_008 start.");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    const sptr<IPowerModeCallback> callBack = new PowerModeTest1Callback();
    powerMgrClient.RegisterPowerModeCallback(callBack);
    PowerMode modeFirst = PowerMode::POWER_SAVE_MODE;
    PowerMode modeSecond = PowerMode::POWER_SAVE_MODE;
    powerMgrClient.SetDeviceMode(modeFirst);
    modeFirst = powerMgrClient.GetDeviceMode();
    EXPECT_EQ(modeFirst, modeSecond) << "PowerSavemode_008 fail to PowerModeCallback";
    POWER_HILOGI(LABEL_TEST, "PowerSavemode_008 end.");
    GTEST_LOG_(INFO) << "PowerSavemode_008: RegisterPowerModeCallback end.";
}

/**
 * @tc.name: PowerSavemode_009
 * @tc.desc: test PowerModeCallback
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrPowerSavemodeTest, PowerSavemode_009, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerSavemode_009: RegisterPowerModeCallback start.";
    POWER_HILOGI(LABEL_TEST, "PowerSavemode_009 Start.");
    int PARM_ONE = 1;
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    const sptr<IPowerModeCallback> callBack = new PowerModeTest1Callback();
    powerMgrClient.RegisterPowerModeCallback(callBack);

    PowerMode modeFirst = static_cast<PowerMode>(PARM_ONE);
    PowerMode modeSecond = static_cast<PowerMode>(PARM_ONE);
    powerMgrClient.SetDeviceMode(modeFirst);
    modeFirst = powerMgrClient.GetDeviceMode();
    EXPECT_NE(modeFirst, modeSecond) << "PowerSavemode_009 fail to PowerModeCallback";
    POWER_HILOGI(LABEL_TEST, "PowerSavemode_009 end.");
    GTEST_LOG_(INFO) << "PowerSavemode_009: RegisterPowerModeCallback end.";
}

/**
 * @tc.name: PowerSavemode_010
 * @tc.desc: test PowerModeCallback
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrPowerSavemodeTest, PowerSavemode_010, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerSavemode_010: RegisterPowerModeCallback start.";
    POWER_HILOGI(LABEL_TEST, "PowerSavemode_010 start.");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    const sptr<IPowerModeCallback> callBack = new PowerModeTest1Callback();
    for (int i = 0; i < 100; i++) {
        powerMgrClient.RegisterPowerModeCallback(callBack);
    }
    PowerMode modeFirst = PowerMode::POWER_SAVE_MODE;
    PowerMode modeSecond = PowerMode::POWER_SAVE_MODE;
    powerMgrClient.SetDeviceMode(modeFirst);
    modeFirst = powerMgrClient.GetDeviceMode();
    EXPECT_EQ(modeFirst, modeSecond) << "PowerSavemode_010 fail to PowerModeCallback";
    POWER_HILOGI(LABEL_TEST, "PowerSavemode_010 end.");
    GTEST_LOG_(INFO) << "PowerSavemode_010: RegisterPowerModeCallback end.";
}

/**
 * @tc.name: PowerSavemode_011
 * @tc.desc: test PowerModeCallback
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrPowerSavemodeTest, PowerSavemode_011, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerSavemode_011: UnRegisterPowerModeCallback start.";
    POWER_HILOGI(LABEL_TEST, "PowerSavemode_011 start.");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    const sptr<IPowerModeCallback> callBack = new PowerModeTest1Callback();
    powerMgrClient.UnRegisterPowerModeCallback(callBack);
    PowerMode modeFirst = PowerMode::POWER_SAVE_MODE;
    PowerMode modeSecond = PowerMode::POWER_SAVE_MODE;
    powerMgrClient.SetDeviceMode(modeFirst);
    modeFirst = powerMgrClient.GetDeviceMode();
    EXPECT_EQ(modeFirst, modeSecond) << "PowerSavemode_011 fail to PowerModeCallback";
    POWER_HILOGI(LABEL_TEST, "PowerSavemode_011 end.");
    GTEST_LOG_(INFO) << "PowerSavemode_011: UnRegisterPowerModeCallback end.";
}

/**
 * @tc.name: PowerSavemode_012
 * @tc.desc: test PowerModeCallback
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrPowerSavemodeTest, PowerSavemode_012, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerSavemode_012: UnRegisterPowerModeCallback start.";
    POWER_HILOGI(LABEL_TEST, "PowerSavemode_012 UnRegisterPowerModeCallback start.");
    int PARM_ONE = 1;
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    const sptr<IPowerModeCallback> callBack = new PowerModeTest1Callback();
    powerMgrClient.UnRegisterPowerModeCallback(callBack);
    PowerMode modeFirst = static_cast<PowerMode>(PARM_ONE);
    PowerMode modeSecond = static_cast<PowerMode>(PARM_ONE);
    powerMgrClient.SetDeviceMode(modeFirst);
    modeFirst = powerMgrClient.GetDeviceMode();
    EXPECT_NE(modeFirst, modeSecond) << "PowerSavemode_012 fail to PowerModeCallback";
    POWER_HILOGI(LABEL_TEST, "PowerSavemode_012 UnRegisterPowerModeCallback end.");
    GTEST_LOG_(INFO) << "PowerSavemode_012: UnRegisterPowerModeCallback end.";
}

/**
 * @tc.name: PowerSavemode_013
 * @tc.desc: test PowerModeCallback
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrPowerSavemodeTest, PowerSavemode_013, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerSavemode_013: UnRegisterPowerModeCallback start.";
    POWER_HILOGI(LABEL_TEST, "PowerSavemode_013 start.");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    const sptr<IPowerModeCallback> callBack = new PowerModeTest1Callback();
    for (int i = 0; i < 100; i++) {
        powerMgrClient.UnRegisterPowerModeCallback(callBack);
    }
    PowerMode modeFirst = PowerMode::POWER_SAVE_MODE;
    PowerMode modeSecond = PowerMode::POWER_SAVE_MODE;
    powerMgrClient.SetDeviceMode(modeFirst);
    modeFirst = powerMgrClient.GetDeviceMode();
    EXPECT_EQ(modeFirst, modeSecond) << "PowerSavemode_013 fail to PowerModeCallback";
    POWER_HILOGI(LABEL_TEST, "PowerSavemode_013 end.");
    GTEST_LOG_(INFO) << "PowerSavemode_013: UnRegisterPowerModeCallback end.";
}
} // namespace

PowerMgrPowerSavemodeTest::CommonEventServiCesSystemTest::CommonEventServiCesSystemTest(
    const CommonEventSubscribeInfo& subscriberInfo) : CommonEventSubscriber(subscriberInfo)
{
    POWER_HILOGI(LABEL_TEST, "CommonEventSubscriber.");
}

void PowerMgrPowerSavemodeTest::CommonEventServiCesSystemTest::OnReceiveEvent(const CommonEventData& data)
{
    std::string action = data.GetWant().GetAction();
    if (action == CommonEventSupport::COMMON_EVENT_POWER_SAVE_MODE_CHANGED) {
        POWER_HILOGI(LABEL_TEST, "CommonEventServiCesSystemTest::OnReceiveEvent.");
        g_i = g_judgeNum;
    }
    uint32_t j = 2;
    EXPECT_EQ(g_i, j) << "PowerSavemode_022 fail to PowerModeCallback";
    POWER_HILOGI(LABEL_TEST, "CommonEventServiCesSystemTest::OnReceiveEvent other.");
}

class CommonEventSaveModeTest : public EventFwk::CommonEventSubscriber {
public:
    CommonEventSaveModeTest() = default;
    explicit CommonEventSaveModeTest(const EventFwk::CommonEventSubscribeInfo& subscriberInfo);
    virtual ~CommonEventSaveModeTest() {};
    virtual void OnReceiveEvent(const EventFwk::CommonEventData& data);
    static shared_ptr<CommonEventSaveModeTest> RegisterEvent();
};

CommonEventSaveModeTest::CommonEventSaveModeTest(const CommonEventSubscribeInfo& subscriberInfo)
    : CommonEventSubscriber(subscriberInfo)
{
}

void CommonEventSaveModeTest::OnReceiveEvent(const CommonEventData& data)
{
    g_action = data.GetWant().GetAction();
    g_cv.notify_one();
}

shared_ptr<CommonEventSaveModeTest> CommonEventSaveModeTest::RegisterEvent()
{
    int32_t retryTimes = 2;
    bool succeed = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_POWER_SAVE_MODE_CHANGED);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventSaveModeTest>(subscribeInfo);
    for (int32_t tryTimes = 0; tryTimes < retryTimes; tryTimes++) {
        succeed = CommonEventManager::SubscribeCommonEvent(subscriberPtr);
    }
    if (!succeed) {
        return nullptr;
    }
    return subscriberPtr;
}

namespace {
/**
 * @tc.name: PowerSavemode_014
 * @tc.desc: ReceiveEvent
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrPowerSavemodeTest, PowerSavemode_014, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerSavemode_014: UnRegisterPowerModeCallback start.";
    POWER_HILOGI(LABEL_TEST, "PowerSavemode_014 UnRegisterPowerModeCallback start.");
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

    POWER_HILOGI(LABEL_TEST, "PowerSavemode_014 UnRegisterPowerModeCallback end.");
    GTEST_LOG_(INFO) << "PowerSavemode_014: UnRegisterPowerModeCallback end.";
}

/**
 * @tc.name: PowerSavemode_015
 * @tc.desc: ReceiveEvent
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrPowerSavemodeTest, PowerSavemode_015, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerSavemode_015: UnRegisterPowerModeCallback start.";
    POWER_HILOGI(LABEL_TEST, "PowerSavemode_015 start.");
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
    POWER_HILOGI(LABEL_TEST, "PowerSavemode_015 end.");
    GTEST_LOG_(INFO) << "PowerSavemode_015: UnRegisterPowerModeCallback start.";
}
} // namespace