/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "power_mgr_client.h"

#include <datetime_ex.h>
#include <if_system_ability_manager.h>
#include <ipc_skeleton.h>
#include <iservice_registry.h>
#include <string_ex.h>
#include <system_ability_definition.h>

#include "permission.h"
#include "power_common.h"

namespace OHOS {
namespace PowerMgr {
namespace {
constexpr int APP_FIRST_UID = 10000;
}

PowerMgrClient::PowerMgrClient() {}
PowerMgrClient::~PowerMgrClient()
{
    if (proxy_ != nullptr) {
        auto remoteObject = proxy_->AsObject();
        if (remoteObject != nullptr) {
            remoteObject->RemoveDeathRecipient(deathRecipient_);
        }
    }
}

ErrCode PowerMgrClient::Connect()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (proxy_ != nullptr) {
        return ERR_OK;
    }

    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        POWER_HILOGE(MODULE_INNERKIT, "%{public}s:Failed to get Registry!", __func__);
        return E_GET_SYSTEM_ABILITY_MANAGER_FAILED;
    }
    sptr<IRemoteObject> remoteObject_ = sam->CheckSystemAbility(POWER_MANAGER_SERVICE_ID);
    if (remoteObject_ == nullptr) {
        POWER_HILOGE(MODULE_INNERKIT, "GetSystemAbility failed!");
        return E_GET_POWER_SERVICE_FAILED;
    }

    deathRecipient_ = sptr<IRemoteObject::DeathRecipient>(new PowerMgrDeathRecipient());
    if (deathRecipient_ == nullptr) {
        POWER_HILOGE(MODULE_INNERKIT, "%{public}s :Failed to create PowerMgrDeathRecipient!", __func__);
        return ERR_NO_MEMORY;
    }
    if ((remoteObject_->IsProxyObject()) && (!remoteObject_->AddDeathRecipient(deathRecipient_))) {
        POWER_HILOGE(MODULE_INNERKIT, "%{public}s :Add death recipient to PowerMgr service failed.", __func__);
        return E_ADD_DEATH_RECIPIENT_FAILED;
    }

    proxy_ = iface_cast<IPowerMgr>(remoteObject_);
    POWER_HILOGI(MODULE_INNERKIT, "%{public}s :Connecting PowerMgrService success.", __func__);
    return ERR_OK;
}

void PowerMgrClient::ResetProxy(const wptr<IRemoteObject>& remote)
{
    std::lock_guard<std::mutex> lock(mutex_);
    RETURN_IF(proxy_ == nullptr);

    auto serviceRemote = proxy_->AsObject();
    if ((serviceRemote != nullptr) && (serviceRemote == remote.promote())) {
        serviceRemote->RemoveDeathRecipient(deathRecipient_);
        proxy_ = nullptr;
    }
}

void PowerMgrClient::PowerMgrDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    if (remote == nullptr) {
        POWER_HILOGE(MODULE_INNERKIT, "PowerMgrDeathRecipient::OnRemoteDied failed, remote is nullptr.");
        return;
    }

    PowerMgrClient::GetInstance().ResetProxy(remote);
    POWER_HILOGI(MODULE_INNERKIT, "PowerMgrDeathRecipient::Recv death notice.");
}

void PowerMgrClient::RebootDevice(const std::string& reason)
{
    RETURN_IF(Connect() != ERR_OK);
    POWER_HILOGE(MODULE_INNERKIT, "%{public}s called.", __func__);
    proxy_->RebootDevice(reason);
}

void PowerMgrClient::ShutDownDevice(const std::string& reason)
{
    RETURN_IF(Connect() != ERR_OK);
    POWER_HILOGE(MODULE_INNERKIT, "%{public}s called.", __func__);
    proxy_->ShutDownDevice(reason);
}

void PowerMgrClient::SuspendDevice(SuspendDeviceType reason, bool suspendImmed)
{
    RETURN_IF(Connect() != ERR_OK);
    proxy_->SuspendDevice(GetTickCount(), reason, suspendImmed);
    POWER_HILOGI(MODULE_INNERKIT, " Calling SuspendDevice success.");
}

void PowerMgrClient::WakeupDevice(WakeupDeviceType reason, const std::string& details)
{
    RETURN_IF(Connect() != ERR_OK);
    // Param details's length must > 0
    if (details.empty()) {
        POWER_HILOGE(MODULE_INNERKIT, "%{public}s : detail length is 0, Wakeup failed!", __func__);
        return;
    }

    std::string subDetails;
    if (details.length() > MAX_STRING_LENGTH) {
        POWER_HILOGI(MODULE_INNERKIT, "%{public}s : detail = %{public}s, length is larger than %{public}d, \
                do substring!", __func__, details.c_str(), MAX_STRING_LENGTH);
        subDetails = details.substr(0, MAX_STRING_LENGTH);
    } else {
        subDetails = details;
    }
    proxy_->WakeupDevice(GetTickCount(), reason, subDetails);
    POWER_HILOGI(MODULE_INNERKIT, " Calling WakeupDevice success.");
}

void PowerMgrClient::RefreshActivity(UserActivityType type, bool needChangeBacklight)
{
    RETURN_IF_WITH_LOG(type == UserActivityType::USER_ACTIVITY_TYPE_ATTENTION, " is not supported!");
    RETURN_IF(Connect() != ERR_OK);

    proxy_->RefreshActivity(GetTickCount(), type, needChangeBacklight);
    POWER_HILOGI(MODULE_INNERKIT, " Calling RefreshActivity Success!");
}

bool PowerMgrClient::ForceSuspendDevice()
{
    RETURN_IF_WITH_RET(Connect() != ERR_OK, false);
    bool ret = proxy_->ForceSuspendDevice(GetTickCount());
    POWER_HILOGI(MODULE_INNERKIT, " Calling ForceSuspendDevice Success!");
    return ret;
}

bool PowerMgrClient::IsScreenOn()
{
    RETURN_IF_WITH_RET(Connect() != ERR_OK, false);
    bool ret = false;
    ret = proxy_->IsScreenOn();
    POWER_HILOGI(MODULE_INNERKIT, " Calling IsScreenOn Success!");
    return ret;
}

std::shared_ptr<RunningLock> PowerMgrClient::CreateRunningLock(const std::string& name, RunningLockType type,
                                                               const int screenOnFlag)
{
    auto uid = IPCSkeleton::GetCallingUid();
    if (uid >= APP_FIRST_UID && !Permission::CheckSelfPermission("ohos.permission.RUNNING_LOCK")) {
        POWER_HILOGE(MODULE_SERVICE, "%{public}s Request failed, %{public}d permission check failed", __func__, uid);
        return nullptr;
    }

    RETURN_IF_WITH_RET(Connect() != ERR_OK, nullptr);
    if (screenOnFlag == RunningLock::CREATE_WITH_SCREEN_ON) {
        if (type != RunningLockType::RUNNINGLOCK_SCREEN) {
            POWER_HILOGE(MODULE_INNERKIT, "%{public}s failed to create RunningLock due to screenOnFlag error",
                __func__);
            return nullptr;
        } else {
            WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_APPLICATION, "RunningLockWakeUpScreen");
        }
    }

    int nameLen = (name.size() > RunningLock::MAX_NAME_LEN) ? RunningLock::MAX_NAME_LEN : name.size();
    std::shared_ptr<RunningLock> runningLock = std::make_shared<RunningLock>(proxy_, name.substr(0, nameLen), type);
    if (runningLock == nullptr) {
        POWER_HILOGE(MODULE_INNERKIT, "%{public}s failed to create new RunningLock record", __func__);
        return nullptr;
    }
    if (!runningLock->Init()) {
        POWER_HILOGE(MODULE_INNERKIT, "%{public}s runningLock->Init failed.", __func__);
        return nullptr;
    }
    POWER_HILOGI(MODULE_INNERKIT, "%{public}s :name %{public}s, type = %d", __func__, name.c_str(), type);
    return runningLock;
}

void PowerMgrClient::ProxyRunningLock(bool proxyLock, pid_t uid, pid_t pid)
{
    RETURN_IF(Connect() != ERR_OK);
    POWER_HILOGI(MODULE_INNERKIT, "%{public}s :proxyLock = %d, uid = %d, pid = %d", __func__,
        proxyLock, uid, pid);
    proxy_->ProxyRunningLock(proxyLock, uid, pid);
}

void PowerMgrClient::RegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback)
{
    RETURN_IF((callback == nullptr) || (Connect() != ERR_OK));
    POWER_HILOGI(MODULE_INNERKIT, "%{public}s.", __func__);
    proxy_->RegisterPowerStateCallback(callback);
}

void PowerMgrClient::UnRegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback)
{
    RETURN_IF((callback == nullptr) || (Connect() != ERR_OK));
    POWER_HILOGI(MODULE_INNERKIT, "%{public}s.", __func__);
    proxy_->UnRegisterPowerStateCallback(callback);
}

void PowerMgrClient::RegisterShutdownCallback(const sptr<IShutdownCallback>& callback)
{
    RETURN_IF((callback == nullptr) || (Connect() != ERR_OK));
    proxy_->RegisterShutdownCallback(callback);
}

void PowerMgrClient::UnRegisterShutdownCallback(const sptr<IShutdownCallback>& callback)
{
    RETURN_IF((callback == nullptr) || (Connect() != ERR_OK));
    proxy_->UnRegisterShutdownCallback(callback);
}
} // namespace PowerMgr
}  // namespace OHOS
