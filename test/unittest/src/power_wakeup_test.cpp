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
#include "power_wakeup_test.h"
#include <fstream>
#include <thread>
#include <unistd.h>

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
static constexpr int SLEEP_WAIT_TIME_S = 2;

class InputCallbackMock : public IInputEventConsumer {
public:
    virtual void OnInputEvent(std::shared_ptr<KeyEvent> keyEvent) const;
    virtual void OnInputEvent(std::shared_ptr<PointerEvent> pointerEvent) const;
    virtual void OnInputEvent(std::shared_ptr<AxisEvent> axisEvent) const;
};

void PowerWakeupTest::SetUpTestCase(void)
{
    g_service = DelayedSpSingleton<PowerMgrService>::GetInstance();
    g_service->OnStart();
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
 * @tc.desc: test ExecWakeupMonitorByReason(Normal and exception)
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
HWTEST_F(PowerWakeupTest, PowerWakeupTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerWakeup001: start";
    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pmsTest_ == nullptr) {
        GTEST_LOG_(INFO) << "PowerWakeupTest001: Failed to get PowerMgrService";
    }

    pmsTest_->Init();
    pmsTest_->WakeupControllerInit();
    pmsTest_->wakeupController_->ExecWakeupMonitorByReason(WakeupDeviceType ::WAKEUP_DEVICE_POWER_BUTTON);
    auto monitor = pmsTest_->wakeupController_->monitorMap_[WakeupDeviceType ::WAKEUP_DEVICE_POWER_BUTTON];
    EXPECT_TRUE(monitor != nullptr);

    GTEST_LOG_(INFO) << "PowerWakeupTest001:  end";
}

/**
 * @tc.name: PowerWakeupTest002
 * @tc.desc: test Wakeup(Normal)
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
HWTEST_F(PowerWakeupTest, PowerWakeupTest002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerWakeupTest002: start";
    sleep(SLEEP_WAIT_TIME_S);
    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pmsTest_ == nullptr) {
        GTEST_LOG_(INFO) << "PowerWakeupTest002: Failed to get PowerMgrService";
    }
    pmsTest_->Init();
    pmsTest_->WakeupControllerInit();
    // test Normal
    pmsTest_->wakeupController_->Wakeup();
    EXPECT_TRUE(pmsTest_->wakeupController_ != nullptr);
    GTEST_LOG_(INFO) << "PowerWakeupTest002:  end";
}

/**
 * @tc.name: PowerWakeupTest003
 * @tc.desc: test ControlListener(Normal)
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
HWTEST_F(PowerWakeupTest, PowerWakeupTest003, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerWakeupTest003: start";

    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pmsTest_ == nullptr) {
        GTEST_LOG_(INFO) << "PowerWakeupTest003: Failed to get PowerMgrService";
    }
    pmsTest_->Init();
    pmsTest_->WakeupControllerInit();

    pmsTest_->wakeupController_->stateMachine_->stateAction_->SetDisplayState(DisplayState::DISPLAY_OFF);
    pmsTest_->wakeupController_->ControlListener(WakeupDeviceType ::WAKEUP_DEVICE_POWER_BUTTON);
    EXPECT_TRUE(pmsTest_->wakeupController_ != nullptr);

    pmsTest_->wakeupController_->stateMachine_->stateAction_->SetDisplayState(DisplayState::DISPLAY_ON);
    pmsTest_->wakeupController_->ControlListener(WakeupDeviceType ::WAKEUP_DEVICE_POWER_BUTTON);
    EXPECT_TRUE(static_cast<uint32_t>(pmsTest_->wakeupController_->stateMachine_->GetState()) ==
        static_cast<uint32_t>(PowerState::AWAKE));

    pmsTest_->wakeupController_->stateMachine_->EmplaceAwake();
    pmsTest_->wakeupController_->ControlListener(WakeupDeviceType ::WAKEUP_DEVICE_POWER_BUTTON);
    EXPECT_TRUE(static_cast<uint32_t>(pmsTest_->wakeupController_->stateMachine_->GetState()) ==
        static_cast<uint32_t>(PowerState::AWAKE));

    pmsTest_->wakeupController_->stateMachine_->SetState(
        PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_TIMEOUT);
    pmsTest_->wakeupController_->ControlListener(WakeupDeviceType ::WAKEUP_DEVICE_POWER_BUTTON);
    EXPECT_TRUE(static_cast<uint32_t>(pmsTest_->wakeupController_->stateMachine_->GetState()) ==
        static_cast<uint32_t>(PowerState::AWAKE));

    pmsTest_->wakeupController_->stateMachine_->SetState(
        PowerState::INACTIVE, StateChangeReason::STATE_CHANGE_REASON_TIMEOUT);
    pmsTest_->suspendController_->stateMachine_->controllerMap_.clear();
    pmsTest_->wakeupController_->ControlListener(WakeupDeviceType ::WAKEUP_DEVICE_POWER_BUTTON);
    EXPECT_TRUE(static_cast<uint32_t>(pmsTest_->wakeupController_->stateMachine_->GetState()) ==
        static_cast<uint32_t>(PowerState::INACTIVE));

    GTEST_LOG_(INFO) << "PowerWakeupTest003:  end";
}

/**
 * @tc.name: PowerWakeupTest004
 * @tc.desc: test GetTargetPath
 * @tc.type: FUNC
 * @tc.require: issueI7G6OY
 */
HWTEST_F(PowerWakeupTest, PowerWakeupTest004, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerWakeupTest004: start";
    std::string targetPath;
    WakeupSourceParser::GetTargetPath(targetPath);
    EXPECT_TRUE(targetPath.size() != 0);
    GTEST_LOG_(INFO) << "PowerWakeupTest004:  end";
}

/**
 * @tc.name: PowerWakeupTest005
 * @tc.desc: test CreateMonitor
 * @tc.type: FUNC
 * @tc.require: issueI7G6OY
 */
HWTEST_F(PowerWakeupTest, PowerWakeupTest005, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerWakeupTest005: start";
    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pmsTest_ == nullptr) {
        GTEST_LOG_(INFO) << "PowerWakeupTest005: Failed to get PowerMgrService";
    }

    pmsTest_->Init();
    pmsTest_->WakeupControllerInit();
    WakeupSource source1(WakeupDeviceType::WAKEUP_DEVICE_SINGLE_CLICK, 1, 0);
    std::shared_ptr<WakeupMonitor> monitor1 = WakeupMonitor::CreateMonitor(source1);
    EXPECT_TRUE(monitor1 != nullptr);

    WakeupSource source2(WakeupDeviceType::WAKEUP_DEVICE_MAX, 1, 0);
    std::shared_ptr<WakeupMonitor> monitor2 = WakeupMonitor::CreateMonitor(source2);
    EXPECT_TRUE(static_cast<uint32_t>(source2.reason_) == static_cast<uint32_t>(WakeupDeviceType::WAKEUP_DEVICE_MAX));
    GTEST_LOG_(INFO) << "PowerWakeupTest005:  end";
}

/**
 * @tc.name: PowerWakeupTest006
 * @tc.desc: test Cancel(Normal and exception)
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
HWTEST_F(PowerWakeupTest, PowerWakeupTest006, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerWakeupTest006: start";

    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pmsTest_ == nullptr) {
        GTEST_LOG_(INFO) << "PowerWakeupTest006: Failed to get PowerMgrService";
    }
    pmsTest_->Init();
    pmsTest_->WakeupControllerInit();
    pmsTest_->wakeupController_->Cancel();
    EXPECT_TRUE(pmsTest_->wakeupController_->monitorMap_.size() == 0);
    GTEST_LOG_(INFO) << "PowerWakeupTest006:  end";
}

/**
 * @tc.name: PowerWakeupTest007
 * @tc.desc: test OnInputEvent(Normal)
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
HWTEST_F(PowerWakeupTest, PowerWakeupTest007, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerWakeupTest007: start";

    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pmsTest_ == nullptr) {
        GTEST_LOG_(INFO) << "PowerWakeupTest007: Failed to get PowerMgrService";
    }
    pmsTest_->Init();
    pmsTest_->WakeupControllerInit();
    InputCallback* callback = new InputCallback();
    InputCallbackMock* callback_mock = reinterpret_cast<InputCallbackMock*>(callback);
    std::shared_ptr<OHOS::MMI::KeyEvent> keyEvent = OHOS::MMI::KeyEvent::Create();
    keyEvent->SetKeyAction(MMI::KeyEvent::KEY_ACTION_DOWN);

    keyEvent->SetKeyCode(OHOS::MMI::KeyEvent::KEYCODE_F1);
    callback_mock->OnInputEvent(keyEvent);
    keyEvent->SetKeyCode(OHOS::MMI::KeyEvent::KEYCODE_0);
    callback_mock->OnInputEvent(keyEvent);
    keyEvent->SetKeyCode(OHOS::MMI::KeyEvent::KEYCODE_F2);
    callback_mock->OnInputEvent(keyEvent);
    delete callback;
    EXPECT_TRUE(pmsTest_->wakeupController_ != nullptr);
    GTEST_LOG_(INFO) << "PowerWakeupTest007:  end";
}

/**
 * @tc.name: PowerWakeupTest008
 * @tc.desc: test OnInputEvent(Normal 1)
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
HWTEST_F(PowerWakeupTest, PowerWakeupTest008, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerWakeupTest008: start";

    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pmsTest_ == nullptr) {
        GTEST_LOG_(INFO) << "PowerWakeupTest008: Failed to get PowerMgrService";
    }
    pmsTest_->Init();
    pmsTest_->WakeupControllerInit();

    constexpr int32_t DRAG_DST_X {500};
    constexpr int32_t DRAG_DST_Y {500};
    int32_t deviceMouseId {0};

    InputCallback* callback = new InputCallback();
    InputCallbackMock* callback_mock = reinterpret_cast<InputCallbackMock*>(callback);
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    MMI::PointerEvent::PointerItem curPointerItem;
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_DOWN);
    pointerEvent->SetPointerId(0);

    pointerEvent->SetSourceType(PointerEvent::SOURCE_TYPE_MOUSE);
    curPointerItem = CreatePointerItem(0, deviceMouseId, {DRAG_DST_X, DRAG_DST_Y}, true);
    pointerEvent->AddPointerItem(curPointerItem);
    callback_mock->OnInputEvent(pointerEvent);

    pointerEvent->SetSourceType(PointerEvent::SOURCE_TYPE_TOUCHPAD);
    curPointerItem = CreatePointerItem(0, deviceMouseId, {DRAG_DST_X, DRAG_DST_Y}, true);
    pointerEvent->AddPointerItem(curPointerItem);
    callback_mock->OnInputEvent(pointerEvent);

    pointerEvent->SetSourceType(PointerEvent::TOOL_TYPE_PEN);
    curPointerItem = CreatePointerItem(0, deviceMouseId, {DRAG_DST_X, DRAG_DST_Y}, true);
    pointerEvent->AddPointerItem(curPointerItem);
    callback_mock->OnInputEvent(pointerEvent);
    EXPECT_TRUE(pmsTest_->wakeupController_ != nullptr);
    delete callback;
}

/**
 * @tc.name: PowerWakeupTest009
 * @tc.desc: test OnInputEvent(Normal 2)
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
HWTEST_F(PowerWakeupTest, PowerWakeupTest009, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerWakeupTest009: start";

    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pmsTest_ == nullptr) {
        GTEST_LOG_(INFO) << "PowerWakeupTest009: Failed to get PowerMgrService";
    }
    pmsTest_->Init();
    pmsTest_->WakeupControllerInit();

    constexpr int32_t DRAG_DST_X {500};
    constexpr int32_t DRAG_DST_Y {500};
    int32_t deviceMouseId {0};

    InputCallback* callback = new InputCallback();
    InputCallbackMock* callback_mock = reinterpret_cast<InputCallbackMock*>(callback);
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    MMI::PointerEvent::PointerItem curPointerItem;
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_DOWN);
    pointerEvent->SetPointerId(0);

    pointerEvent->SetSourceType(PointerEvent::SOURCE_TYPE_TOUCHSCREEN);
    curPointerItem = CreatePointerItem(0, deviceMouseId, {DRAG_DST_X, DRAG_DST_Y}, true);
    pointerEvent->AddPointerItem(curPointerItem);
    callback_mock->OnInputEvent(pointerEvent);

    pointerEvent->SetSourceType(PointerEvent::SOURCE_TYPE_TOUCHSCREEN);
    curPointerItem = CreatePointerItem(0, deviceMouseId, {DRAG_DST_X, DRAG_DST_Y}, true);
    pointerEvent->AddPointerItem(curPointerItem);
    callback_mock->OnInputEvent(pointerEvent);

    std::shared_ptr<MMI::PointerEvent> pointerEvent2 = MMI::PointerEvent::Create();
    pointerEvent2->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_DOWN);
    pointerEvent2->SetPointerId(0);
    curPointerItem = CreatePointerItem(1, PointerEvent::TOOL_TYPE_PEN, {DRAG_DST_X, DRAG_DST_Y}, true);
    curPointerItem.SetToolType(PointerEvent::TOOL_TYPE_PEN);
    pointerEvent2->AddPointerItem(curPointerItem);
    callback_mock->OnInputEvent(pointerEvent2);

    curPointerItem = CreatePointerItem(0, PointerEvent::TOOL_TYPE_PEN, {DRAG_DST_X, DRAG_DST_Y}, true);
    curPointerItem.SetToolType(PointerEvent::TOOL_TYPE_PEN);
    pointerEvent2->AddPointerItem(curPointerItem);
    callback_mock->OnInputEvent(pointerEvent2);

    delete callback;
    EXPECT_TRUE(pmsTest_->wakeupController_ != nullptr);
    GTEST_LOG_(INFO) << "PowerWakeupTest009:  end";
}

/**
 * @tc.name: PowerWakeupTest010
 * @tc.desc: test getSourceKeys
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
HWTEST_F(PowerWakeupTest, PowerWakeupTest010, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerWakeupTest010: start";

    std::shared_ptr<WakeupSources> sources = WakeupSourceParser::ParseSources();
    std::vector<std::string> tmp = sources->getSourceKeys();
    EXPECT_TRUE(tmp.size() != 0);
    GTEST_LOG_(INFO) << "PowerWakeupTest010:  end";
}

/**
 * @tc.name: PowerWakeupTest011
 * @tc.desc: test ParseSourcesProc(exception)
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
HWTEST_F(PowerWakeupTest, PowerWakeupTest011, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerWakeupTest011: start";

    static const std::string jsonStr =
        "{\"powerkey\": {\"enable\": false},\"keyborad\": {\"enable\": false},\"mouse\": {\"enable\": "
        "false},\"touchscreen\": {\"enable\": false,\"click\": 2},\"touchpad\": {\"enable\": false},\"pen\": "
        "{\"enable\": "
        "false},\"lid\": {\"enable\": false},\"switch\": {\"enable\": true},\"xxx\": {\"enable\": false}}";

    std::shared_ptr<WakeupSources> parseSources = std::make_shared<WakeupSources>();
    Json::Reader reader;
    Json::Value root;

    if (!reader.parse(jsonStr.data(), jsonStr.data() + jsonStr.size(), root)) {
        GTEST_LOG_(INFO) << "PowerWakeupTest011: json parse error";
    }

    Json::Value::Members members = root.getMemberNames();
    for (auto iter = members.begin(); iter != members.end(); iter++) {
        std::string key = *iter;
        Json::Value valueObj = root[key];
        WakeupSourceParser::ParseSourcesProc(parseSources, valueObj, key);
    }
    EXPECT_TRUE(parseSources->GetSourceList().size() != 0);
    GTEST_LOG_(INFO) << "PowerWakeupTest011:  end";
}
} // namespace