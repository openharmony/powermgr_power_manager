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

#include "power_mgr_stub.h"

#include <message_parcel.h>
#include <string_ex.h>

#include "power_common.h"
#include "power_mgr_proxy.h"

using namespace OHOS::HiviewDFX;

namespace OHOS {
namespace PowerMgr {
int PowerMgrStub::OnRemoteRequest(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    POWER_HILOGD(MODULE_SERVICE,
        "PowerMgrStub::OnRemoteRequest, cmd = %{public}u, flags= %{public}d", code, option.GetFlags());
    std::u16string descripter = PowerMgrStub::GetDescriptor();
    std::u16string remoteDescripter = data.ReadInterfaceToken();
    if (descripter != remoteDescripter) {
        POWER_HILOGE(MODULE_SERVICE,
            "PowerMgrStub::OnRemoteRequest failed, descriptor is not matched!");
        return E_GET_POWER_SERVICE_FAILED;
    }

    switch (code) {
        case static_cast<int>(IPowerMgr::WAKEUP_DEVICE): {
            return WakeupDeviceStub(data);
        }
        case static_cast<int>(IPowerMgr::SUSPEND_DEVICE): {
            return SuspendDeviceStub(data);
        }
        case static_cast<int>(IPowerMgr::REFRESH_ACTIVITY): {
            return RefreshActivityStub(data);
        }
        case static_cast<int>(IPowerMgr::REBOOT_DEVICE): {
            return RebootDeviceStub(data);
        }
        case static_cast<int>(IPowerMgr::SHUTDOWN_DEVICE): {
            return ShutDownDeviceStub(data);
        }
        case static_cast<int>(IPowerMgr::GET_STATE): {
            return GetStateStub(reply);
        }
        case static_cast<int>(IPowerMgr::IS_SCREEN_ON): {
            return IsScreeOnStub(reply);
        }
        case static_cast<int>(IPowerMgr::FORCE_DEVICE_SUSPEND): {
            return ForceSuspendDeviceStub(data, reply);
        }
        case static_cast<int>(IPowerMgr::CREATE_RUNNINGLOCK): {
            return CreateRunningLockStub(data);
        }
        case static_cast<int>(IPowerMgr::RELEASE_RUNNINGLOCK): {
            return ReleaseRunningLockStub(data);
        }
        case static_cast<int>(IPowerMgr::IS_RUNNINGLOCK_TYPE_SUPPORTED): {
            return IsRunningLockTypeSupportedStub(data);
        }
        case static_cast<int>(IPowerMgr::RUNNINGLOCK_LOCK): {
            return LockStub(data);
        }
        case static_cast<int>(IPowerMgr::RUNNINGLOCK_UNLOCK): {
            return UnLockStub(data);
        }
        case static_cast<int>(IPowerMgr::RUNNINGLOCK_SET_WORK_TRIGGER_LIST): {
            return SetWorkTriggerListStub(data);
        }
        case static_cast<int>(IPowerMgr::PROXY_RUNNINGLOCK): {
            return ProxyRunningLockStub(data);
        }
        case static_cast<int>(IPowerMgr::RUNNINGLOCK_ISUSED): {
            return IsUsedStub(data, reply);
        }
        case static_cast<int>(IPowerMgr::REG_POWER_STATE_CALLBACK): {
            return RegisterPowerStateCallbackStub(data);
        }
        case static_cast<int>(IPowerMgr::UNREG_POWER_STATE_CALLBACK): {
            return UnRegisterPowerStateCallbackStub(data);
        }
        case static_cast<int>(IPowerMgr::REG_SHUTDOWN_CALLBACK): {
            return RegisterShutdownCallbackStub(data);
        }
        case static_cast<int>(IPowerMgr::UNREG_SHUTDOWN_CALLBACK): {
            return UnRegisterShutdownCallbackStub(data);
        }
        case static_cast<int>(IPowerMgr::REG_POWER_MODE_CALLBACK): {
            return RegisterPowerModeCallbackStub(data);
        }
        case static_cast<int>(IPowerMgr::UNREG_POWER_MODE_CALLBACK): {
            return UnRegisterPowerModeCallbackStub(data);
        }
        case static_cast<int>(IPowerMgr::SET_DISPLAY_SUSPEND): {
            return SetDisplaySuspendStub(data);
        }
        case static_cast<int>(IPowerMgr::SETMODE_DEVICE): {
            return SetDeviceModeStub(data);
        }
        case static_cast<int>(IPowerMgr::GETMODE_DEVICE): {
            return GetDeviceModeStub(reply);
        }
        case static_cast<int>(IPowerMgr::SHELL_DUMP): {
            return ShellDumpStub(data, reply);
        }
        default: {
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
        }
    }
    return ERR_OK;
}

int32_t PowerMgrStub::CreateRunningLockStub(MessageParcel& data)
{
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    RETURN_IF_WITH_RET((token == nullptr), E_READ_PARCEL_ERROR);
    std::unique_ptr<RunningLockInfo> runningLockInfo(data.ReadParcelable<RunningLockInfo>());
    RETURN_IF_WITH_RET((runningLockInfo == nullptr), E_READ_PARCEL_ERROR);
    CreateRunningLock(token, *runningLockInfo);
    return ERR_OK;
}

int32_t PowerMgrStub::ReleaseRunningLockStub(MessageParcel& data)
{
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    RETURN_IF_WITH_RET((token == nullptr), E_READ_PARCEL_ERROR);
    ReleaseRunningLock(token);
    return ERR_OK;
}

int32_t PowerMgrStub::IsRunningLockTypeSupportedStub(MessageParcel& data)
{
    uint32_t type = 0;
    READ_PARCEL_WITH_RET(data, Uint32, type, E_READ_PARCEL_ERROR);
    IsRunningLockTypeSupported(type);
    return ERR_OK;
}

int32_t PowerMgrStub::LockStub(MessageParcel& data)
{
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    RETURN_IF_WITH_RET((token == nullptr), E_READ_PARCEL_ERROR);
    std::unique_ptr<RunningLockInfo> runningLockInfo(data.ReadParcelable<RunningLockInfo>());
    RETURN_IF_WITH_RET((runningLockInfo == nullptr), E_READ_PARCEL_ERROR);
    uint32_t timeOutMs = 0;
    READ_PARCEL_WITH_RET(data, Uint32, timeOutMs, E_READ_PARCEL_ERROR);
    Lock(token, *runningLockInfo, timeOutMs);
    return ERR_OK;
}

int32_t PowerMgrStub::UnLockStub(MessageParcel& data)
{
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    RETURN_IF_WITH_RET((token == nullptr), E_READ_PARCEL_ERROR);
    UnLock(token);
    return ERR_OK;
}

int32_t PowerMgrStub::IsUsedStub(MessageParcel& data, MessageParcel& reply)
{
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    RETURN_IF_WITH_RET((token == nullptr), E_READ_PARCEL_ERROR);
    bool ret = IsUsed(token);
    WRITE_PARCEL_WITH_RET(reply, Bool, ret, E_WRITE_PARCEL_ERROR);
    return ERR_OK;
}

int32_t PowerMgrStub::SetWorkTriggerListStub(MessageParcel& data)
{
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    RETURN_IF_WITH_RET((token == nullptr), E_READ_PARCEL_ERROR);
    WorkTriggerList workTriggerList;
    RunningLockInfo::ReadFromParcelWorkTriggerList(data, workTriggerList);
    SetWorkTriggerList(token, workTriggerList);
    return ERR_OK;
}

int32_t PowerMgrStub::ProxyRunningLockStub(MessageParcel& data)
{
    bool proxyLock = false;
    pid_t uid;
    pid_t pid;
    READ_PARCEL_WITH_RET(data, Bool, proxyLock, E_READ_PARCEL_ERROR);
    READ_PARCEL_WITH_RET(data, Int32, uid, E_READ_PARCEL_ERROR);
    READ_PARCEL_WITH_RET(data, Int32, pid, E_READ_PARCEL_ERROR);
    ProxyRunningLock(proxyLock, uid, pid);
    return ERR_OK;
}

int32_t PowerMgrStub::RebootDeviceStub(MessageParcel& data)
{
    std::string reason = Str16ToStr8(data.ReadString16());
    RebootDevice(reason);
    return ERR_OK;
}

int32_t PowerMgrStub::ShutDownDeviceStub(MessageParcel& data)
{
    std::string reason = Str16ToStr8(data.ReadString16());
    ShutDownDevice(reason);
    return ERR_OK;
}

int32_t PowerMgrStub::WakeupDeviceStub(MessageParcel& data)
{
    int64_t time = 0;
    uint32_t reason = 0;

    READ_PARCEL_WITH_RET(data, Int64, time, E_READ_PARCEL_ERROR);
    READ_PARCEL_WITH_RET(data, Uint32, reason, E_READ_PARCEL_ERROR);
    std::string details = Str16ToStr8(data.ReadString16());

    WakeupDevice(time, static_cast<WakeupDeviceType>(reason), details);
    return ERR_OK;
}

int32_t PowerMgrStub::SuspendDeviceStub(MessageParcel& data)
{
    int64_t time = 0;
    uint32_t reason = 0;
    bool suspendImmed = true;

    READ_PARCEL_WITH_RET(data, Int64, time, E_READ_PARCEL_ERROR);
    READ_PARCEL_WITH_RET(data, Uint32, reason, E_READ_PARCEL_ERROR);
    READ_PARCEL_WITH_RET(data, Bool, suspendImmed, E_READ_PARCEL_ERROR);

    SuspendDevice(time, static_cast<SuspendDeviceType>(reason), suspendImmed);
    return ERR_OK;
}

int32_t PowerMgrStub::RefreshActivityStub(MessageParcel& data)
{
    int64_t time = 0;
    uint32_t type = 0;
    bool needChangeBacklight = true;

    READ_PARCEL_WITH_RET(data, Int64, time, E_READ_PARCEL_ERROR);
    READ_PARCEL_WITH_RET(data, Uint32, type, E_READ_PARCEL_ERROR);
    READ_PARCEL_WITH_RET(data, Bool, needChangeBacklight, E_READ_PARCEL_ERROR);

    RefreshActivity(time, static_cast<UserActivityType>(type), needChangeBacklight);
    return ERR_OK;
}

int32_t PowerMgrStub::ForceSuspendDeviceStub(MessageParcel& data, MessageParcel& reply)
{
    bool ret = false;
    int64_t time = 0;

    READ_PARCEL_WITH_RET(data, Int64, time, E_READ_PARCEL_ERROR);

    ret = ForceSuspendDevice(time);
    if (!reply.WriteBool(ret)) {
        POWER_HILOGE(MODULE_SERVICE, "PowerMgrStub:: ForceSuspendDevice Writeback Fail!");
        return E_WRITE_PARCEL_ERROR;
    }
    return ERR_OK;
}

int32_t PowerMgrStub::GetStateStub(MessageParcel& reply)
{
    PowerState ret = GetState();
    if (!reply.WriteUint32(static_cast<uint32_t>(ret))) {
        POWER_HILOGE(MODULE_SERVICE, "PowerMgrStub:: GetStateStub Writeback Fail!");
        return E_WRITE_PARCEL_ERROR;
    }
    return ERR_OK;
}

int32_t PowerMgrStub::IsScreeOnStub(MessageParcel& reply)
{
    bool ret = false;
    ret = IsScreenOn();
    if (!reply.WriteBool(ret)) {
        POWER_HILOGE(MODULE_SERVICE, "PowerMgrStub:: IsScreenOn Writeback Fail!");
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

int32_t PowerMgrStub::RegisterShutdownCallbackStub(MessageParcel& data)
{
    uint32_t priority;
    READ_PARCEL_WITH_RET(data, Uint32, priority, E_READ_PARCEL_ERROR);
    sptr<IRemoteObject> obj = data.ReadRemoteObject();
    RETURN_IF_WITH_RET((obj == nullptr), E_READ_PARCEL_ERROR);
    sptr<IShutdownCallback> callback = iface_cast<IShutdownCallback>(obj);
    RETURN_IF_WITH_RET((callback == nullptr), E_READ_PARCEL_ERROR);
    RegisterShutdownCallback(static_cast<IShutdownCallback::ShutdownPriority>(priority), callback);
    return ERR_OK;
}

int32_t PowerMgrStub::UnRegisterShutdownCallbackStub(MessageParcel& data)
{
    sptr<IRemoteObject> obj = data.ReadRemoteObject();
    RETURN_IF_WITH_RET((obj == nullptr), E_READ_PARCEL_ERROR);
    sptr<IShutdownCallback> callback = iface_cast<IShutdownCallback>(obj);
    RETURN_IF_WITH_RET((callback == nullptr), E_READ_PARCEL_ERROR);
    UnRegisterShutdownCallback(callback);
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

int32_t PowerMgrStub::SetDisplaySuspendStub(MessageParcel& data)
{
    bool enable = false;
    READ_PARCEL_WITH_RET(data, Bool, enable, E_READ_PARCEL_ERROR);
    SetDisplaySuspend(enable);
    return ERR_OK;
}

int32_t PowerMgrStub::SetDeviceModeStub(MessageParcel& data)
{
    uint32_t mode = 0;
    READ_PARCEL_WITH_RET(data, Uint32, mode, E_READ_PARCEL_ERROR);
    SetDeviceMode(mode);
    return ERR_OK;
}

int32_t PowerMgrStub::GetDeviceModeStub(MessageParcel& reply)
{
    uint32_t ret = 0;
    ret = GetDeviceMode();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrStub::GetDeviceModeStub, cmd = %{public}u.", ret);
    if (!reply.WriteUint32(static_cast<uint32_t>(ret))) {
        POWER_HILOGE(MODULE_SERVICE, "PowerMgrStub:: Get device mode Fail!");
        return E_WRITE_PARCEL_ERROR;
    }
    return ERR_OK;
}

int32_t PowerMgrStub::ShellDumpStub(MessageParcel& data, MessageParcel& reply)
{
    uint32_t argc;
    std::vector<std::string> args;

    if (!data.ReadUint32(argc)) {
        POWER_HILOGE(MODULE_INNERKIT, "Readback fail!");
        return E_READ_PARCEL_ERROR;
    }

    for (uint32_t i = 0; i < argc; i++) {
        std::string arg = data.ReadString();
        if (!arg.empty()) {
            args.push_back(arg);
        } else {
            POWER_HILOGE(MODULE_INNERKIT, "read value fail: %{public}d", i);
        }
    }

    std::string ret = ShellDump(args, argc);
    if (!reply.WriteString(ret)) {
        POWER_HILOGE(MODULE_SERVICE, "PowerMgrStub:: Dump Writeback Fail!");
        return E_WRITE_PARCEL_ERROR;
    }
    return ERR_OK;
}
} // namespace PowerMgr
} // namespace OHOS
