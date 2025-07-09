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
#include "power_wakeup_controller_oninput_test.h"
#include <input_manager.h>
#include "power_mgr_service.h"
#include "power_state_machine.h"
#include "wakeup_controller.h"

using namespace testing::ext;
using namespace OHOS::MMI;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;
static sptr<PowerMgrService> g_service;
static constexpr int32_t DISPLAY_POWER_MANAGER_ID = 3308;
static const std::string TEST_DEVICE_ID = "test_device_id";

void PowerWakeupControllerOninputTest::SetUpTestCase(void)
{
    g_service = DelayedSpSingleton<PowerMgrService>::GetInstance();
    g_service->OnStart();
    g_service->OnAddSystemAbility(DISPLAY_POWER_MANAGER_ID, TEST_DEVICE_ID);
}

void PowerWakeupControllerOninputTest::TearDownTestCase(void)
{
    g_service->OnStop();
    DelayedSpSingleton<PowerMgrService>::DestroyInstance();
}

MMI::InputManager *InputManager::GetInstance()
{
    return nullptr;
}

namespace {
/**
 * @tc.name: PowerWakeupControllerOninputTest001
 * @tc.desc: test RegisterMonitor(exception)
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
HWTEST_F(PowerWakeupControllerOninputTest, PowerWakeupControllerOninputTest001, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupControllerOninputTest001: start");
    GTEST_LOG_(INFO) << "PowerWakeupControllerOninputTest001: start";

    auto powerStateMachine = g_service->GetPowerStateMachine();
    g_service->WakeupControllerInit();
    auto wakeupController_ = g_service->GetWakeupController();
    wakeupController_->RegisterMonitor(PowerState::AWAKE);
    EXPECT_EQ(wakeupController_->monitorId_, -1);
    wakeupController_->RegisterMonitor(PowerState::INACTIVE);
    EXPECT_EQ(wakeupController_->monitorId_, -1);
    wakeupController_->RegisterMonitor(PowerState::UNKNOWN);
    EXPECT_EQ(wakeupController_->monitorId_, -1);
    powerStateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_APPLICATION);
    wakeupController_->RegisterMonitor(PowerState::AWAKE);
    EXPECT_EQ(wakeupController_->monitorId_, -1);
    powerStateMachine->SetState(PowerState::INACTIVE, StateChangeReason::STATE_CHANGE_REASON_APPLICATION);
    wakeupController_->RegisterMonitor(PowerState::INACTIVE);
    EXPECT_EQ(wakeupController_->monitorId_, -1);

    GTEST_LOG_(INFO) << "PowerWakeupControllerOninputTest001: end";
    POWER_HILOGI(LABEL_TEST, "PowerWakeupControllerOninputTest001: end");
}
} // namespace