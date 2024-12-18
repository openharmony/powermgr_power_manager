/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "power_mode_callback_proxy.h"

#include <message_parcel.h>
#include "errors.h"
#include "message_option.h"
#include "power_log.h"
#include "power_common.h"
#include "power_mode_callback_ipc_interface_code.h"

namespace OHOS {
namespace PowerMgr {
void PowerModeCallbackProxy::OnPowerModeChanged(PowerMode mode)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF(remote == nullptr);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerModeCallbackProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_POWER_MODE, "Write descriptor failed");
        return;
    }

    RETURN_IF_WRITE_PARCEL_FAILED_NO_RET(data, Uint32, static_cast<uint32_t>(mode));

    int ret = remote->SendRequest(
        static_cast<int>(PowerMgr::PowerModeCallbackInterfaceCode::POWER_MODE_CHANGED),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_POWER_MODE, "%{public}s: SendRequest failed with ret=%{public}d", __func__, ret);
    }
}
} // namespace PowerMgr
} // namespace OHOS
