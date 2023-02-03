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

#include "power_mgr_proxy.h"

#include <message_parcel.h>
#include <string_ex.h>
#include "errors.h"
#include "message_option.h"
#include "power_log.h"
#include "power_common.h"

namespace OHOS {
namespace PowerMgr {
PowerErrors PowerMgrProxy::CreateRunningLock(const sptr<IRemoteObject>& remoteObj,
    const RunningLockInfo& runningLockInfo)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, PowerErrors::ERR_CONNECTION_FAIL);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Write descriptor failed");
        return PowerErrors::ERR_CONNECTION_FAIL;
    }

    WRITE_PARCEL_WITH_RET(data, RemoteObject, remoteObj.GetRefPtr(), PowerErrors::ERR_CONNECTION_FAIL);
    WRITE_PARCEL_WITH_RET(data, Parcelable, &runningLockInfo, PowerErrors::ERR_CONNECTION_FAIL);

    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::CREATE_RUNNINGLOCK),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "SendRequest is failed, ret: %{public}d", ret);
        return PowerErrors::ERR_CONNECTION_FAIL;
    }
    int32_t error;
    READ_PARCEL_WITH_RET(reply, Int32, error, PowerErrors::ERR_OK);
    return static_cast<PowerErrors>(error);
}

bool PowerMgrProxy::ReleaseRunningLock(const sptr<IRemoteObject>& remoteObj)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, false);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Write descriptor failed");
        return false;
    }

    WRITE_PARCEL_WITH_RET(data, RemoteObject, remoteObj.GetRefPtr(), false);

    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::RELEASE_RUNNINGLOCK),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "SendRequest is failed, ret: %{public}d", ret);
        return false;
    }
    return true;
}

bool PowerMgrProxy::IsRunningLockTypeSupported(uint32_t type)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, false);

    bool result = false;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Write descriptor failed");
        return result;
    }

    WRITE_PARCEL_WITH_RET(data, Uint32, static_cast<uint32_t>(type), false);
    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::IS_RUNNINGLOCK_TYPE_SUPPORTED), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "SendRequest is failed, ret: %{public}d", ret);
        return result;
    }

    if (!reply.ReadBool(result)) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "ReadBool fail");
    }

    return result;
}

bool PowerMgrProxy::Lock(const sptr<IRemoteObject>& remoteObj, const RunningLockInfo& runningLockInfo,
    uint32_t timeOutMs)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, false);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Write descriptor failed");
        return false;
    }

    WRITE_PARCEL_WITH_RET(data, RemoteObject, remoteObj.GetRefPtr(), false);
    WRITE_PARCEL_WITH_RET(data, Parcelable, &runningLockInfo, false);
    WRITE_PARCEL_WITH_RET(data, Uint32, timeOutMs, false);

    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::RUNNINGLOCK_LOCK),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "SendRequest is failed, ret: %{public}d", ret);
        return false;
    }
    return true;
}

bool PowerMgrProxy::UnLock(const sptr<IRemoteObject>& remoteObj)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, false);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Write descriptor failed");
        return false;
    }

    WRITE_PARCEL_WITH_RET(data, RemoteObject, remoteObj.GetRefPtr(), false);

    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::RUNNINGLOCK_UNLOCK),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "SendRequest is failed, ret: %{public}d", ret);
        return false;
    }
    return true;
}

bool PowerMgrProxy::IsUsed(const sptr<IRemoteObject>& remoteObj)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, false);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Write descriptor failed");
        return false;
    }

    WRITE_PARCEL_WITH_RET(data, RemoteObject, remoteObj.GetRefPtr(), false);
    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::RUNNINGLOCK_ISUSED),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "SendRequest is failed, ret: %{public}d", ret);
        return false;
    }
    bool used = false;
    READ_PARCEL_WITH_RET(reply, Bool, used, false);
    return used;
}

bool PowerMgrProxy::SetWorkTriggerList(const sptr<IRemoteObject>& remoteObj, const WorkTriggerList& workTriggerList)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, false);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Write descriptor failed");
        return false;
    }

    WRITE_PARCEL_WITH_RET(data, RemoteObject, remoteObj.GetRefPtr(), false);
    RETURN_IF_WITH_RET(!RunningLockInfo::MarshallingWorkTriggerList(data, workTriggerList), false);
    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::RUNNINGLOCK_SET_WORK_TRIGGER_LIST),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "SendRequest is failed, ret: %{public}d", ret);
        return false;
    }
    return true;
}

bool PowerMgrProxy::ProxyRunningLock(bool proxyLock, pid_t uid, pid_t pid)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, false);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Write descriptor failed");
        return false;
    }

    WRITE_PARCEL_WITH_RET(data, Bool, proxyLock, false);
    WRITE_PARCEL_WITH_RET(data, Int32, uid, false);
    WRITE_PARCEL_WITH_RET(data, Int32, pid, false);
    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::PROXY_RUNNINGLOCK),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "SendRequest is failed, ret: %{public}d", ret);
        return false;
    }
    return true;
}

PowerErrors PowerMgrProxy::RebootDevice(const std::string& reason)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, PowerErrors::ERR_CONNECTION_FAIL);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_SHUTDOWN, "Write descriptor failed");
        return PowerErrors::ERR_CONNECTION_FAIL;
    }

    WRITE_PARCEL_WITH_RET(data, String16, Str8ToStr16(reason), PowerErrors::ERR_CONNECTION_FAIL);

    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::REBOOT_DEVICE), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_SHUTDOWN, "SendRequest is failed, ret: %{public}d", ret);
        return PowerErrors::ERR_CONNECTION_FAIL;
    }
    int32_t error;
    READ_PARCEL_WITH_RET(reply, Int32, error, PowerErrors::ERR_OK);
    return static_cast<PowerErrors>(error);
}

PowerErrors PowerMgrProxy::RebootDeviceForDeprecated(const std::string& reason)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, PowerErrors::ERR_CONNECTION_FAIL);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_SHUTDOWN, "Write descriptor failed");
        return PowerErrors::ERR_CONNECTION_FAIL;
    }

    WRITE_PARCEL_WITH_RET(data, String16, Str8ToStr16(reason), PowerErrors::ERR_CONNECTION_FAIL);

    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::REBOOT_DEVICE_FOR_DEPRECATED), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_SHUTDOWN, "SendRequest is failed, ret: %{public}d", ret);
        return PowerErrors::ERR_CONNECTION_FAIL;
    }
    int32_t error;
    READ_PARCEL_WITH_RET(reply, Int32, error, PowerErrors::ERR_OK);
    return static_cast<PowerErrors>(error);
}

PowerErrors PowerMgrProxy::ShutDownDevice(const std::string& reason)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, PowerErrors::ERR_CONNECTION_FAIL);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_SHUTDOWN, "Write descriptor failed");
        return PowerErrors::ERR_CONNECTION_FAIL;
    }

    WRITE_PARCEL_WITH_RET(data, String16, Str8ToStr16(reason), PowerErrors::ERR_CONNECTION_FAIL);

    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::SHUTDOWN_DEVICE), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_SHUTDOWN, "SendRequest is failed, ret: %{public}d", ret);
        return PowerErrors::ERR_CONNECTION_FAIL;
    }
    int32_t error;
    READ_PARCEL_WITH_RET(reply, Int32, error, PowerErrors::ERR_OK);
    return static_cast<PowerErrors>(error);
}

PowerErrors PowerMgrProxy::SuspendDevice(int64_t callTimeMs, SuspendDeviceType reason, bool suspendImmed)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, PowerErrors::ERR_CONNECTION_FAIL);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_SUSPEND, "Write descriptor failed");
        return PowerErrors::ERR_CONNECTION_FAIL;
    }

    WRITE_PARCEL_WITH_RET(data, Int64, callTimeMs, PowerErrors::ERR_CONNECTION_FAIL);
    WRITE_PARCEL_WITH_RET(data, Uint32, static_cast<uint32_t>(reason), PowerErrors::ERR_CONNECTION_FAIL);
    WRITE_PARCEL_WITH_RET(data, Bool, suspendImmed, PowerErrors::ERR_CONNECTION_FAIL);

    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::SUSPEND_DEVICE), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_SUSPEND, "SendRequest is failed, ret: %{public}d", ret);
        return PowerErrors::ERR_CONNECTION_FAIL;
    }
    int32_t error;
    READ_PARCEL_WITH_RET(reply, Int32, error, PowerErrors::ERR_OK);
    return static_cast<PowerErrors>(error);
}

PowerErrors PowerMgrProxy::WakeupDevice(int64_t callTimeMs, WakeupDeviceType reason, const std::string& details)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, PowerErrors::ERR_CONNECTION_FAIL);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_WAKEUP, "Write descriptor failed");
        return PowerErrors::ERR_CONNECTION_FAIL;
    }

    WRITE_PARCEL_WITH_RET(data, Int64, callTimeMs, PowerErrors::ERR_CONNECTION_FAIL);
    WRITE_PARCEL_WITH_RET(data, Uint32, static_cast<uint32_t>(reason), PowerErrors::ERR_CONNECTION_FAIL);
    WRITE_PARCEL_WITH_RET(data, String16, Str8ToStr16(details), PowerErrors::ERR_CONNECTION_FAIL);

    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::WAKEUP_DEVICE), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_WAKEUP, "SendRequest is failed, ret: %{public}d", ret);
        return PowerErrors::ERR_CONNECTION_FAIL;
    }
    int32_t error;
    READ_PARCEL_WITH_RET(reply, Int32, error, PowerErrors::ERR_OK);
    return static_cast<PowerErrors>(error);
}

bool PowerMgrProxy::RefreshActivity(int64_t callTimeMs, UserActivityType type, bool needChangeBacklight)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, false);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_ACTIVITY, "Write descriptor failed");
        return false;
    }

    WRITE_PARCEL_WITH_RET(data, Int64, callTimeMs, false);
    WRITE_PARCEL_WITH_RET(data, Uint32, static_cast<uint32_t>(type), false);
    WRITE_PARCEL_WITH_RET(data, Bool, needChangeBacklight, false);

    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::REFRESH_ACTIVITY), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_ACTIVITY, "SendRequest is failed, ret: %{public}d", ret);
        return false;
    }
    return true;
}

bool PowerMgrProxy::OverrideScreenOffTime(int64_t timeout)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, false);

    bool result = false;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(COMP_SVC, "Write descriptor failed");
        return result;
    }

    WRITE_PARCEL_WITH_RET(data, Int64, timeout, false);

    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::OVERRIDE_DISPLAY_OFF_TIME), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(COMP_SVC, "SendRequest is failed, ret: %{public}d", ret);
        return result;
    }
    if (!reply.ReadBool(result)) {
        POWER_HILOGE(COMP_SVC, "ReadBool fail");
    }

    return result;
}

bool PowerMgrProxy::RestoreScreenOffTime()
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, false);

    bool result = false;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(COMP_FWK, "Write descriptor failed");
        return result;
    }

    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::RESTORE_DISPLAY_OFF_TIME), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(COMP_FWK, "SendRequest is failed, ret: %{public}d", ret);
        return result;
    }
    if (!reply.ReadBool(result)) {
        POWER_HILOGE(COMP_FWK, "ReadBool fail");
    }

    return result;
}

bool PowerMgrProxy::ForceSuspendDevice(int64_t callTimeMs)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, false);

    bool result = false;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_SUSPEND, "Write descriptor failed");
        return result;
    }

    WRITE_PARCEL_WITH_RET(data, Int64, callTimeMs, false);

    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::FORCE_DEVICE_SUSPEND), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_SUSPEND, "SendRequest is failed, ret: %{public}d", ret);
        return result;
    }
    if (!reply.ReadBool(result)) {
        POWER_HILOGE(FEATURE_SUSPEND, "ReadBool fail");
    }

    return result;
}

PowerState PowerMgrProxy::GetState()
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, PowerState::UNKNOWN);

    uint32_t result = 0;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_POWER_STATE, "Write descriptor failed");
        return PowerState::UNKNOWN;
    }

    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::GET_STATE), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_POWER_STATE, "SendRequest is failed, ret: %{public}d", ret);
        return PowerState::UNKNOWN;
    }
    if (!reply.ReadUint32(result)) {
        POWER_HILOGE(FEATURE_POWER_STATE, "ReadUint32 failed");
        return PowerState::UNKNOWN;
    }

    return static_cast<PowerState>(result);
}

bool PowerMgrProxy::IsScreenOn()
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, false);

    bool result = false;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(COMP_FWK, "Write descriptor failed");
        return result;
    }

    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::IS_SCREEN_ON), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(COMP_FWK, "SendRequest is failed, ret: %{public}d", ret);
        return result;
    }
    if (!reply.ReadBool(result)) {
        POWER_HILOGE(COMP_FWK, "ReadBool fail");
    }

    return result;
}

bool PowerMgrProxy::RegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET((remote == nullptr) || (callback == nullptr), false);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_POWER_STATE, "Write descriptor failed");
        return false;
    }

    WRITE_PARCEL_WITH_RET(data, RemoteObject, callback->AsObject(), false);

    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::REG_POWER_STATE_CALLBACK), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_POWER_STATE, "SendRequest is failed, ret: %{public}d", ret);
        return false;
    }
    return true;
}

bool PowerMgrProxy::UnRegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET((remote == nullptr) || (callback == nullptr), false);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_POWER_STATE, "Write descriptor failed");
        return false;
    }

    WRITE_PARCEL_WITH_RET(data, RemoteObject, callback->AsObject(), false);

    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::UNREG_POWER_STATE_CALLBACK), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_POWER_STATE, "SendRequest is failed, ret: %{public}d", ret);
        return false;
    }
    return true;
}

bool PowerMgrProxy::RegisterShutdownCallback(IShutdownCallback::ShutdownPriority priority,
    const sptr<IShutdownCallback>& callback)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET((remote == nullptr) || (callback == nullptr), false);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_SHUTDOWN, "Write descriptor failed");
        return false;
    }

    WRITE_PARCEL_WITH_RET(data, Uint32, static_cast<uint32_t>(priority), false);
    WRITE_PARCEL_WITH_RET(data, RemoteObject, callback->AsObject(), false);

    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::REG_SHUTDOWN_CALLBACK), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_SHUTDOWN, "SendRequest is failed, ret: %{public}d", ret);
        return false;
    }
    return true;
}

bool PowerMgrProxy::UnRegisterShutdownCallback(const sptr<IShutdownCallback>& callback)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET((remote == nullptr) || (callback == nullptr), false);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_SHUTDOWN, "Write descriptor failed");
        return false;
    }

    WRITE_PARCEL_WITH_RET(data, RemoteObject, callback->AsObject(), false);

    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::UNREG_SHUTDOWN_CALLBACK), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_SHUTDOWN, "SendRequest is failed, ret: %{public}d", ret);
        return false;
    }
    return true;
}

bool PowerMgrProxy::RegisterPowerModeCallback(const sptr<IPowerModeCallback>& callback)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET((remote == nullptr) || (callback == nullptr), false);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_POWER_MODE, "Write descriptor failed");
        return false;
    }

    WRITE_PARCEL_WITH_RET(data, RemoteObject, callback->AsObject(), false);

    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::REG_POWER_MODE_CALLBACK), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_POWER_MODE, "SendRequest is failed, ret: %{public}d", ret);
        return false;
    }
    return true;
}

bool PowerMgrProxy::UnRegisterPowerModeCallback(const sptr<IPowerModeCallback>& callback)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET((remote == nullptr) || (callback == nullptr), false);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_POWER_MODE, "Write descriptor failed");
        return false;
    }

    WRITE_PARCEL_WITH_RET(data, RemoteObject, callback->AsObject(), false);

    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::UNREG_POWER_MODE_CALLBACK), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_POWER_MODE, "SendRequest is failed, ret: %{public}d", ret);
        return false;
    }
    return true;
}

bool PowerMgrProxy::SetDisplaySuspend(bool enable)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, false);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_SUSPEND, "Write descriptor failed");
        return false;
    }

    WRITE_PARCEL_WITH_RET(data, Bool, enable, false);

    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::SET_DISPLAY_SUSPEND), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_SUSPEND, "SendRequest is failed, ret: %{public}d", ret);
        return false;
    }
    return true;
}

PowerErrors PowerMgrProxy::SetDeviceMode(const PowerMode& mode)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, PowerErrors::ERR_CONNECTION_FAIL);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_POWER_MODE, "Write descriptor failed");
        return PowerErrors::ERR_CONNECTION_FAIL;
    }

    WRITE_PARCEL_WITH_RET(data, Uint32, static_cast<uint32_t>(mode), PowerErrors::ERR_CONNECTION_FAIL);

    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::SETMODE_DEVICE), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_POWER_MODE, "SendRequest is failed, ret: %{public}d", ret);
        return PowerErrors::ERR_CONNECTION_FAIL;
    }
    int32_t error;
    READ_PARCEL_WITH_RET(reply, Int32, error, PowerErrors::ERR_OK);
    return static_cast<PowerErrors>(error);
}

PowerMode PowerMgrProxy::GetDeviceMode()
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, static_cast<PowerMode>(false));

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_POWER_MODE, "Write descriptor failed");
        return PowerMode::NORMAL_MODE;
    }

    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::GETMODE_DEVICE),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_POWER_MODE, "SendRequest is failed, ret: %{public}d", ret);
        return PowerMode::NORMAL_MODE;
    }

    uint32_t used = static_cast<uint32_t>(PowerMode::NORMAL_MODE);
    if (!reply.ReadUint32(used)) {
        POWER_HILOGE(FEATURE_POWER_MODE, "ReadUint32 fail");
    }
    return static_cast<PowerMode>(used);
}

std::string PowerMgrProxy::ShellDump(const std::vector<std::string>& args, uint32_t argc)
{
    sptr<IRemoteObject> remote = Remote();
    std::string result = "remote error";
    RETURN_IF_WITH_RET(remote == nullptr, result);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(COMP_FWK, "Write descriptor failed");
        return result;
    }

    data.WriteUint32(argc);
    for (uint32_t i = 0; i < argc; i++) {
        data.WriteString(args[i]);
    }
    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::SHELL_DUMP),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(COMP_FWK, "SendRequest is failed, ret: %{public}d", ret);
        return result;
    }
    result = reply.ReadString();

    return result;
}
} // namespace PowerMgr
} // namespace OHOS
