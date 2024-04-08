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

void FFRTUtils::CancelTask(FFRTHandle& handle, FFRTQueue& queue)
{
    queue.cancel(handle);
}

void FFRTUtils::CancelTask(FFRTHandle& handle, std::shared_ptr<FFRTQueue> queue)
{
    queue->cancel(handle);
}
} // namespace PowerMgr
} // namespace OHOS