/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "power_getcontroller_mock_test.h"

#ifdef POWERMGR_GTEST
#define private   public
#define protected public
#endif

#include "power_mgr_service.h"
#include "power_state_machine.h"
#include "sp_singleton.h"
#include "wakeup_controller.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;
static sptr<PowerMgrService> g_service;

void PowerGetControllerMockTest::SetUpTestCase(void)
{
    g_service = DelayedSpSingleton<PowerMgrService>::GetInstance();
    g_service->OnStart();
}

void PowerGetControllerMockTest::TearDownTestCase(void)
{
    g_service->OnStop();
    DelayedSpSingleton<PowerMgrService>::DestroyInstance();
}

namespace {
/**
 * @tc.name: PowerGetControllerMockTest001
 * @tc.desc: test GetSourceList(exception)
 * @tc.type: FUNC
 * @tc.require: issueI7G6OY
 */
HWTEST_F(PowerGetControllerMockTest, PowerGetControllerMockTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerGetControllerMockTest001: start";
    std::shared_ptr<PowerStateMachine> stateMachine = g_service->GetPowerStateMachine();
    std::shared_ptr<WakeupController> wakeupController_ = std::make_shared<WakeupController>(stateMachine);
    EXPECT_TRUE(wakeupController_ != nullptr);
    wakeupController_->Wakeup();

    InputCallback* callback = new InputCallback();
    std::shared_ptr<OHOS::MMI::KeyEvent> keyEvent = OHOS::MMI::KeyEvent::Create();
    keyEvent->SetKeyAction(MMI::KeyEvent::KEY_ACTION_DOWN);
    keyEvent->SetKeyCode(OHOS::MMI::KeyEvent::KEYCODE_F1);
    callback->OnInputEvent(keyEvent);
    EXPECT_TRUE(callback != nullptr);
    delete callback;
    GTEST_LOG_(INFO) << "PowerGetControllerMockTest001:  end";
}
} // namespace
