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

#include <bundle_mgr_proxy.h>
#include <datetime_ex.h>
#include <gtest/gtest.h>
#include <if_system_ability_manager.h>
#include <iostream>
#include <ipc_skeleton.h>
#include <string_ex.h>

#include "common_event_manager.h"
#include "ipower_mode_callback.h"
#include "power_common.h"
#include "power_log.h"
#include "power_mgr_client.h"
#include "power_mgr_service.h"
#include "power_mgr_system_test.h"
#include "power_mode_policy.h"
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
static int32_t g_sleepTime;
static int32_t g_getSleepTimeResult;
const int32_t SLEEPTIME_ID = PowerModePolicy::ServiceType::SLEEPTIME;
} // namespace

void PowerMgrSystemTest::SetUpTestCase(void)
{
    system("mount -o rw,remount /vendor");
}

static void UpdateGlobalSleepTime(std::list<ModePolicy>& info)
{
    for (std::list<ModePolicy>::iterator it = info.begin(); it != info.end(); ++it) {
        if (it->id == SLEEPTIME_ID) {
            g_sleepTime = it->value;
        }
    }
}

static void SetPolicyMode(const int32_t& proxyId)
{
    std::map<int32_t, std::list<ModePolicy>> policyCache;
    std::unique_ptr<PowerSaveMode> mode = std::make_unique<PowerSaveMode>();
    policyCache = mode->GetPolicyCache();
    for (auto info = policyCache.begin(); info != policyCache.end(); ++info) {
        if (info->first == proxyId) {
            UpdateGlobalSleepTime(info->second);
        }
    }
    g_getSleepTimeResult = mode->GetSleepTime(proxyId);
}

namespace {
/**
 * @tc.name: PowerMgrSystemTest_001
 * @tc.desc: test SetDeviceMode in proxy
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrSystemTest, PowerMgrSystemTest_001, TestSize.Level2)
{
    PowerMode mode = PowerMode::POWER_SAVE_MODE;
    SetPolicyMode(static_cast<uint32_t>(mode));
    EXPECT_EQ(g_sleepTime, g_getSleepTimeResult) << "PowerMgrSystemTest_001 fail to SetDeviceMode";
}

/**
 * @tc.name: PowerMgrSystemTest_002
 * @tc.desc: test SetDeviceMode in proxy
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrSystemTest, PowerMgrSystemTest_002, TestSize.Level2)
{
    PowerMode mode = PowerMode::POWER_MODE_MIN;
    SetPolicyMode(static_cast<uint32_t>(mode));
    EXPECT_EQ(g_sleepTime, g_getSleepTimeResult) << "PowerMgrSystemTest_002 fail to SetDeviceMode";
}

/**
 * @tc.name: PowerMgrSystemTest_003
 * @tc.desc: test SetDeviceMode in proxy
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrSystemTest, PowerMgrSystemTest_003, TestSize.Level2)
{
    PowerMode mode = PowerMode::PERFORMANCE_MODE;
    SetPolicyMode(static_cast<uint32_t>(mode));
    EXPECT_EQ(g_sleepTime, g_getSleepTimeResult) << "PowerMgrSystemTest_003 fail to SetDeviceMode";
}

/**
 * @tc.name: PowerMgrSystemTest_004
 * @tc.desc: test SetDeviceMode in proxy
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrSystemTest, PowerMgrSystemTest_004, TestSize.Level2)
{
    PowerMode mode = PowerMode::EXTREME_POWER_SAVE_MODE;
    SetPolicyMode(static_cast<uint32_t>(mode));
    EXPECT_EQ(g_sleepTime, g_getSleepTimeResult) << "PowerMgrSystemTest_004 fail to SetDeviceMode";
}
} // namespace