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

#include "power_mgr_service.h"

#include <datetime_ex.h>
#include <file_ex.h>
#include <hisysevent.h>
#include <if_system_ability_manager.h>
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
#include <input_manager.h>
#endif
#include <ipc_skeleton.h>
#include <iservice_registry.h>
#include <securec.h>
#include <string_ex.h>
#include <system_ability_definition.h>
#include <sys_mgr_client.h>
#include <bundle_mgr_client.h>
#include <unistd.h>

#include "ability_connect_callback_stub.h"
#include "ability_manager_client.h"
#include "ffrt_utils.h"
#include "permission.h"
#include "power_common.h"
#include "power_mgr_dumper.h"
#include "power_vibrator.h"
#include "running_lock_timer_handler.h"
#include "sysparam.h"
#include "system_suspend_controller.h"
#include "xcollie/watchdog.h"

#include "errors.h"
#ifdef HAS_DEVICE_STANDBY_PART
#include "standby_service_client.h"
#endif

using namespace OHOS::AppExecFwk;
using namespace OHOS::AAFwk;

namespace OHOS {
namespace PowerMgr {
namespace {
const std::string POWERMGR_SERVICE_NAME = "PowerMgrService";
const std::string REASON_POWER_KEY = "power_key";
static std::string g_wakeupReason = "";
const std::string POWER_VIBRATOR_CONFIG_FILE = "etc/power_config/power_vibrator.json";
const std::string VENDOR_POWER_VIBRATOR_CONFIG_FILE = "/vendor/etc/power_config/power_vibrator.json";
const std::string SYSTEM_POWER_VIBRATOR_CONFIG_FILE = "/system/etc/power_config/power_vibrator.json";
auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
const bool G_REGISTER_RESULT = SystemAbility::MakeAndRegisterAbility(pms.GetRefPtr());
SysParam::BootCompletedCallback g_bootCompletedCallback;
bool g_inLidMode = false;
} // namespace

std::atomic_bool PowerMgrService::isBootCompleted_ = false;
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
    AddSystemAbilityListener(DEVICE_STANDBY_SERVICE_SYSTEM_ABILITY_ID);
    AddSystemAbilityListener(DISPLAY_MANAGER_SERVICE_ID);
    SystemSuspendController::GetInstance().RegisterHdiStatusListener();
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
    if (!runningLockMgr_) {
        runningLockMgr_ = std::make_shared<RunningLockMgr>(pms);
    }
    if (!runningLockMgr_->Init()) {
        POWER_HILOGE(COMP_SVC, "Running lock init fail");
        return false;
    }
    if (!shutdownController_) {
        shutdownController_ = std::make_shared<ShutdownController>();
    }
    if (!PowerStateMachineInit()) {
        POWER_HILOGE(COMP_SVC, "Power state machine init fail");
    }
    if (!screenOffPreController_) {
        screenOffPreController_ = std::make_shared<ScreenOffPreController>(powerStateMachine_);
        screenOffPreController_->Init();
    }

    POWER_HILOGI(COMP_SVC, "Init success");
    return true;
}

void PowerMgrService::RegisterBootCompletedCallback()
{
    g_bootCompletedCallback = []() {
        POWER_HILOGI(COMP_SVC, "BootCompletedCallback triggered");
        auto power = DelayedSpSingleton<PowerMgrService>::GetInstance();
        if (power == nullptr) {
            POWER_HILOGI(COMP_SVC, "get PowerMgrService fail");
            return;
        }
        if (DelayedSpSingleton<PowerSaveMode>::GetInstance()) {
            auto& powerModeModule = power->GetPowerModeModule();
            powerModeModule.EnableMode(powerModeModule.GetModeItem(), true);
        }
        auto powerStateMachine = power->GetPowerStateMachine();
        powerStateMachine->RegisterDisplayOffTimeObserver();
        powerStateMachine->InitState();
#ifdef POWER_MANAGER_POWER_DIALOG
        power->GetShutdownDialog().LoadDialogConfig();
        power->GetShutdownDialog().KeyMonitorInit();
#endif
#ifndef CONFIG_FACTORY_MODE
        power->HallSensorSubscriberInit();
        POWER_HILOGI(COMP_SVC, "Subscribe Hall sensor");
#else
        POWER_HILOGI(COMP_SVC, "Disabled Hall sensor");
#endif
        power->SwitchSubscriberInit();
        power->InputMonitorInit();
        power->SuspendControllerInit();
        power->WakeupControllerInit();
        power->HibernateControllerInit();
#ifdef POWER_MANAGER_WAKEUP_ACTION
        power->WakeupActionControllerInit();
#endif
        power->VibratorInit();
        isBootCompleted_ = true;
    };
    WakeupRunningLock::Create();
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

void PowerMgrService::KeyMonitorCancel()
{
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
    POWER_HILOGI(FEATURE_INPUT, "Unsubscribe key information");
    InputManager* inputManager = InputManager::GetInstance();
    if (inputManager == nullptr) {
        POWER_HILOGI(FEATURE_INPUT, "InputManager is null");
        return;
    }
    shutdownDialog_.KeyMonitorCancel();
    if (doubleClickId_ >= 0) {
        inputManager->UnsubscribeKeyEvent(doubleClickId_);
    }
    if (monitorId_ >= 0) {
        inputManager->RemoveMonitor(monitorId_);
    }
#endif
}

void PowerMgrService::HallSensorSubscriberInit()
{
#ifdef HAS_SENSORS_SENSOR_PART
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
#endif
}

#ifdef HAS_SENSORS_SENSOR_PART
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

    std::shared_ptr<SuspendController> suspendController = pms->GetSuspendController();
    if (suspendController == nullptr) {
        POWER_HILOGE(FEATURE_INPUT, "get suspendController instance error");
        return;
    }

    std::shared_ptr<WakeupController> wakeupController = pms->GetWakeupController();
    if (wakeupController == nullptr) {
        POWER_HILOGE(FEATURE_INPUT, "wakeupController is not init");
        return;
    }
    const uint32_t LID_CLOSED_HALL_FLAG = 0x1;
    auto data = reinterpret_cast<HallData*>(event->data);
    auto status = static_cast<uint32_t>(data->status);

    if (status & LID_CLOSED_HALL_FLAG) {
        POWER_HILOGI(FEATURE_SUSPEND, "[UL_POWER] Lid close event received, begin to suspend");
        g_inLidMode = true;
        SuspendDeviceType reason = SuspendDeviceType::SUSPEND_DEVICE_REASON_LID;
        suspendController->ExecSuspendMonitorByReason(reason);
    } else {
        if (!g_inLidMode) {
            return;
        }
        POWER_HILOGI(FEATURE_WAKEUP, "[UL_POWER] Lid open event received, begin to wakeup");
        g_inLidMode = false;
        WakeupDeviceType reason = WakeupDeviceType::WAKEUP_DEVICE_LID;
        wakeupController->ExecWakeupMonitorByReason(reason);
    }
}
#endif

void PowerMgrService::HallSensorSubscriberCancel()
{
#ifdef HAS_SENSORS_SENSOR_PART
    if (IsSupportSensor(SENSOR_TYPE_ID_HALL)) {
        DeactivateSensor(SENSOR_TYPE_ID_HALL, &sensorUser_);
        UnsubscribeSensor(SENSOR_TYPE_ID_HALL, &sensorUser_);
    }
#endif
}

bool PowerMgrService::CheckDialogFlag()
{
    bool isLongPress = shutdownDialog_.IsLongPress();
    if (isLongPress) {
        shutdownDialog_.ResetLongPressFlag();
    }
    return true;
}

bool PowerMgrService::CheckDialogAndShuttingDown()
{
    bool isShuttingDown = shutdownController_->IsShuttingDown();
    bool isLongPress = shutdownDialog_.IsLongPress();
    if (isLongPress || isShuttingDown) {
        POWER_HILOGW(
            FEATURE_INPUT, "isLongPress: %{public}d, isShuttingDown: %{public}d", isLongPress, isShuttingDown);
        shutdownDialog_.ResetLongPressFlag();
        return true;
    }
    return false;
}

void PowerMgrService::SwitchSubscriberInit()
{
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
    POWER_HILOGW(FEATURE_INPUT, "Initialize the subscription switch");
    switchId_ =
        InputManager::GetInstance()->SubscribeSwitchEvent([this](std::shared_ptr<OHOS::MMI::SwitchEvent> switchEvent) {
            POWER_HILOGI(FEATURE_WAKEUP, "switch event received");
            std::shared_ptr<SuspendController> suspendController = pms->GetSuspendController();
            if (suspendController == nullptr) {
                POWER_HILOGE(FEATURE_INPUT, "get suspendController instance error");
                return;
            }
            std::shared_ptr<WakeupController> wakeupController = pms->GetWakeupController();
            if (wakeupController == nullptr) {
                POWER_HILOGE(FEATURE_INPUT, "wakeupController is not init");
                return;
            }
            if (switchEvent->GetSwitchValue() == SwitchEvent::SWITCH_OFF) {
                POWER_HILOGI(FEATURE_SUSPEND, "[UL_POWER] Switch close event received, begin to suspend");
                powerStateMachine_->SetSwitchState(false);
                SuspendDeviceType reason = SuspendDeviceType::SUSPEND_DEVICE_REASON_SWITCH;
                suspendController->ExecSuspendMonitorByReason(reason);
            } else {
                POWER_HILOGI(FEATURE_WAKEUP, "[UL_POWER] Switch open event received, begin to wakeup");
                powerStateMachine_->SetSwitchState(true);
                WakeupDeviceType reason = WakeupDeviceType::WAKEUP_DEVICE_SWITCH;
                wakeupController->ExecWakeupMonitorByReason(reason);
            }
        });
#endif
}

void PowerMgrService::SwitchSubscriberCancel()
{
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
    POWER_HILOGI(FEATURE_INPUT, "Unsubscribe switch information");
    if (switchId_ >= 0) {
        InputManager::GetInstance()->UnsubscribeSwitchEvent(switchId_);
        switchId_ = -1;
    }
#endif
}

void PowerMgrService::InputMonitorInit()
{
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
    POWER_HILOGI(FEATURE_INPUT, "PowerMgr service input monitor init");
    std::shared_ptr<PowerMgrInputMonitor> inputMonitor = std::make_shared<PowerMgrInputMonitor>();
    if (inputMonitorId_ < 0) {
        inputMonitorId_ =
            InputManager::GetInstance()->AddMonitor(std::static_pointer_cast<IInputEventConsumer>(inputMonitor));
    }
#endif
}

void PowerMgrService::InputMonitorCancel()
{
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
    POWER_HILOGI(FEATURE_INPUT, "PowerMgr service input monitor cancel");
    InputManager* inputManager = InputManager::GetInstance();
    if (inputMonitorId_ >= 0) {
        inputManager->RemoveMonitor(inputMonitorId_);
        inputMonitorId_ = -1;
    }
#endif
}

void PowerMgrService::HandleKeyEvent(int32_t keyCode)
{
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
    POWER_HILOGD(FEATURE_INPUT, "keyCode: %{public}d", keyCode);
    int64_t now = static_cast<int64_t>(time(nullptr));
    if (IsScreenOn()) {
        this->RefreshActivity(now, UserActivityType::USER_ACTIVITY_TYPE_BUTTON, false);
    } else {
        if (keyCode == KeyEvent::KEYCODE_F1) {
            POWER_HILOGI(FEATURE_WAKEUP, "[UL_POWER] Wakeup by double click");
            std::string reason = "double click";
            reason.append(std::to_string(keyCode));
            this->WakeupDevice(now, WakeupDeviceType::WAKEUP_DEVICE_DOUBLE_CLICK, reason);
        } else if (keyCode >= KeyEvent::KEYCODE_0 && keyCode <= KeyEvent::KEYCODE_NUMPAD_RIGHT_PAREN) {
            POWER_HILOGI(FEATURE_WAKEUP, "[UL_POWER] Wakeup by keyboard");
            std::string reason = "keyboard:";
            reason.append(std::to_string(keyCode));
            this->WakeupDevice(now, WakeupDeviceType::WAKEUP_DEVICE_KEYBOARD, reason);
        }
    }
#endif
}

void PowerMgrService::HandlePointEvent(int32_t type)
{
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
    POWER_HILOGD(FEATURE_INPUT, "type: %{public}d", type);
    int64_t now = static_cast<int64_t>(time(nullptr));
    if (this->IsScreenOn()) {
        this->RefreshActivity(now, UserActivityType::USER_ACTIVITY_TYPE_ATTENTION, false);
    } else {
        if (type == PointerEvent::SOURCE_TYPE_MOUSE) {
            std::string reason = "mouse click";
            POWER_HILOGI(FEATURE_WAKEUP, "[UL_POWER] Wakeup by mouse");
            this->WakeupDevice(now, WakeupDeviceType::WAKEUP_DEVICE_MOUSE, reason);
        }
    }
#endif
}

void PowerMgrService::OnStop()
{
    POWER_HILOGW(COMP_SVC, "Stop service");
    if (!ready_) {
        return;
    }
    powerStateMachine_->CancelDelayTimer(PowerStateMachine::CHECK_USER_ACTIVITY_TIMEOUT_MSG);
    powerStateMachine_->CancelDelayTimer(PowerStateMachine::CHECK_USER_ACTIVITY_OFF_TIMEOUT_MSG);
    powerStateMachine_->UnregisterDisplayOffTimeObserver();
    if (suspendController_) {
        suspendController_->StopSleep();
    }

    SystemSuspendController::GetInstance().UnRegisterPowerHdiCallback();

    KeyMonitorCancel();
    HallSensorSubscriberCancel();
    SwitchSubscriberCancel();
    InputMonitorCancel();
    ready_ = false;
    isBootCompleted_ = false;
    RemoveSystemAbilityListener(DEVICE_STANDBY_SERVICE_SYSTEM_ABILITY_ID);
    RemoveSystemAbilityListener(DISPLAY_MANAGER_SERVICE_ID);
}

void PowerMgrService::Reset()
{
    POWER_HILOGW(COMP_SVC, "start destruct ffrt_queue");
    if (powerStateMachine_) {
        powerStateMachine_->Reset();
    }
    if (suspendController_) {
        suspendController_->Reset();
    }
    if (screenOffPreController_) {
        screenOffPreController_->Reset();
    }
}

void PowerMgrService::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    POWER_HILOGI(COMP_SVC, "systemAbilityId=%{public}d, deviceId=%{private}s", systemAbilityId, deviceId.c_str());
    if (systemAbilityId == DEVICE_STANDBY_SERVICE_SYSTEM_ABILITY_ID) {
        std::lock_guard lock(lockMutex_);
        runningLockMgr_->ResetRunningLocks();
    }
}

void PowerMgrService::OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    POWER_HILOGI(COMP_SVC, "systemAbilityId=%{public}d, deviceId=%{private}s Add",
        systemAbilityId, deviceId.c_str());
    if (systemAbilityId == DISPLAY_MANAGER_SERVICE_ID) {
        RegisterBootCompletedCallback();
    }
}

int32_t PowerMgrService::Dump(int32_t fd, const std::vector<std::u16string>& args)
{
    if (!isBootCompleted_) {
        return ERR_NO_INIT;
    }
    if (!Permission::IsSystem()) {
        return ERR_PERMISSION_DENIED;
    }
    std::vector<std::string> argsInStr;
    std::transform(args.begin(), args.end(), std::back_inserter(argsInStr), [](const std::u16string& arg) {
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
        return PowerErrors::ERR_SYSTEM_API_DENIED;
    }
    return RebootDeviceForDeprecated(reason);
}

PowerErrors PowerMgrService::RebootDeviceForDeprecated(const std::string& reason)
{
    std::lock_guard lock(shutdownMutex_);
    pid_t pid = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    if (!Permission::IsPermissionGranted("ohos.permission.REBOOT")) {
        return PowerErrors::ERR_PERMISSION_DENIED;
    }
    POWER_HILOGI(FEATURE_SHUTDOWN, "Cancel auto sleep timer");
    powerStateMachine_->CancelDelayTimer(PowerStateMachine::CHECK_USER_ACTIVITY_TIMEOUT_MSG);
    powerStateMachine_->CancelDelayTimer(PowerStateMachine::CHECK_USER_ACTIVITY_OFF_TIMEOUT_MSG);
    if (suspendController_) {
        suspendController_->StopSleep();
    }
    POWER_HILOGI(FEATURE_SHUTDOWN, "Do reboot, called pid: %{public}d, uid: %{public}d", pid, uid);
    shutdownController_->Reboot(reason);
    return PowerErrors::ERR_OK;
}

PowerErrors PowerMgrService::ShutDownDevice(const std::string& reason)
{
    auto now = static_cast<int64_t>(time(nullptr));
    if (!Permission::IsSystem()) {
        return PowerErrors::ERR_SYSTEM_API_DENIED;
    }
    if (!Permission::IsPermissionGranted("ohos.permission.REBOOT")) {
        return PowerErrors::ERR_PERMISSION_DENIED;
    }
    if (reason == SHUTDOWN_FAST_REASON) {
        g_wakeupReason = reason;
        POWER_HILOGD(FEATURE_SHUTDOWN, "Calling Fast ShutDownDevice success");
        return SuspendDevice(now, SuspendDeviceType::SUSPEND_DEVICE_REASON_STR, true);
    }
    g_wakeupReason = "";
    std::lock_guard lock(shutdownMutex_);
    pid_t pid = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    POWER_HILOGI(FEATURE_SHUTDOWN, "Cancel auto sleep timer");
    powerStateMachine_->CancelDelayTimer(PowerStateMachine::CHECK_USER_ACTIVITY_TIMEOUT_MSG);
    powerStateMachine_->CancelDelayTimer(PowerStateMachine::CHECK_USER_ACTIVITY_OFF_TIMEOUT_MSG);
    if (suspendController_) {
        suspendController_->StopSleep();
    }

    POWER_HILOGI(FEATURE_SHUTDOWN, "[UL_POWER] Do shutdown, called pid: %{public}d, uid: %{public}d", pid, uid);
    shutdownController_->Shutdown(reason);
    return PowerErrors::ERR_OK;
}

PowerErrors PowerMgrService::SuspendDevice(int64_t callTimeMs, SuspendDeviceType reason, bool suspendImmed)
{
    std::lock_guard lock(suspendMutex_);
    pid_t pid = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    if (!Permission::IsSystem()) {
        return PowerErrors::ERR_SYSTEM_API_DENIED;
    }
    if (shutdownController_->IsShuttingDown()) {
        POWER_HILOGW(FEATURE_SUSPEND, "System is shutting down, can't suspend");
        return PowerErrors::ERR_OK;
    }
    POWER_HILOGI(FEATURE_SUSPEND, "[UL_POWER] Try to suspend device, pid: %{public}d, uid: %{public}d", pid, uid);
    powerStateMachine_->SuspendDeviceInner(pid, callTimeMs, reason, suspendImmed);
    return PowerErrors::ERR_OK;
}

PowerErrors PowerMgrService::WakeupDevice(int64_t callTimeMs, WakeupDeviceType reason, const std::string& details)
{
    std::lock_guard lock(wakeupMutex_);
    if (!Permission::IsSystem()) {
        return PowerErrors::ERR_SYSTEM_API_DENIED;
    }
    pid_t pid = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    POWER_HILOGI(FEATURE_WAKEUP, "[UL_POWER] Try to wakeup device, pid: %{public}d, uid: %{public}d", pid, uid);
    WakeupRunningLock::Lock();
    powerStateMachine_->WakeupDeviceInner(pid, callTimeMs, reason, details, "OHOS");
    WakeupRunningLock::Unlock();
    return PowerErrors::ERR_OK;
}

bool PowerMgrService::RefreshActivity(int64_t callTimeMs, UserActivityType type, bool needChangeBacklight)
{
    std::lock_guard lock(screenMutex_);
    if (powerStateMachine_->CheckRefreshTime()) {
        return false;
    }
    if (!Permission::IsPermissionGranted("ohos.permission.REFRESH_USER_ACTION") || !Permission::IsSystem()) {
        return false;
    }
    pid_t pid = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    POWER_HILOGD(FEATURE_ACTIVITY, "Try to refresh activity, pid: %{public}d, uid: %{public}d", pid, uid);
    powerStateMachine_->RefreshActivityInner(pid, callTimeMs, type, needChangeBacklight);
    return true;
}

bool PowerMgrService::OverrideScreenOffTime(int64_t timeout)
{
    std::lock_guard lock(screenMutex_);
    if (!Permission::IsSystem()) {
        return false;
    }
    POWER_HILOGD(COMP_SVC, "Try to override screen off time");
    return powerStateMachine_->OverrideScreenOffTimeInner(timeout);
}

bool PowerMgrService::RestoreScreenOffTime()
{
    std::lock_guard lock(screenMutex_);
    if (!Permission::IsSystem()) {
        return false;
    }
    POWER_HILOGD(COMP_SVC, "Try to restore screen off time");
    return powerStateMachine_->RestoreScreenOffTimeInner();
}

PowerState PowerMgrService::GetState()
{
    std::lock_guard lock(stateMutex_);
    auto state = powerStateMachine_->GetState();
    POWER_HILOGD(FEATURE_POWER_STATE, "state: %{public}d", state);
    return state;
}

bool PowerMgrService::IsScreenOn()
{
    std::lock_guard lock(stateMutex_);
    auto isScreenOn = powerStateMachine_->IsScreenOn();
    POWER_HILOGI(COMP_SVC, "isScreenOn: %{public}d", isScreenOn);
    return isScreenOn;
}

bool PowerMgrService::ForceSuspendDevice(int64_t callTimeMs)
{
    std::lock_guard lock(suspendMutex_);
    pid_t pid = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    if (!Permission::IsSystem()) {
        return false;
    }
    if (shutdownController_->IsShuttingDown()) {
        POWER_HILOGI(FEATURE_SUSPEND, "System is shutting down, can't force suspend");
        return false;
    }
    POWER_HILOGI(FEATURE_SUSPEND, "[UL_POWER] Try to force suspend device, pid: %{public}d, uid: %{public}d", pid, uid);
    return powerStateMachine_->ForceSuspendDeviceInner(pid, callTimeMs);
}

bool PowerMgrService::Hibernate(bool clearMemory)
{
    std::lock_guard lock(hibernateMutex_);
    pid_t pid = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    if (!Permission::IsSystem()) {
        return false;
    }
    if (shutdownController_->IsShuttingDown()) {
        POWER_HILOGI(FEATURE_SUSPEND, "System is shutting down, can't hibernate");
        return false;
    }
    POWER_HILOGI(FEATURE_SUSPEND,
        "[UL_POWER] Try to hibernate, pid: %{public}d, uid: %{public}d, clearMemory: %{public}d",
        pid, uid, static_cast<int>(clearMemory));
    return powerStateMachine_->HibernateInner(clearMemory);
}

std::string PowerMgrService::GetBundleNameByUid(const int32_t uid)
{
    std::string tempBundleName = "";
    if (uid < OHOS::AppExecFwk::Constants::BASE_APP_UID) {
        return tempBundleName;
    }
    BundleMgrClient bundleObj;

    std::string identity = IPCSkeleton::ResetCallingIdentity();
    ErrCode res = bundleObj.GetNameForUid(uid, tempBundleName);
    IPCSkeleton::SetCallingIdentity(identity);
    if (res != ERR_OK) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Failed to get bundle name for uid=%{public}d, ErrCode=%{public}d",
            uid, static_cast<int32_t>(res));
    }
    POWER_HILOGI(FEATURE_RUNNING_LOCK, "bundle name for uid=%{public}d, name=%{public}s", uid, tempBundleName.c_str());
    return tempBundleName;
}

RunningLockParam PowerMgrService::FillRunningLockParam(const RunningLockInfo& info,
    const uint64_t lockid, int32_t timeOutMS)
{
    RunningLockParam filledParam {};
    filledParam.lockid = lockid;
    filledParam.name = info.name;
    filledParam.type = info.type;
    if (filledParam.type == RunningLockType::RUNNINGLOCK_BACKGROUND) {
        filledParam.type = RunningLockType::RUNNINGLOCK_BACKGROUND_TASK;
    }
    filledParam.timeoutMs = timeOutMS;
    filledParam.pid = IPCSkeleton::GetCallingPid();
    filledParam.uid = IPCSkeleton::GetCallingUid();
    filledParam.bundleName = GetBundleNameByUid(filledParam.uid);
    return filledParam;
}

PowerErrors PowerMgrService::CreateRunningLock(
    const sptr<IRemoteObject>& remoteObj, const RunningLockInfo& runningLockInfo)
{
    std::lock_guard lock(lockMutex_);
    if (!Permission::IsPermissionGranted("ohos.permission.RUNNING_LOCK")) {
        return PowerErrors::ERR_PERMISSION_DENIED;
    }
    if (!IsRunningLockTypeSupported(runningLockInfo.type)) {
        POWER_HILOGW(FEATURE_RUNNING_LOCK,
            "runninglock type is not supported, name=%{public}s, type=%{public}d",
            runningLockInfo.name.c_str(), runningLockInfo.type);
        return PowerErrors::ERR_PARAM_INVALID;
    }

    uintptr_t remoteObjPtr = reinterpret_cast<uintptr_t>(remoteObj.GetRefPtr());
    uint64_t lockid = std::hash<uintptr_t>()(remoteObjPtr);
    RunningLockParam runningLockParam = FillRunningLockParam(runningLockInfo, lockid);
    runningLockMgr_->CreateRunningLock(remoteObj, runningLockParam);
    return PowerErrors::ERR_OK;
}

bool PowerMgrService::ReleaseRunningLock(const sptr<IRemoteObject>& remoteObj)
{
    std::lock_guard lock(lockMutex_);
    bool result = false;
    if (!Permission::IsPermissionGranted("ohos.permission.RUNNING_LOCK")) {
        return result;
    }

    result = runningLockMgr_->ReleaseLock(remoteObj);
    return result;
}

bool PowerMgrService::IsRunningLockTypeSupported(RunningLockType type)
{
    if (Permission::IsHap()) {
        return type == RunningLockType::RUNNINGLOCK_BACKGROUND ||
        type == RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL;
    }
    return type == RunningLockType::RUNNINGLOCK_SCREEN ||
        type == RunningLockType::RUNNINGLOCK_BACKGROUND || // this will be instead by BACKGROUND_XXX types.
        type == RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL ||
        type == RunningLockType::RUNNINGLOCK_COORDINATION ||
        type == RunningLockType::RUNNINGLOCK_BACKGROUND_PHONE ||
        type == RunningLockType::RUNNINGLOCK_BACKGROUND_NOTIFICATION ||
        type == RunningLockType::RUNNINGLOCK_BACKGROUND_AUDIO ||
        type == RunningLockType::RUNNINGLOCK_BACKGROUND_SPORT ||
        type == RunningLockType::RUNNINGLOCK_BACKGROUND_NAVIGATION ||
        type == RunningLockType::RUNNINGLOCK_BACKGROUND_TASK;
}

bool PowerMgrService::Lock(const sptr<IRemoteObject>& remoteObj, int32_t timeOutMs)
{
    if (!Permission::IsPermissionGranted("ohos.permission.RUNNING_LOCK")) {
        return false;
    }
    std::lock_guard lock(lockMutex_);
    runningLockMgr_->Lock(remoteObj);
    if (timeOutMs > 0) {
        std::function<void()> task = [this, remoteObj, timeOutMs]() {
            std::lock_guard lock(lockMutex_);
            RunningLockTimerHandler::GetInstance().UnregisterRunningLockTimer(remoteObj);
            runningLockMgr_->UnLock(remoteObj);
        };
        RunningLockTimerHandler::GetInstance().RegisterRunningLockTimer(remoteObj, task, timeOutMs);
    }
    return true;
}

bool PowerMgrService::UnLock(const sptr<IRemoteObject>& remoteObj)
{
    if (!Permission::IsPermissionGranted("ohos.permission.RUNNING_LOCK")) {
        return false;
    }
    std::lock_guard lock(lockMutex_);
    RunningLockTimerHandler::GetInstance().UnregisterRunningLockTimer(remoteObj);
    runningLockMgr_->UnLock(remoteObj);
    return true;
}

bool PowerMgrService::QueryRunningLockLists(std::map<std::string, RunningLockInfo>& runningLockLists)
{
    std::lock_guard lock(lockMutex_);
    if (!Permission::IsPermissionGranted("ohos.permission.RUNNING_LOCK")) {
        return false;
    }
    runningLockMgr_->QueryRunningLockLists(runningLockLists);
    return true;
}

bool PowerMgrService::RegisterRunningLockCallback(const sptr<IPowerRunninglockCallback>& callback)
{
    std::lock_guard lock(lockMutex_);
    pid_t pid = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    if (!Permission::IsPermissionGranted("ohos.permission.POWER_MANAGER")) {
        return false;
    }
    POWER_HILOGI(FEATURE_RUNNING_LOCK, "pid: %{public}d, uid: %{public}d", pid, uid);
    runningLockMgr_->RegisterRunningLockCallback(callback);
    return true;
}

bool PowerMgrService::UnRegisterRunningLockCallback(const sptr<IPowerRunninglockCallback>& callback)
{
    std::lock_guard lock(lockMutex_);
    pid_t pid = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    if (!Permission::IsPermissionGranted("ohos.permission.POWER_MANAGER")) {
        return false;
    }
    POWER_HILOGI(FEATURE_RUNNING_LOCK, "pid: %{public}d, uid: %{public}d", pid, uid);
    runningLockMgr_->RegisterRunningLockCallback(callback);
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

bool PowerMgrService::ProxyRunningLock(bool isProxied, pid_t pid, pid_t uid)
{
    std::lock_guard lock(lockMutex_);
    if (!Permission::IsSystem()) {
        return false;
    }
    return runningLockMgr_->ProxyRunningLock(isProxied, pid, uid);
}

bool PowerMgrService::ProxyRunningLocks(bool isProxied, const std::vector<std::pair<pid_t, pid_t>>& processInfos)
{
    if (!Permission::IsSystem()) {
        return false;
    }
    std::lock_guard lock(lockMutex_);
    runningLockMgr_->ProxyRunningLocks(isProxied, processInfos);
    return true;
}

bool PowerMgrService::ResetRunningLocks()
{
    if (!Permission::IsSystem()) {
        return false;
    }
    std::lock_guard lock(lockMutex_);
    runningLockMgr_->ResetRunningLocks();
    return true;
}

bool PowerMgrService::RegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback)
{
    std::lock_guard lock(stateMutex_);
    pid_t pid = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    if (!Permission::IsPermissionGranted("ohos.permission.POWER_MANAGER")) {
        return false;
    }
    POWER_HILOGI(FEATURE_POWER_STATE, "pid: %{public}d, uid: %{public}d", pid, uid);
    powerStateMachine_->RegisterPowerStateCallback(callback);
    return true;
}

bool PowerMgrService::UnRegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback)
{
    std::lock_guard lock(stateMutex_);
    pid_t pid = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    if (!Permission::IsPermissionGranted("ohos.permission.POWER_MANAGER")) {
        return false;
    }
    POWER_HILOGI(FEATURE_POWER_STATE, "pid: %{public}d, uid: %{public}d", pid, uid);
    powerStateMachine_->UnRegisterPowerStateCallback(callback);
    return true;
}

bool PowerMgrService::RegisterSyncSleepCallback(const sptr<ISyncSleepCallback>& callback, SleepPriority priority)
{
    std::lock_guard lock(suspendMutex_);
    pid_t pid = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    if (!Permission::IsPermissionGranted("ohos.permission.POWER_MANAGER")) {
        return false;
    }
    POWER_HILOGI(FEATURE_SUSPEND, "pid: %{public}d, uid: %{public}d", pid, uid);
    suspendController_->AddCallback(callback, priority);
    return true;
}

bool PowerMgrService::UnRegisterSyncSleepCallback(const sptr<ISyncSleepCallback>& callback)
{
    std::lock_guard lock(suspendMutex_);
    pid_t pid = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    if (!Permission::IsPermissionGranted("ohos.permission.POWER_MANAGER")) {
        return false;
    }
    POWER_HILOGI(FEATURE_SUSPEND, "pid: %{public}d, uid: %{public}d", pid, uid);
    suspendController_->RemoveCallback(callback);
    return true;
}

bool PowerMgrService::RegisterPowerModeCallback(const sptr<IPowerModeCallback>& callback)
{
    std::lock_guard lock(modeMutex_);
    pid_t pid = IPCSkeleton::GetCallingPid();
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
    std::lock_guard lock(modeMutex_);
    pid_t pid = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    if (!Permission::IsSystem()) {
        return false;
    }
    POWER_HILOGI(FEATURE_POWER_MODE, "pid: %{public}d, uid: %{public}d", pid, uid);
    powerModeModule_.DelPowerModeCallback(callback);
    return true;
}

bool PowerMgrService::RegisterScreenStateCallback(int32_t remainTime, const sptr<IScreenOffPreCallback>& callback)
{
    std::lock_guard lock(screenOffPreMutex_);
    pid_t pid = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    if (!Permission::IsSystem()) {
        return false;
    }
    POWER_HILOGI(FEATURE_SCREEN_OFF_PRE, "pid: %{public}d, uid: %{public}d", pid, uid);
    screenOffPreController_->AddScreenStateCallback(remainTime, callback);
    return true;
}

bool PowerMgrService::UnRegisterScreenStateCallback(const sptr<IScreenOffPreCallback>& callback)
{
    std::lock_guard lock(screenOffPreMutex_);
    pid_t pid = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    if (!Permission::IsSystem()) {
        return false;
    }
    POWER_HILOGI(FEATURE_SCREEN_OFF_PRE, "pid: %{public}d, uid: %{public}d", pid, uid);
    screenOffPreController_->DelScreenStateCallback(callback);
    return true;
}

bool PowerMgrService::SetDisplaySuspend(bool enable)
{
    std::lock_guard lock(screenMutex_);
    pid_t pid = IPCSkeleton::GetCallingPid();
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
    std::lock_guard lock(modeMutex_);
    pid_t pid = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    POWER_HILOGI(FEATURE_POWER_MODE, "pid: %{public}d, uid: %{public}d, mode: %{public}u", pid, uid, mode);
    if (!Permission::IsSystem()) {
        return PowerErrors::ERR_SYSTEM_API_DENIED;
    }
    if (!Permission::IsPermissionGranted("ohos.permission.POWER_OPTIMIZATION")) {
        return PowerErrors::ERR_PERMISSION_DENIED;
    }
    powerModeModule_.SetModeItem(mode);
    return PowerErrors::ERR_OK;
}

PowerMode PowerMgrService::GetDeviceMode()
{
    std::lock_guard lock(modeMutex_);
    pid_t pid = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    auto mode = powerModeModule_.GetModeItem();
    POWER_HILOGI(FEATURE_POWER_MODE, "pid: %{public}d, uid: %{public}d, mode: %{public}u", pid, uid, mode);
    return mode;
}

std::string PowerMgrService::ShellDump(const std::vector<std::string>& args, uint32_t argc)
{
    if (!Permission::IsSystem() || !isBootCompleted_) {
        return "";
    }
    std::lock_guard lock(dumpMutex_);
    pid_t pid = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    POWER_HILOGI(COMP_SVC, "pid: %{public}d, uid: %{public}d", pid, uid);

    std::string result;
    bool ret = PowerMgrDumper::Dump(args, result);
    POWER_HILOGI(COMP_SVC, "ret :%{public}d", ret);
    return result;
}

void PowerMgrService::RegisterShutdownCallback(
    const sptr<ITakeOverShutdownCallback>& callback, ShutdownPriority priority)
{
    RETURN_IF (callback == nullptr);
    RETURN_IF (!Permission::IsSystem());

    std::lock_guard lock(shutdownMutex_);
    shutdownController_->AddCallback(callback, priority);
}

void PowerMgrService::UnRegisterShutdownCallback(const sptr<ITakeOverShutdownCallback>& callback)
{
    RETURN_IF (callback == nullptr);
    RETURN_IF (!Permission::IsSystem());

    std::lock_guard lock(shutdownMutex_);
    shutdownController_->RemoveCallback(callback);
}

void PowerMgrService::RegisterShutdownCallback(
    const sptr<IAsyncShutdownCallback>& callback, ShutdownPriority priority)
{
    RETURN_IF (callback == nullptr);
    RETURN_IF (!Permission::IsSystem());

    std::lock_guard lock(shutdownMutex_);
    shutdownController_->AddCallback(callback, priority);
}

void PowerMgrService::UnRegisterShutdownCallback(const sptr<IAsyncShutdownCallback>& callback)
{
    RETURN_IF (callback == nullptr);
    RETURN_IF (!Permission::IsSystem());

    std::lock_guard lock(shutdownMutex_);
    shutdownController_->RemoveCallback(callback);
}

void PowerMgrService::RegisterShutdownCallback(
    const sptr<ISyncShutdownCallback>& callback, ShutdownPriority priority)
{
    RETURN_IF (callback == nullptr);
    RETURN_IF (!Permission::IsSystem());

    std::lock_guard lock(shutdownMutex_);
    shutdownController_->AddCallback(callback, priority);
}

void PowerMgrService::UnRegisterShutdownCallback(const sptr<ISyncShutdownCallback>& callback)
{
    RETURN_IF (callback == nullptr);
    RETURN_IF (!Permission::IsSystem());

    std::lock_guard lock(shutdownMutex_);
    shutdownController_->RemoveCallback(callback);
}

sptr<RunningLockTokenStub> PowerMgrService::WakeupRunningLock::token_;

void PowerMgrService::WakeupRunningLock::Create()
{
    token_ = new (std::nothrow) RunningLockTokenStub();
    if (!token_) {
        POWER_HILOGE(COMP_SVC, "create runninglock token failed");
        return;
    }
    RunningLockInfo info = {"PowerMgrWakeupLock", OHOS::PowerMgr::RunningLockType::RUNNINGLOCK_BACKGROUND_TASK};
    pms->CreateRunningLock(token_->AsObject(), info);
}

void PowerMgrService::WakeupRunningLock::Lock()
{
    if (!token_) {
        return;
    }
    pms->Lock(token_->AsObject());
}

void PowerMgrService::WakeupRunningLock::Unlock()
{
    if (!token_) {
        return;
    }
    pms->UnLock(token_->AsObject());
}

void PowerMgrService::SuspendControllerInit()
{
    if (!suspendController_) {
        suspendController_ = std::make_shared<SuspendController>(shutdownController_, powerStateMachine_);
    }
    suspendController_->Init();
}

void PowerMgrService::WakeupControllerInit()
{
    if (!wakeupController_) {
        wakeupController_ = std::make_shared<WakeupController>(powerStateMachine_);
    }
    wakeupController_->Init();
}

void PowerMgrService::HibernateControllerInit()
{
    if (!hibernateController_) {
        hibernateController_ = std::make_shared<HibernateController>();
    }
}

bool PowerMgrService::IsCollaborationState()
{
    bool collaborationState = false;
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        return collaborationState;
    }
    auto stateMachine = pms->GetPowerStateMachine();
    if (stateMachine == nullptr) {
        return collaborationState;
    }
    collaborationState = stateMachine->IsRunningLockEnabled(RunningLockType::RUNNINGLOCK_COORDINATION);
    return collaborationState;
}

#ifdef POWER_MANAGER_WAKEUP_ACTION
void PowerMgrService::WakeupActionControllerInit()
{
    if (!wakeupActionController_) {
        wakeupActionController_ = std::make_shared<WakeupActionController>(shutdownController_, powerStateMachine_);
    }
    wakeupActionController_->Init();
}
#endif

void PowerMgrService::VibratorInit()
{
    std::shared_ptr<PowerVibrator> vibrator = PowerVibrator::GetInstance();
    vibrator->LoadConfig(POWER_VIBRATOR_CONFIG_FILE,
        VENDOR_POWER_VIBRATOR_CONFIG_FILE, SYSTEM_POWER_VIBRATOR_CONFIG_FILE);
}

PowerErrors PowerMgrService::IsStandby(bool& isStandby)
{
#ifdef HAS_DEVICE_STANDBY_PART
    DevStandbyMgr::StandbyServiceClient& standbyServiceClient = DevStandbyMgr::StandbyServiceClient::GetInstance();
    ErrCode code = standbyServiceClient.IsDeviceInStandby(isStandby);
    if (code == ERR_OK) {
        return PowerErrors::ERR_OK;
    }
    return PowerErrors::ERR_CONNECTION_FAIL;
#else
    return PowerErrors::ERR_OK;
#endif
}

PowerErrors PowerMgrService::SetForceTimingOut(bool enabled)
{
    if (!Permission::IsSystem()) {
        return PowerErrors::ERR_SYSTEM_API_DENIED;
    }
    powerStateMachine_->SetForceTimingOut(enabled);
    return PowerErrors::ERR_OK;
}

PowerErrors PowerMgrService::LockScreenAfterTimingOut(bool enabledLockScreen, bool checkLock)
{
    if (!Permission::IsSystem()) {
        return PowerErrors::ERR_SYSTEM_API_DENIED;
    }
    powerStateMachine_->LockScreenAfterTimingOut(enabledLockScreen, checkLock);
    return PowerErrors::ERR_OK;
}

#ifdef HAS_MULTIMODALINPUT_INPUT_PART
void PowerMgrInputMonitor::OnInputEvent(std::shared_ptr<KeyEvent> keyEvent) const
{
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        return;
    }
    auto stateMachine = pms->GetPowerStateMachine();
    if (stateMachine == nullptr) {
        return;
    }
    if (keyEvent->HasFlag(InputEvent::EVENT_FLAG_SIMULATE) &&
        stateMachine->IsRunningLockEnabled(RunningLockType::RUNNINGLOCK_COORDINATION) &&
        stateMachine->GetState() == PowerState::AWAKE) {
        stateMachine->SetState(PowerState::DIM, StateChangeReason::STATE_CHANGE_REASON_COORDINATION);
        POWER_HILOGD(FEATURE_INPUT, "Key event has simulate flag in coordinated state, override screen off time");
    }
}

void PowerMgrInputMonitor::OnInputEvent(std::shared_ptr<PointerEvent> pointerEvent) const
{
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        return;
    }
    auto stateMachine = pms->GetPowerStateMachine();
    if (stateMachine == nullptr) {
        return;
    }
    if (pointerEvent->HasFlag(InputEvent::EVENT_FLAG_SIMULATE) &&
        stateMachine->IsRunningLockEnabled(RunningLockType::RUNNINGLOCK_COORDINATION) &&
        stateMachine->GetState() == PowerState::AWAKE) {
        stateMachine->SetState(PowerState::DIM, StateChangeReason::STATE_CHANGE_REASON_COORDINATION);
        POWER_HILOGD(FEATURE_INPUT, "Pointer event has simulate flag in coordinated state, override screen off time");
    }
}

void PowerMgrInputMonitor::OnInputEvent(std::shared_ptr<AxisEvent> axisEvent) const {};
#endif
} // namespace PowerMgr
} // namespace OHOS
