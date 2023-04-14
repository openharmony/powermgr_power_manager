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
#include <string>

#include "common_event_manager.h"
#include "ces_system_test.h"
#define private public
#define protected public
#undef private
#undef protected

#include "datetime_ex.h"

#include "power_common.h"
#include "power_mgr_client.h"
#include "power_mgr_service.h"
#include "power_state_machine.h"

using namespace testing::ext;
using namespace OHOS::EventFwk;
using namespace OHOS::PowerMgr;

CesSystemTest::CommonEventServiCesSystemTest::CommonEventServiCesSystemTest(
    const CommonEventSubscribeInfo &subscriberInfo)
    : CommonEventSubscriber(subscriberInfo)
{}

void CesSystemTest::CommonEventServiCesSystemTest::OnReceiveEvent(const CommonEventData &data)
{
    std::string action = data.GetWant().GetAction();
    if (action == CommonEventSupport::COMMON_EVENT_SHUTDOWN) {
        POWER_HILOGD(LABEL_TEST, "CommonEventServiCesSystemTest::OnReceiveEvent.");
    }
    POWER_HILOGD(LABEL_TEST, "CommonEventServiCesSystemTest::OnReceiveEvent other.");
}

void CesSystemTest::SetUpTestCase()
{}

void CesSystemTest::TearDownTestCase()
{}

void CesSystemTest::SetUp()
{}

void CesSystemTest::TearDown()
{}

namespace {
/*
 * @tc.number: CES_SubscriptionEvent_0100
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Verify the function when the input string is normal
 */
HWTEST_F(CesSystemTest, CES_SubscriptionEvent_0100, Function | MediumTest | Level1)
{
    bool result = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_SHUTDOWN);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServiCesSystemTest>(subscribeInfo);
    result = CommonEventManager::SubscribeCommonEvent(subscriberPtr);
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "CES_SubscriptionEvent_0100: ShutDownDevice start.";
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    if (false) {
        powerMgrClient.ShutDownDevice(string("ShutDownDeviceTest001"));
        sleep(SLEEP_WAIT_TIME_S);
    }
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}
}