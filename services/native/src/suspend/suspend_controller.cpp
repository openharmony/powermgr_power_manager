/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
#include "power_log.h"
#include "power_mgr_service.h"
#include "power_state_callback_stub.h"
#include "powerms_event_handler.h"
#include "setting_helper.h"
#include "system_suspend_controller.h"
#include "wakeup_controller.h"
#include <datetime_ex.h>
#include <input_manager.h>
#include <securec.h>

using namespace OHOS::MMI;

namespace OHOS {
namespace PowerMgr {

namespace {
sptr<SettingObserver> g_suspendSourcesKeyObserver = nullptr;
}

/** SuspendController Implement */

SuspendController::SuspendController(ShutdownService* shutdownService, std::shared_ptr<PowerStateMachine>& stateMachine,
    std::shared_ptr<AppExecFwk::EventRunner>& runner)
{
    shutdownService_ = shutdownService;
    stateMachine_ = stateMachine;
    runner_ = runner;
}

SuspendController::~SuspendController()
{
    if (g_suspendSourcesKeyObserver) {
        SettingHelper::UnregisterSettingSuspendSourcesObserver(g_suspendSourcesKeyObserver);
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
            POWER_HILOGI(FEATURE_INPUT, "OnPowerStateChanged: No controller");
            return;
        }
        if (state == PowerState::AWAKE) {
            POWER_HILOGI(FEATURE_INPUT, "Turn awake, stop sleep timer");
            controller->StopSleep();
        }
    }

private:
    std::weak_ptr<SuspendController> controller_;
};

void SuspendController::Init()
{
    std::shared_ptr<SuspendSources> sources = SuspendSourceParser::ParseSources();
    sourceList_ = sources->GetSourceList();
    if (sourceList_.empty()) {
        POWER_HILOGE(FEATURE_INPUT, "InputManager is null");
        return;
    }

    for (auto source = sourceList_.begin(); source != sourceList_.end(); source++) {
        POWER_HILOGI(FEATURE_INPUT, "registered type %{public}u action %{public}u delayMs %{public}" PRId64 "",
            (*source).GetReason(), (*source).GetAction(), (*source).GetDelay());
        std::shared_ptr<SuspendMonitor> monitor = SuspendMonitor::CreateMonitor(*source);
        if (monitor != nullptr && monitor->Init()) {
            POWER_HILOGI(FEATURE_INPUT, "register type %{public}u", (*source).GetReason());
            monitor->RegisterListener(std::bind(&SuspendController::ControlListener, this, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3));
            monitorMap_.emplace(monitor->GetReason(), monitor);
        }
    }
    handler_ = std::make_shared<SuspendEventHandler>(runner_, shared_from_this());
    sptr<SuspendPowerStateCallback> callback = new SuspendPowerStateCallback(shared_from_this());
    stateMachine_->RegisterPowerStateCallback(callback);
    RegisterSettingsObserver();
}

void SuspendController::RegisterSettingsObserver()
{
    if (g_suspendSourcesKeyObserver) {
        POWER_HILOGE(FEATURE_POWER_STATE, "suspend sources key observer is already registered");
        return;
    }
    SettingObserver::UpdateFunc updateFunc = [&](const std::string&) {
        POWER_HILOGI(COMP_SVC, "lambda setting string update");
        std::string jsonStr = SettingHelper::GetSettingSuspendSources();
        std::shared_ptr<SuspendSources> sources = SuspendSourceParser::ParseSources(jsonStr);
        std::vector<SuspendSource> updateSourceList = sources->GetSourceList();
        if (updateSourceList.size() == 0) {
            return;
        }
        sourceList_ = updateSourceList;
        POWER_HILOGI(COMP_SVC, "go to updateListener");
        Cancel();
        for (auto source = sourceList_.begin(); source != sourceList_.end(); source++) {
            std::shared_ptr<SuspendMonitor> monitor = SuspendMonitor::CreateMonitor(*source);
            if (monitor != nullptr && monitor->Init()) {
                monitor->RegisterListener(std::bind(&SuspendController::ControlListener, this, std::placeholders::_1,
                    std::placeholders::_2, std::placeholders::_3));
                monitorMap_.emplace(monitor->GetReason(), monitor);
            }
        }
    };
    g_suspendSourcesKeyObserver = SettingHelper::RegisterSettingSuspendSourcesObserver(updateFunc);
    POWER_HILOGI(FEATURE_POWER_STATE, "register setting observer");
}

void SuspendController::Execute()
{
    HandleAction(GetLastReason(), GetLastAction());
}

void SuspendController::Cancel()
{
    for (auto monitor = monitorMap_.begin(); monitor != monitorMap_.end(); monitor++) {
        monitor->second->Cancel();
    }
    monitorMap_.clear();
}

void SuspendController::StopSleep()
{
    if (handler_ == nullptr) {
        return ;
    }

    if (sleepAction_ != static_cast<uint32_t>(SuspendAction::ACTION_NONE)) {
        handler_->RemoveEvent(SuspendEventHandler::SLEEP_TIMEOUT_MSG);
        sleepTime_ = -1;
        sleepAction_ = static_cast<uint32_t>(SuspendAction::ACTION_NONE);
    }
}

void SuspendController::HandleEvent(uint32_t eventId)
{
    POWER_HILOGE(FEATURE_INPUT, "HandleEvent: %{public}d", eventId);
    switch (eventId) {
        case PowermsEventHandler::CHECK_USER_ACTIVITY_OFF_TIMEOUT_MSG: {
            auto timeoutSuspendMonitor =
                monitorMap_.find(static_cast<uint32_t>(SuspendDeviceType::SUSPEND_DEVICE_REASON_TIMEOUT));
            if (timeoutSuspendMonitor != monitorMap_.end()) {
                timeoutSuspendMonitor->second->HandleEvent(
                    static_cast<uint32_t>(SuspendEventHandler::SCREEN_OFF_TIMEOUT_MSG));
            }
            break;
        }
        default:
            break;
    }
}

void SuspendController::RecordPowerKeyDown()
{
    bool isScreenOn = stateMachine_->IsScreenOn();
    POWER_HILOGI(FEATURE_INPUT, "Suspend record key down action isScreenOn %{public}d", isScreenOn);
    if (!isScreenOn) {
        powerkeyDownWhenScreenOff_ = true;
    }
}

bool SuspendController::GetPowerkeyDownWhenScreenOff()
{
    bool powerKeyDown = powerkeyDownWhenScreenOff_;
    powerkeyDownWhenScreenOff_ = false;
    return powerKeyDown;
}

void SuspendController::ControlListener(uint32_t reason, uint32_t action, int64_t delay)
{
    POWER_HILOGI(FEATURE_INPUT, "Suspend Request: %{public}d, %{public}d, %{public}" PRId64 "", reason, action, delay);
    if (shutdownService_->IsShuttingDown()) {
        POWER_HILOGI(FEATURE_INPUT, "system is shutting down, stop!");
        return;
    }
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        return;
    }

    if (!pms->IsScreenOn()) {
        return;
    }

    if (stateMachine_->GetState() == PowerState::AWAKE) {
        POWER_HILOGI(FEATURE_INPUT, "Suspend set inactive");
        bool force = true;
        if (reason == static_cast<uint32_t>(SuspendDeviceType::SUSPEND_DEVICE_REASON_TIMEOUT)) {
            force = false;
        }
        bool ret = stateMachine_->SetState(PowerState::INACTIVE,
            stateMachine_->GetReasionBySuspendType(static_cast<SuspendDeviceType>(reason)), force);
        if (ret) {
            StartSleepTimer(reason, action, delay);
        }
    } else {
        POWER_HILOGI(FEATURE_INPUT, "state %{public}u no transiator", stateMachine_->GetState());
    }
}

void SuspendController::StartSleepTimer(uint32_t reason, uint32_t action, int64_t delay)
{
    int64_t timeout = GetTickCount() + delay;
    if ((timeout > sleepTime_) && (sleepTime_ != -1)) {
        POWER_HILOGI(FEATURE_INPUT, "already have a sleep event (%{public}" PRId64 " > %{public}" PRId64 ")", timeout, sleepTime_);
        return;
    }
    sleepTime_ = timeout;
    sleepReason_ = reason;
    sleepAction_ = action;
    sleepDuration_ = delay;
    if (delay == 0) {
        HandleAction(reason, action);
    } else {
        handler_->SendEvent(SuspendEventHandler::SLEEP_TIMEOUT_MSG, delay);
    }
}

void SuspendController::HandleAction(uint32_t reason, uint32_t action)
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
    sleepTime_ = -1;
    sleepAction_ = static_cast<uint32_t>(SuspendAction::ACTION_NONE);
}

void SuspendController::HandleAutoSleep(uint32_t reason)
{
    POWER_HILOGI(FEATURE_INPUT, "auto suspend by %{public}d", reason);

    bool ret = stateMachine_->SetState(
        PowerState::SLEEP, stateMachine_->GetReasionBySuspendType(static_cast<SuspendDeviceType>(reason)));
    if (ret) {
        POWER_HILOGI(FEATURE_INPUT, "State changed, set sleep timer");
        SystemSuspendController::GetInstance().Suspend([]() {}, []() {}, false);
    } else {
        POWER_HILOGI(FEATURE_INPUT, "auto suspend: State change failed");
    }
}

void SuspendController::HandleForceSleep(uint32_t reason)
{
    POWER_HILOGI(FEATURE_INPUT, "force suspend by %{public}d", reason);
    bool ret = stateMachine_->SetState(
        PowerState::SLEEP, stateMachine_->GetReasionBySuspendType(static_cast<SuspendDeviceType>(reason)), true);
    if (ret) {
        POWER_HILOGI(FEATURE_INPUT, "State changed, system suspend");
        SystemSuspendController::GetInstance().Suspend([]() {}, []() {}, true);
    } else {
        POWER_HILOGI(FEATURE_INPUT, "force suspend: State change failed");
    }
}

void SuspendController::HandleHibernate(uint32_t reason)
{
    POWER_HILOGI(FEATURE_INPUT, "force suspend by %{public}d", reason);
    bool ret = stateMachine_->SetState(
        PowerState::HIBERNATE, stateMachine_->GetReasionBySuspendType(static_cast<SuspendDeviceType>(reason)), true);
    if (ret) {
        POWER_HILOGI(FEATURE_INPUT, "State changed, call hibernate");
        // need to add interface
    } else {
        POWER_HILOGI(FEATURE_INPUT, "Hibernate: State change failed");
    }
}

void SuspendController::HandleShutdown(uint32_t reason)
{
    POWER_HILOGI(FEATURE_INPUT, "shutdown by %{public}d", reason);
    shutdownService_->Shutdown(std::to_string(reason));
}

/** SuspendEventHandler Implement */
void SuspendEventHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    std::shared_ptr<SuspendController> controller = controller_.lock();
    if (controller == nullptr) {
        POWER_HILOGI(FEATURE_INPUT, "ProcessEvent: No controller");
        return;
    }
    POWER_HILOGI(FEATURE_INPUT, "recv event %{public}d", event->GetInnerEventId());
    switch (event->GetInnerEventId()) {
        case SLEEP_TIMEOUT_MSG: {
            POWER_HILOGI(FEATURE_INPUT, "reason %{public}u action %{public}u", controller->GetLastReason(),
                controller->GetLastAction());
            controller->HandleAction(controller->GetLastReason(), controller->GetLastAction());
            break;
        }
        default:
            break;
    }
    POWER_HILOGI(FEATURE_INPUT, "process event fin");
}

/** SuspendMonitor Implement */
std::shared_ptr<SuspendMonitor> SuspendMonitor::CreateMonitor(SuspendSource& source)
{
    SuspendDeviceType reason = source.GetReason();
    POWER_HILOGI(FEATURE_INPUT, "CreateMonitor reason %{public}d", reason);
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
        default:
            POWER_HILOGE(FEATURE_INPUT, "CreateMonitor : Invalid reason %{public}d", reason);
            break;
    }
    return monitor;
}

bool SuspendMonitor::IsSupportSensor(SensorTypeId typeId)
{
    bool isSupport = false;
    SensorInfo* sensorInfo = nullptr;
    int32_t count;
    int32_t ret = GetAllSensors(&sensorInfo, &count);
    if (ret != 0 || sensorInfo == nullptr) {
        POWER_HILOGW(FEATURE_INPUT, "Get sensors fail, ret=%{public}d", ret);
        return isSupport;
    }
    for (int32_t i = 0; i < count; i++) {
        if (sensorInfo[i].sensorTypeId == typeId) {
            isSupport = true;
            break;
        }
    }
    return isSupport;
}

/** PowerKeySuspendMonitor Implement */
bool PowerKeySuspendMonitor::Init()
{
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
            POWER_HILOGI(FEATURE_INPUT, "Receive key on notify");
            auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
            if (pms == nullptr) {
                return;
            }
            std::shared_ptr<SuspendController> suspendController = pms->GetSuspendController();
            if (suspendController->GetPowerkeyDownWhenScreenOff()) {
                POWER_HILOGI(FEATURE_INPUT, "no action suspend");
                return;
            }
            Notify();
        });
    POWER_HILOGI(FEATURE_INPUT, "powerkeyReleaseId_ %{public}d", powerkeyReleaseId_);
    return powerkeyReleaseId_ >= 0 ? true : false;
}

void PowerKeySuspendMonitor::Cancel()
{
    if (powerkeyReleaseId_ >= 0) {
        InputManager::GetInstance()->UnsubscribeKeyEvent(powerkeyReleaseId_);
        powerkeyReleaseId_ = -1;
    }
}

/** Timeout Implement */
bool TimeoutSuspendMonitor::Init()
{
    return true;
}

void TimeoutSuspendMonitor::Cancel() {}

void TimeoutSuspendMonitor::HandleEvent(uint32_t eventId)
{
    POWER_HILOGI(FEATURE_INPUT, "TimeoutSuspendMonitor HandleEvent: %{public}d", eventId);
    switch (eventId) {
        case SuspendEventHandler::SCREEN_OFF_TIMEOUT_MSG: {
            Notify();
            break;
        }
        default:
            break;
    }
}

/** LidSuspendMonitor Implement */
std::weak_ptr<LidSuspendMonitor> LidSuspendMonitor::self_;

bool LidSuspendMonitor::Init()
{
    if (!IsSupportSensor(SENSOR_TYPE_ID_HALL)) {
        POWER_HILOGE(FEATURE_INPUT, "SENSOR_TYPE_ID_HALL sensor not support");
        return false;
    }
    if (strcpy_s(sensorUser_.name, sizeof(sensorUser_.name), "PowerManager") != EOK) {
        POWER_HILOGE(FEATURE_INPUT, "strcpy_s error");
        return false;
    }
    self_ = shared_from_this();
    sensorUser_.userData = nullptr;
    sensorUser_.callback = &HallSensorCallback;
    SubscribeSensor(SENSOR_TYPE_ID_HALL, &sensorUser_);
    SetBatch(SENSOR_TYPE_ID_HALL, &sensorUser_, HALL_SAMPLING_RATE, HALL_REPORT_INTERVAL);
    ActivateSensor(SENSOR_TYPE_ID_HALL, &sensorUser_);
    return true;
}

void LidSuspendMonitor::Cancel()
{
    if (IsSupportSensor(SENSOR_TYPE_ID_HALL)) {
        DeactivateSensor(SENSOR_TYPE_ID_HALL, &sensorUser_);
        UnsubscribeSensor(SENSOR_TYPE_ID_HALL, &sensorUser_);
    }
}

void LidSuspendMonitor::HallSensorCallback(SensorEvent* event)
{
    if (event == nullptr || event->sensorTypeId != SENSOR_TYPE_ID_HALL || event->data == nullptr) {
        POWER_HILOGW(FEATURE_INPUT, "Hall sensor event is invalid");
        return;
    }
    const uint32_t LID_CLOSED_HALL_FLAG = 0x1;
    auto data = reinterpret_cast<HallData*>(event->data);
    auto status = static_cast<uint32_t>(data->status);
    if (status & LID_CLOSED_HALL_FLAG) {
        POWER_HILOGI(FEATURE_SUSPEND, "Lid close event received, begin to suspend");
        std::shared_ptr<LidSuspendMonitor> self = self_.lock();
        if (self != nullptr) {
            self->Notify();
        }
    }
}

} // namespace PowerMgr
} // namespace OHOS
