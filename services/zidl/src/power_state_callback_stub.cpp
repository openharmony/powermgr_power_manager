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

#include "power_state_callback_stub.h"

#include <message_parcel.h>

#include "power_common.h"
#include "power_state_callback_ipc_interface_code.h"
#include "power_state_callback_proxy.h"
#include "xcollie/xcollie.h"

namespace OHOS {
namespace PowerMgr {
int PowerStateCallbackStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    POWER_HILOGD(COMP_SVC, "cmd = %{public}d, flags= %{public}d", code, option.GetFlags());
    const int DFX_DELAY_S = 60;
    int id = HiviewDFX::XCollie::GetInstance().SetTimer("PowerStateCallbackStub", DFX_DELAY_S, nullptr, nullptr,
        HiviewDFX::XCOLLIE_FLAG_LOG);
    std::u16string descripter = PowerStateCallbackStub::GetDescriptor();
    std::u16string remoteDescripter = data.ReadInterfaceToken();
    if (descripter != remoteDescripter) {
        POWER_HILOGE(COMP_SVC, "Descriptor is not match");
        HiviewDFX::XCollie::GetInstance().CancelTimer(id);
        return E_GET_POWER_SERVICE_FAILED;
    }

    int ret = ERR_OK;
    if (code == static_cast<uint32_t>(PowerMgr::PowerStateCallbackInterfaceCode::POWER_STATE_CHANGED)) {
        ret = OnPowerStateChangedStub(data);
    } else if (code == static_cast<uint32_t>(PowerMgr::PowerStateCallbackInterfaceCode::ASYNC_POWER_STATE_CHANGED)) {
        ret = OnAsyncPowerStateChangedStub(data);
    } else {
        ret = IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    HiviewDFX::XCollie::GetInstance().CancelTimer(id);
    return ret;
}

int32_t PowerStateCallbackStub::OnPowerStateChangedStub(MessageParcel& data)
{
    uint32_t type;
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Uint32, type, E_READ_PARCEL_ERROR);
    OnPowerStateChanged(static_cast<PowerState>(type));
    return ERR_OK;
}

int32_t PowerStateCallbackStub::OnAsyncPowerStateChangedStub(MessageParcel& data)
{
    uint32_t type;
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Uint32, type, E_READ_PARCEL_ERROR);
    OnAsyncPowerStateChanged(static_cast<PowerState>(type));
    return ERR_OK;
}
} // namespace PowerMgr
} // namespace OHOS
