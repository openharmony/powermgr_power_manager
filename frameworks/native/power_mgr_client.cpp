/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include <cinttypes>
#include <mutex>
#include <memory>
#include <unistd.h>
#include <vector>
#include <datetime_ex.h>
#include <thread>
#include <chrono>
#include <if_system_ability_manager.h>
#include <iservice_registry.h>
#include <system_ability_definition.h>
#include "new"
#include "refbase.h"
#include "ipower_mgr.h"
#include "ipower_mode_callback.h"
#include "ipower_state_callback.h"
#include "ipower_runninglock_callback.h"
#include "iremote_broker.h"
#include "iremote_object.h"
#include "iscreen_off_pre_callback.h"
#include "power_log.h"
#include "power_common.h"
#include "running_lock_info.h"

namespace OHOS {
namespace PowerMgr {
std::vector<std::weak_ptr<RunningLock>> PowerMgrClient::runningLocks_;
std::mutex PowerMgrClient::runningLocksMutex_;
std::mutex g_instanceMutex;

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

PowerMgrClient& PowerMgrClient::GetInstance()
{
    static PowerMgrClient* instance = nullptr;
    if (instance == nullptr) {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        if (instance == nullptr) {
            instance = new PowerMgrClient();
        }
    }
    return *instance;
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

    sptr<IRemoteObject::DeathRecipient> drt = new(std::nothrow) PowerMgrDeathRecipient(*this);
    if (drt == nullptr) {
        POWER_HILOGE(COMP_FWK, "Failed to create PowerMgrDeathRecipient");
        return ERR_NO_MEMORY;
    }
    if ((remoteObject_->IsProxyObject()) && (!remoteObject_->AddDeathRecipient(drt))) {
        POWER_HILOGE(COMP_FWK, "Add death recipient to PowerMgr service failed");
        return E_ADD_DEATH_RECIPIENT_FAILED;
    }

    proxy_ = iface_cast<IPowerMgr>(remoteObject_);
    deathRecipient_ = drt;
    POWER_HILOGI(COMP_FWK, "Connecting PowerMgrService success");
    return ERR_OK;
}

void PowerMgrClient::PowerMgrDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    POWER_HILOGW(COMP_FWK, "Recv death notice, PowerMgr Death");
    client_.ResetProxy(remote);

    // wait for powermgr service restart
    ErrCode ret = E_GET_POWER_SERVICE_FAILED;
    uint32_t retryCount = 0;
    while (++retryCount <= CONNECT_RETRY_COUNT) {
        usleep(CONNECT_RETRY_MS);
        ret = client_.Connect();
        if (ret == ERR_OK) {
            POWER_HILOGI(COMP_FWK, "retry connect success, count %{public}d", retryCount);
            break;
        }
        POWER_HILOGI(COMP_FWK, "retry connect failed, count %{public}d", retryCount);
    }
    if (ret != ERR_OK) {
        return;
    }

    // recover running lock info
    client_.RecoverRunningLocks();
}

void PowerMgrClient::RecoverRunningLocks()
{
    POWER_HILOGI(COMP_FWK, "start to recover running locks");
    std::lock_guard<std::mutex> lock(runningLocksMutex_);
    for (auto runningLock : runningLocks_) {
        if (runningLock.expired()) {
            continue;
        }
        std::shared_ptr<RunningLock> lock = runningLock.lock();
        if (lock != nullptr) {
            lock->Recover(proxy_);
        }
    }
}

void PowerMgrClient::ResetProxy(const wptr<IRemoteObject>& remote)
{
    if (remote == nullptr) {
        POWER_HILOGE(COMP_FWK, "OnRemoteDied failed, remote is nullptr");
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    RETURN_IF(proxy_ == nullptr);

    auto serviceRemote = proxy_->AsObject();
    if ((serviceRemote != nullptr) && (serviceRemote == remote.promote())) {
        serviceRemote->RemoveDeathRecipient(deathRecipient_);
        proxy_ = nullptr;
    }
}

PowerErrors PowerMgrClient::RebootDevice(const std::string& reason)
{
    RETURN_IF_WITH_RET(Connect() != ERR_OK, PowerErrors::ERR_CONNECTION_FAIL);
    return proxy_->RebootDevice(reason);
}

PowerErrors PowerMgrClient::RebootDeviceForDeprecated(const std::string& reason)
{
    RETURN_IF_WITH_RET(Connect() != ERR_OK, PowerErrors::ERR_CONNECTION_FAIL);
    return proxy_->RebootDeviceForDeprecated(reason);
}

PowerErrors PowerMgrClient::ShutDownDevice(const std::string& reason)
{
    RETURN_IF_WITH_RET(Connect() != ERR_OK, PowerErrors::ERR_CONNECTION_FAIL);
    return proxy_->ShutDownDevice(reason);
}

PowerErrors PowerMgrClient::SuspendDevice(SuspendDeviceType reason, bool suspendImmed)
{
    RETURN_IF_WITH_RET(Connect() != ERR_OK, PowerErrors::ERR_CONNECTION_FAIL);
    POWER_HILOGD(FEATURE_SUSPEND, " Calling SuspendDevice success");
    return proxy_->SuspendDevice(GetTickCount(), reason, suspendImmed);
}

PowerErrors PowerMgrClient::WakeupDevice(WakeupDeviceType reason, const std::string& detail)
{
    RETURN_IF_WITH_RET(Connect() != ERR_OK, PowerErrors::ERR_CONNECTION_FAIL);
    POWER_HILOGD(FEATURE_WAKEUP, " Calling WakeupDevice success");
    return proxy_->WakeupDevice(GetTickCount(), reason, detail);
}

bool PowerMgrClient::RefreshActivity(UserActivityType type)
{
    RETURN_IF_WITH_RET(Connect() != ERR_OK, false);
    bool ret = proxy_->RefreshActivity(GetTickCount(), type, true);
    POWER_HILOGD(FEATURE_ACTIVITY, "Calling RefreshActivity Success");
    return ret;
}

bool PowerMgrClient::OverrideScreenOffTime(int64_t timeout)
{
    if (timeout <= 0) {
        POWER_HILOGW(COMP_FWK, "Invalid timeout, timeout=%{public}" PRId64 "", timeout);
        return false;
    }
    RETURN_IF_WITH_RET(Connect() != ERR_OK, false);
    bool ret = proxy_->OverrideScreenOffTime(timeout);
    POWER_HILOGD(COMP_FWK, "Calling OverrideScreenOffTime Success");
    return ret;
}

bool PowerMgrClient::RestoreScreenOffTime()
{
    RETURN_IF_WITH_RET(Connect() != ERR_OK, false);
    bool ret = proxy_->RestoreScreenOffTime();
    POWER_HILOGD(COMP_FWK, "Calling RestoreScreenOffTime Success");
    return ret;
}

bool PowerMgrClient::IsRunningLockTypeSupported(RunningLockType type)
{
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "RunningLockType=%{public}u", type);
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
    POWER_HILOGI(
        FEATURE_RUNNING_LOCK, "Client CreateRunningLock name: %{public}s, type = %{public}d", name.c_str(), type);
    auto error = runningLock->Init();
    if (error != PowerErrors::ERR_OK) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "RunningLock init failed");
        error_ = error;
        return nullptr;
    }

    std::lock_guard<std::mutex> lock(runningLocksMutex_);
    runningLocks_.push_back(std::weak_ptr<RunningLock>(runningLock));
    return runningLock;
}

bool PowerMgrClient::ProxyRunningLock(bool isProxied, pid_t pid, pid_t uid)
{
    RETURN_IF_WITH_RET(Connect() != ERR_OK, false);
    return proxy_->ProxyRunningLock(isProxied, pid, uid);
}

bool PowerMgrClient::ProxyRunningLocks(bool isProxied, const std::vector<std::pair<pid_t, pid_t>>& processInfos)
{
    RETURN_IF_WITH_RET(Connect() != ERR_OK, false);
    return proxy_->ProxyRunningLocks(isProxied, processInfos);
}

bool PowerMgrClient::ResetRunningLocks()
{
    RETURN_IF_WITH_RET(Connect() != ERR_OK, false);
    return proxy_->ResetRunningLocks();
}

bool PowerMgrClient::RegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback)
{
    RETURN_IF_WITH_RET((callback == nullptr) || (Connect() != ERR_OK), false);
    bool ret = proxy_->RegisterPowerStateCallback(callback);
    return ret;
}

bool PowerMgrClient::UnRegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback)
{
    RETURN_IF_WITH_RET((callback == nullptr) || (Connect() != ERR_OK), false);
    bool ret = proxy_->UnRegisterPowerStateCallback(callback);
    return ret;
}

bool PowerMgrClient::RegisterSyncSleepCallback(const sptr<ISyncSleepCallback>& callback, SleepPriority priority)
{
    RETURN_IF_WITH_RET((callback == nullptr) || (Connect() != ERR_OK), false);
    bool ret = proxy_->RegisterSyncSleepCallback(callback, priority);
    return ret;
}

bool PowerMgrClient::UnRegisterSyncSleepCallback(const sptr<ISyncSleepCallback>& callback)
{
    RETURN_IF_WITH_RET((callback == nullptr) || (Connect() != ERR_OK), false);
    bool ret = proxy_->UnRegisterSyncSleepCallback(callback);
    return ret;
}

bool PowerMgrClient::RegisterPowerModeCallback(const sptr<IPowerModeCallback>& callback)
{
    RETURN_IF_WITH_RET((callback == nullptr) || (Connect() != ERR_OK), false);
    bool ret = proxy_->RegisterPowerModeCallback(callback);
    return ret;
}

bool PowerMgrClient::UnRegisterPowerModeCallback(const sptr<IPowerModeCallback>& callback)
{
    RETURN_IF_WITH_RET((callback == nullptr) || (Connect() != ERR_OK), false);
    bool ret = proxy_->UnRegisterPowerModeCallback(callback);
    return ret;
}

bool PowerMgrClient::RegisterRunningLockCallback(const sptr<IPowerRunninglockCallback>& callback)
{
    RETURN_IF_WITH_RET((callback == nullptr) || (Connect() != ERR_OK), false);
    POWER_HILOGI(FEATURE_RUNNING_LOCK, "Register running lock Callback by client");
    bool ret = proxy_->RegisterRunningLockCallback(callback);
    return ret;
}

bool PowerMgrClient::UnRegisterRunningLockCallback(const sptr<IPowerRunninglockCallback>& callback)
{
    RETURN_IF_WITH_RET((callback == nullptr) || (Connect() != ERR_OK), false);
    POWER_HILOGI(FEATURE_RUNNING_LOCK, "Unregister running lock Callback by client");
    bool ret = proxy_->UnRegisterRunningLockCallback(callback);
    return ret;
}

bool PowerMgrClient::RegisterScreenStateCallback(int32_t remainTime, const sptr<IScreenOffPreCallback>& callback)
{
    RETURN_IF_WITH_RET((remainTime <= 0) || (callback == nullptr) || (Connect() != ERR_OK), false);
    POWER_HILOGI(FEATURE_SCREEN_OFF_PRE, "Register screen off pre Callback by client");
    bool ret = proxy_->RegisterScreenStateCallback(remainTime, callback);
    return ret;
}

bool PowerMgrClient::UnRegisterScreenStateCallback(const sptr<IScreenOffPreCallback>& callback)
{
    RETURN_IF_WITH_RET((callback == nullptr) || (Connect() != ERR_OK), false);
    POWER_HILOGI(FEATURE_SCREEN_OFF_PRE, "Unregister screen off pre Callback by client");
    bool ret = proxy_->UnRegisterScreenStateCallback(callback);
    return ret;
}

bool PowerMgrClient::SetDisplaySuspend(bool enable)
{
    RETURN_IF_WITH_RET(Connect() != ERR_OK, false);
    bool ret = proxy_->SetDisplaySuspend(enable);
    return ret;
}

bool PowerMgrClient::Hibernate(bool clearMemory)
{
    RETURN_IF_WITH_RET(Connect() != ERR_OK, false);
    bool ret = proxy_->Hibernate(clearMemory);
    return ret;
}

PowerErrors PowerMgrClient::SetDeviceMode(const PowerMode mode)
{
    RETURN_IF_WITH_RET(Connect() != ERR_OK, PowerErrors::ERR_CONNECTION_FAIL);
    return proxy_->SetDeviceMode(mode);
}

PowerMode PowerMgrClient::GetDeviceMode()
{
    RETURN_IF_WITH_RET(Connect() != ERR_OK, static_cast<PowerMode>(0));
    return static_cast<PowerMode>(proxy_->GetDeviceMode());
}

std::string PowerMgrClient::Dump(const std::vector<std::string>& args)
{
    std::string error = "can't connect service";
    RETURN_IF_WITH_RET(Connect() != ERR_OK, error);
    return proxy_->ShellDump(args, args.size());
}

PowerErrors PowerMgrClient::GetError()
{
    auto temp = error_;
    error_ = PowerErrors::ERR_OK;
    return temp;
}

PowerErrors PowerMgrClient::IsStandby(bool& isStandby)
{
    RETURN_IF_WITH_RET(Connect() != ERR_OK, PowerErrors::ERR_CONNECTION_FAIL);
    PowerErrors ret = proxy_->IsStandby(isStandby);
    return ret;
}

bool PowerMgrClient::QueryRunningLockLists(std::map<std::string, RunningLockInfo>& runningLockLists)
{
    RETURN_IF_WITH_RET(Connect() != ERR_OK, false);
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "Query running lock lists by client");
    return proxy_->QueryRunningLockLists(runningLockLists);
}

PowerErrors PowerMgrClient::SetForceTimingOut(bool enabled)
{
    RETURN_IF_WITH_RET(Connect() != ERR_OK, PowerErrors::ERR_CONNECTION_FAIL);
    PowerErrors ret = proxy_->SetForceTimingOut(enabled);
    return ret;
}

PowerErrors PowerMgrClient::LockScreenAfterTimingOut(bool enabledLockScreen, bool checkLock)
{
    RETURN_IF_WITH_RET(Connect() != ERR_OK, PowerErrors::ERR_CONNECTION_FAIL);
    PowerErrors ret = proxy_->LockScreenAfterTimingOut(enabledLockScreen, checkLock);
    return ret;
}

} // namespace PowerMgr
} // namespace OHOS
