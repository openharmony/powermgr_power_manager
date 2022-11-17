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

#ifndef POWER_MGR_SHUTDOWN_FAST_TEST_H
#define POWER_MGR_SHUTDOWN_FAST_TEST_H

#include <gtest/gtest.h>

#include "common_event_data.h"
#include "common_event_subscriber.h"
#include "power_mode_callback_stub.h"

namespace OHOS {
namespace PowerMgr {

class PowerMgrShutDownFast : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    class CommonEventServiceSystemTest : public EventFwk::CommonEventSubscriber {
    public:
        CommonEventServiceSystemTest();
        CommonEventServiceSystemTest(const EventFwk::CommonEventSubscribeInfo& subscriberInfo);
        virtual ~CommonEventServiceSystemTest() {};
        virtual void OnReceiveEvent(const EventFwk::CommonEventData& data);
        std::shared_ptr<CommonEventServiceSystemTest> OnRegisterEvent(const std::string& eventStr);
};
};
} // namespace PowerMgr
} // namespace OHOS
#endif
