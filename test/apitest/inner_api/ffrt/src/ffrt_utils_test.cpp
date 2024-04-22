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
 * @tc.name: FFRTMutexTest001
 * @tc.desc: test submit tasks with mutex
 * @tc.type: FUNC
 */
HWTEST_F(FFRTUtilsTest, FFRTMutexTest001, TestSize.Level1)
{
    auto mutex = FFRTMutex();
    std::unique_lock lock(mutex);
    FFRTTask task1 = [&mutex]() {
        std::unique_lock lock(mutex, std::try_to_lock);
        EXPECT_FALSE(lock.owns_lock());
    };
    FFRTTask task2 = [&mutex]() {
        std::unique_lock lock(mutex, std::try_to_lock);
        EXPECT_TRUE(lock.owns_lock());
    };
    FFRTUtils::SubmitTaskSync(task1);
    lock.unlock();
    FFRTUtils::SubmitTaskSync(task2);
    EXPECT_TRUE(lock.try_lock());
}

/**
 * @tc.name: FFRTMutexTest002
 * @tc.desc: test mutex map
 * @tc.type: FUNC
 */
HWTEST_F(FFRTUtilsTest, FFRTMutexTest002, TestSize.Level1)
{
    constexpr uint32_t MUTEX_ID_A = 1;
    FFRTMutexMap mutexMap;
    int data = 0;
    FFRTTask taskA = [&mutexMap, &data]() {
        mutexMap.Lock(MUTEX_ID_A);
        data = 1;
        mutexMap.Unlock(MUTEX_ID_A);
    };

    mutexMap.Lock(MUTEX_ID_A);
    FFRTUtils::SubmitTask(taskA);
    ffrt::this_task::sleep_for(std::chrono::milliseconds(50));
    // taskA is waiting for lock, data is not changed
    EXPECT_EQ(data, 0);
    mutexMap.Unlock(MUTEX_ID_A);
    ffrt::this_task::sleep_for(std::chrono::milliseconds(50));
    // tanskA changed data to 1
    EXPECT_EQ(data, 1);
}

/**
 * @tc.name: FFRTMutexTest003
 * @tc.desc: test mutex map, different mutex are independent
 * @tc.type: FUNC
 */
HWTEST_F(FFRTUtilsTest, FFRTMutexTest003, TestSize.Level1)
{
    constexpr uint32_t MUTEX_ID_A = 1;
    constexpr uint32_t MUTEX_ID_B = 2;
    FFRTMutexMap mutexMap;
    int data = 0;

    FFRTTask taskA = [&mutexMap, &data]() {
        mutexMap.Lock(MUTEX_ID_A);
        data = 1;
        mutexMap.Unlock(MUTEX_ID_A);
    };

    mutexMap.Lock(MUTEX_ID_B);
    FFRTUtils::SubmitTask(taskA);
    ffrt::this_task::sleep_for(std::chrono::milliseconds(50));
    // tanskA changed data to 1
    EXPECT_EQ(data, 1);
    mutexMap.Unlock(MUTEX_ID_B);
}

/**
 * @tc.name: FFRTTimerTest001
 * @tc.desc: test FFRTTimer CancelTimer
 * @tc.type: FUNC
 */
HWTEST_F(FFRTUtilsTest, FFRTTimerTest001, TestSize.Level1)
{
    constexpr uint32_t TIMER_ID_A = 1;
    constexpr uint32_t TIMER_ID_B = 2;
    constexpr uint32_t TIMER_ID_C = 3;
    FFRTTimer timer;
    int data = 0;

    FFRTTask taskA = [&data]() {
        data = 1;
    };

    FFRTTask taskB = [&data]() {
        data = 2;
    };

    FFRTTask taskC = [&data]() {
        data = 3;
    };

    timer.SetTimer(TIMER_ID_A, taskA, 50);
    timer.SetTimer(TIMER_ID_B, taskB, 60);
    timer.SetTimer(TIMER_ID_C, taskC, 70);
    timer.CancelAllTimer();
    ffrt::this_task::sleep_for(std::chrono::milliseconds(100));
    // data is not changed
    EXPECT_EQ(data, 0);

    timer.SetTimer(TIMER_ID_A, taskA, 50);
    timer.SetTimer(TIMER_ID_B, taskB, 60);
    timer.SetTimer(TIMER_ID_C, taskC, 70);
    timer.Clear();
    ffrt::this_task::sleep_for(std::chrono::milliseconds(100));
    // data is not changed
    EXPECT_EQ(data, 0);

    timer.SetTimer(TIMER_ID_A, taskA, 50);
    timer.SetTimer(TIMER_ID_B, taskB, 60);
    timer.SetTimer(TIMER_ID_C, taskC, 70);
    timer.CancelTimer(TIMER_ID_B);
    timer.CancelTimer(TIMER_ID_C);
    ffrt::this_task::sleep_for(std::chrono::milliseconds(100));
    // taskA changed data to 1, taskB and taskC are canceled
    EXPECT_EQ(data, 1);
}

/**
 * @tc.name: FFRTTimerTest002
 * @tc.desc: test FFRTTimer GetTaskId
 * @tc.type: FUNC
 */
HWTEST_F(FFRTUtilsTest, FFRTTimerTest002, TestSize.Level1)
{
    FFRTMutexMap mutexMap;
    FFRTTimer timer;

    constexpr uint32_t TIMER_ID_A = 1;
    constexpr uint32_t TIMER_COUNT = 10;
    int count = 0;

    FFRTTask taskA = [&count, &mutexMap]() {
        mutexMap.Lock(TIMER_ID_A);
        count++;
        mutexMap.Unlock(TIMER_ID_A);
    };

    for (int i = 0; i < TIMER_COUNT; i++) {
        timer.SetTimer(TIMER_ID_A, taskA, 50);
    }
    ffrt::this_task::sleep_for(std::chrono::milliseconds(100));
    // only the last timer is run, count should be 1
    EXPECT_EQ(count, 1);
    EXPECT_EQ(timer.GetTaskId(TIMER_ID_A), TIMER_COUNT);

    timer.Clear();
    // task id is set to 0 in Clear()
    EXPECT_EQ(timer.GetTaskId(TIMER_ID_A), 0);
}
} // namespace Test
} // namespace PowerMgr
} // namespace OHOS
