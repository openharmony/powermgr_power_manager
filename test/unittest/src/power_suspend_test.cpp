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

    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pmsTest_ == nullptr) {
        GTEST_LOG_(INFO) << "PowerSuspend001: Failed to get PowerMgrService";
    }

    pmsTest_->Init();
    pmsTest_->SuspendControllerInit();

    sptr<SuspendPowerStateCallback> callback = new SuspendPowerStateCallback(pmsTest_->suspendController_);
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
    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pmsTest_ == nullptr) {
        GTEST_LOG_(INFO) << "PowerSuspendTest002: Failed to get PowerMgrService";
    }
    pmsTest_->Init();
    pmsTest_->SuspendControllerInit();
    pmsTest_->suspendController_->ExecSuspendMonitorByReason(SuspendDeviceType ::SUSPEND_DEVICE_REASON_POWER_KEY);

    auto monitor = pmsTest_->suspendController_->monitorMap_[SuspendDeviceType ::SUSPEND_DEVICE_REASON_POWER_KEY];
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
    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pmsTest_ == nullptr) {
        GTEST_LOG_(INFO) << "PowerSuspendTest003: Failed to get PowerMgrService";
    }

    pmsTest_->Init();
    pmsTest_->SuspendControllerInit();

    pmsTest_->suspendController_->RegisterSettingsObserver();
    EXPECT_TRUE(pmsTest_->suspendController_ != nullptr);

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
    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pmsTest_ == nullptr) {
        GTEST_LOG_(INFO) << "PowerSuspendTest004: Failed to get PowerMgrService";
    }

    pmsTest_->Init();
    pmsTest_->SuspendControllerInit();
    pmsTest_->suspendController_->Execute();
    EXPECT_TRUE(pmsTest_->suspendController_ != nullptr);
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
    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pmsTest_ == nullptr) {
        GTEST_LOG_(INFO) << "PowerSuspendTest005: Failed to get PowerMgrService";
    }

    pmsTest_->Init();
    pmsTest_->SuspendControllerInit();

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
    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pmsTest_ == nullptr) {
        GTEST_LOG_(INFO) << "PowerSuspendTest006: Failed to get PowerMgrService";
    }

    pmsTest_->Init();
    pmsTest_->SuspendControllerInit();
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

    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pmsTest_ == nullptr) {
        GTEST_LOG_(INFO) << "PowerSuspendTest007: Failed to get PowerMgrService";
    }

    pmsTest_->Init();
    pmsTest_->SuspendControllerInit();
    pmsTest_->suspendController_->stateMachine_->stateAction_->SetDisplayState(DisplayState::DISPLAY_OFF);
    pmsTest_->suspendController_->RecordPowerKeyDown();
    EXPECT_TRUE(
        pmsTest_->suspendController_->stateMachine_->stateAction_->GetDisplayState() == DisplayState::DISPLAY_OFF);
    pmsTest_->suspendController_->powerkeyDownWhenScreenOff_ = false;
    
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

    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pmsTest_ == nullptr) {
        GTEST_LOG_(INFO) << "PowerSuspendTest008: Failed to get PowerMgrService";
    }

    pmsTest_->Init();
    pmsTest_->SuspendControllerInit();
    bool powerKeyDown = pmsTest_->suspendController_->GetPowerkeyDownWhenScreenOff();
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

    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pmsTest_ == nullptr) {
        GTEST_LOG_(INFO) << "PowerSuspendTest009: Failed to get PowerMgrService";
    }

    pmsTest_->Init();
    pmsTest_->SuspendControllerInit();

    pmsTest_->suspendController_->stateMachine_->stateAction_->SetDisplayState(DisplayState::DISPLAY_OFF);
    pmsTest_->suspendController_->ControlListener(SuspendDeviceType ::SUSPEND_DEVICE_REASON_POWER_KEY, 1, 0);
    pmsTest_->suspendController_->stateMachine_->stateAction_->SetDisplayState(DisplayState::DISPLAY_ON);

    pmsTest_->suspendController_->stateMachine_->EmplaceInactive();
    pmsTest_->suspendController_->stateMachine_->SetState(
        PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_TIMEOUT);
    pmsTest_->suspendController_->ControlListener(SuspendDeviceType::SUSPEND_DEVICE_REASON_TIMEOUT, 1, 0);
    EXPECT_TRUE(pmsTest_->suspendController_->stateMachine_->GetState() != PowerState::AWAKE);

    pmsTest_->suspendController_->stateMachine_->EmplaceFreeze();
    bool ret = pmsTest_->suspendController_->stateMachine_->SetState(
        PowerState::FREEZE, StateChangeReason::STATE_CHANGE_REASON_TIMEOUT);
    if (ret == false) {
        GTEST_LOG_(INFO) << "PowerSuspendTest009:  FREEZE set  Failed!";
    }
    pmsTest_->suspendController_->ControlListener(SuspendDeviceType ::SUSPEND_DEVICE_REASON_POWER_KEY, 1, 0);
    uint32_t tmp = static_cast<uint32_t>(pmsTest_->suspendController_->stateMachine_->GetState());
    GTEST_LOG_(INFO) << "PowerSuspendTest009: get State:" << tmp;
    EXPECT_TRUE(pmsTest_->suspendController_->stateMachine_->GetState() == PowerState::FREEZE);

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

    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pmsTest_ == nullptr) {
        GTEST_LOG_(INFO) << "PowerSuspendTest011: Failed to get PowerMgrService";
    }

    pmsTest_->Init();
    pmsTest_->SuspendControllerInit();
    pmsTest_->suspendController_->HandleAction(
        SuspendDeviceType::SUSPEND_DEVICE_REASON_TIMEOUT, static_cast<uint32_t>(SuspendAction::ACTION_NONE));
    pmsTest_->suspendController_->HandleAction(
        SuspendDeviceType::SUSPEND_DEVICE_REASON_TIMEOUT, static_cast<uint32_t>(SuspendAction::ACTION_AUTO_SUSPEND));
    EXPECT_TRUE(pmsTest_->suspendController_->stateMachine_->GetState() == PowerState::SLEEP);
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

    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pmsTest_ == nullptr) {
        GTEST_LOG_(INFO) << "PowerSuspendTest012: Failed to get PowerMgrService";
    }

    pmsTest_->Init();
    pmsTest_->SuspendControllerInit();
    pmsTest_->suspendController_->stateMachine_->controllerMap_.clear();
    pmsTest_->suspendController_->HandleForceSleep(SuspendDeviceType::SUSPEND_DEVICE_REASON_TIMEOUT);
    EXPECT_TRUE(pmsTest_->suspendController_->stateMachine_->GetState() == PowerState::SLEEP);
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

    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pmsTest_ == nullptr) {
        GTEST_LOG_(INFO) << "PowerSuspendTest013: Failed to get PowerMgrService";
    }

    pmsTest_->Init();
    pmsTest_->SuspendControllerInit();
    pmsTest_->suspendController_->stateMachine_->controllerMap_.clear();
    pmsTest_->suspendController_->HandleHibernate(SuspendDeviceType::SUSPEND_DEVICE_REASON_TIMEOUT);
    EXPECT_TRUE(pmsTest_->suspendController_->stateMachine_->GetState() != PowerState::HIBERNATE);

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

    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pmsTest_ == nullptr) {
        GTEST_LOG_(INFO) << "PowerSuspendTest014: Failed to get PowerMgrService";
    }

    pmsTest_->Init();
    pmsTest_->SuspendControllerInit();
    EXPECT_TRUE(pmsTest_->suspendController_ != nullptr);

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