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

#include "client_lifecycle.h"

#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "power_log.h"
#include "power_mgr_errors.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace PowerMgr {
sptr<IPowerMgr> ClientLifeCycle::proxy_;
sptr<IRemoteObject::DeathRecipient> ClientLifeCycle::deathRecipient_;
std::mutex ClientLifeCycle::mutex_;

sptr<IPowerMgr> ClientLifeCycle::GetProxy()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (proxy_ == nullptr) {
        auto ret = Connect();
        if (ret != ERR_OK) {
            POWER_HILOGE(COMP_FWK, "Failed to connect PowerMgrService");
        }
    }
    return proxy_;
}

ErrCode ClientLifeCycle::Connect()
{
    if (proxy_ != nullptr) {
        return ERR_OK;
    }

    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        POWER_HILOGE(COMP_FWK, "Failed to obtain SystemAbilityMgr");
        return E_GET_SYSTEM_ABILITY_MANAGER_FAILED;
    }
    sptr<IRemoteObject> remoteObject_ = sam->CheckSystemAbility(POWER_MANAGER_SERVICE_ID);
    if (remoteObject_ == nullptr) {
        POWER_HILOGE(COMP_FWK, "Check SystemAbility failed");
        return E_GET_POWER_SERVICE_FAILED;
    }

    deathRecipient_ = sptr<IRemoteObject::DeathRecipient>(new LifeCycleRecipient());
    if (deathRecipient_ == nullptr) {
        POWER_HILOGE(COMP_FWK, "Failed to create LifeCycleRecipient");
        return ERR_NO_MEMORY;
    }
    if ((remoteObject_->IsProxyObject()) && (!remoteObject_->AddDeathRecipient(deathRecipient_))) {
        POWER_HILOGE(COMP_FWK, "Add death recipient to PowerMgr service failed");
        return E_ADD_DEATH_RECIPIENT_FAILED;
    }

    proxy_ = iface_cast<IPowerMgr>(remoteObject_);
    POWER_HILOGI(COMP_FWK, "Connecting PowerMgrService success");
    return ERR_OK;
}

void ClientLifeCycle::Reset(const wptr<IRemoteObject>& remote)
{
    if (proxy_ == nullptr) {
        return;
    }

    auto service = proxy_->AsObject();
    if ((service != nullptr) && (service == remote.promote())) {
        service->RemoveDeathRecipient(deathRecipient_);
        proxy_ = nullptr;
    }
}

void ClientLifeCycle::LifeCycleRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    if (remote == nullptr) {
        POWER_HILOGW(COMP_FWK, "OnRemoteDied failed, remote is nullptr");
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    ClientLifeCycle::Reset(remote);
    POWER_HILOGW(COMP_FWK, "Recv death notice");
}
} // namespace PowerMgr
} // namespace OHOS
