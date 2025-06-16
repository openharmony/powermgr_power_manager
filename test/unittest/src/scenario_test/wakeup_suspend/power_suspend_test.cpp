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

#include "power_mgr_service.h"
#include "power_mgr_client.h"
#include "power_state_callback_stub.h"
#include "power_state_machine.h"
#include "setting_helper.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;
static sptr<PowerMgrService> g_service;
static constexpr int SLEEP_WAIT_TIME_S = 2;
static constexpr size_t WAKEUP_WAIT_TIME = 500;
static constexpr size_t USECPERMSEC = 1000;
static constexpr int32_t NEXT_WAIT_TIME_S = 1;

void PowerSuspendTest::SetUpTestCase(void)
{
    g_service = DelayedSpSingleton<PowerMgrService>::GetInstance();
    g_service->OnStart();
    g_service->WakeupControllerInit();
    g_service->SuspendControllerInit();
}

void PowerSuspendTest::TearDownTestCase(void)
{
    g_service->OnStop();
    DelayedSpSingleton<PowerMgrService>::DestroyInstance();
}

namespace {
/**
 * @tc.name: PowerSuspendTest001
 * @tc.desc: test simulate powerkey event when screenon
 * @tc.type: FUNC
 */
HWTEST_F(PowerSuspendTest, PowerSuspendTest001, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerSuspendTest001: start");
    g_service->WakeupDevice(
        static_cast<int64_t>(time(nullptr)), WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON, "PowerSuspendTest001");
    EXPECT_TRUE(g_service->IsScreenOn());

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
    sleep(2);
    // the second powerkey event would interrupt the transition to INACTIVE
    EXPECT_TRUE(g_service->IsScreenOn());
    POWER_HILOGI(LABEL_TEST, "PowerSuspendTest001: end");
}
/**
 * @tc.name: PowerSuspendTest002
 * @tc.desc: test simulate powerkey event once when screenon
 * @tc.type: FUNC
 */
HWTEST_F(PowerSuspendTest, PowerSuspendTest002, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerSuspendTest002: start");
    g_service->WakeupDevice(
        static_cast<int64_t>(time(nullptr)), WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON, "PowerSuspendTest002");
    EXPECT_TRUE(g_service->IsScreenOn());

    
    std::shared_ptr<MMI::KeyEvent> keyEventPowerkeyDown = MMI::KeyEvent::Create();
    keyEventPowerkeyDown->SetKeyAction(MMI::KeyEvent::KEY_ACTION_DOWN);
    keyEventPowerkeyDown->SetKeyCode(MMI::KeyEvent::KEYCODE_POWER);
    std::shared_ptr<MMI::KeyEvent> keyEventPowerkeyUp = MMI::KeyEvent::Create();
    keyEventPowerkeyUp->SetKeyAction(MMI::KeyEvent::KEY_ACTION_UP);
    keyEventPowerkeyUp->SetKeyCode(MMI::KeyEvent::KEYCODE_POWER);

    auto inputManager = MMI::InputManager::GetInstance();
    inputManager->SimulateInputEvent(keyEventPowerkeyDown);
    inputManager->SimulateInputEvent(keyEventPowerkeyUp);
    sleep(2);
    EXPECT_FALSE(g_service->IsScreenOn());
    POWER_HILOGI(LABEL_TEST, "PowerSuspendTest002: end");
}

/**
 * @tc.name: PowerSuspendTest003
 * @tc.desc: test IsForceSleeping
 * @tc.type: FUNC
 * @tc.require: issueICE3O4
 */
HWTEST_F(PowerSuspendTest, PowerSuspendTest003, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerSuspendTest003 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    int32_t wakeupReason = (static_cast<int32_t>(WakeupDeviceType::WAKEUP_DEVICE_MAX)) + 1;
    WakeupDeviceType abnormaltype = WakeupDeviceType(wakeupReason);
    powerMgrClient.WakeupDevice();
    sleep(NEXT_WAIT_TIME_S);
    EXPECT_EQ(powerMgrClient.IsForceSleeping(), false);
    powerMgrClient.SuspendDevice();
    sleep(NEXT_WAIT_TIME_S);
    EXPECT_EQ(powerMgrClient.IsForceSleeping(), false);

    powerMgrClient.WakeupDevice();
    sleep(NEXT_WAIT_TIME_S);
    EXPECT_EQ(powerMgrClient.IsForceSleeping(), false);
    powerMgrClient.ForceSuspendDevice();
    sleep(NEXT_WAIT_TIME_S);
#ifdef POWER_MANAGER_ENABLE_FORCE_SLEEP_BROADCAST
    EXPECT_EQ(powerMgrClient.IsForceSleeping(), true);
#else
    EXPECT_EQ(powerMgrClient.IsForceSleeping(), false);
#endif
    powerMgrClient.WakeupDevice(abnormaltype);
    sleep(NEXT_WAIT_TIME_S);
#ifdef POWER_MANAGER_ENABLE_FORCE_SLEEP_BROADCAST
    EXPECT_EQ(powerMgrClient.IsForceSleeping(), true);
#else
    EXPECT_EQ(powerMgrClient.IsForceSleeping(), false);
#endif
    powerMgrClient.WakeupDevice();
    sleep(NEXT_WAIT_TIME_S);
    EXPECT_EQ(powerMgrClient.IsForceSleeping(), false);
    POWER_HILOGI(LABEL_TEST, "PowerSuspendTest003 function end!");
}
} // namespace