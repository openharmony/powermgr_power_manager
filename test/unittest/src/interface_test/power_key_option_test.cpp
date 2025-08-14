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
#include "power_key_option_test.h"
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
#include <input_manager.h>
#endif
#include "power_mgr_service.h"

using namespace testing::ext;
using namespace OHOS::MMI;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;
static sptr<PowerMgrService> g_service;
static constexpr int SLEEP_WAIT_TIME_S = 2;

void PowerKeyOptionTest::SetUpTestCase(void)
{
    g_service = DelayedSpSingleton<PowerMgrService>::GetInstance();
    g_service->OnStart();
}

void PowerKeyOptionTest::TearDownTestCase(void)
{
    g_service->OnStop();
    DelayedSpSingleton<PowerMgrService>::DestroyInstance();
}

#ifdef HAS_MULTIMODALINPUT_INPUT_PART
std::function<void(std::shared_ptr<KeyEvent>)> callbackPowerWake_;
std::function<void(std::shared_ptr<KeyEvent>)> callbackPowerSuspend_;
std::function<void(std::shared_ptr<KeyEvent>)> callbackTp_;

int32_t MMI::InputManager::SubscribeKeyEvent(std::shared_ptr<KeyOption> keyOption,
    std::function<void(std::shared_ptr<KeyEvent>)> callback)
{
    POWER_HILOGI(LABEL_TEST, "PowerKeyOptionTest SubscribeKeyEvent");
    if (keyOption->GetFinalKey() == MMI::KeyEvent::KEYCODE_POWER) {
        if (keyOption->IsFinalKeyDown()) {
            callbackPowerWake_ = callback;
        } else {
            callbackPowerSuspend_ = callback;
        }
    } else if (keyOption->GetFinalKey() == MMI::KeyEvent::KEYCODE_SLEEP) {
        callbackTp_ = callback;
    }
    return 0;
}
#endif

namespace {
/**
 * @tc.name: PowerKeyOptionTest001
 * @tc.desc: test simulate powerkey event when screenoff
 * @tc.type: FUNC
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerKeyOptionTest, PowerKeyOptionTest001, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerKeyOptionTest001 function start!");
    GTEST_LOG_(INFO) << "PowerKeyOptionTest001: start";
    g_service->WakeupControllerInit();
    g_service->SuspendControllerInit();
    g_service->SuspendDevice(
        static_cast<int64_t>(time(nullptr)), SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    EXPECT_FALSE(g_service->IsScreenOn());

    std::shared_ptr<MMI::KeyEvent> keyEventPowerkeyDown = MMI::KeyEvent::Create();
    keyEventPowerkeyDown->SetKeyAction(MMI::KeyEvent::KEY_ACTION_DOWN);
    keyEventPowerkeyDown->SetKeyCode(MMI::KeyEvent::KEYCODE_POWER);
    std::shared_ptr<MMI::KeyEvent> keyEventPowerkeyUp = MMI::KeyEvent::Create();
    keyEventPowerkeyUp->SetKeyAction(MMI::KeyEvent::KEY_ACTION_UP);
    keyEventPowerkeyUp->SetKeyCode(MMI::KeyEvent::KEYCODE_POWER);

    EXPECT_NE(callbackPowerWake_, nullptr);
    callbackPowerWake_(keyEventPowerkeyDown);
    callbackPowerWake_(keyEventPowerkeyUp);
    callbackPowerWake_(keyEventPowerkeyDown);
    callbackPowerWake_(keyEventPowerkeyUp);
    EXPECT_TRUE(g_service->IsScreenOn());

    std::shared_ptr<WakeupController> wakeupController = g_service->GetWakeupController();
    std::shared_ptr<SuspendController> suspendController = g_service->GetSuspendController();
    EXPECT_NE(wakeupController, nullptr);
    EXPECT_NE(suspendController, nullptr);
    wakeupController->monitorMap_.clear();
    suspendController->monitorMap_.clear();
    callbackPowerWake_(keyEventPowerkeyDown);
    callbackPowerWake_(keyEventPowerkeyUp);
    callbackPowerWake_(keyEventPowerkeyDown);
    callbackPowerWake_(keyEventPowerkeyUp);
    GTEST_LOG_(INFO) << "PowerKeyOptionTest001: end";
    POWER_HILOGI(LABEL_TEST, "PowerKeyOptionTest001 function end!");
}
#endif

/**
 * @tc.name: PowerKeyOptionTest002
 * @tc.desc: test simulate powerkey event when screenon
 * @tc.type: FUNC
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerKeyOptionTest, PowerKeyOptionTest002, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerKeyOptionTest002 function start!");
    GTEST_LOG_(INFO) << "PowerKeyOptionTest002: start";
    g_service->WakeupControllerInit();
    g_service->SuspendControllerInit();
    g_service->WakeupDevice(
        static_cast<int64_t>(time(nullptr)), WakeupDeviceType::WAKEUP_DEVICE_PLUG_CHANGE, "plug change");
    EXPECT_TRUE(g_service->IsScreenOn());

    std::shared_ptr<MMI::KeyEvent> keyEventPowerkeyDown = MMI::KeyEvent::Create();
    keyEventPowerkeyDown->SetKeyAction(MMI::KeyEvent::KEY_ACTION_DOWN);
    keyEventPowerkeyDown->SetKeyCode(MMI::KeyEvent::KEYCODE_POWER);
    std::shared_ptr<MMI::KeyEvent> keyEventPowerkeyUp = MMI::KeyEvent::Create();
    keyEventPowerkeyUp->SetKeyAction(MMI::KeyEvent::KEY_ACTION_UP);
    keyEventPowerkeyUp->SetKeyCode(MMI::KeyEvent::KEYCODE_POWER);

    EXPECT_NE(callbackPowerSuspend_, nullptr);
    callbackPowerSuspend_(keyEventPowerkeyDown);
    callbackPowerSuspend_(keyEventPowerkeyUp);
    sleep(SLEEP_WAIT_TIME_S);
    EXPECT_FALSE(g_service->IsScreenOn());

    std::shared_ptr<WakeupController> wakeupController = g_service->GetWakeupController();
    std::shared_ptr<SuspendController> suspendController = g_service->GetSuspendController();
    EXPECT_NE(wakeupController, nullptr);
    EXPECT_NE(suspendController, nullptr);
    wakeupController->monitorMap_.clear();
    suspendController->monitorMap_.clear();
    callbackPowerSuspend_(keyEventPowerkeyDown);
    callbackPowerSuspend_(keyEventPowerkeyUp);
    GTEST_LOG_(INFO) << "PowerKeyOptionTest002: end";
    POWER_HILOGI(LABEL_TEST, "PowerKeyOptionTest002 function end!");
}
#endif

/**
 * @tc.name: PowerKeyOptionTest003
 * @tc.desc: test simulate tpcover event when screenon
 * @tc.type: FUNC
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerKeyOptionTest, PowerKeyOptionTest003, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerKeyOptionTest003 function start!");
    GTEST_LOG_(INFO) << "PowerKeyOptionTest003: start";
    g_service->WakeupControllerInit();
    g_service->SuspendControllerInit();
    g_service->WakeupDevice(
        static_cast<int64_t>(time(nullptr)), WakeupDeviceType::WAKEUP_DEVICE_PLUG_CHANGE, "plug change");
    EXPECT_TRUE(g_service->IsScreenOn());

    std::shared_ptr<MMI::KeyEvent> keyEventTPCover = MMI::KeyEvent::Create();
    keyEventTPCover->SetKeyCode(MMI::KeyEvent::KEYCODE_SLEEP);

    EXPECT_NE(callbackTp_, nullptr);
    callbackTp_(keyEventTPCover);
    sleep(SLEEP_WAIT_TIME_S);
    EXPECT_FALSE(g_service->IsScreenOn());

    std::shared_ptr<WakeupController> wakeupController = g_service->GetWakeupController();
    std::shared_ptr<SuspendController> suspendController = g_service->GetSuspendController();
    EXPECT_NE(wakeupController, nullptr);
    EXPECT_NE(suspendController, nullptr);
    wakeupController->monitorMap_.clear();
    suspendController->monitorMap_.clear();
    callbackTp_(keyEventTPCover);
    GTEST_LOG_(INFO) << "PowerKeyOptionTest003: end";
    POWER_HILOGI(LABEL_TEST, "PowerKeyOptionTest003 function end!");
}
#endif
} // namespace