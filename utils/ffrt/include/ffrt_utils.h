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
#ifndef POWERMGR_FFRT_UTILS_H
#define POWERMGR_FFRT_UTILS_H

#include <functional>
#include <vector>

#include "ffrt_inner.h"

namespace OHOS {
namespace PowerMgr {
/**
 * Defines the task of the FFRT.
 */
using FFRTTask = std::function<void()>;

/**
 * Defines the task handle of the FFRT.
 */
using FFRTHandle = ffrt::task_handle;

/**
 * Defines the task queue of the FFRT.。
 */
using FFRTQueue = ffrt::queue;
class FFRTUtils final {
public:
    /**
     * Submit an FFRT atomization task without blocking the current thread.
     *
     * @param task FFRT task.
     */
    static void SubmitTask(const FFRTTask& task);

    /**
     * Submit an FFRT task blocks the current thread and waits for the task to complete.
     *
     * @param task FFRT task.
     */
    static void SubmitTaskSync(const FFRTTask& task);

    /**
     * Submit an FFRT serial task without blocking the current thread.
     *
     * @param task FFRT task.
     */
    static void SubmitQueueTasks(const std::vector<FFRTTask>& tasks, FFRTQueue& queue);

    /**
     * Submit the FFRT delayed task without blocking the current thread.
     * <p>
     * When the delay time is reached, the task starts to be executed.
     *
     * @param task FFRT task.
     * @param delayMs Delay time, in milliseconds.
     * @param queue FFRT task execution queue.
     *
     * @return FFRT task handle.
     */
    static FFRTHandle SubmitDelayTask(FFRTTask& task, uint32_t delayMs, FFRTQueue& queue);

    /**
     * Submit the FFRT delayed task without blocking the current thread.
     * <p>
     * When the delay time is reached, the task starts to be executed.
     *
     * @param task FFRT task.
     * @param delayMs Delay time, in milliseconds.
     * @param queue Shared_ptr of FFRT task execution queue.
     *
     * @return FFRT task handle.
     */
    static FFRTHandle SubmitDelayTask(FFRTTask& task, uint32_t delayMs, std::shared_ptr<FFRTQueue> queue);

    /**
     * Submit an FFRT timeout task without blocking the current thread.
     * <p>
     * When the timeout period is reached, the task will be canceled.
     *
     * @param task FFRT task.
     * @param timeoutMs Timeout interval, in milliseconds.
     *
     * @return true: The task is executed successfully. false: The task execution times out.
     */
    static bool SubmitTimeoutTask(const FFRTTask& task, uint32_t timeoutMs);

    /**
     * Cancel the FFRT task.
     * <p>
     * You cannot cancel a completed task.
     *
     * @param handle FFRT task.
     */
    static void CancelTask(FFRTHandle& handle, FFRTQueue& queue);

    /**
     * Cancel the FFRT task.
     * <p>
     * You cannot cancel a completed task.
     *
     * @param handle FFRT task.
     * @param queue Shared_ptr of FFRT task cancel queue.
     */
    static void CancelTask(FFRTHandle& handle, std::shared_ptr<FFRTQueue> queue);

    /**
     * The mutex for FFRT tasks.
     */
    class Mutex {
    public:
        Mutex();
        ~Mutex();
        void Lock();
        bool TryLock();
        void Unlock();

    private:
        ffrt::mutex* mutex_;
    };
};
} // namespace PowerMgr
} // namespace OHOS

#endif // POWERMGR_FFRT_UTILS_H
