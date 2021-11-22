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

#ifndef POWERMGR_IPOWER_HDF_SUSPEND_CALLBACK_H
#define POWERMGR_IPOWER_HDF_SUSPEND_CALLBACK_H

#include "ipc_object_stub.h"
#include "power_hdf_info.h"

namespace OHOS {
namespace PowerMgr {
class IPowerHdfCallback : public IPCObjectStub {
public:
    explicit IPowerHdfCallback() : IPCObjectStub(u"ohos.powermgr.IPowerHdfCallback") {};
    virtual ~IPowerHdfCallback() = default;
    int OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
    virtual void OnSuspend() = 0;
    virtual void OnWakeup() = 0;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_IPOWER_HDF_SUSPEND_CALLBACK_H
