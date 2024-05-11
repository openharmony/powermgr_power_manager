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
#include "message_option.h"
#include "shutdown_proxy_delegator.h"
#include "power_log.h"
#include "power_common.h"
#include "power_mgr_ipc_interface_code.h"
#include "running_lock_info.h"

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

    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, RemoteObject, remoteObj.GetRefPtr(), PowerErrors::ERR_CONNECTION_FAIL);
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Parcelable, &runningLockInfo, PowerErrors::ERR_CONNECTION_FAIL);

    int ret = remote->SendRequest(
        static_cast<int>(PowerMgr::PowerMgrInterfaceCode::CREATE_RUNNINGLOCK),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "SendRequest is failed, ret: %{public}d", ret);
        return PowerErrors::ERR_CONNECTION_FAIL;
    }
    int32_t error;
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(reply, Int32, error, PowerErrors::ERR_OK);
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

    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, RemoteObject, remoteObj.GetRefPtr(), false);

    int ret = remote->SendRequest(
        static_cast<int>(PowerMgr::PowerMgrInterfaceCode::RELEASE_RUNNINGLOCK),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "SendRequest is failed, ret: %{public}d", ret);
        return false;
    }
    return true;
}

bool PowerMgrProxy::IsRunningLockTypeSupported(RunningLockType type)
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

    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Uint32, static_cast<uint32_t>(type), false);
    int ret = remote->SendRequest(
        static_cast<int>(PowerMgr::PowerMgrInterfaceCode::IS_RUNNINGLOCK_TYPE_SUPPORTED),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "SendRequest is failed, ret: %{public}d", ret);
        return result;
    }

    if (!reply.ReadBool(result)) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "ReadBool fail");
    }

    return result;
}

bool PowerMgrProxy::Lock(const sptr<IRemoteObject>& remoteObj, int32_t timeOutMs)
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

    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, RemoteObject, remoteObj.GetRefPtr(), false);
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Int32, timeOutMs, false);

    int ret = remote->SendRequest(
        static_cast<int>(PowerMgr::PowerMgrInterfaceCode::RUNNINGLOCK_LOCK),
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

    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, RemoteObject, remoteObj.GetRefPtr(), false);

    int ret = remote->SendRequest(
        static_cast<int>(PowerMgr::PowerMgrInterfaceCode::RUNNINGLOCK_UNLOCK),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "SendRequest is failed, ret: %{public}d", ret);
        return false;
    }
    return true;
}

bool PowerMgrProxy::QueryRunningLockLists(std::map<std::string, RunningLockInfo>& runningLockLists)
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

    int ret = remote->SendRequest(
        static_cast<int>(PowerMgr::PowerMgrInterfaceCode::RUNNINGLOCK_QUERY),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "SendRequest is failed, ret: %{public}d", ret);
        return false;
    }
    int32_t num = reply.ReadInt32();
    for (int i = 0; i < num; i++) {
        std::string key = reply.ReadString();
        RunningLockInfo* info = reply.ReadParcelable<RunningLockInfo>();
        runningLockLists.insert(std::pair<std::string, RunningLockInfo>(key, *info));
        delete info;
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

    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, RemoteObject, remoteObj.GetRefPtr(), false);
    int ret = remote->SendRequest(
        static_cast<int>(PowerMgr::PowerMgrInterfaceCode::RUNNINGLOCK_ISUSED),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "SendRequest is failed, ret: %{public}d", ret);
        return false;
    }
    bool used = false;
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(reply, Bool, used, false);
    return used;
}

bool PowerMgrProxy::ProxyRunningLock(bool isProxied, pid_t pid, pid_t uid)
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

    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Bool, isProxied, false);
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Int32, pid, false);
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Int32, uid, false);
    int ret = remote->SendRequest(
        static_cast<int>(PowerMgr::PowerMgrInterfaceCode::PROXY_RUNNINGLOCK),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "SendRequest is failed, ret: %{public}d", ret);
        return false;
    }
    bool succ = false;
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(reply, Bool, succ, false);
    return succ;
}

bool PowerMgrProxy::ProxyRunningLocks(bool isProxied, const std::vector<std::pair<pid_t, pid_t>>& processInfos)
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

    size_t size = processInfos.size();
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Bool, isProxied, false);
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Int32, size, false);
    for (size_t i = 0; i < size; ++i) {
        RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Int32, processInfos[i].first, false);
        RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Int32, processInfos[i].second, false);
    }
    int ret = remote->SendRequest(static_cast<int>(PowerMgr::PowerMgrInterfaceCode::PROXY_RUNNINGLOCKS),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "SendRequest is failed, ret: %{public}d", ret);
        return false;
    }
    bool succ = false;
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(reply, Bool, succ, false);
    return succ;
}

bool PowerMgrProxy::ResetRunningLocks()
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

    int ret = remote->SendRequest(static_cast<int>(PowerMgr::PowerMgrInterfaceCode::RESET_RUNNINGLOCKS),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "SendRequest is failed, ret: %{public}d", ret);
        return false;
    }
    bool succ = false;
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(reply, Bool, succ, false);
    return succ;
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

    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, String16, Str8ToStr16(reason), PowerErrors::ERR_CONNECTION_FAIL);

    int ret = remote->SendRequest(
        static_cast<int>(PowerMgr::PowerMgrInterfaceCode::REBOOT_DEVICE), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_SHUTDOWN, "SendRequest is failed, ret: %{public}d", ret);
        return PowerErrors::ERR_CONNECTION_FAIL;
    }
    int32_t error;
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(reply, Int32, error, PowerErrors::ERR_OK);
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

    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, String16, Str8ToStr16(reason), PowerErrors::ERR_CONNECTION_FAIL);

    int ret = remote->SendRequest(
        static_cast<int>(PowerMgr::PowerMgrInterfaceCode::REBOOT_DEVICE_FOR_DEPRECATED),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_SHUTDOWN, "SendRequest is failed, ret: %{public}d", ret);
        return PowerErrors::ERR_CONNECTION_FAIL;
    }
    int32_t error;
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(reply, Int32, error, PowerErrors::ERR_OK);
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

    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, String16, Str8ToStr16(reason), PowerErrors::ERR_CONNECTION_FAIL);

    int ret = remote->SendRequest(
        static_cast<int>(PowerMgr::PowerMgrInterfaceCode::SHUTDOWN_DEVICE), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_SHUTDOWN, "SendRequest is failed, ret: %{public}d", ret);
        return PowerErrors::ERR_CONNECTION_FAIL;
    }
    int32_t error;
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(reply, Int32, error, PowerErrors::ERR_OK);
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

    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Int64, callTimeMs, PowerErrors::ERR_CONNECTION_FAIL);
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Uint32,
        static_cast<uint32_t>(reason), PowerErrors::ERR_CONNECTION_FAIL);
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Bool, suspendImmed, PowerErrors::ERR_CONNECTION_FAIL);

    int ret = remote->SendRequest(
        static_cast<int>(PowerMgr::PowerMgrInterfaceCode::SUSPEND_DEVICE), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_SUSPEND, "SendRequest is failed, ret: %{public}d", ret);
        return PowerErrors::ERR_CONNECTION_FAIL;
    }
    int32_t error;
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(reply, Int32, error, PowerErrors::ERR_OK);
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

    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Int64, callTimeMs, PowerErrors::ERR_CONNECTION_FAIL);
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Uint32,
        static_cast<uint32_t>(reason), PowerErrors::ERR_CONNECTION_FAIL);
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, String16, Str8ToStr16(details), PowerErrors::ERR_CONNECTION_FAIL);

    int ret = remote->SendRequest(
        static_cast<int>(PowerMgr::PowerMgrInterfaceCode::WAKEUP_DEVICE), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_WAKEUP, "SendRequest is failed, ret: %{public}d", ret);
        return PowerErrors::ERR_CONNECTION_FAIL;
    }
    int32_t error;
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(reply, Int32, error, PowerErrors::ERR_OK);
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

    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Int64, callTimeMs, false);
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Uint32, static_cast<uint32_t>(type), false);
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Bool, needChangeBacklight, false);

    int ret = remote->SendRequest(
        static_cast<int>(PowerMgr::PowerMgrInterfaceCode::REFRESH_ACTIVITY), data, reply, option);
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

    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Int64, timeout, false);

    int ret = remote->SendRequest(
        static_cast<int>(PowerMgr::PowerMgrInterfaceCode::OVERRIDE_DISPLAY_OFF_TIME),
        data, reply, option);
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

    int ret = remote->SendRequest(
        static_cast<int>(PowerMgr::PowerMgrInterfaceCode::RESTORE_DISPLAY_OFF_TIME),
        data, reply, option);
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
    MessageOption option = { MessageOption::TF_ASYNC };

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_SUSPEND, "Write descriptor failed");
        return result;
    }

    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Int64, callTimeMs, false);

    int ret = remote->SendRequest(
        static_cast<int>(PowerMgr::PowerMgrInterfaceCode::FORCE_DEVICE_SUSPEND), data, reply, option);
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

    int ret = remote->SendRequest(
        static_cast<int>(PowerMgr::PowerMgrInterfaceCode::GET_STATE), data, reply, option);
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

    int ret = remote->SendRequest(
        static_cast<int>(PowerMgr::PowerMgrInterfaceCode::IS_SCREEN_ON), data, reply, option);
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

    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, RemoteObject, callback->AsObject(), false);

    int ret = remote->SendRequest(
        static_cast<int>(PowerMgr::PowerMgrInterfaceCode::REG_POWER_STATE_CALLBACK),
        data, reply, option);
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

    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, RemoteObject, callback->AsObject(), false);

    int ret = remote->SendRequest(
        static_cast<int>(PowerMgr::PowerMgrInterfaceCode::UNREG_POWER_STATE_CALLBACK),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_POWER_STATE, "SendRequest is failed, ret: %{public}d", ret);
        return false;
    }
    return true;
}

bool PowerMgrProxy::RegisterSyncSleepCallback(const sptr<ISyncSleepCallback>& callback, SleepPriority priority)
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

    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, RemoteObject, callback->AsObject(), false);
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Uint32, static_cast<uint32_t>(priority), false);

    int ret = remote->SendRequest(
        static_cast<int>(PowerMgr::PowerMgrInterfaceCode::REG_SYNC_SLEEP_CALLBACK),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_POWER_STATE, "SendRequest is failed, ret: %{public}d", ret);
        return false;
    }
    return true;
}


bool PowerMgrProxy::UnRegisterSyncSleepCallback(const sptr<ISyncSleepCallback>& callback)
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

    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, RemoteObject, callback->AsObject(), false);
 
    int ret = remote->SendRequest(
        static_cast<int>(PowerMgr::PowerMgrInterfaceCode::UNREG_SYNC_SLEEP_CALLBACK),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_POWER_STATE, "SendRequest is failed, ret: %{public}d", ret);
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

    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, RemoteObject, callback->AsObject(), false);

    int ret = remote->SendRequest(
        static_cast<int>(PowerMgr::PowerMgrInterfaceCode::REG_POWER_MODE_CALLBACK),
        data, reply, option);
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

    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, RemoteObject, callback->AsObject(), false);

    int ret = remote->SendRequest(
        static_cast<int>(PowerMgr::PowerMgrInterfaceCode::UNREG_POWER_MODE_CALLBACK),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_POWER_MODE, "SendRequest is failed, ret: %{public}d", ret);
        return false;
    }
    return true;
}

bool PowerMgrProxy::RegisterRunningLockCallback(const sptr<IPowerRunninglockCallback>& callback)
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

    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, RemoteObject, callback->AsObject(), false);

    int ret = remote->SendRequest(
        static_cast<int>(PowerMgr::PowerMgrInterfaceCode::REG_RUNNINGLOCK_CALLBACK),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_POWER_MODE, "SendRequest is failed, ret: %{public}d", ret);
        return false;
    }
    return true;
}

bool PowerMgrProxy::UnRegisterRunningLockCallback(const sptr<IPowerRunninglockCallback>& callback)
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

    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, RemoteObject, callback->AsObject(), false);

    int ret = remote->SendRequest(
        static_cast<int>(PowerMgr::PowerMgrInterfaceCode::UNREG_RUNNINGLOCK_CALLBACK),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_POWER_MODE, "SendRequest is failed, ret: %{public}d", ret);
        return false;
    }
    return true;
}

bool PowerMgrProxy::RegisterScreenStateCallback(int32_t remainTime, const sptr<IScreenOffPreCallback>& callback)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET((remote == nullptr) || (remainTime <= 0) || (callback == nullptr), false);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_SCREEN_OFF_PRE, "Write descriptor failed");
        return false;
    }

    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Int32, remainTime, false);
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, RemoteObject, callback->AsObject(), false);

    int ret = remote->SendRequest(
        static_cast<int>(PowerMgr::PowerMgrInterfaceCode::REG_SCREEN_OFF_PRE_CALLBACK),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_SCREEN_OFF_PRE, "SendRequest is failed, ret: %{public}d", ret);
        return false;
    }
    return true;
}

bool PowerMgrProxy::UnRegisterScreenStateCallback(const sptr<IScreenOffPreCallback>& callback)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET((remote == nullptr) || (callback == nullptr), false);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_SCREEN_OFF_PRE, "Write descriptor failed");
        return false;
    }

    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, RemoteObject, callback->AsObject(), false);

    int ret = remote->SendRequest(
        static_cast<int>(PowerMgr::PowerMgrInterfaceCode::UNREG_SCREEN_OFF_PRE_CALLBACK),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_SCREEN_OFF_PRE, "SendRequest is failed, ret: %{public}d", ret);
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

    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Bool, enable, false);

    int ret = remote->SendRequest(
        static_cast<int>(PowerMgr::PowerMgrInterfaceCode::SET_DISPLAY_SUSPEND),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_SUSPEND, "SendRequest is failed, ret: %{public}d", ret);
        return false;
    }
    return true;
}

bool PowerMgrProxy::Hibernate(bool clearMemory)
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

    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Bool, clearMemory, false);

    int ret = remote->SendRequest(
        static_cast<int>(PowerMgr::PowerMgrInterfaceCode::HIBERNATE),
        data, reply, option);
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

    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Uint32, static_cast<uint32_t>(mode), PowerErrors::ERR_CONNECTION_FAIL);

    int ret = remote->SendRequest(
        static_cast<int>(PowerMgr::PowerMgrInterfaceCode::SETMODE_DEVICE), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_POWER_MODE, "SendRequest is failed, ret: %{public}d", ret);
        return PowerErrors::ERR_CONNECTION_FAIL;
    }
    int32_t error;
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(reply, Int32, error, PowerErrors::ERR_OK);
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

    int ret = remote->SendRequest(
        static_cast<int>(PowerMgr::PowerMgrInterfaceCode::GETMODE_DEVICE),
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
    int ret = remote->SendRequest(
        static_cast<int>(PowerMgr::PowerMgrInterfaceCode::SHELL_DUMP), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(COMP_FWK, "SendRequest is failed, ret: %{public}d", ret);
        return result;
    }
    result = reply.ReadString();

    return result;
}

PowerErrors PowerMgrProxy::IsStandby(bool& isStandby)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, PowerErrors::ERR_CONNECTION_FAIL);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(COMP_FWK, "Write descriptor failed");
        return PowerErrors::ERR_CONNECTION_FAIL;
    }

    int32_t ret = remote->SendRequest(
        static_cast<int>(PowerMgr::PowerMgrInterfaceCode::IS_STANDBY), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(COMP_FWK, "SendRequest is failed, ret: %{public}d", ret);
        return PowerErrors::ERR_CONNECTION_FAIL;
    }

    int32_t error;

    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(reply, Int32, error, PowerErrors::ERR_CONNECTION_FAIL);
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(reply, Bool, isStandby, PowerErrors::ERR_CONNECTION_FAIL);

    return static_cast<PowerErrors>(error);
}

PowerErrors PowerMgrProxy::SetForceTimingOut(bool enabled)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, PowerErrors::ERR_CONNECTION_FAIL);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(COMP_FWK, "Write descriptor failed");
        return PowerErrors::ERR_CONNECTION_FAIL;
    }
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Bool,
        static_cast<uint32_t>(enabled), PowerErrors::ERR_CONNECTION_FAIL);
    int32_t ret = remote->SendRequest(
        static_cast<int>(PowerMgr::PowerMgrInterfaceCode::SET_FORCE_TIMING_OUT), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(COMP_FWK, "SendRequest is failed, ret: %{public}d", ret);
        return PowerErrors::ERR_CONNECTION_FAIL;
    }
    int32_t error;
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(reply, Int32, error, PowerErrors::ERR_CONNECTION_FAIL);
    return static_cast<PowerErrors>(error);
}

PowerErrors PowerMgrProxy::LockScreenAfterTimingOut(bool enabledLockScreen, bool checkLock)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, PowerErrors::ERR_CONNECTION_FAIL);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(COMP_FWK, "Write descriptor failed");
        return PowerErrors::ERR_CONNECTION_FAIL;
    }
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Bool,
        static_cast<uint32_t>(enabledLockScreen), PowerErrors::ERR_CONNECTION_FAIL);
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Bool,
        static_cast<uint32_t>(checkLock), PowerErrors::ERR_CONNECTION_FAIL);
    int32_t ret = remote->SendRequest(
        static_cast<int>(PowerMgr::PowerMgrInterfaceCode::LOCK_SCREEN_AFTER_TIMING_OUT), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(COMP_FWK, "SendRequest is failed, ret: %{public}d", ret);
        return PowerErrors::ERR_CONNECTION_FAIL;
    }
    int32_t error;
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(reply, Int32, error, PowerErrors::ERR_CONNECTION_FAIL);
    return static_cast<PowerErrors>(error);
}

void PowerMgrProxy::RegisterShutdownCallback(const sptr<ITakeOverShutdownCallback>& callback, ShutdownPriority priority)
{
    auto delegator = std::make_unique<ShutdownProxyDelegator>(Remote(), PowerMgrProxy::GetDescriptor());
    delegator->RegisterShutdownCallback(callback, priority);
}

void PowerMgrProxy::UnRegisterShutdownCallback(const sptr<ITakeOverShutdownCallback>& callback)
{
    auto delegator = std::make_unique<ShutdownProxyDelegator>(Remote(), PowerMgrProxy::GetDescriptor());
    delegator->UnRegisterShutdownCallback(callback);
}

void PowerMgrProxy::RegisterShutdownCallback(const sptr<IAsyncShutdownCallback>& callback, ShutdownPriority priority)
{
    auto delegator = std::make_unique<ShutdownProxyDelegator>(Remote(), PowerMgrProxy::GetDescriptor());
    delegator->RegisterShutdownCallback(callback, priority);
}

void PowerMgrProxy::UnRegisterShutdownCallback(const sptr<IAsyncShutdownCallback>& callback)
{
    auto delegator = std::make_unique<ShutdownProxyDelegator>(Remote(), PowerMgrProxy::GetDescriptor());
    delegator->UnRegisterShutdownCallback(callback);
}

void PowerMgrProxy::RegisterShutdownCallback(const sptr<ISyncShutdownCallback>& callback, ShutdownPriority priority)
{
    auto delegator = std::make_unique<ShutdownProxyDelegator>(Remote(), PowerMgrProxy::GetDescriptor());
    delegator->RegisterShutdownCallback(callback, priority);
}

void PowerMgrProxy::UnRegisterShutdownCallback(const sptr<ISyncShutdownCallback>& callback)
{
    auto delegator = std::make_unique<ShutdownProxyDelegator>(Remote(), PowerMgrProxy::GetDescriptor());
    delegator->UnRegisterShutdownCallback(callback);
}
} // namespace PowerMgr
} // namespace OHOS
