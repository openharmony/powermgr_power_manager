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

#include "power_device_mode_test.h"

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


void PowerDeviceModeTest::PowerModeTest1Callback::OnPowerModeChanged(PowerMode mode)
{
    POWER_HILOGI(LABEL_TEST, "PowerModeTest1Callback::OnPowerModeChanged.");
}

void PowerDeviceModeTest::PowerModeTest2Callback::OnPowerModeChanged(PowerMode mode)
{
    POWER_HILOGI(LABEL_TEST, "PowerModeTest2Callback::OnPowerModeChanged.");
}

namespace {
/**
 * @tc.name: PowerStateCallback001
 * @tc.desc: test PowerStateCallback
 * @tc.type: FUNC
 */
HWTEST_F (PowerDeviceModeTest, PowerDeviceModeCallback001, TestSize.Level0)
{
    bool ret = 0;
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    sptr<IPowerModeCallback> cb1 = new PowerModeTest1Callback();
    ret = powerMgrClient.RegisterPowerModeCallback(cb1);
    EXPECT_TRUE(ret);
    ret = 0;
    POWER_HILOGI(LABEL_TEST, "PowerDeviceModeCallback001 1.");
    {
        sptr<IPowerModeCallback> cb2 = new PowerModeTest2Callback();
        ret = powerMgrClient.UnRegisterPowerModeCallback(cb2);
        EXPECT_TRUE(ret);
        ret = 0;
        POWER_HILOGI(LABEL_TEST, "PowerDeviceModeCallback001 2.");
        ret = powerMgrClient.RegisterPowerModeCallback(cb2);
        EXPECT_TRUE(ret);
        ret = 0;
        POWER_HILOGI(LABEL_TEST, "PowerDeviceModeCallback001 3.");
        ret = powerMgrClient.RegisterPowerModeCallback(cb2);
        EXPECT_TRUE(ret);
        ret = 0;
        POWER_HILOGI(LABEL_TEST, "PowerDeviceModeCallback001 4.");
    }
    ret = powerMgrClient.UnRegisterPowerModeCallback(cb1);
    EXPECT_TRUE(ret);
    ret = 0;
    POWER_HILOGI(LABEL_TEST, "PowerDeviceModeTest::PowerDeviceModeCallback001 end.");
}

/**
 * @tc.name: SetDeviceModeTest001
 * @tc.desc: test SetDeviceMode in proxy
 * @tc.type: FUNC
 */
HWTEST_F (PowerDeviceModeTest, SetDeviceModeTest001, TestSize.Level2)
{
    sleep(SLEEP_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "SetDeviceModeTest001: SetDeviceMode start.";
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    PowerMode mode1 = powerMgrClient.GetDeviceMode();
    PowerMode mode = PowerMode::POWER_SAVE_MODE;
    powerMgrClient.SetDeviceMode(mode);
    EXPECT_EQ(powerMgrClient.GetDeviceMode(), mode);
    powerMgrClient.SetDeviceMode(mode1);

    GTEST_LOG_(INFO) << "SetDeviceModeTest001: SetDeviceMode end.";
}

/**
 * @tc.name: GetDeviceModeTest001
 * @tc.desc: test GetDeviceMode in proxy
 * @tc.type: FUNC
 */
HWTEST_F (PowerDeviceModeTest, GetDeviceModeTest001, TestSize.Level2)
{
    PowerMode mode = PowerMode::NORMAL_MODE;
    PowerMode mode1 = PowerMode::NORMAL_MODE;
    PowerMode mode2 = PowerMode::NORMAL_MODE;
    sleep(SLEEP_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "GetDeviceModeTest001: GetDeviceMode start.";
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    mode = powerMgrClient.GetDeviceMode();

    powerMgrClient.SetDeviceMode(mode1);
    mode2 = powerMgrClient.GetDeviceMode();
    EXPECT_EQ(mode1, mode2);
    powerMgrClient.SetDeviceMode(mode);
}
}