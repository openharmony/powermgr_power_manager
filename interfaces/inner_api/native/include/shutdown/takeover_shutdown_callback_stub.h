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

#ifndef POWERMGR_POWER_MANAGER_TAKEOVER_SHUTDOWN_CALLBACK_STUB_H
#define POWERMGR_POWER_MANAGER_TAKEOVER_SHUTDOWN_CALLBACK_STUB_H

#include "iremote_stub.h"
#include "nocopyable.h"

#include "itakeover_shutdown_callback.h"

namespace OHOS {
namespace PowerMgr {
class TakeOverShutdownCallbackStub : public IRemoteStub<ITakeOverShutdownCallback> {
public:
    DISALLOW_COPY_AND_MOVE(TakeOverShutdownCallbackStub);
    TakeOverShutdownCallbackStub() = default;
    ~TakeOverShutdownCallbackStub() override = default;

    int32_t OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override;

    bool OnTakeOverShutdown(bool isReboot) override
    {
        return false;
    }

private:
    int32_t OnTakeOverShutdownCallbackStub(MessageParcel& data, MessageParcel& reply);
};
} // namespace PowerMgr
} // namespace OHOS

#endif // POWERMGR_POWER_MANAGER_TAKEOVER_SHUTDOWN_CALLBACK_STUB_H
