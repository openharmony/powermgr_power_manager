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

#ifndef ULSR_WAKEUP_CALLBACK_HOLER_H
#define ULSR_WAKEUP_CALLBACK_HOLER_H

#include <map>

#include "ipc_skeleton.h"
#include "iremote_object.h"
#include "ulsr/iasync_ulsr_callback.h"

namespace OHOS {
namespace PowerMgr {
class UlsrCallbackHolder : public IRemoteObject::DeathRecipient {
public:
    struct UlsrCallbackCompare {
        bool operator()(const sptr<IAsyncUlsrCallback>& lhs, const sptr<IAsyncUlsrCallback>& rhs) const
        {
            return lhs->AsObject() < rhs->AsObject();
        }
    };

    void OnRemoteDied(const wptr<IRemoteObject>& object) override;
    void AddCallback(const sptr<IAsyncUlsrCallback>& callback, const std::pair<int32_t, int32_t>& pidUid);
    void RemoveCallback(const sptr<IAsyncUlsrCallback>& callback);
    void WakeupNotify();

private:
    std::mutex callbacksMutex_;
    std::map<sptr<IAsyncUlsrCallback>, std::pair<int32_t, int32_t>, UlsrCallbackCompare> cachedCallbacks_;
};
} // namespace PowerMgr
} // namespace OHOS

#endif // ULSR_WAKEUP_CALLBACK_HOLER_H