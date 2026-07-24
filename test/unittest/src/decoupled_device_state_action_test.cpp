/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "decoupled_device_state_action_test.h"

#include "power_mgr_factory.h"

// This file references the screen_decoupling symbol DecoupledDeviceStateAction, so it is compiled only when
// POWER_MANAGER_ENABLE_SCREEN_DECOUPLING is enabled; otherwise the body is skipped to avoid Unavailable.
#ifdef POWER_MANAGER_ENABLE_SCREEN_DECOUPLING
#include "display/decoupled_device_state_action.h"
#include "power_state_machine_info.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

/**
 * @tc.name: DecoupledDeviceStateAction001
 * @tc.desc: test GetDisplayState returns DISPLAY_UNKNOWN
 * @tc.type: FUNC
 */
HWTEST_F(DecoupledDeviceStateActionTest, DecoupledDeviceStateAction001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DecoupledDeviceStateAction001: GetDisplayState UNKNOWN start.";
    auto action = PowerMgrFactory::GetDeviceStateAction();
    EXPECT_TRUE(action->GetDisplayState() == DisplayState::DISPLAY_UNKNOWN);
    GTEST_LOG_(INFO) << "DecoupledDeviceStateAction001: GetDisplayState UNKNOWN end.";
}

/**
 * @tc.name: DecoupledDeviceStateAction002
 * @tc.desc: test SetDisplayState is a no-op and returns SUCCESS
 * @tc.type: FUNC
 */
HWTEST_F(DecoupledDeviceStateActionTest, DecoupledDeviceStateAction002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DecoupledDeviceStateAction002: SetDisplayState noop start.";
    auto action = PowerMgrFactory::GetDeviceStateAction();
    EXPECT_TRUE(action->SetDisplayState(DisplayState::DISPLAY_ON, StateChangeReason::STATE_CHANGE_REASON_INIT) ==
                ActionResult::SUCCESS);
    EXPECT_TRUE(action->SetDisplayState(DisplayState::DISPLAY_OFF, StateChangeReason::STATE_CHANGE_REASON_INIT) ==
                ActionResult::SUCCESS);
    EXPECT_TRUE(action->SetDisplayState(DisplayState::DISPLAY_DIM, StateChangeReason::STATE_CHANGE_REASON_INIT) ==
                ActionResult::SUCCESS);
    GTEST_LOG_(INFO) << "DecoupledDeviceStateAction002: SetDisplayState noop end.";
}

/**
 * @tc.name: DecoupledDeviceStateAction003
 * @tc.desc: test TryToCancelScreenOff returns false
 * @tc.type: FUNC
 */
HWTEST_F(DecoupledDeviceStateActionTest, DecoupledDeviceStateAction003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DecoupledDeviceStateAction003: TryToCancelScreenOff start.";
    auto action = PowerMgrFactory::GetDeviceStateAction();
    EXPECT_FALSE(action->TryToCancelScreenOff());
    GTEST_LOG_(INFO) << "DecoupledDeviceStateAction003: TryToCancelScreenOff end.";
}

/**
 * @tc.name: DecoupledDeviceStateAction004
 * @tc.desc: test screen control/sense methods are no-op and do not crash
 * @tc.type: FUNC
 */
HWTEST_F(DecoupledDeviceStateActionTest, DecoupledDeviceStateAction004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DecoupledDeviceStateAction004: screen methods noop start.";
    auto action = PowerMgrFactory::GetDeviceStateAction();
    action->SetInternalScreenDisplayPower(DisplayState::DISPLAY_ON, StateChangeReason::STATE_CHANGE_REASON_INIT);
    action->SetInternalScreenBrightness();
    action->BeginPowerkeyScreenOff();
    action->EndPowerkeyScreenOff();
    action->SetCoordinated(true);
    action->Suspend(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, 0);
    // NOLINTNEXTLINE(WordsTool.6) The underlying interface has a fixed naming convention UserActivity
    action->RefreshActivity(0, UserActivityType::USER_ACTIVITY_TYPE_OTHER, 0);
    std::function<void(uint32_t)> cb = nullptr;
    action->RegisterCallback(cb);
    EXPECT_TRUE(action->GetDisplayState() == DisplayState::DISPLAY_UNKNOWN);
    GTEST_LOG_(INFO) << "DecoupledDeviceStateAction004: screen methods noop end.";
}

/**
 * @tc.name: DecoupledDeviceStateAction005
 * @tc.desc: test GoToSleep/Wakeup/ForceSuspend multi-call does not crash
 * @tc.type: FUNC
 */
HWTEST_F(DecoupledDeviceStateActionTest, DecoupledDeviceStateAction005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DecoupledDeviceStateAction005: multi-call start.";
    auto action = PowerMgrFactory::GetDeviceStateAction();
    action->ForceSuspend();
    action->Wakeup(0, WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON, "power_key", "system");
    auto onSuspend = []() {};
    auto onWakeup = []() {};
    EXPECT_EQ(action->GoToSleep(onSuspend, onWakeup, true), ActionResult::SUCCESS);
    GTEST_LOG_(INFO) << "DecoupledDeviceStateAction005: multi-call end.";
}

#endif // POWER_MANAGER_ENABLE_SCREEN_DECOUPLING
