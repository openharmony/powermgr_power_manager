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

#include "power_set_mode_test.h"

#include <iostream>

#include <datetime_ex.h>
#include <gtest/gtest.h>
#include <if_system_ability_manager.h>
#include <ipc_skeleton.h>
#include <string>
#include <string_ex.h>

#include "config_policy_utils.h"
#include "power_common.h"
#include "power_mgr_client.h"
#include "power_mgr_service.h"
#include "power_save_mode.h"
#include "power_state_machine.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

char* GetOneCfgFile(const char *pathSuffix, char *buf, unsigned int bufLength)
{
    std::string ret = "/";
    return const_cast<char*>(ret.c_str());
}

namespace {
/**
 * @tc.name: SetModeTest001
 * @tc.desc: test SetDeviceMode in proxy
 * @tc.type: FUNC
 */
HWTEST_F (PowerSetModeTest, SetModeTest001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "SetModeTest001 function start!");
    sleep(SLEEP_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "SetModeTest001: SetMode start.";
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    PowerMode mode1 = PowerMode::POWER_SAVE_MODE;
    if (true) {
        powerMgrClient.SetDeviceMode(mode1);
        EXPECT_EQ(mode1, powerMgrClient.GetDeviceMode());
    }
    sleep(SLEEP_WAIT_TIME_S);
    PowerMode mode2 = PowerMode::PERFORMANCE_MODE;
    if (true) {
        powerMgrClient.SetDeviceMode(mode2);
        EXPECT_EQ(mode2, powerMgrClient.GetDeviceMode());
    }
    sleep(SLEEP_WAIT_TIME_S);
    PowerMode mode3 = PowerMode::EXTREME_POWER_SAVE_MODE;
    if (true) {
        powerMgrClient.SetDeviceMode(mode3);
        EXPECT_EQ(mode3, powerMgrClient.GetDeviceMode());
    }
    POWER_HILOGI(LABEL_TEST, "SetModeTest001 function end!");
    GTEST_LOG_(INFO) << "SetModeTest001: SetMode end.";
}

/**
 * @tc.name: SaveModeTest001
 * @tc.desc: test StartXMlParse
 * @tc.type: FUNC
 */
HWTEST_F (PowerSetModeTest, SaveModeTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SaveModeTest001 start.";
    POWER_HILOGI(LABEL_TEST, "SaveModeTest001 function start!");
    int32_t EXPECT_RESULT = -1;
    auto mode = std::make_shared<PowerSaveMode>();
    int32_t ret = mode->GetSleepTime(0);
    EXPECT_TRUE(ret == EXPECT_RESULT);
    POWER_HILOGI(LABEL_TEST, "SaveModeTest001 function end!");
    GTEST_LOG_(INFO) << "SaveModeTest001 end.";
}
}