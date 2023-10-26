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

#include <iostream>
#include <string_ex.h>

#include <bundle_mgr_proxy.h>
#include <condition_variable>
#include <datetime_ex.h>
#include <if_system_ability_manager.h>
#include <ipc_skeleton.h>
#include <mutex>

#include "common_event_manager.h"
#include "common_event_support.h"
#include "ipower_mode_callback.h"
#include "power_mgr_shutdown_fast_test.h"
#include "power_common.h"
#include "power_mgr_client.h"
#include "power_mgr_service.h"
#include "power_state_machine.h"
#include "power_state_machine_info.h"
#include "running_lock_info.h"
#include "running_lock.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS::EventFwk;
using namespace OHOS;

namespace {
std::condition_variable g_cv;
std::mutex g_mtx;
std::string g_action = "";
constexpr int64_t TIME_OUT = 1;
} // namespace

void PowerMgrShutDownFast::SetUpTestCase()
{
}

void PowerMgrShutDownFast::TearDownTestCase()
{
}

void PowerMgrShutDownFast::SetUp()
{
    g_action = "";
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_APPLICATION, std::string("test call SetUp"));
}

void PowerMgrShutDownFast::TearDown()
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_APPLICATION, std::string("test call TearDown"));
}

PowerMgrShutDownFast::CommonEventServiceSystemTest::CommonEventServiceSystemTest()
{
}

PowerMgrShutDownFast::CommonEventServiceSystemTest::CommonEventServiceSystemTest(
    const CommonEventSubscribeInfo& subscriberInfo)
    : CommonEventSubscriber(subscriberInfo)
{
}

std::shared_ptr<PowerMgrShutDownFast::CommonEventServiceSystemTest>
    PowerMgrShutDownFast::CommonEventServiceSystemTest::OnRegisterEvent(const std::string& eventStr)
{
    bool result = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventStr);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServiceSystemTest>(subscribeInfo);
    result = CommonEventManager::SubscribeCommonEvent(subscriberPtr);
    EXPECT_TRUE(result);
    return subscriberPtr;
}

void PowerMgrShutDownFast::CommonEventServiceSystemTest::OnReceiveEvent(const CommonEventData& data)
{
    g_action = data.GetWant().GetAction();
    GTEST_LOG_(INFO) << "PowerMgr_FastShutDown_action:" << g_action;
    g_cv.notify_one();
}

namespace {
/**
 * @tc.name: PowerMgr_FastShutDown_001
 * @tc.desc: Test whether fast shutdown can trigger screen off event
 * @tc.type: FUNC
 * @tc.require: issueI5I9EI
 */
HWTEST_F(PowerMgrShutDownFast, PowerMgr_FastShutDown_001, TestSize.Level2)
{
    auto eventPtr = std::make_unique<PowerMgrShutDownFast::CommonEventServiceSystemTest>();
    auto subscriberPtr = eventPtr->OnRegisterEvent(CommonEventSupport::COMMON_EVENT_SCREEN_OFF);

    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.ShutDownDevice(SHUTDOWN_FAST_REASON);
    std::unique_lock<std::mutex> lck(g_mtx);
    if (g_cv.wait_for(lck, std::chrono::seconds(TIME_OUT)) == std::cv_status::timeout) {
        g_cv.notify_one();
    }
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);

    EXPECT_TRUE(powerMgrClient.GetState() == PowerState::INACTIVE ||
        powerMgrClient.GetState() == PowerState::SLEEP);
    EXPECT_EQ(CommonEventSupport::COMMON_EVENT_SCREEN_OFF, g_action);
}

/**
 * @tc.name: PowerMgr_FastShutDown_002
 * @tc.desc: Test whether fast shutdown can trigger shut down event
 * @tc.type: FUNC
 * @tc.require: issueI5I9EI
 */
HWTEST_F(PowerMgrShutDownFast, PowerMgr_FastShutDown_002, TestSize.Level2)
{
    auto eventPtr = std::make_unique<PowerMgrShutDownFast::CommonEventServiceSystemTest>();
    auto subscriberPtr = eventPtr->OnRegisterEvent(CommonEventSupport::COMMON_EVENT_SHUTDOWN);
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.ShutDownDevice(SHUTDOWN_FAST_REASON);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);

    EXPECT_TRUE(powerMgrClient.GetState() == PowerState::INACTIVE ||
        powerMgrClient.GetState() == PowerState::SLEEP);
    EXPECT_TRUE(g_action.empty());
}
}
