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

#include "power_mgr_proxy.h"

#include <ipc_types.h>
#include <message_parcel.h>
#include <string_ex.h>

#include "power_common.h"

namespace OHOS {
namespace PowerMgr {
void PowerMgrProxy::Lock(const sptr<IRemoteObject>& token, const RunningLockInfo& runningLockInfo, uint32_t timeOutMs)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF(remote == nullptr);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(MODULE_INNERKIT, "PowerMgrProxy::%{public}s write descriptor failed!", __func__);
        return;
    }

    WRITE_PARCEL_NO_RET(data, RemoteObject, token.GetRefPtr());
    WRITE_PARCEL_NO_RET(data, Parcelable, &runningLockInfo);
    WRITE_PARCEL_NO_RET(data, Uint32, timeOutMs);

    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::RUNNINGLOCK_LOCK),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(MODULE_INNERKIT, "PowerMgrProxy::%{public}s SendRequest is failed, error code: %d", __func__, ret);
        return;
    }
}

void PowerMgrProxy::UnLock(const sptr<IRemoteObject>& token)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF(remote == nullptr);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(MODULE_INNERKIT, "PowerMgrProxy::%{public}s write descriptor failed!", __func__);
        return;
    }

    WRITE_PARCEL_NO_RET(data, RemoteObject, token.GetRefPtr());

    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::RUNNINGLOCK_UNLOCK),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(MODULE_INNERKIT, "PowerMgrProxy::%{public}s SendRequest is failed, error code: %d", __func__, ret);
        return;
    }
}

bool PowerMgrProxy::IsUsed(const sptr<IRemoteObject>& token)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, false);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(MODULE_INNERKIT, "PowerMgrProxy::%{public}s write descriptor failed!", __func__);
        return false;
    }

    WRITE_PARCEL_WITH_RET(data, RemoteObject, token.GetRefPtr(), false);
    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::RUNNINGLOCK_ISUSED),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(MODULE_INNERKIT, "PowerMgrProxy::%{public}s SendRequest is failed, error code: %d", __func__, ret);
        return false;
    }
    bool used = false;
    READ_PARCEL_WITH_RET(reply, Bool, used, false);
    return used;
}

void PowerMgrProxy::SetWorkTriggerList(const sptr<IRemoteObject>& token, const WorkTriggerList& workTriggerList)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF(remote == nullptr);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(MODULE_INNERKIT, "PowerMgrProxy::%{public}s write descriptor failed!", __func__);
        return;
    }

    WRITE_PARCEL_NO_RET(data, RemoteObject, token.GetRefPtr());
    RETURN_IF(!RunningLockInfo::MarshallingWorkTriggerList(data, workTriggerList));
    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::RUNNINGLOCK_SET_WORK_TRIGGER_LIST),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(MODULE_INNERKIT, "PowerMgrProxy::%{public}s SendRequest is failed, error code: %d", __func__, ret);
        return;
    }
}

void PowerMgrProxy::ProxyRunningLock(bool proxyLock, pid_t uid, pid_t pid)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF(remote == nullptr);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(MODULE_INNERKIT, "PowerMgrProxy::%{public}s write descriptor failed!", __func__);
        return;
    }

    WRITE_PARCEL_NO_RET(data, Bool, proxyLock);
    WRITE_PARCEL_NO_RET(data, Int32, uid);
    WRITE_PARCEL_NO_RET(data, Int32, pid);
    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::PROXY_RUNNINGLOCK),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(MODULE_INNERKIT, "PowerMgrProxy::%{public}s SendRequest is failed, error code: %d", __func__, ret);
        return;
    }
}

void PowerMgrProxy::RebootDevice(const std::string& reason)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF(remote == nullptr);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(MODULE_INNERKIT, "PowerMgrProxy::%{public}s write descriptor failed!", __func__);
        return;
    }

    WRITE_PARCEL_NO_RET(data, String16, Str8ToStr16(reason));

    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::REBOOT_DEVICE), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(MODULE_INNERKIT, "PowerMgrProxy::%{public}s Transact is failed, error code: %d", __func__, ret);
    }
}

void PowerMgrProxy::ShutDownDevice(const std::string& reason)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF(remote == nullptr);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(MODULE_INNERKIT, "PowerMgrProxy::%{public}s write descriptor failed!", __func__);
        return;
    }

    WRITE_PARCEL_NO_RET(data, String16, Str8ToStr16(reason));

    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::SHUTDOWN_DEVICE), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(MODULE_INNERKIT, "PowerMgrProxy::%{public}s Transact is failed, error code: %d", __func__, ret);
    }
}

void PowerMgrProxy::SuspendDevice(int64_t callTimeMs, SuspendDeviceType reason, bool suspendImmed)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF(remote == nullptr);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(MODULE_INNERKIT, "PowerMgrProxy::%{public}s write descriptor failed!", __func__);
        return;
    }

    WRITE_PARCEL_NO_RET(data, Int64, callTimeMs);
    WRITE_PARCEL_NO_RET(data, Uint32, static_cast<uint32_t>(reason));
    WRITE_PARCEL_NO_RET(data, Bool, suspendImmed);

    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::SUSPEND_DEVICE), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(MODULE_INNERKIT, "PowerMgrProxy::%{public}s SendRequest is failed, error code: %d", __func__, ret);
    }
}

void PowerMgrProxy::WakeupDevice(int64_t callTimeMs, WakeupDeviceType reason, const std::string& details)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF(remote == nullptr);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(MODULE_INNERKIT, "PowerMgrProxy::%{public}s write descriptor failed!", __func__);
        return;
    }

    WRITE_PARCEL_NO_RET(data, Int64, callTimeMs);
    WRITE_PARCEL_NO_RET(data, Uint32, static_cast<uint32_t>(reason));
    WRITE_PARCEL_NO_RET(data, String16, Str8ToStr16(details));

    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::WAKEUP_DEVICE), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(MODULE_INNERKIT, "PowerMgrProxy::%{public}s SendRequest is failed, error code: %d", __func__, ret);
        return;
    }
}

void PowerMgrProxy::RefreshActivity(int64_t callTimeMs, UserActivityType type, bool needChangeBacklight)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF(remote == nullptr);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(MODULE_INNERKIT, "PowerMgrProxy::%{public}s write descriptor failed!", __func__);
        return;
    }

    WRITE_PARCEL_NO_RET(data, Int64, callTimeMs);
    WRITE_PARCEL_NO_RET(data, Uint32, static_cast<uint32_t>(type));
    WRITE_PARCEL_NO_RET(data, Bool, needChangeBacklight);

    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::REFRESH_ACTIVITY), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(MODULE_INNERKIT, "PowerMgrProxy::%{public}s SendRequest is failed, error code: %d", __func__, ret);
        return;
    }
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
        POWER_HILOGE(MODULE_INNERKIT, "PowerMgrProxy::%{public}s write descriptor failed!", __func__);
        return result;
    }

    WRITE_PARCEL_WITH_RET(data, Int64, callTimeMs, false);

    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::FORCE_DEVICE_SUSPEND), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(MODULE_INNERKIT, "PowerMgrProxy::%{public}s SendRequest is failed, error code: %d", __func__, ret);
        return result;
    }
    if (!reply.ReadBool(result)) {
        POWER_HILOGE(MODULE_INNERKIT, "PowerMgrProxy::%{public}s Readback fail!", __func__);
    }

    return result;
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
        POWER_HILOGE(MODULE_INNERKIT, "PowerMgrProxy::%{public}s write descriptor failed!", __func__);
        return result;
    }

    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::IS_SCREEN_ON), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(MODULE_INNERKIT, "PowerMgrProxy::%{public}s SendRequest is failed, error code: %d", __func__, ret);
        return result;
    }
    if (!reply.ReadBool(result)) {
        POWER_HILOGE(MODULE_INNERKIT, "PowerMgrProxy::%{public}s Readback fail!", __func__);
    }

    return result;
}

void PowerMgrProxy::RegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF((remote == nullptr) || (callback == nullptr));

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(MODULE_INNERKIT, "PowerMgrProxy::%{public}s write descriptor failed!", __func__);
        return;
    }

    WRITE_PARCEL_NO_RET(data, RemoteObject, callback->AsObject());

    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::REG_POWER_STATE_CALLBACK), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(MODULE_INNERKIT, "PowerMgrProxy::%{public}s SendRequest is failed, error code: %d", __func__, ret);
        return;
    }
}

void PowerMgrProxy::UnRegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF((remote == nullptr) || (callback == nullptr));

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(MODULE_INNERKIT, "PowerMgrProxy::%{public}s write descriptor failed!", __func__);
        return;
    }

    WRITE_PARCEL_NO_RET(data, RemoteObject, callback->AsObject());

    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::UNREG_POWER_STATE_CALLBACK), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(MODULE_INNERKIT, "PowerMgrProxy::%{public}s SendRequest is failed, error code: %d", __func__, ret);
        return;
    }
}

void PowerMgrProxy::RegisterShutdownCallback(const sptr<IShutdownCallback>& callback)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF((remote == nullptr) || (callback == nullptr));

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(MODULE_INNERKIT, "Write descriptor failed!");
        return;
    }

    WRITE_PARCEL_NO_RET(data, RemoteObject, callback->AsObject());

    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::REG_SHUTDOWN_CALLBACK), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(MODULE_INNERKIT, "SendRequest is failed, error code: %{public}d", ret);
        return;
    }
}

void PowerMgrProxy::UnRegisterShutdownCallback(const sptr<IShutdownCallback>& callback)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF((remote == nullptr) || (callback == nullptr));

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(MODULE_INNERKIT, "Write descriptor failed!");
        return;
    }

    WRITE_PARCEL_NO_RET(data, RemoteObject, callback->AsObject());

    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::UNREG_SHUTDOWN_CALLBACK), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(MODULE_INNERKIT, "SendRequest is failed, error code: %{public}d", ret);
        return;
    }
}
} // namespace PowerMgr
} // namespace OHOS
