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

#include "ulsr_callback_stub.h"

#include <message_parcel.h>

#include "power_common.h"
#include "ulsr/ulsr_callback_ipc_interface_code.h"
#include "ulsr_callback_proxy.h"
#include "xcollie/xcollie.h"

namespace OHOS {
namespace PowerMgr {
int UlsrCallbackStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    POWER_HILOGD(COMP_SVC, "cmd = %{public}d, flags= %{public}d", code, option.GetFlags());
    const int DFX_DELAY_S = 60;
    int id = HiviewDFX::XCollie::GetInstance().SetTimer("UlsrCallbackStub", DFX_DELAY_S, nullptr, nullptr,
        HiviewDFX::XCOLLIE_FLAG_LOG);
    std::u16string descripter = UlsrCallbackStub::GetDescriptor();
    std::u16string remoteDescripter = data.ReadInterfaceToken();
    if (descripter != remoteDescripter) {
        POWER_HILOGE(COMP_SVC, "Descriptor is not match");
        HiviewDFX::XCollie::GetInstance().CancelTimer(id);
        return E_GET_POWER_SERVICE_FAILED;
    }

    int ret = ERR_OK;
    switch (static_cast<PowerMgr::UlsrCallbackInterfaceCode>(code)) {
        case PowerMgr::UlsrCallbackInterfaceCode::CMD_ON_SYNC_ULSR:
            ret = OnSyncUlsrStub(data);
            break;
        case PowerMgr::UlsrCallbackInterfaceCode::CMD_ON_ASYNC_WAKEUP:
            ret = OnAsyncWakeupStub(data);
            break;
        default:
            ret = IPCObjectStub::OnRemoteRequest(code, data, reply, option);
            break;
    }
    HiviewDFX::XCollie::GetInstance().CancelTimer(id);
    return ret;
}

int32_t UlsrCallbackStub::OnSyncUlsrStub(MessageParcel& data)
{
    OnSyncUlsr();
    return ERR_OK;
}

int32_t UlsrCallbackStub::OnAsyncWakeupStub(MessageParcel& data)
{
    bool ulsrResult = false;
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Bool, ulsrResult, E_READ_PARCEL_ERROR);
    OnAsyncWakeup(ulsrResult);
    return ERR_OK;
}
} // namespace PowerMgr
} // namespace OHOS