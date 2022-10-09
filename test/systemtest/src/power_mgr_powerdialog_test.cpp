/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "power_mgr_powerdialog_test.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS::EventFwk;
using namespace OHOS;
using namespace std;

void PowerMgrPowerDialog::SetUpTestCase()
{
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    pms->OnStart();
    SystemAbility::MakeAndRegisterAbility(pms.GetRefPtr());
}

void PowerMgrPowerDialog::TearDownTestCase()
{
}

void PowerMgrPowerDialog::SetUp()
{
}

void PowerMgrPowerDialog::TearDown()
{
}

namespace {
/**
 * @tc.name: PowerMgr_PowerDialog_001
 * @tc.desc: test pull up PowerDialog in ShowPowerDialog
 * @tc.type: FUNC
 * @tc.require: issueI5I9BF
 */
HWTEST_F(PowerMgrPowerDialog, PowerMgr_PowerDialog_001, TestSize.Level2)
{
    auto powerMgrService = DelayedSpSingleton<PowerMgrService>::GetInstance();
    EXPECT_TRUE(powerMgrService->ShowPowerDialog());  
}
}
