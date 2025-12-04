/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

void PowerModeModuleNativeTest::SetUpTestCase()
{
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    pmsTest->OnStart();
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
HWTEST_F (PowerModeModuleNativeTest, PowerModeModuleNativeTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PowerModeModuleNativeTest001 start.";
    POWER_HILOGI(LABEL_TEST, "PowerModeModuleNativeTest001 function start!");
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
    EXPECT_EQ(mode, powerModeModuleTest->GetModeItem());
    mode = static_cast<PowerMode>(POWERMODE);
    powerModeModuleTest->SetModeItem(mode);
    mode = static_cast<PowerMode>(POWERMODE2);
    powerModeModuleTest->SetModeItem(mode);
    mode = static_cast<PowerMode>(POWERMODE3);
    powerModeModuleTest->SetModeItem(mode);

    shared_ptr<PowerModeModule::CallbackManager> powerModeModuleManager =
                                            make_shared<PowerModeModule::CallbackManager>();
    powerModeModuleManager->OnRemoteDied(nullptr);

    POWER_HILOGI(LABEL_TEST, "PowerModeModuleNativeTest001 function end!");
    GTEST_LOG_(INFO) << "PowerModeModuleNativeTest001 end.";
}

/**
 * @tc.name: PowerModeModuleNativeTest002
 * @tc.desc: test PowerModeModule policy action
 * @tc.type: FUNC
 */
HWTEST_F (PowerModeModuleNativeTest, PowerModeModuleNativeTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PowerModeModuleNativeTest002 start.";
    POWER_HILOGI(LABEL_TEST, "PowerModeModuleNativeTest002 function start!");
    auto modePolicy = DelayedSingleton<PowerModePolicy>::GetInstance();
    std::shared_ptr<PowerModeModule> powerModeModuleTest = make_shared<PowerModeModule>();
    powerModeModuleTest->EnableMode(PowerMode::PERFORMANCE_MODE, false);
    EXPECT_EQ(powerModeModuleTest->GetModeItem(), PowerMode::PERFORMANCE_MODE);
    modePolicy->switchMap_.clear();
    modePolicy->switchMap_.emplace(PowerModePolicy::ServiceType::SOC_PERF, PowerModeModule::SocPerformance::STANDARD);
    powerModeModuleTest->SetSocPerfState(false);
    modePolicy->switchMap_[PowerModePolicy::ServiceType::SOC_PERF] = PowerModeModule::SocPerformance::HIGH;
    powerModeModuleTest->SetSocPerfState(false);
    constexpr int32_t INVALID_SOC_PERFORMANCE_CODE = 3;
    modePolicy->switchMap_[PowerModePolicy::ServiceType::SOC_PERF] = INVALID_SOC_PERFORMANCE_CODE;
    powerModeModuleTest->SetSocPerfState(false);
    POWER_HILOGI(LABEL_TEST, "PowerModeModuleNativeTest002 function end!");
    GTEST_LOG_(INFO) << "PowerModeModuleNativeTest002 end.";
}

/**
 * @tc.name: PowerModePolicyNativeTest001
 * @tc.desc: test GetPowerMode in PowerModePolicy
 * @tc.type: FUNC
 */
HWTEST_F (PowerModeModuleNativeTest, PowerModePolicyNativeTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PowerModePolicyNativeTest001 start.";
    POWER_HILOGI(LABEL_TEST, "PowerModePolicyNativeTest001 function start!");
    PowerModePolicy *powerModePolicyTest = new PowerModePolicy();
    EXPECT_TRUE(powerModePolicyTest->GetPowerModeValuePolicy(MODEITEM) == INIT_VALUE_FALSE);
    EXPECT_TRUE(powerModePolicyTest->GetPolicyFromMap(MODEITEM) == INIT_VALUE_FALSE);
    powerModePolicyTest->ReadPowerModePolicy(READODE);
    powerModePolicyTest->ComparePowerModePolicy();
    ModeActionPolicy action;
    powerModePolicyTest->AddAction(MODEITEM, action);
    POWER_HILOGI(LABEL_TEST, "PowerModePolicyNativeTest001 function end!");
    GTEST_LOG_(INFO) << "PowerModePolicyNativeTest001 end.";
}
}
