/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "power_mgr_service_ipc_adapter.h"
#include "power_common.h"
#include "power_log.h"
#include "power_xcollie.h"
#include "xcollie/watchdog.h"

namespace OHOS {
namespace PowerMgr {
namespace {
constexpr int32_t INIT_VALUE = -1;
constexpr int32_t MAX_PARAM_NUM = 2000;
} // namespace

int32_t PowerMgrServiceAdapter::RebootDeviceIpc(const std::string& reason, int32_t& powerError)
{
    PowerXCollie powerXCollie("PowerMgrServiceAdapter::RebootDevice", false);
    powerError = static_cast<int32_t>(RebootDevice(reason));
    return ERR_OK;
}

int32_t PowerMgrServiceAdapter::RebootDeviceForDeprecatedIpc(const std::string& reason, int32_t& powerError)
{
    PowerXCollie powerXCollie("PowerMgrServiceAdapter::RebootDeviceForDeprecated", false);
    powerError = static_cast<int32_t>(RebootDeviceForDeprecated(reason));
    return ERR_OK;
}

int32_t PowerMgrServiceAdapter::ShutDownDeviceIpc(const std::string& reason, int32_t& powerError)
{
    PowerXCollie powerXCollie("PowerMgrServiceAdapter::ShutDownDevice", false);
    powerError = static_cast<int32_t>(ShutDownDevice(reason));
    return ERR_OK;
}

int32_t PowerMgrServiceAdapter::SetSuspendTagIpc(const std::string& tag, int32_t& powerError)
{
    PowerXCollie powerXCollie("PowerMgrServiceAdapter::SetSuspendTag", false);
    powerError = static_cast<int32_t>(SetSuspendTag(tag));
    return ERR_OK;
}

int32_t PowerMgrServiceAdapter::SuspendDeviceIpc(int64_t callTimeMs, int32_t reasonValue, bool suspendImmed,
    const std::string& apiVersion, int32_t& powerError)
{
    PowerXCollie powerXCollie("PowerMgrServiceAdapter::SuspendDevice", false);
    SuspendDeviceType reason = static_cast<SuspendDeviceType>(reasonValue);
    powerError = static_cast<int32_t>(SuspendDevice(callTimeMs, reason, suspendImmed, apiVersion));
    return ERR_OK;
}

int32_t PowerMgrServiceAdapter::WakeupDeviceIpc(int64_t callTimeMs, int32_t reasonValue,
    const std::string& details, const std::string& apiVersion, int32_t& powerError)
{
    PowerXCollie powerXCollie("PowerMgrServiceAdapter::WakeupDevice", false);
    WakeupDeviceType reason = static_cast<WakeupDeviceType>(reasonValue);
    powerError = static_cast<int32_t>(WakeupDevice(callTimeMs, reason, details, apiVersion));
    return ERR_OK;
}

int32_t PowerMgrServiceAdapter::WakeupDeviceAsyncIpc(
    int64_t callTimeMs, int32_t reasonValue, const std::string& details, const std::string& apiVersion)
{
    WakeupDeviceType reason = static_cast<WakeupDeviceType>(reasonValue);
    return static_cast<int32_t>(WakeupDevice(callTimeMs, reason, details, apiVersion));
}

int32_t PowerMgrServiceAdapter::RefreshActivityIpc(int64_t callTimeMs, int32_t activityType, bool needChangeBacklight)
{
    PowerXCollie powerXCollie("PowerMgrServiceAdapter::RefreshActivity", false);
    UserActivityType type = static_cast<UserActivityType>(activityType);
    RefreshActivity(callTimeMs, type, needChangeBacklight);
    return ERR_OK;
}

int32_t PowerMgrServiceAdapter::OverrideScreenOffTimeIpc(
    int64_t timeout, const std::string& apiVersion, int32_t& powerError)
{
    PowerXCollie powerXCollie("PowerMgrServiceAdapter::OverrideScreenOffTime", false);
    powerError = static_cast<int32_t>(OverrideScreenOffTime(timeout, apiVersion));
    return ERR_OK;
}

int32_t PowerMgrServiceAdapter::RestoreScreenOffTimeIpc(const std::string& apiVersion, int32_t& powerError)
{
    PowerXCollie powerXCollie("PowerMgrServiceAdapter::RestoreScreenOffTime", false);
    powerError = static_cast<int32_t>(RestoreScreenOffTime(apiVersion));
    return ERR_OK;
}

int32_t PowerMgrServiceAdapter::GetStateIpc(int32_t& powerState)
{
    PowerXCollie powerXCollie("PowerMgrServiceAdapter::GetState", false);
    powerState = static_cast<int32_t>(GetState());
    return ERR_OK;
}

int32_t PowerMgrServiceAdapter::IsScreenOnIpc(bool needPrintLog, bool& isScreenOn)
{
    PowerXCollie powerXCollie("PowerMgrServiceAdapter::IsScreenOn", false);
    isScreenOn = IsScreenOn(needPrintLog);
    return ERR_OK;
}

int32_t PowerMgrServiceAdapter::IsFoldScreenOnIpc(bool& isFoldScreenOn)
{
    PowerXCollie powerXCollie("PowerMgrServiceAdapter::IsFoldScreenOn", false);
    isFoldScreenOn = IsFoldScreenOn();
    return ERR_OK;
}


int32_t PowerMgrServiceAdapter::IsCollaborationScreenOnIpc(bool& isCollaborationScreenOn)
{
    PowerXCollie powerXCollie("PowerMgrServiceAdapter::IsCollaborationScreenOn", false);
    isCollaborationScreenOn = IsCollaborationScreenOn();
    return ERR_OK;
}

int32_t PowerMgrServiceAdapter::ForceSuspendDeviceIpc(
    int64_t callTimeMs, const std::string& apiVersion, const sptr<IPowerMgrAsync>& powerProxy)
{
    PowerXCollie powerXCollie("PowerMgrServiceAdapter::ForceSuspendDevice", false);
    int32_t result = static_cast<int32_t>(ForceSuspendDevice(callTimeMs, apiVersion));
    powerProxy->SendAsyncReply(result);
    return result;
}

int32_t PowerMgrServiceAdapter::HibernateIpc(bool clearMemory, const std::string& reason,
    const std::string& apiVersion, const sptr<IPowerMgrAsync>& powerProxy)
{
    PowerXCollie powerXCollie("PowerMgrServiceAdapter::Hibernate", false);
    int32_t result = static_cast<int32_t>(Hibernate(clearMemory, reason, apiVersion));
    powerProxy->SendAsyncReply(result);
    return result;
}

int32_t PowerMgrServiceAdapter::CreateRunningLockIpc(
    const sptr<IRemoteObject>& remoteObj, const RunningLockInfo& runningLockInfo, int32_t& powerError)
{
    PowerXCollie powerXCollie("PowerMgrServiceAdapter::CreateRunningLock", true);
    powerError = static_cast<int32_t>(CreateRunningLock(remoteObj, runningLockInfo));
    return ERR_OK;
}

int32_t PowerMgrServiceAdapter::ReleaseRunningLockIpc(const sptr<IRemoteObject>& remoteObj, const std::string& name)
{
    PowerXCollie powerXCollie("PowerMgrServiceAdapter::ReleaseRunningLock", true);
    ReleaseRunningLock(remoteObj, name);
    return ERR_OK;
}

int32_t PowerMgrServiceAdapter::IsRunningLockTypeSupportedIpc(int32_t lockType, bool& lockSupported)
{
    PowerXCollie powerXCollie("PowerMgrServiceAdapter::IsRunningLockTypeSupported", false);
    RunningLockType type = static_cast<RunningLockType>(lockType);
    lockSupported = IsRunningLockTypeSupported(type);
    return ERR_OK;
}

int32_t PowerMgrServiceAdapter::UpdateWorkSourceIpc(
    const sptr<IRemoteObject>& remoteObj, const std::vector<int32_t>& workSources)
{
    PowerXCollie powerXCollie("PowerMgrServiceAdapter::UpdateWorkSource", true);
    UpdateWorkSource(remoteObj, workSources);
    return ERR_OK;
}

int32_t PowerMgrServiceAdapter::LockIpc(const sptr<IRemoteObject>& remoteObj, int32_t timeOutMs, int32_t& powerError)
{
    PowerXCollie powerXCollie("PowerMgrServiceAdapter::Lock", true);
    powerError = static_cast<int32_t>(Lock(remoteObj, timeOutMs));
    return ERR_OK;
}

int32_t PowerMgrServiceAdapter::UnLockIpc(
    const sptr<IRemoteObject>& remoteObj, const std::string& name, int32_t& powerError)
{
    PowerXCollie powerXCollie("PowerMgrServiceAdapter::UnLock", true);
    powerError = static_cast<int32_t>(UnLock(remoteObj, name));
    return ERR_OK;
}

int32_t PowerMgrServiceAdapter::QueryRunningLockListsIpc(std::map<std::string, RunningLockInfo>& runningLockLists)
{
    PowerXCollie powerXCollie("PowerMgrServiceAdapter::QueryRunningLockLists", true);
    QueryRunningLockLists(runningLockLists);
    int32_t num = static_cast<int32_t>(runningLockLists.size());
    RETURN_IF_WITH_RET(num > MAX_PARAM_NUM, INIT_VALUE);
    return ERR_OK;
}

int32_t PowerMgrServiceAdapter::RegisterRunningLockCallbackIpc(const sptr<IPowerRunninglockCallback>& callback)
{
    PowerXCollie powerXCollie("PowerMgrServiceAdapter::RegisterRunningLockCallback", false);
    RegisterRunningLockCallback(callback);
    return ERR_OK;
}

int32_t PowerMgrServiceAdapter::UnRegisterRunningLockCallbackIpc(const sptr<IPowerRunninglockCallback>& callback)
{
    PowerXCollie powerXCollie("PowerMgrServiceAdapter::UnRegisterRunningLockCallback", false);
    UnRegisterRunningLockCallback(callback);
    return ERR_OK;
}

int32_t PowerMgrServiceAdapter::IsUsedIpc(const sptr<IRemoteObject>& remoteObj, bool& isUsed)
{
    PowerXCollie powerXCollie("PowerMgrServiceAdapter::IsUsed", true);
    isUsed = IsUsed(remoteObj);
    return ERR_OK;
}

int32_t PowerMgrServiceAdapter::ProxyRunningLockIpc(bool isProxied, pid_t pid, pid_t uid)
{
    PowerXCollie powerXCollie("PowerMgrServiceAdapter::ProxyRunningLock", true);
    if (!ProxyRunningLock(isProxied, pid, uid)) {
        POWER_HILOGI(FEATURE_SUSPEND, "ProxyRunningLock failed");
        return INIT_VALUE;
    }
    return ERR_OK;
}

int32_t PowerMgrServiceAdapter::ProxyRunningLocksIpc(bool isProxied, const VectorPair& vectorPairInfos)
{
    PowerXCollie powerXCollie("PowerMgrServiceAdapter::ProxyRunningLocks", true);
    const std::vector<std::pair<pid_t, pid_t>>& processInfos = vectorPairInfos.GetProcessInfos();
    if (!ProxyRunningLocks(isProxied, processInfos)) {
        POWER_HILOGI(FEATURE_SUSPEND, "ProxyRunningLocks failed");
        return INIT_VALUE;
    }
    return ERR_OK;
}

int32_t PowerMgrServiceAdapter::ResetRunningLocksIpc()
{
    PowerXCollie powerXCollie("PowerMgrServiceAdapter::ResetRunningLocks", true);
    if (!ResetRunningLocks()) {
        POWER_HILOGI(FEATURE_SUSPEND, "ResetRunningLocks failed");
        return INIT_VALUE;
    }
    return ERR_OK;
}

int32_t PowerMgrServiceAdapter::RegisterPowerStateCallbackIpc(const sptr<IPowerStateCallback>& callback, bool isSync)
{
    PowerXCollie powerXCollie("PowerMgrServiceAdapter::RegisterPowerStateCallback", false);
    RegisterPowerStateCallback(callback, isSync);
    return ERR_OK;
}

int32_t PowerMgrServiceAdapter::UnRegisterPowerStateCallbackIpc(const sptr<IPowerStateCallback>& callback)
{
    PowerXCollie powerXCollie("PowerMgrServiceAdapter::UnRegisterPowerStateCallback", false);
    UnRegisterPowerStateCallback(callback);
    return ERR_OK;
}

int32_t PowerMgrServiceAdapter::RegisterSyncSleepCallbackIpc(
    const sptr<ISyncSleepCallback>& callback, int32_t priorityValue)
{
    PowerXCollie powerXCollie("PowerMgrServiceAdapter::RegisterSyncSleepCallback", false);
    SleepPriority priority = static_cast<SleepPriority>(priorityValue);
    RegisterSyncSleepCallback(callback, priority);
    return ERR_OK;
}

int32_t PowerMgrServiceAdapter::UnRegisterSyncSleepCallbackIpc(const sptr<ISyncSleepCallback>& callback)
{
    PowerXCollie powerXCollie("PowerMgrServiceAdapter::UnRegisterSyncSleepCallback", false);
    UnRegisterSyncSleepCallback(callback);
    return ERR_OK;
}

int32_t PowerMgrServiceAdapter::RegisterSyncHibernateCallbackIpc(const sptr<ISyncHibernateCallback>& callback)
{
    PowerXCollie powerXCollie("PowerMgrServiceAdapter::RegisterSyncHibernateCallback", false);
    RegisterSyncHibernateCallback(callback);
    return ERR_OK;
}

int32_t PowerMgrServiceAdapter::UnRegisterSyncHibernateCallbackIpc(const sptr<ISyncHibernateCallback>& callback)
{
    PowerXCollie powerXCollie("PowerMgrServiceAdapter::UnRegisterSyncHibernateCallback", false);
    UnRegisterSyncHibernateCallback(callback);
    return ERR_OK;
}

int32_t PowerMgrServiceAdapter::RegisterPowerModeCallbackIpc(const sptr<IPowerModeCallback>& callback)
{
    PowerXCollie powerXCollie("PowerMgrServiceAdapter::RegisterPowerModeCallback", false);
    RegisterPowerModeCallback(callback);
    return ERR_OK;
}

int32_t PowerMgrServiceAdapter::UnRegisterPowerModeCallbackIpc(const sptr<IPowerModeCallback>& callback)
{
    PowerXCollie powerXCollie("PowerMgrServiceAdapter::UnRegisterPowerModeCallback", false);
    UnRegisterPowerModeCallback(callback);
    return ERR_OK;
}

int32_t PowerMgrServiceAdapter::RegisterScreenStateCallbackIpc(
    int32_t remainTime, const sptr<IScreenOffPreCallback>& callback)
{
    PowerXCollie powerXCollie("PowerMgrServiceAdapter::RegisterScreenStateCallback", false);
    RegisterScreenStateCallback(remainTime, callback);
    return ERR_OK;
}

int32_t PowerMgrServiceAdapter::UnRegisterScreenStateCallbackIpc(const sptr<IScreenOffPreCallback>& callback)
{
    PowerXCollie powerXCollie("PowerMgrServiceAdapter::UnRegisterScreenStateCallback", false);
    UnRegisterScreenStateCallback(callback);
    return ERR_OK;
}

int32_t PowerMgrServiceAdapter::SetDisplaySuspendIpc(bool enable)
{
    PowerXCollie powerXCollie("PowerMgrServiceAdapter::SetDisplaySuspend", false);
    SetDisplaySuspend(enable);
    return ERR_OK;
}

int32_t PowerMgrServiceAdapter::SetDeviceModeIpc(int32_t modeValue, int32_t& powerError)
{
    PowerXCollie powerXCollie("PowerMgrServiceAdapter::SetDeviceMode", false);
    PowerMode mode = static_cast<PowerMode>(modeValue);
    powerError = static_cast<int32_t>(SetDeviceMode(mode));
    return ERR_OK;
}

int32_t PowerMgrServiceAdapter::GetDeviceModeIpc(int32_t& powerMode)
{
    PowerXCollie powerXCollie("PowerMgrServiceAdapter::GetDeviceMode", false);
    powerMode = static_cast<int32_t>(GetDeviceMode());
    return ERR_OK;
}

int32_t PowerMgrServiceAdapter::ShellDumpIpc(
    const std::vector<std::string>& args, uint32_t argc, std::string& returnDump)
{
    PowerXCollie powerXCollie("PowerMgrServiceAdapter::ShellDump", false);
    returnDump = ShellDump(args, argc);
    return ERR_OK;
}

int32_t PowerMgrServiceAdapter::RegisterShutdownCallbackIpc(
    const sptr<ITakeOverShutdownCallback>& callback, int32_t priorityValue)
{
    PowerXCollie powerXCollie("PowerMgrServiceAdapter::RegisterShutdownCallback TakeOver", false);
    ShutdownPriority priority = static_cast<ShutdownPriority>(priorityValue);
    RegisterShutdownCallback(callback, priority);
    return ERR_OK;
}

int32_t PowerMgrServiceAdapter::UnRegisterShutdownCallbackIpc(const sptr<ITakeOverShutdownCallback>& callback)
{
    PowerXCollie powerXCollie("PowerMgrServiceAdapter::UnRegisterShutdownCallback TakeOver", false);
    UnRegisterShutdownCallback(callback);
    return ERR_OK;
}

int32_t PowerMgrServiceAdapter::RegisterShutdownCallbackIpc(
    const sptr<IAsyncShutdownCallback>& callback, int32_t priorityValue)
{
    PowerXCollie powerXCollie("PowerMgrServiceAdapter::RegisterShutdownCallback Async", false);
    ShutdownPriority priority = static_cast<ShutdownPriority>(priorityValue);
    RegisterShutdownCallback(callback, priority);
    return ERR_OK;
}

int32_t PowerMgrServiceAdapter::UnRegisterShutdownCallbackIpc(const sptr<IAsyncShutdownCallback>& callback)
{
    PowerXCollie powerXCollie("PowerMgrServiceAdapter::UnRegisterShutdownCallback Async", false);
    UnRegisterShutdownCallback(callback);
    return ERR_OK;
}

int32_t PowerMgrServiceAdapter::RegisterShutdownCallbackIpc(
    const sptr<ISyncShutdownCallback>& callback, int32_t priorityValue)
{
    PowerXCollie powerXCollie("PowerMgrServiceAdapter::RegisterShutdownCallback Sync", false);
    ShutdownPriority priority = static_cast<ShutdownPriority>(priorityValue);
    RegisterShutdownCallback(callback, priority);
    return ERR_OK;
}

int32_t PowerMgrServiceAdapter::UnRegisterShutdownCallbackIpc(const sptr<ISyncShutdownCallback>& callback)
{
    PowerXCollie powerXCollie("PowerMgrServiceAdapter::UnRegisterShutdownCallback Sync", false);
    UnRegisterShutdownCallback(callback);
    return ERR_OK;
}

int32_t PowerMgrServiceAdapter::IsStandbyIpc(bool& isStandby, int32_t& powerError)
{
    PowerXCollie powerXCollie("PowerMgrServiceAdapter::IsStandby", false);
    powerError = static_cast<int32_t>(IsStandby(isStandby));
    return ERR_OK;
}

int32_t PowerMgrServiceAdapter::SetForceTimingOutIpc(
    bool enabled, const sptr<IRemoteObject>& token, int32_t& powerError)
{
    PowerXCollie powerXCollie("PowerMgrServiceAdapter::SetForceTimingOut", false);
    powerError = static_cast<int32_t>(SetForceTimingOut(enabled, token));
    return ERR_OK;
}

int32_t PowerMgrServiceAdapter::LockScreenAfterTimingOutIpc(bool enabledLockScreen, bool checkLock,
    bool sendScreenOffEvent, const sptr<IRemoteObject>& token, int32_t& powerError)
{
    PowerXCollie powerXCollie("PowerMgrServiceAdapter::LockScreenAfterTimingOut", false);
    powerError = static_cast<int32_t>(
        LockScreenAfterTimingOut(enabledLockScreen, checkLock, sendScreenOffEvent, token));
    return ERR_OK;
}

int32_t PowerMgrServiceAdapter::IsRunningLockEnabledIpc(int32_t lockType, bool& result, int32_t& powerError)
{
    PowerXCollie powerXCollie("PowerMgrServiceAdapter::IsRunningLockEnabled", false);
    RunningLockType type = static_cast<RunningLockType>(lockType);
    powerError = static_cast<int32_t>(IsRunningLockEnabled(type, result));
    return ERR_OK;
}

int32_t PowerMgrServiceAdapter::IsForceSleepingIpc(bool& isForceSleeping)
{
    PowerXCollie powerXCollie("PowerMgrServiceAdapter::IsForceSleeping", false);
    isForceSleeping = IsForceSleeping();
    return ERR_OK;
}

int32_t PowerMgrServiceAdapter::RefreshActivityIpc(
    int64_t callTimeMs, int32_t activityType, const std::string& refreshReason, int32_t& powerError)
{
    PowerXCollie powerXCollie("PowerMgrServiceAdapter::RefreshActivity LongIntStringInt", false);
    UserActivityType type = static_cast<UserActivityType>(activityType);
    powerError = static_cast<int32_t>(RefreshActivity(callTimeMs, type, refreshReason));
    return ERR_OK;
}
} // namespace PowerMgr
} // namespace OHOS
