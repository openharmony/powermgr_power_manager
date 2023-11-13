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

#include "sleep_callback_holder.h"
#include "power_common.h"

namespace OHOS {
namespace PowerMgr {
SleepCallbackHolder::SleepCallbackHolder() {}
SleepCallbackHolder::~SleepCallbackHolder() = default;

void SleepCallbackHolder::AddCallback(const sptr<ISyncSleepCallback>& callback, SleepPriority priority)
{
    std::lock_guard<std::mutex> lock(mutex_);
    switch (priority) {
        case SleepPriority::LOW: {
            auto iter = lowPriorityCallbacks_.insert(callback);
            break;
        }
        case SleepPriority::DEFAULT: {
            auto iter = defaultPriorityCallbacks_.insert(callback);
            break;
        }
        case SleepPriority::HIGH: {
            auto iter = highPriorityCallbacks_.insert(callback);
            break;
        }
        default: {
            break;
        }
    }
}

std::set<sptr<ISyncSleepCallback>> SleepCallbackHolder::GetHighPriorityCallbacks()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return highPriorityCallbacks_;
}

std::set<sptr<ISyncSleepCallback>> SleepCallbackHolder::GetDefaultPriorityCallbacks()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return defaultPriorityCallbacks_;
}

std::set<sptr<ISyncSleepCallback>> SleepCallbackHolder::GetLowPriorityCallbacks()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return lowPriorityCallbacks_;
}

void SleepCallbackHolder::RemoveCallback(const sptr<ISyncSleepCallback>& callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    RemoveCallback(lowPriorityCallbacks_, callback);
    RemoveCallback(defaultPriorityCallbacks_, callback);
    RemoveCallback(highPriorityCallbacks_, callback);
}

void SleepCallbackHolder::RemoveCallback(
    std::set<sptr<ISyncSleepCallback>>& callbacks, const sptr<ISyncSleepCallback>& callback)
{
    auto iter = callbacks.find(callback);
    if (iter == callbacks.end()) {
        return;
    }
    callbacks.erase(iter);
}

} // namespace PowerMgr
} // namespace OHOS
