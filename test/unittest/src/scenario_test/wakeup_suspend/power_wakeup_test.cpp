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

#include "power_mgr_service.h"
#include "power_state_machine.h"
#include "setting_helper.h"
#include "json/reader.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;
static sptr<PowerMgrService> g_service;
static constexpr int32_t SLEEP_WAIT_TIME_S = 2;
static constexpr int32_t SLEEP_WAIT_TIME_MS = 400;
static constexpr int32_t DISPLAY_OFF_TIME_MS = 600;
static constexpr int32_t RECOVER_DISPLAY_OFF_TIME_S = 30 * 1000;
static constexpr int32_t DISPLAY_POWER_MANAGER_ID = 3308;
static const std::string TEST_DEVICE_ID = "test_device_id";

void PowerWakeupTest::SetUpTestCase(void)
{
    g_service = DelayedSpSingleton<PowerMgrService>::GetInstance();
    g_service->OnStart();
    g_service->OnAddSystemAbility(DISPLAY_POWER_MANAGER_ID, TEST_DEVICE_ID);
}

void PowerWakeupTest::TearDownTestCase(void)
{
    g_service->OnStop();
    DelayedSpSingleton<PowerMgrService>::DestroyInstance();
}

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
HWTEST_F(PowerWakeupTest, PowerWakeupTest001, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupTest001: start");
    g_service->WakeupControllerInit();
    g_service->SuspendControllerInit();
    g_service->OverrideScreenOffTime(5000);
    g_service->SuspendDevice(
        static_cast<int64_t>(time(nullptr)), SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    EXPECT_FALSE(g_service->IsScreenOn());

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

    inputManager->SimulateInputEvent(keyEventKeyboard);
    inputManager->SimulateInputEvent(keyEventPowerkeyDown);
    inputManager->SimulateInputEvent(keyEventPowerkeyUp);

    sleep(2);
    EXPECT_TRUE(g_service->IsScreenOn());
    g_service->RestoreScreenOffTime();
}

/**
 * @tc.name: PowerWakeupTest002
 * @tc.desc: test simulate normal key event when screenoff
 * @tc.type: FUNC
 */
HWTEST_F(PowerWakeupTest, PowerWakeupTest002, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupTest002: start");

    g_service->WakeupControllerInit();
    g_service->SuspendControllerInit();
    g_service->SuspendDevice(
        static_cast<int64_t>(time(nullptr)), SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    EXPECT_FALSE(g_service->IsScreenOn());

    auto inputManager = MMI::InputManager::GetInstance();
    std::shared_ptr<MMI::KeyEvent> keyEventPowerkeyDown = MMI::KeyEvent::Create();
    keyEventPowerkeyDown->SetKeyAction(MMI::KeyEvent::KEY_ACTION_DOWN);
    keyEventPowerkeyDown->SetKeyCode(MMI::KeyEvent::KEYCODE_0);
    std::shared_ptr<MMI::KeyEvent> keyEventPowerkeyUp = MMI::KeyEvent::Create();
    keyEventPowerkeyUp->SetKeyAction(MMI::KeyEvent::KEY_ACTION_UP);
    keyEventPowerkeyUp->SetKeyCode(MMI::KeyEvent::KEYCODE_0);

    inputManager->SimulateInputEvent(keyEventPowerkeyDown);
    inputManager->SimulateInputEvent(keyEventPowerkeyUp);
    sleep(1);
    EXPECT_TRUE(g_service->IsScreenOn());

    POWER_HILOGI(LABEL_TEST, "PowerWakeupTest002: end");
}
} // namespace
