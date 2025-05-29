/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#include "power_suspend_test.h"
#include <fstream>
#include <thread>
#include <unistd.h>

#ifdef POWERMGR_GTEST
#define private   public
#define protected public
#endif

#include <datetime_ex.h>
#include <input_manager.h>
#include <securec.h>

#include "power_mgr_client.h"
#include "power_state_callback_stub.h"
#include "power_state_machine.h"
#include "setting_helper.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;
static constexpr int32_t SLEEP_WAIT_TIME_S = 2;
static constexpr int32_t SCREEN_OFF_TIME_MS = 5000;

namespace {
/**
 * @tc.name: PowerSuspendTest001
 * @tc.desc: test simulate powerkey event when screenon
 * @tc.type: FUNC
 */
HWTEST_F(PowerSuspendTest, PowerSuspendTest001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerSuspendTest001 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON, "PowerSuspendTest001");
    EXPECT_TRUE(powerMgrClient.IsScreenOn());

    std::shared_ptr<MMI::KeyEvent> keyEventPowerkeyDown = MMI::KeyEvent::Create();
    keyEventPowerkeyDown->SetKeyAction(MMI::KeyEvent::KEY_ACTION_DOWN);
    keyEventPowerkeyDown->SetKeyCode(MMI::KeyEvent::KEYCODE_POWER);
    std::shared_ptr<MMI::KeyEvent> keyEventPowerkeyUp = MMI::KeyEvent::Create();
    keyEventPowerkeyUp->SetKeyAction(MMI::KeyEvent::KEY_ACTION_UP);
    keyEventPowerkeyUp->SetKeyCode(MMI::KeyEvent::KEYCODE_POWER);

    auto inputManager = MMI::InputManager::GetInstance();
    inputManager->SimulateInputEvent(keyEventPowerkeyDown);
    inputManager->SimulateInputEvent(keyEventPowerkeyUp);
    sleep(4);
    EXPECT_FALSE(powerMgrClient.IsScreenOn());
    POWER_HILOGI(LABEL_TEST, "PowerSuspendTest001 function end!");
}
/**
 * @tc.name: PowerSuspendTest002
 * @tc.desc: test simulate powerkey event once when screenon
 * @tc.type: FUNC
 */
HWTEST_F(PowerSuspendTest, PowerSuspendTest002, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerSuspendTest002 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON, "PowerSuspendTest002");
    EXPECT_TRUE(powerMgrClient.IsScreenOn());

    auto inputManager = MMI::InputManager::GetInstance();
    std::shared_ptr<MMI::KeyEvent> keyEventPowerkeyDown = MMI::KeyEvent::Create();
    keyEventPowerkeyDown->SetKeyAction(MMI::KeyEvent::KEY_ACTION_DOWN);
    keyEventPowerkeyDown->SetKeyCode(MMI::KeyEvent::KEYCODE_POWER);
    std::shared_ptr<MMI::KeyEvent> keyEventPowerkeyUp = MMI::KeyEvent::Create();
    keyEventPowerkeyUp->SetKeyAction(MMI::KeyEvent::KEY_ACTION_UP);
    keyEventPowerkeyUp->SetKeyCode(MMI::KeyEvent::KEYCODE_POWER);

    inputManager->SimulateInputEvent(keyEventPowerkeyDown);
    inputManager->SimulateInputEvent(keyEventPowerkeyUp);
    inputManager->SimulateInputEvent(keyEventPowerkeyDown);
    inputManager->SimulateInputEvent(keyEventPowerkeyUp);
    powerMgrClient.OverrideScreenOffTime(SCREEN_OFF_TIME_MS);
    sleep(3);
    // the second powerkey event would interrupt the transition to INACTIVE
    EXPECT_TRUE(powerMgrClient.IsScreenOn());

    POWER_HILOGI(LABEL_TEST, "PowerSuspendTest002 function end!");
}
} // namespace
