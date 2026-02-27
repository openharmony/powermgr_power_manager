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

#include "running_lock_changed_callback_proxy.h"

#include <message_parcel.h>

#include "irunning_lock_changed_callback.h"
#include "running_lock_changed_callback_ipc_interface_code.h"
#include "power_log.h"

namespace OHOS {
namespace PowerMgr {
void RunningLockChangedCallbackProxy::OnAsyncScreenRunningLockChanged(RunningLockChangeState state)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option = { MessageOption::TF_ASYNC };

    if (!data.WriteInterfaceToken(RunningLockChangedCallbackProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Write descriptor failed");
        return;
    }

    if (!data.WriteUint32(static_cast<uint32_t>(state))) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Write state failed");
        return;
    }

    int ret = Remote()->SendRequest(
        static_cast<uint32_t>(RunningLockChangedCallbackInterfaceCode::RUNNINGLOCK_STATE_CHANGED),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Send request failed, ret=%{public}d", ret);
    }
}
} // namespace PowerMgr
} // namespace OHOS
