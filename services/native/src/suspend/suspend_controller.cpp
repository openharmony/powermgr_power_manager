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
#ifdef HAS_HIVIEWDFX_HISYSEVENT_PART
#include <hisysevent.h>
#endif

namespace OHOS {
namespace PowerMgr {
using namespace OHOS::MMI;
namespace {
#ifdef POWER_MANAGER_ENABLE_CHARGING_TYPE_SETTING
sptr<SettingObserver> g_suspendSourcesKeyAcObserver = nullptr;
sptr<SettingObserver> g_suspendSourcesKeyDcObserver = nullptr;
#else
sptr<SettingObserver> g_suspendSourcesKeyObserver = nullptr;
#endif
FFRTMutex g_monitorMutex;
const uint32_t SLEEP_DELAY_MS = 5000;
constexpr int64_t POWERKEY_MIN_INTERVAL = 350; // ms
} // namespace

std::atomic_bool onForceSleep = false;

/** SuspendController Implement */
SuspendController::SuspendController(const std::shared_ptr<ShutdownController>& shutdownController,
    const std::shared_ptr<PowerStateMachine>& stateMachine, const std::shared_ptr<FFRTTimer>& ffrtTimer)
{
    shutdownController_ = shutdownController;
    stateMachine_ = stateMachine;
    ffrtTimer_ = ffrtTimer;
}

SuspendController::~SuspendController()
{
#ifdef POWER_MANAGER_ENABLE_CHARGING_TYPE_SETTING
    if (g_suspendSourcesKeyAcObserver) {
        SettingHelper::UnregisterSettingObserver(g_suspendSourcesKeyAcObserver);
        g_suspendSourcesKeyAcObserver = nullptr;
    }
    if (g_suspendSourcesKeyDcObserver) {
        SettingHelper::UnregisterSettingObserver(g_suspendSourcesKeyDcObserver);
        g_suspendSourcesKeyDcObserver = nullptr;
    }
#else
    if (g_suspendSourcesKeyObserver) {
        SettingHelper::UnregisterSettingObserver(g_suspendSourcesKeyObserver);
        g_suspendSourcesKeyObserver = nullptr;
    }
#endif
    ffrtTimer_.reset();
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
    TriggerSyncSleepCallbackInner(highPriorityCallbacks, "High", isWakeup);
    auto defaultPriorityCallbacks = SleepCallbackHolder::GetInstance().GetDefaultPriorityCallbacks();
    TriggerSyncSleepCallbackInner(defaultPriorityCallbacks, "Default", isWakeup);
    auto lowPriorityCallbacks = SleepCallbackHolder::GetInstance().GetLowPriorityCallbacks();
    TriggerSyncSleepCallbackInner(lowPriorityCallbacks, "Low", isWakeup);

    if (isWakeup && onForceSleep) {
        onForceSleep = false;
    }
}

void SuspendController::TriggerSyncSleepCallbackInner(std::set<sptr<ISyncSleepCallback>>& callbacks,
    const std::string& priority, bool isWakeup)
{
    uint32_t id = 0;
    for (auto &callback : callbacks) {
        if (callback != nullptr) {
            int64_t start = GetTickCount();
            isWakeup ? callback->OnSyncWakeup(onForceSleep) : callback->OnSyncSleep(onForceSleep);
            int64_t cost = GetTickCount() - start;
            POWER_HILOGI(FEATURE_SUSPEND,
                "Trigger %{public}s SyncSleepCallback[%{public}u] success, cost=%{public}" PRId64,
                priority.c_str(), ++id, cost);
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
            POWER_HILOGI(FEATURE_SUSPEND, "monitor init success, type=%{public}u", (*source).GetReason());
            monitor->RegisterListener([this](SuspendDeviceType reason, uint32_t action, uint32_t delay) {
                this->ControlListener(reason, action, delay);
            });
            g_monitorMutex.lock();
            monitorMap_.emplace(monitor->GetReason(), monitor);
            g_monitorMutex.unlock();
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
    FFRTUtils::SubmitTask([this, reason] {
        g_monitorMutex.lock();
        if (monitorMap_.find(reason) != monitorMap_.end()) {
            auto monitor = monitorMap_[reason];
            if (monitor == nullptr) {
                POWER_HILOGI(COMP_SVC, "get monitor fail");
                g_monitorMutex.unlock();
                return;
            }
            monitor->Notify();
        }
        g_monitorMutex.unlock();
    });
}

void SuspendController::UpdateSuspendSources()
{
    POWER_HILOGI(COMP_SVC, "start setting string update");
    std::lock_guard lock(mutex_);

    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        POWER_HILOGE(COMP_SVC, "get PowerMgrService fail");
        return;
    }
    std::string jsonStr;
#ifdef POWER_MANAGER_ENABLE_CHARGING_TYPE_SETTING
    if (pms->IsPowerConnected()) {
        jsonStr = SettingHelper::GetSettingAcSuspendSources();
    } else {
        jsonStr = SettingHelper::GetSettingDcSuspendSources();
    }
#else
    jsonStr = SettingHelper::GetSettingSuspendSources();
#endif
    std::shared_ptr<SuspendSources> sources = SuspendSourceParser::ParseSources(jsonStr);
    std::vector<SuspendSource> updateSourceList = sources->GetSourceList();
    if (updateSourceList.size() == 0) {
        return;
    }
    sourceList_ = updateSourceList;
    POWER_HILOGI(COMP_SVC, "start updateListener");
    Cancel();
    uint32_t id = 0;
    for (auto source = sourceList_.begin(); source != sourceList_.end(); source++, id++) {
        std::shared_ptr<SuspendMonitor> monitor = SuspendMonitor::CreateMonitor(*source);
        POWER_HILOGI(FEATURE_SUSPEND, "UpdateFunc CreateMonitor[%{public}u] reason=%{public}d",
            id, source->GetReason());
        if (monitor != nullptr && monitor->Init()) {
            monitor->RegisterListener([this](SuspendDeviceType reason, uint32_t action, uint32_t delay) {
                this->ControlListener(reason, action, delay);
            });
            g_monitorMutex.lock();
            monitorMap_.emplace(monitor->GetReason(), monitor);
            g_monitorMutex.unlock();
        }
    }
}

void SuspendController::RegisterSettingsObserver()
{
#ifdef POWER_MANAGER_ENABLE_CHARGING_TYPE_SETTING
    if (g_suspendSourcesKeyAcObserver && g_suspendSourcesKeyDcObserver) {
#else
    if (g_suspendSourcesKeyObserver) {
#endif
        POWER_HILOGE(FEATURE_POWER_STATE, "suspend sources key observer is already registered");
        return;
    }
    SettingObserver::UpdateFunc updateFunc = [&](const std::string&) {
        SuspendController::UpdateSuspendSources();
    };
#ifdef POWER_MANAGER_ENABLE_CHARGING_TYPE_SETTING
    if (g_suspendSourcesKeyAcObserver == nullptr) {
        g_suspendSourcesKeyAcObserver = SettingHelper::RegisterSettingAcSuspendSourcesObserver(updateFunc);
    }
    if (g_suspendSourcesKeyDcObserver == nullptr) {
        g_suspendSourcesKeyDcObserver = SettingHelper::RegisterSettingDcSuspendSourcesObserver(updateFunc);
    }
#else
    g_suspendSourcesKeyObserver = SettingHelper::RegisterSettingSuspendSourcesObserver(updateFunc);
#endif
    POWER_HILOGI(FEATURE_POWER_STATE, "register setting observer fin");
}

void SuspendController::Execute()
{
    HandleAction(GetLastReason(), GetLastAction());
}

void SuspendController::Cancel()
{
    g_monitorMutex.lock();
    for (auto monitor = monitorMap_.begin(); monitor != monitorMap_.end(); monitor++) {
        monitor->second->Cancel();
    }
    monitorMap_.clear();
    g_monitorMutex.unlock();
}

void SuspendController::StopSleep()
{
    ffrtMutexMap_.Lock(TIMER_ID_SLEEP);
    if (ffrtTimer_ != nullptr) {
        ffrtTimer_->CancelTimer(TIMER_ID_SLEEP);
    }
    sleepTime_ = -1;
    sleepAction_ = static_cast<uint32_t>(SuspendAction::ACTION_NONE);
    ffrtMutexMap_.Unlock(TIMER_ID_SLEEP);
}

void SuspendController::HandleEvent(int64_t delayTime)
{
    FFRTTask task = [&]() {
        g_monitorMutex.lock();
        auto timeoutSuspendMonitor = monitorMap_.find(SuspendDeviceType::SUSPEND_DEVICE_REASON_TIMEOUT);
        if (timeoutSuspendMonitor == monitorMap_.end()) {
            g_monitorMutex.unlock();
            return;
        }

        auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
        if (pms != nullptr) {
            if (pms->CheckDialogFlag()) {
                POWER_HILOGI(FEATURE_SUSPEND, "Reset long press flag before suspending device by timeout");
            }
        }
        if (stateMachine_ != nullptr) {
            int32_t timeout = stateMachine_->GetDisplayOffTime();
            POWER_HILOGI(FEATURE_INPUT, "This time of timeout is %{public}d ms", timeout);
        }
        g_monitorMutex.unlock();
        auto monitor = timeoutSuspendMonitor->second;
        monitor->HandleEvent();
    };
    if (ffrtTimer_ != nullptr) {
        ffrtTimer_->SetTimer(TIMER_ID_USER_ACTIVITY_OFF, task, delayTime);
    } else {
        POWER_HILOGE(FEATURE_SUSPEND, "%{public}s: SetTimer(%{public}s) failed, timer is null", __func__,
            std::to_string(delayTime).c_str());
    }
}

void SuspendController::CancelEvent()
{
    if (ffrtTimer_ != nullptr) {
        ffrtTimer_->CancelTimer(TIMER_ID_USER_ACTIVITY_OFF);
    }
}

void SuspendController::RecordPowerKeyDown(bool interrupting)
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
        if (interrupting) {
            POWER_HILOGI(FEATURE_SUSPEND, "Suspend record key down after interrupting screen off");
        }
        powerkeyDownWhenScreenOff_ = interrupting;
    }

    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        return;
    }

    if (pms->CheckDialogFlag()) {
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
        "delay=%{public}u, state=%{public}d, type=%{public}u",
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
                if (stateMachine_->IsSwitchOpen()) {
                    POWER_HILOGI(FEATURE_SUSPEND, "switch off event is ignored.");
                    return;
                }
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
        "[UL_POWER] Try to suspend device, pid=%{public}d, uid=%{public}d, reason=%{public}d, action=%{public}u, "
        "delay=%{public}u",
        pid, uid, reason, action, delay);
    bool force = true;
    if (reason == SuspendDeviceType::SUSPEND_DEVICE_REASON_TIMEOUT) {
        force = false;
    }
#ifdef HAS_HIVIEWDFX_HISYSEVENT_PART
    HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::POWER, "SLEEP_START",
        HiviewDFX::HiSysEvent::EventType::BEHAVIOR, "TRIGGER_EVENT_TYPE", static_cast<int32_t>(reason),
        "ACTION_EVENT_TYPE", static_cast<int32_t>(force));
#endif
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
        if (stateMachine_->GetSleepTime() < 0) {
            POWER_HILOGI(FEATURE_SUSPEND, "sleeptime less than zero, no need suspend");
            return;
        }
        delay = delay + SLEEP_DELAY_MS;
    }

    int64_t tick = GetTickCount();
    int64_t timeout = tick + static_cast<int64_t>(delay);
    if (timeout < tick) {
        POWER_HILOGE(FEATURE_SUSPEND, "Sleep timer overflow with tick = %{public}s, delay = %{public}u",
            std::to_string(tick).c_str(), delay);
        return;
    }

    ffrtMutexMap_.Lock(TIMER_ID_SLEEP);
    if ((timeout > sleepTime_) && (sleepTime_ != -1)) {
        POWER_HILOGI(FEATURE_SUSPEND, "already have a sleep event (%{public}" PRId64 " > %{public}" PRId64 ")", timeout,
            sleepTime_);
        ffrtMutexMap_.Unlock(TIMER_ID_SLEEP);
        return;
    }
    sleepTime_ = timeout;
    sleepReason_ = reason;
    sleepAction_ = action;
    sleepDuration_ = delay;
    sleepType_ = action;
    FFRTTask task = [this, reason, action] {
        HandleAction(reason, action);
    };

    if (ffrtTimer_ != nullptr) {
        ffrtTimer_->SetTimer(TIMER_ID_SLEEP, task, delay);
    } else {
        POWER_HILOGE(FEATURE_SUSPEND, "%{public}s: SetTimer(%{public}u) failed, timer is null", __func__, delay);
    }
    ffrtMutexMap_.Unlock(TIMER_ID_SLEEP);
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
    ffrtMutexMap_.Lock(TIMER_ID_SLEEP);
    sleepTime_ = -1;
    sleepAction_ = static_cast<uint32_t>(SuspendAction::ACTION_NONE);
    ffrtMutexMap_.Unlock(TIMER_ID_SLEEP);
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

#ifdef POWER_MANAGER_ENABLE_FORCE_SLEEP_BROADCAST
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms != nullptr && pms->GetSuspendController() != nullptr) {
        pms->GetSuspendController()->SetForceSleepingFlag(true);
        POWER_HILOGI(FEATURE_SUSPEND, "Set flag of force sleeping to true");
    } else {
        POWER_HILOGE(FEATURE_SUSPEND, "Failed to set flag of force sleeping, pms or suspendController is nullptr");
    }
#endif
    bool ret = stateMachine_->SetState(PowerState::SLEEP,
        stateMachine_->GetReasionBySuspendType(reason), true);
    if (ret) {
        POWER_HILOGI(FEATURE_SUSPEND, "State changed, system suspend");
        onForceSleep = true;
        TriggerSyncSleepCallback(false);

        FFRTTask task = [this, reason] {
            SystemSuspendController::GetInstance().Suspend([]() {}, []() {}, true);
        };
        if (ffrtTimer_ != nullptr) {
            ffrtTimer_->SetTimer(TIMER_ID_SLEEP, task, FORCE_SLEEP_DELAY_MS);
        } else {
            POWER_HILOGE(FEATURE_SUSPEND, "%{public}s: SetTimer(%{public}d) failed, timer is null",
                __func__, FORCE_SLEEP_DELAY_MS);
        }
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
    ffrtTimer_.reset();
}

const std::shared_ptr<SuspendMonitor> SuspendMonitor::CreateMonitor(SuspendSource& source)
{
    SuspendDeviceType reason = source.GetReason();
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
            POWER_HILOGI(FEATURE_SUSPEND, "[UL_POWER] Received powerkey up");

            static int64_t lastPowerkeyUpTime = 0;
            int64_t currTime = GetTickCount();
            if (lastPowerkeyUpTime != 0 && currTime - lastPowerkeyUpTime < POWERKEY_MIN_INTERVAL) {
                POWER_HILOGI(FEATURE_WAKEUP, "[UL_POWER] Last powerkey up within 350ms, skip. "
                    "%{public}" PRId64 ", %{public}" PRId64, currTime, lastPowerkeyUpTime);
                return;
            }
            lastPowerkeyUpTime = currTime;

            auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
            if (pms == nullptr) {
                return;
            }
            std::shared_ptr<SuspendController> suspendController = pms->GetSuspendController();
            if (suspendController->GetPowerkeyDownWhenScreenOff()) {
                POWER_HILOGI(FEATURE_SUSPEND,
                    "[UL_POWER] The powerkey was pressed when screenoff, ignore this powerkey up event.");
                return;
            }
            auto powerkeyScreenOffTask = [*this]() mutable {
                Notify();
                powerkeyScreenOff_ = false;
                EndPowerkeyScreenOff();
            };
            BeginPowerkeyScreenOff();
            powerkeyScreenOff_ = true;
            ffrt::submit(powerkeyScreenOffTask, {}, {&powerkeyScreenOff_});
            POWER_HILOGI(FEATURE_SUSPEND, "[UL_POWER]submitted screen off ffrt task");
        });
    POWER_HILOGI(FEATURE_SUSPEND, "powerkeyReleaseId_=%{public}d", powerkeyReleaseId_);
    return powerkeyReleaseId_ >= 0 ? true : false;
#else
    return false;
#endif
}

void PowerKeySuspendMonitor::BeginPowerkeyScreenOff() const
{
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        return;
    }
    auto stateMachine = pms->GetPowerStateMachine();
    if (stateMachine == nullptr) {
        return;
    }
    auto stateAction = stateMachine->GetStateAction();
    if (stateAction == nullptr) {
        return;
    }
    stateAction->BeginPowerkeyScreenOff();
}

void PowerKeySuspendMonitor::EndPowerkeyScreenOff() const
{
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        return;
    }
    auto stateMachine = pms->GetPowerStateMachine();
    if (stateMachine == nullptr) {
        return;
    }
    auto stateAction = stateMachine->GetStateAction();
    if (stateAction == nullptr) {
        return;
    }
    stateAction->EndPowerkeyScreenOff();
}

void PowerKeySuspendMonitor::Cancel()
{
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
    if (powerkeyReleaseId_ >= 0) {
        POWER_HILOGI(FEATURE_SUSPEND, "UnsubscribeKeyEvent: PowerKeySuspendMonitor");
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
    POWER_HILOGI(FEATURE_INPUT, "TimeoutSuspendMonitor HandleEvent.");
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
