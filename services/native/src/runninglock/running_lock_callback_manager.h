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

#ifndef POWERMGR_RUNNING_LOCK_CALLBACK_MANAGER_H
#define POWERMGR_RUNNING_LOCK_CALLBACK_MANAGER_H

#include <cinttypes>
#include <map>
#include <mutex>
#include <set>
#include <tuple>
#include <vector>

#include <iremote_object.h>

#include "ffrt_utils.h"
#include "irunning_lock_changed_callback.h"
#include "running_lock_info.h"

namespace OHOS {
namespace PowerMgr {

#ifdef POWER_MANAGER_ENABLE_MONITOR_RUNNING_LOCK_CHANGE
class RunningLockCallbackManager {
public:
    static constexpr size_t CALLBACK_TUPLE_INDEX_PID = 0;
    static constexpr size_t CALLBACK_TUPLE_INDEX_UID = 1;
    static constexpr size_t CALLBACK_TUPLE_INDEX_DISPLAY_ID = 2;

    RunningLockCallbackManager() = default;
    ~RunningLockCallbackManager() = default;

    void Register(const sptr<IRemoteObject>& callback, int32_t pid, int32_t uid, uint64_t displayId);
    void UnRegister(const sptr<IRemoteObject>& callback, uint64_t displayId);
    void RemoveAll(const sptr<IRemoteObject>& callback);
    bool HasCallbacks(const sptr<IRemoteObject>& callback);
    size_t GetCallbackCount();
    void NotifyScreenRunningLockChanged(RunningLockChangeState state, uint64_t displayId = UINT64_MAX);

#ifdef POWER_MANAGER_ENABLE_DISPLAY_ID_FILTERING
    void HandleScreenLockNotify(bool active, uint64_t displayId);
#endif

private:
    std::multimap<sptr<IRemoteObject>, std::tuple<int32_t, int32_t, uint64_t>> callbacks_;
    ffrt::mutex mutex_;

#ifdef POWER_MANAGER_ENABLE_DISPLAY_ID_FILTERING
    std::set<uint64_t> GetRegisteredDisplayIds();
    uint32_t GetScreenLockCountInternal(uint64_t displayId);
    void UpdateScreenLockCount(bool active, uint64_t displayId);
    void CollectNotifications(
        bool active, uint64_t displayId, std::vector<std::pair<RunningLockChangeState, uint64_t>>& notifications);

    std::map<uint64_t, uint32_t> screenLockCountPerDisplayId_;
    ffrt::mutex countMutex_;
#endif
};
#endif

} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_RUNNING_LOCK_CALLBACK_MANAGER_H