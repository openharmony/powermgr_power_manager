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

#include "suspend_controller.h"
#include <datetime_ex.h>
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
#include <input_manager.h>
#endif
#include <securec.h>
#include <ipc_skeleton.h>
#include "power_log.h"
#include "power_mgr_service.h"
#include "power_state_callback_stub.h"
#include "setting_helper.h"
#include "system_suspend_controller.h"
#include "wakeup_controller.h"

namespace OHOS {
namespace PowerMgr {
using namespace OHOS::MMI;
namespace {
sptr<SettingObserver> g_suspendSourcesKeyObserver = nullptr;
FFRTHandle g_sleepTimeoutHandle;
FFRTHandle g_forceSleepDelayHandle;
FFRTHandle g_userActivityOffTimeoutHandle;
FFRTUtils::Mutex g_monitorMutex;
const uint32_t SLEEP_DELAY_MS = 5000;
} // namespace

std::atomic_bool onForceSleep = false;

/** SuspendController Implement */
SuspendController::SuspendController(
    std::shared_ptr<ShutdownController>& shutdownController, std::shared_ptr<PowerStateMachine>& stateMachine)
{
    shutdownController_ = shutdownController;
    stateMachine_ = stateMachine;
}

SuspendController::~SuspendController()
{
    if (g_suspendSourcesKeyObserver) {
        SettingHelper::UnregisterSettingSuspendSourcesObserver(g_suspendSourcesKeyObserver);
    }
}

void SuspendController::AddCallback(const sptr<ISyncSleepCallback>& callback, SleepPriority priority)
{
    RETURN_IF(callback == nullptr)
    SleepCallbackHolder::GetInstance().AddCallback(callback, priority);
    POWER_HILOGI(FEATURE_SUSPEND,
        "sync sleep callback added, priority=%{public}u, pid=%{public}d, uid=%{public}d", priority,
        IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid());
}

void SuspendController::RemoveCallback(const sptr<ISyncSleepCallback>& callback)
{
    RETURN_IF(callback == nullptr)
    SleepCallbackHolder::GetInstance().RemoveCallback(callback);
    POWER_HILOGI(FEATURE_SUSPEND,
        "sync sleep callback removed, pid=%{public}d, uid=%{public}d",
        IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid());
}

void SuspendController::TriggerSyncSleepCallback(bool isWakeup)
{
    POWER_HILOGI(FEATURE_SUSPEND,
        "TriggerSyncSleepCallback, isWakeup=%{public}d, onForceSleep=%{public}d",
        isWakeup, onForceSleep == true);
    auto highPriorityCallbacks = SleepCallbackHolder::GetInstance().GetHighPriorityCallbacks();
    TriggerSyncSleepCallbackInner(highPriorityCallbacks, isWakeup);
    auto defaultPriorityCallbacks = SleepCallbackHolder::GetInstance().GetDefaultPriorityCallbacks();
    TriggerSyncSleepCallbackInner(defaultPriorityCallbacks, isWakeup);
    auto lowPriorityCallbacks = SleepCallbackHolder::GetInstance().GetLowPriorityCallbacks();
    TriggerSyncSleepCallbackInner(lowPriorityCallbacks, isWakeup);

    if (isWakeup && onForceSleep) {
        onForceSleep = false;
    }
}

void SuspendController::TriggerSyncSleepCallbackInner(std::set<sptr<ISyncSleepCallback>>& callbacks, bool isWakeup)
{
    for (auto &callback : callbacks) {
        if (callback != nullptr) {
            int64_t start = GetTickCount();
            isWakeup ? callback->OnSyncWakeup(onForceSleep) : callback->OnSyncSleep(onForceSleep);
            int64_t cost = GetTickCount() - start;
            POWER_HILOGI(FEATURE_SUSPEND,  "Trigger sync sleep callback success, cost=%{public}" PRId64 "", cost);
        }
    }
}

class SuspendPowerStateCallback : public PowerStateCallbackStub {
public:
    explicit SuspendPowerStateCallback(std::shared_ptr<SuspendController> controller) : controller_(controller) {};
    virtual ~SuspendPowerStateCallback() = default;
    void OnPowerStateChanged(PowerState state) override
    {
        auto controller = controller_.lock();
        if (controller == nullptr) {
            POWER_HILOGI(FEATURE_SUSPEND, "OnPowerStateChanged: No controller");
            return;
        }
        if (state == PowerState::AWAKE) {
            POWER_HILOGI(FEATURE_SUSPEND, "Turn awake, stop sleep timer");
            controller->StopSleep();
        }
    }

private:
    std::weak_ptr<SuspendController> controller_;
};

void SuspendController::Init()
{
    std::lock_guard lock(mutex_);
    queue_ = std::make_shared<FFRTQueue>("power_suspend_controller");
    if (queue_ == nullptr) {
        POWER_HILOGE(FEATURE_SUSPEND, "suspendQueue_ is null");
        return;
    }
    std::shared_ptr<SuspendSources> sources = SuspendSourceParser::ParseSources();
    sourceList_ = sources->GetSourceList();
    if (sourceList_.empty()) {
        POWER_HILOGE(FEATURE_SUSPEND, "InputManager is null");
        return;
    }

    for (auto source = sourceList_.begin(); source != sourceList_.end(); source++) {
        POWER_HILOGI(FEATURE_SUSPEND, "registered type=%{public}u action=%{public}u delayMs=%{public}u",
            (*source).GetReason(), (*source).GetAction(), (*source).GetDelay());
        std::shared_ptr<SuspendMonitor> monitor = SuspendMonitor::CreateMonitor(*source);
        if (monitor != nullptr && monitor->Init()) {
            POWER_HILOGI(FEATURE_SUSPEND, "register type=%{public}u", (*source).GetReason());
            monitor->RegisterListener(std::bind(&SuspendController::ControlListener, this, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3));
            g_monitorMutex.Lock();
            monitorMap_.emplace(monitor->GetReason(), monitor);
            g_monitorMutex.Unlock();
        }
    }
    sptr<SuspendPowerStateCallback> callback = new SuspendPowerStateCallback(shared_from_this());
    if (stateMachine_ == nullptr) {
        POWER_HILOGE(FEATURE_SUSPEND, "Can't get PowerStateMachine");
        return;
    }
    stateMachine_->RegisterPowerStateCallback(callback);
    RegisterSettingsObserver();
}

void SuspendController::ExecSuspendMonitorByReason(SuspendDeviceType reason)
{
    g_monitorMutex.Lock();
    if (monitorMap_.find(reason) != monitorMap_.end()) {
        auto monitor = monitorMap_[reason];
        if (monitor == nullptr) {
            POWER_HILOGI(COMP_SVC, "get monitor fail");
            g_monitorMutex.Unlock();
            return;
        }
        monitor->Notify();
    }
    g_monitorMutex.Unlock();
}

void SuspendController::RegisterSettingsObserver()
{
    if (g_suspendSourcesKeyObserver) {
        POWER_HILOGE(FEATURE_POWER_STATE, "suspend sources key observer is already registered");
        return;
    }
    SettingObserver::UpdateFunc updateFunc = [&](const std::string&) {
        POWER_HILOGI(COMP_SVC, "start setting string update");
        std::lock_guard lock(mutex_);
        std::string jsonStr = SettingHelper::GetSettingSuspendSources();
        std::shared_ptr<SuspendSources> sources = SuspendSourceParser::ParseSources(jsonStr);
        std::vector<SuspendSource> updateSourceList = sources->GetSourceList();
        if (updateSourceList.size() == 0) {
            return;
        }
        sourceList_ = updateSourceList;
        POWER_HILOGI(COMP_SVC, "start updateListener");
        Cancel();
        for (auto source = sourceList_.begin(); source != sourceList_.end(); source++) {
            std::shared_ptr<SuspendMonitor> monitor = SuspendMonitor::CreateMonitor(*source);
            if (monitor != nullptr && monitor->Init()) {
                monitor->RegisterListener(std::bind(&SuspendController::ControlListener, this, std::placeholders::_1,
                    std::placeholders::_2, std::placeholders::_3));
                g_monitorMutex.Lock();
                monitorMap_.emplace(monitor->GetReason(), monitor);
                g_monitorMutex.Unlock();
            }
        }
    };
    g_suspendSourcesKeyObserver = SettingHelper::RegisterSettingSuspendSourcesObserver(updateFunc);
    POWER_HILOGI(FEATURE_POWER_STATE, "register setting observer fin");
}

void SuspendController::Execute()
{
    HandleAction(GetLastReason(), GetLastAction());
}

void SuspendController::Cancel()
{
    g_monitorMutex.Lock();
    for (auto monitor = monitorMap_.begin(); monitor != monitorMap_.end(); monitor++) {
        monitor->second->Cancel();
    }
    monitorMap_.clear();
    g_monitorMutex.Unlock();
}

void SuspendController::StopSleep()
{
    if (sleepAction_ != static_cast<uint32_t>(SuspendAction::ACTION_NONE)) {
        FFRTUtils::CancelTask(g_sleepTimeoutHandle, queue_);
        FFRTUtils::CancelTask(g_forceSleepDelayHandle, queue_);
        sleepTime_ = -1;
        sleepAction_ = static_cast<uint32_t>(SuspendAction::ACTION_NONE);
    }
}

void SuspendController::HandleEvent(int64_t delayTime)
{
    FFRTTask task = [&]() {
        g_monitorMutex.Lock();
        auto timeoutSuspendMonitor = monitorMap_.find(SuspendDeviceType::SUSPEND_DEVICE_REASON_TIMEOUT);
        if (timeoutSuspendMonitor == monitorMap_.end()) {
            g_monitorMutex.Unlock();
            return;
        }
        g_monitorMutex.Unlock();
        auto monitor = timeoutSuspendMonitor->second;
        monitor->HandleEvent();
    };
    g_userActivityOffTimeoutHandle = FFRTUtils::SubmitDelayTask(task, delayTime, queue_);
}

void SuspendController::CancelEvent()
{
    FFRTUtils::CancelTask(g_userActivityOffTimeoutHandle, queue_);
}

void SuspendController::RecordPowerKeyDown()
{
    if (stateMachine_ == nullptr) {
        POWER_HILOGE(FEATURE_SUSPEND, "Can't get PowerStateMachine");
        return;
    }
    bool isScreenOn = stateMachine_->IsScreenOn();
    POWER_HILOGI(FEATURE_SUSPEND, "Suspend record key down action isScreenOn=%{public}d", isScreenOn);
    if (!isScreenOn) {
        powerkeyDownWhenScreenOff_ = true;
    } else {
        powerkeyDownWhenScreenOff_ = false;
    }

    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        return;
    }

    if (pms->CheckDialogAndShuttingDown()) {
        return;
    }
}

bool SuspendController::GetPowerkeyDownWhenScreenOff()
{
    bool powerKeyDown = powerkeyDownWhenScreenOff_;
    powerkeyDownWhenScreenOff_ = false;
    return powerKeyDown;
}

void SuspendController::SuspendWhenScreenOff(SuspendDeviceType reason, uint32_t action, uint32_t delay)
{
    if (reason != SuspendDeviceType::SUSPEND_DEVICE_REASON_SWITCH) {
        POWER_HILOGI(FEATURE_SUSPEND, "Do nothing for reason %{public}d", reason);
        return;
    }
    if (stateMachine_ == nullptr) {
        return;
    }

    POWER_HILOGI(FEATURE_SUSPEND,
        "Suspend when screen off, reason=%{public}d, action=%{public}u, "
        "delay=%{public}u" PRId32 " ,state=%{public}d, type=%{public}u",
        reason, action, delay, stateMachine_->GetState(), sleepType_);
    switch (stateMachine_->GetState()) {
        case PowerState::INACTIVE:
            StopSleep();
            StartSleepTimer(reason, action, delay);
            break;
        case PowerState::SLEEP:
            if (action != static_cast<uint32_t>(SuspendAction::ACTION_FORCE_SUSPEND)) {
                break;
            }
            if (sleepType_ == static_cast<uint32_t>(SuspendAction::ACTION_AUTO_SUSPEND)) {
                SystemSuspendController::GetInstance().Wakeup();
                StartSleepTimer(reason, action, 0);
            } else if (sleepType_ == static_cast<uint32_t>(SuspendAction::ACTION_FORCE_SUSPEND)) {
                SystemSuspendController::GetInstance().Wakeup();
                SystemSuspendController::GetInstance().Suspend([]() {}, []() {}, true);
            } else {
                POWER_HILOGD(FEATURE_SUSPEND, "Nothing to do for no suspend");
            }
            break;
        default:
            break;
    }
}

void SuspendController::ControlListener(SuspendDeviceType reason, uint32_t action, uint32_t delay)
{
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        return;
    }

    if (pms->CheckDialogAndShuttingDown()) {
        return;
    }

    if (!pms->IsScreenOn()) {
        SuspendWhenScreenOff(reason, action, delay);
        return;
    }

    pid_t pid = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    POWER_HILOGI(FEATURE_SUSPEND,
        "Try to suspend device, pid=%{public}d, uid=%{public}d, reason=%{public}d, action=%{public}u, "
        "delay=%{public}u" PRId32 "",
        pid, uid, reason, action, delay);
    bool force = true;
    if (reason == SuspendDeviceType::SUSPEND_DEVICE_REASON_TIMEOUT) {
        force = false;
    }
    if (stateMachine_ == nullptr) {
        POWER_HILOGE(FEATURE_SUSPEND, "Can't get PowerStateMachine");
        return;
    }
    bool ret = stateMachine_->SetState(
        PowerState::INACTIVE, stateMachine_->GetReasionBySuspendType(static_cast<SuspendDeviceType>(reason)), force);
    if (ret) {
        StartSleepTimer(reason, action, delay);
    }
}

void SuspendController::StartSleepTimer(SuspendDeviceType reason, uint32_t action, uint32_t delay)
{
    if (static_cast<SuspendAction>(action) == SuspendAction::ACTION_AUTO_SUSPEND) {
        delay = delay + SLEEP_DELAY_MS;
    }
    const int64_t& tmpRef = delay;
    int64_t timeout = GetTickCount() + tmpRef;
    if ((timeout > sleepTime_) && (sleepTime_ != -1)) {
        POWER_HILOGI(FEATURE_SUSPEND, "already have a sleep event (%{public}" PRId64 " > %{public}" PRId64 ")", timeout,
            sleepTime_);
        return;
    }
    sleepTime_ = timeout;
    sleepReason_ = reason;
    sleepAction_ = action;
    sleepDuration_ = delay;
    sleepType_ = action;
    if (delay == 0) {
        HandleAction(reason, action);
    } else {
        FFRTTask task = [this] {
            HandleAction(GetLastReason(), GetLastAction());
        };
        g_sleepTimeoutHandle = FFRTUtils::SubmitDelayTask(task, delay, queue_);
    }
}

void SuspendController::HandleAction(SuspendDeviceType reason, uint32_t action)
{
    switch (static_cast<SuspendAction>(action)) {
        case SuspendAction::ACTION_AUTO_SUSPEND:
            HandleAutoSleep(reason);
            break;
        case SuspendAction::ACTION_FORCE_SUSPEND:
            HandleForceSleep(reason);
            break;
        case SuspendAction::ACTION_HIBERNATE:
            HandleHibernate(reason);
            break;
        case SuspendAction::ACTION_SHUTDOWN:
            HandleShutdown(reason);
            break;
        case SuspendAction::ACTION_NONE:
        default:
            break;
    }
    if (static_cast<SuspendAction>(action) != SuspendAction::ACTION_FORCE_SUSPEND) {
        sleepTime_ = -1;
        sleepAction_ = static_cast<uint32_t>(SuspendAction::ACTION_NONE);
    }
}

void SuspendController::HandleAutoSleep(SuspendDeviceType reason)
{
    POWER_HILOGI(FEATURE_SUSPEND, "auto suspend by reason=%{public}d", reason);

    if (stateMachine_ == nullptr) {
        POWER_HILOGE(FEATURE_SUSPEND, "Can't get PowerStateMachine");
        return;
    }
    bool ret = stateMachine_->SetState(
        PowerState::SLEEP, stateMachine_->GetReasionBySuspendType(reason));
    if (ret) {
        POWER_HILOGI(FEATURE_SUSPEND, "State changed, set sleep timer");
        TriggerSyncSleepCallback(false);
        SystemSuspendController::GetInstance().Suspend([]() {}, []() {}, false);
    } else {
        POWER_HILOGI(FEATURE_SUSPEND, "auto suspend: State change failed");
    }
}

void SuspendController::HandleForceSleep(SuspendDeviceType reason)
{
    POWER_HILOGI(FEATURE_SUSPEND, "force suspend by reason=%{public}d", reason);
    if (stateMachine_ == nullptr) {
        POWER_HILOGE(FEATURE_SUSPEND, "Can't get PowerStateMachine");
        return;
    }
    bool ret = stateMachine_->SetState(
        PowerState::SLEEP, stateMachine_->GetReasionBySuspendType(reason), true);
    if (ret) {
        POWER_HILOGI(FEATURE_SUSPEND, "State changed, system suspend");
        onForceSleep = true;
        TriggerSyncSleepCallback(false);

        FFRTTask task = [this] {
            SystemSuspendController::GetInstance().Suspend([]() {}, []() {}, true);
            sleepTime_ = -1;
            sleepAction_ = static_cast<uint32_t>(SuspendAction::ACTION_NONE);
        };
        g_forceSleepDelayHandle = FFRTUtils::SubmitDelayTask(task, FORCE_SLEEP_DELAY_MS, queue_);
    } else {
        POWER_HILOGI(FEATURE_SUSPEND, "force suspend: State change failed");
    }
}

void SuspendController::HandleHibernate(SuspendDeviceType reason)
{
    POWER_HILOGI(FEATURE_SUSPEND, "force suspend by reason=%{public}d", reason);
    if (stateMachine_ == nullptr) {
        POWER_HILOGE(FEATURE_SUSPEND, "Can't get PowerStateMachine");
        return;
    }
    bool ret = stateMachine_->SetState(
        PowerState::HIBERNATE, stateMachine_->GetReasionBySuspendType(reason), true);
    if (ret) {
        POWER_HILOGI(FEATURE_SUSPEND, "State changed, call hibernate");
    } else {
        POWER_HILOGI(FEATURE_SUSPEND, "Hibernate: State change failed");
    }
}

void SuspendController::HandleShutdown(SuspendDeviceType reason)
{
    POWER_HILOGI(FEATURE_SUSPEND, "shutdown by reason=%{public}d", reason);
    shutdownController_->Shutdown(std::to_string(static_cast<uint32_t>(reason)));
}

void SuspendController::Reset()
{
    queue_.reset();
}

const std::shared_ptr<SuspendMonitor> SuspendMonitor::CreateMonitor(SuspendSource& source)
{
    SuspendDeviceType reason = source.GetReason();
    POWER_HILOGI(FEATURE_SUSPEND, "CreateMonitor reason=%{public}d", reason);
    std::shared_ptr<SuspendMonitor> monitor = nullptr;
    switch (reason) {
        case SuspendDeviceType::SUSPEND_DEVICE_REASON_POWER_KEY:
            monitor = std::static_pointer_cast<SuspendMonitor>(std::make_shared<PowerKeySuspendMonitor>(source));
            break;
        case SuspendDeviceType::SUSPEND_DEVICE_REASON_TIMEOUT:
            monitor = std::static_pointer_cast<SuspendMonitor>(std::make_shared<TimeoutSuspendMonitor>(source));
            break;
        case SuspendDeviceType::SUSPEND_DEVICE_REASON_LID:
            monitor = std::static_pointer_cast<SuspendMonitor>(std::make_shared<LidSuspendMonitor>(source));
            break;
        case SuspendDeviceType::SUSPEND_DEVICE_REASON_SWITCH:
            monitor = std::static_pointer_cast<SuspendMonitor>(std::make_shared<SwitchSuspendMonitor>(source));
            break;
        default:
            POWER_HILOGE(FEATURE_SUSPEND, "CreateMonitor : Invalid reason=%{public}d", reason);
            break;
    }
    return monitor;
}

/** PowerKeySuspendMonitor Implement */
bool PowerKeySuspendMonitor::Init()
{
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
    if (powerkeyReleaseId_ >= 0) {
        return true;
    }
    std::shared_ptr<OHOS::MMI::KeyOption> keyOption = std::make_shared<OHOS::MMI::KeyOption>();
    std::set<int32_t> preKeys;

    keyOption.reset();
    keyOption = std::make_shared<OHOS::MMI::KeyOption>();
    keyOption->SetPreKeys(preKeys);
    keyOption->SetFinalKey(OHOS::MMI::KeyEvent::KEYCODE_POWER);
    keyOption->SetFinalKeyDown(false);
    keyOption->SetFinalKeyDownDuration(0);
    powerkeyReleaseId_ = InputManager::GetInstance()->SubscribeKeyEvent(
        keyOption, [this](std::shared_ptr<OHOS::MMI::KeyEvent> keyEvent) {
            POWER_HILOGI(FEATURE_SUSPEND, "Receive key on notify");
            auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
            if (pms == nullptr) {
                return;
            }
            std::shared_ptr<SuspendController> suspendController = pms->GetSuspendController();
            if (suspendController->GetPowerkeyDownWhenScreenOff()) {
                POWER_HILOGI(FEATURE_SUSPEND, "no action suspend");
                return;
            }
            Notify();
        });
    POWER_HILOGI(FEATURE_SUSPEND, "powerkeyReleaseId_=%{public}d", powerkeyReleaseId_);
    return powerkeyReleaseId_ >= 0 ? true : false;
#else
    return false;
#endif
}

void PowerKeySuspendMonitor::Cancel()
{
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
    if (powerkeyReleaseId_ >= 0) {
        InputManager::GetInstance()->UnsubscribeKeyEvent(powerkeyReleaseId_);
        powerkeyReleaseId_ = -1;
    }
#endif
}

/** Timeout Implement */
bool TimeoutSuspendMonitor::Init()
{
    return true;
}

void TimeoutSuspendMonitor::Cancel() {}

void TimeoutSuspendMonitor::HandleEvent()
{
    POWER_HILOGI(FEATURE_INPUT, "TimeoutSuspendMonitor HandleEvent");
    Notify();
}

/** LidSuspendMonitor Implement */

bool LidSuspendMonitor::Init()
{
    return true;
}

void LidSuspendMonitor::Cancel() {}

/** SwitchSuspendMonitor Implement */

bool SwitchSuspendMonitor::Init()
{
    return true;
}

void SwitchSuspendMonitor::Cancel() {}

} // namespace PowerMgr
} // namespace OHOS
