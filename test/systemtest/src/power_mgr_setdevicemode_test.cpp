/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "power_mgr_setdevicemode_test.h"
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
std::string g_action = "";
} // namespace

static uint32_t g_i = 0;
static int g_judgeNum = 2;

void PowerMgrSetDeviceModeTest::SetUpTestCase(void)
{
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    pms->OnStart();
    SystemAbility::MakeAndRegisterAbility(pms.GetRefPtr());
}

PowerMgrSetDeviceModeTest::CommonEventServiCesSystemTest::CommonEventServiCesSystemTest(
    const CommonEventSubscribeInfo& subscriberInfo) : CommonEventSubscriber(subscriberInfo)
{
    POWER_HILOGI(LABEL_TEST, "CommonEventSubscriber.");
}

void PowerMgrSetDeviceModeTest::CommonEventServiCesSystemTest::OnReceiveEvent(const CommonEventData& data)
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
 * @tc.name: PowerSetdevicemode_001
 * @tc.desc: ReceiveEvent
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrSetDeviceModeTest, PowerSetdevicemode_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerSetdevicemode_001: UnRegisterPowerModeCallback start.";
    POWER_HILOGI(LABEL_TEST, "PowerSetdevicemode_001 function start!");
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

    POWER_HILOGI(LABEL_TEST, "PowerSetdevicemode_001 function end!");
    GTEST_LOG_(INFO) << "PowerSetdevicemode_001: UnRegisterPowerModeCallback end.";
}

/**
 * @tc.name: PowerSetdevicemode_002
 * @tc.desc: ReceiveEvent
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrSetDeviceModeTest, PowerSetdevicemode_002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerSetdevicemode_002: UnRegisterPowerModeCallback start.";
    POWER_HILOGI(LABEL_TEST, "PowerSetdevicemode_002 function start!");
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
    POWER_HILOGI(LABEL_TEST, "PowerSetdevicemode_002 function end!");
    GTEST_LOG_(INFO) << "PowerSetdevicemode_002: UnRegisterPowerModeCallback start.";
}
} // namespace