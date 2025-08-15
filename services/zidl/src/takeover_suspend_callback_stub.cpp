/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "takeover_suspend_callback_stub.h"
#include "takeover_suspend_callback_proxy.h"
#include "message_parcel.h"
#include "power_common.h"
#include "take_over_callback_ipc_interface_code.h"
#include "xcollie/xcollie.h"
#include "xcollie/xcollie_define.h"

namespace OHOS {
namespace PowerMgr {
int TakeOverSuspendCallbackStub::OnRemoteRequest(
    uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    const int32_t DFX_DELAY_S = 60;
    int32_t id = HiviewDFX::XCollie::GetInstance().SetTimer(
        "PowerSuspendCallback", DFX_DELAY_S, nullptr, nullptr, HiviewDFX::XCOLLIE_FLAG_LOG);
    std::u16string descripter = TakeOverSuspendCallbackStub::GetDescriptor();
    std::u16string remoteDescripter = data.ReadInterfaceToken();
    if (descripter != remoteDescripter) {
        POWER_HILOGE(COMP_SVC, "Descriptor is not match");
        HiviewDFX::XCollie::GetInstance().CancelTimer(id);
        return E_GET_POWER_SERVICE_FAILED;
    }

    int32_t ret = ERR_OK;
    if (code == static_cast<uint32_t>(TakeOverSuspendCallbackInterfaceCode::CMD_ON_TAKEOVER_SUSPEND)) {
        ret = OnTakeOverSuspendCallbackStub(data, reply);
    } else {
        ret = IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    HiviewDFX::XCollie::GetInstance().CancelTimer(id);
    return ret;
}

int32_t TakeOverSuspendCallbackStub::OnTakeOverSuspendCallbackStub(MessageParcel& data, MessageParcel& reply)
{
    uint32_t rawType = 0;
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Uint32, rawType, E_READ_PARCEL_ERROR);
    SuspendDeviceType type = static_cast<SuspendDeviceType>(rawType);
    bool isSuspendDeviceType =
        type >= SuspendDeviceType::SUSPEND_DEVICE_REASON_MIN &&
        type <= SuspendDeviceType::SUSPEND_DEVICE_REASON_MAX;

    if (!isSuspendDeviceType) {
        return E_INNER_ERR;
    }
    bool isTakeOver = OnTakeOverSuspend(type);
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(reply, Bool, isTakeOver, E_WRITE_PARCEL_ERROR);
    return ERR_OK;
}
} // namespace PowerMgr
} // namespace OHOS