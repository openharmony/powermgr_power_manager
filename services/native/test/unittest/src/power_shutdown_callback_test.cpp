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

#include "power_shutdown_callback_test.h"

#include <iostream>

#include <datetime_ex.h>
#include <gtest/gtest.h>
#include <if_system_ability_manager.h>
#include <ipc_skeleton.h>
#include <string_ex.h>

#include "power_common.h"
#include "power_mgr_client.h"
#include "power_mgr_service.h"
#include "power_state_machine.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

void PowerShutdownCallbackTest::PowerShutdownTest1Callback::ShutdownCallback()
{
    POWER_HILOGD(LABEL_TEST, "PowerShutdownTest1Callback::ShutdownCallback.");
}

void PowerShutdownCallbackTest::PowerShutdownTest2Callback::ShutdownCallback()
{
    POWER_HILOGD(LABEL_TEST, "PowerShutdownTest2Callback::ShutdownCallback.");
}

namespace {
/**
 * @tc.name: PowerShutdownCallback001
 * @tc.desc: test ShutdownCallback
 * @tc.type: FUNC
 */
HWTEST_F (PowerShutdownCallbackTest, PowerShutdownCallback001, TestSize.Level0)
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    sptr<IShutdownCallback> cb1 = new PowerShutdownTest1Callback();
    powerMgrClient.RegisterShutdownCallback(cb1);
    sleep(SLEEP_WAIT_TIME_S);
    if (false) {
        powerMgrClient.ShutDownDevice(string("ShutDownDeviceTest001"));
    }
    POWER_HILOGD(LABEL_TEST, "PowerShutdownCallback001 1.");
}
}