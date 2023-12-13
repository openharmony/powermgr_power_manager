/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "device_state_action_native_test.h"

#include <ipc_skeleton.h>

#include "actions/irunning_lock_action.h"
#include "display_power_mgr_client.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

void DeviceStateActionNativeTest::SetUpTestCase() {}

namespace {
using SuspendCallback1 = std::function<void(uint32_t)>;

void DeviceStateActionCallback(uint32_t trigger) {}

/**
 * @tc.name: DeviceStateActionNative001
 * @tc.desc: test init in deviceStateAction
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStateActionNativeTest, DeviceStateActionNative001, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "DeviceStateActionNative001::fun is start!");
    auto deviceStateAction = std::make_shared<DeviceStateAction>();
    SuspendCallback1 sd = DeviceStateActionCallback;
    deviceStateAction->RegisterCallback(sd);

    DisplayPowerMgr::DisplayState stateType = DisplayPowerMgr::DisplayState::DISPLAY_ON;
    deviceStateAction->dispCallback_->OnDisplayStateChanged(
        DISPLAYID, stateType, static_cast<uint32_t>(StateChangeReason::STATE_CHANGE_REASON_APPLICATION));
    deviceStateAction->dispCallback_->OnDisplayStateChanged(
        DISPLAYID_A, stateType, static_cast<uint32_t>(StateChangeReason::STATE_CHANGE_REASON_APPLICATION));
    stateType = DisplayPowerMgr::DisplayState::DISPLAY_OFF;
    deviceStateAction->dispCallback_->OnDisplayStateChanged(
        DISPLAYID_A, stateType, static_cast<uint32_t>(StateChangeReason::STATE_CHANGE_REASON_APPLICATION));
    stateType = DisplayPowerMgr::DisplayState::DISPLAY_UNKNOWN;
    deviceStateAction->dispCallback_->OnDisplayStateChanged(
        DISPLAYID_A, stateType, static_cast<uint32_t>(StateChangeReason::STATE_CHANGE_REASON_APPLICATION));
    deviceStateAction->dispCallback_->notify_ = nullptr;
    deviceStateAction->dispCallback_->NotifyDisplayActionDone(DISPLAYID);

    DisplayState state = DisplayState::DISPLAY_OFF;
    StateChangeReason reason = StateChangeReason::STATE_CHANGE_REASON_INIT;
    EXPECT_TRUE(deviceStateAction->SetDisplayState(state, reason) == ActionResult::SUCCESS);
    EXPECT_TRUE(deviceStateAction->GetDisplayState() == DisplayState::DISPLAY_OFF);
    state = DisplayState::DISPLAY_ON;
    EXPECT_TRUE(deviceStateAction->SetDisplayState(state, reason) == ActionResult::SUCCESS);
    EXPECT_TRUE(deviceStateAction->GetDisplayState() == DisplayState::DISPLAY_ON);
    state = DisplayState::DISPLAY_DIM;
    EXPECT_TRUE(deviceStateAction->SetDisplayState(state, reason) == ActionResult::SUCCESS);
    EXPECT_TRUE(deviceStateAction->GetDisplayState() == DisplayState::DISPLAY_DIM);
    state = DisplayState::DISPLAY_SUSPEND;
    EXPECT_TRUE(deviceStateAction->SetDisplayState(state, reason) == ActionResult::SUCCESS);
    EXPECT_TRUE(deviceStateAction->GetDisplayState() == DisplayState::DISPLAY_SUSPEND);
    state = DisplayState::DISPLAY_UNKNOWN;
    EXPECT_TRUE(deviceStateAction->SetDisplayState(state, reason) == ActionResult::FAILED);
    EXPECT_FALSE(deviceStateAction->GetDisplayState() == DisplayState::DISPLAY_UNKNOWN);
    deviceStateAction->SetDisplayState(DisplayState::DISPLAY_OFF, reason);
    deviceStateAction->SetDisplayState(DisplayState::DISPLAY_ON, reason);
    POWER_HILOGI(LABEL_TEST, "DeviceStateActionNative001::fun is end!");
}
} // namespace
