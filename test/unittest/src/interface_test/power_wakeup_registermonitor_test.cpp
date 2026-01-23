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
} // namespace