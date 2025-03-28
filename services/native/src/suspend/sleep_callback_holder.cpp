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
            lowPriorityCallbacks_.insert(callback);
            break;
        }
        case SleepPriority::DEFAULT: {
            defaultPriorityCallbacks_.insert(callback);
            break;
        }
        case SleepPriority::HIGH: {
            highPriorityCallbacks_.insert(callback);
            break;
        }
        default: {
            break;
        }
    }
    AddCallbackPidUid(callback);
}

void SleepCallbackHolder::AddCallbackPidUid(const sptr<ISyncSleepCallback>& callback)
{
    pid_t pid = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    cachedRegister_.emplace(callback, std::make_pair(pid, uid));
}

SleepCallbackHolder::SleepCallbackContainerType SleepCallbackHolder::GetHighPriorityCallbacks()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return highPriorityCallbacks_;
}

SleepCallbackHolder::SleepCallbackContainerType SleepCallbackHolder::GetDefaultPriorityCallbacks()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return defaultPriorityCallbacks_;
}

SleepCallbackHolder::SleepCallbackContainerType SleepCallbackHolder::GetLowPriorityCallbacks()
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
    RemoveCallbackPidUid(callback);
}

void SleepCallbackHolder::RemoveCallback(
    SleepCallbackHolder::SleepCallbackContainerType& callbacks, const sptr<ISyncSleepCallback>& callback)
{
    auto iter = callbacks.find(callback);
    if (iter == callbacks.end()) {
        POWER_HILOGE(FEATURE_SUSPEND, "Cannot find the sleep callback");
        return;
    }
    callbacks.erase(iter);
}

void SleepCallbackHolder::RemoveCallbackPidUid(const sptr<ISyncSleepCallback>& callback)
{
    auto iter = cachedRegister_.find(callback);
    if (iter != cachedRegister_.end()) {
        cachedRegister_.erase(iter);
    }
}

std::pair<int32_t, int32_t> SleepCallbackHolder::FindCallbackPidUid(const sptr<ISyncSleepCallback>& callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto iter = cachedRegister_.find(callback);
    return (iter != cachedRegister_.end()) ? iter->second : std::make_pair(0, 0);
}

} // namespace PowerMgr
} // namespace OHOS
