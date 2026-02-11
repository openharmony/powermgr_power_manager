/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "suspend_takeover_callback_holder.h"
#include "power_common.h"

namespace OHOS {
namespace PowerMgr {
TakeOverSuspendCallbackHolder::TakeOverSuspendCallbackHolder() {}
TakeOverSuspendCallbackHolder::~TakeOverSuspendCallbackHolder() = default;

void TakeOverSuspendCallbackHolder::AddCallback(
    const sptr<ITakeOverSuspendCallback>& callback, TakeOverSuspendPriority priority)
{
    std::lock_guard<ffrt::mutex> lock(mutex_);
    switch (priority) {
        case TakeOverSuspendPriority::LOW: {
            auto iter = lowPriorityCallbacks_.insert(callback);
            break;
        }
        case TakeOverSuspendPriority::DEFAULT: {
            auto iter = defaultPriorityCallbacks_.insert(callback);
            break;
        }
        case TakeOverSuspendPriority::HIGH: {
            auto iter = highPriorityCallbacks_.insert(callback);
            break;
        }
        default:
            break;
    }
    AddCallbackPidUid(callback);
}

void TakeOverSuspendCallbackHolder::AddCallbackPidUid(const sptr<ITakeOverSuspendCallback>& callback)
{
    pid_t pid = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    cachedRegister_.emplace(callback, std::make_pair(pid, uid));
}

using CallbackContainer = TakeOverSuspendCallbackHolder::TakeoverSuspendCallbackContainerType;

CallbackContainer TakeOverSuspendCallbackHolder::GetHighPriorityCallbacks()
{
    std::lock_guard<ffrt::mutex> lock(mutex_);
    return highPriorityCallbacks_;
}

CallbackContainer TakeOverSuspendCallbackHolder::GetDefaultPriorityCallbacks()
{
    std::lock_guard<ffrt::mutex> lock(mutex_);
    return defaultPriorityCallbacks_;
}

CallbackContainer TakeOverSuspendCallbackHolder::GetLowPriorityCallbacks()
{
    std::lock_guard<ffrt::mutex> lock(mutex_);
    return lowPriorityCallbacks_;
}

void TakeOverSuspendCallbackHolder::RemoveCallback(const sptr<ITakeOverSuspendCallback>& callback)
{
    std::lock_guard<ffrt::mutex> lock(mutex_);
    RemoveCallback(lowPriorityCallbacks_, callback);
    RemoveCallback(defaultPriorityCallbacks_, callback);
    RemoveCallback(highPriorityCallbacks_, callback);
    RemoveCallbackPidUid(callback);
}

void TakeOverSuspendCallbackHolder::RemoveCallback(
    TakeOverSuspendCallbackHolder::TakeoverSuspendCallbackContainerType& callbacks,
    const sptr<ITakeOverSuspendCallback>& callback)
{
    auto iter = callbacks.find(callback);
    if (iter == callbacks.end()) {
        POWER_HILOGE(FEATURE_SUSPEND, "Cannot find the takeover suspend callback");
        return;
    }
    callbacks.erase(iter);
}

void TakeOverSuspendCallbackHolder::RemoveCallbackPidUid(const sptr<ITakeOverSuspendCallback>& callback)
{
    auto iter = cachedRegister_.find(callback);
    if (iter != cachedRegister_.end()) {
        cachedRegister_.erase(iter);
    }
}

std::pair<int32_t, int32_t> TakeOverSuspendCallbackHolder::FindCallbackPidUid(
    const sptr<ITakeOverSuspendCallback>& callback)
{
    std::lock_guard<ffrt::mutex> lock(mutex_);
    auto iter = cachedRegister_.find(callback);
    return (iter != cachedRegister_.end()) ? iter->second : std::make_pair(0, 0);
}
} // namespace PowerMgr
} // namespace OHOS