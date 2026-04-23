/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "hibernate_controller.h"
#include <datetime_ex.h>
#include "power_log.h"
#include "power_common.h"
#include "system_suspend_controller.h"

namespace OHOS {
namespace PowerMgr {
HibernateStatus HibernateController::Hibernate(bool clearMemory)
{
    if (SystemSuspendController::GetInstance().Hibernate()) {
        return HibernateStatus::HIBERNATE_SUCCESS;
    }
    return HibernateStatus::HIBERNATE_FAILURE;
}

void HibernateController::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    RETURN_IF((remote == nullptr) || (remote.promote() == nullptr))
    POWER_HILOGW(FEATURE_SUSPEND, "object dead, need remove the callback");
    auto callback = iface_cast<ISyncHibernateCallback>(remote.promote());
    UnregisterSyncHibernateCallback(callback);
}

void HibernateController::RegisterSyncHibernateCallback(const sptr<ISyncHibernateCallback>& cb)
{
    RETURN_IF((cb == nullptr) || (cb->AsObject() == nullptr));
    std::lock_guard<std::mutex> lock(mutex_);
    auto result = callbacks_.insert(cb);
    if (result.second) {
        cb->AsObject()->AddDeathRecipient(this);
    }
    pid_t pid = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    cachedRegister_.emplace(cb, std::make_pair(pid, uid));
}

void HibernateController::UnregisterSyncHibernateCallback(const sptr<ISyncHibernateCallback>& cb)
{
    RETURN_IF((cb == nullptr) || (cb->AsObject() == nullptr));
    std::lock_guard<std::mutex> lock(mutex_);
    size_t eraseNum = callbacks_.erase(cb);
    if (eraseNum == 0) {
        POWER_HILOGE(FEATURE_SUSPEND, "Cannot remove the hibernate callback");
        return;
    }
    cb->AsObject()->RemoveDeathRecipient(this);
    auto iter = cachedRegister_.find(cb);
    if (iter != cachedRegister_.end()) {
        cachedRegister_.erase(iter);
    }
}

void HibernateController::PreHibernate()
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& cb : callbacks_) {
        auto iter = cachedRegister_.find(cb);
        auto pidUid = (iter != cachedRegister_.end()) ? iter->second : std::make_pair(0, 0);
        if (cb != nullptr) {
            int64_t start = GetTickCount();
            POWER_HILOGI(FEATURE_SUSPEND, "PreHibernateCb P=%{public}dU=%{public}d", pidUid.first, pidUid.second);
            cb->OnSyncHibernate();
            int64_t cost = GetTickCount() - start;
            POWER_HILOGI(FEATURE_SUSPEND, "PreHcb E P=%{public}dU=%{public}dT=%{public}ld",
                pidUid.first, pidUid.second, static_cast<long>(cost));
        }
    }
    prepared_ = true;
}

void HibernateController::PostHibernate(bool hibernateResult)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!prepared_) {
        POWER_HILOGE(FEATURE_SUSPEND, "No need to run OnSyncWakeup");
        return;
    }
    prepared_ = false;
    for (const auto& cb : callbacks_) {
        auto iter = cachedRegister_.find(cb);
        auto pidUid = ((iter != cachedRegister_.end()) ? iter->second : std::make_pair(0, 0));
        if (cb != nullptr) {
            // PostHibernate calling callback pid uid
            int64_t start = GetTickCount();
            POWER_HILOGI(FEATURE_SUSPEND, "PostHibernateCb P=%{public}dU=%{public}d", pidUid.first, pidUid.second);
            cb->OnSyncWakeup(hibernateResult);
            int64_t cost = GetTickCount() - start;
            POWER_HILOGI(FEATURE_SUSPEND, "PostHcb E P=%{public}dU=%{public}dT=%{public}ld",
                pidUid.first, pidUid.second, static_cast<long>(cost));
        }
    }
}
} // namespace PowerMgr
} // namespace OHOS
