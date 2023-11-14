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

#ifndef POWERMGR_POWER_MANAGER_SYNC_SLEEP_CALLBACK_STUB_H
#define POWERMGR_POWER_MANAGER_SYNC_SLEEP_CALLBACK_STUB_H

#include <iremote_stub.h>
#include <nocopyable.h>

#include "suspend/isync_sleep_callback.h"

namespace OHOS {
namespace PowerMgr {
class SyncSleepCallbackStub : public IRemoteStub<ISyncSleepCallback> {
public:
    DISALLOW_COPY_AND_MOVE(SyncSleepCallbackStub);
    SyncSleepCallbackStub() = default;
    virtual ~SyncSleepCallbackStub() = default;

    int OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
    void OnSyncSleep(bool onForceSleep) override {}
    void OnSyncWakeup(bool onForceSleep) override {}

private:
    int32_t OnSyncSleepStub(MessageParcel& data);
    int32_t OnSyncWakeupStub(MessageParcel& data);
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_POWER_MANAGER_SYNC_SLEEP_CALLBACK_STUB_H
