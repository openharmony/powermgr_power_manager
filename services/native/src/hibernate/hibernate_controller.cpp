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
HibernateController::HibernateController()
{
    deathRecipient_ = new HibernateDeathRecipient(*this);
}

HibernateStatus HibernateController::Hibernate(bool clearMemory)
{
    if (SystemSuspendController::GetInstance().Hibernate()) {
        return HibernateStatus::HIBERNATE_SUCCESS;
    }
    return HibernateStatus::HIBERNATE_FAILURE;
}

void HibernateController::HibernateDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    POWER_HILOGW(FEATURE_SUSPEND, "object dead, need remove the hibernate callback");
    if (remote == nullptr || remote.promote() == nullptr) {
        POWER_HILOGE(FEATURE_SUSPEND, "remote is nullptr");
        return;
    }
    auto callback = iface_cast<ISyncHibernateCallback>(remote.promote());
    owner_.UnregisterSyncHibernateCallback(callback);
}

void HibernateController::AddCallbackToHolder(
    const sptr<ISyncHibernateCallback>& cb, HibernateCallbackPriority priority)
{
    switch (priority) {
        case HibernateCallbackPriority::LOW: {
            auto iter = lowPriorityCallbacks_.insert(cb);
            if (iter.second) {
                cb->AsObject()->AddDeathRecipient(deathRecipient_);
            }
            break;
        }
        case HibernateCallbackPriority::DEFAULT: {
            auto iter = defaultPriorityCallbacks_.insert(cb);
            if (iter.second) {
                cb->AsObject()->AddDeathRecipient(deathRecipient_);
            }
            break;
        }
        case HibernateCallbackPriority::HIGH: {
            auto iter = highPriorityCallbacks_.insert(cb);
            if (iter.second) {
                cb->AsObject()->AddDeathRecipient(deathRecipient_);
            }
            break;
        }
        default:
            break;
    }
}

void HibernateController::AddCallbackPidUid(const sptr<ISyncHibernateCallback>& cb)
{
    pid_t pid = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    cachedRegister_.emplace(cb, std::make_pair(pid, uid));
}

void HibernateController::RegisterSyncHibernateCallback(
    const sptr<ISyncHibernateCallback>& cb, HibernateCallbackPriority priority)
{
    RETURN_IF((cb == nullptr) || (cb->AsObject() == nullptr));
    std::lock_guard<std::mutex> lock(mutex_);
    AddCallbackToHolder(cb, priority);
    AddCallbackPidUid(cb);
}

void HibernateController::RemoveCallbackFromHolder(const sptr<ISyncHibernateCallback>& cb)
{
    auto iter = lowPriorityCallbacks_.find(cb);
    if (iter != lowPriorityCallbacks_.end()) {
        lowPriorityCallbacks_.erase(iter);
    }
    iter = defaultPriorityCallbacks_.find(cb);
    if (iter != defaultPriorityCallbacks_.end()) {
        defaultPriorityCallbacks_.erase(iter);
    }
    iter = highPriorityCallbacks_.find(cb);
    if (iter != highPriorityCallbacks_.end()) {
        highPriorityCallbacks_.erase(iter);
    }
}

void HibernateController::RemoveCallbackPidUid(const sptr<ISyncHibernateCallback>& cb)
{
    auto iter = cachedRegister_.find(cb);
    if (iter != cachedRegister_.end()) {
        cachedRegister_.erase(iter);
    }
}

void HibernateController::UnregisterSyncHibernateCallback(const sptr<ISyncHibernateCallback>& cb)
{
    RETURN_IF((cb == nullptr) || (cb->AsObject() == nullptr));
    std::lock_guard<std::mutex> lock(mutex_);
    RemoveCallbackFromHolder(cb);
    cb->AsObject()->RemoveDeathRecipient(deathRecipient_);
    RemoveCallbackPidUid(cb);
}

void HibernateController::TriggerCallbacks(const CallbackContainerType& callbacks,
    bool isPreHibernate, bool hibernateResult)
{
    for (const auto& cb : callbacks) {
        if (cb == nullptr) {
            continue;
        }
        auto iter = cachedRegister_.find(cb);
        auto pidUid = (iter != cachedRegister_.end()) ? iter->second : std::make_pair(0, 0);
        int64_t start = GetTickCount();
        if (isPreHibernate) {
            POWER_HILOGI(FEATURE_SUSPEND, "PreHcb P=%{public}dU=%{public}d", pidUid.first, pidUid.second);
            cb->OnSyncHibernate();
        } else {
            POWER_HILOGI(FEATURE_SUSPEND, "PostHcb P=%{public}dU=%{public}d", pidUid.first, pidUid.second);
            cb->OnSyncWakeup(hibernateResult);
        }
        int64_t cost = GetTickCount() - start;
        POWER_HILOGI(FEATURE_SUSPEND, "HcbEnd P=%{public}dU=%{public}dT=%{public}ld",
            pidUid.first, pidUid.second, static_cast<long>(cost));
    }
}

void HibernateController::PreHibernate()
{
    std::lock_guard<std::mutex> lock(mutex_);
    prepared_ = true;
    TriggerCallbacks(highPriorityCallbacks_, true);
    TriggerCallbacks(defaultPriorityCallbacks_, true);
    TriggerCallbacks(lowPriorityCallbacks_, true);
}

void HibernateController::PostHibernate(bool hibernateResult)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!prepared_) {
        POWER_HILOGE(FEATURE_SUSPEND, "No need to run OnSyncWakeup");
        return;
    }
    prepared_ = false;
    TriggerCallbacks(highPriorityCallbacks_, false, hibernateResult);
    TriggerCallbacks(defaultPriorityCallbacks_, false, hibernateResult);
    TriggerCallbacks(lowPriorityCallbacks_, false, hibernateResult);
}
} // namespace PowerMgr
} // namespace OHOS
