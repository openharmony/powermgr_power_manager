/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#include "running_lock_callback_manager.h"

#ifdef POWER_MANAGER_ENABLE_MONITOR_RUNNING_LOCK_CHANGE

#include "power_common.h"
#include "power_log.h"
#include <actions/irunning_lock_action.h>

namespace OHOS {
namespace PowerMgr {

void RunningLockCallbackManager::Register(
    const sptr<IRemoteObject>& callback, int32_t pid, int32_t uid, uint64_t displayId)
{
    RETURN_IF(callback == nullptr);
    std::lock_guard<ffrt::mutex> lock(mutex_);
    auto range = callbacks_.equal_range(callback);
    for (auto it = range.first; it != range.second; ++it) {
        if (std::get<CALLBACK_TUPLE_INDEX_DISPLAY_ID>(it->second) == displayId) {
            POWER_HILOGI(FEATURE_RUNNING_LOCK, "RegisterRunningLockChangedCallback already exists D=%{public}" PRIu64,
                displayId);
            return;
        }
    }
    callbacks_.emplace(callback, std::make_tuple(pid, uid, displayId));
#ifdef POWER_MANAGER_ENABLE_DISPLAY_ID_FILTERING
    POWER_HILOGI(FEATURE_RUNNING_LOCK, "runningLockChangedCallbacks_.size:%{public}zu, D=%{public}" PRIu64,
        callbacks_.size(), displayId);
#else
    POWER_HILOGI(FEATURE_RUNNING_LOCK, "runningLockChangedCallbacks_.size:%{public}zu", callbacks_.size());
#endif
}

void RunningLockCallbackManager::UnRegister(const sptr<IRemoteObject>& callback, uint64_t displayId)
{
    RETURN_IF(callback == nullptr);
    std::lock_guard<ffrt::mutex> lock(mutex_);
    auto range = callbacks_.equal_range(callback);
    for (auto it = range.first; it != range.second; ++it) {
        if (std::get<CALLBACK_TUPLE_INDEX_DISPLAY_ID>(it->second) == displayId) {
#ifdef POWER_MANAGER_ENABLE_DISPLAY_ID_FILTERING
            POWER_HILOGI(FEATURE_RUNNING_LOCK,
                "UnRegisterRunningLockChangedCallback D=%{public}" PRIu64 ", size:%{public}zu", displayId,
                callbacks_.size() - 1);
#else
            POWER_HILOGI(
                FEATURE_RUNNING_LOCK, "UnRegisterRunningLockChangedCallback size:%{public}zu", callbacks_.size() - 1);
#endif
            callbacks_.erase(it);
            return;
        }
    }
#ifdef POWER_MANAGER_ENABLE_DISPLAY_ID_FILTERING
    POWER_HILOGI(FEATURE_RUNNING_LOCK, "UnRegisterRunningLockChangedCallback not found D=%{public}" PRIu64, displayId);
#else
    POWER_HILOGI(FEATURE_RUNNING_LOCK, "UnRegisterRunningLockChangedCallback not found");
#endif
}

void RunningLockCallbackManager::RemoveAll(const sptr<IRemoteObject>& callback)
{
    RETURN_IF(callback == nullptr);
    std::lock_guard<ffrt::mutex> lock(mutex_);
    auto range = callbacks_.equal_range(callback);
    size_t count = std::distance(range.first, range.second);
#ifdef POWER_MANAGER_ENABLE_DISPLAY_ID_FILTERING
    for (auto it = range.first; it != range.second; ++it) {
        POWER_HILOGI(FEATURE_RUNNING_LOCK, "RemoveAllRunningLockChangedCallbacks D=%{public}" PRIu64,
            std::get<CALLBACK_TUPLE_INDEX_DISPLAY_ID>(it->second));
    }
#endif
    callbacks_.erase(range.first, range.second);
    POWER_HILOGI(FEATURE_RUNNING_LOCK, "RemoveAllRunningLockChangedCallbacks removed %{public}zu, size:%{public}zu",
        count, callbacks_.size());
}

bool RunningLockCallbackManager::HasCallbacks(const sptr<IRemoteObject>& callback)
{
    if (callback == nullptr) {
        return false;
    }
    std::lock_guard<ffrt::mutex> lock(mutex_);
    auto range = callbacks_.equal_range(callback);
    return range.first != range.second;
}

size_t RunningLockCallbackManager::GetCallbackCount()
{
    std::lock_guard<ffrt::mutex> lock(mutex_);
    return callbacks_.size();
}

void RunningLockCallbackManager::NotifyScreenRunningLockChanged(RunningLockChangeState state, uint64_t displayId)
{
    std::vector<std::pair<sptr<IRemoteObject>, std::tuple<int32_t, int32_t, uint64_t>>> callbacksCopy;
    {
        std::lock_guard<ffrt::mutex> lock(mutex_);
        for (const auto& callbackPair : callbacks_) {
            callbacksCopy.push_back(callbackPair);
        }
    }
#ifdef POWER_MANAGER_ENABLE_DISPLAY_ID_FILTERING
    POWER_HILOGI(FEATURE_RUNNING_LOCK,
        "NotifyScreenRunningLockChanged state=%{public}d, D=%{public}" PRIu64 ", CBs=%{public}zu",
        static_cast<uint32_t>(state), displayId, callbacksCopy.size());
#else
    POWER_HILOGI(FEATURE_RUNNING_LOCK, "NotifyScreenRunningLockChanged state=%{public}d, CBs=%{public}zu",
        static_cast<uint32_t>(state), callbacksCopy.size());
#endif
    for (const auto& callbackPair : callbacksCopy) {
        auto callback = callbackPair.first;
        sptr<IRunningLockChangedCallback> screenLockCallback = iface_cast<IRunningLockChangedCallback>(callback);
        if (screenLockCallback != nullptr) {
#ifdef POWER_MANAGER_ENABLE_DISPLAY_ID_FILTERING
            uint64_t callbackDisplayId = std::get<CALLBACK_TUPLE_INDEX_DISPLAY_ID>(callbackPair.second);
            if (callbackDisplayId != displayId) {
                POWER_HILOGD(FEATURE_RUNNING_LOCK,
                    "SRL CB skipped: P=%{public}d U=%{public}d regD=%{public}" PRIu64 " != notifyD=%{public}" PRIu64,
                    std::get<CALLBACK_TUPLE_INDEX_PID>(callbackPair.second),
                    std::get<CALLBACK_TUPLE_INDEX_UID>(callbackPair.second), callbackDisplayId, displayId);
                continue;
            }
            POWER_HILOGI(FEATURE_RUNNING_LOCK, "SRL CB: P=%{public}d U=%{public}d D=%{public}" PRIu64,
                std::get<CALLBACK_TUPLE_INDEX_PID>(callbackPair.second),
                std::get<CALLBACK_TUPLE_INDEX_UID>(callbackPair.second), displayId);
#else
            POWER_HILOGI(FEATURE_RUNNING_LOCK, "SRL CB: P=%{public}d U=%{public}d",
                std::get<CALLBACK_TUPLE_INDEX_PID>(callbackPair.second),
                std::get<CALLBACK_TUPLE_INDEX_UID>(callbackPair.second));
#endif
            screenLockCallback->OnAsyncScreenRunningLockChanged(state, displayId);
        } else {
            POWER_HILOGD(FEATURE_RUNNING_LOCK, "SRL CB iface_cast failed: P=%{public}d U=%{public}d",
                std::get<CALLBACK_TUPLE_INDEX_PID>(callbackPair.second),
                std::get<CALLBACK_TUPLE_INDEX_UID>(callbackPair.second));
        }
    }
}

#ifdef POWER_MANAGER_ENABLE_DISPLAY_ID_FILTERING
std::set<uint64_t> RunningLockCallbackManager::GetRegisteredDisplayIds()
{
    std::set<uint64_t> displayIds;
    for (const auto& callbackPair : callbacks_) {
        uint64_t callbackDisplayId = std::get<CALLBACK_TUPLE_INDEX_DISPLAY_ID>(callbackPair.second);
        displayIds.insert(callbackDisplayId);
    }
    return displayIds;
}

uint32_t RunningLockCallbackManager::GetScreenLockCountInternal(uint64_t displayId)
{
    uint32_t count = 0;
    auto it = screenLockCountPerDisplayId_.find(displayId);
    if (it != screenLockCountPerDisplayId_.end()) {
        count = it->second;
    }
    if (displayId != RUNNINGLOCK_DISPLAY_ID_ALL) {
        auto allIt = screenLockCountPerDisplayId_.find(RUNNINGLOCK_DISPLAY_ID_ALL);
        if (allIt != screenLockCountPerDisplayId_.end()) {
            count += allIt->second;
        }
    }
    return count;
}

void RunningLockCallbackManager::UpdateScreenLockCount(bool active, uint64_t displayId)
{
    if (active) {
        screenLockCountPerDisplayId_[displayId]++;
        return;
    }
    auto it = screenLockCountPerDisplayId_.find(displayId);
    if (it != screenLockCountPerDisplayId_.end() && it->second > 0) {
        it->second--;
        if (it->second == 0) {
            screenLockCountPerDisplayId_.erase(it);
        }
    }
}

void RunningLockCallbackManager::CollectNotifications(
    bool active, uint64_t displayId, std::vector<std::pair<RunningLockChangeState, uint64_t>>& notifications)
{
    RunningLockChangeState state =
        active ? RunningLockChangeState::RUNNINGLOCK_STATE_LOCKED : RunningLockChangeState::RUNNINGLOCK_STATE_UNLOCKED;
    uint32_t countAfter = GetScreenLockCountInternal(displayId);
    uint32_t countBefore = countAfter + (active ? 0 : 1) - (active ? 1 : 0);
    if (active && countBefore == 0 && countAfter == 1) {
        notifications.emplace_back(state, displayId);
    } else if (!active && countBefore == 1 && countAfter == 0) {
        notifications.emplace_back(state, displayId);
    }
}

void RunningLockCallbackManager::HandleScreenLockNotify(bool active, uint64_t displayId)
{
    POWER_HILOGI(FEATURE_RUNNING_LOCK, "HandleScreenLockNotify active=%{public}d, D=%{public}" PRIu64,
        static_cast<int32_t>(active), displayId);
    std::set<uint64_t> registeredDisplayIds;
    std::vector<std::pair<RunningLockChangeState, uint64_t>> notifications;
    {
        std::scoped_lock lock(mutex_, countMutex_);
        registeredDisplayIds = GetRegisteredDisplayIds();
        UpdateScreenLockCount(active, displayId);
        if (displayId == RUNNINGLOCK_DISPLAY_ID_ALL) {
            registeredDisplayIds.erase(RUNNINGLOCK_DISPLAY_ID_ALL);
            for (auto regDisplayId : registeredDisplayIds) {
                CollectNotifications(active, regDisplayId, notifications);
            }
        }
        CollectNotifications(active, displayId, notifications);
    }
    if (notifications.empty()) {
        POWER_HILOGD(FEATURE_RUNNING_LOCK, "HandleScreenLockNotify no notifications: count did not cross threshold");
        return;
    }
    POWER_HILOGI(
        FEATURE_RUNNING_LOCK, "HandleScreenLockNotify collected %{public}zu notifications", notifications.size());
    for (const auto& notifyPair : notifications) {
        NotifyScreenRunningLockChanged(notifyPair.first, notifyPair.second);
    }
}
#endif

} // namespace PowerMgr
} // namespace OHOS
#endif