/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include <system_ability_definition.h>

#include "power_common.h"

namespace OHOS {
namespace PowerMgr {
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
        POWER_HILOGE(COMP_FWK, "Failed to obtain SystemAbilityMgr");
        return E_GET_SYSTEM_ABILITY_MANAGER_FAILED;
    }
    sptr<IRemoteObject> remoteObject_ = sam->CheckSystemAbility(POWER_MANAGER_SERVICE_ID);
    if (remoteObject_ == nullptr) {
        POWER_HILOGE(COMP_FWK, "Check SystemAbility failed");
        return E_GET_POWER_SERVICE_FAILED;
    }

    deathRecipient_ = sptr<IRemoteObject::DeathRecipient>(new PowerMgrDeathRecipient());
    if (deathRecipient_ == nullptr) {
        POWER_HILOGE(COMP_FWK, "Failed to create PowerMgrDeathRecipient");
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
        POWER_HILOGE(COMP_FWK, "OnRemoteDied failed, remote is nullptr");
        return;
    }

    PowerMgrClient::GetInstance().ResetProxy(remote);
    POWER_HILOGW(COMP_FWK, "Recv death notice");
}

void PowerMgrClient::RebootDevice(const std::string& reason)
{
    RETURN_IF(Connect() != ERR_OK);
    proxy_->RebootDevice(reason);
}

void PowerMgrClient::ShutDownDevice(const std::string& reason)
{
    RETURN_IF(Connect() != ERR_OK);
    proxy_->ShutDownDevice(reason);
}

void PowerMgrClient::SuspendDevice(SuspendDeviceType reason, bool suspendImmed)
{
    RETURN_IF(Connect() != ERR_OK);
    proxy_->SuspendDevice(GetTickCount(), reason, suspendImmed);
    POWER_HILOGD(FEATURE_SUSPEND, " Calling SuspendDevice success");
}

void PowerMgrClient::WakeupDevice(WakeupDeviceType reason, const std::string& detail)
{
    RETURN_IF(Connect() != ERR_OK);
    proxy_->WakeupDevice(GetTickCount(), reason, detail);
    POWER_HILOGD(FEATURE_WAKEUP, " Calling WakeupDevice success");
}

void PowerMgrClient::RefreshActivity(UserActivityType type)
{
    RETURN_IF_WITH_LOG(type == UserActivityType::USER_ACTIVITY_TYPE_ATTENTION, "UserActivityType does not support");
    RETURN_IF(Connect() != ERR_OK);
    proxy_->RefreshActivity(GetTickCount(), type, true);
    POWER_HILOGD(FEATURE_ACTIVITY, "Calling RefreshActivity Success");
}

bool PowerMgrClient::IsRunningLockTypeSupported(uint32_t type)
{
    if (type >= static_cast<uint32_t>(RunningLockType::RUNNINGLOCK_BUTT)) {
        POWER_HILOGW(FEATURE_RUNNING_LOCK, "RunningLockType does not support, type: %{public}d", type);
        return false;
    }

    RETURN_IF_WITH_RET(Connect() != ERR_OK, false);
    return proxy_->IsRunningLockTypeSupported(type);
}

bool PowerMgrClient::ForceSuspendDevice()
{
    RETURN_IF_WITH_RET(Connect() != ERR_OK, false);
    bool ret = proxy_->ForceSuspendDevice(GetTickCount());
    POWER_HILOGD(FEATURE_SUSPEND, "Calling ForceSuspendDevice Success");
    return ret;
}

bool PowerMgrClient::IsScreenOn()
{
    RETURN_IF_WITH_RET(Connect() != ERR_OK, false);
    bool ret = false;
    ret = proxy_->IsScreenOn();
    POWER_HILOGD(COMP_FWK, "Calling IsScreenOn Success");
    return ret;
}

PowerState PowerMgrClient::GetState()
{
    RETURN_IF_WITH_RET(Connect() != ERR_OK, PowerState::UNKNOWN);
    return proxy_->GetState();
}

std::shared_ptr<RunningLock> PowerMgrClient::CreateRunningLock(const std::string& name, RunningLockType type)
{
    RETURN_IF_WITH_RET(Connect() != ERR_OK, nullptr);

    uint32_t nameLen = (name.size() > RunningLock::MAX_NAME_LEN) ? RunningLock::MAX_NAME_LEN : name.size();
    std::shared_ptr<RunningLock> runningLock = std::make_shared<RunningLock>(proxy_, name.substr(0, nameLen), type);
    if (runningLock == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Failed to create RunningLock record");
        return nullptr;
    }
    if (!runningLock->Init()) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "RunningLock init failed");
        return nullptr;
    }
    POWER_HILOGI(FEATURE_RUNNING_LOCK, "name: %{public}s, type = %{public}d", name.c_str(), type);
    return runningLock;
}

void PowerMgrClient::RegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback)
{
    RETURN_IF((callback == nullptr) || (Connect() != ERR_OK));
    proxy_->RegisterPowerStateCallback(callback);
}

void PowerMgrClient::UnRegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback)
{
    RETURN_IF((callback == nullptr) || (Connect() != ERR_OK));
    proxy_->UnRegisterPowerStateCallback(callback);
}

void PowerMgrClient::RegisterShutdownCallback(const sptr<IShutdownCallback>& callback,
    IShutdownCallback::ShutdownPriority priority)
{
    RETURN_IF((callback == nullptr) || (Connect() != ERR_OK));
    proxy_->RegisterShutdownCallback(priority, callback);
}

void PowerMgrClient::UnRegisterShutdownCallback(const sptr<IShutdownCallback>& callback)
{
    RETURN_IF((callback == nullptr) || (Connect() != ERR_OK));
    proxy_->UnRegisterShutdownCallback(callback);
}

void PowerMgrClient::RegisterPowerModeCallback(const sptr<IPowerModeCallback>& callback)
{
    RETURN_IF((callback == nullptr) || (Connect() != ERR_OK));
    proxy_->RegisterPowerModeCallback(callback);
}

void PowerMgrClient::UnRegisterPowerModeCallback(const sptr<IPowerModeCallback>& callback)
{
    RETURN_IF((callback == nullptr) || (Connect() != ERR_OK));
    proxy_->UnRegisterPowerModeCallback(callback);
}

void PowerMgrClient::SetDisplaySuspend(bool enable)
{
    RETURN_IF(Connect() != ERR_OK);
    proxy_->SetDisplaySuspend(enable);
}

void PowerMgrClient::SetDeviceMode(const uint32_t mode)
{
    RETURN_IF(Connect() != ERR_OK);
    proxy_->SetDeviceMode(mode);
}

uint32_t PowerMgrClient::GetDeviceMode()
{
    RETURN_IF_WITH_RET(Connect() != ERR_OK, 0);
    return proxy_->GetDeviceMode();
}

std::string PowerMgrClient::Dump(const std::vector<std::string>& args)
{
    std::string error = "can't connect service";
    RETURN_IF_WITH_RET(Connect() != ERR_OK, error);
    return proxy_->ShellDump(args, args.size());
}
} // namespace PowerMgr
}  // namespace OHOS
