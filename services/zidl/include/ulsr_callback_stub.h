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

#ifndef POWERMGR_POWER_MANAGER_ULSR_CALLBACK_STUB_H
#define POWERMGR_POWER_MANAGER_ULSR_CALLBACK_STUB_H

#include <iremote_stub.h>
#include <nocopyable.h>

#include "ulsr/iulsr_callback.h"

namespace OHOS {
namespace PowerMgr {
class UlsrCallbackStub : public IRemoteStub<IUlsrCallback> {
public:
    DISALLOW_COPY_AND_MOVE(UlsrCallbackStub);
    UlsrCallbackStub() = default;
    virtual ~UlsrCallbackStub() = default;

    int OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
    void OnSyncUlsr() override {}
    void OnAsyncWakeup(bool ulsrResult = false) override {}

private:
    int32_t OnSyncUlsrStub(MessageParcel& data);
    int32_t OnAsyncWakeupStub(MessageParcel& data);
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_POWER_MANAGER_ULSR_CALLBACK_STUB_H