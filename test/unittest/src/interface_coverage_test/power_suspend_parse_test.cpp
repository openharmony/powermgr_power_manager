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
#include "power_suspend_parse_test.h"
#include <fstream>
#include <thread>
#include <unistd.h>

#ifdef THERMAL_GTEST
#define private   public
#define protected public
#endif

#include <datetime_ex.h>
#include <input_manager.h>
#include <securec.h>

#include "power_mgr_service.h"
#include "power_state_machine.h"
#include "setting_helper.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;
static sptr<PowerMgrService> g_service;

void PowerSuspendParseTest::SetUpTestCase(void)
{
    g_service = DelayedSpSingleton<PowerMgrService>::GetInstance();
    g_service->OnStart();
}

void PowerSuspendParseTest::TearDownTestCase(void)
{
    g_service->OnStop();
    DelayedSpSingleton<PowerMgrService>::DestroyInstance();
}

namespace {
static const std::string SYSTEM_POWER_SUSPEND_DB_STRING =
    "{ \"powerkey\": { \"action\": 0, \"delayMs\": 1002 }, \"timeout\": { \"action\": 1, \"delayMs\": 2002 }, \"lid\": "
    "{ \"action\": 1, \"delayMs\": 3002 }, \"switch\": { \"action\": 1, \"delayMs\": 4002 } }";
static const std::string SYSTEM_POWER_WAKEUP_FILE = "./power_suspend.json";
static const std::string SYSTEM_POWER_SUSPEND_FILE = "./power_suspend.json";

void TestPowerSuspend(PowerMgrService* pmsTest_)
{
    vector<SuspendSource>::iterator itb = pmsTest_->suspendController_->sourceList_.begin();
    vector<SuspendSource>::iterator ite = pmsTest_->suspendController_->sourceList_.end();
    vector<SuspendSource>::iterator it = itb;

    for (; it != ite; it++) {
        if (it->GetReason() == SuspendDeviceType::SUSPEND_DEVICE_REASON_POWER_KEY) {
            // judge action
            if (it->GetAction() != 1) {
                GTEST_LOG_(INFO) << "PowerSuspendParse001: Failed to parse action";
            }
            // judge delay
            if (it->GetDelay() != 0) {
                GTEST_LOG_(INFO) << "PowerSuspendParse001: Failed to parse delay";
            }
        } else if (it->GetReason() == SuspendDeviceType::SUSPEND_DEVICE_REASON_TIMEOUT) {
            // judge action
            if (it->GetAction() != 1) {
                GTEST_LOG_(INFO) << "PowerSuspendParse001: Failed to parse action";
            }
            // judge delay
            if (it->GetDelay() != 0) {
                GTEST_LOG_(INFO) << "PowerSuspendParse001: Failed to parse delay";
            }
        } else if (it->GetReason() == SuspendDeviceType::SUSPEND_DEVICE_REASON_LID) {
            // judge action
            if (it->GetAction() != 1) {
                GTEST_LOG_(INFO) << "PowerSuspendParse001: Failed to parse action";
            }
            // judge delay
            if (it->GetDelay() != 0) {
                GTEST_LOG_(INFO) << "PowerSuspendParse001: Failed to parse delay";
            }
        } else if (it->GetReason() == SuspendDeviceType::SUSPEND_DEVICE_REASON_SWITCH) {
            // judge action
            if (it->GetAction() != 1) {
                GTEST_LOG_(INFO) << "PowerSuspendParse001: Failed to parse action";
            }
            // judge delay
            if (it->GetDelay() != 0) {
                GTEST_LOG_(INFO) << "PowerSuspendParse001: Failed to parse delay";
            }
        }
    }
}

/**
 * @tc.name: PowerMgrService001
 * @tc.desc: test Suspend source parse
 * @tc.type: FUNC
 */
HWTEST_F(PowerSuspendParseTest, PowerSuspendParse001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerSuspendParse001: start";
    std::string str = SYSTEM_POWER_SUSPEND_FILE;

    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pmsTest_ == nullptr) {
        GTEST_LOG_(INFO) << "PowerSuspendParse001: Failed to get PowerMgrService";
    }

    pmsTest_->Init();
    pmsTest_->SuspendControllerInit();
    pmsTest_->suspendController_->Init();

    TestPowerSuspend(pmsTest_);
    GTEST_LOG_(INFO) << "PowerSuspendParse001:  end";
}
} // namespace