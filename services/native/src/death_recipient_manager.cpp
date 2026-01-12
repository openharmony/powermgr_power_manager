/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "death_recipient_manager.h"

#include <algorithm>

namespace OHOS {
namespace PowerMgr {

void DeathRecipientManager::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    if (!remote.promote()) {
        POWER_HILOGI(COMP_SVC, "proxy no longer exists, return early");
        return;
    }
    auto strongRef = remote.promote();
    POWER_HILOGI(COMP_SVC, "OnRemoteDied Called");
    RemoveDeathRecipient(strongRef);
}

void DeathRecipientManager::AddDeathRecipient(const sptr<IRemoteObject>& invoker, const CBInfo& info)
{
    RETURN_IF_WITH_LOG(!invoker || !invoker->IsProxyObject(), "invoker invalid");
    std::lock_guard lock(callbacksMutex_);
    auto iter = clientDeathRecipientMap_.find(invoker);
    if (iter == clientDeathRecipientMap_.end()) {
        invoker->AddDeathRecipient(this);
        clientDeathRecipientMap_.emplace(invoker, std::set<CBInfo>{info});
        POWER_HILOGI(COMP_SVC, "AddDeathRecipient pid:%{public}d, uid:%{public}d, func:%{public}s",
            info.pid, info.uid, info.funcName.c_str());
    } else {
        auto result = iter->second.insert(info);
        if (result.second) {
            POWER_HILOGI(COMP_SVC, "AddDeathRecipient insert pid:%{public}d, uid:%{public}d, func:%{public}s",
                info.pid, info.uid, info.funcName.c_str());
        }
    }
}

void DeathRecipientManager::AddDeathRecipient(
    const sptr<IRemoteObject>& invoker, const sptr<IRemoteObject::DeathRecipient>& recipient)
{
    RETURN_IF_WITH_LOG(!invoker || !invoker->IsProxyObject(), "invoker invalid");
    std::lock_guard lock(callbacksMutex_);
    auto iter = clientDeathRecipientMap_.find(invoker);
    if (iter == clientDeathRecipientMap_.end()) {
        invoker->AddDeathRecipient(recipient);
        invoker->AddDeathRecipient(this);
        clientDeathRecipientMap_.emplace(invoker, std::set<CBInfo>{});
        POWER_HILOGI(COMP_SVC, "AddDeathRecipient Success");
    }
}

void DeathRecipientManager::RemoveDeathRecipient(const sptr<IRemoteObject>& token)
{
    std::lock_guard lock(callbacksMutex_);
    auto iter = clientDeathRecipientMap_.find(token);
    if (iter != clientDeathRecipientMap_.end()) {
        std::for_each(iter->second.begin(), iter->second.end(), [&token](const CBInfo& info) {
            if (info.func) {
                info.func(token);
            }
            POWER_HILOGI(COMP_SVC, "RemoveDeathRecipient uid:%{public}d pid:%{public}d func:%{public}s",
                info.uid, info.pid, info.funcName.c_str());
        });
        POWER_HILOGI(COMP_SVC, "RemoveDeathRecipient Success");
        clientDeathRecipientMap_.erase(iter);
    }
}
} // namespace PowerMgr
} // namespace OHOS