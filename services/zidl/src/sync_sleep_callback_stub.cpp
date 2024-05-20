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

#include "sync_sleep_callback_stub.h"

#include <message_parcel.h>

#include "power_common.h"
#include "suspend/sync_sleep_callback_ipc_interface_code.h"
#include "sync_sleep_callback_proxy.h"
#include "xcollie/xcollie.h"

namespace OHOS {
namespace PowerMgr {
int SyncSleepCallbackStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    POWER_HILOGD(COMP_SVC, "cmd = %{public}d, flags= %{public}d", code, option.GetFlags());
    const int DFX_DELAY_MS = 10000;
    int id = HiviewDFX::XCollie::GetInstance().SetTimer("SyncSleepCallbackStub", DFX_DELAY_MS, nullptr, nullptr,
        HiviewDFX::XCOLLIE_FLAG_NOOP);
    std::u16string descripter = SyncSleepCallbackStub::GetDescriptor();
    std::u16string remoteDescripter = data.ReadInterfaceToken();
    if (descripter != remoteDescripter) {
        POWER_HILOGE(COMP_SVC, "Descriptor is not match");
        return E_GET_POWER_SERVICE_FAILED;
    }

    int ret = ERR_OK;
    if (code == static_cast<uint32_t>(PowerMgr::SyncSleepCallbackInterfaceCode::CMD_ON_SYNC_SLEEP)) {
        ret = OnSyncSleepStub(data);
    } else if (code == static_cast<uint32_t>(PowerMgr::SyncSleepCallbackInterfaceCode::CMD_ON_SYNC_WAKEUP)) {
        ret = OnSyncWakeupStub(data);
    } else {
        ret = IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    HiviewDFX::XCollie::GetInstance().CancelTimer(id);
    return ret;
}

int32_t SyncSleepCallbackStub::OnSyncSleepStub(MessageParcel& data)
{
    bool onForceSleep;
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Bool, onForceSleep, E_READ_PARCEL_ERROR);
    OnSyncSleep(static_cast<bool>(onForceSleep));
    return ERR_OK;
}

int32_t SyncSleepCallbackStub::OnSyncWakeupStub(MessageParcel& data)
{
    bool onForceSleep;
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Bool, onForceSleep, E_READ_PARCEL_ERROR);
    OnSyncWakeup(static_cast<bool>(onForceSleep));
    return ERR_OK;
}
} // namespace PowerMgr
} // namespace OHOS
