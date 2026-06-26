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

#ifndef ULSR_CALLBACK_HOLDER_H
#define ULSR_CALLBACK_HOLDER_H

#include <mutex>
#include <string>
#include <unordered_map>

#include "ipc_skeleton.h"
#include "iremote_object.h"
#include "ulsr/iulsr_callback.h"

namespace OHOS {
namespace PowerMgr {
class UlsrCallbackHolder : public IRemoteObject::DeathRecipient {
public:
    struct UlsrCallbackRecord {
        sptr<IUlsrCallback> callback;
        int32_t priority;
        int32_t pid;
        int32_t uid;
        int32_t durationMs;
    };
    struct UlsrCallbackKeyHash {
        size_t operator()(const sptr<IUlsrCallback>& callback) const
        {
            if (callback == nullptr) {
                return 0;
            }
            return std::hash<void*>()(callback->AsObject().GetRefPtr());
        }
    };
    struct UlsrCallbackKeyEqual {
        bool operator()(const sptr<IUlsrCallback>& lhs, const sptr<IUlsrCallback>& rhs) const
        {
            if (lhs == nullptr && rhs == nullptr) {
                return true;
            }
            if (lhs == nullptr || rhs == nullptr) {
                return false;
            }
            return lhs->AsObject() == rhs->AsObject();
        }
    };
    using UlsrCallbackContainerType = std::unordered_map<sptr<IUlsrCallback>, UlsrCallbackRecord,
        UlsrCallbackKeyHash, UlsrCallbackKeyEqual>;

    void OnRemoteDied(const wptr<IRemoteObject>& object) override;
    void AddCallback(const sptr<IUlsrCallback>& callback, const std::pair<int32_t, int32_t>& pidUid,
        UlsrPriority priority = UlsrPriority::DEFAULT);
    void RemoveCallback(const sptr<IUlsrCallback>& callback);
    bool SyncUlsrNotify();
    void WakeupNotify(bool ulsrResult = false);

private:
    enum class UlsrCallbackStage : int32_t {
        STAGE_DONE = 0, // init or WakeupNotify() is called
        STAGE_ENTER,    // SyncUlsrNotify() was called but WakeupNotify() hasn't been called
    };

    template<typename Func>
    void ForEachContainer(Func&& func)
    {
        func(highPriorityCallbacks_);
        func(defaultPriorityCallbacks_);
        func(lowPriorityCallbacks_);
    };

    int64_t SyncUlsrNotifyInner(int64_t timeoutMs);
    void ReportSyncUlsrResult(int64_t elapsedTimeMs, bool isTimeout);

    std::mutex callbacksMutex_;
    std::atomic<UlsrCallbackStage> callbackState_{UlsrCallbackStage::STAGE_DONE};
    UlsrCallbackContainerType highPriorityCallbacks_;
    UlsrCallbackContainerType defaultPriorityCallbacks_;
    UlsrCallbackContainerType lowPriorityCallbacks_;
};
} // namespace PowerMgr
} // namespace OHOS

#endif // ULSR_CALLBACK_HOLDER_H