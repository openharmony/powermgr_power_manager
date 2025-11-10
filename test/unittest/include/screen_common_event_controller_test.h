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

#ifndef SCREEN_COMMON_EVENT_CONTROLLER_TEST_H
#define SCREEN_COMMON_EVENT_CONTROLLER_TEST_H

#include <gtest/gtest.h>
#include "iscreen_common_event_controller.h"
#include "screen_common_event_controller.h"
#include "power_mgr_service.h"
#include "power_log.h"
#include "power_state_machine_info.h"
#include <chrono>
#include <ctime>
#include <string>
#include <set>
#include <gmock/gmock.h>

namespace OHOS {
namespace PowerMgr {
class ScreenCommonEventControllerTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}

#ifdef POWER_MANAGER_ENABLE_WATCH_CUSTOMIZED_SCREEN_COMMON_EVENT_RULES
    class MockScreenCommonEventController : public IScreenCommonEventController {
    public:
        MOCK_METHOD(uint32_t, SetScreenOnCommonEventRules, (StateChangeReason reason), (override));
        MOCK_METHOD(uint32_t, SendCustomizedScreenEvent, (PowerState state, const std::set<std::string>& bundleNames),
            (override));
        MOCK_METHOD(uint32_t, NotifyOperateEventAfterScreenOn, (), (override));
    };
#endif
};
} // PowerMgr
} // OHOS
#endif // SCREEN_COMMON_EVENT_CONTROLLER_TEST_H