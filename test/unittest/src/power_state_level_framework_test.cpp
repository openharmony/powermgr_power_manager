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

#include "power_state_level_framework_test.h"

#include <iostream>

#include <gtest/gtest.h>
#include <string_ex.h>

#include "power_mgr_service.h"
#include "power_state_machine.h"
#include "power_state_machine_info.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

namespace {
sptr<PowerMgrService> g_pmsTest;
std::shared_ptr<PowerStateMachine> g_stateMachineTest;
}

void PowerStateLevelFrameworkTest::SetUpTestCase()
{
    g_pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    EXPECT_TRUE(g_pmsTest != nullptr) << "PowerMgrService02 fail to get PowerMgrService";
    g_pmsTest->OnStart();
    g_stateMachineTest = g_pmsTest->GetPowerStateMachine();
}

namespace {
/**
 * @tc.name: PowerStateLevelFramework001
 * @tc.desc: test system-level sleep level enumeration
 * @tc.type: FUNC
 * @tc.require: issueI6JSKU
 */
HWTEST_F (PowerStateLevelFrameworkTest, PowerStateLevelFramework001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerStateLevelFramework001: test system-level sleep level enumeration is start";
    EXPECT_TRUE(static_cast<uint32_t>(PowerState::AWAKE) == AWAKE) << "PowerState AWAKE correct";
    EXPECT_TRUE(static_cast<uint32_t>(PowerState::FREEZE) == FREEZE) << "PowerState FREEZE correct";
    EXPECT_TRUE(static_cast<uint32_t>(PowerState::INACTIVE) == INACTIVE) << "PowerState INACTIVE correct";
    EXPECT_TRUE(static_cast<uint32_t>(PowerState::STAND_BY) == STAND_BY) << "PowerState STAND_BY correct";
    EXPECT_TRUE(static_cast<uint32_t>(PowerState::DOZE) == DOZE) << "PowerState DOZE correct";
    EXPECT_TRUE(static_cast<uint32_t>(PowerState::SLEEP) == SLEEP) << "PowerState SLEEP correct";
    EXPECT_TRUE(static_cast<uint32_t>(PowerState::HIBERNATE) == HIBERNATE) << "PowerState HIBERNATE correct";
    EXPECT_TRUE(static_cast<uint32_t>(PowerState::SHUTDOWN) == SHUTDOWN) << "PowerState HIBERNATE correct";
    GTEST_LOG_(INFO) << "PowerStateLevelFramework001: test system-level sleep level enumeration is end";
}

/**
 * @tc.name: PowerStateLevelFramework002
 * @tc.desc: test the conversion between power states
 * @tc.type: FUNC
 * @tc.require: issueI6OM1F
 */
HWTEST_F (PowerStateLevelFrameworkTest, PowerStateLevelFramework002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerStateLevelFramework002: power state replacement is start";
    auto ret = g_stateMachineTest->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_INIT);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(g_stateMachineTest->GetState() == PowerState::AWAKE);
    GTEST_LOG_(INFO) << "PowerStateLevelFramework002: power state replacement is end";
}

/**
 * @tc.name: PowerStateLevelFramework003
 * @tc.desc: test the conversion between power states
 * @tc.type: FUNC
 * @tc.require: issueI6OM1F
 */
HWTEST_F (PowerStateLevelFrameworkTest, PowerStateLevelFramework003, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerStateLevelFramework003: power state replacement is start";
    auto ret = g_stateMachineTest->SetState(PowerState::FREEZE, StateChangeReason::STATE_CHANGE_REASON_INIT);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(g_stateMachineTest->GetState() == PowerState::FREEZE);
    GTEST_LOG_(INFO) << "PowerStateLevelFramework003: power state replacement is end";
}

/**
 * @tc.name: PowerStateLevelFramework004
 * @tc.desc: test the conversion between power states
 * @tc.type: FUNC
 * @tc.require: issueI6OM1F
 */
HWTEST_F (PowerStateLevelFrameworkTest, PowerStateLevelFramework004, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerStateLevelFramework004: power state replacement is start";
    auto ret = g_stateMachineTest->SetState(PowerState::INACTIVE, StateChangeReason::STATE_CHANGE_REASON_INIT);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(g_stateMachineTest->GetState() == PowerState::INACTIVE);
    GTEST_LOG_(INFO) << "PowerStateLevelFramework004: power state replacement is end";
}

/**
 * @tc.name: PowerStateLevelFramework005
 * @tc.desc: test the conversion between power states
 * @tc.type: FUNC
 * @tc.require: issueI6OM1F
 */
HWTEST_F (PowerStateLevelFrameworkTest, PowerStateLevelFramework005, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerStateLevelFramework005: power state replacement is start";
    auto ret = g_stateMachineTest->SetState(PowerState::STAND_BY, StateChangeReason::STATE_CHANGE_REASON_INIT);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(g_stateMachineTest->GetState() == PowerState::STAND_BY);
    GTEST_LOG_(INFO) << "PowerStateLevelFramework005: power state replacement is end";
}

/**
 * @tc.name: PowerStateLevelFramework006
 * @tc.desc: test the conversion between power states
 * @tc.type: FUNC
 * @tc.require: issueI6OM1F
 */
HWTEST_F (PowerStateLevelFrameworkTest, PowerStateLevelFramework006, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerStateLevelFramework006: power state replacement is start";
    auto ret = g_stateMachineTest->SetState(PowerState::DOZE, StateChangeReason::STATE_CHANGE_REASON_INIT);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(g_stateMachineTest->GetState() == PowerState::DOZE);
    GTEST_LOG_(INFO) << "PowerStateLevelFramework006: power state replacement is end";
}

/**
 * @tc.name: PowerStateLevelFramework007
 * @tc.desc: test the conversion between power states
 * @tc.type: FUNC
 * @tc.require: issueI6OM1F
 */
HWTEST_F (PowerStateLevelFrameworkTest, PowerStateLevelFramework007, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerStateLevelFramework007: power state replacement is start";
    auto ret = g_stateMachineTest->SetState(PowerState::SLEEP, StateChangeReason::STATE_CHANGE_REASON_INIT);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(g_stateMachineTest->GetState() == PowerState::SLEEP);
    GTEST_LOG_(INFO) << "PowerStateLevelFramework007: power state replacement is end";
}

/**
 * @tc.name: PowerStateLevelFramework008
 * @tc.desc: test the conversion between power states
 * @tc.type: FUNC
 * @tc.require: issueI6OM1F
 */
HWTEST_F (PowerStateLevelFrameworkTest, PowerStateLevelFramework008, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerStateLevelFramework008: power state replacement is start";
    auto ret = g_stateMachineTest->SetState(PowerState::HIBERNATE, StateChangeReason::STATE_CHANGE_REASON_INIT);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(g_stateMachineTest->GetState() == PowerState::HIBERNATE);
    GTEST_LOG_(INFO) << "PowerStateLevelFramework008: power state replacement is end";
}

/**
 * @tc.name: PowerStateLevelFramework009
 * @tc.desc: test the conversion between power states
 * @tc.type: FUNC
 * @tc.require: issueI6OM1F
 */
HWTEST_F (PowerStateLevelFrameworkTest, PowerStateLevelFramework009, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerStateLevelFramework009: power state replacement is start";
    auto ret = g_stateMachineTest->SetState(PowerState::SHUTDOWN, StateChangeReason::STATE_CHANGE_REASON_INIT);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(g_stateMachineTest->GetState() == PowerState::SHUTDOWN);
    GTEST_LOG_(INFO) << "PowerStateLevelFramework009: power state replacement is end";
}

/**
 * @tc.name: PowerStateLevelFramework010
 * @tc.desc: test the conversion between power states
 * @tc.type: FUNC
 * @tc.require: issueI6OM1F
 */
HWTEST_F (PowerStateLevelFrameworkTest, PowerStateLevelFramework010, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerStateLevelFramework010: power state replacement is start";
    std::string result;
    g_stateMachineTest->DumpInfo(result);
    EXPECT_TRUE(!result.empty());
    EXPECT_TRUE(result.find("AWAKE") != 0);
    EXPECT_TRUE(result.find("FREEZE") != 0);
    EXPECT_TRUE(result.find("INACTIVE") != 0);
    EXPECT_TRUE(result.find("STAND_BY") != 0);
    EXPECT_TRUE(result.find("DOZE") != 0);
    EXPECT_TRUE(result.find("SLEEP") != 0);
    EXPECT_TRUE(result.find("HIBERNATE") != 0);
    EXPECT_TRUE(result.find("SHUTDOWN") != 0);
    GTEST_LOG_(INFO) << "dumpinfo:" << result;
    GTEST_LOG_(INFO) << "PowerStateLevelFramework010: power state replacement is end";
}
}