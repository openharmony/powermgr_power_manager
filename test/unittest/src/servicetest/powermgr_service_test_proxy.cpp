/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "powermgr_service_test_proxy.h"

#include <message_parcel.h>
#include <string_ex.h>

#include "errors.h"
#include "message_option.h"
#include "power_common.h"
#include "power_log.h"
#include "power_mgr_ipc_interface_code.h"
#include "power_mgr_proxy.h"

namespace OHOS {
namespace PowerMgr {
PowerMgrServiceTestProxy::PowerMgrServiceTestProxy(const sptr<PowerMgrService>& service)
{
    if (service != nullptr) {
        stub_ = static_cast<PowerMgrStub*>(service);
    }
}

PowerErrors PowerMgrServiceTestProxy::CreateRunningLock(const sptr<IRemoteObject>& remoteObj,
    const RunningLockInfo& runningLockInfo)
{
    RETURN_IF_WITH_RET(stub_ == nullptr, PowerErrors::ERR_CONNECTION_FAIL);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Write descriptor failed");
        return PowerErrors::ERR_CONNECTION_FAIL;
    }

    data.WriteRemoteObject(remoteObj.GetRefPtr());
    data.WriteParcelable(&runningLockInfo);

    int ret = stub_->OnRemoteRequest(
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

bool PowerMgrServiceTestProxy::ReleaseRunningLock(const sptr<IRemoteObject>& remoteObj)
{
    RETURN_IF_WITH_RET(stub_ == nullptr, false);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Write descriptor failed");
        return false;
    }

    data.WriteRemoteObject(remoteObj.GetRefPtr());

    int ret = stub_->OnRemoteRequest(
        static_cast<int>(PowerMgr::PowerMgrInterfaceCode::RELEASE_RUNNINGLOCK),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "SendRequest is failed, ret: %{public}d", ret);
        return false;
    }
    return true;
}

bool PowerMgrServiceTestProxy::IsRunningLockTypeSupported(RunningLockType type)
{
    RETURN_IF_WITH_RET(stub_ == nullptr, false);

    bool result = false;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Write descriptor failed");
        return result;
    }

    data.WriteUint32(static_cast<uint32_t>(type));

    int ret = stub_->OnRemoteRequest(
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

bool PowerMgrServiceTestProxy::Lock(const sptr<IRemoteObject>& remoteObj, int32_t timeOutMs)
{
    RETURN_IF_WITH_RET(stub_ == nullptr, false);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Write descriptor failed");
        return false;
    }

    data.WriteRemoteObject(remoteObj.GetRefPtr());
    data.WriteInt32(timeOutMs);

    int ret = stub_->OnRemoteRequest(
        static_cast<int>(PowerMgr::PowerMgrInterfaceCode::RUNNINGLOCK_LOCK),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "SendRequest is failed, ret: %{public}d", ret);
        return false;
    }
    return true;
}

bool PowerMgrServiceTestProxy::UnLock(const sptr<IRemoteObject>& remoteObj)
{
    RETURN_IF_WITH_RET(stub_ == nullptr, false);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Write descriptor failed");
        return false;
    }

    data.WriteRemoteObject(remoteObj.GetRefPtr());

    int ret = stub_->OnRemoteRequest(
        static_cast<int>(PowerMgr::PowerMgrInterfaceCode::RUNNINGLOCK_UNLOCK),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "SendRequest is failed, ret: %{public}d", ret);
        return false;
    }
    return true;
}

bool PowerMgrServiceTestProxy::IsUsed(const sptr<IRemoteObject>& remoteObj)
{
    RETURN_IF_WITH_RET(stub_ == nullptr, false);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Write descriptor failed");
        return false;
    }

    data.WriteRemoteObject(remoteObj.GetRefPtr());
    int ret = stub_->OnRemoteRequest(
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

bool PowerMgrServiceTestProxy::ProxyRunningLock(bool isProxied, pid_t pid, pid_t uid)
{
    RETURN_IF_WITH_RET(stub_ == nullptr, false);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Write descriptor failed");
        return false;
    }

    data.WriteBool(isProxied);
    data.WriteInt32(pid);
    data.WriteInt32(uid);

    int ret = stub_->OnRemoteRequest(
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

bool PowerMgrServiceTestProxy::ProxyRunningLocks(bool isProxied,
    const std::vector<std::pair<pid_t, pid_t>>& processInfos)
{
    RETURN_IF_WITH_RET(stub_ == nullptr, false);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Write descriptor failed");
        return false;
    }

    int32_t size = processInfos.size();
    data.WriteBool(isProxied);
    data.WriteUint32(size);
    for (int i = 0; i < size; ++i) {
        data.WriteInt32(processInfos[i].first);
        data.WriteInt32(processInfos[i].second);
    }

    int ret = stub_->OnRemoteRequest(
        static_cast<int>(PowerMgr::PowerMgrInterfaceCode::PROXY_RUNNINGLOCKS),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "SendRequest is failed, ret: %{public}d", ret);
        return false;
    }
    bool succ = false;
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(reply, Bool, succ, false);
    return succ;
}

bool PowerMgrServiceTestProxy::ResetRunningLocks()
{
    RETURN_IF_WITH_RET(stub_ == nullptr, false);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Write descriptor failed");
        return false;
    }

    int ret = stub_->SendRequest(static_cast<int>(PowerMgr::PowerMgrInterfaceCode::RESET_RUNNINGLOCKS),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "SendRequest is failed, ret: %{public}d", ret);
        return false;
    }
    bool succ = false;
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(reply, Bool, succ, false);
    return succ;
}

PowerErrors PowerMgrServiceTestProxy::RebootDevice(const std::string& reason)
{
    RETURN_IF_WITH_RET(stub_ == nullptr, PowerErrors::ERR_CONNECTION_FAIL);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_SHUTDOWN, "Write descriptor failed");
        return PowerErrors::ERR_CONNECTION_FAIL;
    }

    data.WriteString16(Str8ToStr16(reason));

    int ret = stub_->OnRemoteRequest(
        static_cast<int>(PowerMgr::PowerMgrInterfaceCode::REBOOT_DEVICE), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_SHUTDOWN, "SendRequest is failed, ret: %{public}d", ret);
        return PowerErrors::ERR_CONNECTION_FAIL;
    }
    int32_t error;
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(reply, Int32, error, PowerErrors::ERR_OK);
    return static_cast<PowerErrors>(error);
}

PowerErrors PowerMgrServiceTestProxy::ShutDownDevice(const std::string& reason)
{
    RETURN_IF_WITH_RET(stub_ == nullptr, PowerErrors::ERR_CONNECTION_FAIL);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_SHUTDOWN, "Write descriptor failed");
        return PowerErrors::ERR_CONNECTION_FAIL;
    }

    data.WriteString16(Str8ToStr16(reason));

    int ret = stub_->OnRemoteRequest(
        static_cast<int>(PowerMgr::PowerMgrInterfaceCode::SHUTDOWN_DEVICE), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_SHUTDOWN, "SendRequest is failed, ret: %{public}d", ret);
        return PowerErrors::ERR_CONNECTION_FAIL;
    }
    int32_t error;
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(reply, Int32, error, PowerErrors::ERR_OK);
    return static_cast<PowerErrors>(error);
}

PowerErrors PowerMgrServiceTestProxy::SuspendDevice(int64_t callTimeMs, SuspendDeviceType reason, bool suspendImmed)
{
    RETURN_IF_WITH_RET(stub_ == nullptr, PowerErrors::ERR_CONNECTION_FAIL);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_SUSPEND, "Write descriptor failed");
        return PowerErrors::ERR_CONNECTION_FAIL;
    }

    data.WriteInt64(callTimeMs);
    data.WriteUint32(static_cast<uint32_t>(reason));
    data.WriteBool(suspendImmed);

    int ret = stub_->OnRemoteRequest(
        static_cast<int>(PowerMgr::PowerMgrInterfaceCode::SUSPEND_DEVICE), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_SUSPEND, "SendRequest is failed, ret: %{public}d", ret);
        return PowerErrors::ERR_CONNECTION_FAIL;
    }
    int32_t error;
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(reply, Int32, error, PowerErrors::ERR_OK);
    return static_cast<PowerErrors>(error);
}

PowerErrors PowerMgrServiceTestProxy::WakeupDevice(int64_t callTimeMs, WakeupDeviceType reason,
    const std::string& details)
{
    RETURN_IF_WITH_RET(stub_ == nullptr, PowerErrors::ERR_CONNECTION_FAIL);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_WAKEUP, "Write descriptor failed");
        return PowerErrors::ERR_CONNECTION_FAIL;
    }

    data.WriteInt64(callTimeMs);
    data.WriteUint32(static_cast<uint32_t>(reason));
    data.WriteString16(Str8ToStr16(details));

    int ret = stub_->OnRemoteRequest(
        static_cast<int>(PowerMgr::PowerMgrInterfaceCode::WAKEUP_DEVICE), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_WAKEUP, "SendRequest is failed, ret: %{public}d", ret);
        return PowerErrors::ERR_CONNECTION_FAIL;
    }
    int32_t error;
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(reply, Int32, error, PowerErrors::ERR_OK);
    return static_cast<PowerErrors>(error);
}

bool PowerMgrServiceTestProxy::RefreshActivity(int64_t callTimeMs, UserActivityType type, bool needChangeBacklight)
{
    RETURN_IF_WITH_RET(stub_ == nullptr, false);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_ACTIVITY, "Write descriptor failed");
        return false;
    }

    data.WriteInt64(callTimeMs);
    data.WriteUint32(static_cast<uint32_t>(type));
    data.WriteBool(needChangeBacklight);

    int ret = stub_->OnRemoteRequest(
        static_cast<int>(PowerMgr::PowerMgrInterfaceCode::REFRESH_ACTIVITY), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_ACTIVITY, "SendRequest is failed, ret: %{public}d", ret);
        return false;
    }
    return true;
}

bool PowerMgrServiceTestProxy::OverrideScreenOffTime(int64_t timeout)
{
    RETURN_IF_WITH_RET(stub_ == nullptr, false);

    bool result = false;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(COMP_SVC, "Write descriptor failed");
        return result;
    }

    data.WriteInt64(timeout);

    int ret = stub_->OnRemoteRequest(
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

bool PowerMgrServiceTestProxy::RestoreScreenOffTime()
{
    RETURN_IF_WITH_RET(stub_ == nullptr, false);

    bool result = false;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(COMP_FWK, "Write descriptor failed");
        return result;
    }

    int ret = stub_->OnRemoteRequest(
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

bool PowerMgrServiceTestProxy::ForceSuspendDevice(int64_t callTimeMs)
{
    RETURN_IF_WITH_RET(stub_ == nullptr, false);

    bool result = false;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_SUSPEND, "Write descriptor failed");
        return result;
    }

    data.WriteInt64(callTimeMs);

    int ret = stub_->OnRemoteRequest(
        static_cast<int>(PowerMgr::PowerMgrInterfaceCode::FORCE_DEVICE_SUSPEND),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_SUSPEND, "SendRequest is failed, ret: %{public}d", ret);
        return result;
    }
    if (!reply.ReadBool(result)) {
        POWER_HILOGE(FEATURE_SUSPEND, "ReadBool fail");
    }

    return result;
}

PowerState PowerMgrServiceTestProxy::GetState()
{
    RETURN_IF_WITH_RET(stub_ == nullptr, PowerState::UNKNOWN);

    uint32_t result = 0;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_POWER_STATE, "Write descriptor failed");
        return PowerState::UNKNOWN;
    }

    int ret = stub_->OnRemoteRequest(
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

bool PowerMgrServiceTestProxy::IsScreenOn()
{
    RETURN_IF_WITH_RET(stub_ == nullptr, false);

    bool result = false;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(COMP_FWK, "Write descriptor failed");
        return result;
    }

    int ret = stub_->OnRemoteRequest(
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

bool PowerMgrServiceTestProxy::RegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback)
{
    RETURN_IF_WITH_RET((stub_ == nullptr) || (callback == nullptr), false);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_POWER_STATE, "Write descriptor failed");
        return false;
    }

    data.WriteRemoteObject(callback->AsObject());

    int ret = stub_->OnRemoteRequest(
        static_cast<int>(PowerMgr::PowerMgrInterfaceCode::REG_POWER_STATE_CALLBACK),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_POWER_STATE, "SendRequest is failed, ret: %{public}d", ret);
        return false;
    }
    return true;
}

bool PowerMgrServiceTestProxy::UnRegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback)
{
    RETURN_IF_WITH_RET((stub_ == nullptr) || (callback == nullptr), false);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_POWER_STATE, "Write descriptor failed");
        return false;
    }

    data.WriteRemoteObject(callback->AsObject());

    int ret = stub_->OnRemoteRequest(
        static_cast<int>(PowerMgr::PowerMgrInterfaceCode::UNREG_POWER_STATE_CALLBACK),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_POWER_STATE, "SendRequest is failed, ret: %{public}d", ret);
        return false;
    }
    return true;
}

bool PowerMgrServiceTestProxy::RegisterPowerModeCallback(const sptr<IPowerModeCallback>& callback)
{
    RETURN_IF_WITH_RET((stub_ == nullptr) || (callback == nullptr), false);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_POWER_MODE, "Write descriptor failed");
        return false;
    }

    data.WriteRemoteObject(callback->AsObject());

    int ret = stub_->OnRemoteRequest(
        static_cast<int>(PowerMgr::PowerMgrInterfaceCode::REG_POWER_MODE_CALLBACK),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_POWER_MODE, "SendRequest is failed, ret: %{public}d", ret);
        return false;
    }
    return true;
}

bool PowerMgrServiceTestProxy::UnRegisterPowerModeCallback(const sptr<IPowerModeCallback>& callback)
{
    RETURN_IF_WITH_RET((stub_ == nullptr) || (callback == nullptr), false);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_POWER_MODE, "Write descriptor failed");
        return false;
    }

    data.WriteRemoteObject(callback->AsObject());

    int ret = stub_->OnRemoteRequest(
        static_cast<int>(PowerMgr::PowerMgrInterfaceCode::UNREG_POWER_MODE_CALLBACK),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_POWER_MODE, "SendRequest is failed, ret: %{public}d", ret);
        return false;
    }
    return true;
}

bool PowerMgrServiceTestProxy::RegisterRunningLockCallback(const sptr<IPowerRunninglockCallback>& callback)
{
    RETURN_IF_WITH_RET((stub_ == nullptr) || (callback == nullptr), false);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_POWER_MODE, "Write descriptor failed");
        return false;
    }

    data.WriteRemoteObject(callback->AsObject());

    int ret = stub_->OnRemoteRequest(
        static_cast<int>(PowerMgr::PowerMgrInterfaceCode::REG_RUNNINGLOCK_CALLBACK),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_POWER_MODE, "SendRequest is failed, ret: %{public}d", ret);
        return false;
    }
    return true;
}

bool PowerMgrServiceTestProxy::UnRegisterRunningLockCallback(const sptr<IPowerRunninglockCallback>& callback)
{
    RETURN_IF_WITH_RET((stub_ == nullptr) || (callback == nullptr), false);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_POWER_MODE, "Write descriptor failed");
        return false;
    }

    data.WriteRemoteObject(callback->AsObject());

    int ret = stub_->OnRemoteRequest(
        static_cast<int>(PowerMgr::PowerMgrInterfaceCode::UNREG_RUNNINGLOCK_CALLBACK),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_POWER_MODE, "SendRequest is failed, ret: %{public}d", ret);
        return false;
    }
    return true;
}

bool PowerMgrServiceTestProxy::RegisterScreenStateCallback(int32_t remainTime,
    const sptr<IScreenOffPreCallback>& callback)
{
    RETURN_IF_WITH_RET((stub_ == nullptr) || (remainTime <= 0) || (callback == nullptr), false);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_SCREEN_OFF_PRE, "Write descriptor failed");
        return false;
    }
    data.WriteInt32(remainTime);
    data.WriteRemoteObject(callback->AsObject());

    int ret = stub_->OnRemoteRequest(
        static_cast<int>(PowerMgr::PowerMgrInterfaceCode::REG_SCREEN_OFF_PRE_CALLBACK),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_SCREEN_OFF_PRE, "SendRequest is failed, ret: %{public}d", ret);
        return false;
    }
    return true;
}

bool PowerMgrServiceTestProxy::UnRegisterScreenStateCallback(const sptr<IScreenOffPreCallback>& callback)
{
    RETURN_IF_WITH_RET((stub_ == nullptr) || (callback == nullptr), false);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_SCREEN_OFF_PRE, "Write descriptor failed");
        return false;
    }

    data.WriteRemoteObject(callback->AsObject());

    int ret = stub_->OnRemoteRequest(
        static_cast<int>(PowerMgr::PowerMgrInterfaceCode::UNREG_SCREEN_OFF_PRE_CALLBACK),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_SCREEN_OFF_PRE, "SendRequest is failed, ret: %{public}d", ret);
        return false;
    }
    return true;
}

bool PowerMgrServiceTestProxy::SetDisplaySuspend(bool enable)
{
    RETURN_IF_WITH_RET(stub_ == nullptr, false);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_SUSPEND, "Write descriptor failed");
        return false;
    }

    data.WriteBool(enable);

    int ret = stub_->OnRemoteRequest(
        static_cast<int>(PowerMgr::PowerMgrInterfaceCode::SET_DISPLAY_SUSPEND), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_SUSPEND, "SendRequest is failed, ret: %{public}d", ret);
        return false;
    }
    return true;
}

PowerErrors PowerMgrServiceTestProxy::SetDeviceMode(const PowerMode& mode)
{
    RETURN_IF_WITH_RET(stub_ == nullptr, PowerErrors::ERR_CONNECTION_FAIL);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_POWER_MODE, "Write descriptor failed");
        return PowerErrors::ERR_CONNECTION_FAIL;
    }

    data.WriteUint32(static_cast<uint32_t>(mode));

    int ret = stub_->OnRemoteRequest(
        static_cast<int>(PowerMgr::PowerMgrInterfaceCode::SETMODE_DEVICE), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_POWER_MODE, "SendRequest is failed, ret: %{public}d", ret);
        return PowerErrors::ERR_CONNECTION_FAIL;
    }
    int32_t error;
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(reply, Int32, error, PowerErrors::ERR_OK);
    return static_cast<PowerErrors>(error);
}

PowerMode PowerMgrServiceTestProxy::GetDeviceMode()
{
    RETURN_IF_WITH_RET(stub_ == nullptr, static_cast<PowerMode>(false));

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_POWER_MODE, "Write descriptor failed");
        return PowerMode::NORMAL_MODE;
    }

    int ret = stub_->OnRemoteRequest(
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

std::string PowerMgrServiceTestProxy::ShellDump(const std::vector<std::string>& args, uint32_t argc)
{
    std::string result = "remote error";
    RETURN_IF_WITH_RET(stub_ == nullptr, result);

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
    int ret = stub_->OnRemoteRequest(
        static_cast<int>(PowerMgr::PowerMgrInterfaceCode::SHELL_DUMP),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(COMP_FWK, "SendRequest is failed, ret: %{public}d", ret);
        return result;
    }
    result = reply.ReadString();

    return result;
}

PowerErrors PowerMgrServiceTestProxy::IsStandby(bool& isStandby)
{
    RETURN_IF_WITH_RET(stub_ == nullptr, PowerErrors::ERR_CONNECTION_FAIL);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(COMP_FWK, "Write descriptor failed");
        return PowerErrors::ERR_CONNECTION_FAIL;
    }

    int32_t ret = stub_->SendRequest(
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
} // namespace PowerMgr
} // namespace OHOS
