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
#include "power_log.h"
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

void HibernateController::RegisterSyncHibernateCallback(const sptr<ISyncHibernateCallback>& cb)
{
    std::lock_guard<std::mutex> lock(mutex_);
    callbacks_.insert(cb);
}

void HibernateController::UnregisterSyncHibernateCallback(const sptr<ISyncHibernateCallback>& cb)
{
    std::lock_guard<std::mutex> lock(mutex_);
    size_t eraseNum = callbacks_.erase(cb);
    if (eraseNum == 0) {
        POWER_HILOGE(FEATURE_SUSPEND, "Cannot remove the hibernate callback");
    }
}

void HibernateController::PreHibernate() const
{
    for (const auto &cb : callbacks_) {
        if (cb != nullptr) {
            cb->OnSyncHibernate();
        }
    }
}

void HibernateController::PostHibernate(bool hibernateResult) const
{
    for (const auto &cb : callbacks_) {
        if (cb != nullptr) {
            cb->OnSyncWakeup(hibernateResult);
        }
    }
}
} // namespace PowerMgr
} // namespace OHOS
