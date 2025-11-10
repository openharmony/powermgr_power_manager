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

#include "screen_common_event_controller_test.h"

namespace OHOS {
namespace PowerMgr {
using namespace testing;
using namespace testing::ext;

/**
 * @tc.name: ScreenCommonEventControllerTest
 * @tc.desc: test InitScreenCommonEventController
 * @tc.type: FUNC
 */
#ifdef POWER_MANAGER_ENABLE_WATCH_CUSTOMIZED_SCREEN_COMMON_EVENT_RULES
HWTEST_F(ScreenCommonEventControllerTest, ScreenCommonEventControllerTest001, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "InitScreenCommonEventController function start!");
    ScreenCommonEventController screenController;
    uint32_t result = screenController.InitScreenCommonEventController();
    EXPECT_EQ(result, ERROR_OK);
    POWER_HILOGI(LABEL_TEST, "InitScreenCommonEventController function end!");
}

/**
 * @tc.name: ScreenCommonEventControllerTest
 * @tc.desc: test SetScreenOnCommonEventRules
 * @tc.type: FUNC
 */
HWTEST_F(ScreenCommonEventControllerTest, ScreenCommonEventControllerTest002, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "SetScreenOnCommonEventRules function start!");
    StateChangeReason reason = StateChangeReason::STATE_CHANGE_REASON_MESSAGE_NOTIFICATION;
    ScreenCommonEventControllerTest::MockScreenCommonEventController mockController;
    EXPECT_CALL(mockController, SetScreenOnCommonEventRules(reason)).WillOnce(::testing::Return(ERROR_OK));
    ScreenCommonEventController screenController;
    uint32_t result = screenController.SetScreenOnCommonEventRules(reason);
    EXPECT_EQ(result, ERROR_OK);
    POWER_HILOGI(LABEL_TEST, "SetScreenOnCommonEventRules function end!");
}

/**
 * @tc.name: ScreenCommonEventControllerTest
 * @tc.desc: test NotifyOperateEventAfterScreenOn
 * @tc.type: FUNC
 */
HWTEST_F(ScreenCommonEventControllerTest, ScreenCommonEventControllerTest003, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "NotifyOperateEventAfterScreenOn function start!");
    ScreenCommonEventControllerTest::MockScreenCommonEventController mockController;
    EXPECT_CALL(mockController, NotifyOperateEventAfterScreenOn()).WillOnce(::testing::Return(ERROR_OK));
    ScreenCommonEventController screenController;
    uint32_t result = screenController.NotifyOperateEventAfterScreenOn();
    EXPECT_EQ(result, ERROR_OK);
    POWER_HILOGI(LABEL_TEST, "NotifyOperateEventAfterScreenOn function end!");
}

/**
 * @tc.name: ScreenCommonEventControllerTest
 * @tc.desc: test GetForegroundBundleNames
 * @tc.type: FUNC
 */
HWTEST_F(ScreenCommonEventControllerTest, ScreenCommonEventControllerTest004, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "GetForegroundBundleNames function start!");
    ScreenCommonEventController screenController;
    std::set<std::string> bundleNames = screenController.GetForegroundBundleNames();
    EXPECT_FALSE(bundleNames.size() > 5);
    POWER_HILOGI(LABEL_TEST, "GetForegroundBundleNames function end!");
}
#endif

/**
 * @tc.name: ScreenCommonEventControllerTest
 * @tc.desc: test SendCustomizedScreenEvent
 * @tc.type: FUNC
 */
HWTEST_F(ScreenCommonEventControllerTest, ScreenCommonEventControllerTest005, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "SendCustomizedScreenEvent function start!");
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    auto notify = pms->GetPowerMgrNotify();
    auto now = std::chrono::steady_clock::now();
    auto durationSinceEpoch = now.time_since_epoch();
    int64_t callTime = std::chrono::duration_cast<std::chrono::milliseconds>(durationSinceEpoch).count();
#ifdef POWER_MANAGER_ENABLE_WATCH_CUSTOMIZED_SCREEN_COMMON_EVENT_RULES
    std::set<std::string> bundleNames {};
    ScreenCommonEventControllerTest::MockScreenCommonEventController mockController;
    EXPECT_CALL(mockController, SendCustomizedScreenEvent(PowerState::AWAKE, bundleNames))
        .WillOnce(::testing::Return(ERROR_OK));
    ScreenCommonEventController screenController;
    uint32_t result = screenController.SendCustomizedScreenEvent(notify, PowerState::AWAKE, callTime, "");
    EXPECT_EQ(result, ERROR_OK);
    EXPECT_CALL(mockController, SendCustomizedScreenEvent(PowerState::INACTIVE, bundleNames))
        .WillOnce(::testing::Return(ERROR_OK));
    result = screenController.SendCustomizedScreenEvent(notify, PowerState::INACTIVE, callTime, "");
    EXPECT_EQ(result, ERROR_OK);
#else
    uint32_t ERROR_RESULT_OK = 1;
    ScreenCommonEventController screenEventController;
    uint32_t code = screenEventController.SendCustomizedScreenEvent(notify, PowerState::AWAKE, callTime, "");
    EXPECT_EQ(code, ERROR_RESULT_OK);
    code = screenEventController.SendCustomizedScreenEvent(notify, PowerState::INACTIVE, callTime, "");
    EXPECT_EQ(code, ERROR_RESULT_OK);
    code = screenEventController.SendCustomizedScreenEvent(notify, PowerState::SLEEP, callTime, "");
    EXPECT_EQ(code, ERROR_RESULT_OK);
#endif
    POWER_HILOGI(LABEL_TEST, "SendCustomizedScreenEvent function end!");
}

} // namespace PowerMgr
} // namespace OHOS