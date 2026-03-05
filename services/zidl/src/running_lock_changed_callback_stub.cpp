/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "running_lock_changed_callback_stub.h"

#include <message_parcel.h>

#include "irunning_lock_changed_callback.h"
#include "running_lock_changed_callback_ipc_interface_code.h"
#include "running_lock_changed_callback_proxy.h"
#include "power_log.h"
#include "power_mgr_errors.h"
#include "power_common.h"

namespace OHOS {
namespace PowerMgr {
int RunningLockChangedCallbackStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "cmd = %{public}d, flags= %{public}d", code, option.GetFlags());
    std::u16string descripter = RunningLockChangedCallbackStub::GetDescriptor();
    std::u16string remoteDescripter = data.ReadInterfaceToken();
    if (descripter != remoteDescripter) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Descriptor is not match");
        return E_GET_POWER_SERVICE_FAILED;
    }

    int ret = ERR_OK;
    if (code == static_cast<uint32_t>(RunningLockChangedCallbackInterfaceCode::RUNNINGLOCK_STATE_CHANGED)) {
        uint32_t state;
        RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Uint32, state, E_READ_PARCEL_ERROR);
        OnAsyncScreenRunningLockChanged(static_cast<RunningLockChangeState>(state));
    } else {
        ret = IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return ret;
}
} // namespace PowerMgr
} // namespace OHOS
