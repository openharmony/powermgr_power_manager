/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "power_mgr_service.h"

#include <datetime_ex.h>
#include <file_ex.h>
#include <hisysevent.h>
#include <if_system_ability_manager.h>
#include <input_manager.h>
#include <ipc_skeleton.h>
#include <iservice_registry.h>
#include <securec.h>
#include <string_ex.h>
#include <system_ability_definition.h>
#include <unistd.h>

#include "ability_manager_client.h"
#include "permission.h"
#include "power_common.h"
#include "power_mgr_dumper.h"
#include "system_suspend_controller.h"
#include "ui_service_mgr_client.h"
#include "sysparam.h"
#include "watchdog.h"

using namespace OHOS::AppExecFwk;
using namespace OHOS::AAFwk;

namespace OHOS {
namespace PowerMgr {
namespace {
const std::string POWERMGR_SERVICE_NAME = "PowerMgrService";
const std::string TASK_RUNNINGLOCK_UNLOCK = "RunningLock_UnLock";
const std::string REASON_POWER_KEY = "power_key";
auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
const bool G_REGISTER_RESULT = SystemAbility::MakeAndRegisterAbility(pms.GetRefPtr());
SysParam::BootCompletedCallback g_bootCompletedCallback;
}

using namespace MMI;

PowerMgrService::PowerMgrService() : SystemAbility(POWER_MANAGER_SERVICE_ID, true) {}

PowerMgrService::~PowerMgrService() {}

void PowerMgrService::OnStart()
{
    POWER_HILOGD(COMP_SVC, "Power Management startup");
    if (ready_) {
        POWER_HILOGW(COMP_SVC, "OnStart is ready, nothing to do");
        return;
    }

    if (!Init()) {
        POWER_HILOGE(COMP_SVC, "Call init fail");
        return;
    }
    SystemSuspendController::GetInstance().RegisterHdiStatusListener(handler_);
    if (!Publish(DelayedSpSingleton<PowerMgrService>::GetInstance())) {
        POWER_HILOGE(COMP_SVC, "Register to system ability manager failed");
        return;
    }
    ready_ = true;
    POWER_HILOGI(COMP_SVC, "Add system ability success");
}

bool PowerMgrService::Init()
{
    POWER_HILOGI(COMP_SVC, "Init start");

    if (!eventRunner_) {
        eventRunner_ = AppExecFwk::EventRunner::Create(POWERMGR_SERVICE_NAME);
        if (eventRunner_ == nullptr) {
            POWER_HILOGE(COMP_SVC, "Init failed due to create EventRunner");
            return false;
        }
    }

    if (!handler_) {
        handler_ = std::make_shared<PowermsEventHandler>(eventRunner_, pms);
        std::string handlerName("PowerMgrEventHandler");
        HiviewDFX::Watchdog::GetInstance().AddThread(handlerName, handler_);
    }

    if (!runningLockMgr_) {
        runningLockMgr_ = std::make_shared<RunningLockMgr>(pms);
    }
    if (!runningLockMgr_->Init()) {
        POWER_HILOGE(COMP_SVC, "Running lock init fail");
        return false;
    }
    if (!PowerStateMachineInit()) {
        POWER_HILOGE(COMP_SVC, "Power state machine init fail");
    }

    RegisterBootCompletedCallback();
    POWER_HILOGI(COMP_SVC, "Init success");
    return true;
}
void PowerMgrService::RegisterBootCompletedCallback()
{
    g_bootCompletedCallback = []() {
        POWER_HILOGI(COMP_SVC, "BootCompletedCallback triggered");
        auto power = DelayedSpSingleton<PowerMgrService>::GetInstance();
        if (DelayedSpSingleton<PowerSaveMode>::GetInstance()) {
            auto& powerModeModule = power->GetPowerModeModule();
            powerModeModule.EnableMode(powerModeModule.GetModeItem(), true);
        }
        auto powerStateMachine = power->GetPowerStateMachine();
        powerStateMachine->RegisterDisplayOffTimeObserver();
        powerStateMachine->InitState();
        auto powerHandler = power->GetHandler();
        powerHandler->SendEvent(PowermsEventHandler::INIT_KEY_MONITOR_MSG, 0, INIT_KEY_MONITOR_DELAY_MS);
    };
    SysParam::RegisterBootCompletedCallback(g_bootCompletedCallback);
}

bool PowerMgrService::PowerStateMachineInit()
{
    if (powerStateMachine_ == nullptr) {
        powerStateMachine_ = std::make_shared<PowerStateMachine>(pms);
        if (!(powerStateMachine_->Init())) {
            POWER_HILOGE(COMP_SVC, "Power state machine start fail!");
            return false;
        }
    }
    if (powerMgrNotify_ == nullptr) {
        powerMgrNotify_ = std::make_shared<PowerMgrNotify>();
        powerMgrNotify_->RegisterPublishEvents();
    }
    return true;
}

class InputCallback : public IInputEventConsumer {
    virtual void OnInputEvent(std::shared_ptr<KeyEvent> keyEvent) const;
    virtual void OnInputEvent(std::shared_ptr<PointerEvent> pointerEvent) const;
    virtual void OnInputEvent(std::shared_ptr<AxisEvent> axisEvent) const;
};

void InputCallback::OnInputEvent(std::shared_ptr<KeyEvent> keyEvent) const
{
    POWER_HILOGD(FEATURE_INPUT, "KeyEvent");
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        return;
    }
    pms->HandleKeyEvent(keyEvent->GetKeyCode());
}

void InputCallback::OnInputEvent(std::shared_ptr<PointerEvent> pointerEvent) const
{
    POWER_HILOGD(FEATURE_INPUT, "PointerEvent");
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        return;
    }
    int32_t type = pointerEvent->GetSourceType();
    pms->HandlePointEvent(type);
}

void InputCallback::OnInputEvent(std::shared_ptr<AxisEvent> axisEvent) const
{
    POWER_HILOGD(FEATURE_INPUT, "AxisEvent");
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        return;
    }
}

void PowerMgrService::KeyMonitorInit()
{
    POWER_HILOGD(FEATURE_INPUT, "Initialize the subscription key");
    std::shared_ptr<OHOS::MMI::KeyOption> keyOption = std::make_shared<OHOS::MMI::KeyOption>();
    std::set<int32_t> preKeys;

    keyOption->SetPreKeys(preKeys);
    keyOption->SetFinalKey(OHOS::MMI::KeyEvent::KEYCODE_POWER);
    keyOption->SetFinalKeyDown(true);
    keyOption->SetFinalKeyDownDuration(LONG_PRESS_DELAY_MS);
    powerkeyLongPressId_ = InputManager::GetInstance()->SubscribeKeyEvent(keyOption,
        [this](std::shared_ptr<OHOS::MMI::KeyEvent> keyEvent) {
            POWER_HILOGI(FEATURE_INPUT, "Receive long press powerkey");
            handler_->SendEvent(PowermsEventHandler::SHUTDOWN_REQUEST_MSG);
    });
    if (powerkeyLongPressId_ >= 0) {
        keyOption.reset();
        keyOption = std::make_shared<OHOS::MMI::KeyOption>();
        keyOption->SetPreKeys(preKeys);
        keyOption->SetFinalKey(OHOS::MMI::KeyEvent::KEYCODE_POWER);
        keyOption->SetFinalKeyDown(true);
        keyOption->SetFinalKeyDownDuration(0);
        powerkeyShortPressId_ = InputManager::GetInstance()->SubscribeKeyEvent(keyOption,
            [this](std::shared_ptr<OHOS::MMI::KeyEvent> keyEvent) {
                POWER_HILOGI(FEATURE_INPUT, "Receive short press powerkey");
                powerkeyPressed_ = true;
                if (!IsScreenOn()) {
                    handler_->SendEvent(PowermsEventHandler::SCREEN_ON_TIMEOUT_MSG, 0, POWER_KEY_PRESS_DELAY_MS);
                }
        });

        keyOption.reset();
        keyOption = std::make_shared<OHOS::MMI::KeyOption>();
        keyOption->SetPreKeys(preKeys);
        keyOption->SetFinalKey(OHOS::MMI::KeyEvent::KEYCODE_POWER);
        keyOption->SetFinalKeyDown(false);
        keyOption->SetFinalKeyDownDuration(0);
        powerkeyReleaseId_ = InputManager::GetInstance()->SubscribeKeyEvent(keyOption,
            [this](std::shared_ptr<OHOS::MMI::KeyEvent> keyEvent) {
                powerkeyPressed_ = false;
                this->HandlePowerKeyUp();
        });

        keyOption.reset();
        keyOption = std::make_shared<OHOS::MMI::KeyOption>();
        keyOption->SetPreKeys(preKeys);
        keyOption->SetFinalKey(OHOS::MMI::KeyEvent::KEYCODE_F1);
        keyOption->SetFinalKeyDown(true);
        keyOption->SetFinalKeyDownDuration(0);
        doubleClickId_ = InputManager::GetInstance()->SubscribeKeyEvent(keyOption,
            [this](std::shared_ptr<OHOS::MMI::KeyEvent> keyEvent) {
                POWER_HILOGI(FEATURE_INPUT, "Receive double click");
                this->HandleKeyEvent(keyEvent->GetKeyCode());
        });
    } else if (powerkeyLongPressId_ != ERROR_UNSUPPORT) {
        POWER_HILOGI(FEATURE_INPUT, "SubscribeKeyEvent failed: %{public}d", powerkeyLongPressId_);
        handler_->SendEvent(PowermsEventHandler::INIT_KEY_MONITOR_MSG, 0, INIT_KEY_MONITOR_DELAY_MS);
        return;
    }

    std::shared_ptr<InputCallback> callback = std::make_shared<InputCallback>();
    monitorId_ = InputManager::GetInstance()->AddMonitor(std::static_pointer_cast<IInputEventConsumer>(callback));
}

void PowerMgrService::KeyMonitorCancel()
{
    POWER_HILOGI(FEATURE_INPUT, "Unsubscribe key information");
    InputManager* inputManager = InputManager::GetInstance();
    if (inputManager == nullptr) {
        POWER_HILOGI(FEATURE_INPUT, "InputManager is null");
        return;
    }
    if (powerkeyLongPressId_ >= 0) {
        inputManager->UnsubscribeKeyEvent(powerkeyLongPressId_);
    }
    if (powerkeyShortPressId_ >= 0) {
        inputManager->UnsubscribeKeyEvent(powerkeyShortPressId_);
    }
    if (powerkeyReleaseId_ >= 0) {
        inputManager->UnsubscribeKeyEvent(powerkeyReleaseId_);
    }
    if (doubleClickId_ >= 0) {
        inputManager->UnsubscribeKeyEvent(doubleClickId_);
    }
    if (monitorId_ >= 0) {
        inputManager->RemoveMonitor(monitorId_);
    }
}

void PowerMgrService::HallSensorSubscriberInit()
{
    if (!IsSupportSensor(SENSOR_TYPE_ID_HALL)) {
        POWER_HILOGW(FEATURE_INPUT, "SENSOR_TYPE_ID_HALL sensor not support");
        return;
    }
    if (strcpy_s(sensorUser_.name, sizeof(sensorUser_.name), "PowerManager") != EOK) {
        POWER_HILOGW(FEATURE_INPUT, "strcpy_s error");
        return;
    }
    sensorUser_.userData = nullptr;
    sensorUser_.callback = &HallSensorCallback;
    SubscribeSensor(SENSOR_TYPE_ID_HALL, &sensorUser_);
    SetBatch(SENSOR_TYPE_ID_HALL, &sensorUser_, HALL_SAMPLING_RATE, HALL_REPORT_INTERVAL);
    ActivateSensor(SENSOR_TYPE_ID_HALL, &sensorUser_);
}

bool PowerMgrService::IsSupportSensor(SensorTypeId typeId)
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

void PowerMgrService::HallSensorCallback(SensorEvent* event)
{
    if (event == nullptr || event->sensorTypeId != SENSOR_TYPE_ID_HALL || event->data == nullptr) {
        POWER_HILOGW(FEATURE_INPUT, "Hall sensor event is invalid");
        return;
    }
    const uint32_t LID_CLOSED_HALL_FLAG = 0x1;
    auto now = static_cast<int64_t>(time(nullptr));
    auto data = (HallData*)event->data;
    auto status = static_cast<uint32_t>(data->status);
    if (status & LID_CLOSED_HALL_FLAG) {
        POWER_HILOGI(FEATURE_SUSPEND, "Lid close event received, begin to suspend");
        pms->SuspendDevice(now, SuspendDeviceType::SUSPEND_DEVICE_REASON_LID_SWITCH, false);
    } else {
        POWER_HILOGI(FEATURE_WAKEUP, "Lid open event received, begin to wakeup");
        std::string reason = "lid open";
        pms->WakeupDevice(now, WakeupDeviceType::WAKEUP_DEVICE_LID, reason);
    }
}

void PowerMgrService::HallSensorSubscriberCancel()
{
    if (IsSupportSensor(SENSOR_TYPE_ID_HALL)) {
        DeactivateSensor(SENSOR_TYPE_ID_HALL, &sensorUser_);
        UnsubscribeSensor(SENSOR_TYPE_ID_HALL, &sensorUser_);
    }
}

bool PowerMgrService::ShowPowerDialog()
{
    POWER_HILOGD(COMP_SVC, "PowerMgrService::ShowPowerDialog start.");
    auto client = AbilityManagerClient::GetInstance();
    if (client == nullptr) {
        return false;
    }
    AAFwk::Want want;
    want.SetElementName("com.ohos.powerdialog", "PowerServiceExtAbility");
    int32_t result = client->StartAbility(want);
    if (result != 0) {
        POWER_HILOGE(COMP_SVC, "ShowPowerDialog failed, result = %{public}d", result);
        return false;
    }
    isDialogstatus_ = true;
    POWER_HILOGD(COMP_SVC, "ShowPowerDialog success.");
    return true;
}

void PowerMgrService::HandleShutdownRequest()
{
    POWER_HILOGD(FEATURE_SHUTDOWN, "HandleShutdown");
    bool showSuccess = ShowPowerDialog();
    if (!showSuccess) {
        return;
    }
    int64_t now = static_cast<int64_t>(time(0));
    this->RefreshActivity(now, UserActivityType::USER_ACTIVITY_TYPE_ATTENTION, false);
    if (!IsScreenOn()) {
        POWER_HILOGI(FEATURE_SHUTDOWN, "Wakeup when display off");
        this->WakeupDevice(now, WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON, REASON_POWER_KEY);
    }
}

void PowerMgrService::HandlePowerKeyUp()
{
    POWER_HILOGI(FEATURE_INPUT, "Receive release powerkey");

    if (isDialogstatus_ || this->shutdownService_.IsShuttingDown()) {
        POWER_HILOGW(FEATURE_INPUT, "System is shutting down");
        isDialogstatus_ = false;
        return;
    }
    int64_t now = static_cast<int64_t>(time(0));
    if (this->IsScreenOn()) {
        this->SuspendDevice(now, SuspendDeviceType::SUSPEND_DEVICE_REASON_POWER_BUTTON, false);
    } else {
        this->WakeupDevice(now, WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON, REASON_POWER_KEY);
    }
}

void PowerMgrService::HandleKeyEvent(int32_t keyCode)
{
    POWER_HILOGD(FEATURE_INPUT, "keyCode: %{public}d", keyCode);
    int64_t now = static_cast<int64_t>(time(0));
    if (IsScreenOn()) {
        this->RefreshActivity(now, UserActivityType::USER_ACTIVITY_TYPE_BUTTON, false);
    } else {
        if (keyCode == KeyEvent::KEYCODE_F1) {
            POWER_HILOGI(FEATURE_WAKEUP, "Wakeup by double click");
            std::string reason = "double click";
            reason.append(std::to_string(keyCode));
            this->WakeupDevice(now, WakeupDeviceType::WAKEUP_DEVICE_DOUBLE_CLICK, reason);
        } else if (keyCode >= KeyEvent::KEYCODE_0
            && keyCode <= KeyEvent::KEYCODE_NUMPAD_RIGHT_PAREN) {
            POWER_HILOGI(FEATURE_WAKEUP, "Wakeup by keyboard");
            std::string reason = "keyboard:";
            reason.append(std::to_string(keyCode));
            this->WakeupDevice(now, WakeupDeviceType::WAKEUP_DEVICE_KEYBOARD, reason);
        }
    }
}

void PowerMgrService::HandlePointEvent(int32_t type)
{
    POWER_HILOGD(FEATURE_INPUT, "type: %{public}d", type);
    int64_t now = static_cast<int64_t>(time(0));
    if (this->IsScreenOn()) {
        this->RefreshActivity(now, UserActivityType::USER_ACTIVITY_TYPE_ATTENTION, false);
    } else {
        if (type == PointerEvent::SOURCE_TYPE_MOUSE) {
            std::string reason = "mouse click";
            POWER_HILOGI(FEATURE_WAKEUP, "Wakeup by mouse");
            this->WakeupDevice(now, WakeupDeviceType::WAKEUP_DEVICE_MOUSE, reason);
        }
    }
}

void PowerMgrService::NotifyDisplayActionDone(uint32_t event)
{
    POWER_HILOGI(COMP_SVC, "NotifyDisplayActionDone: %{public}d", event);
    handler_->RemoveEvent(PowermsEventHandler::SCREEN_ON_TIMEOUT_MSG);
}

void PowerMgrService::HandleScreenOnTimeout()
{
    POWER_HILOGD(FEATURE_INPUT, "PowerKey press timeout");
    std::string message = "POWER KEY TIMEOUT ";
    if (powerkeyPressed_) {
        message.append("WITHOUT KEY UP");
    } else {
        message.append("BUT DISPLAY NOT FINISHED");
    }
    HiviewDFX::HiSysEvent::Write("POWER", "SCREEN_ON_TIMEOUT",
        HiviewDFX::HiSysEvent::EventType::FAULT,
        "PID", IPCSkeleton::GetCallingPid(), "UID", IPCSkeleton::GetCallingUid(),
        "PACKAGE_NAME", "", "PROCESS_NAME", "", "MSG", message.c_str());
    POWER_HILOGD(FEATURE_INPUT, "Send HiSysEvent msg end");
}

void PowerMgrService::PowerMgrService::OnStop()
{
    POWER_HILOGW(COMP_SVC, "Stop service");
    if (!ready_) {
        return;
    }
    powerStateMachine_->CancelDelayTimer(
        PowermsEventHandler::CHECK_USER_ACTIVITY_TIMEOUT_MSG);
    powerStateMachine_->CancelDelayTimer(
        PowermsEventHandler::CHECK_USER_ACTIVITY_OFF_TIMEOUT_MSG);
    powerStateMachine_->CancelDelayTimer(
        PowermsEventHandler::CHECK_USER_ACTIVITY_SLEEP_TIMEOUT_MSG);
    powerStateMachine_->UnregisterDisplayOffTimeObserver();
    SystemSuspendController::GetInstance().UnRegisterPowerHdiCallback();
    handler_->RemoveEvent(PowermsEventHandler::SCREEN_ON_TIMEOUT_MSG);

    KeyMonitorCancel();
    HallSensorSubscriberCancel();
    eventRunner_.reset();
    handler_.reset();
    ready_ = false;
}

int32_t PowerMgrService::Dump(int32_t fd, const std::vector<std::u16string>& args)
{
    POWER_HILOGI(COMP_SVC, "Dump service");
    std::vector<std::string> argsInStr;
    std::transform(args.begin(), args.end(), std::back_inserter(argsInStr),
        [](const std::u16string &arg) {
        std::string ret = Str16ToStr8(arg);
        POWER_HILOGI(COMP_SVC, "arg: %{public}s", ret.c_str());
        return ret;
    });
    std::string result;
    PowerMgrDumper::Dump(argsInStr, result);
    if (!SaveStringToFd(fd, result)) {
        POWER_HILOGE(COMP_SVC, "Dump failed, save to fd failed.");
        POWER_HILOGE(COMP_SVC, "Dump Info:\n");
        POWER_HILOGE(COMP_SVC, "%{public}s", result.c_str());
        return ERR_OK;
    }
    return ERR_OK;
}

PowerErrors PowerMgrService::RebootDevice(const std::string& reason)
{
    if (!Permission::IsSystem()) {
        return PowerErrors::ERR_PERMISSION_DENIED;
    }
    return RebootDeviceForDeprecated(reason);
}

PowerErrors PowerMgrService::RebootDeviceForDeprecated(const std::string& reason)
{
    std::lock_guard lock(mutex_);
    pid_t pid = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    if (!Permission::IsPermissionGranted("ohos.permission.REBOOT")) {
        return PowerErrors::ERR_PERMISSION_DENIED;
    }
    POWER_HILOGI(FEATURE_SHUTDOWN, "Cancel auto sleep timer");
    powerStateMachine_->CancelDelayTimer(
        PowermsEventHandler::CHECK_USER_ACTIVITY_TIMEOUT_MSG);
    powerStateMachine_->CancelDelayTimer(
        PowermsEventHandler::CHECK_USER_ACTIVITY_OFF_TIMEOUT_MSG);
    powerStateMachine_->CancelDelayTimer(
        PowermsEventHandler::CHECK_USER_ACTIVITY_SLEEP_TIMEOUT_MSG);

    POWER_HILOGI(FEATURE_SHUTDOWN, "Do reboot, called pid: %{public}d, uid: %{public}d", pid, uid);
    shutdownService_.Reboot(reason);
    return PowerErrors::ERR_OK;
}

PowerErrors PowerMgrService::ShutDownDevice(const std::string& reason)
{
    std::lock_guard lock(mutex_);
    pid_t pid  = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    if (!Permission::IsSystemApl() && !Permission::IsSystemHapPermGranted("ohos.permission.REBOOT")) {
        return PowerErrors::ERR_PERMISSION_DENIED;
    }
    POWER_HILOGI(FEATURE_SHUTDOWN, "Cancel auto sleep timer");
    powerStateMachine_->CancelDelayTimer(
        PowermsEventHandler::CHECK_USER_ACTIVITY_TIMEOUT_MSG);
    powerStateMachine_->CancelDelayTimer(
        PowermsEventHandler::CHECK_USER_ACTIVITY_OFF_TIMEOUT_MSG);
    powerStateMachine_->CancelDelayTimer(
        PowermsEventHandler::CHECK_USER_ACTIVITY_SLEEP_TIMEOUT_MSG);

    POWER_HILOGI(FEATURE_SHUTDOWN, "Do shutdown, called pid: %{public}d, uid: %{public}d", pid, uid);
    shutdownService_.Shutdown(reason);
    return PowerErrors::ERR_OK;
}

PowerErrors PowerMgrService::SuspendDevice(int64_t callTimeMs,
    SuspendDeviceType reason,
    bool suspendImmed)
{
    std::lock_guard lock(mutex_);
    pid_t pid  = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    if (!Permission::IsSystem()) {
        return PowerErrors::ERR_PERMISSION_DENIED;
    }
    if (shutdownService_.IsShuttingDown()) {
        POWER_HILOGW(FEATURE_SUSPEND, "System is shutting down, can't suspend");
        return PowerErrors::ERR_OK;
    }
    POWER_HILOGI(FEATURE_SUSPEND, "Try to suspend device, pid: %{public}d, uid: %{public}d", pid, uid);
    powerStateMachine_->SuspendDeviceInner(pid, callTimeMs, reason, suspendImmed);
    return PowerErrors::ERR_OK;
}

PowerErrors PowerMgrService::WakeupDevice(int64_t callTimeMs,
    WakeupDeviceType reason,
    const std::string& details)
{
    std::lock_guard lock(mutex_);
    pid_t pid  = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    if (!Permission::IsSystem()) {
        return PowerErrors::ERR_PERMISSION_DENIED;
    }
    POWER_HILOGI(FEATURE_WAKEUP, "Try to wakeup device, pid: %{public}d, uid: %{public}d", pid, uid);
    powerStateMachine_->WakeupDeviceInner(pid, callTimeMs, reason, details, "OHOS");
    return PowerErrors::ERR_OK;
}

bool PowerMgrService::RefreshActivity(int64_t callTimeMs,
    UserActivityType type,
    bool needChangeBacklight)
{
    std::lock_guard lock(mutex_);
    if (powerStateMachine_->CheckRefreshTime()) {
        return false;
    }
    pid_t pid  = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    if (!Permission::IsSystem() && !Permission::IsPermissionGranted("ohos.permission.REFRESH_USER_ACTION")) {
        return false;
    }
    POWER_HILOGD(FEATURE_ACTIVITY, "Try to refresh activity, pid: %{public}d, uid: %{public}d", pid, uid);
    powerStateMachine_->RefreshActivityInner(pid, callTimeMs, type, needChangeBacklight);
    return true;
}

bool PowerMgrService::OverrideScreenOffTime(int64_t timeout)
{
    std::lock_guard lock(mutex_);
    if (!Permission::IsSystem()) {
        return false;
    }
    POWER_HILOGD(COMP_SVC, "Try to override screen off time");
    return powerStateMachine_->OverrideScreenOffTimeInner(timeout);
}

bool PowerMgrService::RestoreScreenOffTime()
{
    std::lock_guard lock(mutex_);
    if (!Permission::IsSystem()) {
        return false;
    }
    POWER_HILOGD(COMP_SVC, "Try to restore screen off time");
    return powerStateMachine_->RestoreScreenOffTimeInner();
}


PowerState PowerMgrService::GetState()
{
    std::lock_guard lock(mutex_);
    auto state = powerStateMachine_->GetState();
    POWER_HILOGD(FEATURE_POWER_STATE, "state: %{public}d", state);
    return state;
}

bool PowerMgrService::IsScreenOn()
{
    std::lock_guard lock(mutex_);
    auto isScreenOn = powerStateMachine_->IsScreenOn();
    POWER_HILOGI(COMP_SVC, "isScreenOn: %{public}d", isScreenOn);
    return isScreenOn;
}

bool PowerMgrService::ForceSuspendDevice(int64_t callTimeMs)
{
    std::lock_guard lock(mutex_);
    pid_t pid  = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    if (!Permission::IsSystem()) {
        return false;
    }
    if (shutdownService_.IsShuttingDown()) {
        POWER_HILOGI(FEATURE_SUSPEND, "System is shutting down, can't force suspend");
        return false;
    }
    POWER_HILOGI(FEATURE_SUSPEND, "Try to force suspend device, pid: %{public}d, uid: %{public}d", pid, uid);
    return powerStateMachine_->ForceSuspendDeviceInner(pid, callTimeMs);
}

inline void PowerMgrService::FillUserIPCInfo(UserIPCInfo &userIPCinfo)
{
    userIPCinfo.pid = IPCSkeleton::GetCallingPid();
    userIPCinfo.uid = IPCSkeleton::GetCallingUid();
}

PowerErrors PowerMgrService::CreateRunningLock(const sptr<IRemoteObject>& remoteObj,
    const RunningLockInfo& runningLockInfo)
{
    std::lock_guard lock(lockMutex_);
    if (!Permission::IsSystem() && !Permission::IsPermissionGranted("ohos.permission.RUNNING_LOCK")) {
        return PowerErrors::ERR_PERMISSION_DENIED;
    }

    POWER_HILOGI(FEATURE_RUNNING_LOCK, "name: %{public}s, type: %{public}d",
        runningLockInfo.name.c_str(), runningLockInfo.type);

    UserIPCInfo userIPCInfo;
    FillUserIPCInfo(userIPCInfo);
    runningLockMgr_->CreateRunningLock(remoteObj, runningLockInfo, userIPCInfo);
    return PowerErrors::ERR_OK;
}

bool PowerMgrService::ReleaseRunningLock(const sptr<IRemoteObject>& remoteObj)
{
    std::lock_guard lock(lockMutex_);
    bool result = false;
    if (!Permission::IsSystem() && !Permission::IsPermissionGranted("ohos.permission.RUNNING_LOCK")) {
        return result;
    }

    result = runningLockMgr_->ReleaseLock(remoteObj);
    return result;
}

bool PowerMgrService::IsRunningLockTypeSupported(uint32_t type)
{
    std::lock_guard lock(lockMutex_);
    if (type >= static_cast<uint32_t>(RunningLockType::RUNNINGLOCK_BUTT)) {
        return false;
    }
    return true;
}

bool PowerMgrService::Lock(const sptr<IRemoteObject>& remoteObj,
    const RunningLockInfo& runningLockInfo,
    uint32_t timeOutMS)
{
    std::lock_guard lock(lockMutex_);
    if (!Permission::IsSystem() && !Permission::IsPermissionGranted("ohos.permission.RUNNING_LOCK")) {
        return false;
    }

    POWER_HILOGI(FEATURE_RUNNING_LOCK,
        "timeOutMS: %{public}d, name: %{public}s, type: %{public}d",
        timeOutMS,
        runningLockInfo.name.c_str(),
        runningLockInfo.type);

    UserIPCInfo userIPCInfo;
    FillUserIPCInfo(userIPCInfo);
    runningLockMgr_->Lock(remoteObj, runningLockInfo, userIPCInfo, timeOutMS);
    return true;
}

bool PowerMgrService::UnLock(const sptr<IRemoteObject>& remoteObj)
{
    std::lock_guard lock(lockMutex_);
    if (!Permission::IsSystem() && !Permission::IsPermissionGranted("ohos.permission.RUNNING_LOCK")) {
        return false;
    }
    runningLockMgr_->UnLock(remoteObj);
    return true;
}

void PowerMgrService::ForceUnLock(const sptr<IRemoteObject>& remoteObj)
{
    std::lock_guard lock(lockMutex_);
    runningLockMgr_->UnLock(remoteObj);
    runningLockMgr_->ReleaseLock(remoteObj);
}

bool PowerMgrService::IsUsed(const sptr<IRemoteObject>& remoteObj)
{
    std::lock_guard lock(lockMutex_);
    auto isUsed = runningLockMgr_->IsUsed(remoteObj);
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "RunningLock is Used: %{public}d", isUsed);
    return isUsed;
}

void PowerMgrService::NotifyRunningLockChanged(bool isUnLock)
{
    if (isUnLock) {
        // When unlock we try to suspend
        if (!runningLockMgr_->ExistValidRunningLock()
            && !powerStateMachine_->IsScreenOn()) {
            // runninglock is empty and Screen is off,
            // so we try to suspend device from Z side.
            POWER_HILOGI(FEATURE_RUNNING_LOCK, "RunningLock is empty, try to suspend");
            powerStateMachine_->SuspendDeviceInner(getpid(), GetTickCount(),
                SuspendDeviceType::SUSPEND_DEVICE_REASON_MIN, true, true);
        }
    }
}

bool PowerMgrService::SetWorkTriggerList(const sptr<IRemoteObject>& remoteObj,
    const WorkTriggerList& workTriggerList)
{
    std::lock_guard lock(lockMutex_);
    if (!Permission::IsSystem() && !Permission::IsPermissionGranted("ohos.permission.RUNNING_LOCK")) {
        return false;
    }

    runningLockMgr_->SetWorkTriggerList(remoteObj, workTriggerList);
    return true;
}

bool PowerMgrService::ProxyRunningLock(bool proxyLock, pid_t uid, pid_t pid)
{
    std::lock_guard lock(lockMutex_);
    if (!Permission::IsSystem()) {
        return false;
    }
    runningLockMgr_->ProxyRunningLock(proxyLock, uid, pid);
    return true;
}

bool PowerMgrService::RegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback)
{
    std::lock_guard lock(mutex_);
    pid_t pid  = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    if (!Permission::IsSystem() && !Permission::IsPermissionGranted("ohos.permission.POWER_MANAGER")) {
        return false;
    }
    POWER_HILOGI(FEATURE_POWER_STATE, "pid: %{public}d, uid: %{public}d", pid, uid);
    powerStateMachine_->RegisterPowerStateCallback(callback);
    return true;
}

bool PowerMgrService::UnRegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback)
{
    std::lock_guard lock(mutex_);
    pid_t pid  = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    if (!Permission::IsSystem() && !Permission::IsPermissionGranted("ohos.permission.POWER_MANAGER")) {
        return false;
    }
    POWER_HILOGI(FEATURE_POWER_STATE, "pid: %{public}d, uid: %{public}d", pid, uid);
    powerStateMachine_->UnRegisterPowerStateCallback(callback);
    return true;
}

bool PowerMgrService::RegisterShutdownCallback(IShutdownCallback::ShutdownPriority priority,
    const sptr<IShutdownCallback>& callback)
{
    std::lock_guard lock(mutex_);
    pid_t pid  = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    if (!Permission::IsSystem()) {
        return false;
    }
    POWER_HILOGI(FEATURE_SHUTDOWN, "pid: %{public}d, uid: %{public}d, priority: %{public}d",
        pid, uid, priority);
    shutdownService_.AddShutdownCallback(priority, callback);
    return true;
}

bool PowerMgrService::UnRegisterShutdownCallback(const sptr<IShutdownCallback>& callback)
{
    std::lock_guard lock(mutex_);
    pid_t pid  = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    if (!Permission::IsSystem()) {
        return false;
    }
    POWER_HILOGI(FEATURE_SHUTDOWN, "pid: %{public}d, uid: %{public}d", pid, uid);
    shutdownService_.DelShutdownCallback(callback);
    return true;
}

bool PowerMgrService::RegisterPowerModeCallback(const sptr<IPowerModeCallback>& callback)
{
    std::lock_guard lock(mutex_);
    pid_t pid  = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    if (!Permission::IsSystem()) {
        return false; 
    }
    POWER_HILOGI(FEATURE_POWER_MODE, "pid: %{public}d, uid: %{public}d", pid, uid);
    powerModeModule_.AddPowerModeCallback(callback);
    return true;
}

bool PowerMgrService::UnRegisterPowerModeCallback(const sptr<IPowerModeCallback>& callback)
{
    std::lock_guard lock(mutex_);
    pid_t pid  = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    if (!Permission::IsSystem()) {
        return false;
    }
    POWER_HILOGI(FEATURE_POWER_MODE, "pid: %{public}d, uid: %{public}d", pid, uid);
    powerModeModule_.DelPowerModeCallback(callback);
    return true;
}

bool PowerMgrService::SetDisplaySuspend(bool enable)
{
    std::lock_guard lock(mutex_);
    pid_t pid  = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    if (!Permission::IsSystem()) {
        return false;
    }
    POWER_HILOGI(FEATURE_SUSPEND, "pid: %{public}d, uid: %{public}d, enable: %{public}d", pid, uid, enable);
    powerStateMachine_->SetDisplaySuspend(enable);
    return true;
}

PowerErrors PowerMgrService::SetDeviceMode(const PowerMode& mode)
{
    std::lock_guard lock(mutex_);
    pid_t pid = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    POWER_HILOGI(FEATURE_POWER_MODE, "pid: %{public}d, uid: %{public}d, mode: %{public}u", pid, uid, mode);
    if (!Permission::IsSystemApl() && !Permission::IsSystemHapPermGranted("ohos.permission.POWER_OPTIMIZATION")) {
        return PowerErrors::ERR_PERMISSION_DENIED;
    }
    powerModeModule_.SetModeItem(mode);
    return PowerErrors::ERR_OK;
}

PowerMode PowerMgrService::GetDeviceMode()
{
    std::lock_guard lock(mutex_);
    pid_t pid = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    auto mode = powerModeModule_.GetModeItem();
    POWER_HILOGI(FEATURE_POWER_MODE, "pid: %{public}d, uid: %{public}d, mode: %{public}u", pid, uid, mode);
    return mode;
}

std::string PowerMgrService::ShellDump(const std::vector<std::string>& args, uint32_t argc)
{
    if (!Permission::IsSystem()) {
        return "";
    }
    std::lock_guard lock(mutex_);
    pid_t pid = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    POWER_HILOGI(COMP_SVC, "pid: %{public}d, uid: %{public}d", pid, uid);

    std::string result;
    bool ret = PowerMgrDumper::Dump(args, result);
    POWER_HILOGI(COMP_SVC, "ret :%{public}d", ret);
    return result;
}
} // namespace PowerMgr
} // namespace OHOS
