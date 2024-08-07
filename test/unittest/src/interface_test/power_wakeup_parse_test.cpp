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
#include "power_wakeup_parse_test.h"
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

void PowerWakeupParseTest::SetUpTestCase(void)
{
    g_service = DelayedSpSingleton<PowerMgrService>::GetInstance();
    g_service->OnStart();
}

void PowerWakeupParseTest::TearDownTestCase(void)
{
    g_service->OnStop();
    DelayedSpSingleton<PowerMgrService>::DestroyInstance();
}

namespace {
static const constexpr uint32_t DOUBLC_CLICK_INNER = 2;
static const std::string SYSTEM_POWER_WAKEUP_DB_STRING =
    "{\"powerkey\": {\"enable\": false},\"keyborad\": {\"enable\": false},\"mouse\": {\"enable\": "
    "false},\"touchscreen\": {\"enable\": false,\"click\": 2},\"touchpad\": {\"enable\": false},\"pen\": {\"enable\": "
    "false},\"lid\": {\"enable\": false},\"switch\": {\"enable\": false}}";
static const std::string SYSTEM_POWER_WAKEUP_FILE = "./power_wakeup.json";

void TestPowerWakeup(PowerMgrService* pmsTest_)
{
    vector<WakeupSource>::iterator itb = pmsTest_->wakeupController_->sourceList_.begin();
    vector<WakeupSource>::iterator ite = pmsTest_->wakeupController_->sourceList_.end();
    vector<WakeupSource>::iterator it = itb;

    for (; it != ite; it++) {
        if (it->GetReason() == WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON) {
            if (it->IsEnable() != true) {
                GTEST_LOG_(INFO) << "PowerWakeupParse001: Failed to parse Enable";
            }
        } else if (it->GetReason() == WakeupDeviceType::WAKEUP_DEVICE_KEYBOARD) {
            if (it->IsEnable() != true) {
                GTEST_LOG_(INFO) << "PowerWakeupParse001: Failed to parse Enable";
            }
        } else if (it->GetReason() == WakeupDeviceType::WAKEUP_DEVICE_TOUCH_SCREEN) {
            if (it->IsEnable() != true) {
                GTEST_LOG_(INFO) << "PowerWakeupParse001: Failed to parse Enable";
            }
            if (it->GetClick() != DOUBLC_CLICK_INNER) {
                GTEST_LOG_(INFO) << "PowerWakeupParse001: Failed to parse Click";
            }
        } else if (it->GetReason() == WakeupDeviceType::WAKEUP_DEVICE_TOUCHPAD) {
            if (it->IsEnable() != true) {
                GTEST_LOG_(INFO) << "PowerWakeupParse001: Failed to parse Enable";
            }
        } else if (it->GetReason() == WakeupDeviceType::WAKEUP_DEVICE_PEN) {
            if (it->IsEnable() != true) {
                GTEST_LOG_(INFO) << "PowerWakeupParse001: Failed to parse Enable";
            }
        } else if (it->GetReason() == WakeupDeviceType::WAKEUP_DEVICE_LID) {
            if (it->IsEnable() != true) {
                GTEST_LOG_(INFO) << "PowerWakeupParse001: Failed to parse Enable";
            }
        } else if (it->GetReason() == WakeupDeviceType::WAKEUP_DEVICE_SWITCH) {
            if (it->IsEnable() != true) {
                GTEST_LOG_(INFO) << "PowerWakeupParse001: Failed to parse Enable";
            }
        } else if (it->GetReason() == WakeupDeviceType::WAKEUP_DEVICE_MOUSE) {
            if (it->IsEnable() != true) {
                GTEST_LOG_(INFO) << "PowerWakeupParse001: Failed to parse Enable";
            }
        }
    }
}

/**
 * @tc.name: PowerWakeupParse001
 * @tc.desc: test Wakeup source parse
 * @tc.type: FUNC
 */
HWTEST_F(PowerWakeupParseTest, PowerWakeupParse001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerSuspendParse001: start";
    std::string str = SYSTEM_POWER_WAKEUP_FILE;

    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pmsTest_ == nullptr) {
        GTEST_LOG_(INFO) << "PowerWakeupParse001: Failed to get PowerMgrService";
    }

    pmsTest_->Init();
    pmsTest_->WakeupControllerInit();
    pmsTest_->wakeupController_->Init();

    TestPowerWakeup(pmsTest_);

    GTEST_LOG_(INFO) << "PowerWakeupParse001:  end";
}
} // namespace