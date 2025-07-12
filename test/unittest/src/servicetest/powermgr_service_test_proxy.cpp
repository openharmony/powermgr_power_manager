/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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
#include "hilog/log.h"
#include "power_common.h"
#include "power_log.h"
#include "ipower_mgr.h"
#include "power_mgr_proxy.h"

using OHOS::HiviewDFX::HiLog;
namespace OHOS {
namespace PowerMgr {
constexpr int32_t MAX_PROXY_RUNNINGLOCK_NUM = 2000;
PowerMgrServiceTestProxy::PowerMgrServiceTestProxy(const sptr<PowerMgrService>& service)
{
    if (service != nullptr) {
        stub_ = static_cast<PowerMgrStub*>(service);
    }
}

int32_t PowerMgrServiceTestProxy::CreateRunningLockIpc(const sptr<IRemoteObject>& remoteObj,
    const RunningLockInfo& runningLockInfo, int32_t& powerError)
{
    RETURN_IF_WITH_RET(stub_ == nullptr, ERR_INVALID_DATA);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        HiLog::Error(LABEL, "Write interface token failed!");
        return ERR_INVALID_VALUE;
    }

    if (!data.WriteRemoteObject(remoteObj)) {
        HiLog::Error(LABEL, "Write [remoteObj] failed!");
        return ERR_INVALID_DATA;
    }
    if (!data.WriteParcelable(&runningLockInfo)) {
        HiLog::Error(LABEL, "Write [runningLockInfo] failed!");
        return ERR_INVALID_DATA;
    }

    int32_t result = stub_->OnRemoteRequest(
        static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_CREATE_RUNNING_LOCK_IPC), data, reply, option);
    if (FAILED(result)) {
        HiLog::Error(LABEL, "Send request failed!");
        return result;
    }

    ErrCode errCode = reply.ReadInt32();
    if (FAILED(errCode)) {
        HiLog::Error(LABEL, "Read result failed, code is: %{public}d.",
            static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_CREATE_RUNNING_LOCK_IPC));
        return errCode;
    }

    powerError = reply.ReadInt32();
    return ERR_OK;
}

int32_t PowerMgrServiceTestProxy::ReleaseRunningLockIpc(const sptr<IRemoteObject>& remoteObj, const std::string& name)
{
    RETURN_IF_WITH_RET(stub_ == nullptr, ERR_INVALID_DATA);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        HiLog::Error(LABEL, "Write interface token failed!");
        return ERR_INVALID_VALUE;
    }

    if (!data.WriteRemoteObject(remoteObj)) {
        HiLog::Error(LABEL, "Write [remoteObj] failed!");
        return ERR_INVALID_DATA;
    }
    if (!data.WriteString16(Str8ToStr16(name))) {
        HiLog::Error(LABEL, "Write [name] failed!");
        return ERR_INVALID_DATA;
    }

    int32_t result = stub_->OnRemoteRequest(
        static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_RELEASE_RUNNING_LOCK_IPC), data, reply, option);
    if (FAILED(result)) {
        HiLog::Error(LABEL, "Send request failed!");
        return result;
    }

    ErrCode errCode = reply.ReadInt32();
    if (FAILED(errCode)) {
        HiLog::Error(LABEL, "Read result failed, code is: %{public}d.",
            static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_RELEASE_RUNNING_LOCK_IPC));
        return errCode;
    }

    return ERR_OK;
}

int32_t PowerMgrServiceTestProxy::IsRunningLockTypeSupportedIpc(int32_t lockType, bool& lockTypesSupported)
{
    RETURN_IF_WITH_RET(stub_ == nullptr, ERR_INVALID_DATA);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        HiLog::Error(LABEL, "Write interface token failed!");
        return ERR_INVALID_VALUE;
    }

    if (!data.WriteInt32(lockType)) {
        HiLog::Error(LABEL, "Write [lockType] failed!");
        return ERR_INVALID_DATA;
    }

    int32_t result = stub_->OnRemoteRequest(
        static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_IS_RUNNING_LOCK_TYPE_SUPPORTED_IPC), data, reply, option);
    if (FAILED(result)) {
        HiLog::Error(LABEL, "Send request failed!");
        return result;
    }

    ErrCode errCode = reply.ReadInt32();
    if (FAILED(errCode)) {
        HiLog::Error(LABEL, "Read result failed, code is: %{public}d.",
            static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_IS_RUNNING_LOCK_TYPE_SUPPORTED_IPC));
        return errCode;
    }

    lockTypesSupported = reply.ReadInt32() == 1 ? true : false;
    return ERR_OK;
}

int32_t PowerMgrServiceTestProxy::LockIpc(const sptr<IRemoteObject>& remoteObj, int32_t timeOutMs, int32_t& powerError)
{
    RETURN_IF_WITH_RET(stub_ == nullptr, ERR_INVALID_DATA);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        HiLog::Error(LABEL, "Write interface token failed!");
        return ERR_INVALID_VALUE;
    }

    if (!data.WriteRemoteObject(remoteObj)) {
        HiLog::Error(LABEL, "Write [remoteObj] failed!");
        return ERR_INVALID_DATA;
    }
    if (!data.WriteInt32(timeOutMs)) {
        HiLog::Error(LABEL, "Write [timeOutMs] failed!");
        return ERR_INVALID_DATA;
    }

    int32_t result = stub_->OnRemoteRequest(
        static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_LOCK_IPC), data, reply, option);
    if (FAILED(result)) {
        HiLog::Error(LABEL, "Send request failed!");
        return result;
    }

    ErrCode errCode = reply.ReadInt32();
    if (FAILED(errCode)) {
        HiLog::Error(LABEL, "Read result failed, code is: %{public}d.",
            static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_LOCK_IPC));
        return errCode;
    }

    powerError = reply.ReadInt32();
    return ERR_OK;
}

int32_t PowerMgrServiceTestProxy::UnLockIpc(const sptr<IRemoteObject>& remoteObj,
    const std::string& name, int32_t& powerError)
{
    RETURN_IF_WITH_RET(stub_ == nullptr, ERR_INVALID_DATA);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        HiLog::Error(LABEL, "Write interface token failed!");
        return ERR_INVALID_VALUE;
    }

    if (!data.WriteRemoteObject(remoteObj)) {
        HiLog::Error(LABEL, "Write [remoteObj] failed!");
        return ERR_INVALID_DATA;
    }
    if (!data.WriteString16(Str8ToStr16(name))) {
        HiLog::Error(LABEL, "Write [name] failed!");
        return ERR_INVALID_DATA;
    }

    int32_t result = stub_->OnRemoteRequest(
        static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_UN_LOCK_IPC), data, reply, option);
    if (FAILED(result)) {
        HiLog::Error(LABEL, "Send request failed!");
        return result;
    }

    ErrCode errCode = reply.ReadInt32();
    if (FAILED(errCode)) {
        HiLog::Error(LABEL, "Read result failed, code is: %{public}d.",
            static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_UN_LOCK_IPC));
        return errCode;
    }

    powerError = reply.ReadInt32();
    return ERR_OK;
}

int32_t PowerMgrServiceTestProxy::IsUsedIpc(const sptr<IRemoteObject>& remoteObj, bool& isUsed)
{
    RETURN_IF_WITH_RET(stub_ == nullptr, ERR_INVALID_DATA);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        HiLog::Error(LABEL, "Write interface token failed!");
        return ERR_INVALID_VALUE;
    }

    if (!data.WriteRemoteObject(remoteObj)) {
        HiLog::Error(LABEL, "Write [remoteObj] failed!");
        return ERR_INVALID_DATA;
    }

    int32_t result = stub_->OnRemoteRequest(
        static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_IS_USED_IPC), data, reply, option);
    if (FAILED(result)) {
        HiLog::Error(LABEL, "Send request failed!");
        return result;
    }

    ErrCode errCode = reply.ReadInt32();
    if (FAILED(errCode)) {
        HiLog::Error(LABEL, "Read result failed, code is: %{public}d.",
            static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_IS_USED_IPC));
        return errCode;
    }

    isUsed = reply.ReadInt32() == 1 ? true : false;
    return ERR_OK;
}

int32_t PowerMgrServiceTestProxy::ProxyRunningLockIpc(bool isProxied, int32_t pid, int32_t uid)
{
    RETURN_IF_WITH_RET(stub_ == nullptr, ERR_INVALID_DATA);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        HiLog::Error(LABEL, "Write interface token failed!");
        return ERR_INVALID_VALUE;
    }

    if (!data.WriteInt32(isProxied ? 1 : 0)) {
        HiLog::Error(LABEL, "Write [isProxied] failed!");
        return ERR_INVALID_DATA;
    }
    if (!data.WriteInt32(pid)) {
        HiLog::Error(LABEL, "Write [pid] failed!");
        return ERR_INVALID_DATA;
    }
    if (!data.WriteInt32(uid)) {
        HiLog::Error(LABEL, "Write [uid] failed!");
        return ERR_INVALID_DATA;
    }

    int32_t result = stub_->OnRemoteRequest(
        static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_PROXY_RUNNING_LOCK_IPC), data, reply, option);
    if (FAILED(result)) {
        HiLog::Error(LABEL, "Send request failed!");
        return result;
    }

    ErrCode errCode = reply.ReadInt32();
    if (FAILED(errCode)) {
        HiLog::Error(LABEL, "Read result failed, code is: %{public}d.",
            static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_PROXY_RUNNING_LOCK_IPC));
        return errCode;
    }

    return ERR_OK;
}

int32_t PowerMgrServiceTestProxy::ProxyRunningLocksIpc(bool isProxied, const VectorPair& vectorPairInfos)
{
    RETURN_IF_WITH_RET(stub_ == nullptr, ERR_INVALID_DATA);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        HiLog::Error(LABEL, "Write interface token failed!");
        return ERR_INVALID_VALUE;
    }

    if (!data.WriteInt32(isProxied ? 1 : 0)) {
        HiLog::Error(LABEL, "Write [isProxied] failed!");
        return ERR_INVALID_DATA;
    }
    if (!data.WriteParcelable(&vectorPairInfos)) {
        HiLog::Error(LABEL, "Write [vectorPairInfos] failed!");
        return ERR_INVALID_DATA;
    }

    int32_t result = stub_->OnRemoteRequest(
        static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_PROXY_RUNNING_LOCKS_IPC), data, reply, option);
    if (FAILED(result)) {
        HiLog::Error(LABEL, "Send request failed!");
        return result;
    }

    ErrCode errCode = reply.ReadInt32();
    if (FAILED(errCode)) {
        HiLog::Error(LABEL, "Read result failed, code is: %{public}d.",
            static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_PROXY_RUNNING_LOCKS_IPC));
        return errCode;
    }

    return ERR_OK;
}

int32_t PowerMgrServiceTestProxy::ResetRunningLocksIpc()
{
    RETURN_IF_WITH_RET(stub_ == nullptr, ERR_INVALID_DATA);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        HiLog::Error(LABEL, "Write interface token failed!");
        return ERR_INVALID_VALUE;
    }

    int32_t result = stub_->OnRemoteRequest(
        static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_RESET_RUNNING_LOCKS_IPC), data, reply, option);
    if (FAILED(result)) {
        HiLog::Error(LABEL, "Send request failed!");
        return result;
    }

    ErrCode errCode = reply.ReadInt32();
    if (FAILED(errCode)) {
        HiLog::Error(LABEL, "Read result failed, code is: %{public}d.",
            static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_RESET_RUNNING_LOCKS_IPC));
        return errCode;
    }

    return ERR_OK;
}

int32_t PowerMgrServiceTestProxy::RebootDeviceIpc(const std::string& reason, int32_t& powerError)
{
    RETURN_IF_WITH_RET(stub_ == nullptr, ERR_INVALID_DATA);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        HiLog::Error(LABEL, "Write interface token failed!");
        return ERR_INVALID_VALUE;
    }

    if (!data.WriteString16(Str8ToStr16(reason))) {
        HiLog::Error(LABEL, "Write [reason] failed!");
        return ERR_INVALID_DATA;
    }

    int32_t result = stub_->OnRemoteRequest(
        static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_REBOOT_DEVICE_IPC), data, reply, option);
    if (FAILED(result)) {
        HiLog::Error(LABEL, "Send request failed!");
        return result;
    }

    ErrCode errCode = reply.ReadInt32();
    if (FAILED(errCode)) {
        HiLog::Error(LABEL, "Read result failed, code is: %{public}d.",
            static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_REBOOT_DEVICE_IPC));
        return errCode;
    }

    powerError = reply.ReadInt32();
    return ERR_OK;
}

int32_t PowerMgrServiceTestProxy::ShutDownDeviceIpc(const std::string& reason, int32_t& powerError)
{
    RETURN_IF_WITH_RET(stub_ == nullptr, ERR_INVALID_DATA);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        HiLog::Error(LABEL, "Write interface token failed!");
        return ERR_INVALID_VALUE;
    }

    if (!data.WriteString16(Str8ToStr16(reason))) {
        HiLog::Error(LABEL, "Write [reason] failed!");
        return ERR_INVALID_DATA;
    }

    int32_t result = stub_->OnRemoteRequest(
        static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_SHUT_DOWN_DEVICE_IPC), data, reply, option);
    if (FAILED(result)) {
        HiLog::Error(LABEL, "Send request failed!");
        return result;
    }

    ErrCode errCode = reply.ReadInt32();
    if (FAILED(errCode)) {
        HiLog::Error(LABEL, "Read result failed, code is: %{public}d.",
            static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_SHUT_DOWN_DEVICE_IPC));
        return errCode;
    }

    powerError = reply.ReadInt32();
    return ERR_OK;
}

int32_t PowerMgrServiceTestProxy::SuspendDeviceIpc(int64_t callTimeMs, int32_t reasonValue,
    bool suspendImmed, const std::string& apiVersion, int32_t& powerError)
{
    RETURN_IF_WITH_RET(stub_ == nullptr, ERR_INVALID_DATA);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        HiLog::Error(LABEL, "Write interface token failed!");
        return ERR_INVALID_VALUE;
    }

    if (!data.WriteInt64(callTimeMs)) {
        HiLog::Error(LABEL, "Write [callTimeMs] failed!");
        return ERR_INVALID_DATA;
    }
    if (!data.WriteInt32(reasonValue)) {
        HiLog::Error(LABEL, "Write [reasonValue] failed!");
        return ERR_INVALID_DATA;
    }
    if (!data.WriteInt32(suspendImmed ? 1 : 0)) {
        HiLog::Error(LABEL, "Write [suspendImmed] failed!");
        return ERR_INVALID_DATA;
    }
    if (!data.WriteString16(Str8ToStr16(apiVersion))) {
        HiLog::Error(LABEL, "Write [apiVersion] failed!");
        return ERR_INVALID_DATA;
    }

    int32_t result = stub_->OnRemoteRequest(
        static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_SUSPEND_DEVICE_IPC), data, reply, option);
    if (FAILED(result)) {
        HiLog::Error(LABEL, "Send request failed!");
        return result;
    }

    ErrCode errCode = reply.ReadInt32();
    if (FAILED(errCode)) {
        HiLog::Error(LABEL, "Read result failed, code is: %{public}d.",
            static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_SUSPEND_DEVICE_IPC));
        return errCode;
    }

    powerError = reply.ReadInt32();
    return ERR_OK;
}

int32_t PowerMgrServiceTestProxy::WakeupDeviceIpc(int64_t callTimeMs, int32_t reasonValue,
    const std::string& details, const std::string& apiVersion, int32_t& powerError)
{
    RETURN_IF_WITH_RET(stub_ == nullptr, ERR_INVALID_DATA);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        HiLog::Error(LABEL, "Write interface token failed!");
        return ERR_INVALID_VALUE;
    }

    if (!data.WriteInt64(callTimeMs)) {
        HiLog::Error(LABEL, "Write [callTimeMs] failed!");
        return ERR_INVALID_DATA;
    }
    if (!data.WriteInt32(reasonValue)) {
        HiLog::Error(LABEL, "Write [reasonValue] failed!");
        return ERR_INVALID_DATA;
    }
    if (!data.WriteString16(Str8ToStr16(details))) {
        HiLog::Error(LABEL, "Write [details] failed!");
        return ERR_INVALID_DATA;
    }
    if (!data.WriteString16(Str8ToStr16(apiVersion))) {
        HiLog::Error(LABEL, "Write [apiVersion] failed!");
        return ERR_INVALID_DATA;
    }

    int32_t result = stub_->OnRemoteRequest(
        static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_WAKEUP_DEVICE_IPC), data, reply, option);
    if (FAILED(result)) {
        HiLog::Error(LABEL, "Send request failed!");
        return result;
    }

    ErrCode errCode = reply.ReadInt32();
    if (FAILED(errCode)) {
        HiLog::Error(LABEL, "Read result failed, code is: %{public}d.",
            static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_WAKEUP_DEVICE_IPC));
        return errCode;
    }

    powerError = reply.ReadInt32();
    return ERR_OK;
}

int32_t PowerMgrServiceTestProxy::RefreshActivityIpc(int64_t callTimeMs, int32_t activityType, bool needChangeBacklight)
{
    RETURN_IF_WITH_RET(stub_ == nullptr, ERR_INVALID_DATA);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        HiLog::Error(LABEL, "Write interface token failed!");
        return ERR_INVALID_VALUE;
    }

    if (!data.WriteInt64(callTimeMs)) {
        HiLog::Error(LABEL, "Write [callTimeMs] failed!");
        return ERR_INVALID_DATA;
    }
    if (!data.WriteInt32(activityType)) {
        HiLog::Error(LABEL, "Write [activityType] failed!");
        return ERR_INVALID_DATA;
    }
    if (!data.WriteInt32(needChangeBacklight ? 1 : 0)) {
        HiLog::Error(LABEL, "Write [needChangeBacklight] failed!");
        return ERR_INVALID_DATA;
    }

    uint32_t code = static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_REFRESH_ACTIVITY_IPC);
    int32_t result = stub_->OnRemoteRequest(code, data, reply, option);
    if (FAILED(result)) {
        HiLog::Error(LABEL, "Send request failed!");
        return result;
    }

    ErrCode errCode = reply.ReadInt32();
    if (FAILED(errCode)) {
        HiLog::Error(LABEL, "Read result failed, code is: %{public}d.", code);
        return errCode;
    }

    return ERR_OK;
}

int32_t PowerMgrServiceTestProxy::OverrideScreenOffTimeIpc(int64_t timeout, int32_t& powerError)
{
    RETURN_IF_WITH_RET(stub_ == nullptr, ERR_INVALID_DATA);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        HiLog::Error(LABEL, "Write interface token failed!");
        return ERR_INVALID_VALUE;
    }

    if (!data.WriteInt64(timeout)) {
        HiLog::Error(LABEL, "Write [timeout] failed!");
        return ERR_INVALID_DATA;
    }

    int32_t result = stub_->OnRemoteRequest(
        static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_OVERRIDE_SCREEN_OFF_TIME_IPC), data, reply, option);
    if (FAILED(result)) {
        HiLog::Error(LABEL, "Send request failed!");
        return result;
    }

    ErrCode errCode = reply.ReadInt32();
    if (FAILED(errCode)) {
        HiLog::Error(LABEL, "Read result failed, code is: %{public}d.",
            static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_OVERRIDE_SCREEN_OFF_TIME_IPC));
        return errCode;
    }

    powerError = reply.ReadInt32();
    return ERR_OK;
}

int32_t PowerMgrServiceTestProxy::RestoreScreenOffTimeIpc(const std::string& apiVersion, int32_t& powerError)
{
    RETURN_IF_WITH_RET(stub_ == nullptr, ERR_INVALID_DATA);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        HiLog::Error(LABEL, "Write interface token failed!");
        return ERR_INVALID_VALUE;
    }

    if (!data.WriteString16(Str8ToStr16(apiVersion))) {
        HiLog::Error(LABEL, "Write [apiVersion] failed!");
        return ERR_INVALID_DATA;
    }

    int32_t result = stub_->OnRemoteRequest(
        static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_RESTORE_SCREEN_OFF_TIME_IPC), data, reply, option);
    if (FAILED(result)) {
        HiLog::Error(LABEL, "Send request failed!");
        return result;
    }

    ErrCode errCode = reply.ReadInt32();
    if (FAILED(errCode)) {
        HiLog::Error(LABEL, "Read result failed, code is: %{public}d.",
            static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_RESTORE_SCREEN_OFF_TIME_IPC));
        return errCode;
    }

    powerError = reply.ReadInt32();
    return ERR_OK;
}

int32_t PowerMgrServiceTestProxy::ForceSuspendDeviceIpc(int64_t callTimeMs)
{
    RETURN_IF_WITH_RET(stub_ == nullptr, ERR_INVALID_DATA);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        HiLog::Error(LABEL, "Write interface token failed!");
        return ERR_INVALID_VALUE;
    }

    if (!data.WriteInt64(callTimeMs)) {
        HiLog::Error(LABEL, "Write [callTimeMs] failed!");
        return ERR_INVALID_DATA;
    }

    int32_t result = stub_->OnRemoteRequest(
        static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_FORCE_SUSPEND_DEVICE_IPC), data, reply, option);
    if (FAILED(result)) {
        HiLog::Error(LABEL, "Send request failed!");
        return result;
    }
    int32_t error = ERR_OK;
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(reply, Int32, error, ERR_OK);
    return error;
}

int32_t PowerMgrServiceTestProxy::GetStateIpc(int32_t& powerState)
{
    RETURN_IF_WITH_RET(stub_ == nullptr, ERR_INVALID_DATA);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        HiLog::Error(LABEL, "Write interface token failed!");
        return ERR_INVALID_VALUE;
    }

    int32_t result = stub_->OnRemoteRequest(
        static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_GET_STATE_IPC), data, reply, option);
    if (FAILED(result)) {
        HiLog::Error(LABEL, "Send request failed!");
        return result;
    }

    ErrCode errCode = reply.ReadInt32();
    if (FAILED(errCode)) {
        HiLog::Error(LABEL, "Read result failed, code is: %{public}d.",
            static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_GET_STATE_IPC));
        return errCode;
    }

    powerState = reply.ReadInt32();
    return ERR_OK;
}

int32_t PowerMgrServiceTestProxy::IsScreenOnIpc(bool needPrintLog, bool& isScreenOn)
{
    RETURN_IF_WITH_RET(stub_ == nullptr, ERR_INVALID_DATA);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        HiLog::Error(LABEL, "Write interface token failed!");
        return ERR_INVALID_VALUE;
    }

    if (!data.WriteInt32(needPrintLog ? 1 : 0)) {
        HiLog::Error(LABEL, "Write [needPrintLog] failed!");
        return ERR_INVALID_DATA;
    }

    int32_t result = stub_->OnRemoteRequest(
        static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_IS_SCREEN_ON_IPC), data, reply, option);
    if (FAILED(result)) {
        HiLog::Error(LABEL, "Send request failed!");
        return result;
    }

    ErrCode errCode = reply.ReadInt32();
    if (FAILED(errCode)) {
        HiLog::Error(LABEL, "Read result failed, code is: %{public}d.",
            static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_IS_SCREEN_ON_IPC));
        return errCode;
    }

    isScreenOn = reply.ReadInt32() == 1 ? true : false;
    return ERR_OK;
}

int32_t PowerMgrServiceTestProxy::IsForceSleepingIpc(bool& isForceSleeping)
{
    RETURN_IF_WITH_RET(stub_ == nullptr, ERR_INVALID_DATA);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        HiLog::Error(LABEL, "Write descriptor failed");
        return ERR_INVALID_VALUE;
    }

    int32_t result = stub_->OnRemoteRequest(
        static_cast<int>(IPowerMgrIpcCode::COMMAND_IS_FORCE_SLEEPING_IPC), data, reply, option);
    if (FAILED(result)) {
        HiLog::Error(LABEL, "Send request failed!");
        return result;
    }

    ErrCode errCode = reply.ReadInt32();
    if (FAILED(errCode)) {
        HiLog::Error(LABEL, "Read result failed, code is: %{public}d.",
            static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_IS_FORCE_SLEEPING_IPC));
        return errCode;
    }

    isForceSleeping = reply.ReadInt32() == 1 ? true : false;
    return ERR_OK;
}

int32_t PowerMgrServiceTestProxy::RegisterPowerStateCallbackIpc(
    const sptr<IPowerStateCallback>& powerCallback, bool isSync)
{
    RETURN_IF_WITH_RET(stub_ == nullptr, ERR_INVALID_DATA);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        HiLog::Error(LABEL, "Write interface token failed!");
        return ERR_INVALID_VALUE;
    }

    if (powerCallback == nullptr) {
        HiLog::Error(LABEL, "powerCallback is nullptr!");
        return ERR_INVALID_DATA;
    }
    if (!data.WriteRemoteObject(powerCallback->AsObject())) {
        HiLog::Error(LABEL, "Write [powerCallback] failed!");
        return ERR_INVALID_DATA;
    }
    if (!data.WriteInt32(isSync ? 1 : 0)) {
        HiLog::Error(LABEL, "Write [isSync] failed!");
        return ERR_INVALID_DATA;
    }

    int32_t result = stub_->OnRemoteRequest(
        static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_REGISTER_POWER_STATE_CALLBACK_IPC), data, reply, option);
    if (FAILED(result)) {
        HiLog::Error(LABEL, "Send request failed!");
        return result;
    }

    ErrCode errCode = reply.ReadInt32();
    if (FAILED(errCode)) {
        HiLog::Error(LABEL, "Read result failed, code is: %{public}d.",
            static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_REGISTER_POWER_STATE_CALLBACK_IPC));
        return errCode;
    }

    return ERR_OK;
}

int32_t PowerMgrServiceTestProxy::UnRegisterPowerStateCallbackIpc(const sptr<IPowerStateCallback>& powerCallback)
{
    RETURN_IF_WITH_RET(stub_ == nullptr, ERR_INVALID_DATA);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        HiLog::Error(LABEL, "Write interface token failed!");
        return ERR_INVALID_VALUE;
    }

    if (powerCallback == nullptr) {
        HiLog::Error(LABEL, "powerCallback is nullptr!");
        return ERR_INVALID_DATA;
    }
    if (!data.WriteRemoteObject(powerCallback->AsObject())) {
        HiLog::Error(LABEL, "Write [powerCallback] failed!");
        return ERR_INVALID_DATA;
    }

    int32_t result = stub_->OnRemoteRequest(
        static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_UN_REGISTER_POWER_STATE_CALLBACK_IPC), data, reply, option);
    if (FAILED(result)) {
        HiLog::Error(LABEL, "Send request failed!");
        return result;
    }

    ErrCode errCode = reply.ReadInt32();
    if (FAILED(errCode)) {
        HiLog::Error(LABEL, "Read result failed, code is: %{public}d.",
            static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_UN_REGISTER_POWER_STATE_CALLBACK_IPC));
        return errCode;
    }

    return ERR_OK;
}

int32_t PowerMgrServiceTestProxy::RegisterPowerModeCallbackIpc(const sptr<IPowerModeCallback>& powerCallback)
{
    RETURN_IF_WITH_RET(stub_ == nullptr, ERR_INVALID_DATA);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        HiLog::Error(LABEL, "Write interface token failed!");
        return ERR_INVALID_VALUE;
    }

    if (powerCallback == nullptr) {
        HiLog::Error(LABEL, "powerCallback is nullptr!");
        return ERR_INVALID_DATA;
    }
    if (!data.WriteRemoteObject(powerCallback->AsObject())) {
        HiLog::Error(LABEL, "Write [powerCallback] failed!");
        return ERR_INVALID_DATA;
    }

    int32_t result = stub_->OnRemoteRequest(
        static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_REGISTER_POWER_MODE_CALLBACK_IPC), data, reply, option);
    if (FAILED(result)) {
        HiLog::Error(LABEL, "Send request failed!");
        return result;
    }

    ErrCode errCode = reply.ReadInt32();
    if (FAILED(errCode)) {
        HiLog::Error(LABEL, "Read result failed, code is: %{public}d.",
            static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_REGISTER_POWER_MODE_CALLBACK_IPC));
        return errCode;
    }

    return ERR_OK;
}

int32_t PowerMgrServiceTestProxy::UnRegisterPowerModeCallbackIpc(const sptr<IPowerModeCallback>& powerCallback)
{
    RETURN_IF_WITH_RET(stub_ == nullptr, ERR_INVALID_DATA);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        HiLog::Error(LABEL, "Write interface token failed!");
        return ERR_INVALID_VALUE;
    }

    if (powerCallback == nullptr) {
        HiLog::Error(LABEL, "powerCallback is nullptr!");
        return ERR_INVALID_DATA;
    }
    if (!data.WriteRemoteObject(powerCallback->AsObject())) {
        HiLog::Error(LABEL, "Write [powerCallback] failed!");
        return ERR_INVALID_DATA;
    }

    int32_t result = stub_->OnRemoteRequest(
        static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_UN_REGISTER_POWER_MODE_CALLBACK_IPC), data, reply, option);
    if (FAILED(result)) {
        HiLog::Error(LABEL, "Send request failed!");
        return result;
    }

    ErrCode errCode = reply.ReadInt32();
    if (FAILED(errCode)) {
        HiLog::Error(LABEL, "Read result failed, code is: %{public}d.",
            static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_UN_REGISTER_POWER_MODE_CALLBACK_IPC));
        return errCode;
    }

    return ERR_OK;
}

int32_t PowerMgrServiceTestProxy::RegisterScreenStateCallbackIpc(
    int32_t remainTime, const sptr<IScreenOffPreCallback>& powerCallback)
{
    RETURN_IF_WITH_RET(stub_ == nullptr, ERR_INVALID_DATA);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        HiLog::Error(LABEL, "Write interface token failed!");
        return ERR_INVALID_VALUE;
    }

    if (!data.WriteInt32(remainTime)) {
        HiLog::Error(LABEL, "Write [remainTime] failed!");
        return ERR_INVALID_DATA;
    }
    if (powerCallback == nullptr) {
        HiLog::Error(LABEL, "powerCallback is nullptr!");
        return ERR_INVALID_DATA;
    }
    if (!data.WriteRemoteObject(powerCallback->AsObject())) {
        HiLog::Error(LABEL, "Write [powerCallback] failed!");
        return ERR_INVALID_DATA;
    }

    int32_t result = stub_->OnRemoteRequest(
        static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_REGISTER_SCREEN_STATE_CALLBACK_IPC), data, reply, option);
    if (FAILED(result)) {
        HiLog::Error(LABEL, "Send request failed!");
        return result;
    }

    ErrCode errCode = reply.ReadInt32();
    if (FAILED(errCode)) {
        HiLog::Error(LABEL, "Read result failed, code is: %{public}d.",
            static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_REGISTER_SCREEN_STATE_CALLBACK_IPC));
        return errCode;
    }

    return ERR_OK;
}

int32_t PowerMgrServiceTestProxy::UnRegisterScreenStateCallbackIpc(const sptr<IScreenOffPreCallback>& powerCallback)
{
    RETURN_IF_WITH_RET(stub_ == nullptr, ERR_INVALID_DATA);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        HiLog::Error(LABEL, "Write interface token failed!");
        return ERR_INVALID_VALUE;
    }

    if (powerCallback == nullptr) {
        HiLog::Error(LABEL, "powerCallback is nullptr!");
        return ERR_INVALID_DATA;
    }
    if (!data.WriteRemoteObject(powerCallback->AsObject())) {
        HiLog::Error(LABEL, "Write [powerCallback] failed!");
        return ERR_INVALID_DATA;
    }

    int32_t result = stub_->OnRemoteRequest(
        static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_UN_REGISTER_SCREEN_STATE_CALLBACK_IPC), data, reply, option);
    if (FAILED(result)) {
        HiLog::Error(LABEL, "Send request failed!");
        return result;
    }

    ErrCode errCode = reply.ReadInt32();
    if (FAILED(errCode)) {
        HiLog::Error(LABEL, "Read result failed, code is: %{public}d.",
            static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_UN_REGISTER_SCREEN_STATE_CALLBACK_IPC));
        return errCode;
    }

    return ERR_OK;
}

int32_t PowerMgrServiceTestProxy::RegisterRunningLockCallbackIpc(const sptr<IPowerRunninglockCallback>& powerCallback)
{
    RETURN_IF_WITH_RET(stub_ == nullptr, ERR_INVALID_DATA);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        HiLog::Error(LABEL, "Write interface token failed!");
        return ERR_INVALID_VALUE;
    }

    if (powerCallback == nullptr) {
        HiLog::Error(LABEL, "powerCallback is nullptr!");
        return ERR_INVALID_DATA;
    }
    if (!data.WriteRemoteObject(powerCallback->AsObject())) {
        HiLog::Error(LABEL, "Write [powerCallback] failed!");
        return ERR_INVALID_DATA;
    }

    int32_t result = stub_->OnRemoteRequest(
        static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_REGISTER_RUNNING_LOCK_CALLBACK_IPC), data, reply, option);
    if (FAILED(result)) {
        HiLog::Error(LABEL, "Send request failed!");
        return result;
    }

    ErrCode errCode = reply.ReadInt32();
    if (FAILED(errCode)) {
        HiLog::Error(LABEL, "Read result failed, code is: %{public}d.",
            static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_REGISTER_RUNNING_LOCK_CALLBACK_IPC));
        return errCode;
    }

    return ERR_OK;
}

int32_t PowerMgrServiceTestProxy::UnRegisterRunningLockCallbackIpc(const sptr<IPowerRunninglockCallback>& powerCallback)
{
    RETURN_IF_WITH_RET(stub_ == nullptr, ERR_INVALID_DATA);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        HiLog::Error(LABEL, "Write interface token failed!");
        return ERR_INVALID_VALUE;
    }

    if (powerCallback == nullptr) {
        HiLog::Error(LABEL, "powerCallback is nullptr!");
        return ERR_INVALID_DATA;
    }
    if (!data.WriteRemoteObject(powerCallback->AsObject())) {
        HiLog::Error(LABEL, "Write [powerCallback] failed!");
        return ERR_INVALID_DATA;
    }

    int32_t result = stub_->OnRemoteRequest(
        static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_UN_REGISTER_RUNNING_LOCK_CALLBACK_IPC), data, reply, option);
    if (FAILED(result)) {
        HiLog::Error(LABEL, "Send request failed!");
        return result;
    }

    ErrCode errCode = reply.ReadInt32();
    if (FAILED(errCode)) {
        HiLog::Error(LABEL, "Read result failed, code is: %{public}d.",
            static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_UN_REGISTER_RUNNING_LOCK_CALLBACK_IPC));
        return errCode;
    }

    return ERR_OK;
}

int32_t PowerMgrServiceTestProxy::SetDisplaySuspendIpc(bool enable)
{
    RETURN_IF_WITH_RET(stub_ == nullptr, ERR_INVALID_DATA);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        HiLog::Error(LABEL, "Write interface token failed!");
        return ERR_INVALID_VALUE;
    }

    if (!data.WriteInt32(enable ? 1 : 0)) {
        HiLog::Error(LABEL, "Write [enable] failed!");
        return ERR_INVALID_DATA;
    }

    int32_t result = stub_->OnRemoteRequest(
        static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_SET_DISPLAY_SUSPEND_IPC), data, reply, option);
    if (FAILED(result)) {
        HiLog::Error(LABEL, "Send request failed!");
        return result;
    }

    ErrCode errCode = reply.ReadInt32();
    if (FAILED(errCode)) {
        HiLog::Error(LABEL, "Read result failed, code is: %{public}d.",
            static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_SET_DISPLAY_SUSPEND_IPC));
        return errCode;
    }

    return ERR_OK;
}

int32_t PowerMgrServiceTestProxy::SetDeviceModeIpc(int32_t modeValue, int32_t& powerError)
{
    RETURN_IF_WITH_RET(stub_ == nullptr, ERR_INVALID_DATA);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        HiLog::Error(LABEL, "Write interface token failed!");
        return ERR_INVALID_VALUE;
    }

    if (!data.WriteInt32(modeValue)) {
        HiLog::Error(LABEL, "Write [modeValue] failed!");
        return ERR_INVALID_DATA;
    }

    int32_t result = stub_->OnRemoteRequest(
        static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_SET_DEVICE_MODE_IPC), data, reply, option);
    if (FAILED(result)) {
        HiLog::Error(LABEL, "Send request failed!");
        return result;
    }

    ErrCode errCode = reply.ReadInt32();
    if (FAILED(errCode)) {
        HiLog::Error(LABEL, "Read result failed, code is: %{public}d.",
            static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_SET_DEVICE_MODE_IPC));
        return errCode;
    }

    powerError = reply.ReadInt32();
    return ERR_OK;
}

int32_t PowerMgrServiceTestProxy::GetDeviceModeIpc(int32_t& powerMode)
{
    RETURN_IF_WITH_RET(stub_ == nullptr, ERR_INVALID_DATA);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        HiLog::Error(LABEL, "Write interface token failed!");
        return ERR_INVALID_VALUE;
    }

    int32_t result = stub_->OnRemoteRequest(
        static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_GET_DEVICE_MODE_IPC), data, reply, option);
    if (FAILED(result)) {
        HiLog::Error(LABEL, "Send request failed!");
        return result;
    }

    ErrCode errCode = reply.ReadInt32();
    if (FAILED(errCode)) {
        HiLog::Error(LABEL, "Read result failed, code is: %{public}d.",
            static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_GET_DEVICE_MODE_IPC));
        return errCode;
    }

    powerMode = reply.ReadInt32();
    return ERR_OK;
}

int32_t PowerMgrServiceTestProxy::ShellDumpIpc(const std::vector<std::string>& args,
    uint32_t argc, std::string& returnDump)
{
    RETURN_IF_WITH_RET(stub_ == nullptr, ERR_INVALID_DATA);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        HiLog::Error(LABEL, "Write interface token failed!");
        return ERR_INVALID_VALUE;
    }

    if (args.size() > static_cast<size_t>(VECTOR_MAX_SIZE)) {
        HiLog::Error(LABEL, "The vector/array size exceeds the security limit!");
        return ERR_INVALID_DATA;
    }
    data.WriteInt32(args.size());
    for (auto it3 = args.begin(); it3 != args.end(); ++it3) {
        if (!data.WriteString16(Str8ToStr16((*it3)))) {
            HiLog::Error(LABEL, "Write [(*it3)] failed!");
            return ERR_INVALID_DATA;
        }
    }
    if (!data.WriteUint32(argc)) {
        HiLog::Error(LABEL, "Write [argc] failed!");
        return ERR_INVALID_DATA;
    }

    int32_t result = stub_->OnRemoteRequest(
        static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_SHELL_DUMP_IPC), data, reply, option);
    if (FAILED(result)) {
        HiLog::Error(LABEL, "Send request failed!");
        return result;
    }

    ErrCode errCode = reply.ReadInt32();
    if (FAILED(errCode)) {
        HiLog::Error(LABEL, "Read result failed, code is: %{public}d.",
            static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_SHELL_DUMP_IPC));
        return errCode;
    }

    returnDump = Str16ToStr8(reply.ReadString16());
    return ERR_OK;
}

int32_t PowerMgrServiceTestProxy::IsStandbyIpc(bool& isStandby, int32_t& powerError)
{
    RETURN_IF_WITH_RET(stub_ == nullptr, ERR_INVALID_DATA);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        HiLog::Error(LABEL, "Write interface token failed!");
        return ERR_INVALID_VALUE;
    }

    int32_t result = stub_->OnRemoteRequest(
        static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_IS_STANDBY_IPC), data, reply, option);
    if (FAILED(result)) {
        HiLog::Error(LABEL, "Send request failed!");
        return result;
    }

    ErrCode errCode = reply.ReadInt32();
    if (FAILED(errCode)) {
        HiLog::Error(LABEL, "Read result failed, code is: %{public}d.",
            static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_IS_STANDBY_IPC));
        return errCode;
    }

    isStandby = reply.ReadInt32() == 1 ? true : false;
    powerError = reply.ReadInt32();
    return ERR_OK;
}
} // namespace PowerMgr
} // namespace OHOS
