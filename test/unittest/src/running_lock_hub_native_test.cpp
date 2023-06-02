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

#include "running_lock_hub_native_test.h"

#include <fcntl.h>
#include <ipc_skeleton.h>

#include <datetime_ex.h>
#include <input_manager.h>
#include <securec.h>

#include "actions/irunning_lock_action.h"
#include "running_lock_hub.h"
#include "suspend_controller.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

void RunningLockHubNativeTest::SetUpTestCase()
{
}

namespace {
void RunningSuspendCallback()
{
}

/**
 * @tc.name: RunningLockNative001
 * @tc.desc: test Acquire and Release inrunningLockMgr
 * @tc.type: FUNC
 */
HWTEST_F(RunningLockHubNativeTest, RunningLockNative001, TestSize.Level0)
{
    auto sc = std::make_shared<Suspend::SuspendController>();
    auto runningLockHub = std::make_shared<Suspend::RunningLockHub>(sc);
    runningLockHub->Acquire("777");
    EXPECT_FALSE(runningLockHub->runningLockMap_.empty());
    runningLockHub->Release("777");
    runningLockHub->lockFd_ = static_cast<UniqueFd>(UNFD);
    runningLockHub->Acquire("777");
    runningLockHub->Release("777");
    runningLockHub->Release("333");
    EXPECT_TRUE(runningLockHub->runningLockMap_.empty());
}

/**
 * @tc.name: RunningLockNative001
 * @tc.desc: test Suspend inrunningLockMgr
 * @tc.type: FUNC
 */
HWTEST_F(RunningLockHubNativeTest, RunningLockNative002, TestSize.Level0)
{
    auto SuspendController = std::make_shared<Suspend::SuspendController>();
    SuspendController->Wakeup();
    SuspendController->Suspend(RunningSuspendCallback, RunningSuspendCallback, false);
    SuspendController->Suspend(RunningSuspendCallback, RunningSuspendCallback, false);
    EXPECT_FALSE(SuspendController->suspend_->WriteWakeupCount("77"));
    SuspendController->suspend_->wakeupCountFd = static_cast<UniqueFd>(FD);
    EXPECT_TRUE(SuspendController->suspend_->WriteWakeupCount("77"));
    SuspendController->suspend_->wakeupCountFd =
        UniqueFd(TEMP_FAILURE_RETRY(open(Suspend::RunningLockHub::LOCK_PATH, O_RDWR | O_CLOEXEC)));
    EXPECT_TRUE(SuspendController->suspend_->WriteWakeupCount("77"));
    SuspendController->IncSuspendBlockCounter();
    SuspendController->IncSuspendBlockCounter();
    SuspendController->DecSuspendBlockCounter();
    SuspendController->DecSuspendBlockCounter();
}
} // namespace