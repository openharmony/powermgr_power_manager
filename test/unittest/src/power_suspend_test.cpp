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
#include "power_state_callback_stub.h"
#include "power_state_machine.h"
#include "setting_helper.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;
static sptr<PowerMgrService> g_service;
static constexpr int SLEEP_WAIT_TIME_S = 2;

void PowerSuspendTest::SetUpTestCase(void)
{
    g_service = DelayedSpSingleton<PowerMgrService>::GetInstance();
    g_service->OnStart();
}

void PowerSuspendTest::TearDownTestCase(void)
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
 * @tc.name: PowerSuspendTest001
 * @tc.desc: test OnPowerStateChanged(exception)
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
HWTEST_F(PowerSuspendTest, PowerSuspendTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerSuspend001: start";
    g_service->SuspendControllerInit();
    g_service->WakeupControllerInit();
    sptr<SuspendPowerStateCallback> callback = new SuspendPowerStateCallback(g_service->suspendController_);
    callback->controller_.reset();
    callback->OnPowerStateChanged(PowerState::AWAKE);
    auto controller = callback->controller_.lock();
    EXPECT_EQ(controller, nullptr);
    GTEST_LOG_(INFO) << "PowerSuspendTest001:  end";
}

/**
 * @tc.name: PowerWakeupTest002
 * @tc.desc: test ExecSuspendMonitorByReason(Normal and exception)
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
HWTEST_F(PowerSuspendTest, PowerSuspendTest002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerSuspendTest002: start";
    sleep(SLEEP_WAIT_TIME_S);
    g_service->SuspendControllerInit();
    g_service->suspendController_->ExecSuspendMonitorByReason(SuspendDeviceType ::SUSPEND_DEVICE_REASON_POWER_KEY);
    auto monitor = g_service->suspendController_->monitorMap_[SuspendDeviceType ::SUSPEND_DEVICE_REASON_POWER_KEY];
    EXPECT_TRUE(monitor != nullptr);
    GTEST_LOG_(INFO) << "PowerSuspendTest002:  end";
}

/**
 * @tc.name: PowerSuspendTest003
 * @tc.desc: test RegisterSettingsObserver(exception 1, nothing!)
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
HWTEST_F(PowerSuspendTest, PowerSuspendTest003, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerSuspendTest003: start";
    g_service->SuspendControllerInit();
    g_service->suspendController_->RegisterSettingsObserver();
    EXPECT_TRUE(g_service->suspendController_ != nullptr);

    GTEST_LOG_(INFO) << "PowerSuspendTest003:  end";
}

/**
 * @tc.name: PowerSuspendTest004
 * @tc.desc: test Execute
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
HWTEST_F(PowerSuspendTest, PowerSuspendTest004, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerSuspendTest004: start";
    g_service->SuspendControllerInit();
    g_service->suspendController_->Execute();
    EXPECT_TRUE(g_service->suspendController_ != nullptr);
    GTEST_LOG_(INFO) << "PowerSuspendTest004:  end";
}

/**
 * @tc.name: PowerSuspendTest005
 * @tc.desc: test CreateMonitor
 * @tc.type: FUNC
 * @tc.require: issueI7G6OY
 */
HWTEST_F(PowerSuspendTest, PowerSuspendTest005, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerSuspendTest005: start";
    g_service->SuspendControllerInit();

    SuspendSource source(SuspendDeviceType::SUSPEND_DEVICE_REASON_STR, 1, 0);
    std::shared_ptr<SuspendMonitor> monitor = SuspendMonitor::CreateMonitor(source);
    EXPECT_TRUE(monitor == nullptr);
    GTEST_LOG_(INFO) << "PowerSuspendTest005:  end";
}

/**
 * @tc.name: PowerSuspendTest006
 * @tc.desc: test mapSuspendDeviceType
 * @tc.type: FUNC
 * @tc.require: issueI7G6OY
 */
HWTEST_F(PowerSuspendTest, PowerSuspendTest006, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerSuspendTest006: start";
    g_service->SuspendControllerInit();
    std::string key = " ";
    SuspendDeviceType suspendDeviceType = SuspendSources::mapSuspendDeviceType(key);
    EXPECT_TRUE(static_cast<uint32_t>(suspendDeviceType) ==
        static_cast<uint32_t>(SuspendDeviceType::SUSPEND_DEVICE_REASON_MIN));
    GTEST_LOG_(INFO) << "PowerSuspendTest006:  end";
}

/**
 * @tc.name: PowerSuspendTest007
 * @tc.desc: test RecordPowerKeyDown
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
HWTEST_F(PowerSuspendTest, PowerSuspendTest007, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerSuspendTest007: start";

    g_service->SuspendControllerInit();
    g_service->suspendController_->stateMachine_->stateAction_->SetDisplayState(DisplayState::DISPLAY_OFF);
    g_service->suspendController_->RecordPowerKeyDown();
    EXPECT_TRUE(
        g_service->suspendController_->stateMachine_->stateAction_->GetDisplayState() == DisplayState::DISPLAY_OFF);
    g_service->suspendController_->powerkeyDownWhenScreenOff_ = false;
    
    GTEST_LOG_(INFO) << "PowerSuspendTest007:  end";
}

/**
 * @tc.name: PowerSuspendTest008
 * @tc.desc: test GetPowerkeyDownWhenScreenOff
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
HWTEST_F(PowerSuspendTest, PowerSuspendTest008, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerSuspendTest008: start";

    g_service->SuspendControllerInit();
    bool powerKeyDown = g_service->suspendController_->GetPowerkeyDownWhenScreenOff();
    EXPECT_TRUE(powerKeyDown == false);

    GTEST_LOG_(INFO) << "PowerSuspendTest008:  end";
}

/**
 * @tc.name: PowerSuspendTest009
 * @tc.desc: test ControlListener(Normal)
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
HWTEST_F(PowerSuspendTest, PowerSuspendTest009, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerSuspendTest009: start";

    g_service->SuspendControllerInit();

    g_service->SuspendDevice(
        static_cast<int64_t>(time(nullptr)), SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    g_service->suspendController_->ControlListener(SuspendDeviceType ::SUSPEND_DEVICE_REASON_POWER_KEY, 1, 0);
    g_service->WakeupDevice(
        static_cast<int64_t>(time(nullptr)), WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON, "PowerSuspendTest009");

    g_service->suspendController_->stateMachine_->EmplaceInactive();
    g_service->WakeupDevice(
        static_cast<int64_t>(time(nullptr)), WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON, "PowerSuspendTest009");
    g_service->suspendController_->ControlListener(SuspendDeviceType::SUSPEND_DEVICE_REASON_TIMEOUT, 1, 0);
    EXPECT_TRUE(g_service->suspendController_->stateMachine_->GetState() != PowerState::AWAKE);

    g_service->suspendController_->stateMachine_->EmplaceFreeze();
    bool ret = g_service->suspendController_->stateMachine_->SetState(
        PowerState::FREEZE, StateChangeReason::STATE_CHANGE_REASON_UNKNOWN);
    if (ret == false) {
        GTEST_LOG_(INFO) << "PowerSuspendTest009:  FREEZE set  Failed!";
    }
    g_service->suspendController_->ControlListener(SuspendDeviceType ::SUSPEND_DEVICE_REASON_POWER_KEY, 1, 0);
    uint32_t tmp = static_cast<uint32_t>(g_service->suspendController_->stateMachine_->GetState());
    GTEST_LOG_(INFO) << "PowerSuspendTest009: get State:" << tmp;
    EXPECT_TRUE(g_service->suspendController_->stateMachine_->GetState() == PowerState::FREEZE);

    GTEST_LOG_(INFO) << "PowerSuspendTest009:  end";
}

/**
 * @tc.name: PowerSuspendTest011
 * @tc.desc: test HandleAction
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
HWTEST_F(PowerSuspendTest, PowerSuspendTest011, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerSuspendTest011: start";
    g_service->SuspendControllerInit();
    g_service->suspendController_->HandleAction(
        SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, static_cast<uint32_t>(SuspendAction::ACTION_NONE));
    g_service->suspendController_->HandleAction(SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION,
        static_cast<uint32_t>(SuspendAction::ACTION_AUTO_SUSPEND));
    EXPECT_TRUE(g_service->suspendController_->stateMachine_->GetState() == PowerState::SLEEP);
    GTEST_LOG_(INFO) << "PowerSuspendTest011:  end";
}

/**
 * @tc.name: PowerSuspendTest012
 * @tc.desc: test HandleForceSleep
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
HWTEST_F(PowerSuspendTest, PowerSuspendTest012, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerSuspendTest012: start";
    g_service->SuspendControllerInit();
    g_service->suspendController_->stateMachine_->controllerMap_.clear();
    g_service->suspendController_->HandleForceSleep(SuspendDeviceType::SUSPEND_DEVICE_REASON_FORCE_SUSPEND);
    EXPECT_TRUE(g_service->suspendController_->stateMachine_->GetState() == PowerState::SLEEP);
    g_service->suspendController_->stateMachine_->InitStateMap();
    GTEST_LOG_(INFO) << "PowerSuspendTest012:  end";
}

/**
 * @tc.name: PowerSuspendTest013
 * @tc.desc: test HandleHibernate
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
HWTEST_F(PowerSuspendTest, PowerSuspendTest013, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerSuspendTest013: start";
    g_service->SuspendControllerInit();
    g_service->suspendController_->stateMachine_->controllerMap_.clear();
    g_service->suspendController_->HandleHibernate(SuspendDeviceType::SUSPEND_DEVICE_REASON_TIMEOUT);
    EXPECT_TRUE(g_service->suspendController_->stateMachine_->GetState() != PowerState::HIBERNATE);
    g_service->suspendController_->stateMachine_->InitStateMap();

    GTEST_LOG_(INFO) << "PowerSuspendTest013:  end";
}

/**
 * @tc.name: PowerSuspendTest014
 * @tc.desc: test HandleShutdown
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
HWTEST_F(PowerSuspendTest, PowerSuspendTest014, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerSuspendTest014: start";
    g_service->SuspendControllerInit();
    EXPECT_TRUE(g_service->suspendController_ != nullptr);
    GTEST_LOG_(INFO) << "PowerSuspendTest014:  end";
}

/**
 * @tc.name: PowerSuspendTest016
 * @tc.desc: test getSourceKeys
 * @tc.type: FUNC
 * @tc.require: issueI7COGR
 */
HWTEST_F(PowerSuspendTest, PowerSuspendTest016, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerSuspendTest016: start";
    std::shared_ptr<SuspendSources> sources = SuspendSourceParser::ParseSources();
    std::vector<std::string> tmp = sources->getSourceKeys();
    EXPECT_TRUE(tmp.size() != 0);
    GTEST_LOG_(INFO) << "PowerSuspendTest016:  end";
}
} // namespace