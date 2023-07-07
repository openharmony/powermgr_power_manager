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
std::unique_ptr<ShutdownStubDelegator> g_shutdownDelegator;
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
        if (g_shutdownDelegator == nullptr) {
            g_shutdownDelegator = std::make_unique<ShutdownStubDelegator>(*this);
        }
        RETURN_IF_WITH_RET(g_shutdownDelegator == nullptr, ERR_NO_INIT)

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
            ret = IsScreeOnStub(reply);
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
        case static_cast<int>(PowerMgr::PowerMgrInterfaceCode::PROXY_RUNNINGLOCK):
            ret = ProxyRunningLockStub(data, reply);
            break;
        case static_cast<int>(PowerMgr::PowerMgrInterfaceCode::RUNNINGLOCK_ISUSED):
            ret = IsUsedStub(data, reply);
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
        case static_cast<int>(PowerMgr::PowerMgrInterfaceCode::SETMODE_DEVICE):
            ret = SetDeviceModeStub(data, reply);
            break;
        case static_cast<int>(PowerMgr::PowerMgrInterfaceCode::GETMODE_DEVICE):
            ret = GetDeviceModeStub(reply);
            break;
        case static_cast<int>(PowerMgr::PowerMgrInterfaceCode::SHELL_DUMP):
            ret = ShellDumpStub(data, reply);
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
    WRITE_PARCEL_WITH_RET(reply, Int32, static_cast<int32_t>(error), E_WRITE_PARCEL_ERROR);
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
    READ_PARCEL_WITH_RET(data, Uint32, type, E_READ_PARCEL_ERROR);
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
    READ_PARCEL_WITH_RET(data, Int32, timeOutMs, E_READ_PARCEL_ERROR);
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

int32_t PowerMgrStub::IsUsedStub(MessageParcel& data, MessageParcel& reply)
{
    sptr<IRemoteObject> remoteObj = data.ReadRemoteObject();
    RETURN_IF_WITH_RET((remoteObj == nullptr), E_READ_PARCEL_ERROR);
    bool ret = IsUsed(remoteObj);
    WRITE_PARCEL_WITH_RET(reply, Bool, ret, E_WRITE_PARCEL_ERROR);
    return ERR_OK;
}

int32_t PowerMgrStub::ProxyRunningLockStub(MessageParcel& data, MessageParcel& reply)
{
    bool isProxied = false;
    pid_t uid;
    pid_t pid;
    READ_PARCEL_WITH_RET(data, Bool, isProxied, E_READ_PARCEL_ERROR);
    READ_PARCEL_WITH_RET(data, Int32, pid, E_READ_PARCEL_ERROR);
    READ_PARCEL_WITH_RET(data, Int32, uid, E_READ_PARCEL_ERROR);
    bool ret = ProxyRunningLock(isProxied, pid, uid);
    WRITE_PARCEL_WITH_RET(reply, Bool, ret, E_WRITE_PARCEL_ERROR);
    return ERR_OK;
}

int32_t PowerMgrStub::RebootDeviceStub(MessageParcel& data, MessageParcel& reply)
{
    std::string reason = Str16ToStr8(data.ReadString16());
    PowerErrors error = RebootDevice(reason);
    WRITE_PARCEL_WITH_RET(reply, Int32, static_cast<int32_t>(error), E_WRITE_PARCEL_ERROR);
    return ERR_OK;
}

int32_t PowerMgrStub::RebootDeviceForDeprecatedStub(MessageParcel& data, MessageParcel& reply)
{
    std::string reason = Str16ToStr8(data.ReadString16());
    PowerErrors error = RebootDeviceForDeprecated(reason);
    WRITE_PARCEL_WITH_RET(reply, Int32, static_cast<int32_t>(error), E_WRITE_PARCEL_ERROR);
    return ERR_OK;
}

int32_t PowerMgrStub::ShutDownDeviceStub(MessageParcel& data, MessageParcel& reply)
{
    std::string reason = Str16ToStr8(data.ReadString16());
    PowerErrors error = ShutDownDevice(reason);
    WRITE_PARCEL_WITH_RET(reply, Int32, static_cast<int32_t>(error), E_WRITE_PARCEL_ERROR);
    return ERR_OK;
}

int32_t PowerMgrStub::WakeupDeviceStub(MessageParcel& data, MessageParcel& reply)
{
    int64_t time = 0;
    uint32_t reason = 0;

    READ_PARCEL_WITH_RET(data, Int64, time, E_READ_PARCEL_ERROR);
    READ_PARCEL_WITH_RET(data, Uint32, reason, E_READ_PARCEL_ERROR);
    std::string details = Str16ToStr8(data.ReadString16());

    PowerErrors error = WakeupDevice(time, static_cast<WakeupDeviceType>(reason), details);
    WRITE_PARCEL_WITH_RET(reply, Int32, static_cast<int32_t>(error), E_WRITE_PARCEL_ERROR);
    return ERR_OK;
}

int32_t PowerMgrStub::SuspendDeviceStub(MessageParcel& data, MessageParcel& reply)
{
    int64_t time = 0;
    uint32_t reason = 0;
    bool suspendImmed = true;

    READ_PARCEL_WITH_RET(data, Int64, time, E_READ_PARCEL_ERROR);
    READ_PARCEL_WITH_RET(data, Uint32, reason, E_READ_PARCEL_ERROR);
    READ_PARCEL_WITH_RET(data, Bool, suspendImmed, E_READ_PARCEL_ERROR);

    PowerErrors error = SuspendDevice(time, static_cast<SuspendDeviceType>(reason), suspendImmed);
    WRITE_PARCEL_WITH_RET(reply, Int32, static_cast<int32_t>(error), E_WRITE_PARCEL_ERROR);
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

int32_t PowerMgrStub::OverrideScreenOffTimeStub(MessageParcel& data, MessageParcel& reply)
{
    int64_t timeout = 0;

    READ_PARCEL_WITH_RET(data, Int64, timeout, E_READ_PARCEL_ERROR);

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

    READ_PARCEL_WITH_RET(data, Int64, time, E_READ_PARCEL_ERROR);

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

int32_t PowerMgrStub::IsScreeOnStub(MessageParcel& reply)
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

int32_t PowerMgrStub::SetDeviceModeStub(MessageParcel& data, MessageParcel& reply)
{
    uint32_t mode = 0;
    READ_PARCEL_WITH_RET(data, Uint32, mode, E_READ_PARCEL_ERROR);
    PowerErrors error = SetDeviceMode(static_cast<PowerMode>(mode));
    WRITE_PARCEL_WITH_RET(reply, Int32, static_cast<int32_t>(error), E_WRITE_PARCEL_ERROR);
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
    return (code >= static_cast<int32_t>(PowerMgr::ShutdownClientInterfaceCode::CMD_START)) &&
        (code <= static_cast<int32_t>(PowerMgr::ShutdownClientInterfaceCode::CMD_END));
}
} // namespace PowerMgr
} // namespace OHOS
