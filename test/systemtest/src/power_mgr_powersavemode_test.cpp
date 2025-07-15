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

#include <bundle_mgr_proxy.h>
#include <condition_variable>
#include <datetime_ex.h>
#include <gtest/gtest.h>
#include <if_system_ability_manager.h>
#include <iostream>
#include <ipc_skeleton.h>
#include <mutex>
#include <string_ex.h>

#include "common_event_manager.h"
#include "ipower_mode_callback.h"
#include "power_common.h"
#include "power_mgr_client.h"
#include "power_mgr_powersavemode_test.h"
#include "power_mgr_service.h"
#include "power_state_machine.h"
#include "power_state_machine_info.h"
#include "running_lock.h"
#include "running_lock_info.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS::EventFwk;
using namespace OHOS;
using namespace std;

namespace {
std::condition_variable g_cv;
std::mutex g_mtx;
std::string g_action = "";
constexpr int64_t TIME_OUT = 1;
} // namespace

static uint32_t g_i = 0;
static int g_judgeNum = 2;

void PowerMgrPowerSavemodeTest::PowerModeTest1Callback::OnPowerModeChanged(PowerMode mode)
{
    POWER_HILOGI(LABEL_TEST, "PowerModeTest1Callback::OnPowerModeChanged.");
}

void PowerMgrPowerSavemodeTest::SetUpTestCase(void)
{
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    pms->OnStart();
    SystemAbility::MakeAndRegisterAbility(pms.GetRefPtr());
}

namespace {
/**
 * @tc.name: PowerSavemode_001
 * @tc.desc: test SetDeviceMode in proxy
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrPowerSavemodeTest, PowerSavemode_001, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerSavemode_001: SetDeviceMode start.";
    POWER_HILOGI(LABEL_TEST, "PowerSavemode_001 function start!");

    auto& powerMgrClient = PowerMgrClient::GetInstance();
    PowerMode modeFirst = PowerMode::POWER_SAVE_MODE;
    PowerMode modeSecond = PowerMode::POWER_SAVE_MODE;
    powerMgrClient.SetDeviceMode(modeFirst);
    modeFirst = powerMgrClient.GetDeviceMode();
    EXPECT_EQ(modeFirst, modeSecond) << "PowerSavemode_001 fail to SetDeviceMode";
    POWER_HILOGI(LABEL_TEST, "PowerSavemode_001 function end!");
    GTEST_LOG_(INFO) << "PowerSavemode_001: SetDeviceMode end.";
}

/**
 * @tc.name: PowerSavemode_002
 * @tc.desc: test SetDeviceMode in proxy
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrPowerSavemodeTest, PowerSavemode_002, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerSavemode_002: SetDeviceMode start.";
    POWER_HILOGI(LABEL_TEST, "PowerSavemode_002 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    PowerMode modeFirst = PowerMode::PERFORMANCE_MODE;
    PowerMode modeSecond = PowerMode::PERFORMANCE_MODE;
    powerMgrClient.SetDeviceMode(modeFirst);
    modeFirst = powerMgrClient.GetDeviceMode();
    EXPECT_EQ(modeFirst, modeSecond) << "PowerSavemode_002 fail to SetDeviceMode";
    POWER_HILOGI(LABEL_TEST, "PowerSavemode_002 function end!");
    GTEST_LOG_(INFO) << "PowerSavemode_002: SetDeviceMode end.";
}

/**
 * @tc.name: PowerSavemode_003
 * @tc.desc: test SetDeviceMode in proxy
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrPowerSavemodeTest, PowerSavemode_003, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerSavemode_003: SetDeviceMode start.";
    POWER_HILOGI(LABEL_TEST, "PowerSavemode_003 function start!");
    int PARM_ONE = 1;
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    PowerMode modeFirst = static_cast<PowerMode>(PARM_ONE); // modeFirst equal to 1
    PowerMode modeSecond = static_cast<PowerMode>(PARM_ONE);
    powerMgrClient.SetDeviceMode(modeFirst);
    modeFirst = powerMgrClient.GetDeviceMode();
    EXPECT_NE(modeFirst, modeSecond) << "PowerSavemode_003 fail to SetDeviceMode abnormal";
    POWER_HILOGI(LABEL_TEST, "PowerSavemode_003 function end!");
    GTEST_LOG_(INFO) << "PowerSavemode_003: SetDeviceMode end.";
}

/**
 * @tc.name: PowerSavemode_004
 * @tc.desc: test GetDeviceMode in proxy
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrPowerSavemodeTest, PowerSavemode_004, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerSavemode_004: GetDeviceMode start.";
    POWER_HILOGI(LABEL_TEST, "PowerSavemode_004 function start!");
    int PARM_ZERO = 0;
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    PowerMode modeFirst = static_cast<PowerMode>(PARM_ZERO);
    PowerMode modeSecond = PowerMode::POWER_SAVE_MODE;
    modeFirst = powerMgrClient.GetDeviceMode();
    EXPECT_NE(modeFirst, modeSecond) << "PowerSavemode_004 fail to GetDeviceMode";
    POWER_HILOGI(LABEL_TEST, "PowerSavemode_004 function end!");
    GTEST_LOG_(INFO) << "PowerSavemode_004: GetDeviceMode end.";
}

/**
 * @tc.name: PowerSavemode_005
 * @tc.desc: test GetDeviceMode in proxy
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrPowerSavemodeTest, PowerSavemode_005, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerSavemode_005: GetDeviceMode start.";
    POWER_HILOGI(LABEL_TEST, "PowerSavemode_005 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    PowerMode modeFirst = PowerMode::POWER_SAVE_MODE;
    PowerMode modeSecond = PowerMode::POWER_SAVE_MODE;
    powerMgrClient.SetDeviceMode(modeFirst);
    for (int i = 0; i < 100; i++) {
        modeFirst = powerMgrClient.GetDeviceMode();
    }
    EXPECT_EQ(modeFirst, modeSecond) << "PowerSavemode_005 fail to GetDeviceMode";
    POWER_HILOGI(LABEL_TEST, "PowerSavemode_005 function end!");
    GTEST_LOG_(INFO) << "PowerSavemode_005: GetDeviceMode end.";
}

/**
 * @tc.name: PowerSavemode_006
 * @tc.desc: test PowerModeCallback
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrPowerSavemodeTest, PowerSavemode_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PowerSavemode_006: RegisterPowerModeCallback start.";
    POWER_HILOGI(LABEL_TEST, "PowerSavemode_006 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    const sptr<IPowerModeCallback> callBack = new PowerModeTest1Callback();
    powerMgrClient.RegisterPowerModeCallback(callBack);
    PowerMode modeFirst = PowerMode::POWER_SAVE_MODE;
    PowerMode modeSecond = PowerMode::POWER_SAVE_MODE;
    powerMgrClient.SetDeviceMode(modeFirst);
    modeFirst = powerMgrClient.GetDeviceMode();
    EXPECT_EQ(modeFirst, modeSecond) << "PowerSavemode_006 fail to PowerModeCallback";
    POWER_HILOGI(LABEL_TEST, "PowerSavemode_006 function end!");
    GTEST_LOG_(INFO) << "PowerSavemode_006: RegisterPowerModeCallback end.";
}

/**
 * @tc.name: PowerSavemode_007
 * @tc.desc: test PowerModeCallback
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrPowerSavemodeTest, PowerSavemode_007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PowerSavemode_007: RegisterPowerModeCallback start.";
    POWER_HILOGI(LABEL_TEST, "PowerSavemode_007 function start!");
    int PARM_ONE = 1;
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    const sptr<IPowerModeCallback> callBack = new PowerModeTest1Callback();
    powerMgrClient.RegisterPowerModeCallback(callBack);

    PowerMode modeFirst = static_cast<PowerMode>(PARM_ONE);
    PowerMode modeSecond = static_cast<PowerMode>(PARM_ONE);
    powerMgrClient.SetDeviceMode(modeFirst);
    modeFirst = powerMgrClient.GetDeviceMode();
    EXPECT_NE(modeFirst, modeSecond) << "PowerSavemode_007 fail to PowerModeCallback";
    POWER_HILOGI(LABEL_TEST, "PowerSavemode_007 function end!");
    GTEST_LOG_(INFO) << "PowerSavemode_007: RegisterPowerModeCallback end.";
}

/**
 * @tc.name: PowerSavemode_008
 * @tc.desc: test PowerModeCallback
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrPowerSavemodeTest, PowerSavemode_008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PowerSavemode_008: RegisterPowerModeCallback start.";
    POWER_HILOGI(LABEL_TEST, "PowerSavemode_008 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    const sptr<IPowerModeCallback> callBack = new PowerModeTest1Callback();
    for (int i = 0; i < 100; i++) {
        powerMgrClient.RegisterPowerModeCallback(callBack);
    }
    PowerMode modeFirst = PowerMode::POWER_SAVE_MODE;
    PowerMode modeSecond = PowerMode::POWER_SAVE_MODE;
    powerMgrClient.SetDeviceMode(modeFirst);
    modeFirst = powerMgrClient.GetDeviceMode();
    EXPECT_EQ(modeFirst, modeSecond) << "PowerSavemode_008 fail to PowerModeCallback";
    POWER_HILOGI(LABEL_TEST, "PowerSavemode_008 function end!");
    GTEST_LOG_(INFO) << "PowerSavemode_008: RegisterPowerModeCallback end.";
}

/**
 * @tc.name: PowerSavemode_009
 * @tc.desc: test PowerModeCallback
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrPowerSavemodeTest, PowerSavemode_009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PowerSavemode_009: UnRegisterPowerModeCallback start.";
    POWER_HILOGI(LABEL_TEST, "PowerSavemode_009 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    const sptr<IPowerModeCallback> callBack = new PowerModeTest1Callback();
    powerMgrClient.UnRegisterPowerModeCallback(callBack);
    PowerMode modeFirst = PowerMode::POWER_SAVE_MODE;
    PowerMode modeSecond = PowerMode::POWER_SAVE_MODE;
    powerMgrClient.SetDeviceMode(modeFirst);
    modeFirst = powerMgrClient.GetDeviceMode();
    EXPECT_EQ(modeFirst, modeSecond) << "PowerSavemode_009 fail to PowerModeCallback";
    POWER_HILOGI(LABEL_TEST, "PowerSavemode_009 function end!");
    GTEST_LOG_(INFO) << "PowerSavemode_009: UnRegisterPowerModeCallback end.";
}

/**
 * @tc.name: PowerSavemode_010
 * @tc.desc: test PowerModeCallback
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrPowerSavemodeTest, PowerSavemode_010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PowerSavemode_010: UnRegisterPowerModeCallback start.";
    POWER_HILOGI(LABEL_TEST, "PowerSavemode_010 function start!");
    int PARM_ONE = 1;
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    const sptr<IPowerModeCallback> callBack = new PowerModeTest1Callback();
    powerMgrClient.UnRegisterPowerModeCallback(callBack);
    PowerMode modeFirst = static_cast<PowerMode>(PARM_ONE);
    PowerMode modeSecond = static_cast<PowerMode>(PARM_ONE);
    powerMgrClient.SetDeviceMode(modeFirst);
    modeFirst = powerMgrClient.GetDeviceMode();
    EXPECT_NE(modeFirst, modeSecond) << "PowerSavemode_010 fail to PowerModeCallback";
    POWER_HILOGI(LABEL_TEST, "PowerSavemode_010 function end!");
    GTEST_LOG_(INFO) << "PowerSavemode_010: UnRegisterPowerModeCallback end.";
}

/**
 * @tc.name: PowerSavemode_011
 * @tc.desc: test PowerModeCallback
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrPowerSavemodeTest, PowerSavemode_011, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PowerSavemode_011: UnRegisterPowerModeCallback start.";
    POWER_HILOGI(LABEL_TEST, "PowerSavemode_011 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    const sptr<IPowerModeCallback> callBack = new PowerModeTest1Callback();
    for (int i = 0; i < 100; i++) {
        powerMgrClient.UnRegisterPowerModeCallback(callBack);
    }
    PowerMode modeFirst = PowerMode::POWER_SAVE_MODE;
    PowerMode modeSecond = PowerMode::POWER_SAVE_MODE;
    powerMgrClient.SetDeviceMode(modeFirst);
    modeFirst = powerMgrClient.GetDeviceMode();
    EXPECT_EQ(modeFirst, modeSecond) << "PowerSavemode_011 fail to PowerModeCallback";
    POWER_HILOGI(LABEL_TEST, "PowerSavemode_011 function end!");
    GTEST_LOG_(INFO) << "PowerSavemode_011: UnRegisterPowerModeCallback end.";
}

/**
 * @tc.name: PowerSavemode_012
 * @tc.desc: test SetDeviceMode in proxy
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrPowerSavemodeTest, PowerSavemode_012, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerSavemode_012: SetDeviceMode start.";
    POWER_HILOGI(LABEL_TEST, "PowerSavemode_012 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    PowerMode modeFirst = PowerMode::CUSTOM_POWER_SAVE_MODE;
    PowerMode modeSecond = PowerMode::CUSTOM_POWER_SAVE_MODE;
    powerMgrClient.SetDeviceMode(modeFirst);
    modeFirst = powerMgrClient.GetDeviceMode();
    EXPECT_EQ(modeFirst, modeSecond) << "PowerSavemode_012 fail to SetDeviceMode";
    POWER_HILOGI(LABEL_TEST, "PowerSavemode_012 function end!");
    GTEST_LOG_(INFO) << "PowerSavemode_012: SetDeviceMode end.";
}

/**
 * @tc.name: PowerSavemode_013
 * @tc.desc: test SetDeviceMode in proxy
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrPowerSavemodeTest, PowerSavemode_013, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerSavemode_013: SetDeviceMode start.";
    POWER_HILOGI(LABEL_TEST, "PowerSavemode_013 function start!");
    int PARM_ONE = 606;
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    const sptr<IPowerModeCallback> callBack = new PowerModeTest1Callback();
    powerMgrClient.UnRegisterPowerModeCallback(callBack);
    PowerMode modeFirst = static_cast<PowerMode>(PARM_ONE);
    PowerMode modeSecond = static_cast<PowerMode>(PARM_ONE);
    powerMgrClient.SetDeviceMode(modeFirst);
    modeFirst = powerMgrClient.GetDeviceMode();
    EXPECT_NE(modeFirst, modeSecond) << "PowerSavemode_013 fail to PowerModeCallback";
    POWER_HILOGI(LABEL_TEST, "PowerSavemode_013 function end!");
    GTEST_LOG_(INFO) << "PowerSavemode_013: SetDeviceMode end.";
}

/**
 * @tc.name: PowerSavemode_014
 * @tc.desc: test SetDeviceMode in proxy
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrPowerSavemodeTest, PowerSavemode_014, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerSavemode_014: SetDeviceMode start.";
    POWER_HILOGI(LABEL_TEST, "PowerSavemode_014 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    PowerMode modeFirst = PowerMode::EXTREME_POWER_SAVE_MODE;
    PowerMode modeSecond = PowerMode::EXTREME_POWER_SAVE_MODE;
    powerMgrClient.SetDeviceMode(modeFirst);
    modeFirst = powerMgrClient.GetDeviceMode();
    PowerMode normalMode = PowerMode::NORMAL_MODE; // Depends on the battery capacity
    EXPECT_TRUE(modeFirst == modeSecond || modeFirst == normalMode) <<
        "PowerSavemode_014 fail to SetDeviceMode";
    POWER_HILOGI(LABEL_TEST, "PowerSavemode_014 function end!");
    GTEST_LOG_(INFO) << "PowerSavemode_014: SetDeviceMode end.";
}

/**
 * @tc.name: PowerSavemode_015
 * @tc.desc: test SetDeviceMode in proxy
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrPowerSavemodeTest, PowerSavemode_015, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerSavemode_015: SetDeviceMode start.";
    POWER_HILOGI(LABEL_TEST, "PowerSavemode_015 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    PowerMode modeFirst = PowerMode::EXTREME_POWER_SAVE_MODE;
    PowerMode modeSecond = PowerMode::EXTREME_POWER_SAVE_MODE;
    for (int i = 0; i < 100; i++) {
        powerMgrClient.SetDeviceMode(modeFirst);
    }
    modeFirst = powerMgrClient.GetDeviceMode();
    PowerMode normalMode = PowerMode::NORMAL_MODE; // Depends on the battery capacity
    EXPECT_TRUE(modeFirst == modeSecond || modeFirst == normalMode) <<
        "PowerSavemode_015 fail to SetDeviceMode";
    POWER_HILOGI(LABEL_TEST, "PowerSavemode_015 function end!");
    GTEST_LOG_(INFO) << "PowerSavemode_015: SetDeviceMode end.";
}
} // namespace