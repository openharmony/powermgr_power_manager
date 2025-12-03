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

#include "ulsr_callback_holder.h"
#include "power_common.h"

namespace OHOS {
namespace PowerMgr {
void UlsrCallbackHolder::OnRemoteDied(const wptr<IRemoteObject>& object)
{
    RETURN_IF((object == nullptr) || (object.promote() == nullptr))
    POWER_HILOGW(FEATURE_WAKEUP, "object dead, need remove the callback");
    RemoveCallback(iface_cast<IAsyncUlsrCallback>(object.promote()));
}

void UlsrCallbackHolder::AddCallback(const sptr<IAsyncUlsrCallback>& callback,
    const std::pair<int32_t, int32_t>& pidUid)
{
    RETURN_IF((callback == nullptr) || (callback->AsObject() == nullptr));
    std::lock_guard<std::mutex> lock(callbacksMutex_);
    auto iter = cachedCallbacks_.insert(std::make_pair(callback, pidUid));
    if (iter.second) {
        callback->AsObject()->AddDeathRecipient(this);
    }
    POWER_HILOGI(FEATURE_WAKEUP, "ULSRcb add end.");
}

void UlsrCallbackHolder::RemoveCallback(const sptr<IAsyncUlsrCallback>& callback)
{
    RETURN_IF((callback == nullptr) || (callback->AsObject() == nullptr));
    std::lock_guard<std::mutex> lock(callbacksMutex_);
    auto iter = cachedCallbacks_.find(callback);
    if (iter == cachedCallbacks_.end()) {
        POWER_HILOGW(FEATURE_WAKEUP, "ULSRcb callback not found");
        return;
    }
    cachedCallbacks_.erase(iter);
    callback->AsObject()->RemoveDeathRecipient(this);
    POWER_HILOGI(FEATURE_WAKEUP, "ULSRcb remove end.");
}

void UlsrCallbackHolder::WakeupNotify()
{
    std::lock_guard<std::mutex> lock(callbacksMutex_);
    for (const auto& iter : cachedCallbacks_) {
        POWER_HILOGI(FEATURE_WAKEUP, "ULSRcb P=%{public}d,U=%{public}d", iter.second.first, iter.second.second);
        if (!iter.first) {
            POWER_HILOGE(FEATURE_WAKEUP, "ULSRcb callback null error");
            continue;
        }
        iter.first->OnAsyncWakeup();
    }
}
} // namespace PowerMgr
} // namespace OHOS