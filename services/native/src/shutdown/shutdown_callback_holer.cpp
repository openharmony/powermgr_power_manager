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

#include "shutdown_callback_holer.h"

#include "power_common.h"

namespace OHOS {
namespace PowerMgr {
void ShutdownCallbackHolder::OnRemoteDied(const wptr<IRemoteObject>& object)
{
    RETURN_IF((object == nullptr) || (object.promote() == nullptr))
    POWER_HILOGW(FEATURE_SHUTDOWN, "object dead, need remove the callback");
    RemoveCallback(object.promote());
}

void ShutdownCallbackHolder::AddCallback(const sptr<IRemoteObject>& callback, ShutdownPriority priority)
{
    std::unique_lock<std::mutex> lock(mutex_);
    switch (priority) {
        case ShutdownPriority::LOW: {
            auto iter = lowPriorityCallbacks_.insert(callback);
            if (iter.second) {
                callback->AddDeathRecipient(this);
            }
            break;
        }
        case ShutdownPriority::DEFAULT: {
            auto iter = defaultPriorityCallbacks_.insert(callback);
            if (iter.second) {
                callback->AddDeathRecipient(this);
            }
            break;
        }
        case ShutdownPriority::HIGH: {
            auto iter = highPriorityCallbacks_.insert(callback);
            if (iter.second) {
                callback->AddDeathRecipient(this);
            }
            break;
        }
        default: {
            break;
        }
    }
}

std::set<sptr<IRemoteObject>> ShutdownCallbackHolder::GetHighPriorityCallbacks()
{
    std::unique_lock<std::mutex> lock(mutex_);
    return highPriorityCallbacks_;
}

std::set<sptr<IRemoteObject>> ShutdownCallbackHolder::GetDefaultPriorityCallbacks()
{
    std::unique_lock<std::mutex> lock(mutex_);
    return defaultPriorityCallbacks_;
}

std::set<sptr<IRemoteObject>> ShutdownCallbackHolder::GetLowPriorityCallbacks()
{
    std::unique_lock<std::mutex> lock(mutex_);
    return lowPriorityCallbacks_;
}

void ShutdownCallbackHolder::RemoveCallback(const sptr<IRemoteObject>& callback)
{
    std::unique_lock<std::mutex> lock(mutex_);
    RemoveCallback(lowPriorityCallbacks_, callback);
    RemoveCallback(defaultPriorityCallbacks_, callback);
    RemoveCallback(highPriorityCallbacks_, callback);
}

void ShutdownCallbackHolder::RemoveCallback(
    std::set<sptr<IRemoteObject>>& callbacks, const sptr<IRemoteObject>& callback)
{
    auto iter = callbacks.find(callback);
    if (iter == callbacks.end()) {
        return;
    }
    callbacks.erase(iter);
}

} // namespace PowerMgr
} // namespace OHOS
