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

#include "ffrt_utils.h"
#include "power_log.h"

namespace OHOS {
namespace PowerMgr {
void FFRTUtils::SubmitTask(const FFRTTask& task)
{
    ffrt::submit(task);
}

void FFRTUtils::SubmitTaskSync(const FFRTTask& task)
{
    ffrt::submit(task);
    ffrt::wait();
}

void FFRTUtils::SubmitQueueTasks(const std::vector<FFRTTask>& tasks, FFRTQueue& queue)
{
    if (tasks.empty()) {
        return;
    }
    for (auto task : tasks) {
        queue.submit(task);
    }
}

FFRTHandle FFRTUtils::SubmitDelayTask(FFRTTask& task, uint32_t delayMs, FFRTQueue& queue)
{
    using namespace std::chrono;
    milliseconds ms(delayMs);
    microseconds us = duration_cast<microseconds>(ms);
    return queue.submit_h(task, ffrt::task_attr().delay(us.count()));
}

FFRTHandle FFRTUtils::SubmitDelayTask(FFRTTask& task, uint32_t delayMs, std::shared_ptr<FFRTQueue> queue)
{
    using namespace std::chrono;
    milliseconds ms(delayMs);
    microseconds us = duration_cast<microseconds>(ms);
    return queue->submit_h(task, ffrt::task_attr().delay(us.count()));
}

bool FFRTUtils::SubmitTimeoutTask(const FFRTTask& task, uint32_t timeoutMs)
{
    ffrt::future<void> future = ffrt::async(task);
    auto status = future.wait_for(std::chrono::milliseconds(timeoutMs));
    return status == ffrt::future_status::ready;
}

int FFRTUtils::CancelTask(FFRTHandle& handle, FFRTQueue& queue)
{
    return queue.cancel(handle);
}

int FFRTUtils::CancelTask(FFRTHandle& handle, std::shared_ptr<FFRTQueue> queue)
{
    return queue->cancel(handle);
}

void FFRTMutexMap::Lock(uint32_t mutexId)
{
    mutexMap_[mutexId].lock();
}

void FFRTMutexMap::Unlock(uint32_t mutexId)
{
    mutexMap_[mutexId].unlock();
}

FFRTTimer::FFRTTimer(): queue_("ffrt_timer")
{
}

FFRTTimer::FFRTTimer(const char *timer_name): queue_(timer_name)
{
}

FFRTTimer::~FFRTTimer()
{
    Clear();
}

void FFRTTimer::Clear()
{
    mutex_.lock();
    POWER_HILOGD(FEATURE_UTIL, "FFRT Timer Clear");
    CancelAllTimerInner();
    handleMap_.clear();
    taskId_.clear();
    mutex_.unlock();
}

void FFRTTimer::CancelAllTimer()
{
    mutex_.lock();
    CancelAllTimerInner();
    mutex_.unlock();
}

void FFRTTimer::CancelTimer(uint32_t timerId)
{
    mutex_.lock();
    CancelTimerInner(timerId);
    mutex_.unlock();
}

void FFRTTimer::SetTimer(uint32_t timerId, FFRTTask& task)
{
    mutex_.lock();
    CancelTimerInner(timerId);
    ++taskId_[timerId];
    POWER_HILOGD(FEATURE_UTIL, "Timer[%{public}u] Add Task[%{public}u]", timerId, taskId_[timerId]);
    FFRTUtils::SubmitTask(task);
    mutex_.unlock();
}

void FFRTTimer::SetTimer(uint32_t timerId, FFRTTask& task, uint32_t delayMs)
{
    if (delayMs == 0) {
        return SetTimer(timerId, task);
    }

    mutex_.lock();
    CancelTimerInner(timerId);
    ++taskId_[timerId];
    POWER_HILOGD(FEATURE_UTIL, "Timer[%{public}u] Add Task[%{public}u] with delay = %{public}u",
        timerId, taskId_[timerId], delayMs);
    handleMap_[timerId] = FFRTUtils::SubmitDelayTask(task, delayMs, queue_);
    mutex_.unlock();
}

uint32_t FFRTTimer::GetTaskId(uint32_t timerId)
{
    mutex_.lock();
    uint32_t id = taskId_[timerId];
    mutex_.unlock();
    return id;
}

/* inner functions must be called when mutex_ is locked */
void FFRTTimer::CancelAllTimerInner()
{
    for (auto &p : handleMap_) {
        if (p.second != nullptr) {
            POWER_HILOGD(FEATURE_UTIL, "Timer[%{public}u] Cancel Task[%{public}u]", p.first, taskId_[p.first]);
            FFRTUtils::CancelTask(p.second, queue_);
            p.second = nullptr;
        }
    }
}

void FFRTTimer::CancelTimerInner(uint32_t timerId)
{
    if (handleMap_[timerId] != nullptr) {
        POWER_HILOGD(FEATURE_UTIL, "Timer[%{public}u] Cancel Task[%{public}u]", timerId, taskId_[timerId]);
        FFRTUtils::CancelTask(handleMap_[timerId], queue_);
        handleMap_[timerId] = nullptr;
    }
}

} // namespace PowerMgr
} // namespace OHOS