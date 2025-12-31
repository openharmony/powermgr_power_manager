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

#include "power_utils_test.h"

#include "power_log.h"
#include "power_utils.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;


void PowerUtilsTest::SetUpTestCase(void)
{
}

void PowerUtilsTest::TearDownTestCase(void)
{
}

namespace {
/**
 * @tc.name: PowerUtilsTest001
 * @tc.desc: Test GetDisplayStateString
 * @tc.type: FUNC
 */
HWTEST_F (PowerUtilsTest, PowerUtilsTest001, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerUtilsTest001 function start!");
    EXPECT_EQ(PowerUtils::GetDisplayStateString(DisplayState::DISPLAY_OFF), "DISPLAY_OFF");
    EXPECT_EQ(PowerUtils::GetDisplayStateString(DisplayState::DISPLAY_DIM), "DISPLAY_DIM");
    EXPECT_EQ(PowerUtils::GetDisplayStateString(DisplayState::DISPLAY_ON), "DISPLAY_ON");
    EXPECT_EQ(PowerUtils::GetDisplayStateString(DisplayState::DISPLAY_SUSPEND), "DISPLAY_SUSPEND");
    EXPECT_EQ(PowerUtils::GetDisplayStateString(DisplayState::DISPLAY_UNKNOWN), "DISPLAY_UNKNOWN");
    EXPECT_EQ(PowerUtils::GetDisplayStateString(static_cast<DisplayState>(9)), "DISPLAY_UNKNOWN");
    POWER_HILOGI(LABEL_TEST, "PowerUtilsTest001 function end!");
}

/**
 * @tc.name: PowerUtilsTest002
 * @tc.desc: Test GetRunningLockTypeString
 * @tc.type: FUNC
 */
HWTEST_F (PowerUtilsTest, PowerUtilsTest002, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerUtilsTest002 function start!");
    EXPECT_EQ(PowerUtils::GetRunningLockTypeString(RunningLockType::RUNNINGLOCK_SCREEN), "SCREEN");
    EXPECT_EQ(PowerUtils::GetRunningLockTypeString(RunningLockType::RUNNINGLOCK_BACKGROUND), "BACKGROUND");
    EXPECT_EQ(PowerUtils::GetRunningLockTypeString(RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL),
        "PROXIMITY_SCREEN_CONTROL");
    EXPECT_EQ(PowerUtils::GetRunningLockTypeString(RunningLockType::RUNNINGLOCK_COORDINATION),
        "RUNNINGLOCK_COORDINATION");
    EXPECT_EQ(PowerUtils::GetRunningLockTypeString(RunningLockType::RUNNINGLOCK_BACKGROUND_PHONE),
        "BACKGROUND_PHONE");
    EXPECT_EQ(PowerUtils::GetRunningLockTypeString(RunningLockType::RUNNINGLOCK_BACKGROUND_NOTIFICATION),
        "BACKGROUND_NOTIFICATION");
    EXPECT_EQ(PowerUtils::GetRunningLockTypeString(RunningLockType::RUNNINGLOCK_BACKGROUND_AUDIO),
        "BACKGROUND_AUDIO");
    EXPECT_EQ(PowerUtils::GetRunningLockTypeString(RunningLockType::RUNNINGLOCK_BACKGROUND_SPORT),
        "BACKGROUND_SPORT");
    EXPECT_EQ(PowerUtils::GetRunningLockTypeString(RunningLockType::RUNNINGLOCK_BACKGROUND_NAVIGATION),
        "BACKGROUND_NAVIGATION");
    EXPECT_EQ(PowerUtils::GetRunningLockTypeString(RunningLockType::RUNNINGLOCK_BACKGROUND_TASK), "BACKGROUND_TASK");
    EXPECT_EQ(PowerUtils::GetRunningLockTypeString(RunningLockType::RUNNINGLOCK_BACKGROUND_USER_IDLE),
        "BACKGROUND_USER_IDLE");
    EXPECT_EQ(PowerUtils::GetRunningLockTypeString(RunningLockType::RUNNINGLOCK_BUTT), "BUTT");
    EXPECT_EQ(PowerUtils::GetRunningLockTypeString(static_cast<RunningLockType>(92)), "UNKNOWN");
    POWER_HILOGI(LABEL_TEST, "PowerUtilsTest002 function end!");
}

/**
 * @tc.name: PowerUtilsTest003
 * @tc.desc: Test GetDmsReasonByPowerReason
 * @tc.type: FUNC
 */
HWTEST_F (PowerUtilsTest, PowerUtilsTest003, TestSize.Level0)
{
    using namespace Rosen;
    POWER_HILOGI(LABEL_TEST, "PowerUtilsTest003 function start!");
    EXPECT_EQ(PowerUtils::GetDmsReasonByPowerReason(StateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT),
        PowerStateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT);
    EXPECT_EQ(PowerUtils::GetDmsReasonByPowerReason(StateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_SUCCESS),
        PowerStateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_SUCCESS);
    EXPECT_EQ(PowerUtils::GetDmsReasonByPowerReason(
        StateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_FAIL_SCREEN_ON),
        PowerStateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_FAIL_SCREEN_ON);
    EXPECT_EQ(PowerUtils::GetDmsReasonByPowerReason(
        StateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_FAIL_SCREEN_OFF),
        PowerStateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_FAIL_SCREEN_OFF);
    EXPECT_EQ(PowerUtils::GetDmsReasonByPowerReason(StateChangeReason::STATE_CHANGE_REASON_POWER_KEY),
        PowerStateChangeReason::STATE_CHANGE_REASON_POWER_KEY);
    EXPECT_EQ(PowerUtils::GetDmsReasonByPowerReason(StateChangeReason::STATE_CHANGE_REASON_TIMEOUT_NO_SCREEN_LOCK),
        PowerStateChangeReason::STATE_CHANGE_REASON_COLLABORATION);
    EXPECT_EQ(PowerUtils::GetDmsReasonByPowerReason(StateChangeReason::STATE_CHANGE_REASON_SWITCH),
        PowerStateChangeReason::STATE_CHANGE_REASON_SWITCH);
    EXPECT_EQ(PowerUtils::GetDmsReasonByPowerReason(StateChangeReason::STATE_CHANGE_REASON_SCREEN_CONNECT),
        PowerStateChangeReason::STATE_CHANGE_REASON_SCREEN_CONNECT);
    EXPECT_EQ(PowerUtils::GetDmsReasonByPowerReason(StateChangeReason::STATE_CHANGE_REASON_EX_SCREEN_INIT),
        PowerStateChangeReason::STATE_CHANGE_REASON_EX_SCREEN_INIT);
    EXPECT_EQ(PowerUtils::GetDmsReasonByPowerReason(StateChangeReason::STATE_CHANGE_REASON_HIBERNATE),
        PowerStateChangeReason::STATE_CHANGE_REASON_HIBERNATE);
    EXPECT_EQ(PowerUtils::GetDmsReasonByPowerReason(StateChangeReason::STATE_CHANGE_REASON_ABNORMAL_SCREEN_CONNECT),
        PowerStateChangeReason::STATE_CHANGE_REASON_ABNORMAL_SCREEN_CONNECT);
    EXPECT_EQ(PowerUtils::GetDmsReasonByPowerReason(StateChangeReason::STATE_CHANGE_REASON_ROLLBACK_HIBERNATE),
        PowerStateChangeReason::STATE_CHANGE_REASON_ROLLBACK_HIBERNATE);
    EXPECT_EQ(PowerUtils::GetDmsReasonByPowerReason(StateChangeReason::STATE_CHANGE_REASON_START_DREAM),
        PowerStateChangeReason::STATE_CHANGE_REASON_START_DREAM);
    EXPECT_EQ(PowerUtils::GetDmsReasonByPowerReason(StateChangeReason::STATE_CHANGE_REASON_END_DREAM),
        PowerStateChangeReason::STATE_CHANGE_REASON_END_DREAM);
    EXPECT_EQ(PowerUtils::GetDmsReasonByPowerReason(static_cast<StateChangeReason>(92)),
        static_cast<PowerStateChangeReason>(92));
    POWER_HILOGI(LABEL_TEST, "PowerUtilsTest003 function end!");
}

/**
 * @tc.name: PowerUtilsTest004
 * @tc.desc: Test GetReasonByUserActivity
 * @tc.type: FUNC
 */
HWTEST_F (PowerUtilsTest, PowerUtilsTest004, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerUtilsTest004 function start!");
    EXPECT_EQ(PowerUtils::GetReasonByUserActivity(UserActivityType::USER_ACTIVITY_TYPE_BUTTON),
        StateChangeReason::STATE_CHANGE_REASON_HARD_KEY);
    EXPECT_EQ(PowerUtils::GetReasonByUserActivity(UserActivityType::USER_ACTIVITY_TYPE_TOUCH),
        StateChangeReason::STATE_CHANGE_REASON_TOUCH);
    EXPECT_EQ(PowerUtils::GetReasonByUserActivity(UserActivityType::USER_ACTIVITY_TYPE_ACCESSIBILITY),
        StateChangeReason::STATE_CHANGE_REASON_ACCESSIBILITY);
    EXPECT_EQ(PowerUtils::GetReasonByUserActivity(UserActivityType::USER_ACTIVITY_TYPE_SOFTWARE),
        StateChangeReason::STATE_CHANGE_REASON_APPLICATION);
    EXPECT_EQ(PowerUtils::GetReasonByUserActivity(UserActivityType::USER_ACTIVITY_TYPE_SWITCH),
        StateChangeReason::STATE_CHANGE_REASON_SWITCH);
    EXPECT_EQ(PowerUtils::GetReasonByUserActivity(UserActivityType::USER_ACTIVITY_TYPE_CABLE),
      StateChangeReason::STATE_CHANGE_REASON_CABLE);
    EXPECT_EQ(PowerUtils::GetReasonByUserActivity(UserActivityType::USER_ACTIVITY_TYPE_ATTENTION),
        StateChangeReason::STATE_CHANGE_REASON_UNKNOWN);
    EXPECT_EQ(PowerUtils::GetReasonByUserActivity(UserActivityType::USER_ACTIVITY_TYPE_OTHER),
        StateChangeReason::STATE_CHANGE_REASON_UNKNOWN);
    EXPECT_EQ(PowerUtils::GetReasonByUserActivity(static_cast<UserActivityType>(92)),
        StateChangeReason::STATE_CHANGE_REASON_UNKNOWN);
    POWER_HILOGI(LABEL_TEST, "PowerUtilsTest004 function end!");
}
}