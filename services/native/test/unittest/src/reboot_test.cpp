/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include <iostream>
#include <datetime_ex.h>
#include <gtest/gtest.h>
#include <if_system_ability_manager.h>
#include <ipc_skeleton.h>
#include <string_ex.h>
#include "power_reboot_test.h"
#include "power_common.h"
#include "power_mgr_client.h"
#include "power_mgr_service.h"
#include "power_state_machine.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;
namespace {
/**
 * @tc.name: RebootDeviceTest001
 * @tc.desc: test RebootDevice in proxy
 * @tc.type: FUNC
 */
HWTEST_F (PowerRebootTest, RebootDeviceTest001, TestSize.Level2)
{
    sleep(SLEEP_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "RebootDeviceTest001: RebootDevice start.";
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    if (false) {
        powerMgrClient.RebootDevice(string("RebootDeviceTest001"));
    }
    GTEST_LOG_(INFO) << "RebootDeviceTest001: RebootDevice end.";
}

/**
 * @tc.name: RebootDeviceTest002
 * @tc.desc: test RebootDeviceForDeprecated in proxy
 * @tc.type: FUNC
 */
HWTEST_F (PowerRebootTest, RebootDeviceTest002, TestSize.Level2)
{
    sleep(SLEEP_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "RebootDeviceTest002: RebootDeviceForDeprecated start.";
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    if (false) {
        powerMgrClient.RebootDeviceForDeprecated(string("RebootDeviceTest002"));
    }
    GTEST_LOG_(INFO) << "RebootDeviceTest002: RebootDeviceForDeprecated end.";
}
}