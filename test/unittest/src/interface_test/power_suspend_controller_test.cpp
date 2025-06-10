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
#include "power_suspend_controller_test.h"
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
#include "power_state_callback_stub.h"
#include "power_state_machine.h"
#include "setting_helper.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;
static sptr<PowerMgrService> g_service;
static constexpr int SLEEP_WAIT_TIME_S = 2;
static constexpr int NEXT_WAIT_TIME_S = 1;

void PowerSuspendControllerTest::SetUpTestCase(void)
{
    g_service = DelayedSpSingleton<PowerMgrService>::GetInstance();
    g_service->OnStart();
}

void PowerSuspendControllerTest::TearDownTestCase(void)
{
    g_service->OnStop();
    DelayedSpSingleton<PowerMgrService>::DestroyInstance();
}

class SuspendPowerStateCallback : public PowerStateCallbackStub {
public:
    explicit SuspendPowerStateCallback(std::shared_ptr<SuspendController> controller) : controller_(controller) {};
    virtual ~SuspendPowerStateCallback() = default;
    void OnPowerStateChanged(PowerState state) override
    {
        auto controller = controller_.lock();
        if (controller == nullptr) {
            POWER_HILOGI(FEATURE_SUSPEND, "OnPowerStateChanged: No controller");
            return;
        }
        if (state == PowerState::AWAKE) {
            POWER_HILOGI(FEATURE_SUSPEND, "Turn awake, stop sleep timer");
            controller->StopSleep();
        }
    }

private:
    std::weak_ptr<SuspendController> controller_;
};

namespace {
/**
 * @tc.name: PowerSuspendControllerTest001
 * @tc.desc: test OnPowerStateChanged(exception)
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
HWTEST_F(PowerSuspendControllerTest, PowerSuspendControllerTest001, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerSuspendControllerTest001 function start!");
    GTEST_LOG_(INFO) << "PowerSuspend001: start";
    g_service->SuspendControllerInit();
    g_service->WakeupControllerInit();
    sptr<SuspendPowerStateCallback> callback = new SuspendPowerStateCallback(g_service->suspendController_);
    callback->controller_.reset();
    callback->OnPowerStateChanged(PowerState::AWAKE);
    auto controller = callback->controller_.lock();
    EXPECT_EQ(controller, nullptr);
    GTEST_LOG_(INFO) << "PowerSuspendControllerTest001: end";
    POWER_HILOGI(LABEL_TEST, "PowerSuspendControllerTest001 function end!");
}

/**
 * @tc.name: PowerSuspendControllerTest002
 * @tc.desc: test ExecSuspendMonitorByReason(Normal and exception)
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
HWTEST_F(PowerSuspendControllerTest, PowerSuspendControllerTest002, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerSuspendControllerTest002 function start!");
    GTEST_LOG_(INFO) << "PowerSuspendControllerTest002: start";
    sleep(SLEEP_WAIT_TIME_S);
    g_service->SuspendControllerInit();
    g_service->suspendController_->ExecSuspendMonitorByReason(SuspendDeviceType ::SUSPEND_DEVICE_REASON_POWER_KEY);
    auto monitor = g_service->suspendController_->monitorMap_[SuspendDeviceType ::SUSPEND_DEVICE_REASON_POWER_KEY];
    EXPECT_TRUE(monitor != nullptr);
    GTEST_LOG_(INFO) << "PowerSuspendControllerTest002: end";
    POWER_HILOGI(LABEL_TEST, "PowerSuspendControllerTest002 function end!");
}

/**
 * @tc.name: PowerSuspendControllerTest003
 * @tc.desc: test RegisterSettingsObserver(exception 1, nothing!)
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
HWTEST_F(PowerSuspendControllerTest, PowerSuspendControllerTest003, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerSuspendControllerTest003 function start!");
    GTEST_LOG_(INFO) << "PowerSuspendControllerTest003: start";
    g_service->SuspendControllerInit();
    g_service->suspendController_->RegisterSettingsObserver();
    EXPECT_TRUE(g_service->suspendController_ != nullptr);
    GTEST_LOG_(INFO) << "PowerSuspendControllerTest003: end";
    POWER_HILOGI(LABEL_TEST, "PowerSuspendControllerTest003 function end!");
}

/**
 * @tc.name: PowerSuspendControllerTest004
 * @tc.desc: test Execute
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
HWTEST_F(PowerSuspendControllerTest, PowerSuspendControllerTest004, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerSuspendControllerTest004 function start!");
    GTEST_LOG_(INFO) << "PowerSuspendControllerTest004: start";
    g_service->SuspendControllerInit();
    g_service->suspendController_->Execute();
    EXPECT_TRUE(g_service->suspendController_ != nullptr);
    GTEST_LOG_(INFO) << "PowerSuspendControllerTest004: end";
    POWER_HILOGI(LABEL_TEST, "PowerSuspendControllerTest004 function end!");
}

/**
 * @tc.name: PowerSuspendControllerTest005
 * @tc.desc: test CreateMonitor
 * @tc.type: FUNC
 * @tc.require: issueI7G6OY
 */
HWTEST_F(PowerSuspendControllerTest, PowerSuspendControllerTest005, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerSuspendControllerTest005 function start!");
    GTEST_LOG_(INFO) << "PowerSuspendControllerTest005: start";
    g_service->SuspendControllerInit();
    SuspendSource source(SuspendDeviceType::SUSPEND_DEVICE_REASON_STR, 1, 0);
    std::shared_ptr<SuspendMonitor> monitor = SuspendMonitor::CreateMonitor(source);
    EXPECT_TRUE(monitor == nullptr);
    GTEST_LOG_(INFO) << "PowerSuspendControllerTest005: end";
    POWER_HILOGI(LABEL_TEST, "PowerSuspendControllerTest005 function end!");
}

/**
 * @tc.name: PowerSuspendControllerTest006
 * @tc.desc: test mapSuspendDeviceType
 * @tc.type: FUNC
 * @tc.require: issueI7G6OY
 */
HWTEST_F(PowerSuspendControllerTest, PowerSuspendControllerTest006, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerSuspendControllerTest006 function start!");
    GTEST_LOG_(INFO) << "PowerSuspendControllerTest006: start";
    g_service->SuspendControllerInit();
    std::string key = " ";
    SuspendDeviceType suspendDeviceType = SuspendSources::mapSuspendDeviceType(key);
    EXPECT_TRUE(static_cast<uint32_t>(suspendDeviceType) ==
        static_cast<uint32_t>(SuspendDeviceType::SUSPEND_DEVICE_REASON_MIN));
    GTEST_LOG_(INFO) << "PowerSuspendControllerTest006: end";
    POWER_HILOGI(LABEL_TEST, "PowerSuspendControllerTest006 function end!");
}

/**
 * @tc.name: PowerSuspendControllerTest007
 * @tc.desc: test RecordPowerKeyDown
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
HWTEST_F(PowerSuspendControllerTest, PowerSuspendControllerTest007, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerSuspendControllerTest007 function start!");
    GTEST_LOG_(INFO) << "PowerSuspendControllerTest007: start";
    g_service->SuspendControllerInit();
    g_service->suspendController_->stateMachine_->stateAction_->SetDisplayState(DisplayState::DISPLAY_OFF);
    g_service->suspendController_->RecordPowerKeyDown();
    EXPECT_TRUE(
        g_service->suspendController_->stateMachine_->stateAction_->GetDisplayState() == DisplayState::DISPLAY_OFF);
    g_service->suspendController_->powerkeyDownWhenScreenOff_ = false;
    GTEST_LOG_(INFO) << "PowerSuspendControllerTest007: end";
    POWER_HILOGI(LABEL_TEST, "PowerSuspendControllerTest007 function end!");
}

/**
 * @tc.name: PowerSuspendControllerTest008
 * @tc.desc: test GetPowerkeyDownWhenScreenOff
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
HWTEST_F(PowerSuspendControllerTest, PowerSuspendControllerTest008, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerSuspendControllerTest008 function start!");
    GTEST_LOG_(INFO) << "PowerSuspendControllerTest008: start";

    g_service->SuspendControllerInit();
    bool powerKeyDown = g_service->suspendController_->GetPowerkeyDownWhenScreenOff();
    EXPECT_TRUE(powerKeyDown == false);

    GTEST_LOG_(INFO) << "PowerSuspendControllerTest008: end";
    POWER_HILOGI(LABEL_TEST, "PowerSuspendControllerTest008 function end!");
}

/**
 * @tc.name: PowerSuspendControllerTest009
 * @tc.desc: test ControlListener(Normal)
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
HWTEST_F(PowerSuspendControllerTest, PowerSuspendControllerTest009, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerSuspendControllerTest009 function start!");
    GTEST_LOG_(INFO) << "PowerSuspendControllerTest009: start";

    g_service->SuspendControllerInit();

    g_service->SuspendDevice(
        static_cast<int64_t>(time(nullptr)), SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    g_service->suspendController_->ControlListener(SuspendDeviceType::SUSPEND_DEVICE_REASON_POWER_KEY, 1, 0);
    g_service->WakeupDevice(static_cast<int64_t>(time(nullptr)),
        WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON, "PowerSuspendControllerTest009");

    g_service->suspendController_->stateMachine_->EmplaceInactive();
    g_service->WakeupDevice(static_cast<int64_t>(time(nullptr)),
        WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON, "PowerSuspendControllerTest009");
    g_service->suspendController_->ControlListener(SuspendDeviceType::SUSPEND_DEVICE_REASON_TIMEOUT, 1, 0);
    EXPECT_TRUE(g_service->suspendController_->stateMachine_->GetState() != PowerState::AWAKE);

    g_service->suspendController_->stateMachine_->EmplaceFreeze();
    bool ret = g_service->suspendController_->stateMachine_->SetState(
        PowerState::FREEZE, StateChangeReason::STATE_CHANGE_REASON_UNKNOWN);
    if (ret == false) {
        GTEST_LOG_(INFO) << "PowerSuspendControllerTest009:  FREEZE set  Failed!";
    }
    g_service->suspendController_->ControlListener(SuspendDeviceType::SUSPEND_DEVICE_REASON_POWER_KEY, 1, 0);
    uint32_t tmp = static_cast<uint32_t>(g_service->suspendController_->stateMachine_->GetState());
    GTEST_LOG_(INFO) << "PowerSuspendControllerTest009: get State:" << tmp;
    EXPECT_TRUE(g_service->suspendController_->stateMachine_->GetState() == PowerState::FREEZE);

    GTEST_LOG_(INFO) << "PowerSuspendControllerTest009: end";
    POWER_HILOGI(LABEL_TEST, "PowerSuspendControllerTest009 function end!");
}

/**
 * @tc.name: PowerSuspendControllerTest010
 * @tc.desc: test HandleAction
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
HWTEST_F(PowerSuspendControllerTest, PowerSuspendControllerTest010, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerSuspendControllerTest010 function start!");
    GTEST_LOG_(INFO) << "PowerSuspendControllerTest010: start";
    g_service->SuspendControllerInit();
    g_service->suspendController_->HandleAction(
        SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, static_cast<uint32_t>(SuspendAction::ACTION_NONE));
    g_service->suspendController_->HandleAction(SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION,
        static_cast<uint32_t>(SuspendAction::ACTION_AUTO_SUSPEND));
    EXPECT_TRUE(g_service->suspendController_->stateMachine_->GetState() == PowerState::SLEEP);
    GTEST_LOG_(INFO) << "PowerSuspendControllerTest010: end";
    POWER_HILOGI(LABEL_TEST, "PowerSuspendControllerTest010 function end!");
}

/**
 * @tc.name: PowerSuspendControllerTest011
 * @tc.desc: test HandleForceSleep
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
HWTEST_F(PowerSuspendControllerTest, PowerSuspendControllerTest011, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerSuspendControllerTest011 function start!");
    GTEST_LOG_(INFO) << "PowerSuspendControllerTest011: start";
    g_service->SuspendControllerInit();
    g_service->suspendController_->stateMachine_->controllerMap_.clear();
    g_service->suspendController_->HandleForceSleep(SuspendDeviceType::SUSPEND_DEVICE_REASON_FORCE_SUSPEND);
    EXPECT_TRUE(g_service->suspendController_->stateMachine_->GetState() == PowerState::SLEEP);
    g_service->suspendController_->stateMachine_->InitStateMap();
    GTEST_LOG_(INFO) << "PowerSuspendControllerTest011: end";
    POWER_HILOGI(LABEL_TEST, "PowerSuspendControllerTest011 function end!");
}

/**
 * @tc.name: PowerSuspendControllerTest012
 * @tc.desc: test HandleHibernate
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
HWTEST_F(PowerSuspendControllerTest, PowerSuspendControllerTest012, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerSuspendControllerTest012 function start!");
    GTEST_LOG_(INFO) << "PowerSuspendControllerTest012: start";
    g_service->SuspendControllerInit();
    g_service->suspendController_->stateMachine_->controllerMap_.clear();
    g_service->suspendController_->HandleHibernate(SuspendDeviceType::SUSPEND_DEVICE_REASON_TIMEOUT);
    EXPECT_TRUE(g_service->suspendController_->stateMachine_->GetState() != PowerState::HIBERNATE);
    g_service->suspendController_->stateMachine_->InitStateMap();

    GTEST_LOG_(INFO) << "PowerSuspendControllerTest012: end";
    POWER_HILOGI(LABEL_TEST, "PowerSuspendControllerTest012 function end!");
}

/**
 * @tc.name: PowerSuspendControllerTest013
 * @tc.desc: test HandleShutdown
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
HWTEST_F(PowerSuspendControllerTest, PowerSuspendControllerTest013, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerSuspendControllerTest013 function start!");
    GTEST_LOG_(INFO) << "PowerSuspendControllerTest013: start";
    g_service->SuspendControllerInit();
    EXPECT_TRUE(g_service->suspendController_ != nullptr);
    GTEST_LOG_(INFO) << "PowerSuspendControllerTest013: end";
    POWER_HILOGI(LABEL_TEST, "PowerSuspendControllerTest013 function end!");
}

/**
 * @tc.name: PowerSuspendControllerTest014
 * @tc.desc: test getSourceKeys
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
HWTEST_F(PowerSuspendControllerTest, PowerSuspendControllerTest014, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerSuspendControllerTest014 function start!");
    GTEST_LOG_(INFO) << "PowerSuspendControllerTest014: start";
    std::shared_ptr<SuspendSources> sources = SuspendSourceParser::ParseSources();
    std::vector<std::string> tmp = sources->getSourceKeys();
    EXPECT_TRUE(tmp.size() != 0);
    GTEST_LOG_(INFO) << "PowerSuspendControllerTest014: end";
    POWER_HILOGI(LABEL_TEST, "PowerSuspendControllerTest014 function end!");
}

/**
 * @tc.name: PowerSuspendControllerTest015
 * @tc.desc: test IsForceSleeping
 * @tc.type: FUNC
 * @tc.require: issueICE3O4
 */
HWTEST_F(PowerSuspendControllerTest, PowerSuspendControllerTest015, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerSuspendControllerTest015 function start!");
    GTEST_LOG_(INFO) << "PowerSuspendControllerTest015: start";
    g_service->SuspendControllerInit();
    g_service->suspendController_->ExecSuspendMonitorByReason(SuspendDeviceType::SUSPEND_DEVICE_REASON_POWER_KEY);
    sleep(NEXT_WAIT_TIME_S);
#ifdef POWER_MANAGER_ENABLE_FORCE_SLEEP_BROADCAST
    EXPECT_EQ(g_service->IsForceSleeping(), true);
#else
    EXPECT_EQ(g_service->IsForceSleeping(), false);
#endif
    GTEST_LOG_(INFO) << "PowerSuspendControllerTest015: end";
    POWER_HILOGI(LABEL_TEST, "PowerSuspendControllerTest015 function end!");
}

/**
 * @tc.name: PowerSuspendControllerTest016
 * @tc.desc: test IsForceSleeping
 * @tc.type: FUNC
 * @tc.require: issueICE3O4
 */
HWTEST_F(PowerSuspendControllerTest, PowerSuspendControllerTest016, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerSuspendControllerTest016 function start!");
    GTEST_LOG_(INFO) << "PowerSuspendControllerTest016: start";
    g_service->SuspendControllerInit();
    g_service->suspendController_->ExecSuspendMonitorByReason(SuspendDeviceType::SUSPEND_DEVICE_REASON_SWITCH);
    sleep(NEXT_WAIT_TIME_S);
#ifdef POWER_MANAGER_ENABLE_FORCE_SLEEP_BROADCAST
    EXPECT_EQ(g_service->IsForceSleeping(), true);
#else
    EXPECT_EQ(g_service->IsForceSleeping(), false);
#endif
    GTEST_LOG_(INFO) << "PowerSuspendControllerTest016: end";
    POWER_HILOGI(LABEL_TEST, "PowerSuspendControllerTest016 function end!");
}
} // namespace