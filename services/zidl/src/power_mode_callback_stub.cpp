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

#include "power_mode_callback_stub.h"

#include <message_parcel.h>

#include "power_common.h"
#include "power_mode_callback_proxy.h"

namespace OHOS {
namespace PowerMgr {
int PowerModeCallbackStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
    {
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrStub::OnRemoteRequest, cmd = %d, flags= %d", code, option.GetFlags());
    std::u16string descripter = PowerModeCallbackStub::GetDescriptor();
    std::u16string remoteDescripter = data.ReadInterfaceToken();
    if (descripter != remoteDescripter) {
        POWER_HILOGE(MODULE_SERVICE, "PowerMgrStub::OnRemoteRequest failed, descriptor is not matched!");
        return E_GET_POWER_SERVICE_FAILED;
    }

    switch (code) {
        case static_cast<uint32_t>(IPowerModeCallback::POWER_MODE_CHANGED): {
            return OnPowerModeCallbackStub();
        }
        default:
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return ERR_OK;
}

int32_t PowerModeCallbackStub::OnPowerModeCallbackStub()
{
    PowerModeCallback();
    return ERR_OK;
}
} // namespace PowerMgr
} // namespace OHOS
