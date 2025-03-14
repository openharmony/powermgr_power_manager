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

#include "running_lock_scenario_test.h"

#include <ipc_skeleton.h>

#include "actions/irunning_lock_action.h"
#include "power_log.h"
#include "power_mgr_service.h"
#include "running_lock_mgr.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

namespace {
constexpr int32_t US_PER_MS = 1000;
constexpr int32_t app0Uid = 8;
constexpr int32_t app1Uid = 9;
}

namespace {
/**
 * @tc.name: RunningLockScenarioTest001
 * @tc.desc: Test runninglock single app scenario
 * @tc.type: FUNC
 * @tc.require
 */
HWTEST_F (RunningLockScenarioTest, RunningLockScenarioTest001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockScenarioTest001 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    pid_t curUid = getuid();
    pid_t curPid = getpid();

    std::shared_ptr<RunningLock> runningLock = powerMgrClient.CreateRunningLock(
        "background.test019", RunningLockType::RUNNINGLOCK_BACKGROUND);
    ASSERT_NE(runningLock, nullptr);
    runningLock->Lock();

    std::vector<int32_t> workSource { app0Uid };
    EXPECT_TRUE(runningLock->UpdateWorkSource(workSource) == 0);
    EXPECT_TRUE(runningLock->IsUsed());

    runningLock->UnLock();
    EXPECT_FALSE(runningLock->IsUsed());
    POWER_HILOGI(LABEL_TEST, "RunningLockScenarioTest001 function end!");
}

/**
 * @tc.name: RunningLockScenarioTest002
 * @tc.desc: Test runninglock single app scenario with proxy
 * @tc.type: FUNC
 * @tc.require
 */
HWTEST_F (RunningLockScenarioTest, RunningLockScenarioTest002, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockScenarioTest002 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    pid_t curUid = getuid();
    pid_t curPid = getpid();

    std::shared_ptr<RunningLock> runningLock = powerMgrClient.CreateRunningLock(
        "background.test020", RunningLockType::RUNNINGLOCK_BACKGROUND);
    ASSERT_NE(runningLock, nullptr);
    runningLock->Lock();
    // open app
    std::vector<int32_t> workSource1 { app0Uid };
    EXPECT_TRUE(runningLock->UpdateWorkSource(workSource1) == 0);
    usleep(100*1000);
    EXPECT_TRUE(runningLock->IsUsed());
    // freeze app
    EXPECT_TRUE(powerMgrClient.ProxyRunningLock(true, curPid, app0Uid));
    EXPECT_FALSE(runningLock->IsUsed());
    // thaw app
    EXPECT_TRUE(powerMgrClient.ProxyRunningLock(false, curPid, app0Uid));
    EXPECT_TRUE(runningLock->IsUsed());
    // close app
    std::vector<int32_t> workSource0 {};
    EXPECT_TRUE(runningLock->UpdateWorkSource(workSource0) == 0);
    usleep(100*1000);
    EXPECT_TRUE(runningLock->IsUsed());

    runningLock->UnLock();
    EXPECT_FALSE(runningLock->IsUsed());
    POWER_HILOGI(LABEL_TEST, "RunningLockScenarioTest002 function end!");
}

/**
 * @tc.name: RunningLockScenarioTest003
 * @tc.desc: Test runninglock multi apps scenario with proxy
 * @tc.type: FUNC
 * @tc.require
 */
HWTEST_F (RunningLockScenarioTest, RunningLockScenarioTest003, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockScenarioTest003 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    pid_t curUid = getuid();
    pid_t curPid = getpid();

    std::vector<int32_t> workSource00 {};
    std::vector<int32_t> workSource10 { app0Uid };
    std::vector<int32_t> workSource01 { app1Uid };
    std::vector<int32_t> workSource11 { app0Uid, app1Uid };

    std::shared_ptr<RunningLock> runningLock = powerMgrClient.CreateRunningLock(
        "background.test021", RunningLockType::RUNNINGLOCK_BACKGROUND);
    ASSERT_NE(runningLock, nullptr);
    runningLock->Lock();
    // open app0
    EXPECT_TRUE(runningLock->UpdateWorkSource(workSource10) == 0);
    EXPECT_TRUE(runningLock->IsUsed());
    // open app1
    EXPECT_TRUE(runningLock->UpdateWorkSource(workSource11) == 0);
    EXPECT_TRUE(runningLock->IsUsed());
    // freeze app0
    EXPECT_TRUE(powerMgrClient.ProxyRunningLock(true, curPid, app0Uid));
    EXPECT_TRUE(runningLock->IsUsed());
    // freeze app1
    EXPECT_TRUE(powerMgrClient.ProxyRunningLock(true, curPid, app1Uid));
    EXPECT_FALSE(runningLock->IsUsed());
    // thaw app0
    EXPECT_TRUE(powerMgrClient.ProxyRunningLock(false, curPid, app0Uid));
    EXPECT_TRUE(runningLock->IsUsed());
    // thaw app1
    EXPECT_TRUE(powerMgrClient.ProxyRunningLock(false, curPid, app1Uid));
    EXPECT_TRUE(runningLock->IsUsed());
    // close app0
    EXPECT_TRUE(runningLock->UpdateWorkSource(workSource01) == 0);
    EXPECT_TRUE(runningLock->IsUsed());
    // close app1
    EXPECT_TRUE(runningLock->UpdateWorkSource(workSource00) == 0);
    EXPECT_TRUE(runningLock->IsUsed());

    runningLock->UnLock();
    EXPECT_FALSE(runningLock->IsUsed());
    POWER_HILOGI(LABEL_TEST, "RunningLockScenarioTest003 function end!");
}

/**
 * @tc.name: RunningLockScenarioTest004
 * @tc.desc: Test runninglock multi apps scenario with proxy
 * @tc.type: FUNC
 * @tc.require
 */
HWTEST_F (RunningLockScenarioTest, RunningLockScenarioTest004, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockScenarioTest004 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    pid_t curUid = getuid();
    pid_t curPid = getpid();

    std::vector<int32_t> workSource00 {};
    std::vector<int32_t> workSource10 { app0Uid };
    std::vector<int32_t> workSource11 { app0Uid, app1Uid };

    std::shared_ptr<RunningLock> runningLock = powerMgrClient.CreateRunningLock(
        "background.test004", RunningLockType::RUNNINGLOCK_BACKGROUND);
    ASSERT_NE(runningLock, nullptr);
    runningLock->Lock();
    // open app0
    EXPECT_TRUE(runningLock->UpdateWorkSource(workSource10) == 0);
    usleep(100*1000);
    EXPECT_TRUE(runningLock->IsUsed());
    // freeze app0
    EXPECT_TRUE(powerMgrClient.ProxyRunningLock(true, curPid, app0Uid));
    EXPECT_FALSE(runningLock->IsUsed());
    // open app1
    EXPECT_TRUE(runningLock->UpdateWorkSource(workSource11) == 0);
    usleep(100*1000);
    EXPECT_TRUE(runningLock->IsUsed());
    // close app1
    EXPECT_TRUE(runningLock->UpdateWorkSource(workSource10) == 0);
    usleep(100*1000);
    EXPECT_FALSE(runningLock->IsUsed());
    // thaw app0
    EXPECT_TRUE(powerMgrClient.ProxyRunningLock(false, curPid, app0Uid));
    EXPECT_TRUE(runningLock->IsUsed());
    // close app0
    EXPECT_TRUE(runningLock->UpdateWorkSource(workSource00) == 0);
    usleep(100*1000);
    EXPECT_TRUE(runningLock->IsUsed());

    runningLock->UnLock();
    EXPECT_FALSE(runningLock->IsUsed());
    POWER_HILOGI(LABEL_TEST, "RunningLockScenarioTest004 function end!");
}
} // namespace