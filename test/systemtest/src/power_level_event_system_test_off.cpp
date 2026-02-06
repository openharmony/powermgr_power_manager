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
#include "power_level_event_system_test_off.h"
#include "power_mgr_client.h"
#include "power_mgr_service.h"
#include "power_state_machine.h"
#include "power_state_machine_info.h"
#include "running_lock.h"
#include "running_lock_info.h"
#include <condition_variable>
#include <mutex>

using namespace testing;
using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS::EventFwk;
using namespace OHOS;
using namespace std;

static sptr<PowerMgrService> g_service;
static MockStateAction* g_shutdownState;
static MockStateAction* g_stateAction;
static MockPowerAction* g_powerAction;
static MockLockAction* g_lockAction;

namespace {
std::condition_variable g_cv;
std::mutex g_mtx;
std::string g_action = "";
constexpr int64_t TIME_OUT = 1;
} // namespace

static void ResetMockAction()
{
    POWER_HILOGI(LABEL_TEST, "ResetMockAction:Start");
    g_stateAction = new MockStateAction();
    g_shutdownState = new MockStateAction();
    g_powerAction = new MockPowerAction();
    g_lockAction = new MockLockAction();
    g_service->EnableMock(g_stateAction, g_shutdownState, g_powerAction, g_lockAction);
    POWER_HILOGI(LABEL_TEST, "ResetMockAction:End");
}

void PowerLevelEventSystemTestOff::SetUpTestCase(void)
{
    // create singleton service object at the beginning
    g_service = DelayedSpSingleton<PowerMgrService>::GetInstance();
    g_service->OnStart();
    SystemAbility::MakeAndRegisterAbility(g_service.GetRefPtr());
    ResetMockAction();
}

void PowerLevelEventSystemTestOff::TearDownTestCase(void)
{
    g_service->OnStop();
    DelayedSpSingleton<PowerMgrService>::DestroyInstance();
}

class CommonEventScreenOffTest : public EventFwk::CommonEventSubscriber {
public:
    explicit CommonEventScreenOffTest(const EventFwk::CommonEventSubscribeInfo& subscriberInfo);
    virtual ~CommonEventScreenOffTest() {};
    virtual void OnReceiveEvent(const EventFwk::CommonEventData& data);
    static shared_ptr<CommonEventScreenOffTest> RegisterEvent();
};

CommonEventScreenOffTest::CommonEventScreenOffTest(const CommonEventSubscribeInfo& subscriberInfo)
    : CommonEventSubscriber(subscriberInfo)
{
}

void CommonEventScreenOffTest::OnReceiveEvent(const CommonEventData& data)
{
    g_action = data.GetWant().GetAction();
    g_cv.notify_one();
}

shared_ptr<CommonEventScreenOffTest> CommonEventScreenOffTest::RegisterEvent()
{
    int32_t retryTimes = 2;
    bool succeed = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_SCREEN_OFF);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventScreenOffTest>(subscribeInfo);
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
 * @tc.name: PowerLevelEventSystemTestOff_001
 * @tc.desc: ReceiveEvent
 * @tc.type: FUNC
 * @tc.require: issueI5HUVS
 */
HWTEST_F(PowerLevelEventSystemTestOff, PowerLevelEventSystemTestOff_001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerLevelEventSystemTestOff_001 function start!");
    GTEST_LOG_(INFO) << "PowerLevelEventSystemTestOff_001 start";
    shared_ptr<CommonEventScreenOffTest> subscriber = CommonEventScreenOffTest::RegisterEvent();
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_APPLICATION);
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    powerMgrClient.SuspendDevice(SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION);
    std::unique_lock<std::mutex> lck(g_mtx);
    if (g_cv.wait_for(lck, std::chrono::seconds(TIME_OUT)) == std::cv_status::timeout) {
        g_cv.notify_one();
    }
    CommonEventManager::UnSubscribeCommonEvent(subscriber);
    GTEST_LOG_(INFO) << "PowerLevelEventSystemTestOff_001 end";
    POWER_HILOGI(LABEL_TEST, "PowerLevelEventSystemTestOff_001 function end!");
}
} // namespace
