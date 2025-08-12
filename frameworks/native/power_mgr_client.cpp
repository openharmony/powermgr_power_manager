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
#include "power_mgr_async_reply_stub.h"

namespace OHOS {
namespace PowerMgr {
std::vector<std::weak_ptr<RunningLock>> PowerMgrClient::runningLocks_;
std::mutex PowerMgrClient::runningLocksMutex_;
std::mutex g_instanceMutex;
constexpr int32_t MAX_VERSION_STRING_SIZE = 4;
constexpr uint32_t PARAM_MAX_NUM = 10;

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
    int32_t powerError = static_cast<int32_t>(PowerErrors::ERR_CONNECTION_FAIL);
    proxy->RebootDeviceIpc(reason, powerError);
    return static_cast<PowerErrors>(powerError);
}

PowerErrors PowerMgrClient::RebootDeviceForDeprecated(const std::string& reason)
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, PowerErrors::ERR_CONNECTION_FAIL);
    int32_t powerError = static_cast<int32_t>(PowerErrors::ERR_CONNECTION_FAIL);
    proxy->RebootDeviceForDeprecatedIpc(reason, powerError);
    return static_cast<PowerErrors>(powerError);
}

PowerErrors PowerMgrClient::ShutDownDevice(const std::string& reason)
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, PowerErrors::ERR_CONNECTION_FAIL);
    int32_t powerError = static_cast<int32_t>(PowerErrors::ERR_CONNECTION_FAIL);
    proxy->ShutDownDeviceIpc(reason, powerError);
    return static_cast<PowerErrors>(powerError);
}

PowerErrors PowerMgrClient::SetSuspendTag(const std::string &tag)
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, PowerErrors::ERR_CONNECTION_FAIL);
    POWER_HILOGI(FEATURE_SUSPEND, "Set suspend tag: %{public}s", tag.c_str());
    int32_t powerError = static_cast<int32_t>(PowerErrors::ERR_CONNECTION_FAIL);
    proxy->SetSuspendTagIpc(tag, powerError);
    return static_cast<PowerErrors>(powerError);
}

PowerErrors PowerMgrClient::SuspendDevice(
    SuspendDeviceType reason, bool suspendImmed, const std::string& apiVersion)
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, PowerErrors::ERR_CONNECTION_FAIL);
    POWER_HILOGD(FEATURE_SUSPEND, " Calling SuspendDevice success");
    int32_t powerError = static_cast<int32_t>(PowerErrors::ERR_CONNECTION_FAIL);
    int32_t reasonValue = static_cast<int32_t>(reason);
    RETURN_IF_WITH_RET(apiVersion.size() >= MAX_VERSION_STRING_SIZE, PowerErrors::ERR_PARAM_INVALID);
    proxy->SuspendDeviceIpc(GetTickCount(), reasonValue, suspendImmed, apiVersion, powerError);
    return static_cast<PowerErrors>(powerError);
}

PowerErrors PowerMgrClient::WakeupDevice(
    WakeupDeviceType reason, const std::string& detail, const std::string& apiVersion)
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, PowerErrors::ERR_CONNECTION_FAIL);
    POWER_HILOGD(FEATURE_WAKEUP, " Calling WakeupDevice success");
    int32_t powerError = static_cast<int32_t>(PowerErrors::ERR_CONNECTION_FAIL);
    int32_t reasonValue = static_cast<int32_t>(reason);
    RETURN_IF_WITH_RET(apiVersion.size() >= MAX_VERSION_STRING_SIZE, PowerErrors::ERR_PARAM_INVALID);
    proxy->WakeupDeviceIpc(GetTickCount(), reasonValue, detail, apiVersion, powerError);
    return static_cast<PowerErrors>(powerError);
}

void PowerMgrClient::WakeupDeviceAsync(WakeupDeviceType reason, const std::string& detail)
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF(proxy == nullptr);
    POWER_HILOGD(FEATURE_WAKEUP, " Calling WakeupDeviceAsync success");
    int32_t reasonValue = static_cast<int32_t>(reason);
    std::string apiVersion;
    proxy->WakeupDeviceAsyncIpc(GetTickCount(), reasonValue, detail, apiVersion);
    return;
}

bool PowerMgrClient::RefreshActivity(UserActivityType type)
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, false);
    bool ret = false;
    int32_t activityType = static_cast<int32_t>(type);
    int32_t result = proxy->RefreshActivityIpc(GetTickCount(), activityType, true);
    if (result == ERR_OK) {
        ret = true;
    }
    POWER_HILOGD(FEATURE_ACTIVITY, "Calling RefreshActivity Success");
    return ret;
}

PowerErrors PowerMgrClient::OverrideScreenOffTime(int64_t timeout, const std::string& apiVersion)
{
    if (timeout <= 0) {
        POWER_HILOGW(COMP_FWK, "Invalid timeout, timeout=%{public}" PRId64 "", timeout);
        return PowerErrors::ERR_PARAM_INVALID;
    }
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, PowerErrors::ERR_CONNECTION_FAIL);
    int32_t powerError = static_cast<int32_t>(PowerErrors::ERR_CONNECTION_FAIL);
    RETURN_IF_WITH_RET(apiVersion.size() >= MAX_VERSION_STRING_SIZE, PowerErrors::ERR_PARAM_INVALID);
    proxy->OverrideScreenOffTimeIpc(timeout, apiVersion, powerError);
    PowerErrors ret = static_cast<PowerErrors>(powerError);
    POWER_HILOGD(COMP_FWK, "Calling OverrideScreenOffTime Success");
    return ret;
}

PowerErrors PowerMgrClient::RestoreScreenOffTime(const std::string& apiVersion)
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, PowerErrors::ERR_CONNECTION_FAIL);
    int32_t powerError = static_cast<int32_t>(PowerErrors::ERR_CONNECTION_FAIL);
    RETURN_IF_WITH_RET(apiVersion.size() >= MAX_VERSION_STRING_SIZE, PowerErrors::ERR_PARAM_INVALID);
    proxy->RestoreScreenOffTimeIpc(apiVersion, powerError);
    PowerErrors ret = static_cast<PowerErrors>(powerError);
    POWER_HILOGD(COMP_FWK, "Calling RestoreScreenOffTime Success");
    return ret;
}

bool PowerMgrClient::IsRunningLockTypeSupported(RunningLockType type)
{
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "RunningLockType=%{public}u", type);
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, false);
    bool lockSupported = false;
    int32_t lockType = static_cast<int32_t>(type);
    proxy->IsRunningLockTypeSupportedIpc(lockType, lockSupported);
    return lockSupported;
}

PowerErrors PowerMgrClient::ForceSuspendDevice(const std::string& apiVersion)
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, PowerErrors::ERR_CONNECTION_FAIL);
    sptr<PowerMgrStubAsync> asyncCallback = new PowerMgrStubAsync();
    sptr<IPowerMgrAsync> powerProxy = iface_cast<IPowerMgrAsync>(asyncCallback);
    RETURN_IF_WITH_RET(apiVersion.size() >= MAX_VERSION_STRING_SIZE, PowerErrors::ERR_PARAM_INVALID);
    int32_t result = proxy->ForceSuspendDeviceIpc(GetTickCount(), apiVersion, powerProxy);
    RETURN_IF_WITH_RET(result != ERR_OK, PowerErrors::ERR_CONNECTION_FAIL);
    // Wait for the asynchronous callback to return, with a timeout of 100 milliseconds
    PowerErrors ret = static_cast<PowerErrors>(asyncCallback->WaitForAsyncReply(100));
    POWER_HILOGD(FEATURE_SUSPEND, "Calling ForceSuspendDevice Success");
    return ret;
}

bool PowerMgrClient::IsScreenOn(bool needPrintLog)
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, false);
    bool ret = false;
    proxy->IsScreenOnIpc(needPrintLog, ret);
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
    proxy->IsFoldScreenOnIpc(ret);
    POWER_HILOGI(COMP_FWK, "IsFoldScreenOn=%{public}d, caller pid=%{public}d", ret, getpid());
    return ret;
}

bool PowerMgrClient::IsCollaborationScreenOn()
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, false);
    bool ret = false;
    proxy->IsCollaborationScreenOnIpc(ret);
    POWER_HILOGI(COMP_FWK, "IsCollaborationScreenOn=%{public}d, caller pid=%{public}d", ret, getpid());
    return ret;
}

bool PowerMgrClient::IsForceSleeping()
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, false);
    bool ret = false;
    proxy->IsForceSleepingIpc(ret);
    POWER_HILOGD(COMP_FWK, "IsForceSleeping=%{public}d, caller pid=%{public}d", ret, getpid());
    return ret;
}

PowerState PowerMgrClient::GetState()
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, PowerState::UNKNOWN);
    int32_t powerState = static_cast<int32_t>(PowerErrors::ERR_FAILURE);
    proxy->GetStateIpc(powerState);
    return static_cast<PowerState>(powerState);
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
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, false);
    bool result = false;
    int32_t ret = proxy->ProxyRunningLockIpc(isProxied, pid, uid);
    if (ret == ERR_OK) {
        result = true;
    }
    return result;
}

bool PowerMgrClient::ProxyRunningLocks(bool isProxied, const std::vector<std::pair<pid_t, pid_t>>& processInfos)
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, false);
    VectorPair vectorPairInfos;
    vectorPairInfos.SetProcessInfos(processInfos);
    int32_t ret = proxy->ProxyRunningLocksIpc(isProxied, vectorPairInfos);
    return ret == ERR_OK;
}

bool PowerMgrClient::ResetRunningLocks()
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, false);
    int32_t ret = proxy->ResetRunningLocksIpc();
    return ret == ERR_OK;
}

bool PowerMgrClient::RegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback, bool isSync)
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET((callback == nullptr) || (proxy == nullptr), false);
    int32_t ret = proxy->RegisterPowerStateCallbackIpc(callback, isSync);
    return ret == ERR_OK;
}

bool PowerMgrClient::UnRegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback)
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET((callback == nullptr) || (proxy == nullptr), false);
    int32_t ret = proxy->UnRegisterPowerStateCallbackIpc(callback);
    return ret == ERR_OK;
}

bool PowerMgrClient::RegisterSyncSleepCallback(const sptr<ISyncSleepCallback>& callback, SleepPriority priority)
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET((callback == nullptr) || (proxy == nullptr), false);
    int32_t priorityValue = static_cast<int32_t>(priority);
    int32_t ret = proxy->RegisterSyncSleepCallbackIpc(callback, priorityValue);
    return ret == ERR_OK;
}

bool PowerMgrClient::UnRegisterSyncSleepCallback(const sptr<ISyncSleepCallback>& callback)
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET((callback == nullptr) || (proxy == nullptr), false);
    int32_t ret = proxy->UnRegisterSyncSleepCallbackIpc(callback);
    return ret == ERR_OK;
}

bool PowerMgrClient::RegisterSuspendTakeoverCallback(const sptr<ITakeOverSuspendCallback>& callback,
                                                     TakeOverSuspendPriority priority)
{
    POWER_HILOGI(FEATURE_SUSPEND, "%{public}s start", __func__);
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    if ((callback == nullptr) || (proxy == nullptr)) {
        POWER_HILOGE(FEATURE_SUSPEND, "%{public}s callback or proxy is nullptr", __func__);
        return false;
    }
    int32_t ret = proxy->RegisterSuspendTakeoverCallbackIpc(callback, static_cast<int>(priority));
    return ret == ERR_OK;
}

bool PowerMgrClient::UnRegisterSuspendTakeoverCallback(const sptr<ITakeOverSuspendCallback>& callback)
{
    POWER_HILOGI(FEATURE_SUSPEND, "%{public}s start", __func__);
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    if ((callback == nullptr) || (proxy == nullptr)) {
        POWER_HILOGE(FEATURE_SUSPEND, "%{public}s callback or proxy is nullptr", __func__);
        return false;
    }
    int32_t ret = proxy->UnRegisterSuspendTakeoverCallbackIpc(callback);
    return ret == ERR_OK;
}

bool PowerMgrClient::RegisterSyncHibernateCallback(const sptr<ISyncHibernateCallback>& callback)
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET((callback == nullptr) || (proxy == nullptr), false);
    int32_t ret = proxy->RegisterSyncHibernateCallbackIpc(callback);
    return ret == ERR_OK;
}

bool PowerMgrClient::UnRegisterSyncHibernateCallback(const sptr<ISyncHibernateCallback>& callback)
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET((callback == nullptr) || (proxy == nullptr), false);
    int32_t ret = proxy->UnRegisterSyncHibernateCallbackIpc(callback);
    return ret == ERR_OK;
}

bool PowerMgrClient::RegisterPowerModeCallback(const sptr<IPowerModeCallback>& callback)
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET((callback == nullptr) || (proxy == nullptr), false);
    int32_t ret = proxy->RegisterPowerModeCallbackIpc(callback);
    return ret == ERR_OK;
}

bool PowerMgrClient::UnRegisterPowerModeCallback(const sptr<IPowerModeCallback>& callback)
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET((callback == nullptr) || (proxy == nullptr), false);
    int32_t ret = proxy->UnRegisterPowerModeCallbackIpc(callback);
    return ret == ERR_OK;
}

bool PowerMgrClient::RegisterScreenStateCallback(int32_t remainTime, const sptr<IScreenOffPreCallback>& callback)
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET((remainTime <= 0) || (callback == nullptr) || (proxy == nullptr), false);
    POWER_HILOGI(FEATURE_SCREEN_OFF_PRE, "Register screen off pre Callback by client");
    int32_t ret = proxy->RegisterScreenStateCallbackIpc(remainTime, callback);
    return ret == ERR_OK;
}

bool PowerMgrClient::UnRegisterScreenStateCallback(const sptr<IScreenOffPreCallback>& callback)
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET((callback == nullptr) || (proxy == nullptr), false);
    POWER_HILOGI(FEATURE_SCREEN_OFF_PRE, "Unregister screen off pre Callback by client");
    int32_t ret = proxy->UnRegisterScreenStateCallbackIpc(callback);
    return ret == ERR_OK;
}

bool PowerMgrClient::RegisterRunningLockCallback(const sptr<IPowerRunninglockCallback>& callback)
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET((callback == nullptr) || (proxy == nullptr), false);
    POWER_HILOGI(FEATURE_RUNNING_LOCK, "Register running lock Callback by client");
    int32_t ret = proxy->RegisterRunningLockCallbackIpc(callback);
    return ret == ERR_OK;
}

bool PowerMgrClient::UnRegisterRunningLockCallback(const sptr<IPowerRunninglockCallback>& callback)
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET((callback == nullptr) || (proxy == nullptr), false);
    POWER_HILOGI(FEATURE_RUNNING_LOCK, "Unregister running lock Callback by client");
    int32_t ret = proxy->UnRegisterRunningLockCallbackIpc(callback);
    return ret == ERR_OK;
}

bool PowerMgrClient::SetDisplaySuspend(bool enable)
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, false);
    int32_t ret = proxy->SetDisplaySuspendIpc(enable);
    return ret == ERR_OK;
}

PowerErrors PowerMgrClient::Hibernate(bool clearMemory, const std::string& reason, const std::string& apiVersion)
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, PowerErrors::ERR_CONNECTION_FAIL);
    sptr<PowerMgrStubAsync> asyncCallback = new PowerMgrStubAsync();
    sptr<IPowerMgrAsync> powerProxy = iface_cast<IPowerMgrAsync>(asyncCallback);
    RETURN_IF_WITH_RET(apiVersion.size() >= MAX_VERSION_STRING_SIZE, PowerErrors::ERR_PARAM_INVALID);
    int32_t result = proxy->HibernateIpc(clearMemory, reason, apiVersion, powerProxy);
    RETURN_IF_WITH_RET(result != ERR_OK, PowerErrors::ERR_CONNECTION_FAIL);
    // Wait for the asynchronous callback to return, with a timeout of 100 milliseconds
    PowerErrors ret = static_cast<PowerErrors>(asyncCallback->WaitForAsyncReply(100));
    return ret;
}

PowerErrors PowerMgrClient::SetDeviceMode(const PowerMode mode)
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, PowerErrors::ERR_CONNECTION_FAIL);
    int32_t powerError = static_cast<int32_t>(PowerErrors::ERR_CONNECTION_FAIL);
    int32_t modeValue = static_cast<int32_t>(mode);
    proxy->SetDeviceModeIpc(modeValue, powerError);
    return static_cast<PowerErrors>(powerError);
}

PowerMode PowerMgrClient::GetDeviceMode()
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, static_cast<PowerMode>(0));
    int32_t powerMode = 0;
    proxy->GetDeviceModeIpc(powerMode);
    return static_cast<PowerMode>(powerMode);
}

std::string PowerMgrClient::Dump(const std::vector<std::string>& args)
{
    std::string error = "can't connect service";
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, error);
    std::string returnDump = "remote error";
    uint32_t argc = args.size();
    if (argc >= PARAM_MAX_NUM) {
        POWER_HILOGW(COMP_FWK, "params exceed limit, argc=%{public}u", argc);
        return returnDump;
    }
    proxy->ShellDumpIpc(args, argc, returnDump);
    return returnDump;
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
    int32_t powerError = static_cast<int32_t>(PowerErrors::ERR_CONNECTION_FAIL);
    proxy->IsStandbyIpc(isStandby, powerError);
    return static_cast<PowerErrors>(powerError);
}

bool PowerMgrClient::QueryRunningLockLists(std::map<std::string, RunningLockInfo>& runningLockLists)
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, false);
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "Query running lock lists by client");
    int32_t ret = proxy->QueryRunningLockListsIpc(runningLockLists);
    return ret == ERR_OK;
}

PowerErrors PowerMgrClient::SetForceTimingOut(bool enabled)
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, PowerErrors::ERR_CONNECTION_FAIL);
    int32_t powerError = static_cast<int32_t>(PowerErrors::ERR_CONNECTION_FAIL);
    proxy->SetForceTimingOutIpc(enabled, token_, powerError);
    PowerErrors ret = static_cast<PowerErrors>(powerError);
    return ret;
}

PowerErrors PowerMgrClient::LockScreenAfterTimingOut(bool enabledLockScreen, bool checkLock, bool sendScreenOffEvent)
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, PowerErrors::ERR_CONNECTION_FAIL);
    int32_t powerError = static_cast<int32_t>(PowerErrors::ERR_CONNECTION_FAIL);
    proxy->LockScreenAfterTimingOutIpc(enabledLockScreen, checkLock, sendScreenOffEvent, token_, powerError);
    PowerErrors ret = static_cast<PowerErrors>(powerError);
    return ret;
}

PowerErrors PowerMgrClient::IsRunningLockEnabled(const RunningLockType type, bool& result)
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, PowerErrors::ERR_CONNECTION_FAIL);
    int32_t powerError = static_cast<int32_t>(PowerErrors::ERR_CONNECTION_FAIL);
    int32_t lockType = static_cast<int32_t>(type);
    proxy->IsRunningLockEnabledIpc(lockType, result, powerError);
    return static_cast<PowerErrors>(powerError);
}

PowerErrors PowerMgrClient::RefreshActivity(UserActivityType type, const std::string& refreshReason)
{
    sptr<IPowerMgr> proxy = GetPowerMgrProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, PowerErrors::ERR_CONNECTION_FAIL);
    int32_t activityType = static_cast<int32_t>(type);
    int32_t powerError = static_cast<int32_t>(PowerErrors::ERR_CONNECTION_FAIL);
    proxy->RefreshActivityIpc(GetTickCount(), activityType, refreshReason, powerError);
    return static_cast<PowerErrors>(powerError);
}
} // namespace PowerMgr
} // namespace OHOS
