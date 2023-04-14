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

#include "power_mgr_notify_test.h"

#include <iostream>

#include <common_event_data.h>
#include <common_event_manager.h>
#include <common_event_publish_info.h>
#include <common_event_subscriber.h>
#include <common_event_support.h>
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
using namespace OHOS::AAFwk;
using namespace OHOS::Notification;

using NeedWaitFunc = std::function<bool()>;
using Clock = std::chrono::steady_clock;
using TimePoint = std::chrono::time_point<Clock>;

namespace {
constexpr int MAX_RETRY_TIME = 2;
constexpr int WAIT_EVENT_TIME_S = 1;
constexpr int RETRY_WAIT_TIME_US = 50000;
constexpr int SLEEP_WAIT_TIME_S = 3;

class TestCommonEventSubscriber : public CommonEventSubscriber {
public:
    std::mutex onReceivedLock_;
    bool received_ = false;
    TimePoint receivedTime_;
    std::string action_;
    shared_ptr<OHOS::AppExecFwk::EventHandler> receiveHandler_ = nullptr;

    bool receivedScreenOFF = false;
    bool receivedScreenOn = false;

    explicit TestCommonEventSubscriber(const sptr<CommonEventSubscribeInfo>& subscribeInfo)
        : CommonEventSubscriber(subscribeInfo) {}

    TestCommonEventSubscriber() {}

    ~TestCommonEventSubscriber() override {}
    void OnReceive(const sptr<CommonEventData> &event) override
    {
        GTEST_LOG_(INFO) << "PowerMgrMonitor:: OnReceive!!";
        receiveHandler_ = OHOS::AppExecFwk::EventHandler::Current();
        receivedTime_ = Clock::now();
        received_ = true;
        action_ = event->GetIntent()->GetAction();
        if (action_ == CommonEventSupport::COMMON_EVENT_SCREEN_OFF) {
            receivedScreenOFF = true;
        }
        if (action_ == CommonEventSupport::COMMON_EVENT_SCREEN_ON) {
            receivedScreenOn = true;
        }
    }
};

shared_ptr<TestCommonEventSubscriber> RegisterEvent()
{
    GTEST_LOG_(INFO) << "PowerMgrNotifyTest:: Regist Subscriber Start!!";
    sptr<AAFwk::Skills> skill = new AAFwk::Skills();
    skill->AddAction(CommonEventSupport::COMMON_EVENT_SCREEN_OFF);
    skill->AddAction(CommonEventSupport::COMMON_EVENT_SCREEN_ON);

    sptr<CommonEventSubscribeInfo> subscriberInfo = new CommonEventSubscribeInfo();
    subscriberInfo->SetSkills(skill);
    int tryTimes = 0;
    shared_ptr<TestCommonEventSubscriber> subscriber = make_shared<TestCommonEventSubscriber>(subscriberInfo);
    // Notice, rightnow AddAbilityListener is not ok, we use this method to make sure register success
    while (tryTimes < MAX_RETRY_TIME) {
        const auto result = CommonEventManager::GetInstance().SubscribeCommonEvent(subscriber);
        if (ERR_OK == result) {
            break;
        } else {
            GTEST_LOG_(INFO) << "PowerMgrNotifyTest:: Fail to register Subscriber, Sleep 50ms and try again!!!";
            usleep(RETRY_WAIT_TIME_US); // sleep 50ms
            // Reset powerMgrMonitor_, otherwise we will register fail
            subscriber = make_shared<TestCommonEventSubscriber>(subscriberInfo);
        }
        tryTimes++;
    }
    if (MAX_RETRY_TIME == tryTimes) {
        GTEST_LOG_(INFO) << "PowerMgrNotifyTest:: Fail to register Subscriber!!!";
        return nullptr;
    }
    GTEST_LOG_(INFO) << "PowerMgrNotifyTest:: register Subscriber Success!!";
    return subscriber;
}
}

void PowerMgrNotifyTest::SetUpTestCase(void)
{
}

void PowerMgrNotifyTest::TearDownTestCase(void)
{
}

void PowerMgrNotifyTest::SetUp(void)
{
}

void PowerMgrNotifyTest::TearDown(void)
{
}

namespace {
/**
 * @tc.name: PowerMgrNotifyTest001
 * @tc.desc: test powermgr notify for screen Off
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrNotifyTest, PowerMgrNotifyTest001, TestSize.Level0)
{
    // We need wait for 15s, to preivent the last test interfere(screen is in keyguard scene and screen is ON).
    int waitForStatusOk = 15;
    sleep(waitForStatusOk);
    GTEST_LOG_(INFO) << "PowerMgrNotifyTest001: Test ScreenOFF Notification start.";
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    // Wakeup Device before test
    GTEST_LOG_(INFO) << "PowerMgrNotifyTest001: Wakeup Device before test.";
    powerMgrClient.WakeupDevice();
    sleep(SLEEP_WAIT_TIME_S); // wait for 3 second
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrNotifyTest001: Prepare Fail, Screen is OFF.";
    GTEST_LOG_(INFO) << "PowerMgrNotifyTest001: Screen is On, Begin to Suspend Device!";

    shared_ptr<TestCommonEventSubscriber> subscriber = RegisterEvent();
    EXPECT_FALSE(subscriber == nullptr);

    powerMgrClient.SuspendDevice();

    sleep(WAIT_EVENT_TIME_S);

    auto err = CommonEventManager::GetInstance().UnsubscribeCommonEvent(subscriber);
    EXPECT_EQ(ERR_OK, err);

    GTEST_LOG_(INFO) << "PowerMgrNotifyTest001: Test ScreenOFF Notification end.";
}

/**
 * @tc.name: PowerMgrNotifyTest002
 * @tc.desc: test powermgr notify for screen On
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrNotifyTest, PowerMgrNotifyTest002, TestSize.Level0)
{
    sleep(SLEEP_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrNotifyTest002: Test ScreenOn Notification start.";
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    // Wakeup Device before test
    GTEST_LOG_(INFO) << "PowerMgrNotifyTest002: Suspend Device before test.";
    powerMgrClient.SuspendDevice();
    sleep(SLEEP_WAIT_TIME_S); // wait for 3 second
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerMgrNotifyTest002: Prepare Fail, Screen is On.";
    GTEST_LOG_(INFO) << "PowerMgrNotifyTest002: Screen is Off, Begin to Wakeup Device!";

    shared_ptr<TestCommonEventSubscriber> subscriber = RegisterEvent();
    EXPECT_FALSE(subscriber == nullptr);

    powerMgrClient.WakeupDevice();

    sleep(WAIT_EVENT_TIME_S);

    auto err = CommonEventManager::GetInstance().UnsubscribeCommonEvent(subscriber);
    EXPECT_EQ(ERR_OK, err);

    GTEST_LOG_(INFO) << "PowerMgrNotifyTest002: Test ScreenOn Notification end.";
}
}
