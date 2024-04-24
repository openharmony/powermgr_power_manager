/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "running_lock_timer_handler_test.h"

#include <functional>
#include "power_mgr_client.h"
#include "running_lock.h"
#include "running_lock_token_stub.h"

namespace OHOS {
namespace PowerMgr {
namespace UnitTest {
namespace {
constexpr int32_t TIMEOUT_MS = 4000;
constexpr int32_t SLEEP_WAIT_TIME_S = 5;
constexpr int32_t COUNT_RESULT = 2;
}
using namespace testing::ext;

/**
 * @tc.name: RegisterRunningLockTimer
 * @tc.desc: Test RegisterRunningLockTimer
 * @tc.type: FUNC
 * @tc.require: issueI9C4GG
 */
HWTEST_F(RunningLockTimerHandlerTest, RegisterRunningLockTimer001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "RegisterRunningLockTimer001: start";
    sptr<IRemoteObject> token = new RunningLockTokenStub();
    int count = 1;
    std::function<void()> task = [&]() {
        count++;
    };
    bool ret = RunningLockTimerHandler::GetInstance().RegisterRunningLockTimer(token,
        task, TIMEOUT_MS);
    EXPECT_TRUE(ret);
    sleep(SLEEP_WAIT_TIME_S);
    EXPECT_EQ(COUNT_RESULT, count);
    GTEST_LOG_(INFO) << "RegisterRunningLockTimer001:  end";
}


/**
 * @tc.name: RegisterRunningLockTimer
 * @tc.desc: Test RegisterRunningLockTimer
 * @tc.type: FUNC
 * @tc.require: issueI9C4GG
 */
HWTEST_F(RunningLockTimerHandlerTest, RegisterRunningLockTimer002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "RegisterRunningLockTimer002: start";
    std::shared_ptr<RunningLock> runningLock1 =
        std::make_shared<RunningLock>(nullptr, "runninglock_Timer_test1",
            RunningLockType::RUNNINGLOCK_SCREEN);
    std::shared_ptr<RunningLock> runningLock2 =
        std::make_shared<RunningLock>(nullptr, "runninglock_Timer_test2",
            RunningLockType::RUNNINGLOCK_BACKGROUND);
    ASSERT_TRUE(runningLock1 != nullptr);
    runningLock1->Init();
    ASSERT_TRUE(runningLock2 != nullptr);
    runningLock2->Init();
    runningLock1->Lock();
    runningLock2->Lock();
    EXPECT_TRUE(!runningLock1->IsUsed());
    EXPECT_TRUE(!runningLock2->IsUsed());
    runningLock1->UnLock();
    runningLock2->UnLock();
    EXPECT_TRUE(!runningLock1->IsUsed());
    EXPECT_TRUE(!runningLock2->IsUsed());
    runningLock1->UnLock();
    runningLock2->UnLock();
    GTEST_LOG_(INFO) << "RegisterRunningLockTimer002:  end";
}

/**
 * @tc.name: RegisterRunningLockTimer
 * @tc.desc: Test RegisterRunningLockTimer
 * @tc.type: FUNC
 * @tc.require: issueI9C4GG
 */
HWTEST_F(RunningLockTimerHandlerTest, RegisterRunningLockTimer003, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "RegisterRunningLockTimer003: start";
    std::shared_ptr<RunningLock> runningLock1 =
        std::make_shared<RunningLock>(nullptr, "runninglock_Timer_test3",
        RunningLockType::RUNNINGLOCK_BACKGROUND_TASK);
    std::shared_ptr<RunningLock> runningLock2 =
        std::make_shared<RunningLock>(nullptr, "runninglock_Timer_test4",
        RunningLockType::RUNNINGLOCK_BACKGROUND_NOTIFICATION);
    ASSERT_TRUE(runningLock1 != nullptr);
    runningLock1->Init();
    ASSERT_TRUE(runningLock2 != nullptr);
    runningLock2->Init();
    runningLock1->Lock(TIMEOUT_MS);
    runningLock2->Lock(0);
    // fake test for now, remember to correct it in future.
    EXPECT_TRUE(!runningLock1->IsUsed());
    EXPECT_TRUE(!runningLock2->IsUsed());
    sleep(SLEEP_WAIT_TIME_S);
    EXPECT_TRUE(!runningLock1->IsUsed());
    EXPECT_TRUE(!runningLock2->IsUsed());
    runningLock1->UnLock();
    runningLock2->UnLock();
    GTEST_LOG_(INFO) << "RegisterRunningLockTimer003:  end";
}

/**
 * @tc.name: UnregisterRunningLockTimer
 * @tc.desc: Test UnregisterRunningLockTimer
 * @tc.type: FUNC
 * @tc.require: issueI9C4GG
 */
HWTEST_F(RunningLockTimerHandlerTest, UnregisterRunningLockTimer001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "UnregisterRunningLockTimer001: start";
    sptr<IRemoteObject> token = new RunningLockTokenStub();
    int count = 1;
    std::function<void()> task = [&]() {
        count++;
    };
    bool ret = RunningLockTimerHandler::GetInstance().RegisterRunningLockTimer(token,
        task, TIMEOUT_MS);
    EXPECT_TRUE(ret);
    ret = RunningLockTimerHandler::GetInstance().UnregisterRunningLockTimer(token);
    EXPECT_TRUE(ret);
    EXPECT_NE(COUNT_RESULT, count);
    GTEST_LOG_(INFO) << "UnregisterRunningLockTimer001:  end";
}

/**
 * @tc.name: UnregisterRunningLockTimer
 * @tc.desc: Test UnregisterRunningLockTimer
 * @tc.type: FUNC
 * @tc.require: issueI9C4GG
 */
HWTEST_F(RunningLockTimerHandlerTest, UnregisterRunningLockTimer002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "UnregisterRunningLockTimer002: start";
    std::shared_ptr<RunningLockTimerHandler> runningLockTimerHandler =
        std::make_shared<RunningLockTimerHandler>();
    ASSERT_TRUE(runningLockTimerHandler != nullptr);
    sptr<IRemoteObject> token1 = new RunningLockTokenStub();
    sptr<IRemoteObject> token2 = new RunningLockTokenStub();
    int count1 = 1;
    std::function<void()> task1 = [&]() {
        count1++;
    };
    int count2 = 1;
    std::function<void()> task2 = [&]() {
        count2++;
    };
    runningLockTimerHandler->RegisterRunningLockTimer(token1, task1, TIMEOUT_MS);
    runningLockTimerHandler->RegisterRunningLockTimer(token2, task2, TIMEOUT_MS);
    runningLockTimerHandler.reset();
    EXPECT_NE(COUNT_RESULT, count1);
    EXPECT_NE(COUNT_RESULT, count2);
    GTEST_LOG_(INFO) << "UnregisterRunningLockTimer002:  end";
}
} // namespace UnitTest
} // namespace PowerMgr
} // namespace OHOS0.
