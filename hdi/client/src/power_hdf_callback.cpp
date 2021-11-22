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

#include "ipower_hdf_callback.h"

namespace OHOS {
namespace PowerMgr {
int IPowerHdfCallback::OnRemoteRequest(uint32_t code,
    MessageParcel &data,
    MessageParcel &reply,
    MessageOption &option)
{
    switch (code) {
        case CMD_ON_SUSPEND: {
            OnSuspend();
            return ERR_OK;
        }
        case CMD_ON_WAKEUP: {
            OnWakeup();
            return ERR_OK;
        }
        default: {
            return ERR_INVALID_OPERATION;
        }
    }
}
} // namespace PowerMgr
} // namespace OHOS