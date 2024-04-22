/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

void PowerLevelEventSystemTestOff::SetUpTestCase(void)
{
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    pms->OnStart();
    SystemAbility::MakeAndRegisterAbility(pms.GetRefPtr());
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
 * @tc.name: PowerSavemode_001
 * @tc.desc: ReceiveEvent
 * @tc.type: FUNC
 * @tc.require: issueI5HUVS
 */
HWTEST_F(PowerLevelEventSystemTestOff, PowerSavemode_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerScreenOff_001 start";
    shared_ptr<CommonEventScreenOffTest> subscriber = CommonEventScreenOffTest::RegisterEvent();
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_APPLICATION);
    powerMgrClient.SuspendDevice(SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION);
    std::unique_lock<std::mutex> lck(g_mtx);
    if (g_cv.wait_for(lck, std::chrono::seconds(TIME_OUT)) == std::cv_status::timeout) {
        g_cv.notify_one();
    }
    CommonEventManager::UnSubscribeCommonEvent(subscriber);
    EXPECT_EQ(CommonEventSupport::COMMON_EVENT_SCREEN_OFF, g_action);
    POWER_HILOGI(LABEL_TEST, "PowerScreenOff_001_end");
}
} // namespace
