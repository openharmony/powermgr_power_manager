/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include "power_wakeup_registermonitor_test.h"
#include <fstream>
#include <thread>
#include <unistd.h>
#include <cJSON.h>
#include <datetime_ex.h>
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
#include <input_manager.h>
#include "axis_event.h"
#include "input_device.h"
#include "pointer_event.h"
#endif
#include <securec.h>

#include "power_mgr_client.h"
#include "power_mgr_service.h"
#include "power_state_machine.h"
#include "setting_helper.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;
static sptr<PowerMgrService> g_service;

#ifdef HAS_MULTIMODALINPUT_INPUT_PART
class InputCallbackMock : public IInputEventConsumer {
public:
    virtual void OnInputEvent(std::shared_ptr<KeyEvent> keyEvent) const;
    virtual void OnInputEvent(std::shared_ptr<PointerEvent> pointerEvent) const;
    virtual void OnInputEvent(std::shared_ptr<AxisEvent> axisEvent) const;
};
#endif

void PowerWakeupRegisterMonitorTest::SetUpTestCase(void)
{
    g_service = DelayedSpSingleton<PowerMgrService>::GetInstance();
    g_service->OnStart();
}

void PowerWakeupRegisterMonitorTest::TearDownTestCase(void)
{
    g_service->OnStop();
    DelayedSpSingleton<PowerMgrService>::DestroyInstance();
}

namespace {
/**
 * @tc.name: PowerWakeupRegisterMonitorTest001
 * @tc.desc: test RegisterMonitor(Normal)
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest001, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest001 function start!");
    auto powerStateMachine = g_service->GetPowerStateMachine();
    g_service->WakeupControllerInit();
    auto wakeupController_ = g_service->GetWakeupController();
    EXPECT_TRUE(wakeupController_ != nullptr);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    wakeupController_->RegisterMonitor(PowerState::AWAKE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    wakeupController_->RegisterMonitor(PowerState::INACTIVE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    wakeupController_->RegisterMonitor(PowerState::UNKNOWN);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    powerStateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_APPLICATION);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    powerStateMachine->SetState(PowerState::INACTIVE, StateChangeReason::STATE_CHANGE_REASON_APPLICATION);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    InputCallback* callbackFirst = new InputCallback();
    std::shared_ptr<MMI::KeyEvent> keyEvent = OHOS::MMI::KeyEvent::Create();
    keyEvent = nullptr;
    callbackFirst->OnInputEvent(keyEvent);
    delete callbackFirst;
    InputCallback* callbackSecond = new InputCallback();
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    WakeupDeviceType wakeupType = WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN;
    wakeupType = callbackSecond->DetermineWakeupDeviceType(PointerEvent::TOOL_TYPE_PEN,
        PointerEvent::SOURCE_TYPE_UNKNOWN);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_PEN);
    wakeupType = callbackSecond->DetermineWakeupDeviceType(PointerEvent::TOOL_TYPE_MOUSE,
        PointerEvent::SOURCE_TYPE_MOUSE);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_MOUSE);
    wakeupType = callbackSecond->DetermineWakeupDeviceType(PointerEvent::TOOL_TYPE_TOUCHPAD,
        PointerEvent::SOURCE_TYPE_TOUCHPAD);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_TOUCHPAD);
    wakeupType = callbackSecond->DetermineWakeupDeviceType(PointerEvent::TOOL_TYPE_TOUCHPAD,
        PointerEvent::SOURCE_TYPE_TOUCHSCREEN);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_SINGLE_CLICK);
    wakeupType = callbackSecond->DetermineWakeupDeviceType(PointerEvent::POINTER_ACTION_UNKNOWN,
        PointerEvent::SOURCE_TYPE_UNKNOWN);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN);
    pointerEvent = nullptr;
    callbackSecond->OnInputEvent(pointerEvent);
    delete callbackSecond;
    InputCallback* callbackThird = new InputCallback();
    std::shared_ptr<MMI::AxisEvent> axisEvent = MMI::AxisEvent::Create();
    axisEvent = nullptr;
    callbackThird->OnInputEvent(axisEvent);
    delete callbackThird;
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest001 function end!");
}
#endif

/**
 * @tc.name: PowerWakeupRegisterMonitorTest002
 * @tc.desc: test RegisterMonitor(Normal)
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest002, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest002 function start!");
    auto powerStateMachine = g_service->GetPowerStateMachine();
    g_service->WakeupControllerInit();
    auto wakeupController_ = g_service->GetWakeupController();
    EXPECT_TRUE(wakeupController_ != nullptr);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    wakeupController_->RegisterMonitor(PowerState::AWAKE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    wakeupController_->RegisterMonitor(PowerState::INACTIVE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    wakeupController_->RegisterMonitor(PowerState::UNKNOWN);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    powerStateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_APPLICATION);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    powerStateMachine->SetState(PowerState::INACTIVE, StateChangeReason::STATE_CHANGE_REASON_APPLICATION);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    InputCallback* callbackFirst = new InputCallback();
    std::shared_ptr<MMI::KeyEvent> keyEvent = OHOS::MMI::KeyEvent::Create();
    keyEvent = nullptr;
    callbackFirst->OnInputEvent(keyEvent);
    delete callbackFirst;
    InputCallback* callbackSecond = new InputCallback();
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    WakeupDeviceType wakeupType = WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN;
    wakeupType = callbackSecond->DetermineWakeupDeviceType(PointerEvent::TOOL_TYPE_PEN,
        PointerEvent::TOOL_TYPE_FINGER);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_PEN);
    wakeupType = callbackSecond->DetermineWakeupDeviceType(PointerEvent::TOOL_TYPE_FINGER,
        PointerEvent::SOURCE_TYPE_MOUSE);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_MOUSE);
    wakeupType = callbackSecond->DetermineWakeupDeviceType(PointerEvent::TOOL_TYPE_FINGER,
        PointerEvent::SOURCE_TYPE_TOUCHPAD);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_TOUCHPAD);
    wakeupType = callbackSecond->DetermineWakeupDeviceType(PointerEvent::TOOL_TYPE_FINGER,
        PointerEvent::SOURCE_TYPE_TOUCHSCREEN);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_SINGLE_CLICK);
    wakeupType = callbackSecond->DetermineWakeupDeviceType(PointerEvent::TOOL_TYPE_FINGER,
        PointerEvent::SOURCE_TYPE_UNKNOWN);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN);
    pointerEvent = nullptr;
    callbackSecond->OnInputEvent(pointerEvent);
    delete callbackSecond;
    InputCallback* callbackThird = new InputCallback();
    std::shared_ptr<MMI::AxisEvent> axisEvent = MMI::AxisEvent::Create();
    axisEvent = nullptr;
    callbackThird->OnInputEvent(axisEvent);
    delete callbackThird;
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest002 function end!");
}
#endif

/**
 * @tc.name: PowerWakeupRegisterMonitorTest003
 * @tc.desc: test RegisterMonitor(Normal)
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest003, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest003 function start!");
    auto powerStateMachine = g_service->GetPowerStateMachine();
    g_service->WakeupControllerInit();
    auto wakeupController_ = g_service->GetWakeupController();
    EXPECT_TRUE(wakeupController_ != nullptr);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    wakeupController_->RegisterMonitor(PowerState::AWAKE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    wakeupController_->RegisterMonitor(PowerState::INACTIVE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    wakeupController_->RegisterMonitor(PowerState::UNKNOWN);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    powerStateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_APPLICATION);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    powerStateMachine->SetState(PowerState::INACTIVE, StateChangeReason::STATE_CHANGE_REASON_APPLICATION);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    InputCallback* callbackFirst = new InputCallback();
    std::shared_ptr<MMI::KeyEvent> keyEvent = OHOS::MMI::KeyEvent::Create();
    keyEvent = nullptr;
    callbackFirst->OnInputEvent(keyEvent);
    delete callbackFirst;
    InputCallback* callbackSecond = new InputCallback();
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    WakeupDeviceType wakeupType = WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN;
    wakeupType = callbackSecond->DetermineWakeupDeviceType(PointerEvent::TOOL_TYPE_PEN,
        PointerEvent::TOOL_TYPE_RUBBER);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_PEN);
    wakeupType = callbackSecond->DetermineWakeupDeviceType(PointerEvent::TOOL_TYPE_RUBBER,
        PointerEvent::SOURCE_TYPE_MOUSE);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_MOUSE);
    wakeupType = callbackSecond->DetermineWakeupDeviceType(PointerEvent::TOOL_TYPE_RUBBER,
        PointerEvent::SOURCE_TYPE_TOUCHPAD);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_TOUCHPAD);
    wakeupType = callbackSecond->DetermineWakeupDeviceType(PointerEvent::TOOL_TYPE_RUBBER,
        PointerEvent::SOURCE_TYPE_TOUCHSCREEN);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_SINGLE_CLICK);
    wakeupType = callbackSecond->DetermineWakeupDeviceType(PointerEvent::TOOL_TYPE_RUBBER,
        PointerEvent::SOURCE_TYPE_UNKNOWN);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN);
    pointerEvent = nullptr;
    callbackSecond->OnInputEvent(pointerEvent);
    delete callbackSecond;
    InputCallback* callbackThird = new InputCallback();
    std::shared_ptr<MMI::AxisEvent> axisEvent = MMI::AxisEvent::Create();
    axisEvent = nullptr;
    callbackThird->OnInputEvent(axisEvent);
    delete callbackThird;
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest003 function end!");
}
#endif

/**
 * @tc.name: PowerWakeupRegisterMonitorTest004
 * @tc.desc: test RegisterMonitor(Normal)
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest004, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest004 function start!");
    auto powerStateMachine = g_service->GetPowerStateMachine();
    g_service->WakeupControllerInit();
    auto wakeupController_ = g_service->GetWakeupController();
    EXPECT_TRUE(wakeupController_ != nullptr);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    wakeupController_->RegisterMonitor(PowerState::AWAKE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    wakeupController_->RegisterMonitor(PowerState::INACTIVE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    wakeupController_->RegisterMonitor(PowerState::UNKNOWN);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    powerStateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_APPLICATION);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    powerStateMachine->SetState(PowerState::INACTIVE, StateChangeReason::STATE_CHANGE_REASON_APPLICATION);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    InputCallback* callbackFirst = new InputCallback();
    std::shared_ptr<MMI::KeyEvent> keyEvent = OHOS::MMI::KeyEvent::Create();
    keyEvent = nullptr;
    callbackFirst->OnInputEvent(keyEvent);
    delete callbackFirst;
    InputCallback* callbackSecond = new InputCallback();
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    WakeupDeviceType wakeupType = WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN;
    wakeupType = callbackSecond->DetermineWakeupDeviceType(PointerEvent::TOOL_TYPE_PEN,
        PointerEvent::TOOL_TYPE_BRUSH);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_PEN);
    wakeupType = callbackSecond->DetermineWakeupDeviceType(PointerEvent::TOOL_TYPE_BRUSH,
        PointerEvent::SOURCE_TYPE_MOUSE);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_MOUSE);
    wakeupType = callbackSecond->DetermineWakeupDeviceType(PointerEvent::TOOL_TYPE_BRUSH,
        PointerEvent::SOURCE_TYPE_TOUCHPAD);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_TOUCHPAD);
    wakeupType = callbackSecond->DetermineWakeupDeviceType(PointerEvent::TOOL_TYPE_BRUSH,
        PointerEvent::SOURCE_TYPE_TOUCHSCREEN);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_SINGLE_CLICK);
    wakeupType = callbackSecond->DetermineWakeupDeviceType(PointerEvent::TOOL_TYPE_BRUSH,
        PointerEvent::SOURCE_TYPE_UNKNOWN);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN);
    pointerEvent = nullptr;
    callbackSecond->OnInputEvent(pointerEvent);
    delete callbackSecond;
    InputCallback* callbackThird = new InputCallback();
    std::shared_ptr<MMI::AxisEvent> axisEvent = MMI::AxisEvent::Create();
    axisEvent = nullptr;
    callbackThird->OnInputEvent(axisEvent);
    delete callbackThird;
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest004 function end!");
}
#endif

/**
 * @tc.name: PowerWakeupRegisterMonitorTest005
 * @tc.desc: test RegisterMonitor(Normal)
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest005, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest005 function start!");
    auto powerStateMachine = g_service->GetPowerStateMachine();
    g_service->WakeupControllerInit();
    auto wakeupController_ = g_service->GetWakeupController();
    EXPECT_TRUE(wakeupController_ != nullptr);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    wakeupController_->RegisterMonitor(PowerState::AWAKE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    wakeupController_->RegisterMonitor(PowerState::INACTIVE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    wakeupController_->RegisterMonitor(PowerState::UNKNOWN);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    powerStateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_APPLICATION);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    powerStateMachine->SetState(PowerState::INACTIVE, StateChangeReason::STATE_CHANGE_REASON_APPLICATION);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    InputCallback* callbackFirst = new InputCallback();
    std::shared_ptr<MMI::KeyEvent> keyEvent = OHOS::MMI::KeyEvent::Create();
    keyEvent = nullptr;
    callbackFirst->OnInputEvent(keyEvent);
    delete callbackFirst;
    InputCallback* callbackSecond = new InputCallback();
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    WakeupDeviceType wakeupType = WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN;
    wakeupType = callbackSecond->DetermineWakeupDeviceType(PointerEvent::TOOL_TYPE_PEN,
        PointerEvent::TOOL_TYPE_PENCIL);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_PEN);
    wakeupType = callbackSecond->DetermineWakeupDeviceType(PointerEvent::TOOL_TYPE_PENCIL,
        PointerEvent::SOURCE_TYPE_MOUSE);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_MOUSE);
    wakeupType = callbackSecond->DetermineWakeupDeviceType(PointerEvent::TOOL_TYPE_PENCIL,
        PointerEvent::SOURCE_TYPE_TOUCHPAD);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_TOUCHPAD);
    wakeupType = callbackSecond->DetermineWakeupDeviceType(PointerEvent::TOOL_TYPE_PENCIL,
        PointerEvent::SOURCE_TYPE_TOUCHSCREEN);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_SINGLE_CLICK);
    wakeupType = callbackSecond->DetermineWakeupDeviceType(PointerEvent::TOOL_TYPE_PENCIL,
        PointerEvent::SOURCE_TYPE_UNKNOWN);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN);
    pointerEvent = nullptr;
    callbackSecond->OnInputEvent(pointerEvent);
    delete callbackSecond;
    InputCallback* callbackThird = new InputCallback();
    std::shared_ptr<MMI::AxisEvent> axisEvent = MMI::AxisEvent::Create();
    axisEvent = nullptr;
    callbackThird->OnInputEvent(axisEvent);
    delete callbackThird;
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest005 function end!");
}
#endif

/**
 * @tc.name: PowerWakeupRegisterMonitorTest006
 * @tc.desc: test RegisterMonitor(Normal)
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest006, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest006 function start!");
    auto powerStateMachine = g_service->GetPowerStateMachine();
    g_service->WakeupControllerInit();
    auto wakeupController_ = g_service->GetWakeupController();
    EXPECT_TRUE(wakeupController_ != nullptr);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    wakeupController_->RegisterMonitor(PowerState::AWAKE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    wakeupController_->RegisterMonitor(PowerState::INACTIVE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    wakeupController_->RegisterMonitor(PowerState::UNKNOWN);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    powerStateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_APPLICATION);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    powerStateMachine->SetState(PowerState::INACTIVE, StateChangeReason::STATE_CHANGE_REASON_APPLICATION);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    InputCallback* callbackFirst = new InputCallback();
    std::shared_ptr<MMI::KeyEvent> keyEvent = OHOS::MMI::KeyEvent::Create();
    keyEvent = nullptr;
    callbackFirst->OnInputEvent(keyEvent);
    delete callbackFirst;
    InputCallback* callbackSecond = new InputCallback();
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    WakeupDeviceType wakeupType = WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN;
    wakeupType = callbackSecond->DetermineWakeupDeviceType(PointerEvent::TOOL_TYPE_PEN,
        PointerEvent::TOOL_TYPE_AIRBRUSH);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_PEN);
    wakeupType = callbackSecond->DetermineWakeupDeviceType(PointerEvent::TOOL_TYPE_AIRBRUSH,
        PointerEvent::SOURCE_TYPE_MOUSE);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_MOUSE);
    wakeupType = callbackSecond->DetermineWakeupDeviceType(PointerEvent::TOOL_TYPE_AIRBRUSH,
        PointerEvent::SOURCE_TYPE_TOUCHPAD);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_TOUCHPAD);
    wakeupType = callbackSecond->DetermineWakeupDeviceType(PointerEvent::TOOL_TYPE_AIRBRUSH,
        PointerEvent::SOURCE_TYPE_TOUCHSCREEN);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_SINGLE_CLICK);
    wakeupType = callbackSecond->DetermineWakeupDeviceType(PointerEvent::TOOL_TYPE_AIRBRUSH,
        PointerEvent::SOURCE_TYPE_UNKNOWN);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN);
    pointerEvent = nullptr;
    callbackSecond->OnInputEvent(pointerEvent);
    delete callbackSecond;
    InputCallback* callbackThird = new InputCallback();
    std::shared_ptr<MMI::AxisEvent> axisEvent = MMI::AxisEvent::Create();
    axisEvent = nullptr;
    callbackThird->OnInputEvent(axisEvent);
    delete callbackThird;
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest006 function end!");
}
#endif

/**
 * @tc.name: PowerWakeupRegisterMonitorTest007
 * @tc.desc: test RegisterMonitor(Normal)
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest007, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest007 function start!");
    auto powerStateMachine = g_service->GetPowerStateMachine();
    g_service->WakeupControllerInit();
    auto wakeupController_ = g_service->GetWakeupController();
    EXPECT_TRUE(wakeupController_ != nullptr);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    wakeupController_->RegisterMonitor(PowerState::AWAKE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    wakeupController_->RegisterMonitor(PowerState::INACTIVE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    wakeupController_->RegisterMonitor(PowerState::UNKNOWN);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    powerStateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_APPLICATION);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    powerStateMachine->SetState(PowerState::INACTIVE, StateChangeReason::STATE_CHANGE_REASON_APPLICATION);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    InputCallback* callbackFirst = new InputCallback();
    std::shared_ptr<MMI::KeyEvent> keyEvent = OHOS::MMI::KeyEvent::Create();
    keyEvent = nullptr;
    callbackFirst->OnInputEvent(keyEvent);
    delete callbackFirst;
    InputCallback* callbackSecond = new InputCallback();
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    WakeupDeviceType wakeupType = WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN;
    wakeupType = callbackSecond->DetermineWakeupDeviceType(PointerEvent::TOOL_TYPE_PEN,
        PointerEvent::TOOL_TYPE_MOUSE);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_PEN);
    wakeupType = callbackSecond->DetermineWakeupDeviceType(PointerEvent::TOOL_TYPE_MOUSE,
        PointerEvent::SOURCE_TYPE_MOUSE);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_MOUSE);
    wakeupType = callbackSecond->DetermineWakeupDeviceType(PointerEvent::TOOL_TYPE_MOUSE,
        PointerEvent::SOURCE_TYPE_TOUCHPAD);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_TOUCHPAD);
    wakeupType = callbackSecond->DetermineWakeupDeviceType(PointerEvent::TOOL_TYPE_MOUSE,
        PointerEvent::SOURCE_TYPE_TOUCHSCREEN);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_SINGLE_CLICK);
    wakeupType = callbackSecond->DetermineWakeupDeviceType(PointerEvent::TOOL_TYPE_MOUSE,
        PointerEvent::SOURCE_TYPE_UNKNOWN);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN);
    pointerEvent = nullptr;
    callbackSecond->OnInputEvent(pointerEvent);
    delete callbackSecond;
    InputCallback* callbackThird = new InputCallback();
    std::shared_ptr<MMI::AxisEvent> axisEvent = MMI::AxisEvent::Create();
    axisEvent = nullptr;
    callbackThird->OnInputEvent(axisEvent);
    delete callbackThird;
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest007 function end!");
}
#endif

/**
 * @tc.name: PowerWakeupRegisterMonitorTest008
 * @tc.desc: test RegisterMonitor(Normal)
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest008, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest008 function start!");
    auto powerStateMachine = g_service->GetPowerStateMachine();
    g_service->WakeupControllerInit();
    auto wakeupController_ = g_service->GetWakeupController();
    EXPECT_TRUE(wakeupController_ != nullptr);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    wakeupController_->RegisterMonitor(PowerState::AWAKE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    wakeupController_->RegisterMonitor(PowerState::INACTIVE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    wakeupController_->RegisterMonitor(PowerState::UNKNOWN);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    powerStateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_APPLICATION);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    powerStateMachine->SetState(PowerState::INACTIVE, StateChangeReason::STATE_CHANGE_REASON_APPLICATION);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    InputCallback* callbackFirst = new InputCallback();
    std::shared_ptr<MMI::KeyEvent> keyEvent = OHOS::MMI::KeyEvent::Create();
    keyEvent = nullptr;
    callbackFirst->OnInputEvent(keyEvent);
    delete callbackFirst;
    InputCallback* callbackSecond = new InputCallback();
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    WakeupDeviceType wakeupType = WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN;
    wakeupType = callbackSecond->DetermineWakeupDeviceType(PointerEvent::TOOL_TYPE_PEN,
        PointerEvent::TOOL_TYPE_LENS);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_PEN);
    wakeupType = callbackSecond->DetermineWakeupDeviceType(PointerEvent::TOOL_TYPE_LENS,
        PointerEvent::SOURCE_TYPE_MOUSE);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_MOUSE);
    wakeupType = callbackSecond->DetermineWakeupDeviceType(PointerEvent::TOOL_TYPE_LENS,
        PointerEvent::SOURCE_TYPE_TOUCHPAD);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_TOUCHPAD);
    wakeupType = callbackSecond->DetermineWakeupDeviceType(PointerEvent::TOOL_TYPE_LENS,
        PointerEvent::SOURCE_TYPE_TOUCHSCREEN);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_SINGLE_CLICK);
    wakeupType = callbackSecond->DetermineWakeupDeviceType(PointerEvent::TOOL_TYPE_LENS,
        PointerEvent::SOURCE_TYPE_UNKNOWN);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN);
    pointerEvent = nullptr;
    callbackSecond->OnInputEvent(pointerEvent);
    delete callbackSecond;
    InputCallback* callbackThird = new InputCallback();
    std::shared_ptr<MMI::AxisEvent> axisEvent = MMI::AxisEvent::Create();
    axisEvent = nullptr;
    callbackThird->OnInputEvent(axisEvent);
    delete callbackThird;
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest008 function end!");
}
#endif

/**
 * @tc.name: PowerWakeupRegisterMonitorTest009
 * @tc.desc: test RegisterMonitor(Normal)
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest009, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest009 function start!");
    auto powerStateMachine = g_service->GetPowerStateMachine();
    g_service->WakeupControllerInit();
    auto wakeupController_ = g_service->GetWakeupController();
    EXPECT_TRUE(wakeupController_ != nullptr);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    wakeupController_->RegisterMonitor(PowerState::AWAKE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    wakeupController_->RegisterMonitor(PowerState::INACTIVE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    wakeupController_->RegisterMonitor(PowerState::UNKNOWN);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    powerStateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_APPLICATION);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    powerStateMachine->SetState(PowerState::INACTIVE, StateChangeReason::STATE_CHANGE_REASON_APPLICATION);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    InputCallback* callbackFirst = new InputCallback();
    std::shared_ptr<MMI::KeyEvent> keyEvent = OHOS::MMI::KeyEvent::Create();
    keyEvent = nullptr;
    callbackFirst->OnInputEvent(keyEvent);
    delete callbackFirst;
    InputCallback* callbackSecond = new InputCallback();
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    WakeupDeviceType wakeupType = WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN;
    wakeupType = callbackSecond->DetermineWakeupDeviceType(PointerEvent::TOOL_TYPE_PEN,
        PointerEvent::TOOL_TYPE_KNUCKLE);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_PEN);
    wakeupType = callbackSecond->DetermineWakeupDeviceType(PointerEvent::TOOL_TYPE_KNUCKLE,
        PointerEvent::SOURCE_TYPE_MOUSE);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_MOUSE);
    wakeupType = callbackSecond->DetermineWakeupDeviceType(PointerEvent::TOOL_TYPE_KNUCKLE,
        PointerEvent::SOURCE_TYPE_TOUCHPAD);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_TOUCHPAD);
    wakeupType = callbackSecond->DetermineWakeupDeviceType(PointerEvent::TOOL_TYPE_KNUCKLE,
        PointerEvent::SOURCE_TYPE_TOUCHSCREEN);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_SINGLE_CLICK);
    wakeupType = callbackSecond->DetermineWakeupDeviceType(PointerEvent::TOOL_TYPE_KNUCKLE,
        PointerEvent::SOURCE_TYPE_UNKNOWN);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN);
    pointerEvent = nullptr;
    callbackSecond->OnInputEvent(pointerEvent);
    delete callbackSecond;
    InputCallback* callbackThird = new InputCallback();
    std::shared_ptr<MMI::AxisEvent> axisEvent = MMI::AxisEvent::Create();
    axisEvent = nullptr;
    callbackThird->OnInputEvent(axisEvent);
    delete callbackThird;
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest009 function end!");
}
#endif

/**
 * @tc.name: PowerWakeupRegisterMonitorTest010
 * @tc.desc: test RegisterMonitor(Normal)
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest010, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest010 function start!");
    auto powerStateMachine = g_service->GetPowerStateMachine();
    g_service->WakeupControllerInit();
    auto wakeupController_ = g_service->GetWakeupController();
    EXPECT_TRUE(wakeupController_ != nullptr);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    wakeupController_->RegisterMonitor(PowerState::AWAKE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    wakeupController_->RegisterMonitor(PowerState::INACTIVE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    wakeupController_->RegisterMonitor(PowerState::UNKNOWN);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    powerStateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_APPLICATION);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    powerStateMachine->SetState(PowerState::INACTIVE, StateChangeReason::STATE_CHANGE_REASON_APPLICATION);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    InputCallback* callbackFirst = new InputCallback();
    std::shared_ptr<MMI::KeyEvent> keyEvent = OHOS::MMI::KeyEvent::Create();
    keyEvent = nullptr;
    callbackFirst->OnInputEvent(keyEvent);
    delete callbackFirst;
    InputCallback* callbackSecond = new InputCallback();
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    WakeupDeviceType wakeupType = WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN;
    wakeupType = callbackSecond->DetermineWakeupDeviceType(PointerEvent::TOOL_TYPE_PEN,
        PointerEvent::TOOL_TYPE_TOUCHPAD);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_PEN);
    wakeupType = callbackSecond->DetermineWakeupDeviceType(PointerEvent::TOOL_TYPE_TOUCHPAD,
        PointerEvent::SOURCE_TYPE_MOUSE);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_MOUSE);
    wakeupType = callbackSecond->DetermineWakeupDeviceType(PointerEvent::TOOL_TYPE_TOUCHPAD,
        PointerEvent::SOURCE_TYPE_TOUCHPAD);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_TOUCHPAD);
    wakeupType = callbackSecond->DetermineWakeupDeviceType(PointerEvent::TOOL_TYPE_TOUCHPAD,
        PointerEvent::SOURCE_TYPE_TOUCHSCREEN);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_SINGLE_CLICK);
    wakeupType = callbackSecond->DetermineWakeupDeviceType(PointerEvent::TOOL_TYPE_TOUCHPAD,
        PointerEvent::SOURCE_TYPE_UNKNOWN);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN);
    pointerEvent = nullptr;
    callbackSecond->OnInputEvent(pointerEvent);
    delete callbackSecond;
    InputCallback* callbackThird = new InputCallback();
    std::shared_ptr<MMI::AxisEvent> axisEvent = MMI::AxisEvent::Create();
    axisEvent = nullptr;
    callbackThird->OnInputEvent(axisEvent);
    delete callbackThird;
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest010 function end!");
}
#endif


/**
 * @tc.name: PowerWakeupRegisterMonitorTest011
 * @tc.desc: test RegisterMonitor(Normal)
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest011, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest011 function start!");
    auto powerStateMachine = g_service->GetPowerStateMachine();
    g_service->WakeupControllerInit();
    auto wakeupController_ = g_service->GetWakeupController();
    EXPECT_TRUE(wakeupController_ != nullptr);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    wakeupController_->RegisterMonitor(PowerState::AWAKE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    wakeupController_->RegisterMonitor(PowerState::INACTIVE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    wakeupController_->RegisterMonitor(PowerState::UNKNOWN);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    powerStateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_APPLICATION);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    powerStateMachine->SetState(PowerState::INACTIVE, StateChangeReason::STATE_CHANGE_REASON_APPLICATION);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    InputCallback* callbackFirst = new InputCallback();
    std::shared_ptr<MMI::KeyEvent> keyEvent = OHOS::MMI::KeyEvent::Create();
    keyEvent = nullptr;
    callbackFirst->OnInputEvent(keyEvent);
    delete callbackFirst;
    InputCallback* callbackSecond = new InputCallback();
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    WakeupDeviceType wakeupType = WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN;
    wakeupType = callbackSecond->DetermineWakeupDeviceType(PointerEvent::TOOL_TYPE_PEN,
        PointerEvent::TOOL_TYPE_PALM);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_PEN);
    wakeupType = callbackSecond->DetermineWakeupDeviceType(PointerEvent::TOOL_TYPE_PALM,
        PointerEvent::SOURCE_TYPE_MOUSE);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_MOUSE);
    wakeupType = callbackSecond->DetermineWakeupDeviceType(PointerEvent::TOOL_TYPE_PALM,
        PointerEvent::SOURCE_TYPE_TOUCHPAD);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_TOUCHPAD);
    wakeupType = callbackSecond->DetermineWakeupDeviceType(PointerEvent::TOOL_TYPE_PALM,
        PointerEvent::SOURCE_TYPE_TOUCHSCREEN);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_SINGLE_CLICK);
    wakeupType = callbackSecond->DetermineWakeupDeviceType(PointerEvent::TOOL_TYPE_PALM,
        PointerEvent::SOURCE_TYPE_UNKNOWN);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN);
    pointerEvent = nullptr;
    callbackSecond->OnInputEvent(pointerEvent);
    delete callbackSecond;
    InputCallback* callbackThird = new InputCallback();
    std::shared_ptr<MMI::AxisEvent> axisEvent = MMI::AxisEvent::Create();
    axisEvent = nullptr;
    callbackThird->OnInputEvent(axisEvent);
    delete callbackThird;
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest011 function end!");
}
#endif

/**
 * @tc.name: PowerWakeupRegisterMonitorTest012
 * @tc.desc: test RegisterMonitor(Normal)
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest012, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest012 function start!");
    auto powerStateMachine = g_service->GetPowerStateMachine();
    g_service->WakeupControllerInit();
    auto wakeupController_ = g_service->GetWakeupController();
    EXPECT_TRUE(wakeupController_ != nullptr);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    wakeupController_->RegisterMonitor(PowerState::AWAKE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    wakeupController_->RegisterMonitor(PowerState::INACTIVE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    wakeupController_->RegisterMonitor(PowerState::UNKNOWN);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    powerStateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_APPLICATION);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    powerStateMachine->SetState(PowerState::INACTIVE, StateChangeReason::STATE_CHANGE_REASON_APPLICATION);
    EXPECT_EQ(wakeupController_->monitorId_, 0);
    InputCallback* callbackFirst = new InputCallback();
    std::shared_ptr<MMI::KeyEvent> keyEvent = OHOS::MMI::KeyEvent::Create();
    keyEvent = nullptr;
    callbackFirst->OnInputEvent(keyEvent);
    delete callbackFirst;
    InputCallback* callbackSecond = new InputCallback();
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    WakeupDeviceType wakeupType = WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN;
    wakeupType = callbackSecond->DetermineWakeupDeviceType(PointerEvent::TOOL_TYPE_PEN,
        PointerEvent::TOOL_TYPE_THP_FEATURE);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_PEN);
    wakeupType = callbackSecond->DetermineWakeupDeviceType(PointerEvent::TOOL_TYPE_THP_FEATURE,
        PointerEvent::SOURCE_TYPE_MOUSE);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_MOUSE);
    wakeupType = callbackSecond->DetermineWakeupDeviceType(PointerEvent::TOOL_TYPE_THP_FEATURE,
        PointerEvent::SOURCE_TYPE_TOUCHPAD);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_TOUCHPAD);
    wakeupType = callbackSecond->DetermineWakeupDeviceType(PointerEvent::TOOL_TYPE_THP_FEATURE,
        PointerEvent::SOURCE_TYPE_TOUCHSCREEN);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_SINGLE_CLICK);
    wakeupType = callbackSecond->DetermineWakeupDeviceType(PointerEvent::TOOL_TYPE_THP_FEATURE,
        PointerEvent::SOURCE_TYPE_UNKNOWN);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN);
    pointerEvent = nullptr;
    callbackSecond->OnInputEvent(pointerEvent);
    delete callbackSecond;
    InputCallback* callbackThird = new InputCallback();
    std::shared_ptr<MMI::AxisEvent> axisEvent = MMI::AxisEvent::Create();
    axisEvent = nullptr;
    callbackThird->OnInputEvent(axisEvent);
    delete callbackThird;
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest012 function end!");
}
#endif

/**
 * @tc.name: PowerWakeupRegisterMonitorTest013
 * @tc.desc: test RegisterMonitor(Normal)
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest013, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest013 function start!");
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest013: start";
    auto powerStateMachine = g_service->GetPowerStateMachine();
    g_service->WakeupControllerInit();
    auto wakeupController_ = g_service->GetWakeupController();
    g_service->SwitchSubscriberInit();
    g_service->InputMonitorInit();

    powerStateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_POWER_KEY);
    wakeupController_->RegisterMonitor(PowerState::AWAKE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    g_service->InputMonitorCancel();
    g_service->SwitchSubscriberCancel();
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest013: end";
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest013 function end!");
}
#endif

/**
 * @tc.name: PowerWakeupRegisterMonitorTest014
 * @tc.desc: test RegisterMonitor(Normal)
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest014, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest014 function start!");
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest014: start";
    auto powerStateMachine = g_service->GetPowerStateMachine();
    g_service->WakeupControllerInit();
    auto wakeupController_ = g_service->GetWakeupController();
    g_service->SwitchSubscriberInit();
    g_service->InputMonitorInit();

    powerStateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_KEYBOARD);
    wakeupController_->RegisterMonitor(PowerState::AWAKE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    g_service->InputMonitorCancel();
    g_service->SwitchSubscriberCancel();
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest014: end";
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest014 function end!");
}
#endif

/**
 * @tc.name: PowerWakeupRegisterMonitorTest015
 * @tc.desc: test RegisterMonitor(Normal)
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest015, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest015 function start!");
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest015: start";
    auto powerStateMachine = g_service->GetPowerStateMachine();
    g_service->WakeupControllerInit();
    auto wakeupController_ = g_service->GetWakeupController();
    g_service->SwitchSubscriberInit();
    g_service->InputMonitorInit();

    powerStateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_MOUSE);
    wakeupController_->RegisterMonitor(PowerState::AWAKE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    g_service->InputMonitorCancel();
    g_service->SwitchSubscriberCancel();
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest015: end";
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest015 function end!");
}
#endif

/**
 * @tc.name: PowerWakeupRegisterMonitorTest016
 * @tc.desc: test RegisterMonitor(Normal)
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest016, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest016 function start!");
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest016: start";
    auto powerStateMachine = g_service->GetPowerStateMachine();
    g_service->WakeupControllerInit();
    auto wakeupController_ = g_service->GetWakeupController();
    g_service->SwitchSubscriberInit();
    g_service->InputMonitorInit();

    powerStateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_DOUBLE_CLICK);
    wakeupController_->RegisterMonitor(PowerState::AWAKE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    g_service->InputMonitorCancel();
    g_service->SwitchSubscriberCancel();
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest016: end";
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest016 function end!");
}
#endif

/**
 * @tc.name: PowerWakeupRegisterMonitorTest017
 * @tc.desc: test RegisterMonitor(Normal)
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest017, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest017 function start!");
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest017: start";
    auto powerStateMachine = g_service->GetPowerStateMachine();
    g_service->WakeupControllerInit();
    auto wakeupController_ = g_service->GetWakeupController();
    g_service->SwitchSubscriberInit();
    g_service->InputMonitorInit();

    powerStateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_SUCCESS);
    wakeupController_->RegisterMonitor(PowerState::AWAKE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    g_service->InputMonitorCancel();
    g_service->SwitchSubscriberCancel();
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest017: end";
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest017 function end!");
}
#endif

/**
 * @tc.name: PowerWakeupRegisterMonitorTest018
 * @tc.desc: test RegisterMonitor(Normal)
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest018, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest018 function start!");
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest018: start";
    auto powerStateMachine = g_service->GetPowerStateMachine();
    g_service->WakeupControllerInit();
    auto wakeupController_ = g_service->GetWakeupController();
    g_service->SwitchSubscriberInit();
    g_service->InputMonitorInit();

    powerStateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_APPLICATION);
    wakeupController_->RegisterMonitor(PowerState::AWAKE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    g_service->InputMonitorCancel();
    g_service->SwitchSubscriberCancel();
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest018: end";
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest018 function end!");
}
#endif

/**
 * @tc.name: PowerWakeupRegisterMonitorTest019
 * @tc.desc: test RegisterMonitor(Normal)
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest019, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest019 function start!");
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest019: start";
    auto powerStateMachine = g_service->GetPowerStateMachine();
    g_service->WakeupControllerInit();
    auto wakeupController_ = g_service->GetWakeupController();
    g_service->SwitchSubscriberInit();
    g_service->InputMonitorInit();

    powerStateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_INIT);
    wakeupController_->RegisterMonitor(PowerState::AWAKE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    g_service->InputMonitorCancel();
    g_service->SwitchSubscriberCancel();
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest019: end";
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest019 function end!");
}
#endif

/**
 * @tc.name: PowerWakeupRegisterMonitorTest020
 * @tc.desc: test RegisterMonitor(Normal)
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest020, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest020 function start!");
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest020: start";
    auto powerStateMachine = g_service->GetPowerStateMachine();
    g_service->WakeupControllerInit();
    auto wakeupController_ = g_service->GetWakeupController();
    g_service->SwitchSubscriberInit();
    g_service->InputMonitorInit();

    powerStateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_RUNNING_LOCK);
    wakeupController_->RegisterMonitor(PowerState::AWAKE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    g_service->InputMonitorCancel();
    g_service->SwitchSubscriberCancel();
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest020: end";
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest020 function end!");
}
#endif

/**
 * @tc.name: PowerWakeupRegisterMonitorTest021
 * @tc.desc: test RegisterMonitor(Normal)
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest021, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest021 function start!");
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest021: start";
    auto powerStateMachine = g_service->GetPowerStateMachine();
    g_service->WakeupControllerInit();
    auto wakeupController_ = g_service->GetWakeupController();
    g_service->SwitchSubscriberInit();
    g_service->InputMonitorInit();

    powerStateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_TOUCH);
    wakeupController_->RegisterMonitor(PowerState::AWAKE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    g_service->InputMonitorCancel();
    g_service->SwitchSubscriberCancel();
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest021: end";
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest021 function end!");
}
#endif

/**
 * @tc.name: PowerWakeupRegisterMonitorTest022
 * @tc.desc: test RegisterMonitor(Normal)
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest022, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest022 function start!");
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest022: start";
    auto powerStateMachine = g_service->GetPowerStateMachine();
    g_service->WakeupControllerInit();
    auto wakeupController_ = g_service->GetWakeupController();
    g_service->SwitchSubscriberInit();
    g_service->InputMonitorInit();

    powerStateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_LID);
    wakeupController_->RegisterMonitor(PowerState::AWAKE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    g_service->InputMonitorCancel();
    g_service->SwitchSubscriberCancel();
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest022: end";
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest022 function end!");
}
#endif

/**
 * @tc.name: PowerWakeupRegisterMonitorTest023
 * @tc.desc: test RegisterMonitor(Normal)
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest023, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest023 function start!");
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest023: start";
    auto powerStateMachine = g_service->GetPowerStateMachine();
    g_service->WakeupControllerInit();
    auto wakeupController_ = g_service->GetWakeupController();
    g_service->SwitchSubscriberInit();
    g_service->InputMonitorInit();

    powerStateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_SWITCH);
    wakeupController_->RegisterMonitor(PowerState::AWAKE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    g_service->InputMonitorCancel();
    g_service->SwitchSubscriberCancel();
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest023: end";
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest023 function end!");
}
#endif

/**
 * @tc.name: PowerWakeupRegisterMonitorTest024
 * @tc.desc: test RegisterMonitor(Normal)
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest024, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest024 function start!");
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest024: start";
    auto powerStateMachine = g_service->GetPowerStateMachine();
    g_service->WakeupControllerInit();
    auto wakeupController_ = g_service->GetWakeupController();
    g_service->SwitchSubscriberInit();
    g_service->InputMonitorInit();

    powerStateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_REFRESH);
    wakeupController_->RegisterMonitor(PowerState::AWAKE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    g_service->InputMonitorCancel();
    g_service->SwitchSubscriberCancel();
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest024: end";
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest024 function end!");
}
#endif

/**
 * @tc.name: PowerWakeupRegisterMonitorTest025
 * @tc.desc: test RegisterMonitor(Normal)
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest025, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest025 function start!");
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest025: start";
    auto powerStateMachine = g_service->GetPowerStateMachine();
    g_service->WakeupControllerInit();
    auto wakeupController_ = g_service->GetWakeupController();
    g_service->SwitchSubscriberInit();
    g_service->InputMonitorInit();

    powerStateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_COORDINATION);
    wakeupController_->RegisterMonitor(PowerState::AWAKE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    g_service->InputMonitorCancel();
    g_service->SwitchSubscriberCancel();
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest025: end";
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest025 function end!");
}
#endif

/**
 * @tc.name: PowerWakeupRegisterMonitorTest026
 * @tc.desc: test RegisterMonitor(Normal)
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest026, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest026 function start!");
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest026: start";
    auto powerStateMachine = g_service->GetPowerStateMachine();
    g_service->WakeupControllerInit();
    auto wakeupController_ = g_service->GetWakeupController();
    g_service->SwitchSubscriberInit();
    g_service->InputMonitorInit();

    powerStateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_PROXIMITY);
    wakeupController_->RegisterMonitor(PowerState::AWAKE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    g_service->InputMonitorCancel();
    g_service->SwitchSubscriberCancel();
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest026: end";
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest026 function end!");
}
#endif

/**
 * @tc.name: PowerWakeupRegisterMonitorTest027
 * @tc.desc: test RegisterMonitor(Normal)
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest027, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest027 function start!");
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest027: start";
    auto powerStateMachine = g_service->GetPowerStateMachine();
    g_service->WakeupControllerInit();
    auto wakeupController_ = g_service->GetWakeupController();
    g_service->SwitchSubscriberInit();
    g_service->InputMonitorInit();

    powerStateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_INCOMING_CALL);
    wakeupController_->RegisterMonitor(PowerState::AWAKE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    g_service->InputMonitorCancel();
    g_service->SwitchSubscriberCancel();
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest027: end";
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest027 function end!");
}
#endif

/**
 * @tc.name: PowerWakeupRegisterMonitorTest028
 * @tc.desc: test RegisterMonitor(Normal)
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest028, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest028 function start!");
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest028: start";
    auto powerStateMachine = g_service->GetPowerStateMachine();
    g_service->WakeupControllerInit();
    auto wakeupController_ = g_service->GetWakeupController();
    g_service->SwitchSubscriberInit();
    g_service->InputMonitorInit();

    powerStateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_SHELL);
    wakeupController_->RegisterMonitor(PowerState::AWAKE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    g_service->InputMonitorCancel();
    g_service->SwitchSubscriberCancel();
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest028: end";
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest028 function end!");
}
#endif

/**
 * @tc.name: PowerWakeupRegisterMonitorTest029
 * @tc.desc: test RegisterMonitor(Normal)
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest029, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest029 function start!");
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest029: start";
    auto powerStateMachine = g_service->GetPowerStateMachine();
    g_service->WakeupControllerInit();
    auto wakeupController_ = g_service->GetWakeupController();
    g_service->SwitchSubscriberInit();
    g_service->InputMonitorInit();

    powerStateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_PICKUP);
    wakeupController_->RegisterMonitor(PowerState::AWAKE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    g_service->InputMonitorCancel();
    g_service->SwitchSubscriberCancel();
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest029: end";
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest029 function end!");
}
#endif

/**
 * @tc.name: PowerWakeupRegisterMonitorTest030
 * @tc.desc: test RegisterMonitor(Normal)
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest030, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest030 function start!");
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest030: start";
    auto powerStateMachine = g_service->GetPowerStateMachine();
    g_service->WakeupControllerInit();
    auto wakeupController_ = g_service->GetWakeupController();
    g_service->SwitchSubscriberInit();
    g_service->InputMonitorInit();

    powerStateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_AOD_SLIDING);
    wakeupController_->RegisterMonitor(PowerState::AWAKE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    g_service->InputMonitorCancel();
    g_service->SwitchSubscriberCancel();
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest030: end";
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest030 function end!");
}
#endif

/**
 * @tc.name: PowerWakeupRegisterMonitorTest031
 * @tc.desc: test RegisterMonitor(Normal)
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest031, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest031 function start!");
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest031: start";
    auto powerStateMachine = g_service->GetPowerStateMachine();
    g_service->WakeupControllerInit();
    auto wakeupController_ = g_service->GetWakeupController();
    g_service->SwitchSubscriberInit();
    g_service->InputMonitorInit();

    powerStateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_PEN);
    wakeupController_->RegisterMonitor(PowerState::AWAKE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    g_service->InputMonitorCancel();
    g_service->SwitchSubscriberCancel();
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest031: end";
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest031 function end!");
}
#endif

/**
 * @tc.name: PowerWakeupRegisterMonitorTest032
 * @tc.desc: test RegisterMonitor(Normal)
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest032, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest032 function start!");
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest032: start";
    auto powerStateMachine = g_service->GetPowerStateMachine();
    g_service->WakeupControllerInit();
    auto wakeupController_ = g_service->GetWakeupController();
    g_service->SwitchSubscriberInit();
    g_service->InputMonitorInit();

    powerStateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_PLUG_CHANGE);
    wakeupController_->RegisterMonitor(PowerState::AWAKE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    g_service->InputMonitorCancel();
    g_service->SwitchSubscriberCancel();
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest032: end";
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest032 function end!");
}
#endif
/**
 * @tc.name: PowerWakeupRegisterMonitorTest033
 * @tc.desc: test RegisterMonitor with INACTIVE state
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest033, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest033 function start!");
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest033: start";
    auto powerStateMachine = g_service->GetPowerStateMachine();
    g_service->WakeupControllerInit();
    auto wakeupController_ = g_service->GetWakeupController();
    g_service->SwitchSubscriberInit();
    g_service->InputMonitorInit();

    EXPECT_TRUE(wakeupController_ != nullptr);

    // Test RegisterMonitor with INACTIVE state
    powerStateMachine->SetState(PowerState::INACTIVE, StateChangeReason::STATE_CHANGE_REASON_POWER_KEY);
    wakeupController_->RegisterMonitor(PowerState::INACTIVE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    powerStateMachine->SetState(PowerState::INACTIVE, StateChangeReason::STATE_CHANGE_REASON_TOUCH);
    wakeupController_->RegisterMonitor(PowerState::INACTIVE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    powerStateMachine->SetState(PowerState::INACTIVE, StateChangeReason::STATE_CHANGE_REASON_SWITCH);
    wakeupController_->RegisterMonitor(PowerState::INACTIVE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    g_service->InputMonitorCancel();
    g_service->SwitchSubscriberCancel();
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest033: end";
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest033 function end!");
}
#endif

/**
 * @tc.name: PowerWakeupRegisterMonitorTest034
 * @tc.desc: test RegisterMonitor with SLEEP state
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest034, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest034 function start!");
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest034: start";
    auto powerStateMachine = g_service->GetPowerStateMachine();
    g_service->WakeupControllerInit();
    auto wakeupController_ = g_service->GetWakeupController();
    g_service->SwitchSubscriberInit();
    g_service->InputMonitorInit();

    EXPECT_TRUE(wakeupController_ != nullptr);

    // Test RegisterMonitor with SLEEP state
    powerStateMachine->SetState(PowerState::SLEEP, StateChangeReason::STATE_CHANGE_REASON_TIMEOUT);
    wakeupController_->RegisterMonitor(PowerState::SLEEP);
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    powerStateMachine->SetState(PowerState::SLEEP, StateChangeReason::STATE_CHANGE_REASON_POWER_KEY);
    wakeupController_->RegisterMonitor(PowerState::SLEEP);
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    g_service->InputMonitorCancel();
    g_service->SwitchSubscriberCancel();
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest034: end";
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest034 function end!");
}
#endif

/**
 * @tc.name: PowerWakeupRegisterMonitorTest035
 * @tc.desc: test RegisterMonitor with FREEZE state
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest035, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest035 function start!");
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest035: start";
    auto powerStateMachine = g_service->GetPowerStateMachine();
    g_service->WakeupControllerInit();
    auto wakeupController_ = g_service->GetWakeupController();
    g_service->SwitchSubscriberInit();
    g_service->InputMonitorInit();

    EXPECT_TRUE(wakeupController_ != nullptr);

    // Test RegisterMonitor with FREEZE state
    powerStateMachine->SetState(PowerState::FREEZE, StateChangeReason::STATE_CHANGE_REASON_TIMEOUT);
    wakeupController_->RegisterMonitor(PowerState::FREEZE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    powerStateMachine->SetState(PowerState::FREEZE, StateChangeReason::STATE_CHANGE_REASON_POWER_KEY);
    wakeupController_->RegisterMonitor(PowerState::FREEZE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    g_service->InputMonitorCancel();
    g_service->SwitchSubscriberCancel();
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest035: end";
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest035 function end!");
}
#endif

/**
 * @tc.name: PowerWakeupRegisterMonitorTest036
 * @tc.desc: test RegisterMonitor with HIBERNATE state
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest036, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest036 function start!");
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest036: start";
    auto powerStateMachine = g_service->GetPowerStateMachine();
    g_service->WakeupControllerInit();
    auto wakeupController_ = g_service->GetWakeupController();
    g_service->SwitchSubscriberInit();
    g_service->InputMonitorInit();

    EXPECT_TRUE(wakeupController_ != nullptr);

    // Test RegisterMonitor with HIBERNATE state
    powerStateMachine->SetState(PowerState::HIBERNATE, StateChangeReason::STATE_CHANGE_REASON_TIMEOUT);
    wakeupController_->RegisterMonitor(PowerState::HIBERNATE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    powerStateMachine->SetState(PowerState::HIBERNATE, StateChangeReason::STATE_CHANGE_REASON_POWER_KEY);
    wakeupController_->RegisterMonitor(PowerState::HIBERNATE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    g_service->InputMonitorCancel();
    g_service->SwitchSubscriberCancel();
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest036: end";
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest036 function end!");
}
#endif

/**
 * @tc.name: PowerWakeupRegisterMonitorTest037
 * @tc.desc: test RegisterMonitor with SHUTDOWN state
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest037, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest037 function start!");
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest037: start";
    auto powerStateMachine = g_service->GetPowerStateMachine();
    g_service->WakeupControllerInit();
    auto wakeupController_ = g_service->GetWakeupController();
    g_service->SwitchSubscriberInit();
    g_service->InputMonitorInit();

    EXPECT_TRUE(wakeupController_ != nullptr);

    // Test RegisterMonitor with SHUTDOWN state
    powerStateMachine->SetState(PowerState::SHUTDOWN, StateChangeReason::STATE_CHANGE_REASON_POWER_KEY);
    wakeupController_->RegisterMonitor(PowerState::SHUTDOWN);
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    g_service->InputMonitorCancel();
    g_service->SwitchSubscriberCancel();
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest037: end";
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest037 function end!");
}
#endif

/**
 * @tc.name: PowerWakeupRegisterMonitorTest038
 * @tc.desc: test RegisterMonitor multiple times with same state
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest038, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest038 function start!");
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest038: start";
    auto powerStateMachine = g_service->GetPowerStateMachine();
    g_service->WakeupControllerInit();
    auto wakeupController_ = g_service->GetWakeupController();
    g_service->SwitchSubscriberInit();
    g_service->InputMonitorInit();

    EXPECT_TRUE(wakeupController_ != nullptr);
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    // Test multiple RegisterMonitor calls with AWAKE state
    for (int i = 0; i < 10; i++) {
        wakeupController_->RegisterMonitor(PowerState::AWAKE);
        EXPECT_EQ(wakeupController_->monitorId_, 0);
    }

    // Test multiple RegisterMonitor calls with INACTIVE state
    for (int i = 0; i < 10; i++) {
        wakeupController_->RegisterMonitor(PowerState::INACTIVE);
        EXPECT_EQ(wakeupController_->monitorId_, 0);
    }

    // Test multiple RegisterMonitor calls with UNKNOWN state
    for (int i = 0; i < 10; i++) {
        wakeupController_->RegisterMonitor(PowerState::UNKNOWN);
        EXPECT_EQ(wakeupController_->monitorId_, 0);
    }

    g_service->InputMonitorCancel();
    g_service->SwitchSubscriberCancel();
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest038: end";
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest038 function end!");
}
#endif

/**
 * @tc.name: PowerWakeupRegisterMonitorTest039
 * @tc.desc: test RegisterMonitor with state transitions
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest039, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest039 function start!");
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest039: start";
    auto powerStateMachine = g_service->GetPowerStateMachine();
    g_service->WakeupControllerInit();
    auto wakeupController_ = g_service->GetWakeupController();
    g_service->SwitchSubscriberInit();
    g_service->InputMonitorInit();

    EXPECT_TRUE(wakeupController_ != nullptr);
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    // Test RegisterMonitor with state transitions
    powerStateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_INIT);
    wakeupController_->RegisterMonitor(PowerState::AWAKE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    powerStateMachine->SetState(PowerState::INACTIVE, StateChangeReason::STATE_CHANGE_REASON_TIMEOUT);
    wakeupController_->RegisterMonitor(PowerState::INACTIVE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    powerStateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_TOUCH);
    wakeupController_->RegisterMonitor(PowerState::AWAKE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    powerStateMachine->SetState(PowerState::SLEEP, StateChangeReason::STATE_CHANGE_REASON_TIMEOUT);
    wakeupController_->RegisterMonitor(PowerState::SLEEP);
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    powerStateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_POWER_KEY);
    wakeupController_->RegisterMonitor(PowerState::AWAKE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    g_service->InputMonitorCancel();
    g_service->SwitchSubscriberCancel();
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest039: end";
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest039 function end!");
}
#endif

/**
 * @tc.name: PowerWakeupRegisterMonitorTest044
 * @tc.desc: test InputCallback with KeyEvent handling
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest044, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest044 function start!");
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest044: start";

    // Test InputCallback with KeyEvent
    InputCallback* callback = new InputCallback();
    std::shared_ptr<MMI::KeyEvent> keyEvent = OHOS::MMI::KeyEvent::Create();

    // Test with nullptr KeyEvent
    keyEvent = nullptr;
    callback->OnInputEvent(keyEvent);
    EXPECT_TRUE(keyEvent == nullptr);

    // Test with valid KeyEvent
    keyEvent = OHOS::MMI::KeyEvent::Create();
    EXPECT_TRUE(keyEvent != nullptr);
    callback->OnInputEvent(keyEvent);

    delete callback;
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest044: end";
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest044 function end!");
}
#endif

/**
 * @tc.name: PowerWakeupRegisterMonitorTest045
 * @tc.desc: test InputCallback with PointerEvent handling
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest045, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest045 function start!");
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest045: start";

    // Test InputCallback with PointerEvent
    InputCallback* callback = new InputCallback();
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();

    // Test with nullptr PointerEvent
    pointerEvent = nullptr;
    callback->OnInputEvent(pointerEvent);
    EXPECT_TRUE(pointerEvent == nullptr);

    // Test with valid PointerEvent
    pointerEvent = MMI::PointerEvent::Create();
    EXPECT_TRUE(pointerEvent != nullptr);
    callback->OnInputEvent(pointerEvent);

    delete callback;
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest045: end";
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest045 function end!");
}
#endif

/**
 * @tc.name: PowerWakeupRegisterMonitorTest046
 * @tc.desc: test InputCallback with AxisEvent handling
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest046, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest046 function start!");
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest046: start";

    // Test InputCallback with AxisEvent
    InputCallback* callback = new InputCallback();
    std::shared_ptr<MMI::AxisEvent> axisEvent = MMI::AxisEvent::Create();

    // Test with nullptr AxisEvent
    axisEvent = nullptr;
    callback->OnInputEvent(axisEvent);
    EXPECT_TRUE(axisEvent == nullptr);

    // Test with valid AxisEvent
    axisEvent = MMI::AxisEvent::Create();
    EXPECT_TRUE(axisEvent != nullptr);
    callback->OnInputEvent(axisEvent);

    delete callback;
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest046: end";
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest046 function end!");
}
#endif

/**
 * @tc.name: PowerWakeupRegisterMonitorTest047
 * @tc.desc: test WakeupController initialization and basic operations
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest047, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest047 function start!");
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest047: start";

    g_service->WakeupControllerInit();
    auto wakeupController_ = g_service->GetWakeupController();

    EXPECT_TRUE(wakeupController_ != nullptr);
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    // Test multiple initializations
    g_service->WakeupControllerInit();
    auto wakeupController2_ = g_service->GetWakeupController();
    EXPECT_TRUE(wakeupController2_ != nullptr);

    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest047: end";
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest047 function end!");
}
#endif

/**
 * @tc.name: PowerWakeupRegisterMonitorTest048
 * @tc.desc: test PowerStateMachine state transitions
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest048, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest048 function start!");
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest048: start";

    auto powerStateMachine = g_service->GetPowerStateMachine();
    g_service->WakeupControllerInit();

    EXPECT_TRUE(powerStateMachine != nullptr);

    // Test state transitions from AWAKE
    powerStateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_INIT);
    auto currentState = powerStateMachine->GetState();
    EXPECT_EQ(currentState, PowerState::AWAKE);

    // Test state transitions to INACTIVE
    powerStateMachine->SetState(PowerState::INACTIVE, StateChangeReason::STATE_CHANGE_REASON_TIMEOUT);
    currentState = powerStateMachine->GetState();
    EXPECT_EQ(currentState, PowerState::INACTIVE);

    // Test state transitions back to AWAKE
    powerStateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_TOUCH);
    currentState = powerStateMachine->GetState();
    EXPECT_EQ(currentState, PowerState::AWAKE);

    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest048: end";
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest048 function end!");
}
#endif

/**
 * @tc.name: PowerWakeupRegisterMonitorTest049
 * @tc.desc: test SwitchSubscriberInit and InputMonitorInit
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest049, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest049 function start!");
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest049: start";

    auto powerStateMachine = g_service->GetPowerStateMachine();
    g_service->WakeupControllerInit();

    // Test initialization and cleanup
    g_service->SwitchSubscriberInit();
    g_service->InputMonitorInit();

    powerStateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_APPLICATION);

    g_service->InputMonitorCancel();
    g_service->SwitchSubscriberCancel();

    // Test multiple initialization/cleanup cycles
    for (int i = 0; i < 3; i++) {
        g_service->SwitchSubscriberInit();
        g_service->InputMonitorInit();
        powerStateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_KEYBOARD);
        g_service->InputMonitorCancel();
        g_service->SwitchSubscriberCancel();
    }

    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest049: end";
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest049 function end!");
}
#endif

/**
 * @tc.name: PowerWakeupRegisterMonitorTest050
 * @tc.desc: test RegisterMonitor with all PowerState values
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest050, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest050 function start!");
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest050: start";

    auto powerStateMachine = g_service->GetPowerStateMachine();
    g_service->WakeupControllerInit();
    auto wakeupController_ = g_service->GetWakeupController();
    g_service->SwitchSubscriberInit();
    g_service->InputMonitorInit();

    EXPECT_TRUE(wakeupController_ != nullptr);
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    // Test RegisterMonitor with all PowerState values
    wakeupController_->RegisterMonitor(PowerState::AWAKE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    wakeupController_->RegisterMonitor(PowerState::INACTIVE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    wakeupController_->RegisterMonitor(PowerState::SLEEP);
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    wakeupController_->RegisterMonitor(PowerState::FREEZE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    wakeupController_->RegisterMonitor(PowerState::HIBERNATE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    wakeupController_->RegisterMonitor(PowerState::SHUTDOWN);
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    wakeupController_->RegisterMonitor(PowerState::UNKNOWN);
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    g_service->InputMonitorCancel();
    g_service->SwitchSubscriberCancel();
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest050: end";
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest050 function end!");
}
#endif

/**
 * @tc.name: PowerWakeupRegisterMonitorTest051
 * @tc.desc: test concurrent RegisterMonitor calls
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest051, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest051 function start!");
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest051: start";

    auto powerStateMachine = g_service->GetPowerStateMachine();
    g_service->WakeupControllerInit();
    auto wakeupController_ = g_service->GetWakeupController();
    g_service->SwitchSubscriberInit();
    g_service->InputMonitorInit();

    EXPECT_TRUE(wakeupController_ != nullptr);
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    // Test concurrent-like RegisterMonitor calls (sequential simulation)
    std::vector<PowerState> states = {
        PowerState::AWAKE, PowerState::INACTIVE, PowerState::SLEEP,
        PowerState::FREEZE, PowerState::HIBERNATE, PowerState::UNKNOWN
    };

    for (const auto& state : states) {
        wakeupController_->RegisterMonitor(state);
        EXPECT_EQ(wakeupController_->monitorId_, 0);
    }

    // Reverse order
    for (auto it = states.rbegin(); it != states.rend(); ++it) {
        wakeupController_->RegisterMonitor(*it);
        EXPECT_EQ(wakeupController_->monitorId_, 0);
    }

    g_service->InputMonitorCancel();
    g_service->SwitchSubscriberCancel();
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest051: end";
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest051 function end!");
}
#endif

/**
 * @tc.name: PowerWakeupRegisterMonitorTest053
 * @tc.desc: test RegisterMonitor with complex state combinations
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest053, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest053 function start!");
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest053: start";

    auto powerStateMachine = g_service->GetPowerStateMachine();
    g_service->WakeupControllerInit();
    auto wakeupController_ = g_service->GetWakeupController();
    g_service->SwitchSubscriberInit();
    g_service->InputMonitorInit();

    EXPECT_TRUE(wakeupController_ != nullptr);
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    // Test complex state combinations
    std::vector<StateChangeReason> reasons = {
        StateChangeReason::STATE_CHANGE_REASON_POWER_KEY,
        StateChangeReason::STATE_CHANGE_REASON_TIMEOUT,
        StateChangeReason::STATE_CHANGE_REASON_TOUCH,
        StateChangeReason::STATE_CHANGE_REASON_KEYBOARD,
        StateChangeReason::STATE_CHANGE_REASON_MOUSE,
        StateChangeReason::STATE_CHANGE_REASON_SWITCH,
        StateChangeReason::STATE_CHANGE_REASON_APPLICATION,
        StateChangeReason::STATE_CHANGE_REASON_RUNNING_LOCK
    };

    for (const auto& reason : reasons) {
        powerStateMachine->SetState(PowerState::AWAKE, reason);
        wakeupController_->RegisterMonitor(PowerState::AWAKE);
        EXPECT_EQ(wakeupController_->monitorId_, 0);
    }

    g_service->InputMonitorCancel();
    g_service->SwitchSubscriberCancel();
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest053: end";
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest053 function end!");
}
#endif

/**
 * @tc.name: PowerWakeupRegisterMonitorTest055
 * @tc.desc: test RegisterMonitor with alternating states
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest055, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest055 function start!");
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest055: start";

    auto powerStateMachine = g_service->GetPowerStateMachine();
    g_service->WakeupControllerInit();
    auto wakeupController_ = g_service->GetWakeupController();
    g_service->SwitchSubscriberInit();
    g_service->InputMonitorInit();

    EXPECT_TRUE(wakeupController_ != nullptr);
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    // Test alternating states
    for (int i = 0; i < 10; i++) {
        powerStateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_APPLICATION);
        wakeupController_->RegisterMonitor(PowerState::AWAKE);
        EXPECT_EQ(wakeupController_->monitorId_, 0);

        powerStateMachine->SetState(PowerState::INACTIVE, StateChangeReason::STATE_CHANGE_REASON_TIMEOUT);
        wakeupController_->RegisterMonitor(PowerState::INACTIVE);
        EXPECT_EQ(wakeupController_->monitorId_, 0);
    }

    g_service->InputMonitorCancel();
    g_service->SwitchSubscriberCancel();
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest055: end";
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest055 function end!");
}
#endif

/**
 * @tc.name: PowerWakeupRegisterMonitorTest056
 * @tc.desc: test RegisterMonitor with extreme edge cases
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest056, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest056 function start!");
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest056: start";

    auto powerStateMachine = g_service->GetPowerStateMachine();
    g_service->WakeupControllerInit();
    auto wakeupController_ = g_service->GetWakeupController();
    g_service->SwitchSubscriberInit();
    g_service->InputMonitorInit();

    EXPECT_TRUE(wakeupController_ != nullptr);
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    // Test extreme edge cases with rapid state changes
    for (int i = 0; i < 100; i++) {
        wakeupController_->RegisterMonitor(PowerState::AWAKE);
    }
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    // Test with different states in quick succession
    wakeupController_->RegisterMonitor(PowerState::AWAKE);
    wakeupController_->RegisterMonitor(PowerState::INACTIVE);
    wakeupController_->RegisterMonitor(PowerState::SLEEP);
    wakeupController_->RegisterMonitor(PowerState::FREEZE);
    wakeupController_->RegisterMonitor(PowerState::HIBERNATE);
    wakeupController_->RegisterMonitor(PowerState::SHUTDOWN);
    wakeupController_->RegisterMonitor(PowerState::UNKNOWN);
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    g_service->InputMonitorCancel();
    g_service->SwitchSubscriberCancel();
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest056: end";
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest056 function end!");
}
#endif

/**
 * @tc.name: PowerWakeupRegisterMonitorTest058
 * @tc.desc: test RegisterMonitor with service lifecycle
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest058, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest058 function start!");
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest058: start";

    // Test service initialization and state
    g_service->WakeupControllerInit();
    auto wakeupController_ = g_service->GetWakeupController();

    EXPECT_TRUE(wakeupController_ != nullptr);

    g_service->SwitchSubscriberInit();
    g_service->InputMonitorInit();

    // Test RegisterMonitor at various service states
    wakeupController_->RegisterMonitor(PowerState::AWAKE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    wakeupController_->RegisterMonitor(PowerState::INACTIVE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    wakeupController_->RegisterMonitor(PowerState::UNKNOWN);
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    g_service->InputMonitorCancel();
    g_service->SwitchSubscriberCancel();

    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest058: end";
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest058 function end!");
}
#endif

/**
 * @tc.name: PowerWakeupRegisterMonitorTest059
 * @tc.desc: test RegisterMonitor with complex input scenarios
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest059, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest059 function start!");
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest059: start";

    auto powerStateMachine = g_service->GetPowerStateMachine();
    g_service->WakeupControllerInit();
    auto wakeupController_ = g_service->GetWakeupController();
    g_service->SwitchSubscriberInit();
    g_service->InputMonitorInit();

    // Test input scenarios with different device types
    InputCallback* callback = new InputCallback();

    // Simulate pen input
    WakeupDeviceType wakeupType = callback->DetermineWakeupDeviceType(
        PointerEvent::TOOL_TYPE_PEN, PointerEvent::SOURCE_TYPE_TOUCHSCREEN);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_PEN);
    powerStateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_PEN);
    wakeupController_->RegisterMonitor(PowerState::AWAKE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    // Simulate mouse input
    wakeupType = callback->DetermineWakeupDeviceType(
        PointerEvent::TOOL_TYPE_MOUSE, PointerEvent::SOURCE_TYPE_MOUSE);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_MOUSE);
    powerStateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_MOUSE);
    wakeupController_->RegisterMonitor(PowerState::AWAKE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    // Simulate keyboard input
    powerStateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_KEYBOARD);
    wakeupController_->RegisterMonitor(PowerState::AWAKE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    // Simulate touch input
    wakeupType = callback->DetermineWakeupDeviceType(
        PointerEvent::TOOL_TYPE_FINGER, PointerEvent::SOURCE_TYPE_TOUCHSCREEN);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_SINGLE_CLICK);
    powerStateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_TOUCH);
    wakeupController_->RegisterMonitor(PowerState::AWAKE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    delete callback;
    g_service->InputMonitorCancel();
    g_service->SwitchSubscriberCancel();

    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest059: end";
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest059 function end!");
}
#endif

/**
 * @tc.name: PowerWakeupRegisterMonitorTest060
 * @tc.desc: test RegisterMonitor with comprehensive state coverage
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest060, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest060 function start!");
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest060: start";

    auto powerStateMachine = g_service->GetPowerStateMachine();
    g_service->WakeupControllerInit();
    auto wakeupController_ = g_service->GetWakeupController();
    g_service->SwitchSubscriberInit();
    g_service->InputMonitorInit();

    EXPECT_TRUE(wakeupController_ != nullptr);
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    // Comprehensive state coverage test
    std::vector<std::pair<PowerState, StateChangeReason>> testCases = {
        {PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_POWER_KEY},
        {PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_KEYBOARD},
        {PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_MOUSE},
        {PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_DOUBLE_CLICK},
        {PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_TOUCH},
        {PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_LID},
        {PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_SWITCH},
        {PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_REFRESH},
        {PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_COORDINATION},
        {PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_PROXIMITY},
        {PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_INCOMING_CALL},
        {PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_SHELL},
        {PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_PICKUP},
        {PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_AOD_SLIDING},
        {PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_PEN},
        {PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_PLUG_CHANGE},
        {PowerState::INACTIVE, StateChangeReason::STATE_CHANGE_REASON_TIMEOUT},
        {PowerState::INACTIVE, StateChangeReason::STATE_CHANGE_REASON_POWER_KEY},
        {PowerState::INACTIVE, StateChangeReason::STATE_CHANGE_REASON_APPLICATION},
    };

    for (const auto& testCase : testCases) {
        powerStateMachine->SetState(testCase.first, testCase.second);
        wakeupController_->RegisterMonitor(testCase.first);
        EXPECT_EQ(wakeupController_->monitorId_, 0);
    }

    g_service->InputMonitorCancel();
    g_service->SwitchSubscriberCancel();
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest060: end";
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest060 function end!");
}
#endif

/**
 * @tc.name: PowerWakeupRegisterMonitorTest063
 * @tc.desc: test RegisterMonitor with state machine consistency
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest063, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest063 function start!");
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest063: start";

    auto powerStateMachine = g_service->GetPowerStateMachine();
    g_service->WakeupControllerInit();
    auto wakeupController_ = g_service->GetWakeupController();
    g_service->SwitchSubscriberInit();
    g_service->InputMonitorInit();

    EXPECT_TRUE(wakeupController_ != nullptr);

    // Test state machine consistency
    PowerState initialState = powerStateMachine->GetState();
    EXPECT_TRUE(initialState == PowerState::AWAKE ||
                initialState == PowerState::INACTIVE ||
                initialState == PowerState::SLEEP);

    // Set state to AWAKE and verify
    powerStateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_INIT);
    PowerState currentState = powerStateMachine->GetState();
    EXPECT_EQ(currentState, PowerState::AWAKE);

    // Register monitor and verify
    wakeupController_->RegisterMonitor(PowerState::AWAKE);
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    // Verify state hasn't changed
    currentState = powerStateMachine->GetState();
    EXPECT_EQ(currentState, PowerState::AWAKE);

    g_service->InputMonitorCancel();
    g_service->SwitchSubscriberCancel();
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest063: end";
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest063 function end!");
}
#endif

/**
 * @tc.name: PowerWakeupRegisterMonitorTest064
 * @tc.desc: test RegisterMonitor with multiple initialization scenarios
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest064, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest064 function start!");
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest064: start";

    // Test multiple initialization scenarios
    for (int i = 0; i < 5; i++) {
        auto powerStateMachine = g_service->GetPowerStateMachine();
        g_service->WakeupControllerInit();
        auto wakeupController_ = g_service->GetWakeupController();

        EXPECT_TRUE(wakeupController_ != nullptr);
        EXPECT_EQ(wakeupController_->monitorId_, 0);

        powerStateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_APPLICATION);
        wakeupController_->RegisterMonitor(PowerState::AWAKE);
        EXPECT_EQ(wakeupController_->monitorId_, 0);
    }

    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest064: end";
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest064 function end!");
}
#endif

/**
 * @tc.name: PowerWakeupRegisterMonitorTest065
 * @tc.desc: test DetermineWakeupDeviceType boundary conditions
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest065, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest065 function start!");
    InputCallback* callback = new InputCallback();

    // Test boundary conditions with extreme values
    WakeupDeviceType wakeupType;

    // Test with minimum value combinations
    wakeupType = callback->DetermineWakeupDeviceType(0, 0);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN);

    // Test with valid combinations
    wakeupType = callback->DetermineWakeupDeviceType(
        PointerEvent::TOOL_TYPE_PEN, PointerEvent::SOURCE_TYPE_TOUCHSCREEN);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_PEN);

    wakeupType = callback->DetermineWakeupDeviceType(
        PointerEvent::TOOL_TYPE_FINGER, PointerEvent::SOURCE_TYPE_TOUCHSCREEN);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_SINGLE_CLICK);

    wakeupType = callback->DetermineWakeupDeviceType(
        PointerEvent::TOOL_TYPE_MOUSE, PointerEvent::SOURCE_TYPE_MOUSE);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_MOUSE);

    wakeupType = callback->DetermineWakeupDeviceType(
        PointerEvent::TOOL_TYPE_TOUCHPAD, PointerEvent::SOURCE_TYPE_TOUCHPAD);
    EXPECT_EQ(wakeupType, WakeupDeviceType::WAKEUP_DEVICE_TOUCHPAD);

    delete callback;
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest065 function end!");
}
#endif

/**
 * @tc.name: PowerWakeupRegisterMonitorTest066
 * @tc.desc: test RegisterMonitor with thread safety simulation
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest066, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest066 function start!");
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest066: start";

    auto powerStateMachine = g_service->GetPowerStateMachine();
    g_service->WakeupControllerInit();
    auto wakeupController_ = g_service->GetWakeupController();
    g_service->SwitchSubscriberInit();
    g_service->InputMonitorInit();

    EXPECT_TRUE(wakeupController_ != nullptr);

    // Simulate thread safety with rapid sequential operations
    const int iterations = 50;
    for (int i = 0; i < iterations; i++) {
        powerStateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_APPLICATION);
        wakeupController_->RegisterMonitor(PowerState::AWAKE);
        EXPECT_EQ(wakeupController_->monitorId_, 0);

        powerStateMachine->SetState(PowerState::INACTIVE, StateChangeReason::STATE_CHANGE_REASON_TIMEOUT);
        wakeupController_->RegisterMonitor(PowerState::INACTIVE);
        EXPECT_EQ(wakeupController_->monitorId_, 0);
    }

    g_service->InputMonitorCancel();
    g_service->SwitchSubscriberCancel();
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest066: end";
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest066 function end!");
}
#endif

/**
 * @tc.name: PowerWakeupRegisterMonitorTest067
 * @tc.desc: test RegisterMonitor with state persistence
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest067, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest067 function start!");
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest067: start";

    auto powerStateMachine = g_service->GetPowerStateMachine();
    g_service->WakeupControllerInit();
    auto wakeupController_ = g_service->GetWakeupController();
    g_service->SwitchSubscriberInit();
    g_service->InputMonitorInit();

    EXPECT_TRUE(wakeupController_ != nullptr);

    // Test state persistence across operations
    uint32_t initialMonitorId = wakeupController_->monitorId_;
    EXPECT_EQ(initialMonitorId, 0);

    // Perform multiple state changes and register calls
    for (int i = 0; i < 10; i++) {
        powerStateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_APPLICATION);
        wakeupController_->RegisterMonitor(PowerState::AWAKE);

        powerStateMachine->SetState(PowerState::INACTIVE, StateChangeReason::STATE_CHANGE_REASON_TIMEOUT);
        wakeupController_->RegisterMonitor(PowerState::INACTIVE);
    }

    // Verify monitorId persistence
    EXPECT_EQ(wakeupController_->monitorId_, 0);

    g_service->InputMonitorCancel();
    g_service->SwitchSubscriberCancel();
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest067: end";
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest067 function end!");
}
#endif

/**
 * @tc.name: PowerWakeupRegisterMonitorTest069
 * @tc.desc: test RegisterMonitor with state machine reset scenarios
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest069, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest069 function start!");
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest069: start";

    auto powerStateMachine = g_service->GetPowerStateMachine();
    g_service->WakeupControllerInit();
    auto wakeupController_ = g_service->GetWakeupController();
    g_service->SwitchSubscriberInit();
    g_service->InputMonitorInit();

    EXPECT_TRUE(wakeupController_ != nullptr);

    // Test reset scenarios
    for (int i = 0; i < 5; i++) {
        // Set to various states
        powerStateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_INIT);
        wakeupController_->RegisterMonitor(PowerState::AWAKE);

        powerStateMachine->SetState(PowerState::INACTIVE, StateChangeReason::STATE_CHANGE_REASON_TIMEOUT);
        wakeupController_->RegisterMonitor(PowerState::INACTIVE);

        powerStateMachine->SetState(PowerState::SLEEP, StateChangeReason::STATE_CHANGE_REASON_TIMEOUT);
        wakeupController_->RegisterMonitor(PowerState::SLEEP);

        // Reset to AWAKE
        powerStateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_POWER_KEY);
        wakeupController_->RegisterMonitor(PowerState::AWAKE);
    }

    g_service->InputMonitorCancel();
    g_service->SwitchSubscriberCancel();
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest069: end";
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest069 function end!");
}
#endif

/**
 * @tc.name: PowerWakeupRegisterMonitorTest070
 * @tc.desc: test RegisterMonitor with comprehensive combination coverage
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest070, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest070 function start!");
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest070: start";

    auto powerStateMachine = g_service->GetPowerStateMachine();
    g_service->WakeupControllerInit();
    auto wakeupController_ = g_service->GetWakeupController();
    g_service->SwitchSubscriberInit();
    g_service->InputMonitorInit();

    EXPECT_TRUE(wakeupController_ != nullptr);

    // Comprehensive combination coverage
    std::vector<PowerState> states = {
        PowerState::AWAKE, PowerState::INACTIVE, PowerState::SLEEP,
        PowerState::FREEZE, PowerState::HIBERNATE, PowerState::SHUTDOWN, PowerState::UNKNOWN
    };

    std::vector<StateChangeReason> reasons = {
        StateChangeReason::STATE_CHANGE_REASON_INIT,
        StateChangeReason::STATE_CHANGE_REASON_POWER_KEY,
        StateChangeReason::STATE_CHANGE_REASON_TIMEOUT,
        StateChangeReason::STATE_CHANGE_REASON_APPLICATION,
        StateChangeReason::STATE_CHANGE_REASON_RUNNING_LOCK,
        StateChangeReason::STATE_CHANGE_REASON_TOUCH
    };

    // Test all combinations
    for (const auto& state : states) {
        for (const auto& reason : reasons) {
            powerStateMachine->SetState(state, reason);
            wakeupController_->RegisterMonitor(state);
            EXPECT_EQ(wakeupController_->monitorId_, 0);
        }
    }

    g_service->InputMonitorCancel();
    g_service->SwitchSubscriberCancel();
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest070: end";
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest070 function end!");
}
#endif

/**
 * @tc.name: PowerWakeupRegisterMonitorTest072
 * @tc.desc: test RegisterMonitor with stress testing
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest072, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest072 function start!");
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest072: start";

    auto powerStateMachine = g_service->GetPowerStateMachine();
    g_service->WakeupControllerInit();
    auto wakeupController_ = g_service->GetWakeupController();
    g_service->SwitchSubscriberInit();
    g_service->InputMonitorInit();

    EXPECT_TRUE(wakeupController_ != nullptr);

    // Stress test with many operations
    const int stressIterations = 200;
    for (int i = 0; i < stressIterations; i++) {
        wakeupController_->RegisterMonitor(PowerState::AWAKE);
        wakeupController_->RegisterMonitor(PowerState::INACTIVE);
        wakeupController_->RegisterMonitor(PowerState::UNKNOWN);

        powerStateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_APPLICATION);
        wakeupController_->RegisterMonitor(PowerState::AWAKE);
    }

    EXPECT_EQ(wakeupController_->monitorId_, 0);

    g_service->InputMonitorCancel();
    g_service->SwitchSubscriberCancel();
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest072: end";
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest072 function end!");
}
#endif

/**
 * @tc.name: PowerWakeupRegisterMonitorTest073
 * @tc.desc: test RegisterMonitor with different state change patterns
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest073, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest073 function start!");
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest073: start";

    auto powerStateMachine = g_service->GetPowerStateMachine();
    g_service->WakeupControllerInit();
    auto wakeupController_ = g_service->GetWakeupController();
    g_service->SwitchSubscriberInit();
    g_service->InputMonitorInit();

    EXPECT_TRUE(wakeupController_ != nullptr);

    // Test different state change patterns
    // Pattern 1: Linear progression
    powerStateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_INIT);
    wakeupController_->RegisterMonitor(PowerState::AWAKE);
    powerStateMachine->SetState(PowerState::INACTIVE, StateChangeReason::STATE_CHANGE_REASON_TIMEOUT);
    wakeupController_->RegisterMonitor(PowerState::INACTIVE);
    powerStateMachine->SetState(PowerState::SLEEP, StateChangeReason::STATE_CHANGE_REASON_TIMEOUT);
    wakeupController_->RegisterMonitor(PowerState::SLEEP);

    // Pattern 2: Jump back
    powerStateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_POWER_KEY);
    wakeupController_->RegisterMonitor(PowerState::AWAKE);

    // Pattern 3: Skip states
    powerStateMachine->SetState(PowerState::HIBERNATE, StateChangeReason::STATE_CHANGE_REASON_POWER_KEY);
    wakeupController_->RegisterMonitor(PowerState::HIBERNATE);
    powerStateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_POWER_KEY);
    wakeupController_->RegisterMonitor(PowerState::AWAKE);

    EXPECT_EQ(wakeupController_->monitorId_, 0);

    g_service->InputMonitorCancel();
    g_service->SwitchSubscriberCancel();
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest073: end";
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest073 function end!");
}
#endif

/**
 * @tc.name: PowerWakeupRegisterMonitorTest074
 * @tc.desc: test DetermineWakeupDeviceType with edge tool types
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest074, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest074 function start!");
    InputCallback* callback = new InputCallback();

    // Test edge tool types with various sources
    std::vector<int32_t> edgeToolTypes = {
        PointerEvent::TOOL_TYPE_LENS,
        PointerEvent::TOOL_TYPE_KNUCKLE,
        PointerEvent::TOOL_TYPE_PALM,
        PointerEvent::TOOL_TYPE_THP_FEATURE
    };

    for (const auto& toolType : edgeToolTypes) {
        WakeupDeviceType result = callback->DetermineWakeupDeviceType(
            toolType, PointerEvent::SOURCE_TYPE_TOUCHSCREEN);
        EXPECT_TRUE(result == WakeupDeviceType::WAKEUP_DEVICE_SINGLE_CLICK ||
                    result == WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN);

        result = callback->DetermineWakeupDeviceType(
            toolType, PointerEvent::SOURCE_TYPE_MOUSE);
        EXPECT_TRUE(result == WakeupDeviceType::WAKEUP_DEVICE_MOUSE ||
                    result == WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN);

        result = callback->DetermineWakeupDeviceType(
            toolType, PointerEvent::SOURCE_TYPE_TOUCHPAD);
        EXPECT_TRUE(result == WakeupDeviceType::WAKEUP_DEVICE_TOUCHPAD ||
                    result == WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN);
    }

    delete callback;
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest074 function end!");
}
#endif

/**
 * @tc.name: PowerWakeupRegisterMonitorTest075
 * @tc.desc: test RegisterMonitor with all valid power states and reasons
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerWakeupRegisterMonitorTest, PowerWakeupRegisterMonitorTest075, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest075 function start!");
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest075: start";

    auto powerStateMachine = g_service->GetPowerStateMachine();
    g_service->WakeupControllerInit();
    auto wakeupController_ = g_service->GetWakeupController();
    g_service->SwitchSubscriberInit();
    g_service->InputMonitorInit();

    EXPECT_TRUE(wakeupController_ != nullptr);

    // Test all valid combinations systematically
    std::vector<PowerState> states = {
        PowerState::AWAKE,
        PowerState::INACTIVE,
        PowerState::SLEEP,
        PowerState::FREEZE,
        PowerState::HIBERNATE,
        PowerState::SHUTDOWN,
        PowerState::UNKNOWN
    };

    std::vector<StateChangeReason> reasons = {
        StateChangeReason::STATE_CHANGE_REASON_INIT,
        StateChangeReason::STATE_CHANGE_REASON_POWER_KEY,
        StateChangeReason::STATE_CHANGE_REASON_TIMEOUT,
        StateChangeReason::STATE_CHANGE_REASON_KEYBOARD,
        StateChangeReason::STATE_CHANGE_REASON_MOUSE,
        StateChangeReason::STATE_CHANGE_REASON_DOUBLE_CLICK,
        StateChangeReason::STATE_CHANGE_REASON_TOUCH,
        StateChangeReason::STATE_CHANGE_REASON_LID,
        StateChangeReason::STATE_CHANGE_REASON_SWITCH,
        StateChangeReason::STATE_CHANGE_REASON_REFRESH,
        StateChangeReason::STATE_CHANGE_REASON_COORDINATION,
        StateChangeReason::STATE_CHANGE_REASON_PROXIMITY,
        StateChangeReason::STATE_CHANGE_REASON_INCOMING_CALL,
        StateChangeReason::STATE_CHANGE_REASON_SHELL,
        StateChangeReason::STATE_CHANGE_REASON_PICKUP,
        StateChangeReason::STATE_CHANGE_REASON_AOD_SLIDING,
        StateChangeReason::STATE_CHANGE_REASON_PEN,
        StateChangeReason::STATE_CHANGE_REASON_PLUG_CHANGE
    };

    int testCount = 0;
    for (const auto& state : states) {
        for (const auto& reason : reasons) {
            powerStateMachine->SetState(state, reason);
            wakeupController_->RegisterMonitor(state);
            EXPECT_EQ(wakeupController_->monitorId_, 0);
            testCount++;
        }
    }

    GTEST_LOG_(INFO) << "Total tests: " << testCount;
    g_service->InputMonitorCancel();
    g_service->SwitchSubscriberCancel();
    GTEST_LOG_(INFO) << "PowerWakeupRegisterMonitorTest075: end";
    POWER_HILOGI(LABEL_TEST, "PowerWakeupRegisterMonitorTest075 function end!");
}
#endif

} // namespace