/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "power_runninglock_callback_stub.h"

#include <message_parcel.h>

#include "power_common.h"
#include "power_runninglock_callback_ipc_interface_code.h"
#include "power_runninglock_callback_proxy.h"
#include "xcollie/xcollie.h"

namespace OHOS {
namespace PowerMgr {
int PowerRunningLockCallbackStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    POWER_HILOGD(COMP_SVC, "cmd = %{public}d, flags= %{public}d", code, option.GetFlags());
    const int DFX_DELAY_MS = 10000;
    int id = HiviewDFX::XCollie::GetInstance().SetTimer("PowerRunningLockCallbackStub", DFX_DELAY_MS, nullptr, nullptr,
        HiviewDFX::XCOLLIE_FLAG_NOOP);
    std::u16string descripter = PowerRunningLockCallbackStub::GetDescriptor();
    std::u16string remoteDescripter = data.ReadInterfaceToken();
    if (descripter != remoteDescripter) {
        POWER_HILOGE(COMP_SVC, "Descriptor is not match");
        return E_GET_POWER_SERVICE_FAILED;
    }

    int ret = ERR_OK;
    if (code == static_cast<uint32_t>(PowerMgr::PowerRunningLockCallbackInterfaceCode::POWER_RUNNINGLOCK_CHANGED)) {
        std::string message;
        RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, String, message, E_READ_PARCEL_ERROR);
        HandleRunningLockMessage(message);
    } else {
        ret = IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    HiviewDFX::XCollie::GetInstance().CancelTimer(id);
    return ret;
}
} // namespace PowerMgr
} // namespace OHOS