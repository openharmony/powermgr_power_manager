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
#include "power_wakeup_controller_test.h"
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

void PowerWakeupControllerTest::SetUpTestCase(void)
{
    PowerMgrClient::GetInstance().SuspendDevice();
    EXPECT_FALSE(PowerMgrClient::GetInstance().IsScreenOn());
    g_service = DelayedSpSingleton<PowerMgrService>::GetInstance();
    g_service->OnStart();
    g_service->OnAddSystemAbility(DISPLAY_POWER_MANAGER_ID, TEST_DEVICE_ID);
}

void PowerWakeupControllerTest::TearDownTestCase(void)
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
 * @tc.name: PowerWakeupControllerTest001
 * @tc.desc: test ExecWakeupMonitorByReason(Normal and exception)
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
HWTEST_F(PowerWakeupControllerTest, PowerWakeupControllerTest001, TestSize.Level0)
{
    POWER_HILOGD(LABEL_TEST, "PowerWakeupControllerTest001 start");
    GTEST_LOG_(INFO) << "PowerWakeup001: start";
    g_service->WakeupControllerInit();
    g_service->wakeupController_->ExecWakeupMonitorByReason(WakeupDeviceType ::WAKEUP_DEVICE_POWER_BUTTON);
    auto monitor = g_service->wakeupController_->monitorMap_[WakeupDeviceType ::WAKEUP_DEVICE_POWER_BUTTON];
    EXPECT_TRUE(monitor != nullptr);

    GTEST_LOG_(INFO) << "PowerWakeupControllerTest001:  end";
    POWER_HILOGD(LABEL_TEST, "PowerWakeupControllerTest001 end");
}

/**
 * @tc.name: PowerWakeupControllerTest002
 * @tc.desc: test Wakeup(Normal)
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
HWTEST_F(PowerWakeupControllerTest, PowerWakeupControllerTest002, TestSize.Level0)
{
    POWER_HILOGD(LABEL_TEST, "PowerWakeupControllerTest002 start");
    GTEST_LOG_(INFO) << "PowerWakeupControllerTest002: start";
    sleep(SLEEP_WAIT_TIME_S);
    g_service->WakeupControllerInit();
    // test Normal
    g_service->wakeupController_->Wakeup();
    EXPECT_TRUE(g_service->wakeupController_ != nullptr);
    GTEST_LOG_(INFO) << "PowerWakeupControllerTest002:  end";
    POWER_HILOGD(LABEL_TEST, "PowerWakeupControllerTest002 end");
}

/**
 * @tc.name: PowerWakeupControllerTest003
 * @tc.desc: test ControlListener(Normal)
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
HWTEST_F(PowerWakeupControllerTest, PowerWakeupControllerTest003, TestSize.Level0)
{
    POWER_HILOGD(LABEL_TEST, "PowerWakeupControllerTest003 start");
    GTEST_LOG_(INFO) << "PowerWakeupControllerTest003: start";

    g_service->WakeupControllerInit();
    g_service->SuspendControllerInit();

    g_service->SuspendDevice(
        static_cast<int64_t>(time(nullptr)), SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    g_service->wakeupController_->ControlListener(WakeupDeviceType ::WAKEUP_DEVICE_POWER_BUTTON);
    EXPECT_TRUE(g_service->wakeupController_ != nullptr);
    g_service->WakeupDevice(static_cast<int64_t>(time(nullptr)),
        WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON, "PowerWakeupControllerTest003");
    g_service->wakeupController_->ControlListener(WakeupDeviceType ::WAKEUP_DEVICE_POWER_BUTTON);
    EXPECT_TRUE(static_cast<uint32_t>(g_service->wakeupController_->stateMachine_->GetState()) ==
        static_cast<uint32_t>(PowerState::AWAKE));

    g_service->wakeupController_->stateMachine_->EmplaceAwake();
    g_service->wakeupController_->ControlListener(WakeupDeviceType ::WAKEUP_DEVICE_POWER_BUTTON);
    EXPECT_TRUE(static_cast<uint32_t>(g_service->wakeupController_->stateMachine_->GetState()) ==
        static_cast<uint32_t>(PowerState::AWAKE));

    g_service->WakeupDevice(static_cast<int64_t>(time(nullptr)),
        WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON, "PowerWakeupControllerTest003");
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

    GTEST_LOG_(INFO) << "PowerWakeupControllerTest003:  end";
    POWER_HILOGD(LABEL_TEST, "PowerWakeupControllerTest003 end");
}

/**
 * @tc.name: PowerWakeupControllerTest004
 * @tc.desc: test GetTargetPath
 * @tc.type: FUNC
 * @tc.require: issueI7G6OY
 */
HWTEST_F(PowerWakeupControllerTest, PowerWakeupControllerTest004, TestSize.Level0)
{
    POWER_HILOGD(LABEL_TEST, "PowerWakeupControllerTest004 start");
    GTEST_LOG_(INFO) << "PowerWakeupControllerTest004: start";
    std::string targetPath;
    WakeupSourceParser::GetTargetPath(targetPath);
    EXPECT_TRUE(targetPath.size() != 0);
    GTEST_LOG_(INFO) << "PowerWakeupControllerTest004:  end";
    POWER_HILOGD(LABEL_TEST, "PowerWakeupControllerTest004 end");
}

/**
 * @tc.name: PowerWakeupControllerTest005
 * @tc.desc: test CreateMonitor
 * @tc.type: FUNC
 * @tc.require: issueI7G6OY
 */
HWTEST_F(PowerWakeupControllerTest, PowerWakeupControllerTest005, TestSize.Level0)
{
    POWER_HILOGD(LABEL_TEST, "PowerWakeupControllerTest005 start");
    GTEST_LOG_(INFO) << "PowerWakeupControllerTest005: start";
    g_service->WakeupControllerInit();
    WakeupSource source1(WakeupDeviceType::WAKEUP_DEVICE_SINGLE_CLICK, 1, 0);
    std::shared_ptr<WakeupMonitor> monitor1 = WakeupMonitor::CreateMonitor(source1);
    EXPECT_TRUE(monitor1 != nullptr);

    WakeupSource source2(WakeupDeviceType::WAKEUP_DEVICE_MAX, 1, 0);
    std::shared_ptr<WakeupMonitor> monitor2 = WakeupMonitor::CreateMonitor(source2);
    EXPECT_TRUE(static_cast<uint32_t>(source2.reason_) == static_cast<uint32_t>(WakeupDeviceType::WAKEUP_DEVICE_MAX));
    GTEST_LOG_(INFO) << "PowerWakeupControllerTest005:  end";
    POWER_HILOGD(LABEL_TEST, "PowerWakeupControllerTest005 end");
}

/**
 * @tc.name: PowerWakeupControllerTest006
 * @tc.desc: test Cancel(Normal and exception)
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
HWTEST_F(PowerWakeupControllerTest, PowerWakeupControllerTest006, TestSize.Level0)
{
    POWER_HILOGD(LABEL_TEST, "PowerWakeupControllerTest006 start");
    GTEST_LOG_(INFO) << "PowerWakeupControllerTest006: start";

    g_service->WakeupControllerInit();
    g_service->wakeupController_->Cancel();
    EXPECT_TRUE(g_service->wakeupController_->monitorMap_.size() == 0);
    GTEST_LOG_(INFO) << "PowerWakeupControllerTest006:  end";
    POWER_HILOGD(LABEL_TEST, "PowerWakeupControllerTest006 end");
}

/**
 * @tc.name: PowerWakeupControllerTest007
 * @tc.desc: test OnInputEvent(Normal)
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
HWTEST_F(PowerWakeupControllerTest, PowerWakeupControllerTest007, TestSize.Level0)
{
    POWER_HILOGD(LABEL_TEST, "PowerWakeupControllerTest007 start");
    GTEST_LOG_(INFO) << "PowerWakeupControllerTest007: start";
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
    GTEST_LOG_(INFO) << "PowerWakeupControllerTest007:  end";
    POWER_HILOGD(LABEL_TEST, "PowerWakeupControllerTest007 end");
}

/**
 * @tc.name: PowerWakeupControllerTest008
 * @tc.desc: test OnInputEvent(Normal 1)
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
HWTEST_F(PowerWakeupControllerTest, PowerWakeupControllerTest008, TestSize.Level0)
{
    POWER_HILOGD(LABEL_TEST, "PowerWakeupControllerTest008 start");
    GTEST_LOG_(INFO) << "PowerWakeupControllerTest008: start";

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
    POWER_HILOGD(LABEL_TEST, "PowerWakeupControllerTest008 end");
}

/**
 * @tc.name: PowerWakeupControllerTest009
 * @tc.desc: test OnInputEvent(Normal 2)
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
HWTEST_F(PowerWakeupControllerTest, PowerWakeupControllerTest009, TestSize.Level0)
{
    POWER_HILOGD(LABEL_TEST, "PowerWakeupControllerTest009 start");
    GTEST_LOG_(INFO) << "PowerWakeupControllerTest009: start";

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
    GTEST_LOG_(INFO) << "PowerWakeupControllerTest009:  end";
    POWER_HILOGD(LABEL_TEST, "PowerWakeupControllerTest009 end");
}

/**
 * @tc.name: PowerWakeupControllerTest010
 * @tc.desc: test getSourceKeys
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
HWTEST_F(PowerWakeupControllerTest, PowerWakeupControllerTest010, TestSize.Level0)
{
    POWER_HILOGD(LABEL_TEST, "PowerWakeupControllerTest010 start");
    GTEST_LOG_(INFO) << "PowerWakeupControllerTest010: start";

    std::shared_ptr<WakeupSources> sources = WakeupSourceParser::ParseSources();
    std::vector<std::string> tmp = sources->getSourceKeys();
    EXPECT_TRUE(tmp.size() != 0);
    GTEST_LOG_(INFO) << "PowerWakeupControllerTest010:  end";
    POWER_HILOGD(LABEL_TEST, "PowerWakeupControllerTest010 end");
}

/**
 * @tc.name: PowerWakeupControllerTest011
 * @tc.desc: test ParseSourcesProc(exception)
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
HWTEST_F(PowerWakeupControllerTest, PowerWakeupControllerTest011, TestSize.Level0)
{
    POWER_HILOGD(LABEL_TEST, "PowerWakeupControllerTest011 start");
    GTEST_LOG_(INFO) << "PowerWakeupControllerTest011: start";

    static const std::string jsonStr =
        "{\"powerkey\": {\"enable\": false},\"keyborad\": {\"enable\": false},\"mouse\": {\"enable\": "
        "false},\"touchscreen\": {\"enable\": false,\"click\": 2},\"touchpad\": {\"enable\": false},\"pen\": "
        "{\"enable\": "
        "false},\"lid\": {\"enable\": false},\"switch\": {\"enable\": true},\"xxx\": {\"enable\": false}}";

    std::shared_ptr<WakeupSources> parseSources = std::make_shared<WakeupSources>();
    Json::Reader reader;
    Json::Value root;

    if (!reader.parse(jsonStr.data(), jsonStr.data() + jsonStr.size(), root)) {
        GTEST_LOG_(INFO) << "PowerWakeupControllerTest011: json parse error";
    }

    Json::Value::Members members = root.getMemberNames();
    for (auto iter = members.begin(); iter != members.end(); iter++) {
        std::string key = *iter;
        Json::Value valueObj = root[key];
        WakeupSourceParser::ParseSourcesProc(parseSources, valueObj, key);
    }
    EXPECT_TRUE(parseSources->GetSourceList().size() != 0);
    GTEST_LOG_(INFO) << "PowerWakeupControllerTest011:  end";
    POWER_HILOGD(LABEL_TEST, "PowerWakeupControllerTest011 end");
}

/**
 * @tc.name: PowerWakeupControllerTest012
 * @tc.desc: test OnInputEvent KeyEvent RefreshActivity
 * @tc.type: FUNC
 */
HWTEST_F(PowerWakeupControllerTest, PowerWakeupControllerTest012, TestSize.Level0)
{
    POWER_HILOGD(LABEL_TEST, "PowerWakeupControllerTest012: start");
    g_service->WakeupControllerInit();
    g_service->SetDisplayOffTime(DISPLAY_OFF_TIME_MS);
    g_service->WakeupDevice(static_cast<int64_t>(time(nullptr)),
        WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON, "PowerWakeupControllerTest012");
    EXPECT_TRUE(g_service->IsScreenOn());
    usleep(SLEEP_WAIT_TIME_MS * 1000);
    std::shared_ptr<OHOS::MMI::KeyEvent> keyEvent = OHOS::MMI::KeyEvent::Create();
    InputCallback callback;
    callback.OnInputEvent(keyEvent);
    usleep(SLEEP_WAIT_TIME_MS * 1000);
    EXPECT_TRUE(g_service->IsScreenOn());
    g_service->SetDisplayOffTime(RECOVER_DISPLAY_OFF_TIME_S);
    POWER_HILOGD(LABEL_TEST, "PowerWakeupControllerTest012: end");
}

/**
 * @tc.name: PowerWakeupControllerTest013
 * @tc.desc: test OnInputEvent PointerEvent RefreshActivity
 * @tc.type: FUNC
 */
HWTEST_F(PowerWakeupControllerTest, PowerWakeupControllerTest013, TestSize.Level0)
{
    POWER_HILOGD(LABEL_TEST, "PowerWakeupControllerTest013: start");
    g_service->WakeupControllerInit();
    g_service->SetDisplayOffTime(DISPLAY_OFF_TIME_MS);
    g_service->WakeupDevice(static_cast<int64_t>(time(nullptr)),
        WakeupDeviceType::WAKEUP_DEVICE_APPLICATION, "PowerWakeupControllerTest013");
    EXPECT_TRUE(g_service->IsScreenOn());
    usleep(SLEEP_WAIT_TIME_MS * 1000);
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    InputCallback callback;
    callback.OnInputEvent(pointerEvent);
    usleep(SLEEP_WAIT_TIME_MS * 1000);
    EXPECT_TRUE(g_service->IsScreenOn());
    g_service->SetDisplayOffTime(RECOVER_DISPLAY_OFF_TIME_S);
    POWER_HILOGD(LABEL_TEST, "PowerWakeupControllerTest013: end");
}

/**
 * @tc.name: PowerWakeupControllerTest014
 * @tc.desc: test OnInputEvent AxisEvent RefreshActivity
 * @tc.type: FUNC
 */
HWTEST_F(PowerWakeupControllerTest, PowerWakeupControllerTest014, TestSize.Level0)
{
    POWER_HILOGD(LABEL_TEST, "PowerWakeupControllerTest014: start");
    g_service->WakeupControllerInit();
    g_service->SetDisplayOffTime(DISPLAY_OFF_TIME_MS);
    g_service->WakeupDevice(static_cast<int64_t>(time(nullptr)),
        WakeupDeviceType::WAKEUP_DEVICE_APPLICATION, "PowerWakeupControllerTest014");
    EXPECT_TRUE(g_service->IsScreenOn());
    usleep(SLEEP_WAIT_TIME_MS * 1000);
    std::shared_ptr<MMI::AxisEvent> axisEvent = MMI::AxisEvent::Create();
    InputCallback callback;
    callback.OnInputEvent(axisEvent);
    usleep(SLEEP_WAIT_TIME_MS * 1000);
    EXPECT_TRUE(g_service->IsScreenOn());
    g_service->SetDisplayOffTime(RECOVER_DISPLAY_OFF_TIME_S);
    POWER_HILOGD(LABEL_TEST, "PowerWakeupControllerTest014: end");
}

#ifdef POWER_DOUBLECLICK_ENABLE
/**
 * @tc.name: PowerWakeupControllerTest015
 * @tc.desc: test switch to turn on the screen by double click
 * @tc.type: FUNC
 */
HWTEST_F(PowerWakeupControllerTest, PowerWakeupControllerTest015, TestSize.Level0)
{
    POWER_HILOGD(LABEL_TEST, "PowerWakeupControllerTest015: start");
    g_service->WakeupControllerInit();
    auto wakeupController_ = g_service->GetWakeupController();
    EXPECT_TRUE(wakeupController_ != nullptr);
    wakeupController_->ChangeWakeupSourceConfig(true);
    auto resCode = wakeupController_->SetWakeupDoubleClickSensor(true);
    EXPECT_TRUE(resCode != -1);
    POWER_HILOGD(LABEL_TEST, "PowerWakeupControllerTest015: end");
}
#endif
#ifdef POWER_PICKUP_ENABLE
/**
 * @tc.name: PowerWakeupControllerTest016
 * @tc.desc: test switch to turn on the screen by raising your hand
 * @tc.type: FUNC
 */
HWTEST_F(PowerWakeupControllerTest, PowerWakeupControllerTest016, TestSize.Level0)
{
    POWER_HILOGD(LABEL_TEST, "PowerWakeupControllerTest016: start");
    g_service->WakeupControllerInit();
    auto wakeupController_ = g_service->GetWakeupController();
    EXPECT_TRUE(wakeupController_ != nullptr);
    wakeupController_->ChangePickupWakeupSourceConfig(true);
    wakeupController_->PickupConnectMotionConfig(true);
    POWER_HILOGD(LABEL_TEST, "PowerWakeupControllerTest016: end");
}
#endif

/**
 * @tc.name: PowerWakeupControllerTest017
 * @tc.desc: test ExecWakeupMonitorByReason(WAKEUP_DEVICE_PEN)
 * @tc.type: FUNC
 * @tc.require: issueI9V16C
 */
HWTEST_F(PowerWakeupControllerTest, PowerWakeupControllerTest017, TestSize.Level0)
{
    POWER_HILOGD(LABEL_TEST, "PowerWakeupControllerTest017 start");
    GTEST_LOG_(INFO) << "PowerWakeupControllerTest017: start";
    g_service->WakeupControllerInit();
    g_service->wakeupController_->ExecWakeupMonitorByReason(WakeupDeviceType ::WAKEUP_DEVICE_PEN);
    auto monitor = g_service->wakeupController_->monitorMap_[WakeupDeviceType ::WAKEUP_DEVICE_PEN];
    EXPECT_TRUE(monitor != nullptr);

    GTEST_LOG_(INFO) << "PowerWakeupControllerTest017:  end";
    POWER_HILOGD(LABEL_TEST, "PowerWakeupControllerTest017 end");
}

/**
 * @tc.name: PowerWakeupControllerTest018
 * @tc.desc: test ExecWakeupMonitorByReason(WAKEUP_DEVICE_DOUBLE_CLICK)
 * @tc.type: FUNC
 * @tc.require: issueI9V16C
 */
HWTEST_F(PowerWakeupControllerTest, PowerWakeupControllerTest018, TestSize.Level0)
{
    POWER_HILOGD(LABEL_TEST, "PowerWakeupControllerTest018 start");
    GTEST_LOG_(INFO) << "PowerWakeupControllerTest018: start";
    g_service->WakeupControllerInit();
    g_service->wakeupController_->ExecWakeupMonitorByReason(WakeupDeviceType ::WAKEUP_DEVICE_DOUBLE_CLICK);
    auto monitor = g_service->wakeupController_->monitorMap_[WakeupDeviceType ::WAKEUP_DEVICE_DOUBLE_CLICK];
    EXPECT_TRUE(monitor != nullptr);

    GTEST_LOG_(INFO) << "PowerWakeupControllerTest018: end";
    POWER_HILOGD(LABEL_TEST, "PowerWakeupControllerTest018 end");
}

/**
 * @tc.name: PowerWakeupControllerTest019
 * @tc.desc: test simulate powerkey event when screenoff
 * @tc.type: FUNC
 */
HWTEST_F(PowerWakeupControllerTest, PowerWakeupControllerTest019, TestSize.Level0)
{
    POWER_HILOGD(LABEL_TEST, "PowerWakeupControllerTest019: start");

    g_service->WakeupControllerInit();
    g_service->SuspendControllerInit();
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

    inputManager->SimulateInputEvent(keyEventPowerkeyDown);
    inputManager->SimulateInputEvent(keyEventPowerkeyUp);
    inputManager->SimulateInputEvent(keyEventPowerkeyDown);
    inputManager->SimulateInputEvent(keyEventPowerkeyUp);
    sleep(2);
    //wake it up when the screen goes off after timeout
    g_service->RefreshActivity(
        static_cast<int64_t>(std::chrono::system_clock::now().time_since_epoch().count()),
            UserActivityType::USER_ACTIVITY_TYPE_TOUCH, true);
    EXPECT_TRUE(g_service->IsScreenOn());

    POWER_HILOGD(LABEL_TEST, "PowerWakeupControllerTest019: end");
}

/**
 * @tc.name: PowerWakeupControllerTest020
 * @tc.desc: test ProcessPowerOffInternalScreenOnly and ProcessPowerOnInternalScreenOnly
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
HWTEST_F(PowerWakeupControllerTest, PowerWakeupControllerTest020, TestSize.Level0)
{
    POWER_HILOGD(LABEL_TEST, "PowerWakeupControllerTest020: start");
#ifdef POWER_MANAGER_ENABLE_EXTERNAL_SCREEN_MANAGEMENT
    g_service->SuspendControllerInit();
    g_service->WakeupControllerInit();
    auto powerStateMachine = g_service->GetPowerStateMachine();
    auto suspendController = g_service->GetSuspendController();
    auto wakeupController = g_service->GetWakeupController();

    // Mock open switch to wakeup deivce
    powerStateMachine->SetSwitchState(true);
    g_service->WakeupDevice(
        static_cast<int64_t>(time(nullptr)), WakeupDeviceType::WAKEUP_DEVICE_SWITCH, "PowerWakeupControllerTest020");
    EXPECT_TRUE(powerStateMachine->IsScreenOn());
    // Mock close switch to suspend deivce, action is configured as ACTION_NONE
    powerStateMachine->SetSwitchState(false);
    suspendController->ControlListener(SuspendDeviceType::SUSPEND_DEVICE_REASON_SWITCH, SuspendAction::ACTION_NONE, 0);
    EXPECT_TRUE(powerStateMachine->IsScreenOn());

    // Mock open switch to power on internal screen
    powerStateMachine->SetSwitchState(true);
    wakeupController->ControlListener(WakeupDeviceType::WAKEUP_DEVICE_SWITCH);
    EXPECT_TRUE(powerStateMachine->IsScreenOn());
#endif
    POWER_HILOGD(LABEL_TEST, "PowerWakeupControllerTest020: end");
}
} // namespace
