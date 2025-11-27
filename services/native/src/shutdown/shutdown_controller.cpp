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

#include "shutdown_controller.h"

#include "ipc_skeleton.h"
#include "iremote_broker.h"
#include "power_common.h"
#include "power_time.h"
#include "power_mgr_factory.h"
#include "screen_manager_lite.h"
#include "parameters.h"

#include <algorithm>
#include <cinttypes>
#include <common_event_data.h>
#include <common_event_manager.h>
#include <common_event_publish_info.h>
#include <common_event_support.h>
#include <datetime_ex.h>
#include <future>
#ifdef HAS_HIVIEWDFX_HISYSEVENT_PART
#include <hisysevent.h>
#endif
#include <thread>
#include <dlfcn.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <vector>

using namespace OHOS::AAFwk;
using namespace OHOS::EventFwk;
using namespace std;

namespace OHOS {
namespace PowerMgr {
namespace {
constexpr std::chrono::seconds CALLBACK_TIMEOUT = 10s;
constexpr std::chrono::seconds OFF_TIMEOUT = 5s;
const time_t MAX_TIMEOUT_SEC = 30;
#ifdef POWER_MANAGER_ENABLE_JUDGING_TAKEOVER_SHUTDOWN
const vector<string> REASONS_DISABLE_TAKE_OVER = {"LowCapacity", "HibernateFail"};
#endif

enum PowerEventType {
    DEFAULT = 0,
    REBOOT = 1,
    SHUTDOWN = 2,
    SUSPEND = 3,
    HIBERNATE = 4
};
}

ShutdownController::ShutdownController() : started_(false)
{
    POWER_HILOGD(FEATURE_SHUTDOWN, "Instance create");
    devicePowerAction_ = PowerMgrFactory::GetDevicePowerAction();
    deviceStateAction_ = PowerMgrFactory::GetDeviceStateAction();
    takeoverShutdownCallbackHolder_ = new ShutdownCallbackHolder();
    asyncShutdownCallbackHolder_ = new ShutdownCallbackHolder();
    syncShutdownCallbackHolder_ = new ShutdownCallbackHolder();
}

void ShutdownController::Reboot(const std::string& reason, bool force)
{
    RebootOrShutdown(reason, true, force);
}

void ShutdownController::Shutdown(const std::string& reason)
{
    RebootOrShutdown(reason, false);
}

bool ShutdownController::IsShuttingDown()
{
    return started_;
}

static void SetFrameworkFinishBootStage(void)
{
    int fd = open("/dev/bbox", O_WRONLY);
    if (fd < 0) {
        POWER_HILOGE(FEATURE_SHUTDOWN, "open /dev/bbox failed!");
        return;
    }

    fdsan_exchange_owner_tag(fd, 0, DOMAIN_FEATURE_SHUTDOWN);
    POWER_HILOGI(FEATURE_SHUTDOWN, "Set shutdown timeout");

    int stage = SHUT_STAGE_FRAMEWORK_FINISH;
    int ret = ioctl(fd, SET_SHUT_STAGE, &stage);
    if (ret < 0) {
        POWER_HILOGE(FEATURE_SHUTDOWN, "set shut stage failed!");
    }

    POWER_HILOGI(FEATURE_SHUTDOWN, "Set shutdown timeout mechanism finished.");
    fdsan_close_with_tag(fd, DOMAIN_FEATURE_SHUTDOWN);

    return;
}

namespace {
std::future<void> g_futForSyncCb;
std::future<void> g_futForOff;
}

void ShutdownController::SetShutdownReason(const std::string& reason)
{
    constexpr uint32_t PARAM_ZERO = 0;
    constexpr uint32_t MAX_LENGTH = 95;
    std::string tempReason = reason;
    if (reason.size() > MAX_LENGTH) {
        tempReason = reason.substr(PARAM_ZERO, MAX_LENGTH);
        POWER_KHILOGE(FEATURE_SHUTDOWN, "Truncate reason,reason=%{public}s,tempReason=%{public}s",
            reason.c_str(), tempReason.c_str());
    } else {
        POWER_KHILOGI(FEATURE_SHUTDOWN, "SetShutdownReason=%{public}s", tempReason.c_str());
    }
    system::SetParameter("persist.dfx.shutdown_reason", tempReason);
}

void ShutdownController::RebootOrShutdown(const std::string& reason, bool isReboot, bool force)
{
    // skip takeover callback
    // SetFrameworkFinishBootStage done in client
    if (!force) {
        if (started_) {
            POWER_KHILOGI(FEATURE_SHUTDOWN, "Shutdown is already running");
            return;
        }
        started_ = true;
        bool isTakeOver = TakeOverShutdownAction(reason, isReboot);
        if (isTakeOver) {
            started_ = false;
            return;
        }
        if (reason != "test_case") {
            SetFrameworkFinishBootStage();
        }
    }
    started_ = true;
    POWER_KHILOGI(FEATURE_SHUTDOWN, "Start to detach shutdown thread");
#ifdef HAS_HIVIEWDFX_HISYSEVENT_PART
    ReportDoShutdown();
#endif
    SetShutdownReason(reason);
    PublishShutdownEvent();
    std::string actionTimeStr = std::to_string(GetCurrentRealTimeMs());
    PowerEventType eventType = isReboot ? PowerEventType::REBOOT : PowerEventType::SHUTDOWN;
    system::SetParameter("persist.dfx.eventtype", to_string(eventType));
    if (force) {
        // only two threads are allowed, next time it blocks
        // either block or thread leak. since the previous action is shutdown, block is safer than thread leak
        g_futForSyncCb = async(launch::async, &ShutdownController::TriggerSyncShutdownCallback, this, isReboot);
        g_futForSyncCb.wait_for(CALLBACK_TIMEOUT);
        actionTimeStr = actionTimeStr + "," + std::to_string(GetCurrentRealTimeMs());
        g_futForOff = async(launch::async, &ShutdownController::TurnOffScreen, this);
        g_futForOff.wait_for(OFF_TIMEOUT);
    } else {
        TriggerSyncShutdownCallback(isReboot);
        actionTimeStr = actionTimeStr + "," + std::to_string(GetCurrentRealTimeMs());
        TurnOffScreen();
    }
    actionTimeStr = actionTimeStr + "," + std::to_string(GetCurrentRealTimeMs());
    system::SetParameter("persist.dfx.shutdownactiontime", actionTimeStr);
    make_unique<thread>([=] {
        Prepare(isReboot);
        POWER_KHILOGI(FEATURE_SHUTDOWN, "reason = %{public}s, reboot = %{public}d", reason.c_str(), isReboot);
        if (devicePowerAction_ != nullptr) {
            std::string shutdownDeviceTime = std::to_string(GetCurrentRealTimeMs());
            system::SetParameter("persist.dfx.shutdowncompletetime", shutdownDeviceTime);
            isReboot ? devicePowerAction_->Reboot(reason) : devicePowerAction_->Shutdown(reason);
        }
        started_ = false;
    })->detach();
}

void ShutdownController::Prepare(bool isReboot)
{
    auto callbackStart = [&]() {
        TriggerAsyncShutdownCallback(isReboot);
    };

    packaged_task<void()> callbackTask(callbackStart);
    future<void> fut = callbackTask.get_future();
    make_unique<thread>(std::move(callbackTask))->detach();

    POWER_KHILOGI(FEATURE_SHUTDOWN, "Waiting for the callback execution complete...");
    future_status status = fut.wait_for(std::chrono::seconds(MAX_TIMEOUT_SEC));
    if (status == future_status::timeout) {
        POWER_HILOGW(FEATURE_SHUTDOWN, "Shutdown callback execution timeout");
    }
    POWER_KHILOGI(FEATURE_SHUTDOWN, "The callback execution is complete");
}

void ShutdownController::PublishShutdownEvent() const
{
    POWER_KHILOGI(FEATURE_SHUTDOWN, "Start of publishing shutdown event");
    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(false);
    IntentWant shutdownWant;
    shutdownWant.SetAction(CommonEventSupport::COMMON_EVENT_SHUTDOWN);
    CommonEventData event(shutdownWant);
    if (!CommonEventManager::PublishCommonEvent(event, publishInfo, nullptr)) {
        POWER_KHILOGE(FEATURE_SHUTDOWN, "Publish the shutdown event fail");
        return;
    }
    POWER_KHILOGI(FEATURE_SHUTDOWN, "End of publishing shutdown event");
}

void ShutdownController::TurnOffScreen()
{
    POWER_HILOGD(FEATURE_SHUTDOWN, "Turn off screen before shutdown");
    bool ret = Rosen::ScreenManagerLite::GetInstance().SetScreenPowerForAll(Rosen::ScreenPowerState::POWER_OFF,
        Rosen::PowerStateChangeReason::STATE_CHANGE_REASON_SHUT_DOWN);
#ifdef HAS_HIVIEWDFX_HISYSEVENT_PART
    if (!ret) {
        std::string eventReason = "TurnOffScreen failed";
        HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::POWER, "ABNORMAL_FAULT",
            HiviewDFX::HiSysEvent::EventType::FAULT, "TYPE", "EXTERNAL_SCREEN", "REASON", eventReason);
    }
#endif
    POWER_KHILOGE(FEATURE_SHUTDOWN, "Turn off screen before shutting down, ret = %{public}d", ret);
}

void ShutdownController::AddCallback(const sptr<ITakeOverShutdownCallback>& callback, ShutdownPriority priority)
{
    RETURN_IF(callback->AsObject() == nullptr)
    takeoverShutdownCallbackHolder_->AddCallback(callback->AsObject(), priority);
    POWER_HILOGI(FEATURE_SHUTDOWN,
        "takeover shutdown callback added, priority=%{public}u, pid=%{public}d, uid=%{public}d", priority,
        IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid());
}

void ShutdownController::AddCallback(const sptr<IAsyncShutdownCallback>& callback, ShutdownPriority priority)
{
    RETURN_IF(callback->AsObject() == nullptr)
    asyncShutdownCallbackHolder_->AddCallback(callback->AsObject(), priority);
    POWER_HILOGI(FEATURE_SHUTDOWN,
        "async shutdown callback added, priority=%{public}u, pid=%{public}d, uid=%{public}d", priority,
        IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid());
}

void ShutdownController::AddCallback(const sptr<ISyncShutdownCallback>& callback, ShutdownPriority priority)
{
    RETURN_IF(callback->AsObject() == nullptr)
    syncShutdownCallbackHolder_->AddCallback(callback->AsObject(), priority);
    POWER_HILOGI(FEATURE_SHUTDOWN,
        "sync shutdown callback added, priority=%{public}u, pid=%{public}d, uid=%{public}d", priority,
        IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid());
}

void ShutdownController::RemoveCallback(const sptr<ITakeOverShutdownCallback>& callback)
{
    RETURN_IF(callback->AsObject() == nullptr)
    takeoverShutdownCallbackHolder_->RemoveCallback(callback->AsObject());
    POWER_HILOGI(FEATURE_SHUTDOWN, "takeover shutdown callback removed, pid=%{public}d, uid=%{public}d",
        IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid());
}

void ShutdownController::RemoveCallback(const sptr<IAsyncShutdownCallback>& callback)
{
    RETURN_IF(callback->AsObject() == nullptr)
    asyncShutdownCallbackHolder_->RemoveCallback(callback->AsObject());
    POWER_HILOGI(FEATURE_SHUTDOWN, "async shutdown callback removed, pid=%{public}d, uid=%{public}d",
        IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid());
}

void ShutdownController::RemoveCallback(const sptr<ISyncShutdownCallback>& callback)
{
    RETURN_IF(callback->AsObject() == nullptr)
    syncShutdownCallbackHolder_->RemoveCallback(callback->AsObject());
    POWER_HILOGI(FEATURE_SHUTDOWN, "sync shutdown callback removed, pid=%{public}d, uid=%{public}d",
        IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid());
}

bool ShutdownController::TriggerTakeOverShutdownCallback(const TakeOverInfo& info)
{
    bool isTakeover = false;
    auto highPriorityCallbacks = takeoverShutdownCallbackHolder_->GetHighPriorityCallbacks();
    isTakeover = TriggerTakeOverShutdownCallbackInner(highPriorityCallbacks, info);
    RETURN_IF_WITH_RET(isTakeover, true);
    auto defaultPriorityCallbacks = takeoverShutdownCallbackHolder_->GetDefaultPriorityCallbacks();
    isTakeover = TriggerTakeOverShutdownCallbackInner(defaultPriorityCallbacks, info);
    RETURN_IF_WITH_RET(isTakeover, true);
    auto lowPriorityCallbacks = takeoverShutdownCallbackHolder_->GetLowPriorityCallbacks();
    isTakeover = TriggerTakeOverShutdownCallbackInner(lowPriorityCallbacks, info);
    return isTakeover;
}

bool ShutdownController::TriggerTakeOverHibernateCallback(const TakeOverInfo& info)
{
    bool isTakeover = false;
    auto highPriorityCallbacks = takeoverShutdownCallbackHolder_->GetHighPriorityCallbacks();
    isTakeover = TriggerTakeOverHibernateCallbackInner(highPriorityCallbacks, info);
    RETURN_IF_WITH_RET(isTakeover, true);
    auto defaultPriorityCallbacks = takeoverShutdownCallbackHolder_->GetDefaultPriorityCallbacks();
    isTakeover = TriggerTakeOverHibernateCallbackInner(defaultPriorityCallbacks, info);
    RETURN_IF_WITH_RET(isTakeover, true);
    auto lowPriorityCallbacks = takeoverShutdownCallbackHolder_->GetLowPriorityCallbacks();
    isTakeover = TriggerTakeOverHibernateCallbackInner(lowPriorityCallbacks, info);
    return isTakeover;
}

bool ShutdownController::TriggerTakeOverShutdownCallbackInner(
    std::set<sptr<IRemoteObject>>& callbacks, const TakeOverInfo& info)
{
    bool isTakeover = false;
    int32_t takeoverUid = 0;
    for (const auto& obj : callbacks) {
        auto pidUid = takeoverShutdownCallbackHolder_->FindCallbackPidUid(obj);
        // ITakeOverShutdownCallback->OnTakeOverShutdown calling pid uid
        POWER_HILOGI(FEATURE_SHUTDOWN, "TOScb P=%{public}dU=%{public}d", pidUid.first, pidUid.second);
        auto callback = iface_cast<ITakeOverShutdownCallback>(obj);
        if (!callback) {
            POWER_HILOGE(FEATURE_SHUTDOWN, "TOScb callback null error");
            continue;
        }
        if (callback->OnTakeOverShutdown(info)) {
            isTakeover = true;
            takeoverUid = pidUid.second;
        }
    }
#ifdef HAS_HIVIEWDFX_HISYSEVENT_PART
    if (isTakeover) {
        HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::POWER, "SHUTDOWN_STATISTIC",
            HiviewDFX::HiSysEvent::EventType::STATISTIC, "TAKEOVER_UID", takeoverUid);
    }
#endif
    return isTakeover;
}

bool ShutdownController::TriggerTakeOverHibernateCallbackInner(
    std::set<sptr<IRemoteObject>>& callbacks, const TakeOverInfo& info)
{
    bool isTakeover = false;
    int32_t takeoverUid = 0;
    for (const auto& obj : callbacks) {
        auto pidUid = takeoverShutdownCallbackHolder_->FindCallbackPidUid(obj);
        // ITakeOverShutdownCallback->OnTakeOverHibernate calling pid uid
        POWER_HILOGI(FEATURE_SHUTDOWN, "TOHcb P=%{public}dU=%{public}d", pidUid.first, pidUid.second);
        auto callback = iface_cast<ITakeOverShutdownCallback>(obj);
        if (!callback) {
            POWER_HILOGE(FEATURE_SHUTDOWN, "TOHcb callback null error");
            continue;
        }
        if (callback->OnTakeOverHibernate(info)) {
            isTakeover = true;
            takeoverUid = pidUid.second;
        }
    }
#ifdef HAS_HIVIEWDFX_HISYSEVENT_PART
    if (isTakeover) {
        HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::POWER, "HIBERNATE_STATISTIC",
            HiviewDFX::HiSysEvent::EventType::STATISTIC, "TAKEOVER_UID", takeoverUid);
    }
#endif
    return isTakeover;
}

void ShutdownController::TriggerAsyncShutdownCallback(bool isReboot)
{
    auto highPriorityCallbacks = asyncShutdownCallbackHolder_->GetHighPriorityCallbacks();
    TriggerAsyncShutdownCallbackInner(highPriorityCallbacks, isReboot);
    auto defaultPriorityCallbacks = asyncShutdownCallbackHolder_->GetDefaultPriorityCallbacks();
    TriggerAsyncShutdownCallbackInner(defaultPriorityCallbacks, isReboot);
    auto lowPriorityCallbacks = asyncShutdownCallbackHolder_->GetLowPriorityCallbacks();
    TriggerAsyncShutdownCallbackInner(lowPriorityCallbacks, isReboot);
}

void ShutdownController::TriggerAsyncShutdownCallbackInner(std::set<sptr<IRemoteObject>>& callbacks, bool isReboot)
{
    for (const auto &obj : callbacks) {
        auto pidUid = asyncShutdownCallbackHolder_->FindCallbackPidUid(obj);
        sptr<IAsyncShutdownCallback> callback = iface_cast<IAsyncShutdownCallback>(obj);
        if (callback != nullptr) {
            int64_t start = GetTickCount();
            POWER_KHILOGI(FEATURE_SHUTDOWN, "Async Shutdown Callback pid=%{public}d", pidUid.first);
            callback->OnAsyncShutdown();
            callback->OnAsyncShutdownOrReboot(isReboot);
            int64_t cost = GetTickCount() - start;
            POWER_KHILOGI(FEATURE_SHUTDOWN,
                "ASUTcb P=%{public}dU=%{public}dT=%{public}" PRId64 "", pidUid.first, pidUid.second, cost);
        }
    }
}

void ShutdownController::TriggerSyncShutdownCallback(bool isReboot)
{
    auto highPriorityCallbacks = syncShutdownCallbackHolder_->GetHighPriorityCallbacks();
    TriggerSyncShutdownCallbackInner(highPriorityCallbacks, isReboot);
    auto defaultPriorityCallbacks = syncShutdownCallbackHolder_->GetDefaultPriorityCallbacks();
    TriggerSyncShutdownCallbackInner(defaultPriorityCallbacks, isReboot);
    auto lowPriorityCallbacks = syncShutdownCallbackHolder_->GetLowPriorityCallbacks();
    TriggerSyncShutdownCallbackInner(lowPriorityCallbacks, isReboot);
}

void ShutdownController::TriggerSyncShutdownCallbackInner(std::set<sptr<IRemoteObject>>& callbacks, bool isReboot)
{
    for (auto &obj : callbacks) {
        auto pidUid = syncShutdownCallbackHolder_->FindCallbackPidUid(obj);
        sptr<ISyncShutdownCallback> callback = iface_cast<ISyncShutdownCallback>(obj);
        if (callback != nullptr) {
            int64_t start = GetTickCount();
            POWER_KHILOGI(FEATURE_SHUTDOWN, "Sync Shutdown Callback pid=%{public}d", pidUid.first);
            callback->OnSyncShutdown();
            callback->OnSyncShutdownOrReboot(isReboot);
            int64_t cost = GetTickCount() - start;
            // ISyncShutdownCallback calling pid uid, cost time
            POWER_KHILOGI(FEATURE_SHUTDOWN,
                "ISUTcb P=%{public}dU=%{public}dT=%{public}" PRId64 "", pidUid.first, pidUid.second, cost);
        }
    }
}

bool ShutdownController::TakeOverShutdownAction(const std::string& reason, bool isReboot)
{
    if (AllowedToBeTakenOver(reason)) {
        return TriggerTakeOverShutdownCallback(TakeOverInfo(reason, isReboot));
    }
    return false;
}

bool ShutdownController::AllowedToBeTakenOver(const std::string& reason) const
{
#ifdef POWER_MANAGER_ENABLE_JUDGING_TAKEOVER_SHUTDOWN
    if (find(REASONS_DISABLE_TAKE_OVER.cbegin(), REASONS_DISABLE_TAKE_OVER.cend(), reason)
        != REASONS_DISABLE_TAKE_OVER.cend()) {
        POWER_HILOGI(FEATURE_SHUTDOWN, "forbid to takeover shutdown, reason:%{public}s", reason.c_str());
        return false;
    }
    return true;
#endif
    (void)reason;
    return true;
}

#ifdef HAS_HIVIEWDFX_HISYSEVENT_PART
bool ShutdownController::ReportDoShutdown()
{
    HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::POWER, "STATE", HiviewDFX::HiSysEvent::EventType::STATISTIC,
        "STATE", static_cast<uint32_t>(PowerState::SHUTDOWN));
    HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::POWER, "SHUTDOWN_STATISTIC",
        HiviewDFX::HiSysEvent::EventType::STATISTIC, "DO_SHUTDOWN", static_cast<int8_t>(true));
    return true;
}
#endif
} // namespace PowerMgr
} // namespace OHOS
