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

#include "shutdown_stub_delegator.h"

#include "power_common.h"
#include "shutdown/shutdown_client_ipc_interface_code.h"
#include "xcollie/xcollie.h"

namespace OHOS {
namespace PowerMgr {
namespace {
constexpr int32_t DFX_DELAY_MS = 10000;

}
int32_t ShutdownStubDelegator::HandleRemoteRequest(
    uint32_t code, MessageParcel& data, [[maybe_unused]] MessageParcel& reply, [[maybe_unused]] MessageOption& option)
{
    int32_t id = HiviewDFX::XCollie::GetInstance().SetTimer("ShutdownStub", DFX_DELAY_MS, nullptr, nullptr,
        HiviewDFX::XCOLLIE_FLAG_NOOP);
    int32_t ret = ERR_OK;
    switch (code) {
        case static_cast<int32_t>(PowerMgr::ShutdownClientInterfaceCode::CMD_REG_TAKEOVER_SHUTDOWN_CALLBACK):
            ret = RegisterTakeOverShutdownCallback(data);
            break;
        case static_cast<int32_t>(PowerMgr::ShutdownClientInterfaceCode::CMD_UNREG_TAKEOVER_SHUTDOWN_CALLBACK):
            ret = UnRegisterTakeOverShutdownCallback(data);
            break;
        case static_cast<int32_t>(PowerMgr::ShutdownClientInterfaceCode::CMD_REG_ASYNC_SHUTDOWN_CALLBACK):
            ret = RegisterAsyncShutdownCallback(data);
            break;
        case static_cast<int32_t>(PowerMgr::ShutdownClientInterfaceCode::CMD_UNREG_ASYNC_SHUTDOWN_CALLBACK):
            ret = UnRegisterAsyncShutdownCallback(data);
            break;
        case static_cast<int32_t>(PowerMgr::ShutdownClientInterfaceCode::CMD_REG_SYNC_SHUTDOWN_CALLBACK):
            ret = RegisterSyncShutdownCallback(data);
            break;
        case static_cast<int32_t>(PowerMgr::ShutdownClientInterfaceCode::CMD_UNREG_SYNC_SHUTDOWN_CALLBACK):
            ret = UnRegisterSyncShutdownCallback(data);
            break;
        default:
            ret = ERR_INVALID_OPERATION;
            break;
    }
    HiviewDFX::XCollie::GetInstance().CancelTimer(id);
    return ret;
}

int32_t ShutdownStubDelegator::RegisterTakeOverShutdownCallback(MessageParcel& data)
{
    uint32_t priority;
    sptr<IRemoteObject> obj = data.ReadRemoteObject();
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Uint32, priority, E_READ_PARCEL_ERROR);
    RETURN_IF_WITH_RET((obj == nullptr), E_READ_PARCEL_ERROR);
    auto callback = iface_cast<ITakeOverShutdownCallback>(obj);
    RETURN_IF_WITH_RET((callback == nullptr), E_READ_PARCEL_ERROR);
    stub_.RegisterShutdownCallback(callback, static_cast<ShutdownPriority>(priority));
    return ERR_OK;
}

int32_t ShutdownStubDelegator::UnRegisterTakeOverShutdownCallback(MessageParcel& data)
{
    sptr<IRemoteObject> obj = data.ReadRemoteObject();
    RETURN_IF_WITH_RET((obj == nullptr), E_READ_PARCEL_ERROR);
    auto callback = iface_cast<ITakeOverShutdownCallback>(obj);
    RETURN_IF_WITH_RET((callback == nullptr), E_READ_PARCEL_ERROR);
    stub_.UnRegisterShutdownCallback(callback);
    return ERR_OK;
}

int32_t ShutdownStubDelegator::RegisterAsyncShutdownCallback(MessageParcel& data)
{
    uint32_t priority;
    sptr<IRemoteObject> obj = data.ReadRemoteObject();
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Uint32, priority, E_READ_PARCEL_ERROR);
    RETURN_IF_WITH_RET((obj == nullptr), E_READ_PARCEL_ERROR);
    auto callback = iface_cast<IAsyncShutdownCallback>(obj);
    RETURN_IF_WITH_RET((callback == nullptr), E_READ_PARCEL_ERROR);
    stub_.RegisterShutdownCallback(callback, static_cast<ShutdownPriority>(priority));
    return ERR_OK;
}

int32_t ShutdownStubDelegator::UnRegisterAsyncShutdownCallback(MessageParcel& data)
{
    sptr<IRemoteObject> obj = data.ReadRemoteObject();
    RETURN_IF_WITH_RET((obj == nullptr), E_READ_PARCEL_ERROR);
    auto callback = iface_cast<IAsyncShutdownCallback>(obj);
    RETURN_IF_WITH_RET((callback == nullptr), E_READ_PARCEL_ERROR);
    stub_.UnRegisterShutdownCallback(callback);
    return ERR_OK;
}

int32_t ShutdownStubDelegator::RegisterSyncShutdownCallback(MessageParcel& data)
{
    uint32_t priority;
    sptr<IRemoteObject> obj = data.ReadRemoteObject();
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Uint32, priority, E_READ_PARCEL_ERROR);
    RETURN_IF_WITH_RET((obj == nullptr), E_READ_PARCEL_ERROR);
    auto callback = iface_cast<ISyncShutdownCallback>(obj);
    RETURN_IF_WITH_RET((callback == nullptr), E_READ_PARCEL_ERROR);
    stub_.RegisterShutdownCallback(callback, static_cast<ShutdownPriority>(priority));
    return ERR_OK;
}

int32_t ShutdownStubDelegator::UnRegisterSyncShutdownCallback(MessageParcel& data)
{
    sptr<IRemoteObject> obj = data.ReadRemoteObject();
    RETURN_IF_WITH_RET((obj == nullptr), E_READ_PARCEL_ERROR);
    auto callback = iface_cast<ISyncShutdownCallback>(obj);
    RETURN_IF_WITH_RET((callback == nullptr), E_READ_PARCEL_ERROR);
    stub_.UnRegisterShutdownCallback(callback);
    return ERR_OK;
}
} // namespace PowerMgr
} // namespace OHOS
