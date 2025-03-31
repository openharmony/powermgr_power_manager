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
#include "power_wakeup_test.h"
#include <fstream>
#include <thread>
#include <unistd.h>

#include "axis_event.h"
#include "input_device.h"
#include "pointer_event.h"
#include <datetime_ex.h>
#include <input_manager.h>
#include <securec.h>

#include "power_mgr_client.h"
#include "power_state_machine.h"
#include "setting_helper.h"
#include "json/reader.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

static constexpr int32_t SLEEP_WAIT_TIME_S = 2;
static constexpr int32_t SLEEP_WAIT_TIME_MS = 400;
static constexpr int32_t DISPLAY_OFF_TIME_MS = 600;
static constexpr int32_t RECOVER_DISPLAY_OFF_TIME_S = 30 * 1000;
static constexpr int32_t DISPLAY_POWER_MANAGER_ID = 3308;
static constexpr int32_t SCREEN_OFF_TIME_MS = 5000;
static const std::string TEST_DEVICE_ID = "test_device_id";

namespace {
MMI::PointerEvent::PointerItem CreatePointerItem(
    int32_t pointerId, int32_t deviceId, const std::pair<int32_t, int32_t>& displayLocation, bool isPressed)
{
    MMI::PointerEvent::PointerItem item;
    item.SetPointerId(pointerId);
    item.SetDeviceId(deviceId);
    item.SetDisplayX(displayLocation.first);
    item.SetDisplayY(displayLocation.second);
    item.SetPressed(isPressed);
    return item;
}

/**
 * @tc.name: PowerWakeupTest001
 * @tc.desc: test keyboard wakeup and powerkey pressed at the same time
 * @tc.type: FUNC
 */
HWTEST_F(PowerWakeupTest, PowerWakeupTest001, TestSize.Level1)
{
    POWER_HILOGD(LABEL_TEST, "PowerWakeupTest001 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.OverrideScreenOffTime(SCREEN_OFF_TIME_MS);
    powerMgrClient.SuspendDevice(SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    EXPECT_FALSE(powerMgrClient.IsScreenOn());

    auto inputManager = MMI::InputManager::GetInstance();

    std::shared_ptr<MMI::KeyEvent> keyEventPowerkeyDown = MMI::KeyEvent::Create();
    keyEventPowerkeyDown->SetKeyAction(MMI::KeyEvent::KEY_ACTION_DOWN);
    keyEventPowerkeyDown->SetKeyCode(MMI::KeyEvent::KEYCODE_POWER);

    std::shared_ptr<MMI::KeyEvent> keyEventPowerkeyUp = MMI::KeyEvent::Create();
    keyEventPowerkeyUp->SetKeyAction(MMI::KeyEvent::KEY_ACTION_UP);
    keyEventPowerkeyUp->SetKeyCode(MMI::KeyEvent::KEYCODE_POWER);

    std::shared_ptr<MMI::KeyEvent> keyEventKeyboard = MMI::KeyEvent::Create();
    keyEventKeyboard->SetKeyAction(MMI::KeyEvent::KEY_ACTION_DOWN);
    keyEventKeyboard->SetKeyCode(MMI::KeyEvent::KEYCODE_0);

    inputManager->SimulateInputEvent(keyEventPowerkeyDown);
    inputManager->SimulateInputEvent(keyEventPowerkeyUp);
    inputManager->SimulateInputEvent(keyEventKeyboard);
    powerMgrClient.OverrideScreenOffTime(SCREEN_OFF_TIME_MS);
    sleep(2);
    EXPECT_TRUE(powerMgrClient.IsScreenOn());
    powerMgrClient.RestoreScreenOffTime();
    POWER_HILOGD(LABEL_TEST, "PowerWakeupTest001 function end!");
}

/**
 * @tc.name: PowerWakeupTest002
 * @tc.desc: test simulate normal key event when screenoff
 * @tc.type: FUNC
 */
HWTEST_F(PowerWakeupTest, PowerWakeupTest002, TestSize.Level1)
{
    POWER_HILOGD(LABEL_TEST, "PowerWakeupTest002 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.SuspendDevice(SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    EXPECT_FALSE(powerMgrClient.IsScreenOn());

    std::shared_ptr<MMI::KeyEvent> keyEventPowerkeyDown = MMI::KeyEvent::Create();
    keyEventPowerkeyDown->SetKeyAction(MMI::KeyEvent::KEY_ACTION_DOWN);
    keyEventPowerkeyDown->SetKeyCode(MMI::KeyEvent::KEYCODE_0);
    std::shared_ptr<MMI::KeyEvent> keyEventPowerkeyUp = MMI::KeyEvent::Create();
    keyEventPowerkeyUp->SetKeyAction(MMI::KeyEvent::KEY_ACTION_UP);
    keyEventPowerkeyUp->SetKeyCode(MMI::KeyEvent::KEYCODE_0);

    auto inputManager = MMI::InputManager::GetInstance();
    inputManager->SimulateInputEvent(keyEventPowerkeyDown);
    inputManager->SimulateInputEvent(keyEventPowerkeyUp);
    sleep(1);
    EXPECT_TRUE(powerMgrClient.IsScreenOn());

    POWER_HILOGD(LABEL_TEST, "PowerWakeupTest002 function end!");
}
} // namespace
