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
#include "takeover_suspend_callback_proxy.h"
#include "iremote_object.h"
#include "message_option.h"
#include "message_parcel.h"
#include "power_common.h"
#include "take_over_callback_ipc_interface_code.h"

namespace OHOS {
namespace PowerMgr {
bool TakeOverSuspendCallbackProxy::OnTakeOverSuspend(SuspendDeviceType type)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        POWER_HILOGE(FEATURE_SUSPEND, "remote is nullptr");
        return false;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(TakeOverSuspendCallbackProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_SUSPEND, "Write descriptor failed");
        return false;
    }
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Uint32, static_cast<uint32_t>(type), false);
    int ret = remote->SendRequest(
        static_cast<int32_t>(PowerMgr::TakeOverSuspendCallbackInterfaceCode::CMD_ON_TAKEOVER_SUSPEND),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_SUSPEND, "SendRequest is failed, ret=%{public}d", ret);
        return false;
    }

    bool isTakeOver = false;
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(reply, Bool, isTakeOver, false);
    return isTakeOver;
}
} // namespace PowerMgr
} // namespace OHOS