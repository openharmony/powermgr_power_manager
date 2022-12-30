/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "power_mode_module_native_test.h"
#include "event_runner.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

void PowerModeModuleNativeTest::SetUpTestCase()
{
}

void PowerModeModuleNativeTest::TearDownTestCase()
{
}

void PowerModeModuleNativeTest::SetUp()
{
}

void PowerModeModuleNativeTest::TearDown()
{
}

void PowerModeTestCallback::OnPowerModeChanged(PowerMode mode)
{
    POWER_HILOGI(LABEL_TEST, "PowerModeTestCallback::OnPowerModeChanged.");
}

namespace {
using ModeActionPolicy = std::function<void(bool)>;

/**
 * @tc.name: PowerModeModuleNativeTest001
 * @tc.desc: test callback in PowerModeModule
 */
HWTEST_F (PowerModeModuleNativeTest, PowerModeModuleNativeTest001, TestSize.Level0)
{
    shared_ptr<PowerModeModule> powerModeModuleTest = make_shared<PowerModeModule>();
    powerModeModuleTest->mode_ = PowerMode::PERFORMANCE_MODE;
    powerModeModuleTest->PublishPowerModeEvent();
    powerModeModuleTest->mode_ = PowerMode::EXTREME_POWER_SAVE_MODE;
    powerModeModuleTest->PublishPowerModeEvent();
    powerModeModuleTest->mode_ = static_cast<PowerMode>(POWERMODE);
    powerModeModuleTest->PublishPowerModeEvent();

    sptr<IPowerModeCallback> object = new PowerModeTestCallback();
    powerModeModuleTest->AddPowerModeCallback(object);
    powerModeModuleTest->Prepare();
    powerModeModuleTest->AddPowerModeCallback(nullptr);
    powerModeModuleTest->DelPowerModeCallback(object);
    powerModeModuleTest->DelPowerModeCallback(nullptr);

    powerModeModuleTest->callbackMgr_ = nullptr;
    powerModeModuleTest->Prepare();
    powerModeModuleTest->AddPowerModeCallback(object);
    powerModeModuleTest->DelPowerModeCallback(object);
    powerModeModuleTest->started_ = true;
    PowerMode mode = PowerMode::PERFORMANCE_MODE;
    powerModeModuleTest->EnableMode(mode, true);

    powerModeModuleTest->mode_ = PowerMode::PERFORMANCE_MODE;
    powerModeModuleTest->SetModeItem(mode);
    mode = static_cast<PowerMode>(POWERMODE);
    powerModeModuleTest->SetModeItem(mode);

    shared_ptr<PowerModeModule::CallbackManager> powerModeModuleManager =
                                            make_shared<PowerModeModule::CallbackManager>();
    powerModeModuleManager->OnRemoteDied(nullptr);

    POWER_HILOGI(LABEL_TEST, "PowerModeModuleNativeTest001 end.");
}

/**
 * @tc.name: PowerEventHandlerNativeTest002
 * @tc.desc: test ProcessEvent in PowerEventHandler
 * @tc.type: FUNC
 */
HWTEST_F (PowerModeModuleNativeTest, PowerEventHandlerNativeTest002, TestSize.Level0)
{
    wptr<PowerMgrService> pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    EXPECT_TRUE(pmsTest != nullptr) << "PowerEventHandlerNativeTest002 fail to get PowerMgrService";

    sptr<IPowerModeCallback> powerModeModuleCb = new PowerModeTestCallback();
    EXPECT_TRUE(pmsTest->RegisterPowerModeCallback(powerModeModuleCb));
    EXPECT_TRUE(pmsTest->UnRegisterPowerModeCallback(powerModeModuleCb));

    shared_ptr<PowermsEventHandler> powerEventHandlerTest = make_shared<PowermsEventHandler>(
        AppExecFwk::EventRunner::Create(true), pmsTest);
    AppExecFwk::InnerEvent::Pointer event =
        AppExecFwk::InnerEvent::Get(GETI, powerEventHandlerTest, GETI);
    event->innerEventId_ = PowermsEventHandler::CHECK_RUNNINGLOCK_OVERTIME_MSG;
    powerEventHandlerTest->ProcessEvent(event);
    event->innerEventId_ = PowermsEventHandler::SCREEN_ON_TIMEOUT_MSG;
    powerEventHandlerTest->ProcessEvent(event);
    event->innerEventId_ = PowermsEventHandler::REGISTER_POWER_HDI_CALLBACK + 1;
    powerEventHandlerTest->ProcessEvent(event);

    shared_ptr<PowermsEventHandler> powerEventHandlerTestN = make_shared<PowermsEventHandler>(
        AppExecFwk::EventRunner::Create(true), nullptr);
    powerEventHandlerTestN->ProcessEvent(event);

    POWER_HILOGI(LABEL_TEST, "PowerEventHandlerNativeTest002 end.");
}

/**
 * @tc.name: PowerModePolicyNativeTest003
 * @tc.desc: test GetPowerMode in PowerModePolicy
 * @tc.type: FUNC
 */
HWTEST_F (PowerModeModuleNativeTest, PowerModePolicyNativeTest003, TestSize.Level0)
{
    PowerModePolicy *powerModePolicyTest = new PowerModePolicy();
    EXPECT_TRUE(powerModePolicyTest->GetPowerModeValuePolicy(MODEITEM) == INIT_VALUE_FALSE);
    EXPECT_TRUE(powerModePolicyTest->GetPolicyFromMap(MODEITEM) == INIT_VALUE_FALSE);
    EXPECT_TRUE(powerModePolicyTest->GetRecoverPolicyFromMap(MODEITEM) == INIT_VALUE_FALSE);
    EXPECT_TRUE(powerModePolicyTest->GetPowerModeRecoverPolicy(MODEITEM) == INIT_VALUE_FALSE);
    powerModePolicyTest->ReadRecoverPolicy(READODE);
    powerModePolicyTest->CompareModeItem(MODEITEM, MODEITEM);
    ModeActionPolicy action;
    powerModePolicyTest->AddAction(MODEITEM, action);
    EXPECT_TRUE(powerModePolicyTest->GetPowerModeRecoverPolicy(MODEITEM) == INIT_VALUE_FALSE);
    EXPECT_TRUE(powerModePolicyTest->GetRecoverPolicyFromMap(MODEITEM) == INIT_VALUE_FALSE);

    POWER_HILOGI(LABEL_TEST, "PowerModePolicyNativeTest003 end.");
}
}
