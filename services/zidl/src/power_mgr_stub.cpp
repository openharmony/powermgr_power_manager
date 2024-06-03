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

#include "power_mgr_stub.h"

#include <message_parcel.h>
#include <string_ex.h>

#include "running_lock_info.h"
#include "power_common.h"
#include "power_errors.h"
#include "power_mgr_ipc_interface_code.h"
#include "shutdown_stub_delegator.h"
#include "shutdown/shutdown_client_ipc_interface_code.h"
#include "xcollie/xcollie.h"

using namespace OHOS::HiviewDFX;

namespace OHOS {
namespace PowerMgr {
namespace {
constexpr uint32_t PARAM_MAX_NUM = 10;
constexpr int32_t MAX_PROXY_RUNNINGLOCK_NUM = 2000;
std::unique_ptr<ShutdownStubDelegator> g_shutdownDelegator;
std::mutex g_shutdownMutex;
}
int PowerMgrStub::OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    POWER_HILOGD(COMP_FWK, "code=%{public}u, flags=%{public}d", code, option.GetFlags());
    std::u16string descriptor = PowerMgrStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        POWER_HILOGE(COMP_FWK, "Descriptor is not matched");
        return E_GET_POWER_SERVICE_FAILED;
    }
    if (IsShutdownCommand(code)) {
        {
            std::lock_guard<std::mutex> lock(g_shutdownMutex);
            if (g_shutdownDelegator == nullptr) {
                g_shutdownDelegator = std::make_unique<ShutdownStubDelegator>(*this);
                RETURN_IF_WITH_RET(g_shutdownDelegator == nullptr, ERR_NO_INIT)
            }
        }
        int ret = g_shutdownDelegator->HandleRemoteRequest(code, data, reply, option);
        if (ret == ERR_INVALID_OPERATION) {
            ret = IPCObjectStub::OnRemoteRequest(code, data, reply, option);
        }
        return ret;
    }

    const int DFX_DELAY_MS = 10000;
    int id = HiviewDFX::XCollie::GetInstance().SetTimer("PowerMgrStub", DFX_DELAY_MS, nullptr, nullptr,
        HiviewDFX::XCOLLIE_FLAG_NOOP);

    int ret = ERR_OK;
    switch (code) {
        case static_cast<int>(PowerMgr::PowerMgrInterfaceCode::WAKEUP_DEVICE):
            ret = WakeupDeviceStub(data, reply);
            break;
        case static_cast<int>(PowerMgr::PowerMgrInterfaceCode::SUSPEND_DEVICE):
            ret = SuspendDeviceStub(data, reply);
            break;
        case static_cast<int>(PowerMgr::PowerMgrInterfaceCode::REFRESH_ACTIVITY):
            ret = RefreshActivityStub(data);
            break;
        case static_cast<int>(PowerMgr::PowerMgrInterfaceCode::REBOOT_DEVICE):
            ret = RebootDeviceStub(data, reply);
            break;
        case static_cast<int>(PowerMgr::PowerMgrInterfaceCode::REBOOT_DEVICE_FOR_DEPRECATED):
            ret = RebootDeviceForDeprecatedStub(data, reply);
            break;
        case static_cast<int>(PowerMgr::PowerMgrInterfaceCode::SHUTDOWN_DEVICE):
            ret = ShutDownDeviceStub(data, reply);
            break;
        case static_cast<int>(PowerMgr::PowerMgrInterfaceCode::OVERRIDE_DISPLAY_OFF_TIME):
            ret = OverrideScreenOffTimeStub(data, reply);
            break;
        case static_cast<int>(PowerMgr::PowerMgrInterfaceCode::RESTORE_DISPLAY_OFF_TIME):
            ret = RestoreScreenOffTimeStub(reply);
            break;
        case static_cast<int>(PowerMgr::PowerMgrInterfaceCode::GET_STATE):
            ret = GetStateStub(reply);
            break;
        case static_cast<int>(PowerMgr::PowerMgrInterfaceCode::IS_SCREEN_ON):
            ret = IsScreenOnStub(reply);
            break;
        case static_cast<int>(PowerMgr::PowerMgrInterfaceCode::FORCE_DEVICE_SUSPEND):
            ret = ForceSuspendDeviceStub(data, reply);
            break;
        case static_cast<int>(PowerMgr::PowerMgrInterfaceCode::CREATE_RUNNINGLOCK):
            ret = CreateRunningLockStub(data, reply);
            break;
        case static_cast<int>(PowerMgr::PowerMgrInterfaceCode::RELEASE_RUNNINGLOCK):
            ret = ReleaseRunningLockStub(data);
            break;
        case static_cast<int>(PowerMgr::PowerMgrInterfaceCode::IS_RUNNINGLOCK_TYPE_SUPPORTED):
            ret = IsRunningLockTypeSupportedStub(data, reply);
            break;
        case static_cast<int>(PowerMgr::PowerMgrInterfaceCode::RUNNINGLOCK_LOCK):
            ret = LockStub(data);
            break;
        case static_cast<int>(PowerMgr::PowerMgrInterfaceCode::RUNNINGLOCK_UNLOCK):
            ret = UnLockStub(data);
            break;
        case static_cast<int>(PowerMgr::PowerMgrInterfaceCode::RUNNINGLOCK_QUERY):
            ret = QueryRunningLockListsStub(data, reply);
            break;
        case static_cast<int>(PowerMgr::PowerMgrInterfaceCode::PROXY_RUNNINGLOCK):
            ret = ProxyRunningLockStub(data, reply);
            break;
        case static_cast<int>(PowerMgr::PowerMgrInterfaceCode::RUNNINGLOCK_ISUSED):
            ret = IsUsedStub(data, reply);
            break;
        case static_cast<int>(PowerMgr::PowerMgrInterfaceCode::PROXY_RUNNINGLOCKS):
            ret = ProxyRunningLocksStub(data, reply);
            break;
        case static_cast<int>(PowerMgr::PowerMgrInterfaceCode::RESET_RUNNINGLOCKS):
            ret = ResetAllPorxyStub(data, reply);
            break;
        case static_cast<int>(PowerMgr::PowerMgrInterfaceCode::REG_POWER_STATE_CALLBACK):
            ret = RegisterPowerStateCallbackStub(data);
            break;
        case static_cast<int>(PowerMgr::PowerMgrInterfaceCode::UNREG_POWER_STATE_CALLBACK):
            ret = UnRegisterPowerStateCallbackStub(data);
            break;
        case static_cast<int>(PowerMgr::PowerMgrInterfaceCode::REG_POWER_MODE_CALLBACK):
            ret = RegisterPowerModeCallbackStub(data);
            break;
        case static_cast<int>(PowerMgr::PowerMgrInterfaceCode::UNREG_POWER_MODE_CALLBACK):
            ret = UnRegisterPowerModeCallbackStub(data);
            break;
        case static_cast<int>(PowerMgr::PowerMgrInterfaceCode::SET_DISPLAY_SUSPEND):
            ret = SetDisplaySuspendStub(data);
            break;
        case static_cast<int>(PowerMgr::PowerMgrInterfaceCode::HIBERNATE):
            ret = HibernateStub(data);
            break;
        case static_cast<int>(PowerMgr::PowerMgrInterfaceCode::SETMODE_DEVICE):
            ret = SetDeviceModeStub(data, reply);
            break;
        case static_cast<int>(PowerMgr::PowerMgrInterfaceCode::GETMODE_DEVICE):
            ret = GetDeviceModeStub(reply);
            break;
        case static_cast<int>(PowerMgr::PowerMgrInterfaceCode::SHELL_DUMP):
            ret = ShellDumpStub(data, reply);
            break;
        case static_cast<int>(PowerMgr::PowerMgrInterfaceCode::IS_STANDBY):
            ret = IsStandbyStub(data, reply);
            break;
        case static_cast<int>(PowerMgr::PowerMgrInterfaceCode::REG_SYNC_SLEEP_CALLBACK):
            ret = RegisterSyncSleepCallbackStub(data);
            break;
        case static_cast<int>(PowerMgr::PowerMgrInterfaceCode::UNREG_SYNC_SLEEP_CALLBACK):
            ret = UnRegisterSyncSleepCallbackStub(data);
            break;
        case static_cast<int>(PowerMgr::PowerMgrInterfaceCode::SET_FORCE_TIMING_OUT):
            ret = SetForceTimingOutStub(data, reply);
            break;
        case static_cast<int>(PowerMgr::PowerMgrInterfaceCode::LOCK_SCREEN_AFTER_TIMING_OUT):
            ret = LockScreenAfterTimingOutStub(data, reply);
            break;
        case static_cast<int>(PowerMgr::PowerMgrInterfaceCode::REG_RUNNINGLOCK_CALLBACK):
            ret = RegisterRunningLockCallbackStub(data);
            break;
        case static_cast<int>(PowerMgr::PowerMgrInterfaceCode::UNREG_RUNNINGLOCK_CALLBACK):
            ret = UnRegisterRunningLockCallbackStub(data);
            break;
        case static_cast<int>(PowerMgr::PowerMgrInterfaceCode::REG_SCREEN_OFF_PRE_CALLBACK):
            ret = RegisterScreenStateCallbackStub(data);
            break;
        case static_cast<int>(PowerMgr::PowerMgrInterfaceCode::UNREG_SCREEN_OFF_PRE_CALLBACK):
            ret = UnRegisterScreenStateCallbackStub(data);
            break;
        default:
            ret = IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    HiviewDFX::XCollie::GetInstance().CancelTimer(id);
    return ret;
}

int32_t PowerMgrStub::CreateRunningLockStub(MessageParcel& data, MessageParcel& reply)
{
    sptr<IRemoteObject> remoteObj = data.ReadRemoteObject();
    RETURN_IF_WITH_RET((remoteObj == nullptr), E_READ_PARCEL_ERROR);
    std::unique_ptr<RunningLockInfo> runningLockInfo(data.ReadParcelable<RunningLockInfo>());
    RETURN_IF_WITH_RET((runningLockInfo == nullptr), E_READ_PARCEL_ERROR);
    PowerErrors error = CreateRunningLock(remoteObj, *runningLockInfo);
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(reply, Int32, static_cast<int32_t>(error), E_WRITE_PARCEL_ERROR);
    return ERR_OK;
}

int32_t PowerMgrStub::ReleaseRunningLockStub(MessageParcel& data)
{
    sptr<IRemoteObject> remoteObj = data.ReadRemoteObject();
    RETURN_IF_WITH_RET((remoteObj == nullptr), E_READ_PARCEL_ERROR);
    ReleaseRunningLock(remoteObj);
    return ERR_OK;
}

int32_t PowerMgrStub::IsRunningLockTypeSupportedStub(MessageParcel& data, MessageParcel& reply)
{
    auto type = static_cast<uint32_t >(RunningLockType::RUNNINGLOCK_BUTT);
    bool ret = false;
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Uint32, type, E_READ_PARCEL_ERROR);
    ret = IsRunningLockTypeSupported(static_cast<RunningLockType>(type));
    if (!reply.WriteBool(ret)) {
        POWER_HILOGE(FEATURE_SUSPEND, "WriteBool fail");
        return E_WRITE_PARCEL_ERROR;
    }
    return ERR_OK;
}

int32_t PowerMgrStub::LockStub(MessageParcel& data)
{
    sptr<IRemoteObject> remoteObj = data.ReadRemoteObject();
    RETURN_IF_WITH_RET((remoteObj == nullptr), E_READ_PARCEL_ERROR);
    int32_t timeOutMs = 0;
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Int32, timeOutMs, E_READ_PARCEL_ERROR);
    Lock(remoteObj, timeOutMs);
    return ERR_OK;
}

int32_t PowerMgrStub::UnLockStub(MessageParcel& data)
{
    sptr<IRemoteObject> remoteObj = data.ReadRemoteObject();
    RETURN_IF_WITH_RET((remoteObj == nullptr), E_READ_PARCEL_ERROR);
    UnLock(remoteObj);
    return ERR_OK;
}

int32_t PowerMgrStub::QueryRunningLockListsStub(MessageParcel& data, MessageParcel& reply)
{
    std::map<std::string, RunningLockInfo> runningLockLists;
    QueryRunningLockLists(runningLockLists);
    reply.WriteInt32(runningLockLists.size());
    for (auto it : runningLockLists) {
        reply.WriteString(it.first);
        reply.WriteParcelable(&it.second);
    }
    return ERR_OK;
}
int32_t PowerMgrStub::IsUsedStub(MessageParcel& data, MessageParcel& reply)
{
    sptr<IRemoteObject> remoteObj = data.ReadRemoteObject();
    RETURN_IF_WITH_RET((remoteObj == nullptr), E_READ_PARCEL_ERROR);
    bool ret = IsUsed(remoteObj);
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(reply, Bool, ret, E_WRITE_PARCEL_ERROR);
    return ERR_OK;
}

int32_t PowerMgrStub::ProxyRunningLockStub(MessageParcel& data, MessageParcel& reply)
{
    bool isProxied = false;
    pid_t uid;
    pid_t pid;
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Bool, isProxied, E_READ_PARCEL_ERROR);
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Int32, pid, E_READ_PARCEL_ERROR);
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Int32, uid, E_READ_PARCEL_ERROR);
    bool ret = ProxyRunningLock(isProxied, pid, uid);
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(reply, Bool, ret, E_WRITE_PARCEL_ERROR);
    return ERR_OK;
}

int32_t PowerMgrStub::ProxyRunningLocksStub(MessageParcel& data, MessageParcel& reply)
{
    bool isProxied = false;
    std::vector<std::pair<pid_t, pid_t>> processInfos {};
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Bool, isProxied, E_READ_PARCEL_ERROR);
    int32_t size {0};
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Int32, size, E_READ_PARCEL_ERROR);
    if (size <= 0 || size > MAX_PROXY_RUNNINGLOCK_NUM) {
        POWER_HILOGW(COMP_FWK, "size exceed limit, size=%{public}d", size);
        return E_EXCEED_PARAM_LIMIT;
    }
    processInfos.resize(size);
    for (int i = 0; i < size; ++i) {
        RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Int32, processInfos[i].first, E_READ_PARCEL_ERROR);
        RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Int32, processInfos[i].second, E_READ_PARCEL_ERROR);
    }
    bool ret = ProxyRunningLocks(isProxied, processInfos);
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(reply, Bool, ret, E_WRITE_PARCEL_ERROR);
    return ERR_OK;
}

int32_t PowerMgrStub::ResetAllPorxyStub(MessageParcel& data, MessageParcel& reply)
{
    bool ret = ResetRunningLocks();
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(reply, Bool, ret, E_WRITE_PARCEL_ERROR);
    return ERR_OK;
}

int32_t PowerMgrStub::RebootDeviceStub(MessageParcel& data, MessageParcel& reply)
{
    std::string reason = Str16ToStr8(data.ReadString16());
    PowerErrors error = RebootDevice(reason);
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(reply, Int32, static_cast<int32_t>(error), E_WRITE_PARCEL_ERROR);
    return ERR_OK;
}

int32_t PowerMgrStub::RebootDeviceForDeprecatedStub(MessageParcel& data, MessageParcel& reply)
{
    std::string reason = Str16ToStr8(data.ReadString16());
    PowerErrors error = RebootDeviceForDeprecated(reason);
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(reply, Int32, static_cast<int32_t>(error), E_WRITE_PARCEL_ERROR);
    return ERR_OK;
}

int32_t PowerMgrStub::ShutDownDeviceStub(MessageParcel& data, MessageParcel& reply)
{
    std::string reason = Str16ToStr8(data.ReadString16());
    PowerErrors error = ShutDownDevice(reason);
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(reply, Int32, static_cast<int32_t>(error), E_WRITE_PARCEL_ERROR);
    return ERR_OK;
}

int32_t PowerMgrStub::WakeupDeviceStub(MessageParcel& data, MessageParcel& reply)
{
    int64_t time = 0;
    uint32_t reason = 0;

    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Int64, time, E_READ_PARCEL_ERROR);
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Uint32, reason, E_READ_PARCEL_ERROR);
    std::string details = Str16ToStr8(data.ReadString16());

    PowerErrors error = WakeupDevice(time, static_cast<WakeupDeviceType>(reason), details);
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(reply, Int32, static_cast<int32_t>(error), E_WRITE_PARCEL_ERROR);
    return ERR_OK;
}

int32_t PowerMgrStub::SuspendDeviceStub(MessageParcel& data, MessageParcel& reply)
{
    int64_t time = 0;
    uint32_t reason = 0;
    bool suspendImmed = true;

    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Int64, time, E_READ_PARCEL_ERROR);
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Uint32, reason, E_READ_PARCEL_ERROR);
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Bool, suspendImmed, E_READ_PARCEL_ERROR);

    PowerErrors error = SuspendDevice(time, static_cast<SuspendDeviceType>(reason), suspendImmed);
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(reply, Int32, static_cast<int32_t>(error), E_WRITE_PARCEL_ERROR);
    return ERR_OK;
}

int32_t PowerMgrStub::RefreshActivityStub(MessageParcel& data)
{
    int64_t time = 0;
    uint32_t type = 0;
    bool needChangeBacklight = true;

    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Int64, time, E_READ_PARCEL_ERROR);
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Uint32, type, E_READ_PARCEL_ERROR);
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Bool, needChangeBacklight, E_READ_PARCEL_ERROR);

    RefreshActivity(time, static_cast<UserActivityType>(type), needChangeBacklight);
    return ERR_OK;
}

int32_t PowerMgrStub::OverrideScreenOffTimeStub(MessageParcel& data, MessageParcel& reply)
{
    int64_t timeout = 0;

    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Int64, timeout, E_READ_PARCEL_ERROR);

    bool ret = OverrideScreenOffTime(timeout);
    if (!reply.WriteBool(ret)) {
        POWER_HILOGE(COMP_FWK, "WriteBool fail");
        return E_WRITE_PARCEL_ERROR;
    }
    return ERR_OK;
}

int32_t PowerMgrStub::RestoreScreenOffTimeStub(MessageParcel& reply)
{
    bool ret = RestoreScreenOffTime();
    if (!reply.WriteBool(ret)) {
        POWER_HILOGE(COMP_FWK, "WriteBool fail");
        return E_WRITE_PARCEL_ERROR;
    }
    return ERR_OK;
}

int32_t PowerMgrStub::ForceSuspendDeviceStub(MessageParcel& data, MessageParcel& reply)
{
    bool ret = false;
    int64_t time = 0;

    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Int64, time, E_READ_PARCEL_ERROR);

    ret = ForceSuspendDevice(time);
    if (!reply.WriteBool(ret)) {
        POWER_HILOGE(FEATURE_SUSPEND, "WriteBool fail");
        return E_WRITE_PARCEL_ERROR;
    }
    return ERR_OK;
}

int32_t PowerMgrStub::GetStateStub(MessageParcel& reply)
{
    PowerState ret = GetState();
    if (!reply.WriteUint32(static_cast<uint32_t>(ret))) {
        POWER_HILOGE(FEATURE_POWER_STATE, "WriteUint32 fail");
        return E_WRITE_PARCEL_ERROR;
    }
    return ERR_OK;
}

int32_t PowerMgrStub::IsScreenOnStub(MessageParcel& reply)
{
    bool ret = false;
    ret = IsScreenOn();
    if (!reply.WriteBool(ret)) {
        POWER_HILOGE(COMP_FWK, "WriteBool fail");
        return E_WRITE_PARCEL_ERROR;
    }
    return ERR_OK;
}

int32_t PowerMgrStub::RegisterPowerStateCallbackStub(MessageParcel& data)
{
    sptr<IRemoteObject> obj = data.ReadRemoteObject();
    RETURN_IF_WITH_RET((obj == nullptr), E_READ_PARCEL_ERROR);
    sptr<IPowerStateCallback> callback = iface_cast<IPowerStateCallback>(obj);
    RETURN_IF_WITH_RET((callback == nullptr), E_READ_PARCEL_ERROR);
    RegisterPowerStateCallback(callback);
    return ERR_OK;
}

int32_t PowerMgrStub::UnRegisterPowerStateCallbackStub(MessageParcel& data)
{
    sptr<IRemoteObject> obj = data.ReadRemoteObject();
    RETURN_IF_WITH_RET((obj == nullptr), E_READ_PARCEL_ERROR);
    sptr<IPowerStateCallback> callback = iface_cast<IPowerStateCallback>(obj);
    RETURN_IF_WITH_RET((callback == nullptr), E_READ_PARCEL_ERROR);
    UnRegisterPowerStateCallback(callback);
    return ERR_OK;
}

int32_t PowerMgrStub::RegisterSyncSleepCallbackStub(MessageParcel& data)
{
    uint32_t priority;
    sptr<IRemoteObject> obj = data.ReadRemoteObject();
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Uint32, priority, E_READ_PARCEL_ERROR);
    RETURN_IF_WITH_RET((obj == nullptr), E_READ_PARCEL_ERROR);
    sptr<ISyncSleepCallback> callback = iface_cast<ISyncSleepCallback>(obj);
    RETURN_IF_WITH_RET((callback == nullptr), E_READ_PARCEL_ERROR);
    RegisterSyncSleepCallback(callback);
    return ERR_OK;
}

int32_t PowerMgrStub::UnRegisterSyncSleepCallbackStub(MessageParcel& data)
{
    sptr<IRemoteObject> obj = data.ReadRemoteObject();
    RETURN_IF_WITH_RET((obj == nullptr), E_READ_PARCEL_ERROR);
    sptr<ISyncSleepCallback> callback = iface_cast<ISyncSleepCallback>(obj);
    RETURN_IF_WITH_RET((callback == nullptr), E_READ_PARCEL_ERROR);
    UnRegisterSyncSleepCallback(callback);
    return ERR_OK;
}

int32_t PowerMgrStub::RegisterPowerModeCallbackStub(MessageParcel& data)
{
    sptr<IRemoteObject> obj = data.ReadRemoteObject();
    RETURN_IF_WITH_RET((obj == nullptr), E_READ_PARCEL_ERROR);
    sptr<IPowerModeCallback> callback = iface_cast<IPowerModeCallback>(obj);
    RETURN_IF_WITH_RET((callback == nullptr), E_READ_PARCEL_ERROR);
    RegisterPowerModeCallback(callback);
    return ERR_OK;
}

int32_t PowerMgrStub::UnRegisterPowerModeCallbackStub(MessageParcel& data)
{
    sptr<IRemoteObject> obj = data.ReadRemoteObject();
    RETURN_IF_WITH_RET((obj == nullptr), E_READ_PARCEL_ERROR);
    sptr<IPowerModeCallback> callback = iface_cast<IPowerModeCallback>(obj);
    RETURN_IF_WITH_RET((callback == nullptr), E_READ_PARCEL_ERROR);
    UnRegisterPowerModeCallback(callback);
    return ERR_OK;
}

int32_t PowerMgrStub::RegisterRunningLockCallbackStub(MessageParcel& data)
{
    sptr<IRemoteObject> obj = data.ReadRemoteObject();
    RETURN_IF_WITH_RET((obj == nullptr), E_READ_PARCEL_ERROR);
    sptr<IPowerRunninglockCallback> callback = iface_cast<IPowerRunninglockCallback>(obj);
    RETURN_IF_WITH_RET((callback == nullptr), E_READ_PARCEL_ERROR);
    RegisterRunningLockCallback(callback);
    return ERR_OK;
}

int32_t PowerMgrStub::UnRegisterRunningLockCallbackStub(MessageParcel& data)
{
    sptr<IRemoteObject> obj = data.ReadRemoteObject();
    RETURN_IF_WITH_RET((obj == nullptr), E_READ_PARCEL_ERROR);
    sptr<IPowerRunninglockCallback> callback = iface_cast<IPowerRunninglockCallback>(obj);
    RETURN_IF_WITH_RET((callback == nullptr), E_READ_PARCEL_ERROR);
    UnRegisterRunningLockCallback(callback);
    return ERR_OK;
}

int32_t PowerMgrStub::RegisterScreenStateCallbackStub(MessageParcel& data)
{
    int32_t remainTime = 0;
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Int32, remainTime, E_READ_PARCEL_ERROR);

    sptr<IRemoteObject> obj = data.ReadRemoteObject();
    RETURN_IF_WITH_RET((obj == nullptr), E_READ_PARCEL_ERROR);
    sptr<IScreenOffPreCallback> callback = iface_cast<IScreenOffPreCallback>(obj);
    RETURN_IF_WITH_RET((callback == nullptr), E_READ_PARCEL_ERROR);
    RegisterScreenStateCallback(remainTime, callback);
    return ERR_OK;
}

int32_t PowerMgrStub::UnRegisterScreenStateCallbackStub(MessageParcel& data)
{
    sptr<IRemoteObject> obj = data.ReadRemoteObject();
    RETURN_IF_WITH_RET((obj == nullptr), E_READ_PARCEL_ERROR);
    sptr<IScreenOffPreCallback> callback = iface_cast<IScreenOffPreCallback>(obj);
    RETURN_IF_WITH_RET((callback == nullptr), E_READ_PARCEL_ERROR);
    UnRegisterScreenStateCallback(callback);
    return ERR_OK;
}

int32_t PowerMgrStub::SetDisplaySuspendStub(MessageParcel& data)
{
    bool enable = false;
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Bool, enable, E_READ_PARCEL_ERROR);
    SetDisplaySuspend(enable);
    return ERR_OK;
}

int32_t PowerMgrStub::HibernateStub(MessageParcel& data)
{
    bool clearMemory = false;
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Bool, clearMemory, E_READ_PARCEL_ERROR);
    Hibernate(clearMemory);
    return ERR_OK;
}

int32_t PowerMgrStub::SetDeviceModeStub(MessageParcel& data, MessageParcel& reply)
{
    uint32_t mode = 0;
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Uint32, mode, E_READ_PARCEL_ERROR);
    PowerErrors error = SetDeviceMode(static_cast<PowerMode>(mode));
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(reply, Int32, static_cast<int32_t>(error), E_WRITE_PARCEL_ERROR);
    return ERR_OK;
}

int32_t PowerMgrStub::GetDeviceModeStub(MessageParcel& reply)
{
    uint32_t ret = 0;
    ret = static_cast<uint32_t>(GetDeviceMode());
    if (!reply.WriteUint32(static_cast<uint32_t>(ret))) {
        POWER_HILOGE(FEATURE_POWER_MODE, "WriteUint32 fail");
        return E_WRITE_PARCEL_ERROR;
    }
    return ERR_OK;
}

int32_t PowerMgrStub::ShellDumpStub(MessageParcel& data, MessageParcel& reply)
{
    uint32_t argc;
    std::vector<std::string> args;

    if (!data.ReadUint32(argc)) {
        POWER_HILOGE(COMP_FWK, "ReadUint32 fail");
        return E_READ_PARCEL_ERROR;
    }

    if (argc >= PARAM_MAX_NUM) {
        POWER_HILOGW(COMP_FWK, "params exceed limit, argc=%{public}d", argc);
        return E_EXCEED_PARAM_LIMIT;
    }

    for (uint32_t i = 0; i < argc; i++) {
        std::string arg = data.ReadString();
        if (arg.empty()) {
            POWER_HILOGW(COMP_FWK, "read args fail, arg index=%{public}d", i);
            return E_READ_PARCEL_ERROR;
        }
        args.push_back(arg);
    }

    std::string ret = ShellDump(args, argc);
    if (!reply.WriteString(ret)) {
        POWER_HILOGE(COMP_FWK, "WriteString fail");
        return E_WRITE_PARCEL_ERROR;
    }
    return ERR_OK;
}

bool PowerMgrStub::IsShutdownCommand(uint32_t code)
{
    return (code >= static_cast<uint32_t>(PowerMgr::ShutdownClientInterfaceCode::CMD_START)) &&
        (code <= static_cast<uint32_t>(PowerMgr::ShutdownClientInterfaceCode::CMD_END));
}

int32_t PowerMgrStub::IsStandbyStub(MessageParcel& data, MessageParcel& reply)
{
    bool isStandby = false;
    PowerErrors ret = IsStandby(isStandby);
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(reply, Int32, static_cast<int32_t>(ret), E_WRITE_PARCEL_ERROR);
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(reply, Bool, isStandby, E_WRITE_PARCEL_ERROR);
    return ERR_OK;
}

int32_t PowerMgrStub::SetForceTimingOutStub(MessageParcel& data, MessageParcel& reply)
{
    bool enabled = false;
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Bool, enabled, E_READ_PARCEL_ERROR);
    PowerErrors ret = SetForceTimingOut(enabled);
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(reply, Int32, static_cast<int32_t>(ret), E_WRITE_PARCEL_ERROR);
    return ERR_OK;
}

int32_t PowerMgrStub::LockScreenAfterTimingOutStub(MessageParcel& data, MessageParcel& reply)
{
    bool enabledLockScreen = true;
    bool checkLock = false;
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Bool, enabledLockScreen, E_READ_PARCEL_ERROR);
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Bool, checkLock, E_READ_PARCEL_ERROR);
    PowerErrors ret = LockScreenAfterTimingOut(enabledLockScreen, checkLock);
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(reply, Int32, static_cast<int32_t>(ret), E_WRITE_PARCEL_ERROR);
    return ERR_OK;
}
} // namespace PowerMgr
} // namespace OHOS
