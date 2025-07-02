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

#ifndef POWERMGR_POWER_MANAGER_SHUTDOWN_STUB_DELEGATOR_H
#define POWERMGR_POWER_MANAGER_SHUTDOWN_STUB_DELEGATOR_H

#include "power_mgr_stub.h"

namespace OHOS {
namespace PowerMgr {
class ShutdownStubDelegator {
public:
    explicit ShutdownStubDelegator(IRemoteStub<IPowerMgr>& stub) : stub_(stub) {}
    int32_t HandleRemoteRequest(uint32_t code, MessageParcel& data, [[maybe_unused]] MessageParcel& reply,
        [[maybe_unused]] MessageOption& option);

private:
    int32_t RegisterTakeOverShutdownCallback(MessageParcel& data);
    int32_t UnRegisterTakeOverShutdownCallback(MessageParcel& data);

    int32_t RegisterAsyncShutdownCallback(MessageParcel& data);
    int32_t UnRegisterAsyncShutdownCallback(MessageParcel& data);
    int32_t RegisterSyncShutdownCallback(MessageParcel& data);
    int32_t UnRegisterSyncShutdownCallback(MessageParcel& data);

    IRemoteStub<IPowerMgr>& stub_;
};
} // namespace PowerMgr
} // namespace OHOS

#endif // POWERMGR_POWER_MANAGER_SHUTDOWN_STUB_DELEGATOR_H
