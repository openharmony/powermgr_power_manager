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

#include <datetime_ex.h>
#include <future>
#ifdef HAS_HIVIEWDFX_HISYSEVENT_PART
#include <hisysevent.h>
#endif

#include "ffrt_utils.h"
#include "power_common.h"
#include "power_state_machine_info.h"

namespace OHOS {
namespace PowerMgr {
namespace {
constexpr int32_t ULSR_SYNC_CALLBACK_TIMEOUT_MS = 30000; // Maximum total execution time for all ULSR sync callbacks
}

void UlsrCallbackHolder::OnRemoteDied(const wptr<IRemoteObject>& object)
{
    RETURN_IF((object == nullptr) || (object.promote() == nullptr));
    POWER_HILOGW(FEATURE_WAKEUP, "object dead, need remove the callback");
    RemoveCallback(iface_cast<IUlsrCallback>(object.promote()));
}

void UlsrCallbackHolder::AddCallback(const sptr<IUlsrCallback>& callback,
    const std::pair<int32_t, int32_t>& pidUid, UlsrPriority priority)
{
    RETURN_IF((callback == nullptr) || (callback->AsObject() == nullptr));
    std::lock_guard<std::mutex> lock(callbacksMutex_);

    POWER_HILOGI(FEATURE_WAKEUP, "ULSRcb add: PR=%{public}d, P=%{public}d, U=%{public}d",
        static_cast<int32_t>(priority), pidUid.first, pidUid.second);

    // Check if callback already exists in any priority queue
    if (highPriorityCallbacks_.find(callback) != highPriorityCallbacks_.end() ||
        defaultPriorityCallbacks_.find(callback) != defaultPriorityCallbacks_.end() ||
        lowPriorityCallbacks_.find(callback) != lowPriorityCallbacks_.end()) {
        POWER_HILOGW(FEATURE_WAKEUP, "ULSRcb add failed, callback already exists");
        return;
    }

    UlsrCallbackRecord record = {callback, static_cast<int32_t>(priority), pidUid.first, pidUid.second, -1};
    switch (priority) {
        case UlsrPriority::HIGH:
            highPriorityCallbacks_.emplace(callback, record);
            break;
        case UlsrPriority::DEFAULT:
            defaultPriorityCallbacks_.emplace(callback, record);
            break;
        case UlsrPriority::LOW:
            lowPriorityCallbacks_.emplace(callback, record);
            break;
        default:
            POWER_HILOGE(FEATURE_WAKEUP, "ULSRcb add failed, priority error");
            return;
    }
    callback->AsObject()->AddDeathRecipient(this);
    POWER_HILOGI(FEATURE_WAKEUP, "ULSRcb add end, PR=%{public}d, P=%{public}d, U=%{public}d",
        static_cast<int32_t>(priority), pidUid.first, pidUid.second);
}

void UlsrCallbackHolder::RemoveCallback(const sptr<IUlsrCallback>& callback)
{
    RETURN_IF((callback == nullptr) || (callback->AsObject() == nullptr));
    std::lock_guard<std::mutex> lock(callbacksMutex_);

    ForEachContainer([&callback](auto& container) {
        container.erase(callback);
    });

    callback->AsObject()->RemoveDeathRecipient(this);
    POWER_HILOGI(FEATURE_WAKEUP, "ULSRcb remove end");
}

bool UlsrCallbackHolder::SyncUlsrNotify()
{
    // Anti-re-entry check: UlsrCallbackStage MUST be STAGE_DONE when calling OnSyncUlsr
    UlsrCallbackStage expected = UlsrCallbackStage::STAGE_DONE;
    if (!callbackState_.compare_exchange_strong(expected, UlsrCallbackStage::STAGE_ENTER,
        std::memory_order_acq_rel, std::memory_order_acquire)) {
        POWER_HILOGW(FEATURE_SUSPEND, "ULSRcb SyncUlsrNotify invalid state transition, state: %{public}d, "
            "expected: %{public}d", static_cast<int32_t>(callbackState_.load()), static_cast<int32_t>(expected));
        return false;
    }

    std::lock_guard<std::mutex> lock(callbacksMutex_);

    // Clear duration records before executing callbacks
    ForEachContainer([](auto& container) {
        for (auto& [cb, record] : container) {
            record.durationMs = -1;
        }
    });

    int64_t beginTimeMs = GetTickCount();
    auto notifyInnerTask = [this] () {
        SyncUlsrNotifyInner(ULSR_SYNC_CALLBACK_TIMEOUT_MS);
    };
    std::packaged_task<void()> callbackTask(notifyInnerTask);
    std::future<void> fut = callbackTask.get_future();
    std::make_unique<std::thread>(std::move(callbackTask))->detach();
    std::future_status status = fut.wait_for(std::chrono::milliseconds(ULSR_SYNC_CALLBACK_TIMEOUT_MS));
    bool isTimeout = status == std::future_status::timeout;
    int64_t endTimeMs = GetTickCount();

    ReportSyncUlsrResult(endTimeMs - beginTimeMs, isTimeout);
    POWER_HILOGI(FEATURE_SUSPEND, "ULSRcb SyncUlsrNotify end, isTimeout: %{public}d", isTimeout);
    return !isTimeout;
}

void UlsrCallbackHolder::ReportSyncUlsrResult(int64_t elapsedTimeMs, bool isTimeout)
{
    // Build reason string: "priority:pid:uid:duration;..."
    std::ostringstream oss;
    ForEachContainer([&oss](const auto& container) {
        for (const auto& [cb, record] : container) {
            oss << record.priority << ":" << record.pid << ":" << record.uid << ":" << record.durationMs << ";";
        }
    });
    std::string reasonStr = oss.str();
    POWER_HILOGI(FEATURE_SUSPEND, "ULSRcb SyncUlsrNotifyInner end, T=%{public}ld, R=%{public}s",
        static_cast<long>(elapsedTimeMs), reasonStr.c_str());

    if (isTimeout) {
        POWER_HILOGE(FEATURE_SUSPEND, "ULSRcb SyncUlsrNotify timeout, skip waiting remaining callbacks");
#ifdef HAS_HIVIEWDFX_HISYSEVENT_PART
        pid_t pid = IPCSkeleton::GetCallingPid();
        auto uid = IPCSkeleton::GetCallingUid();
        HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::POWER, "INTERFACE_CONSUMING_TIMEOUT",
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR, "PID", pid, "UID", uid,
            "TYPE", static_cast<int32_t>(InterfaceTimeoutType::INTERFACE_TIMEOUT_TYPE_ULSR_SYNC_CALLBACK),
            "REASON", reasonStr, "TIME", static_cast<int32_t>(elapsedTimeMs));
#endif
    }
}

int64_t UlsrCallbackHolder::SyncUlsrNotifyInner(int64_t timeoutMs)
{
    ForEachContainer([&timeoutMs](auto& container) {
        if (timeoutMs <= 0) {
            return;
        }
        for (auto& [cb, record] : container) {
            if (cb == nullptr) {
                POWER_HILOGE(FEATURE_SUSPEND, "ULSRcb SyncUlsrNotifyInner callback is nullptr, skip");
                continue;
            }
            if (timeoutMs <= 0) {
                return;
            }
            int64_t cbBegin = GetTickCount();
            cb->OnSyncUlsr();
            int64_t cbEnd = GetTickCount();
            record.durationMs = static_cast<int32_t>(cbEnd - cbBegin);
            timeoutMs -= cbEnd - cbBegin;
            POWER_HILOGI(FEATURE_SUSPEND, "ULSRcb SyncUlsrNotifyInner PR=%{public}d, P=%{public}d, U=%{public}d, "
                "D=%{public}d", record.priority, record.pid, record.uid, record.durationMs);
        }
    });
    return timeoutMs;
}

void UlsrCallbackHolder::WakeupNotify(bool ulsrResult)
{
    // Anti-re-entry check: UlsrCallbackStage MUST be STAGE_ENTER when calling OnAsyncWakeup
    UlsrCallbackStage expected = UlsrCallbackStage::STAGE_ENTER;
    if (!callbackState_.compare_exchange_strong(expected, UlsrCallbackStage::STAGE_DONE,
        std::memory_order_acq_rel, std::memory_order_acquire)) {
        POWER_HILOGW(FEATURE_WAKEUP, "ULSRcb WakeupNotify invalid state transition, state: %{public}d, "
            "expected: %{public}d", static_cast<int32_t>(callbackState_.load()), static_cast<int32_t>(expected));
        return;
    }

    std::lock_guard<std::mutex> lock(callbacksMutex_);
    
    ForEachContainer([ulsrResult](auto& container) {
        for (const auto& [cb, record] : container) {
            if (cb == nullptr) {
                POWER_HILOGE(FEATURE_WAKEUP, "ULSRcb WakeupNotify callback null error");
                continue;
            }
            POWER_HILOGI(FEATURE_WAKEUP, "ULSRcb WakeupNotify PR=%{public}d, P=%{public}d, U=%{public}d",
                record.priority, record.pid, record.uid);
            cb->OnAsyncWakeup(ulsrResult);
        }
    });
    POWER_HILOGI(FEATURE_WAKEUP, "ULSRcb WakeupNotify end");
}

UlsrCallbackStage UlsrCallbackHolder::GetCallbackState() const
{
    UlsrCallbackStage state = callbackState_.load();
    POWER_HILOGI(FEATURE_WAKEUP, "ULSRcb GetCallbackState S=%{public}d", static_cast<int32_t>(state));
    return state;
}
} // namespace PowerMgr
} // namespace OHOS