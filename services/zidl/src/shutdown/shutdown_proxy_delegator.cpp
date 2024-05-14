/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "shutdown_proxy_delegator.h"

#include "power_common.h"
#include "shutdown/shutdown_client_ipc_interface_code.h"

namespace OHOS {
namespace PowerMgr {
void ShutdownProxyDelegator::RegisterShutdownCallback(
    const sptr<ITakeOverShutdownCallback>& callback, ShutdownPriority priority)
{
    RETURN_IF((remote_ == nullptr) || (callback == nullptr))
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(descriptor_)) {
        POWER_HILOGE(FEATURE_SHUTDOWN, "Write descriptor failed");
        return;
    }

    RETURN_IF_WRITE_PARCEL_FAILED_NO_RET(data, RemoteObject, callback->AsObject());
    RETURN_IF_WRITE_PARCEL_FAILED_NO_RET(data, Uint32, static_cast<uint32_t>(priority));

    int ret = remote_->SendRequest(
        static_cast<int>(PowerMgr::ShutdownClientInterfaceCode::CMD_REG_TAKEOVER_SHUTDOWN_CALLBACK),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_SHUTDOWN, "SendRequest is failed, ret=%{public}d", ret);
    }
}

void ShutdownProxyDelegator::UnRegisterShutdownCallback(const sptr<ITakeOverShutdownCallback>& callback)
{
    RETURN_IF((remote_ == nullptr) || (callback == nullptr))
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(descriptor_)) {
        POWER_HILOGE(FEATURE_SHUTDOWN, "Write descriptor failed");
        return;
    }

    RETURN_IF_WRITE_PARCEL_FAILED_NO_RET(data, RemoteObject, callback->AsObject());

    int ret = remote_->SendRequest(
        static_cast<int>(PowerMgr::ShutdownClientInterfaceCode::CMD_UNREG_TAKEOVER_SHUTDOWN_CALLBACK),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_SHUTDOWN, "SendRequest is failed, ret=%{public}d", ret);
    }
}

void ShutdownProxyDelegator::RegisterShutdownCallback(
    const sptr<IAsyncShutdownCallback>& callback, ShutdownPriority priority)
{
    RETURN_IF((remote_ == nullptr) || (callback == nullptr))
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(descriptor_)) {
        POWER_HILOGE(FEATURE_SHUTDOWN, "Write descriptor failed");
        return;
    }

    RETURN_IF_WRITE_PARCEL_FAILED_NO_RET(data, RemoteObject, callback->AsObject());
    RETURN_IF_WRITE_PARCEL_FAILED_NO_RET(data, Uint32, static_cast<uint32_t>(priority));

    int ret = remote_->SendRequest(
        static_cast<int>(PowerMgr::ShutdownClientInterfaceCode::CMD_REG_ASYNC_SHUTDOWN_CALLBACK),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_SHUTDOWN, "SendRequest is failed, ret=%{public}d", ret);
    }
}

void ShutdownProxyDelegator::UnRegisterShutdownCallback(const sptr<IAsyncShutdownCallback>& callback)
{
    RETURN_IF((remote_ == nullptr) || (callback == nullptr))
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(descriptor_)) {
        POWER_HILOGE(FEATURE_SHUTDOWN, "Write descriptor failed");
        return;
    }

    RETURN_IF_WRITE_PARCEL_FAILED_NO_RET(data, RemoteObject, callback->AsObject());

    int ret = remote_->SendRequest(
        static_cast<int>(PowerMgr::ShutdownClientInterfaceCode::CMD_UNREG_ASYNC_SHUTDOWN_CALLBACK),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_SHUTDOWN, "SendRequest is failed, ret=%{public}d", ret);
    }
}

void ShutdownProxyDelegator::RegisterShutdownCallback(
    const sptr<ISyncShutdownCallback>& callback, ShutdownPriority priority)
{
    RETURN_IF((remote_ == nullptr) || (callback == nullptr))
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(descriptor_)) {
        POWER_HILOGE(FEATURE_SHUTDOWN, "Write descriptor failed");
        return;
    }

    RETURN_IF_WRITE_PARCEL_FAILED_NO_RET(data, RemoteObject, callback->AsObject());
    RETURN_IF_WRITE_PARCEL_FAILED_NO_RET(data, Uint32, static_cast<uint32_t>(priority));

    int ret = remote_->SendRequest(
        static_cast<int>(PowerMgr::ShutdownClientInterfaceCode::CMD_REG_SYNC_SHUTDOWN_CALLBACK),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_SHUTDOWN, "SendRequest is failed, ret=%{public}d", ret);
    }
}

void ShutdownProxyDelegator::UnRegisterShutdownCallback(const sptr<ISyncShutdownCallback>& callback)
{
    RETURN_IF((remote_ == nullptr) || (callback == nullptr))
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(descriptor_)) {
        POWER_HILOGE(FEATURE_SHUTDOWN, "Write descriptor failed");
        return;
    }

    RETURN_IF_WRITE_PARCEL_FAILED_NO_RET(data, RemoteObject, callback->AsObject());

    int ret = remote_->SendRequest(
        static_cast<int>(PowerMgr::ShutdownClientInterfaceCode::CMD_UNREG_SYNC_SHUTDOWN_CALLBACK),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_SHUTDOWN, "SendRequest is failed, ret=%{public}d", ret);
    }
}
} // namespace PowerMgr
} // namespace OHOS
