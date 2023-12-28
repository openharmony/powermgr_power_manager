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
#include "ffrt_utils_test.h"

#include "ffrt_utils.h"

namespace OHOS {
namespace PowerMgr {
namespace Test {
using namespace testing::ext;
/**
 * @tc.name: FFRTUtilsTest001
 * @tc.desc: test submit task
 * @tc.type: FUNC
 */
HWTEST_F(FFRTUtilsTest, FFRTUtilsTest001, TestSize.Level1)
{
    int32_t x = 0;
    FFRTTask task = [&]() {
        x = 2;
    };
    FFRTUtils::SubmitTask(task); // submit an async task
    ffrt::wait(); // wait async task finish
    EXPECT_EQ(x, 2);
}

/**
 * @tc.name: FFRTUtilsTest002
 * @tc.desc: test submit task sync
 * @tc.type: FUNC
 */
HWTEST_F(FFRTUtilsTest, FFRTUtilsTest002, TestSize.Level1)
{
    int32_t x = 0;
    FFRTTask task = [&]() {
        x = 2;
    };
    FFRTUtils::SubmitTaskSync(task); // submit a sync task
    EXPECT_EQ(x, 2);
}

/**
 * @tc.name: FFRTUtilsTest003
 * @tc.desc: test submit queue tasks
 * @tc.type: FUNC
 */
HWTEST_F(FFRTUtilsTest, FFRTUtilsTest003, TestSize.Level1)
{
    int x = 0;
    FFRTTask task1 = [&]() {
        ffrt::this_task::sleep_for(std::chrono::milliseconds(1));
        x = 2;
    };
    FFRTTask task2 = [&]() {
        ffrt::this_task::sleep_for(std::chrono::milliseconds(30));
        x += 2;
    };
    FFRTTask task3 = [&]() {
        ffrt::this_task::sleep_for(std::chrono::milliseconds(50));
        x += 2;
    };

    FFRTQueue queue("test_power_ffrt_queue");
    FFRTUtils::SubmitQueueTasks({task1, task2, task3}, queue); // submit batch tasks to a queue

    ffrt::this_task::sleep_for(std::chrono::milliseconds(10));
    EXPECT_EQ(x, 2); // task1 finished

    ffrt::this_task::sleep_for(std::chrono::milliseconds(50));
    EXPECT_EQ(x, 4); // task2 finished

    ffrt::this_task::sleep_for(std::chrono::milliseconds(80));
    EXPECT_EQ(x, 6); // task3 finished
}

/**
 * @tc.name: FFRTUtilsTest004
 * @tc.desc: test submit delay task
 * @tc.type: FUNC
 */
HWTEST_F(FFRTUtilsTest, FFRTUtilsTest004, TestSize.Level1)
{
    int x = 0;
    FFRTTask task = [&]() {
        x = 2;
    };

    FFRTQueue queue("test_power_ffrt_queue");
    FFRTUtils::SubmitDelayTask(task, 10, queue); // submit delay task to a queue

    ffrt::this_task::sleep_for(std::chrono::milliseconds(5));
    EXPECT_EQ(x, 0); // task not executed

    ffrt::this_task::sleep_for(std::chrono::milliseconds(7));
    EXPECT_EQ(x, 2); // task finished
}

/**
 * @tc.name: FFRTUtilsTest005
 * @tc.desc: test cancel delay task
 * @tc.type: FUNC
 */
HWTEST_F(FFRTUtilsTest, FFRTUtilsTest005, TestSize.Level1)
{
    int x = 0;
    FFRTTask task = [&]() {
        x = 2;
    };

    FFRTQueue queue("test_power_ffrt_queue");
    auto handle = FFRTUtils::SubmitDelayTask(task, 10, queue); // submit delay task to a queue

    ffrt::this_task::sleep_for(std::chrono::milliseconds(5));
    EXPECT_EQ(x, 0); // task not executed

    FFRTUtils::CancelTask(handle, queue); // cancel the delay task from the queue
    EXPECT_EQ(x, 0); // task not executed

    ffrt::this_task::sleep_for(std::chrono::milliseconds(10));
    EXPECT_EQ(x, 0); // task not executed, because it is already canceled
}

/**
 * @tc.name: FFRTUtilsTest006
 * @tc.desc: test submit timeout task and the task is executed successfully
 * @tc.type: FUNC
 */
HWTEST_F(FFRTUtilsTest, FFRTUtilsTest006, TestSize.Level1)
{
    int x = 0;
    FFRTTask task = [&]() {
        ffrt::this_task::sleep_for(std::chrono::milliseconds(5)); // task sleep 5ms
        x = 2;
    };
    bool ret = FFRTUtils::SubmitTimeoutTask(task, 10); // task time out is 10ms
    EXPECT_TRUE(ret); // task will not timeout
    EXPECT_EQ(x, 2); // task finished
}

/**
 * @tc.name: FFRTUtilsTest007
 * @tc.desc: test submit timeout task and the task execution times out
 * @tc.type: FUNC
 */
HWTEST_F(FFRTUtilsTest, FFRTUtilsTest007, TestSize.Level1)
{
    int x = 0;
    FFRTTask task = [&]() {
        ffrt::this_task::sleep_for(std::chrono::milliseconds(10)); // task sleep 10ms
        x = 2;
    };
    bool ret = FFRTUtils::SubmitTimeoutTask(task, 5); // task time out is 5ms
    EXPECT_FALSE(ret); // task will timeout
    EXPECT_EQ(x, 0); // task not finished
}

/**
 * @tc.name: FFRTUtilsMutexTest001
 * @tc.desc: test submit tasks with mutex
 * @tc.type: FUNC
 */
HWTEST_F(FFRTUtilsTest, FFRTUtilsMutexTest001, TestSize.Level1)
{
    int x = 0;
    auto mutex = FFRTUtils::Mutex();
    FFRTTask task1 = [&]() {
        mutex.Lock(); // mutex lock
        ffrt::this_task::sleep_for(std::chrono::milliseconds(10)); // task sleep 10ms
        x += 2;
        mutex.Unlock(); // mutex unlock
    };
    FFRTTask task2 = [&]() {
        mutex.Lock();
        ffrt::this_task::sleep_for(std::chrono::milliseconds(5)); // task sleep 5ms
        x += 3;
        mutex.Unlock();
    };
    FFRTUtils::SubmitTask(task1); // submit task async
    ffrt::this_task::sleep_for(std::chrono::milliseconds(2)); // task sleep 2ms
    FFRTUtils::SubmitTask(task2); // submit task async

    ffrt::this_task::sleep_for(std::chrono::milliseconds(5));
    EXPECT_EQ(x, 0); // task1 not finished, and task2 cannot access mutex block
    EXPECT_FALSE(mutex.TryLock()); // mutex is locked by task1, try lock returns false

    ffrt::this_task::sleep_for(std::chrono::milliseconds(7));
    EXPECT_EQ(x, 2); // task1 finished

    ffrt::this_task::sleep_for(std::chrono::milliseconds(5));
    EXPECT_EQ(x, 5); // task2 finished
    EXPECT_TRUE(mutex.TryLock()); // mutex is unlocked, try lock returns true
}
} // namespace Test
} // namespace PowerMgr
} // namespace OHOS
