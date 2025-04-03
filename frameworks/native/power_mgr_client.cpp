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

PowerMgrClient::PowerMgrClient()
{
    token_ = sptr<IPCObjectStub>::MakeSptr(u"ohos.powermgr.ClientAlivenessToken");
}

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
    return ERR_OK;
}

sptr<IPowerMgr> PowerMgrClient::GetPowerMgrProxy()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (proxy_ != nullptr) {
        return proxy_;
    }

    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        POWER_HILOGE(COMP_FWK, "Failed to obtain SystemAbilityMgr");
        return proxy_;
    }
    sptr<IRemoteObject> remoteObject_ = sam->CheckSystemAbility(POWER_MANAGER_SERVICE_ID);
    if (remoteObject_ == nullptr) {
        POWER_HILOGE(COMP_FWK, "Check SystemAbility failed");
        return proxy_;
    }

    sptr<IRemoteObject::DeathRecipient> drt = new(std::nothrow) PowerMgrDeathRecipient(*this);
    if (drt == nullptr) {
        POWER_HILOGE(COMP_FWK, "Failed to create PowerMgrDeathRecipient");
        return proxy_;
    }
    if ((remoteObject_->IsProxyObject()) && (!remoteObject_->AddDeathRecipient(drt))) {
        POWER_HILOGE(COMP_FWK, "Add death recipient to PowerMgr service failed");
        return proxy_;
    }

    proxy_ = iface_cast<IPowerMgr>(remoteObject_);
    deathRecipient_ = drt;
    POWER_HILOGI(COMP_FWK, "Connecting PowerMgrService success, pid=%{public}d", getpid());
    return proxy_;
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
        sptr<IPowerMgr> proxy = client_.GetPowerMgrProxy();
        if (proxy != nullptr) {
            POWER_HILOGI(COMP_FWK, "retry connect success, count %{public}d", retryCount);
            ret = ERR_OK;
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
            sptr<IPowerMgr> proxy = GetPowerMgrProxy();
            RETURN_IF(proxy == nullptr);
            lock->Recover(proxy);
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
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, PowerErrors::ERR_CONNECTION_FAIL);
    return proxy->RebootDevice(reason);
}

PowerErrors PowerMgrClient::RebootDeviceForDeprecated(const std::string& reason)
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, PowerErrors::ERR_CONNECTION_FAIL);
    return proxy->RebootDeviceForDeprecated(reason);
}

PowerErrors PowerMgrClient::ShutDownDevice(const std::string& reason)
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, PowerErrors::ERR_CONNECTION_FAIL);
    return proxy->ShutDownDevice(reason);
}

PowerErrors PowerMgrClient::SetSuspendTag(const std::string &tag)
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, PowerErrors::ERR_CONNECTION_FAIL);
    POWER_HILOGI(FEATURE_SUSPEND, "Set suspend tag: %{public}s", tag.c_str());
    return proxy->SetSuspendTag(tag);
}

PowerErrors PowerMgrClient::SuspendDevice(SuspendDeviceType reason, bool suspendImmed)
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, PowerErrors::ERR_CONNECTION_FAIL);
    POWER_HILOGD(FEATURE_SUSPEND, " Calling SuspendDevice success");
    return proxy->SuspendDevice(GetTickCount(), reason, suspendImmed);
}

PowerErrors PowerMgrClient::WakeupDevice(WakeupDeviceType reason, const std::string& detail)
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, PowerErrors::ERR_CONNECTION_FAIL);
    POWER_HILOGD(FEATURE_WAKEUP, " Calling WakeupDevice success");
    return proxy->WakeupDevice(GetTickCount(), reason, detail);
}

void PowerMgrClient::WakeupDeviceAsync(WakeupDeviceType reason, const std::string& detail)
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF(proxy == nullptr);
    POWER_HILOGD(FEATURE_WAKEUP, " Calling WakeupDeviceAsync success");
    return proxy->WakeupDeviceAsync(GetTickCount(), reason, detail);
}

bool PowerMgrClient::RefreshActivity(UserActivityType type)
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, false);
    bool ret = proxy->RefreshActivity(GetTickCount(), type, true);
    POWER_HILOGD(FEATURE_ACTIVITY, "Calling RefreshActivity Success");
    return ret;
}

PowerErrors PowerMgrClient::OverrideScreenOffTime(int64_t timeout)
{
    if (timeout <= 0) {
        POWER_HILOGW(COMP_FWK, "Invalid timeout, timeout=%{public}" PRId64 "", timeout);
        return PowerErrors::ERR_PARAM_INVALID;
    }
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, PowerErrors::ERR_CONNECTION_FAIL);
    PowerErrors ret = proxy->OverrideScreenOffTime(timeout);
    POWER_HILOGD(COMP_FWK, "Calling OverrideScreenOffTime Success");
    return ret;
}

PowerErrors PowerMgrClient::RestoreScreenOffTime()
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, PowerErrors::ERR_CONNECTION_FAIL);
    PowerErrors ret = proxy->RestoreScreenOffTime();
    POWER_HILOGD(COMP_FWK, "Calling RestoreScreenOffTime Success");
    return ret;
}

bool PowerMgrClient::IsRunningLockTypeSupported(RunningLockType type)
{
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "RunningLockType=%{public}u", type);
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, false);
    return proxy->IsRunningLockTypeSupported(type);
}

PowerErrors PowerMgrClient::ForceSuspendDevice()
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, PowerErrors::ERR_CONNECTION_FAIL);
    PowerErrors ret = proxy->ForceSuspendDevice(GetTickCount());
    POWER_HILOGD(FEATURE_SUSPEND, "Calling ForceSuspendDevice Success");
    return ret;
}

bool PowerMgrClient::IsScreenOn(bool needPrintLog)
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, false);
    bool ret = false;
    ret = proxy->IsScreenOn(needPrintLog);
    if (needPrintLog) {
        POWER_HILOGI(COMP_FWK, "IsScreenOn=%{public}d, caller pid=%{public}d", ret, getpid());
    } else {
        POWER_HILOGD(COMP_FWK, "IsScreenOn=%{public}d, caller pid=%{public}d", ret, getpid());
    }
    return ret;
}

bool PowerMgrClient::IsFoldScreenOn()
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, false);
    bool ret = false;
    ret = proxy->IsFoldScreenOn();
    POWER_HILOGI(COMP_FWK, "IsFoldScreenOn=%{public}d, caller pid=%{public}d", ret, getpid());
    return ret;
}

bool PowerMgrClient::IsCollaborationScreenOn()
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, false);
    bool ret = false;
    ret = proxy->IsCollaborationScreenOn();
    POWER_HILOGI(COMP_FWK, "IsCollaborationScreenOn=%{public}d, caller pid=%{public}d", ret, getpid());
    return ret;
}

PowerState PowerMgrClient::GetState()
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, PowerState::UNKNOWN);
    return proxy->GetState();
}

std::shared_ptr<RunningLock> PowerMgrClient::CreateRunningLock(const std::string& name, RunningLockType type)
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, nullptr);

    uint32_t nameLen = (name.size() > RunningLock::MAX_NAME_LEN) ? RunningLock::MAX_NAME_LEN : name.size();
    std::string nameExt = name.substr(0, nameLen) + "_" + std::to_string(GetTickCount());
    std::shared_ptr<RunningLock> runningLock = std::make_shared<RunningLock>(proxy, nameExt, type);
    if (runningLock == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Failed to create RunningLock record");
        return nullptr;
    }
    // Client CreateRunningLock
    POWER_HILOGI(COMP_LOCK, "CrtN:%{public}s,T=%{public}d", name.c_str(), type);
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
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, false);
    return proxy->ProxyRunningLocks(isProxied, processInfos);
}

bool PowerMgrClient::ResetRunningLocks()
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, false);
    return proxy->ResetRunningLocks();
}

bool PowerMgrClient::RegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback, bool isSync)
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET((callback == nullptr) || (proxy == nullptr), false);
    bool ret = proxy->RegisterPowerStateCallback(callback, isSync);
    return ret;
}

bool PowerMgrClient::UnRegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback)
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET((callback == nullptr) || (proxy == nullptr), false);
    bool ret = proxy->UnRegisterPowerStateCallback(callback);
    return ret;
}

bool PowerMgrClient::RegisterSyncSleepCallback(const sptr<ISyncSleepCallback>& callback, SleepPriority priority)
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET((callback == nullptr) || (proxy == nullptr), false);
    bool ret = proxy->RegisterSyncSleepCallback(callback, priority);
    return ret;
}

bool PowerMgrClient::UnRegisterSyncSleepCallback(const sptr<ISyncSleepCallback>& callback)
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET((callback == nullptr) || (proxy == nullptr), false);
    bool ret = proxy->UnRegisterSyncSleepCallback(callback);
    return ret;
}

bool PowerMgrClient::RegisterSyncHibernateCallback(const sptr<ISyncHibernateCallback>& callback)
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET((callback == nullptr) || (proxy == nullptr), false);
    bool ret = proxy->RegisterSyncHibernateCallback(callback);
    return ret;
}

bool PowerMgrClient::UnRegisterSyncHibernateCallback(const sptr<ISyncHibernateCallback>& callback)
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET((callback == nullptr) || (proxy == nullptr), false);
    bool ret = proxy->UnRegisterSyncHibernateCallback(callback);
    return ret;
}

bool PowerMgrClient::RegisterPowerModeCallback(const sptr<IPowerModeCallback>& callback)
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET((callback == nullptr) || (proxy == nullptr), false);
    bool ret = proxy->RegisterPowerModeCallback(callback);
    return ret;
}

bool PowerMgrClient::UnRegisterPowerModeCallback(const sptr<IPowerModeCallback>& callback)
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET((callback == nullptr) || (proxy == nullptr), false);
    bool ret = proxy->UnRegisterPowerModeCallback(callback);
    return ret;
}

bool PowerMgrClient::RegisterScreenStateCallback(int32_t remainTime, const sptr<IScreenOffPreCallback>& callback)
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET((remainTime <= 0) || (callback == nullptr) || (proxy == nullptr), false);
    POWER_HILOGI(FEATURE_SCREEN_OFF_PRE, "Register screen off pre Callback by client");
    bool ret = proxy->RegisterScreenStateCallback(remainTime, callback);
    return ret;
}

bool PowerMgrClient::UnRegisterScreenStateCallback(const sptr<IScreenOffPreCallback>& callback)
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET((callback == nullptr) || (proxy == nullptr), false);
    POWER_HILOGI(FEATURE_SCREEN_OFF_PRE, "Unregister screen off pre Callback by client");
    bool ret = proxy->UnRegisterScreenStateCallback(callback);
    return ret;
}

bool PowerMgrClient::RegisterRunningLockCallback(const sptr<IPowerRunninglockCallback>& callback)
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET((callback == nullptr) || (proxy == nullptr), false);
    POWER_HILOGI(FEATURE_RUNNING_LOCK, "Register running lock Callback by client");
    bool ret = proxy->RegisterRunningLockCallback(callback);
    return ret;
}

bool PowerMgrClient::UnRegisterRunningLockCallback(const sptr<IPowerRunninglockCallback>& callback)
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET((callback == nullptr) || (proxy == nullptr), false);
    POWER_HILOGI(FEATURE_RUNNING_LOCK, "Unregister running lock Callback by client");
    bool ret = proxy->UnRegisterRunningLockCallback(callback);
    return ret;
}

bool PowerMgrClient::SetDisplaySuspend(bool enable)
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, false);
    bool ret = proxy->SetDisplaySuspend(enable);
    return ret;
}

PowerErrors PowerMgrClient::Hibernate(bool clearMemory)
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, PowerErrors::ERR_CONNECTION_FAIL);
    return proxy->Hibernate(clearMemory);
}

PowerErrors PowerMgrClient::SetDeviceMode(const PowerMode mode)
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, PowerErrors::ERR_CONNECTION_FAIL);
    return proxy->SetDeviceMode(mode);
}

PowerMode PowerMgrClient::GetDeviceMode()
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, static_cast<PowerMode>(0));
    return static_cast<PowerMode>(proxy->GetDeviceMode());
}

std::string PowerMgrClient::Dump(const std::vector<std::string>& args)
{
    std::string error = "can't connect service";
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, error);
    return proxy->ShellDump(args, args.size());
}

PowerErrors PowerMgrClient::GetError()
{
    auto temp = error_;
    error_ = PowerErrors::ERR_OK;
    return temp;
}

PowerErrors PowerMgrClient::IsStandby(bool& isStandby)
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, PowerErrors::ERR_CONNECTION_FAIL);
    PowerErrors ret = proxy->IsStandby(isStandby);
    return ret;
}

bool PowerMgrClient::QueryRunningLockLists(std::map<std::string, RunningLockInfo>& runningLockLists)
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, false);
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "Query running lock lists by client");
    return proxy->QueryRunningLockLists(runningLockLists);
}

PowerErrors PowerMgrClient::SetForceTimingOut(bool enabled)
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, PowerErrors::ERR_CONNECTION_FAIL);
    PowerErrors ret = proxy->SetForceTimingOut(enabled, token_);
    return ret;
}

PowerErrors PowerMgrClient::LockScreenAfterTimingOut(bool enabledLockScreen, bool checkLock, bool sendScreenOffEvent)
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, PowerErrors::ERR_CONNECTION_FAIL);
    PowerErrors ret = proxy->LockScreenAfterTimingOut(enabledLockScreen, checkLock, sendScreenOffEvent, token_);
    return ret;
}

} // namespace PowerMgr
} // namespace OHOS
