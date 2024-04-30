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

#include "sync_hibernate_callback_stub.h"

#include <message_parcel.h>

#include "power_common.h"
#include "hibernate/sync_hibernate_callback_ipc_interface_code.h"
#include "sync_hibernate_callback_proxy.h"
#include "xcollie/xcollie.h"

namespace OHOS {
namespace PowerMgr {
int SyncHibernateCallbackStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    POWER_HILOGD(COMP_SVC, "cmd = %{public}d, flags= %{public}d", code, option.GetFlags());
    const int DFX_DELAY_MS = 10000;
    int id = HiviewDFX::XCollie::GetInstance().SetTimer("SyncHibernateCallbackStub", DFX_DELAY_MS, nullptr, nullptr,
        HiviewDFX::XCOLLIE_FLAG_NOOP);
    std::u16string descripter = SyncHibernateCallbackStub::GetDescriptor();
    std::u16string remoteDescripter = data.ReadInterfaceToken();
    if (descripter != remoteDescripter) {
        POWER_HILOGE(COMP_SVC, "Descriptor is not match");
        return E_GET_POWER_SERVICE_FAILED;
    }

    int ret = ERR_OK;
    if (code == static_cast<uint32_t>(PowerMgr::SyncHibernateCallbackInterfaceCode::CMD_ON_SYNC_HIBERNATE)) {
        ret = OnSyncHibernateStub(data);
    } else if (code == static_cast<uint32_t>(PowerMgr::SyncHibernateCallbackInterfaceCode::CMD_ON_SYNC_WAKEUP)) {
        ret = OnSyncWakeupStub(data);
    } else {
        ret = IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    HiviewDFX::XCollie::GetInstance().CancelTimer(id);
    return ret;
}

int32_t SyncHibernateCallbackStub::OnSyncHibernateStub(MessageParcel& data)
{
    OnSyncHibernate();
    return ERR_OK;
}

int32_t SyncHibernateCallbackStub::OnSyncWakeupStub(MessageParcel& data)
{
    OnSyncWakeup();
    return ERR_OK;
}
} // namespace PowerMgr
} // namespace OHOS
