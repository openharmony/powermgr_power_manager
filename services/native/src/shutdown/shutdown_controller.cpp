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

#include "shutdown_controller.h"

#include <cinttypes>
#include <datetime_ex.h>
#include "ipc_skeleton.h"
#include "iremote_broker.h"
#include "power_common.h"

namespace OHOS {
namespace PowerMgr {
ShutdownController::ShutdownController()
{
    takeoverShutdownCallbackHolder_ = new ShutdownCallbackHolder();
    asyncShutdownCallbackHolder_ = new ShutdownCallbackHolder();
    syncShutdownCallbackHolder_ = new ShutdownCallbackHolder();
}

void ShutdownController::AddCallback(const sptr<ITakeOverShutdownCallback>& callback, ShutdownPriority priority)
{
    RETURN_IF(callback->AsObject() == nullptr)
    takeoverShutdownCallbackHolder_->AddCallback(callback->AsObject(), priority);
    POWER_HILOGI(FEATURE_SHUTDOWN,
        "takeover shutdown callback added, priority=%{public}u, pid=%{public}d, uid=%{public}d", priority,
        IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid());
}

void ShutdownController::AddCallback(const sptr<IAsyncShutdownCallback>& callback, ShutdownPriority priority)
{
    RETURN_IF(callback->AsObject() == nullptr)
    asyncShutdownCallbackHolder_->AddCallback(callback->AsObject(), priority);
    POWER_HILOGI(FEATURE_SHUTDOWN,
        "async shutdown callback added, priority=%{public}u, pid=%{public}d, uid=%{public}d", priority,
        IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid());
}

void ShutdownController::AddCallback(const sptr<ISyncShutdownCallback>& callback, ShutdownPriority priority)
{
    RETURN_IF(callback->AsObject() == nullptr)
    syncShutdownCallbackHolder_->AddCallback(callback->AsObject(), priority);
    POWER_HILOGI(FEATURE_SHUTDOWN,
        "sync shutdown callback added, priority=%{public}u, pid=%{public}d, uid=%{public}d", priority,
        IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid());
}

void ShutdownController::RemoveCallback(const sptr<ITakeOverShutdownCallback>& callback)
{
    RETURN_IF(callback->AsObject() == nullptr)
    takeoverShutdownCallbackHolder_->RemoveCallback(callback->AsObject());
    POWER_HILOGI(FEATURE_SHUTDOWN, "takeover shutdown callback removed, pid=%{public}d, uid=%{public}d",
        IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid());
}

void ShutdownController::RemoveCallback(const sptr<IAsyncShutdownCallback>& callback)
{
    RETURN_IF(callback->AsObject() == nullptr)
    asyncShutdownCallbackHolder_->RemoveCallback(callback->AsObject());
    POWER_HILOGI(FEATURE_SHUTDOWN, "async shutdown callback removed, pid=%{public}d, uid=%{public}d",
        IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid());
}

void ShutdownController::RemoveCallback(const sptr<ISyncShutdownCallback>& callback)
{
    RETURN_IF(callback->AsObject() == nullptr)
    syncShutdownCallbackHolder_->RemoveCallback(callback->AsObject());
    POWER_HILOGI(FEATURE_SHUTDOWN, "sync shutdown callback removed, pid=%{public}d, uid=%{public}d",
        IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid());
}

bool ShutdownController::TriggerTakeOverShutdownCallback(bool isReboot)
{
    bool isTakeover = false;
    auto highPriorityCallbacks = takeoverShutdownCallbackHolder_->GetHighPriorityCallbacks();
    isTakeover |= TriggerTakeOverShutdownCallbackInner(highPriorityCallbacks, isReboot);
    RETURN_IF_WITH_RET(isTakeover, true);
    auto defaultPriorityCallbacks = takeoverShutdownCallbackHolder_->GetDefaultPriorityCallbacks();
    isTakeover |= TriggerTakeOverShutdownCallbackInner(defaultPriorityCallbacks, isReboot);
    RETURN_IF_WITH_RET(isTakeover, true);
    auto lowPriorityCallbacks = takeoverShutdownCallbackHolder_->GetLowPriorityCallbacks();
    isTakeover |= TriggerTakeOverShutdownCallbackInner(lowPriorityCallbacks, isReboot);
    return isTakeover;
}

bool ShutdownController::TriggerTakeOverShutdownCallbackInner(std::set<sptr<IRemoteObject>>& callbacks, bool isReboot)
{
    bool isTakeover = false;
    for (const auto& obj : callbacks) {
        auto callback = iface_cast<ITakeOverShutdownCallback>(obj);
        isTakeover |= callback->OnTakeOverShutdown(isReboot);
    }
    return isTakeover;
}

void ShutdownController::TriggerAsyncShutdownCallback()
{
    auto highPriorityCallbacks = asyncShutdownCallbackHolder_->GetHighPriorityCallbacks();
    TriggerAsyncShutdownCallbackInner(highPriorityCallbacks);
    auto defaultPriorityCallbacks = asyncShutdownCallbackHolder_->GetDefaultPriorityCallbacks();
    TriggerAsyncShutdownCallbackInner(defaultPriorityCallbacks);
    auto lowPriorityCallbacks = asyncShutdownCallbackHolder_->GetLowPriorityCallbacks();
    TriggerAsyncShutdownCallbackInner(lowPriorityCallbacks);
}

void ShutdownController::TriggerAsyncShutdownCallbackInner(std::set<sptr<IRemoteObject>>& callbacks)
{
    for (auto &obj : callbacks) {
        sptr<IAsyncShutdownCallback> callback = iface_cast<IAsyncShutdownCallback>(obj);
        if (callback != nullptr) {
            int64_t start = GetTickCount();
            callback->OnAsyncShutdown();
            int64_t cost = GetTickCount() - start;
            POWER_HILOGD(FEATURE_SHUTDOWN, "Callback finished, cost=%{public}" PRId64 "", cost);
        }
    }
}

void ShutdownController::TriggerSyncShutdownCallback()
{
    auto highPriorityCallbacks = syncShutdownCallbackHolder_->GetHighPriorityCallbacks();
    TriggerSyncShutdownCallbackInner(highPriorityCallbacks);
    auto defaultPriorityCallbacks = syncShutdownCallbackHolder_->GetDefaultPriorityCallbacks();
    TriggerSyncShutdownCallbackInner(defaultPriorityCallbacks);
    auto lowPriorityCallbacks = syncShutdownCallbackHolder_->GetLowPriorityCallbacks();
    TriggerSyncShutdownCallbackInner(lowPriorityCallbacks);
}

void ShutdownController::TriggerSyncShutdownCallbackInner(std::set<sptr<IRemoteObject>>& callbacks)
{
    for (auto &obj : callbacks) {
        sptr<ISyncShutdownCallback> callback = iface_cast<ISyncShutdownCallback>(obj);
        if (callback != nullptr) {
            int64_t start = GetTickCount();
            callback->OnSyncShutdown();
            int64_t cost = GetTickCount() - start;
            POWER_HILOGD(FEATURE_SHUTDOWN, "Callback finished, cost=%{public}" PRId64 "", cost);
        }
    }
}
} // namespace PowerMgr
} // namespace OHOS
