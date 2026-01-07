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

namespace OHOS {
namespace PowerMgr {

DeathRecipientManager& DeathRecipientManager::GetInstance()
{
    static DeathRecipientManager instance;
    return instance;
}

void DeathRecipientManager::CommonDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    if (!remote.promote()) {
        POWER_HILOGI(COMP_SVC, "proxy no longer exists, return early");
        return;
    }
    auto strongRef = remote.promote();
    POWER_HILOGI(COMP_SVC, "OnRemoteDied Called, pid:%{public}d, uid:%{public}d, func:%{public}s",
        callbackInfo_.pid, callbackInfo_.uid, callbackInfo_.funcName.c_str());
    if (callbackInfo_.func) {
        callbackInfo_.func(strongRef);
    }
}

void DeathRecipientManager::AddDeathRecipient(
    int32_t uid, const sptr<IRemoteObject>& invoker, const sptr<IRemoteObject::DeathRecipient>& recipient)
{
    RETURN_IF_WITH_LOG(!invoker || !invoker->IsProxyObject(), "invoker invalid");
    std::lock_guard lock(callbacksMutex_);
    if (clientDeathRecipientMap_.find(uid) == clientDeathRecipientMap_.end()) {
        invoker->AddDeathRecipient(recipient);
        clientDeathRecipientMap_.emplace(uid, invoker);
    }
}

void DeathRecipientManager::AddDeathRecipient(const sptr<IRemoteObject>& invoker, const CBInfo& callbackInfo)
{
    RETURN_IF_WITH_LOG(!invoker || !invoker->IsProxyObject(), "invoker invalid");
    std::lock_guard lock(callbacksMutex_);
    if (clientDeathRecipientMap_.find(callbackInfo.uid) == clientDeathRecipientMap_.end()) {
        sptr<CommonDeathRecipient> drt = sptr<CommonDeathRecipient>::MakeSptr(callbackInfo);
        invoker->AddDeathRecipient(drt);
        clientDeathRecipientMap_.emplace(callbackInfo.uid, invoker);
    }
}

void DeathRecipientManager::RemoveDeathRecipient(int32_t uid)
{
    std::lock_guard lock(callbacksMutex_);
    auto iter = clientDeathRecipientMap_.find(uid);
    if (iter != clientDeathRecipientMap_.end()) {
        clientDeathRecipientMap_.erase(iter);
    }
}
} // namespace PowerMgr
} // namespace OHOS