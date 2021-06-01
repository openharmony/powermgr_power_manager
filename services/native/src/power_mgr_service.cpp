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

#include "power_mgr_service.h"

#include <datetime_ex.h>
#include <file_ex.h>
#include <if_system_ability_manager.h>
#include <ipc_skeleton.h>
#include <iservice_registry.h>
#include <string_ex.h>
#include <system_ability_definition.h>
#include <unistd.h>

#include "permission.h"
#include "power_common.h"
#include "power_mgr_dumper.h"

namespace OHOS {
namespace PowerMgr {
namespace {
const std::string POWERMGR_SERVICE_NAME = "PowerMgrService";
const std::string TASK_RUNNINGLOCK_UNLOCK = "RunningLock_UnLock";
constexpr int APP_FIRST_UID = 10000;
auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
const bool G_REGISTER_RESULT = SystemAbility::MakeAndRegisterAbility(pms.GetRefPtr());
}

PowerMgrService::PowerMgrService() : SystemAbility(POWER_MANAGER_SERVICE_ID, true) {}

PowerMgrService::~PowerMgrService() {}

void PowerMgrService::OnStart()
{
    POWER_HILOGI(MODULE_SERVICE, "OnStart enter.");
    if (ready_) {
        POWER_HILOGE(MODULE_SERVICE, "OnStart is ready, nothing to do.");
        return;
    }

    if (!Init()) {
        POWER_HILOGE(MODULE_SERVICE, "OnStart call init fail");
        return;
    }
    if (!Publish(DelayedSpSingleton<PowerMgrService>::GetInstance())) {
        POWER_HILOGE(MODULE_SERVICE, "OnStart register to system ability manager failed.");
        return;
    }
    ready_ = true;
    POWER_HILOGI(MODULE_SERVICE, "OnStart and add system ability success.");
}

bool PowerMgrService::Init()
{
    POWER_HILOGI(MODULE_SERVICE, "Init start");

    if (!eventRunner_) {
        eventRunner_ = AppExecFwk::EventRunner::Create(POWERMGR_SERVICE_NAME);
        if (eventRunner_ == nullptr) {
            POWER_HILOGE(MODULE_SERVICE, "Init failed due to create EventRunner");
            return false;
        }
    }

    if (!handler_) {
        handler_ = std::make_shared<PowermsEventHandler>(eventRunner_, pms);
    }

    if (!runningLockMgr_) {
        runningLockMgr_ = std::make_shared<RunningLockMgr>(pms);
    }
    if (!runningLockMgr_->Init()) {
        POWER_HILOGE(MODULE_SERVICE, "OnStart init fail");
        return false;
    }
    if (!PowerStateMachineInit()) {
        POWER_HILOGE(MODULE_SERVICE, "power state machine init fail!");
    }
    POWER_HILOGI(MODULE_SERVICE, "Init success");
    return true;
}

bool PowerMgrService::PowerStateMachineInit()
{
    if (powerStateMachine_ == nullptr) {
        powerStateMachine_ = std::make_shared<PowerStateMachine>(pms);
        if (!(powerStateMachine_->Init())) {
            POWER_HILOGE(MODULE_SERVICE, "power state machine start fail!");
            return false;
        }
    }
    if (powerMgrNotify_ == nullptr) {
        powerMgrNotify_ = std::make_shared<PowerMgrNotify>();
        powerMgrNotify_->RegisterPublishEvents();
    }
    return true;
}

void PowerMgrService::OnStop()
{
    POWER_HILOGI(MODULE_SERVICE, "stop service");
    if (!ready_) {
        return;
    }
    eventRunner_.reset();
    handler_.reset();
    ready_ = false;
}

int32_t PowerMgrService::Dump(int32_t fd, const std::vector<std::u16string>& args)
{
    std::vector<std::string> argsInStr;
    std::transform(args.begin(), args.end(), std::back_inserter(argsInStr), [](const std::u16string &arg) {
        return Str16ToStr8(arg);
    });
    std::string result;
    PowerMgrDumper::Dump(argsInStr, result);
    if (!SaveStringToFd(fd, result)) {
        POWER_HILOGE(MODULE_SERVICE, "PowerMgrService::Dump failed, save to fd failed.");
        return ERR_OK;
    }
    return ERR_OK;
}

void PowerMgrService::RebootDevice(const std::string& reason)
{
    pid_t pid = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    if (reason.find("recovery") != std::string::npos) {
        if (!Permission::CheckCallingPermission("ohos.permission.REBOOT_RECOVERY")) {
            POWER_HILOGE(MODULE_SERVICE, "%{public}s Request failed, %{public}d permission check fail", __func__, pid);
            return;
        }
    } else {
        if ((uid >= APP_FIRST_UID) && !Permission::CheckCallingPermission("ohos.permission.REBOOT")) {
            POWER_HILOGE(MODULE_SERVICE, "%{public}s Request failed, %{public}d permission check fail", __func__, pid);
            return;
        }
    }
    POWER_HILOGI(MODULE_SERVICE, "PID: %{public}d Call %{public}s !", pid, __func__);
    shutdownService_.Reboot(reason);
}

void PowerMgrService::ShutDownDevice(const std::string& reason)
{
    pid_t pid  = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    if ((uid >= APP_FIRST_UID) && !Permission::CheckCallingPermission("ohos.permission.REBOOT")) {
        POWER_HILOGE(MODULE_SERVICE, "%{public}s Request failed, %{public}d permission check fail", __func__, pid);
        return;
    }
    POWER_HILOGI(MODULE_SERVICE, "PID: %{public}d Call %{public}s !", pid, __func__);
    shutdownService_.Shutdown(reason);
}

void PowerMgrService::SuspendDevice(int64_t callTimeMs, SuspendDeviceType reason, bool suspendImmed)
{
    auto uid = IPCSkeleton::GetCallingUid();
    if (uid >= APP_FIRST_UID) {
        POWER_HILOGE(MODULE_SERVICE, "%{public}s Request failed, illegal calling uid %{public}d.", __func__, uid);
        return;
    }
    pid_t pid = IPCSkeleton::GetCallingPid();
    powerStateMachine_->SuspendDeviceInner(pid, callTimeMs, reason, suspendImmed);
}

void PowerMgrService::WakeupDevice(int64_t callTimeMs, WakeupDeviceType reason, const std::string& details)
{
    auto uid = IPCSkeleton::GetCallingUid();
    if (uid >= APP_FIRST_UID) {
        POWER_HILOGE(MODULE_SERVICE, "%{public}s Request failed, illegal calling uid %{public}d.", __func__, uid);
        return;
    }
    pid_t pid = IPCSkeleton::GetCallingPid();
    powerStateMachine_->WakeupDeviceInner(pid, callTimeMs, reason, details, "OHOS");
}

void PowerMgrService::RefreshActivity(int64_t callTimeMs, UserActivityType type, bool needChangeBacklight)
{
    auto uid = IPCSkeleton::GetCallingUid();
    if ((uid >= APP_FIRST_UID) || !Permission::CheckCallingPermission("ohos.permission.REFRESH_USER_ACTION")) {
        POWER_HILOGE(MODULE_SERVICE, "%{public}s Request failed, illegal calling uid %{public}d.", __func__, uid);
        return;
    }
    pid_t pid = IPCSkeleton::GetCallingPid();
    powerStateMachine_->RefreshActivityInner(pid, callTimeMs, type, needChangeBacklight);
}

bool PowerMgrService::IsScreenOn()
{
    return powerStateMachine_->IsScreenOn();
}

bool PowerMgrService::ForceSuspendDevice(int64_t callTimeMs)
{
    auto uid = IPCSkeleton::GetCallingUid();
    if (uid >= APP_FIRST_UID) {
        POWER_HILOGE(MODULE_SERVICE, "%{public}s Request failed, illegal calling uid %{public}d.", __func__, uid);
        return false;
    }
    pid_t pid = IPCSkeleton::GetCallingPid();
    return powerStateMachine_->ForceSuspendDeviceInner(pid, callTimeMs);
}

inline void PowerMgrService::FillUserIPCInfo(UserIPCInfo &userIPCinfo)
{
    userIPCinfo.pid = IPCSkeleton::GetCallingPid();
    userIPCinfo.uid = IPCSkeleton::GetCallingUid();
}

void PowerMgrService::Lock(const sptr<IRemoteObject>& token, const RunningLockInfo& runningLockInfo, uint32_t timeOutMS)
{
    auto uid = IPCSkeleton::GetCallingUid();
    if ((uid >= APP_FIRST_UID) && !Permission::CheckCallingPermission("ohos.permission.RUNNING_LOCK")) {
        POWER_HILOGE(MODULE_SERVICE, "%{public}s Request failed, %{public}d permission check fail", __func__, uid);
        return;
    }

    POWER_HILOGI(MODULE_SERVICE, "%{public}s :timeOutMS = %d, name = %s, type = %d", __func__,
        timeOutMS, runningLockInfo.name.c_str(), runningLockInfo.type);

    UserIPCInfo userIPCInfo;
    FillUserIPCInfo(userIPCInfo);
    runningLockMgr_->Lock(token, runningLockInfo, userIPCInfo, timeOutMS);
    // notify runninglock is changed, true means unlock, false means lock
    NotifyRunningLockChanged(false);
}

void PowerMgrService::UnLock(const sptr<IRemoteObject>& token)
{
    auto uid = IPCSkeleton::GetCallingUid();
    if ((uid >= APP_FIRST_UID) && !Permission::CheckCallingPermission("ohos.permission.RUNNING_LOCK")) {
        POWER_HILOGE(MODULE_SERVICE, "%{public}s Request failed, %{public}d permission check fail", __func__, uid);
        return;
    }

    POWER_HILOGI(MODULE_SERVICE, "%{public}s called.", __func__);
    runningLockMgr_->UnLock(token);
    // notify runninglock is changed, true means unlock, false means lock
    NotifyRunningLockChanged(true);
}

void PowerMgrService::ForceUnLock(const sptr<IRemoteObject>& token)
{
    POWER_HILOGI(MODULE_SERVICE, "%{public}s called.", __func__);
    UnLock(token);
}

bool PowerMgrService::IsUsed(const sptr<IRemoteObject>& token)
{
    POWER_HILOGI(MODULE_SERVICE, "%{public}s called.", __func__);
    return runningLockMgr_->IsUsed(token);
}

void PowerMgrService::NotifyRunningLockChanged(bool isUnLock)
{
    if (isUnLock) {
        // When unlock we try to suspend
        if (!runningLockMgr_->ExistValidRunningLock() && !powerStateMachine_->IsScreenOn()) {
            // runninglock is empty and Screen is off, so we try to suspend device from Z side.
            POWER_HILOGI(MODULE_SERVICE, "%{public}s :RunningLock is empty, try to suspend from Z Side!", __func__);
            powerStateMachine_->SuspendDeviceInner(getpid(), GetTickCount(),
                                                   SuspendDeviceType::SUSPEND_DEVICE_REASON_MIN, true, true);
        }
    }
}

void PowerMgrService::SetWorkTriggerList(const sptr<IRemoteObject>& token, const WorkTriggerList& workTriggerList)
{
    auto uid = IPCSkeleton::GetCallingUid();
    if ((uid >= APP_FIRST_UID) && !Permission::CheckCallingPermission("ohos.permission.RUNNING_LOCK")) {
        POWER_HILOGE(MODULE_SERVICE, "%{public}s Request failed, %{public}d permission check fail", __func__, uid);
        return;
    }

    POWER_HILOGI(MODULE_SERVICE, "%{public}s called.", __func__);
    runningLockMgr_->SetWorkTriggerList(token, workTriggerList);
}

void PowerMgrService::ProxyRunningLock(bool proxyLock, pid_t uid, pid_t pid)
{
    auto calllingUid = IPCSkeleton::GetCallingUid();
    if (calllingUid >= APP_FIRST_UID) {
        POWER_HILOGE(MODULE_SERVICE, "%{public}s Request failed, illegal calling uid %{public}d.", __func__,
            calllingUid);
        return;
    }
    runningLockMgr_->ProxyRunningLock(proxyLock, uid, pid);
}

void PowerMgrService::RegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback)
{
    auto uid = IPCSkeleton::GetCallingUid();
    if ((uid >= APP_FIRST_UID) && !Permission::CheckCallingPermission("ohos.permission.POWER_MANAGER")) {
        POWER_HILOGE(MODULE_SERVICE, "%{public}s Request failed, %{public}d permission check fail", __func__, uid);
        return;
    }
    powerStateMachine_->RegisterPowerStateCallback(callback);
}

void PowerMgrService::UnRegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback)
{
    auto uid = IPCSkeleton::GetCallingUid();
    if ((uid >= APP_FIRST_UID) && !Permission::CheckCallingPermission("ohos.permission.POWER_MANAGER")) {
        POWER_HILOGE(MODULE_SERVICE, "%{public}s Request failed, %{public}d permission check fail", __func__, uid);
        return;
    }
    powerStateMachine_->UnRegisterPowerStateCallback(callback);
}

void PowerMgrService::RegisterShutdownCallback(const sptr<IShutdownCallback>& callback)
{
    auto uid = IPCSkeleton::GetCallingUid();
    if (uid >= APP_FIRST_UID) {
        POWER_HILOGE(MODULE_SERVICE, "Register failed, %{public}d fail", uid);
        return;
    }
    POWER_HILOGE(MODULE_SERVICE, "Register shutdown callback: %{public}d", uid);
    shutdownService_.AddShutdownCallback(callback);
}

void PowerMgrService::UnRegisterShutdownCallback(const sptr<IShutdownCallback>& callback)
{
    auto uid = IPCSkeleton::GetCallingUid();
    if (uid >= APP_FIRST_UID) {
        POWER_HILOGE(MODULE_SERVICE, "UnRegister failed, %{public}d fail", uid);
        return;
    }
    POWER_HILOGE(MODULE_SERVICE, "UnRegister shutdown callback: %{public}d", uid);
    shutdownService_.DelShutdownCallback(callback);
}
} // namespace PowerMgr
} // namespace OHOS
