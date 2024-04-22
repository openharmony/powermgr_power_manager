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
 * @tc.desc: test ExecWakeupMonitorByReason(Normal and exception)
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
HWTEST_F(PowerWakeupTest, PowerWakeupTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerWakeup001: start";
    g_service->WakeupControllerInit();
    g_service->wakeupController_->ExecWakeupMonitorByReason(WakeupDeviceType ::WAKEUP_DEVICE_POWER_BUTTON);
    auto monitor = g_service->wakeupController_->monitorMap_[WakeupDeviceType ::WAKEUP_DEVICE_POWER_BUTTON];
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
    g_service->WakeupControllerInit();
    // test Normal
    g_service->wakeupController_->Wakeup();
    EXPECT_TRUE(g_service->wakeupController_ != nullptr);
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

    g_service->WakeupControllerInit();
    g_service->SuspendControllerInit();

    g_service->SuspendDevice(
        static_cast<int64_t>(time(nullptr)), SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    g_service->wakeupController_->ControlListener(WakeupDeviceType ::WAKEUP_DEVICE_POWER_BUTTON);
    EXPECT_TRUE(g_service->wakeupController_ != nullptr);
    g_service->WakeupDevice(
        static_cast<int64_t>(time(nullptr)), WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON, "PowerWakeupTest003");
    g_service->wakeupController_->ControlListener(WakeupDeviceType ::WAKEUP_DEVICE_POWER_BUTTON);
    EXPECT_TRUE(static_cast<uint32_t>(g_service->wakeupController_->stateMachine_->GetState()) ==
        static_cast<uint32_t>(PowerState::AWAKE));

    g_service->wakeupController_->stateMachine_->EmplaceAwake();
    g_service->wakeupController_->ControlListener(WakeupDeviceType ::WAKEUP_DEVICE_POWER_BUTTON);
    EXPECT_TRUE(static_cast<uint32_t>(g_service->wakeupController_->stateMachine_->GetState()) ==
        static_cast<uint32_t>(PowerState::AWAKE));

    g_service->WakeupDevice(
        static_cast<int64_t>(time(nullptr)), WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON, "PowerWakeupTest003");
    g_service->wakeupController_->ControlListener(WakeupDeviceType ::WAKEUP_DEVICE_POWER_BUTTON);
    EXPECT_TRUE(static_cast<uint32_t>(g_service->wakeupController_->stateMachine_->GetState()) ==
        static_cast<uint32_t>(PowerState::AWAKE));

    g_service->wakeupController_->stateMachine_->SetState(
        PowerState::INACTIVE, StateChangeReason::STATE_CHANGE_REASON_TIMEOUT);
    g_service->suspendController_->stateMachine_->controllerMap_.clear();
    g_service->wakeupController_->ControlListener(WakeupDeviceType ::WAKEUP_DEVICE_POWER_BUTTON);
    EXPECT_TRUE(static_cast<uint32_t>(g_service->wakeupController_->stateMachine_->GetState()) ==
        static_cast<uint32_t>(PowerState::INACTIVE));
    g_service->suspendController_->stateMachine_->InitStateMap();

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
    g_service->WakeupControllerInit();
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

    g_service->WakeupControllerInit();
    g_service->wakeupController_->Cancel();
    EXPECT_TRUE(g_service->wakeupController_->monitorMap_.size() == 0);
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
    g_service->WakeupControllerInit();
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
    EXPECT_TRUE(g_service->wakeupController_ != nullptr);
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

    g_service->WakeupControllerInit();

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
    EXPECT_TRUE(g_service->wakeupController_ != nullptr);
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

    g_service->WakeupControllerInit();

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
    EXPECT_TRUE(g_service->wakeupController_ != nullptr);
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

/**
 * @tc.name: PowerWakeupTest012
 * @tc.desc: test OnInputEvent KeyEvent RefreshActivity
 * @tc.type: FUNC
 */
HWTEST_F(PowerWakeupTest, PowerWakeupTest012, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupTest012: start");
    g_service->WakeupControllerInit();
    g_service->SetDisplayOffTime(DISPLAY_OFF_TIME_MS);
    g_service->WakeupDevice(
        static_cast<int64_t>(time(nullptr)), WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON, "PowerWakeupTest012");
    EXPECT_TRUE(g_service->IsScreenOn());
    usleep(SLEEP_WAIT_TIME_MS * 1000);
    std::shared_ptr<OHOS::MMI::KeyEvent> keyEvent = OHOS::MMI::KeyEvent::Create();
    InputCallback callback;
    callback.OnInputEvent(keyEvent);
    usleep(SLEEP_WAIT_TIME_MS * 1000);
    EXPECT_TRUE(g_service->IsScreenOn());
    g_service->SetDisplayOffTime(RECOVER_DISPLAY_OFF_TIME_S);
    POWER_HILOGI(LABEL_TEST, "PowerWakeupTest012: end");
}

/**
 * @tc.name: PowerWakeupTest013
 * @tc.desc: test OnInputEvent PointerEvent RefreshActivity
 * @tc.type: FUNC
 */
HWTEST_F(PowerWakeupTest, PowerWakeupTest013, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupTest013: start");
    g_service->WakeupControllerInit();
    g_service->SetDisplayOffTime(DISPLAY_OFF_TIME_MS);
    g_service->WakeupDevice(
        static_cast<int64_t>(time(nullptr)), WakeupDeviceType::WAKEUP_DEVICE_APPLICATION, "PowerWakeupTest013");
    EXPECT_TRUE(g_service->IsScreenOn());
    usleep(SLEEP_WAIT_TIME_MS * 1000);
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    InputCallback callback;
    callback.OnInputEvent(pointerEvent);
    usleep(SLEEP_WAIT_TIME_MS * 1000);
    EXPECT_TRUE(g_service->IsScreenOn());
    g_service->SetDisplayOffTime(RECOVER_DISPLAY_OFF_TIME_S);
    POWER_HILOGI(LABEL_TEST, "PowerWakeupTest013: end");
}

/**
 * @tc.name: PowerWakeupTest014
 * @tc.desc: test OnInputEvent AxisEvent RefreshActivity
 * @tc.type: FUNC
 */
HWTEST_F(PowerWakeupTest, PowerWakeupTest014, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupTest014: start");
    g_service->WakeupControllerInit();
    g_service->SetDisplayOffTime(DISPLAY_OFF_TIME_MS);
    g_service->WakeupDevice(
        static_cast<int64_t>(time(nullptr)), WakeupDeviceType::WAKEUP_DEVICE_APPLICATION, "PowerWakeupTest014");
    EXPECT_TRUE(g_service->IsScreenOn());
    usleep(SLEEP_WAIT_TIME_MS * 1000);
    std::shared_ptr<MMI::AxisEvent> axisEvent = MMI::AxisEvent::Create();
    InputCallback callback;
    callback.OnInputEvent(axisEvent);
    usleep(SLEEP_WAIT_TIME_MS * 1000);
    EXPECT_TRUE(g_service->IsScreenOn());
    g_service->SetDisplayOffTime(RECOVER_DISPLAY_OFF_TIME_S);
    POWER_HILOGI(LABEL_TEST, "PowerWakeupTest014: end");
}
} // namespace
