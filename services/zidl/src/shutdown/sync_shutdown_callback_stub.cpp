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

#include "shutdown/sync_shutdown_callback_stub.h"

#include "message_parcel.h"
#include "power_common.h"
#include "shutdown/sync_shutdown_callback_ipc_interface_code.h"
#include "xcollie/xcollie.h"
#include "xcollie/xcollie_define.h"

namespace OHOS {
namespace PowerMgr {
int SyncShutdownCallbackStub::OnRemoteRequest(
    uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    POWER_HILOGD(FEATURE_SHUTDOWN, "cmd=%{public}d, flags=%{public}d", code, option.GetFlags());
    const int32_t DFX_DELAY_S = 60;
    int32_t id = HiviewDFX::XCollie::GetInstance().SetTimer(
        "PowerShutdownCallback", DFX_DELAY_S, nullptr, nullptr, HiviewDFX::XCOLLIE_FLAG_LOG);
    std::u16string descripter = SyncShutdownCallbackStub::GetDescriptor();
    std::u16string remoteDescripter = data.ReadInterfaceToken();
    if (descripter != remoteDescripter) {
        POWER_HILOGE(COMP_SVC, "Descriptor is not match");
        HiviewDFX::XCollie::GetInstance().CancelTimer(id);
        return E_GET_POWER_SERVICE_FAILED;
    }

    int32_t ret = ERR_OK;
    if (code == static_cast<uint32_t>(PowerMgr::SyncShutdownCallbackInterfaceCode::CMD_ON_SYNC_SHUTDOWN_OR_REBOOT)) {
        ret = OnSyncShutdownOrRebootCallbackStub(data, reply);
    } else if (code == static_cast<uint32_t>(PowerMgr::SyncShutdownCallbackInterfaceCode::CMD_ON_SYNC_SHUTDOWN)) {
        ret = OnSyncShutdownCallbackStub(data, reply);
    } else {
        ret = IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    HiviewDFX::XCollie::GetInstance().CancelTimer(id);
    return ret;
}

int32_t SyncShutdownCallbackStub::OnSyncShutdownCallbackStub(MessageParcel& data, MessageParcel& reply)
{
    OnSyncShutdown();
    return ERR_OK;
}

int32_t SyncShutdownCallbackStub::OnSyncShutdownOrRebootCallbackStub(MessageParcel& data, MessageParcel& reply)
{
    bool isReboot;
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Bool, isReboot, E_READ_PARCEL_ERROR);
    OnSyncShutdownOrReboot(isReboot);
    return ERR_OK;
}

} // namespace PowerMgr
} // namespace OHOS
