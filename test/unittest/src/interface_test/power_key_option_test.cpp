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
#include "mock_state_action.h"

using namespace testing::ext;
using namespace OHOS::MMI;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;
namespace {
sptr<PowerMgrService> g_service;
DisplayState g_displayState = DisplayState::DISPLAY_ON;
constexpr int SLEEP_WAIT_TIME_US = 500000;
constexpr uint32_t NO_DELAY = 0;
bool g_killProcsee = false;
std::map<SubscriberState, std::shared_ptr<ServiceState>> g_stateMap {
    {SubscriberState::FAILURE, std::make_shared<DeadServiceState>()},
    {SubscriberState::RETRY_SUCCESS, std::make_shared<RestartingServiceState>()},
    {SubscriberState::SUCCESS, std::make_shared<AliveServiceState>()}
};
}

namespace OHOS::PowerMgr {
SubscriberState RequestContext::HandleRequest()
{
    SubscriberState ret = SubscriberState::SUCCESS;
    if (state_) {
        ret = state_->Handle(*this);
    }
    return ret;
}

SubscriberState AliveServiceState::Handle(RequestContext& context)
{
    if (g_killProcsee) {
        context.state_ = g_stateMap[SubscriberState::FAILURE];
        return SubscriberState::FAILURE;
    }
    return SubscriberState::SUCCESS;
}

SubscriberState RestartingServiceState::Handle(RequestContext& context)
{
    constexpr int32_t MAX_RETRY_COUNT = 2;
    if (deathRetryCount_ >= MAX_RETRY_COUNT) {
        context.state_ = g_stateMap[SubscriberState::SUCCESS];
        deathRetryCount_ = 0;
        g_killProcsee = false;
        return SubscriberState::RETRY_SUCCESS;
    }
    deathRetryCount_++;
    return SubscriberState::FAILURE;
}

SubscriberState DeadServiceState::Handle(RequestContext& context)
{
    constexpr int32_t MAX_RETRY_COUNT = 4;
    deathRetryCount_++;
    if (deathRetryCount_ >= MAX_RETRY_COUNT) {
        context.state_ = g_stateMap[SubscriberState::RETRY_SUCCESS];
        deathRetryCount_ = 0;
    }
    return SubscriberState::FAILURE;
}
} // namespace OHOS::PowerMgr

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

void PowerKeyOptionTest::MockDisplayAction()
{
    auto stateMachine = g_service->GetPowerStateMachine();
    ASSERT_TRUE(stateMachine != nullptr) << "MockDisplayAction failed to get PowerStateMachine";
    ::testing::NiceMock<MockStateAction>* stateActionMock = new ::testing::NiceMock<MockStateAction>;
    stateMachine->EnableMock(stateActionMock);
    EXPECT_CALL(*stateActionMock, SetDisplayState(::testing::_, ::testing::_))
        .WillRepeatedly([](DisplayState state, StateChangeReason reason) {
            POWER_HILOGI(LABEL_TEST,
                "PowerKeyOptionTest SetDisplayState state:%{public}d", static_cast<int32_t>(state));
            g_displayState = state;
            return ActionResult::SUCCESS;
        });
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly([]() {
            POWER_HILOGI(LABEL_TEST,
                "PowerKeyOptionTest GetDisplayState state:%{public}d", static_cast<int32_t>(g_displayState));
            return g_displayState;
        });
    ::testing::Mock::AllowLeak(stateActionMock);
}

void PowerKeyOptionTest::SetUp() {}

void PowerKeyOptionTest::TearDown()
{
    g_killProcsee = false;
}

#ifdef HAS_MULTIMODALINPUT_INPUT_PART
std::function<void(std::shared_ptr<KeyEvent>)> callbackPowerWake_ = nullptr;
std::function<void(std::shared_ptr<KeyEvent>)> callbackPowerSuspend_ = nullptr;
std::function<void(std::shared_ptr<KeyEvent>)> callbackTp_ = nullptr;

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
    static RequestContext context(g_stateMap[SubscriberState::SUCCESS]);
    return static_cast<int32_t>(context.HandleRequest());
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
    MockDisplayAction();
    g_service->SuspendDevice(
        static_cast<int64_t>(time(nullptr)), SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    EXPECT_FALSE(g_service->IsScreenOn());
    if (callbackPowerWake_ != nullptr) {
        std::shared_ptr<MMI::KeyEvent> keyEventPowerkeyDown = MMI::KeyEvent::Create();
        keyEventPowerkeyDown->SetKeyAction(MMI::KeyEvent::KEY_ACTION_DOWN);
        keyEventPowerkeyDown->SetKeyCode(MMI::KeyEvent::KEYCODE_POWER);
        std::shared_ptr<MMI::KeyEvent> keyEventPowerkeyUp = MMI::KeyEvent::Create();
        keyEventPowerkeyUp->SetKeyAction(MMI::KeyEvent::KEY_ACTION_UP);
        keyEventPowerkeyUp->SetKeyCode(MMI::KeyEvent::KEYCODE_POWER);

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
    }
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
    MockDisplayAction();
    g_service->WakeupDevice(
        static_cast<int64_t>(time(nullptr)), WakeupDeviceType::WAKEUP_DEVICE_PLUG_CHANGE, "plug change");
    EXPECT_TRUE(g_service->IsScreenOn());
    if (callbackPowerSuspend_ != nullptr) {
        std::shared_ptr<MMI::KeyEvent> keyEventPowerkeyDown = MMI::KeyEvent::Create();
        keyEventPowerkeyDown->SetKeyAction(MMI::KeyEvent::KEY_ACTION_DOWN);
        keyEventPowerkeyDown->SetKeyCode(MMI::KeyEvent::KEYCODE_POWER);
        std::shared_ptr<MMI::KeyEvent> keyEventPowerkeyUp = MMI::KeyEvent::Create();
        keyEventPowerkeyUp->SetKeyAction(MMI::KeyEvent::KEY_ACTION_UP);
        keyEventPowerkeyUp->SetKeyCode(MMI::KeyEvent::KEYCODE_POWER);

        callbackPowerSuspend_(keyEventPowerkeyDown);
        callbackPowerSuspend_(keyEventPowerkeyUp);
        usleep(SLEEP_WAIT_TIME_US);
        EXPECT_FALSE(g_service->IsFoldScreenOn());

        std::shared_ptr<WakeupController> wakeupController = g_service->GetWakeupController();
        std::shared_ptr<SuspendController> suspendController = g_service->GetSuspendController();
        EXPECT_NE(wakeupController, nullptr);
        EXPECT_NE(suspendController, nullptr);
        wakeupController->monitorMap_.clear();
        suspendController->monitorMap_.clear();
        callbackPowerSuspend_(keyEventPowerkeyDown);
        callbackPowerSuspend_(keyEventPowerkeyUp);
    }
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
    MockDisplayAction();
    g_service->WakeupDevice(
        static_cast<int64_t>(time(nullptr)), WakeupDeviceType::WAKEUP_DEVICE_PLUG_CHANGE, "plug change");
    EXPECT_TRUE(g_service->IsScreenOn());
    if (callbackTp_ != nullptr) {
        std::shared_ptr<MMI::KeyEvent> keyEventTPCover = MMI::KeyEvent::Create();
        keyEventTPCover->SetKeyCode(MMI::KeyEvent::KEYCODE_SLEEP);

        callbackTp_(keyEventTPCover);
        usleep(SLEEP_WAIT_TIME_US);
        EXPECT_FALSE(g_service->IsFoldScreenOn());

        std::shared_ptr<WakeupController> wakeupController = g_service->GetWakeupController();
        std::shared_ptr<SuspendController> suspendController = g_service->GetSuspendController();
        EXPECT_NE(wakeupController, nullptr);
        EXPECT_NE(suspendController, nullptr);
        wakeupController->monitorMap_.clear();
        suspendController->monitorMap_.clear();
        callbackTp_(keyEventTPCover);
    }
    GTEST_LOG_(INFO) << "PowerKeyOptionTest003: end";
    POWER_HILOGI(LABEL_TEST, "PowerKeyOptionTest003 function end!");
}
#endif

/**
 * @tc.name: PowerKeyOptionTest004
 * @tc.desc: test PowerkeyWakeupMonitor init retry
 * @tc.type: FUNC
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerKeyOptionTest, PowerKeyOptionTest004, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerKeyOptionTest004 function start!");
    GTEST_LOG_(INFO) << "PowerKeyOptionTest004: start";
    WakeupSource source(
        WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON, true, static_cast<uint32_t>(WakeUpAction::CLICK_DOUBLE));
    auto powerkeyWakeupMonitor = std::make_shared<PowerkeyWakeupMonitor>(source);
    EXPECT_TRUE(powerkeyWakeupMonitor->Init());
    powerkeyWakeupMonitor->Cancel();
    g_killProcsee = true;
    EXPECT_FALSE(powerkeyWakeupMonitor->Init());
    EXPECT_TRUE(powerkeyWakeupMonitor->Init());
    EXPECT_TRUE(powerkeyWakeupMonitor->Init());
    GTEST_LOG_(INFO) << "PowerKeyOptionTest004: end";
    POWER_HILOGI(LABEL_TEST, "PowerKeyOptionTest004 function end!");
}
#endif

/**
 * @tc.name: PowerKeyOptionTest005
 * @tc.desc: test PowerKeySuspendMonitor init retry
 * @tc.type: FUNC
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerKeyOptionTest, PowerKeyOptionTest005, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerKeyOptionTest005 function start!");
    GTEST_LOG_(INFO) << "PowerKeyOptionTest005: start";
    SuspendSource source(SuspendDeviceType::SUSPEND_DEVICE_REASON_POWER_KEY,
        static_cast<uint32_t>(SuspendAction::ACTION_AUTO_SUSPEND), NO_DELAY);
    auto powerkeySuspendMonitor = std::make_shared<PowerKeySuspendMonitor>(source);
    EXPECT_TRUE(powerkeySuspendMonitor->Init());
    powerkeySuspendMonitor->Cancel();
    g_killProcsee = true;
    EXPECT_FALSE(powerkeySuspendMonitor->Init());
    EXPECT_TRUE(powerkeySuspendMonitor->Init());
    EXPECT_TRUE(powerkeySuspendMonitor->Init());
    GTEST_LOG_(INFO) << "PowerKeyOptionTest005: end";
    POWER_HILOGI(LABEL_TEST, "PowerKeyOptionTest005 function end!");
}
#endif
} // namespace