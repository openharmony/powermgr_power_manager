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

#include <algorithm>
#include <datetime_ex.h>
#include <file_ex.h>
#ifdef HAS_HIVIEWDFX_HISYSEVENT_PART
#include <hisysevent.h>
#endif
#include <if_system_ability_manager.h>
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
#include <input_manager.h>
#endif
#include <ipc_skeleton.h>
#include <iservice_registry.h>
#include <modulemgr.h>
#include <securec.h>
#include <string_ex.h>
#include <system_ability_definition.h>
#include <sys_mgr_client.h>
#include <bundle_mgr_client.h>
#include <unistd.h>
#include "ability_connect_callback_stub.h"
#include "ability_manager_client.h"
#include "ffrt_utils.h"
#include <multi_invoker_helper/multi_invoker_helper.h>
#include "permission.h"
#include "power_common.h"
#include "power_ext_intf_wrapper.h"
#include "power_mgr_dumper.h"
#include "power_vibrator.h"
#include "power_xcollie.h"
#include "setting_helper.h"
#include "running_lock_timer_handler.h"
#include "sysparam.h"
#include "system_suspend_controller.h"
#include "xcollie/watchdog.h"
#include "errors.h"
#include "parameters.h"
#ifdef POWER_LID_FOLD_ENABLE
#include "display_manager_lite.h"
#endif
#ifdef HAS_DEVICE_STANDBY_PART
#include "standby_service_client.h"
#endif
#ifdef POWER_MANAGER_ENABLE_CHARGING_TYPE_SETTING
#include "battery_srv_client.h"
#endif
#ifdef MSDP_MOVEMENT_ENABLE
#include <dlfcn.h>
#endif

using namespace OHOS::AppExecFwk;
using namespace OHOS::AAFwk;
namespace OHOS {
namespace PowerMgr {
// Explicit specialization definition(an initializer is required for it to be a definition)
template<> sptr<PowerMgrService> DelayedSpSingleton<PowerMgrService>::instance_ {};
namespace {
MODULE_MGR *g_moduleMgr = nullptr;
#if (defined(__aarch64__) || defined(__x86_64__))
const char* POWER_PLUGIN_AUTORUN_PATH = "/system/lib64/powerplugin/autorun";
#else
const char* POWER_PLUGIN_AUTORUN_PATH = "/system/lib/powerplugin/autorun";
#endif
const std::string POWERMGR_SERVICE_NAME = "PowerMgrService";
const std::string REASON_POWER_KEY = "power_key";
static std::string g_wakeupReason = "";
const std::string POWER_VIBRATOR_CONFIG_FILE = "etc/power_config/power_vibrator.json";
const std::string VENDOR_POWER_VIBRATOR_CONFIG_FILE = "/vendor/etc/power_config/power_vibrator.json";
const std::string SYSTEM_POWER_VIBRATOR_CONFIG_FILE = "/system/etc/power_config/power_vibrator.json";
static const char* POWER_MANAGER_EXT_PATH = "libpower_manager_ext.z.so";
constexpr int32_t WAKEUP_LOCK_TIMEOUT_MS = 5000;
constexpr int32_t HIBERNATE_GUARD_TIMEOUT_MS = 40000; // PREPARE_HIBERNATE_TIMEOUT_MS + 10000
constexpr int32_t COLLABORATION_REMOTE_DEVICE_ID = 0xAAAAAAFF;
constexpr int32_t INPUT_TASK_TIMEOUT = 50000;
constexpr uint64_t VIRTUAL_SCREEN_START_ID = 1000;
auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
const bool G_REGISTER_RESULT = SystemAbility::MakeAndRegisterAbility(pms.GetRefPtr());
SysParam::BootCompletedCallback g_bootCompletedCallback;
static std::mutex powerInitMutex_;
#ifdef POWER_PICKUP_ENABLE
bool g_isPickUpOpen = false;
#endif
constexpr int32_t API19 = 19;
} // namespace

std::atomic_bool PowerMgrService::isBootCompleted_ = false;
std::atomic_bool PowerMgrService::isNeedReInit_  = false;
std::atomic_bool PowerMgrService::displayManagerServiceCrash_ = false;
#ifdef HAS_SENSORS_SENSOR_PART
std::atomic_bool PowerMgrService::isInLidMode_ = false;
#endif
#ifdef POWER_LID_FOLD_ENABLE
std::atomic_bool PowerMgrService::foldScreenFlag_ = false;
#endif
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
#ifndef FUZZ_TEST
    g_moduleMgr = ModuleMgrScan(POWER_PLUGIN_AUTORUN_PATH);
#endif
    if (!Init()) {
        POWER_HILOGE(COMP_SVC, "powermgr service init fail");
        return;
    }
    AddSystemAbilityListener(SUSPEND_MANAGER_SYSTEM_ABILITY_ID);
    AddSystemAbilityListener(DEVICE_STANDBY_SERVICE_SYSTEM_ABILITY_ID);
    AddSystemAbilityListener(DISPLAY_MANAGER_SERVICE_ID);
    AddSystemAbilityListener(DISPLAY_MANAGER_SERVICE_SA_ID);
#ifdef MSDP_MOVEMENT_ENABLE
    AddSystemAbilityListener(MSDP_MOVEMENT_SERVICE_ID);
#endif
#ifdef POWER_PICKUP_ENABLE
    AddSystemAbilityListener(MSDP_MOTION_SERVICE_ID);
#endif
    AddSystemAbilityListener(COMMON_EVENT_SERVICE_ID);
#ifndef FUZZ_TEST
    SystemSuspendController::GetInstance().RegisterHdiStatusListener();
    PowerExtIntfWrapper::Instance().Init();
#endif
    
    if (!Publish(DelayedSpSingleton<PowerMgrService>::GetInstance())) {
        POWER_HILOGE(COMP_SVC, "Register to system ability manager failed");
        return;
    }
    ready_ = true;
    system::SetParameter("bootevent.powermgr.ready", "true");
    POWER_HILOGI(COMP_SVC, "Add system ability success");
}

bool PowerMgrService::Init()
{
    POWER_HILOGI(COMP_SVC, "powermgr service init start");
    if (!ffrtTimer_) {
        ffrtTimer_ = std::make_shared<FFRTTimer>("power_manager_ffrt_queue");
    }
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
    isDuringCallStateEnable_ = system::GetBoolParameter("const.power.during_call_state_enable", false);
#ifdef POWER_LID_FOLD_ENABLE
    foldScreenFlag_ = system::GetParameter("const.window.foldscreen.type", "") != "";
#endif
    POWER_HILOGI(COMP_SVC, "powermgr service init success %{public}d", isDuringCallStateEnable_);
    return true;
}

void PowerMgrService::RegisterBootCompletedCallback()
{
    POWER_HILOGI(COMP_SVC, "plan to RegisterBootCompletedCallback.");
    g_bootCompletedCallback = []() {
        std::lock_guard lock(powerInitMutex_);
        if (!isNeedReInit_ ) {
            POWER_HILOGW(COMP_SVC, "Power initialization is not required.");
            return;
        }
        POWER_HILOGI(COMP_SVC, "BootCompletedCallback triggered");
        isNeedReInit_  = false;
        auto power = DelayedSpSingleton<PowerMgrService>::GetInstance();
        if (power == nullptr) {
            POWER_HILOGI(COMP_SVC, "get PowerMgrService fail");
            return;
        }
        auto powerStateMachine = power->GetPowerStateMachine();
#ifdef POWER_PICKUP_ENABLE
        SettingHelper::CopyDataForUpdateScene();
#endif
        SettingHelper::UpdateCurrentUserId(); // update setting user id before get setting values
#ifdef POWER_MANAGER_ENABLE_CHARGING_TYPE_SETTING
        power->PowerConnectStatusInit();
        power->UpdateSettingInvalidDisplayOffTime(); // update setting value if invalid before register
#endif
        powerStateMachine->RegisterDisplayOffTimeObserver();
        powerStateMachine->InitState();
        SettingHelper::RegisterAodSwitchObserver();
#ifdef POWER_MANAGER_POWER_DIALOG
        power->GetShutdownDialog().LoadDialogConfig();
        power->GetShutdownDialog().KeyMonitorInit();
#endif
        power->SwitchSubscriberInit();
        power->InputMonitorInit();
        power->SuspendControllerInit();
        power->WakeupControllerInit();
        power->SubscribeCommonEvent();
#ifdef POWER_MANAGER_WAKEUP_ACTION
        power->WakeupActionControllerInit();
#endif
        PowerExternalAbilityInit();
        power->KeepScreenOnInit();
        isBootCompleted_ = true;
    };
    SysParam::RegisterBootCompletedCallbackForPowerSa(g_bootCompletedCallback);
}

void PowerMgrService::PowerExternalAbilityInit()
{
    auto power = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (power == nullptr) {
        POWER_HILOGI(COMP_SVC, "get PowerMgrService fail");
        return;
    }
#ifdef POWER_MANAGER_ENABLE_EXTERNAL_SCREEN_MANAGEMENT
    // External screen listener must be registered after SuspendControllerInit and WakeupControllerInit
    power->UnRegisterExternalScreenListener();
    power->RegisterExternalScreenListener();
    power->ExternalScreenInit();
#endif
    power->VibratorInit();
#ifdef POWER_DOUBLECLICK_ENABLE
    power->RegisterSettingWakeupDoubleClickObservers();
#endif
#ifdef POWER_PICKUP_ENABLE
    power->RegisterSettingWakeupPickupGestureObserver();
#endif
#ifndef CONFIG_FACTORY_MODE
    power->RegisterSettingWakeUpLidObserver();
    POWER_HILOGI(COMP_SVC, "Allow subscribe Hall sensor");
#else
    POWER_HILOGI(COMP_SVC, "Not allow subscribe Hall sensor");
#endif
    power->RegisterSettingPowerModeObservers();
    power->RegisterSettingDuringCallObservers();
    power->RegisterExternalCallback();
}

void PowerMgrService::RegisterSettingPowerModeObservers()
{
    auto power = DelayedSpSingleton<PowerMgrService>::GetInstance();
    power->GetPowerModeModule().InitPowerMode();
    SettingObserver::UpdateFunc updateFunc = [&](const std::string &key) { PowerModeSettingUpdateFunc(key); };
    SettingHelper::RegisterSettingPowerModeObserver(updateFunc);
}

void PowerMgrService::PowerModeSettingUpdateFunc(const std::string &key)
{
    auto power = DelayedSpSingleton<PowerMgrService>::GetInstance();
    int32_t currMode = static_cast<int32_t>(power->GetDeviceMode());
    int32_t saveMode = SettingHelper::ReadCurrentMode(currMode);
    if (currMode == saveMode) {
        return;
    }
    POWER_HILOGI(COMP_SVC, "PowerModeSettingUpdateFunc curr:%{public}d, saveMode:%{public}d", currMode, saveMode);
    power->SetDeviceMode(static_cast<PowerMode>(saveMode));
}

void PowerMgrService::RegisterSettingDuringCallObservers()
{
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (!pms->IsDuringCallStateEnable()) {
        return;
    }
    POWER_HILOGI(COMP_SVC, "will RegisterSettingDuringCallObservers");
    SettingObserver::UpdateFunc updateFunc = [&](const std::string &key) { DuringCallSettingUpdateFunc(key); };
    SettingHelper::RegisterSettingDuringCallObserver(updateFunc);
}

void PowerMgrService::DuringCallSettingUpdateFunc(const std::string &key)
{
    auto power = DelayedSpSingleton<PowerMgrService>::GetInstance();
    bool duringCallState = SettingHelper::GetSettingDuringCallState(key);
    POWER_HILOGI(COMP_SVC, "DuringCallState is %{public}d", duringCallState);
    auto stateMachine = power->GetPowerStateMachine();
    if (stateMachine == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "PowerStateMachine is nullptr");
        return;
    }
    stateMachine->SetDuringCallState(duringCallState);
}

bool PowerMgrService::IsDeveloperMode()
{
    return OHOS::system::GetBoolParameter("const.security.developermode.state", true);
}

void PowerMgrService::KeepScreenOnInit()
{
    if (ptoken_ != nullptr) {
        POWER_HILOGI(COMP_SVC, "runninglock token is not null");
        return;
    }
    ptoken_ = new (std::nothrow) RunningLockTokenStub();
    if (ptoken_ == nullptr) {
        POWER_HILOGI(COMP_SVC, "create runninglock token failed");
        return;
    }
    RunningLockInfo info = {"PowerMgrKeepOnLock", OHOS::PowerMgr::RunningLockType::RUNNINGLOCK_SCREEN};
    PowerErrors ret = pms->CreateRunningLock(ptoken_, info);
    if (ret != PowerErrors::ERR_OK) {
        POWER_HILOGI(COMP_SVC, "create runninglock failed");
    }
    return;
}

void PowerMgrService::KeepScreenOn(bool isOpenOn)
{
    if (!IsDeveloperMode()) {
        POWER_HILOGI(COMP_SVC, "not developer mode");
        return;
    }
    if (ptoken_ == nullptr) {
        POWER_HILOGI(COMP_SVC, "runninglock token is null");
        return;
    }
    if (isOpenOn) {
        POWER_HILOGI(COMP_SVC, "try lock RUNNINGLOCK_SCREEN");
        pms->Lock(ptoken_);
    } else {
        POWER_HILOGI(COMP_SVC, "try unlock RUNNINGLOCK_SCREEN");
        pms->UnLock(ptoken_);
    }
    return;
}

#ifdef POWER_DOUBLECLICK_ENABLE
void PowerMgrService::RegisterSettingWakeupDoubleClickObservers()
{
    SettingObserver::UpdateFunc updateFunc = [&](const std::string& key) {WakeupDoubleClickSettingUpdateFunc(key); };
    SettingHelper::RegisterSettingWakeupDoubleObserver(updateFunc);
}

void PowerMgrService::WakeupDoubleClickSettingUpdateFunc(const std::string& key)
{
    bool isSettingEnable = GetSettingWakeupDoubleClick(key);
    WakeupController::ChangeWakeupSourceConfig(isSettingEnable);
    WakeupController::SetWakeupDoubleClickSensor(isSettingEnable);
    POWER_HILOGI(COMP_SVC, "WakeupDoubleClickSettingUpdateFunc isSettingEnable=%{public}d", isSettingEnable);
}

bool PowerMgrService::GetSettingWakeupDoubleClick(const std::string& key)
{
    return SettingHelper::GetSettingWakeupDouble(key);
}
#endif
#ifdef POWER_PICKUP_ENABLE
void PowerMgrService::RegisterSettingWakeupPickupGestureObserver()
{
    SettingObserver::UpdateFunc updateFunc = [&](const std::string& key) {WakeupPickupGestureSettingUpdateFunc(key);};
    SettingHelper::RegisterSettingWakeupPickupObserver(updateFunc);
}

void PowerMgrService::WakeupPickupGestureSettingUpdateFunc(const std::string& key)
{
    bool isSettingEnable = SettingHelper::GetSettingWakeupPickup(key);
    g_isPickUpOpen = isSettingEnable;
    WakeupController::PickupConnectMotionConfig(isSettingEnable);
    POWER_HILOGI(COMP_SVC, "PickupConnectMotionConfig done, isSettingEnable=%{public}d", isSettingEnable);
    WakeupController::ChangePickupWakeupSourceConfig(isSettingEnable);
    POWER_HILOGI(COMP_SVC, "ChangePickupWakeupSourceConfig done");
}
#endif

bool PowerMgrService::PowerStateMachineInit()
{
    if (powerStateMachine_ == nullptr) {
        powerStateMachine_ = std::make_shared<PowerStateMachine>(pms, ffrtTimer_);
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

void PowerMgrService::WakeupLidSettingUpdateFunc(const std::string& key)
{
    POWER_HILOGI(COMP_SVC, "Receive lid wakeup setting update.");
    bool enable = SettingHelper::GetSettingWakeupLid(key);
    if (enable) {
        pms->HallSensorSubscriberInit();
    } else {
        pms->HallSensorSubscriberCancel();
    }
    std::shared_ptr<WakeupController> wakeupController = pms->GetWakeupController();
    if (wakeupController == nullptr) {
        POWER_HILOGE(FEATURE_INPUT, "wakeupController is not init");
        return;
    }
    wakeupController->ChangeLidWakeupSourceConfig(enable);
    POWER_HILOGI(COMP_SVC, "ChangeLidWakeupSourceConfig done");
}

void PowerMgrService::RegisterSettingWakeUpLidObserver()
{
    pms->HallSensorSubscriberInit();
    POWER_HILOGI(COMP_SVC, "Start to registerSettingWakeUpLidObserver");
    if (!SettingHelper::IsWakeupLidSettingValid()) {
        POWER_HILOGE(COMP_UTILS, "settings.power.wakeup_lid is valid.");
        return;
    }
    SettingObserver::UpdateFunc updateFunc = [&](const std::string& key) {WakeupLidSettingUpdateFunc(key);};
    SettingHelper::RegisterSettingWakeupLidObserver(updateFunc);
}

void PowerMgrService::HallSensorSubscriberInit()
{
#ifdef HAS_SENSORS_SENSOR_PART
    POWER_HILOGI(COMP_SVC, "Start to subscribe hall sensor");
    if (!IsSupportSensor(SENSOR_TYPE_ID_HALL)) {
        std::string eventReason = "SENSOR_TYPE_ID_HALL sensor not support";
        POWER_HILOGW(FEATURE_INPUT, "%{public}s", eventReason.c_str());
#ifdef HAS_HIVIEWDFX_HISYSEVENT_PART
        HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::POWER, "ABNORMAL_FAULT",
            HiviewDFX::HiSysEvent::EventType::FAULT, "TYPE", "SCREEN_ON_OFF", "REASON", eventReason);
#endif
        return;
    }
    if (strcpy_s(sensorUser_.name, sizeof(sensorUser_.name), "PowerManager") != EOK) {
        POWER_HILOGW(FEATURE_INPUT, "strcpy_s error");
        return;
    }
    isInLidMode_ = false;
    sensorUser_.userData = nullptr;
    sensorUser_.callback = &HallSensorCallback;
    int32_t ret = SubscribeSensor(SENSOR_TYPE_ID_HALL, &sensorUser_);
    SetBatch(SENSOR_TYPE_ID_HALL, &sensorUser_, HALL_SAMPLING_RATE, HALL_REPORT_INTERVAL);
    int32_t active = ActivateSensor(SENSOR_TYPE_ID_HALL, &sensorUser_);
    if (ret != ERR_OK || active != ERR_OK) {
        std::string eventReason = "HallSensorSubscriberInit failed";
        POWER_HILOGW(FEATURE_INPUT, "%{public}s,ret=%{public}d,active=%{public}d", eventReason.c_str(), ret, active);
#ifdef HAS_HIVIEWDFX_HISYSEVENT_PART
        HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::POWER, "ABNORMAL_FAULT",
            HiviewDFX::HiSysEvent::EventType::FAULT, "TYPE", "SCREEN_ON_OFF", "REASON", eventReason);
#endif
    }
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

#ifdef POWER_LID_FOLD_ENABLE
    Rosen::FoldDisplayMode mode = Rosen::DisplayManagerLite::GetInstance().GetFoldDisplayMode();
    if (foldScreenFlag_ && (mode != Rosen::FoldDisplayMode::MAIN)) {
        POWER_HILOGI(FEATURE_SUSPEND, "[UL_POWER] disable Lid mode in the expanded state");
        isInLidMode_ = false;
        return;
    }
#endif
    if (status & LID_CLOSED_HALL_FLAG) {
        if (isInLidMode_) {
            POWER_HILOGI(FEATURE_SUSPEND, "[UL_POWER] Lid close event received again");
            return;
        }
        POWER_HILOGI(FEATURE_SUSPEND, "[UL_POWER] Lid close event received, begin to suspend");
        isInLidMode_ = true;
        SuspendDeviceType reason = SuspendDeviceType::SUSPEND_DEVICE_REASON_LID;
        suspendController->ExecSuspendMonitorByReason(reason);
    } else {
        if (!isInLidMode_) {
            POWER_HILOGI(FEATURE_WAKEUP, "[UL_POWER] Lid open event received again");
            return;
        }
        POWER_HILOGI(FEATURE_WAKEUP, "[UL_POWER] Lid open event received, begin to wakeup");
        isInLidMode_ = false;
        WakeupDeviceType reason = WakeupDeviceType::WAKEUP_DEVICE_LID;
        wakeupController->ExecWakeupMonitorByReason(reason);
    }
}
#endif

void PowerMgrService::HallSensorSubscriberCancel()
{
#ifdef HAS_SENSORS_SENSOR_PART
    POWER_HILOGI(COMP_SVC, "Start to cancel the subscribe of hall sensor");
    if (IsSupportSensor(SENSOR_TYPE_ID_HALL)) {
        DeactivateSensor(SENSOR_TYPE_ID_HALL, &sensorUser_);
        UnsubscribeSensor(SENSOR_TYPE_ID_HALL, &sensorUser_);
    }
    isInLidMode_ = false;
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
    auto inputManager = InputManager::GetInstance();
    if (!inputManager) {
        std::string eventReason = "SwitchSubscriberInit inputManager is null";
        POWER_HILOGW(FEATURE_INPUT, "%{public}s", eventReason.c_str());
#ifdef HAS_HIVIEWDFX_HISYSEVENT_PART
        HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::POWER, "ABNORMAL_FAULT",
            HiviewDFX::HiSysEvent::EventType::FAULT, "TYPE", "SCREEN_ON_OFF", "REASON", eventReason);
#endif
        return;
    }
    switchId_ =
        inputManager->SubscribeSwitchEvent([this](std::shared_ptr<OHOS::MMI::SwitchEvent> switchEvent) {
            POWER_HILOGI(FEATURE_WAKEUP, "switch event received");
            if (switchEvent->GetSwitchValue() == SwitchEvent::SWITCH_OFF) {
                POWER_HILOGI(FEATURE_SUSPEND, "[UL_POWER] Switch close event received, begin to suspend");
                std::shared_ptr<SuspendController> suspendController = pms->GetSuspendController();
                if (suspendController == nullptr) {
                    POWER_HILOGE(FEATURE_INPUT, "get suspendController instance error");
                    return;
                }
                powerStateMachine_->SetSwitchState(false);
                SuspendDeviceType reason = SuspendDeviceType::SUSPEND_DEVICE_REASON_SWITCH;
                suspendController->ExecSuspendMonitorByReason(reason);
            } else {
                POWER_HILOGI(FEATURE_WAKEUP, "[UL_POWER] Switch open event received, begin to wakeup");
                std::shared_ptr<WakeupController> wakeupController = pms->GetWakeupController();
                if (wakeupController == nullptr) {
                    POWER_HILOGE(FEATURE_INPUT, "get wakeupController instance error");
                    return;
                }
                powerStateMachine_->SetSwitchState(true);
                WakeupDeviceType reason = WakeupDeviceType::WAKEUP_DEVICE_SWITCH;
                wakeupController->ExecWakeupMonitorByReason(reason);
            }
        });
    if (switchId_ == -1 || switchId_ > INPUT_TASK_TIMEOUT) {
        std::string eventReason = "SwitchSubscriberInit failed";
        POWER_HILOGW(FEATURE_INPUT, "%{public}s,switchId_=%{public}d", eventReason.c_str(), switchId_);
#ifdef HAS_HIVIEWDFX_HISYSEVENT_PART
        HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::POWER, "ABNORMAL_FAULT",
            HiviewDFX::HiSysEvent::EventType::FAULT, "TYPE", "SCREEN_ON_OFF", "REASON", eventReason);
#endif
    }
#endif
}

void PowerMgrService::SwitchSubscriberCancel()
{
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
    POWER_HILOGI(FEATURE_INPUT, "Unsubscribe switch information");
    auto inputManager = InputManager::GetInstance();
    if (!inputManager) {
        POWER_HILOGE(FEATURE_INPUT, "SwitchSubscriberCancel inputManager is null");
        return;
    }
    if (switchId_ >= 0) {
        inputManager->UnsubscribeSwitchEvent(switchId_);
        switchId_ = -1;
    }
#endif
}

void PowerMgrService::InputMonitorInit()
{
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
    POWER_HILOGI(FEATURE_INPUT, "PowerMgr service input monitor init");
    std::shared_ptr<PowerMgrInputMonitor> inputMonitor = std::make_shared<PowerMgrInputMonitor>();
    auto inputManager = InputManager::GetInstance();
    if (!inputManager) {
        POWER_HILOGE(FEATURE_INPUT, "InputMonitorInit inputManager is null");
        return;
    }
    if (inputMonitorId_ < 0) {
        inputMonitorId_ =
            inputManager->AddMonitor(std::static_pointer_cast<IInputEventConsumer>(inputMonitor));
    }
#endif
}

void PowerMgrService::InputMonitorCancel()
{
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
    POWER_HILOGI(FEATURE_INPUT, "PowerMgr service input monitor cancel");
    InputManager* inputManager = InputManager::GetInstance();
    if (!inputManager) {
        POWER_HILOGE(FEATURE_INPUT, "InputMonitorCancel inputManager is null");
        return;
    }
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
        this->RefreshActivityInner(now, UserActivityType::USER_ACTIVITY_TYPE_BUTTON, false);
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
        this->RefreshActivityInner(now, UserActivityType::USER_ACTIVITY_TYPE_ATTENTION, false);
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
#ifdef POWER_MANAGER_ENABLE_EXTERNAL_SCREEN_MANAGEMENT
    UnRegisterExternalScreenListener();
#endif
    SwitchSubscriberCancel();
    InputMonitorCancel();
    ready_ = false;
    isBootCompleted_ = false;
    RemoveSystemAbilityListener(SUSPEND_MANAGER_SYSTEM_ABILITY_ID);
    RemoveSystemAbilityListener(DEVICE_STANDBY_SERVICE_SYSTEM_ABILITY_ID);
    RemoveSystemAbilityListener(DISPLAY_MANAGER_SERVICE_ID);
#ifdef MSDP_MOVEMENT_ENABLE
    RemoveSystemAbilityListener(MSDP_MOVEMENT_SERVICE_ID);
#endif
#ifdef POWER_PICKUP_ENABLE
    RemoveSystemAbilityListener(MSDP_MOTION_SERVICE_ID);
#endif
#ifdef POWER_DOUBLECLICK_ENABLE
    SettingHelper::UnregisterSettingWakeupDoubleObserver();
#endif
#ifdef POWER_PICKUP_ENABLE
    SettingHelper::UnregisterSettingWakeupPickupObserver();
#endif
    SettingHelper::UnRegisterSettingWakeupLidObserver();
    SettingHelper::UnRegisterSettingPowerModeObserver();
    if (!OHOS::EventFwk::CommonEventManager::UnSubscribeCommonEvent(subscriberPtr_)) {
        POWER_HILOGE(COMP_SVC, "Power Onstop unregister to commonevent manager failed!");
    }
#ifdef MSDP_MOVEMENT_ENABLE
    UnRegisterMovementCallback();
#endif
    UnregisterExternalCallback();
#ifndef FUZZ_TEST
    PowerExtIntfWrapper::Instance().DeInit();
    ModuleMgrDestroy(g_moduleMgr);
    g_moduleMgr = nullptr;
#endif
}

void PowerMgrService::Reset()
{
    POWER_HILOGW(COMP_SVC, "start destruct ffrt_queue");
    if (powerStateMachine_) {
        // release one strong reference to ffrtTimer
        powerStateMachine_->Reset();
    }
    if (suspendController_) {
        // release one strong reference to ffrtTimer
        suspendController_->Reset();
    }
    ffrtTimer_.reset(); // all strong references gone, ffrtTimer will be destructed.
    if (screenOffPreController_) {
        // another queue without using FFRTTimer, leave it as is for now.
        screenOffPreController_->Reset();
    }
}

void PowerMgrService::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    POWER_HILOGI(COMP_SVC, "systemAbilityId=%{public}d, deviceId=%{private}s", systemAbilityId, deviceId.c_str());
    if (systemAbilityId == SUSPEND_MANAGER_SYSTEM_ABILITY_ID ||
        systemAbilityId == DEVICE_STANDBY_SERVICE_SYSTEM_ABILITY_ID) {
        std::lock_guard lock(lockMutex_);
        runningLockMgr_->ResetRunningLocks();
    }
#ifdef MSDP_MOVEMENT_ENABLE
    if (systemAbilityId == MSDP_MOVEMENT_SERVICE_ID) {
        auto power = DelayedSpSingleton<PowerMgrService>::GetInstance();
        if (power == nullptr) {
            POWER_HILOGI(COMP_SVC, "get PowerMgrService fail");
            return;
        }
        power->ResetMovementState();
    }
#endif
    if (systemAbilityId ==  DISPLAY_MANAGER_SERVICE_SA_ID) {
        std::lock_guard lock(powerInitMutex_);
        POWER_HILOGI(COMP_SVC, "get DISPLAY_MANAGER_SERVICE_SA_ID crash in PowerService.");
        displayManagerServiceCrash_ = true;
    }
}

void PowerMgrService::OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    POWER_HILOGI(COMP_SVC, "systemAbilityId=%{public}d, deviceId=%{private}s Add",
        systemAbilityId, deviceId.c_str());
#ifdef HAS_DISPLAY_MANAGER_PART
    if (systemAbilityId == DISPLAY_MANAGER_SERVICE_ID) {
#else
    if (systemAbilityId == DISPLAY_MANAGER_SERVICE_SA_ID) {
#endif
        std::lock_guard lock(powerInitMutex_);
        POWER_HILOGI(COMP_SVC, "get DISPLAY_MANAGER_SERVICE_ID in PowerService");
        isNeedReInit_  = true;
        RegisterBootCompletedCallback();
    }

#ifdef HAS_DISPLAY_MANAGER_PART
    if (systemAbilityId ==  DISPLAY_MANAGER_SERVICE_SA_ID) {
        std::lock_guard lock(powerInitMutex_);
        POWER_HILOGI(COMP_SVC, "get DISPLAY_MANAGER_SERVICE_SA_ID in PowerService");
        if (displayManagerServiceCrash_) {
            isNeedReInit_  = true;
            RegisterBootCompletedCallback();
            displayManagerServiceCrash_ = false;
        }
    }
#endif
#ifdef MSDP_MOVEMENT_ENABLE
    if (systemAbilityId == MSDP_MOVEMENT_SERVICE_ID) {
        auto power = DelayedSpSingleton<PowerMgrService>::GetInstance();
        if (power == nullptr) {
            POWER_HILOGI(COMP_SVC, "get PowerMgrService fail");
            return;
        }
        power->UnRegisterMovementCallback();
        power->RegisterMovementCallback();
    }
#endif
#ifdef POWER_PICKUP_ENABLE
    if (systemAbilityId == MSDP_MOTION_SERVICE_ID && g_isPickUpOpen == true) {
        WakeupController::PickupConnectMotionConfig(false);
        WakeupController::PickupConnectMotionConfig(true);
    }
#endif
    OnAddSystemAbilityInner(systemAbilityId, deviceId);
}

void PowerMgrService::OnAddSystemAbilityInner(int32_t systemAbilityId, [[maybe_unused]] const std::string& deviceId)
{
    if (systemAbilityId == COMMON_EVENT_SERVICE_ID) {
        this->GetPowerModeModule().SubscribeCommonEvent();
        return;
    }
}

#ifdef MSDP_MOVEMENT_ENABLE
static const char* MOVEMENT_SUBSCRIBER_CONFIG = "RegisterMovementCallback";
static const char* MOVEMENT_UNSUBSCRIBER_CONFIG = "UnRegisterMovementCallback";
static const char* RESET_MOVEMENT_STATE_CONFIG = "ResetMovementState";
typedef void(*FuncMovementSubscriber)();
typedef void(*FuncMovementUnsubscriber)();
typedef void(*FuncResetMovementState)();

void PowerMgrService::RegisterMovementCallback()
{
    POWER_HILOGI(COMP_SVC, "Start to RegisterMovementCallback");
    void *subscriberHandler = dlopen(POWER_MANAGER_EXT_PATH, RTLD_LAZY | RTLD_NODELETE);
    if (subscriberHandler == nullptr) {
        POWER_HILOGE(COMP_SVC, "Dlopen RegisterMovementCallback failed, reason : %{public}s", dlerror());
        return;
    }

    FuncMovementSubscriber MovementSubscriber =
        reinterpret_cast<FuncMovementSubscriber>(dlsym(subscriberHandler, MOVEMENT_SUBSCRIBER_CONFIG));
    if (MovementSubscriber == nullptr) {
        POWER_HILOGE(COMP_SVC, "RegisterMovementCallback is null, reason : %{public}s", dlerror());
        dlclose(subscriberHandler);
        subscriberHandler = nullptr;
        return;
    }
    MovementSubscriber();
    POWER_HILOGI(COMP_SVC, "RegisterMovementCallback Success");
    dlclose(subscriberHandler);
    subscriberHandler = nullptr;
    return;
}

void PowerMgrService::UnRegisterMovementCallback()
{
    POWER_HILOGI(COMP_SVC, "Start to UnRegisterMovementCallback");
    void *unSubscriberHandler = dlopen(POWER_MANAGER_EXT_PATH, RTLD_LAZY | RTLD_NODELETE);
    if (unSubscriberHandler == nullptr) {
        POWER_HILOGE(COMP_SVC, "Dlopen UnRegisterMovementCallback failed, reason : %{public}s", dlerror());
        return;
    }

    FuncMovementUnsubscriber MovementUnsubscriber =
        reinterpret_cast<FuncMovementUnsubscriber>(dlsym(unSubscriberHandler, MOVEMENT_UNSUBSCRIBER_CONFIG));
    if (MovementUnsubscriber == nullptr) {
        POWER_HILOGE(COMP_SVC, "UnRegisterMovementCallback is null, reason : %{public}s", dlerror());
        dlclose(unSubscriberHandler);
        unSubscriberHandler = nullptr;
        return;
    }
    MovementUnsubscriber();
    POWER_HILOGI(COMP_SVC, "UnRegisterMovementCallback Success");
    dlclose(unSubscriberHandler);
    unSubscriberHandler = nullptr;
    return;
}

void PowerMgrService::ResetMovementState()
{
    POWER_HILOGI(COMP_SVC, "Start to ResetMovementState");
    void *resetMovementStateHandler = dlopen(POWER_MANAGER_EXT_PATH, RTLD_LAZY | RTLD_NODELETE);
    if (resetMovementStateHandler == nullptr) {
        POWER_HILOGE(COMP_SVC, "Dlopen ResetMovementState failed, reason : %{public}s", dlerror());
        return;
    }

    FuncResetMovementState ResetMovementState =
        reinterpret_cast<FuncResetMovementState>(dlsym(resetMovementStateHandler, RESET_MOVEMENT_STATE_CONFIG));
    if (ResetMovementState == nullptr) {
        POWER_HILOGE(COMP_SVC, "ResetMovementState is null, reason : %{public}s", dlerror());
        dlclose(resetMovementStateHandler);
        resetMovementStateHandler = nullptr;
        return;
    }
    ResetMovementState();
    POWER_HILOGI(COMP_SVC, "ResetMovementState Success");
    dlclose(resetMovementStateHandler);
    resetMovementStateHandler = nullptr;
    return;
}
#endif

static const char* REGISTER_EXTERNAL_CONFIG = "RegisterExternalCallback";
static const char* UNREGISTER_EXTERNAL_CONFIG = "UnregisterExternalCallback";
using WakeupFunc = std::function<void(WakeupDeviceType)>;
using SuspendFunc = std::function<void(SuspendDeviceType)>;
using PowerConfigFunc = std::function<int32_t(std::string&, std::string&)>;
using TriggerSyncSleepCallbackFunc = std::function<void(bool)>;
typedef void (*FuncRegisterExternalCallback)(WakeupFunc, SuspendFunc, PowerConfigFunc, PowerConfigFunc,
    TriggerSyncSleepCallbackFunc);
typedef void (*FuncUnregisterExternalCallback)();

void PowerMgrService::RegisterExternalCallback()
{
    POWER_HILOGI(COMP_SVC, "Start to RegisterExternalCallback");
    void* subscriberHandler = dlopen(POWER_MANAGER_EXT_PATH, RTLD_LAZY | RTLD_NODELETE);
    if (subscriberHandler == nullptr) {
        POWER_HILOGE(COMP_SVC, "Dlopen RegisterExternalCallback failed, reason : %{public}s", dlerror());
        return;
    }

    FuncRegisterExternalCallback registerExternalCallback =
        reinterpret_cast<FuncRegisterExternalCallback>(dlsym(subscriberHandler, REGISTER_EXTERNAL_CONFIG));
    if (registerExternalCallback == nullptr) {
        POWER_HILOGE(COMP_SVC, "RegisterExternalCallback is null, reason : %{public}s", dlerror());
        dlclose(subscriberHandler);
        subscriberHandler = nullptr;
        return;
    }
    registerExternalCallback(
        [this](WakeupDeviceType reason) {
            POWER_HILOGI(COMP_SVC, "[UL_POWER] wakeup callback triggered, reason: %{public}d", reason);
            wakeupController_->ExecWakeupMonitorByReason(reason);
        },
        [this](SuspendDeviceType reason) {
            POWER_HILOGI(COMP_SVC, "[UL_POWER] suspend callback triggered, reason: %{public}d", reason);
            suspendController_->ExecSuspendMonitorByReason(reason);
        },
        [](std::string& sceneName, std::string& value) {
            return SystemSuspendController::GetInstance().GetPowerConfig(sceneName, value);
        },
        [](std::string& sceneName, std::string value) {
            return SystemSuspendController::GetInstance().SetPowerConfig(sceneName, value);
        },
        [this](bool isWakeup) {
            POWER_HILOGI(COMP_SVC, "[UL_POWER] sync sleep callback triggered, isWakeup: %{public}d", isWakeup);
            suspendController_->TriggerSyncSleepCallback(isWakeup);
        });
    POWER_HILOGI(COMP_SVC, "RegisterExternalCallback Success");
    dlclose(subscriberHandler);
    subscriberHandler = nullptr;
}

void PowerMgrService::UnregisterExternalCallback()
{
    POWER_HILOGI(COMP_SVC, "Start to UnregisterExternalCallback");
    void* unSubscriberHandler = dlopen(POWER_MANAGER_EXT_PATH, RTLD_LAZY | RTLD_NODELETE);
    if (unSubscriberHandler == nullptr) {
        POWER_HILOGE(COMP_SVC, "Dlopen UnregisterExternalCallback failed, reason : %{public}s", dlerror());
        return;
    }

    FuncUnregisterExternalCallback unregisterExternalCallback =
        reinterpret_cast<FuncUnregisterExternalCallback>(dlsym(unSubscriberHandler, UNREGISTER_EXTERNAL_CONFIG));
    if (unregisterExternalCallback == nullptr) {
        POWER_HILOGE(COMP_SVC, "UnregisterExternalCallback is null, reason : %{public}s", dlerror());
        dlclose(unSubscriberHandler);
        unSubscriberHandler = nullptr;
        return;
    }
    unregisterExternalCallback();
    POWER_HILOGI(COMP_SVC, "UnregisterExternalCallback Success");
    dlclose(unSubscriberHandler);
    unSubscriberHandler = nullptr;
}

static const char* ON_CHARGE_STATE_CHANGED_CONFIG = "OnChargeStateChanged";
typedef void (*FuncOnChargeStateChanged)();

void PowerMgrService::OnChargeStateChanged()
{
    POWER_HILOGI(COMP_SVC, "Start to OnChargeStateChanged");
    void* subscriberHandler = dlopen(POWER_MANAGER_EXT_PATH, RTLD_LAZY | RTLD_NODELETE);
    if (subscriberHandler == nullptr) {
        POWER_HILOGE(COMP_SVC, "Dlopen OnChargeStateChanged failed, reason : %{public}s", dlerror());
        return;
    }

    FuncOnChargeStateChanged onChargeStateChanged =
        reinterpret_cast<FuncOnChargeStateChanged>(dlsym(subscriberHandler, ON_CHARGE_STATE_CHANGED_CONFIG));
    if (onChargeStateChanged == nullptr) {
        POWER_HILOGE(COMP_SVC, "OnChargeStateChanged is null, reason : %{public}s", dlerror());
        dlclose(subscriberHandler);
        subscriberHandler = nullptr;
        return;
    }
    onChargeStateChanged();
    POWER_HILOGI(COMP_SVC, "OnChargeStateChanged Success");
    dlclose(subscriberHandler);
    subscriberHandler = nullptr;
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
        POWER_HILOGD(COMP_SVC, "arg: %{public}s", ret.c_str());
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

PowerErrors PowerMgrService::RebootDeviceForDeprecated(const std::string& reason, bool force)
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
#ifdef HAS_HIVIEWDFX_HISYSEVENT_PART
    powerStateMachine_->ReportShutdownStart(uid, reason, true);
#endif
    POWER_KHILOGI(FEATURE_SHUTDOWN, "Do reboot, called pid: %{public}d, uid: %{public}d", pid, uid);
    shutdownController_->Reboot(reason, force);
    return PowerErrors::ERR_OK;
}

PowerErrors PowerMgrService::ForceRebootDevice(const std::string& reason)
{
    if (!Permission::IsSystem()) {
        return PowerErrors::ERR_SYSTEM_API_DENIED;
    }
    return RebootDeviceForDeprecated(reason, true);
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
    SystemSuspendController::GetInstance().Wakeup(); // stop suspend loop
#ifdef HAS_HIVIEWDFX_HISYSEVENT_PART
    powerStateMachine_->ReportShutdownStart(uid, reason, false);
#endif
    POWER_KHILOGI(FEATURE_SHUTDOWN, "[UL_POWER] Do shutdown, called pid: %{public}d, uid: %{public}d", pid, uid);
    shutdownController_->Shutdown(reason);
    return PowerErrors::ERR_OK;
}

PowerErrors PowerMgrService::GetShutdownReason(std::string& reason)
{
    if (!Permission::IsSystem()) {
        POWER_HILOGE(FEATURE_SHUTDOWN, "Get SR failed,System permission reject");
        return PowerErrors::ERR_SYSTEM_API_DENIED;
    }
    pid_t pid = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    reason = system::GetParameter("persist.dfx.shutdown_reason", "");
    POWER_HILOGI(FEATURE_SHUTDOWN, "Get SR,P=%{public}d,U=%{public}d,R=%{public}s", pid, uid, reason.c_str());
    return PowerErrors::ERR_OK;
}

PowerErrors PowerMgrService::SetSuspendTag(const std::string& tag)
{
    std::lock_guard lock(suspendMutex_);
    pid_t pid = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    if (!Permission::IsSystem()) {
        return PowerErrors::ERR_SYSTEM_API_DENIED;
    }
    POWER_HILOGI(FEATURE_SUSPEND, "pid: %{public}d, uid: %{public}d, tag: %{public}s", pid, uid, tag.c_str());
    SystemSuspendController::GetInstance().SetSuspendTag(tag);
    return PowerErrors::ERR_OK;
}

PowerErrors PowerMgrService::SuspendDevice(
    int64_t callTimeMs, SuspendDeviceType reason, bool suspendImmed, const std::string& apiVersion)
{
    std::lock_guard lock(suspendMutex_);
    pid_t pid = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    if (!Permission::IsSystem()) {
        POWER_HILOGI(FEATURE_SUSPEND, "SuspendDevice failed, System permission intercept");
        return PowerErrors::ERR_SYSTEM_API_DENIED;
    }
    int32_t version = static_cast<int32_t>(strtol(apiVersion.c_str(), nullptr, 10));
    if (version >= API19 && !Permission::IsPermissionGranted("ohos.permission.POWER_MANAGER")) {
        POWER_HILOGI(FEATURE_SUSPEND, "SuspendDevice failed, The application does not have the permission");
        return PowerErrors::ERR_PERMISSION_DENIED;
    }

#ifdef POWER_MANAGER_ENABLE_WATCH_BOOT_COMPLETED
    if (isBootCompleted_ == false) {
        POWER_HILOGI(FEATURE_SUSPEND, "SuspendDevice failed, not boot completed, pid: %{public}d, uid: %{public}d",
            pid, uid);
        return PowerErrors::ERR_FAILURE;
    }
#endif

    if (shutdownController_->IsShuttingDown()) {
        POWER_HILOGW(FEATURE_SUSPEND, "System is shutting down, can't suspend");
        return PowerErrors::ERR_OK;
    }
#ifdef HAS_HIVIEWDFX_HISYSEVENT_PART
    powerStateMachine_->ReportSuspendStart(uid, static_cast<int32_t>(reason), false);
#endif
    POWER_HILOGI(FEATURE_SUSPEND, "[UL_POWER] Try to suspend device, pid: %{public}d, uid: %{public}d", pid, uid);
    powerStateMachine_->SuspendDeviceInner(pid, callTimeMs, reason, suspendImmed);
    return PowerErrors::ERR_OK;
}

PowerErrors PowerMgrService::WakeupDevice(
    int64_t callTimeMs, WakeupDeviceType reason, const std::string& details, const std::string& apiVersion)
{
    std::lock_guard lock(wakeupMutex_);
    if (!Permission::IsSystem()) {
        POWER_HILOGI(FEATURE_SUSPEND, "WakeupDevice failed, System permission intercept");
        return PowerErrors::ERR_SYSTEM_API_DENIED;
    }
    int32_t version = static_cast<int32_t>(strtol(apiVersion.c_str(), nullptr, 10));
    if (version >= API19 && !Permission::IsPermissionGranted("ohos.permission.POWER_MANAGER")) {
        POWER_HILOGI(FEATURE_SUSPEND, "WakeupDevice failed, The application does not have the permission");
        return PowerErrors::ERR_PERMISSION_DENIED;
    }
    pid_t pid = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();

#ifdef POWER_MANAGER_ENABLE_WATCH_BOOT_COMPLETED
    if (isBootCompleted_ == false) {
        POWER_HILOGI(FEATURE_WAKEUP, "WakeupDevice failed, not boot completed, pid: %{public}d, uid: %{public}d",
            pid, uid);
        return PowerErrors::ERR_FAILURE;
    }
#endif
#ifdef HAS_HIVIEWDFX_HISYSEVENT_PART
    powerStateMachine_->ReportWakeupStart(uid, static_cast<int32_t>(reason));
#endif
    POWER_HILOGI(FEATURE_WAKEUP, "[UL_POWER] Try to wakeup device, pid: %{public}d, uid: %{public}d", pid, uid);

    BackgroundRunningLock wakeupRunningLock("W", WAKEUP_LOCK_TIMEOUT_MS);
    if (details == "display_doze") {
        bool ret = powerStateMachine_->SetDozeMode(DisplayState::DISPLAY_DOZE);
        return ret ? PowerErrors::ERR_OK : PowerErrors::ERR_FAILURE;
    }
    if (details == "display_doze_suspend") {
        bool ret = powerStateMachine_->SetDozeMode(DisplayState::DISPLAY_DOZE_SUSPEND);
        return ret ? PowerErrors::ERR_OK : PowerErrors::ERR_FAILURE;
    }
    if (details == "display_off") {
        bool ret = powerStateMachine_->SetDozeMode(DisplayState::DISPLAY_OFF);
        return ret ? PowerErrors::ERR_OK : PowerErrors::ERR_FAILURE;
    }
    powerStateMachine_->WakeupDeviceInner(pid, callTimeMs, reason, details, "OHOS");
    return PowerErrors::ERR_OK;
}

bool PowerMgrService::RefreshActivity(int64_t callTimeMs, UserActivityType type, bool needChangeBacklight)
{
    if (!Permission::IsPermissionGranted("ohos.permission.REFRESH_USER_ACTION") || !Permission::IsSystem()) {
        return false;
    }
    pid_t pid = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    POWER_HILOGI(FEATURE_ACTIVITY,
        "Try to refresh activity, pid: %{public}d, uid: %{public}d, activity type: %{public}u", pid, uid, type);
    return RefreshActivityInner(callTimeMs, type, needChangeBacklight);
}

bool PowerMgrService::RefreshActivityInner(int64_t callTimeMs, UserActivityType type, bool needChangeBacklight)
{
    std::lock_guard lock(screenMutex_);
    if (powerStateMachine_->CheckRefreshTime()) {
        return false;
    }
    pid_t pid = IPCSkeleton::GetCallingPid();
    powerStateMachine_->RefreshActivityInner(pid, callTimeMs, type, needChangeBacklight);
    return true;
}

PowerErrors PowerMgrService::OverrideScreenOffTime(int64_t timeout, const std::string& apiVersion)
{
    std::lock_guard lock(screenMutex_);
    pid_t pid = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    if (!Permission::IsSystem()) {
        POWER_HILOGI(COMP_SVC, "OverrideScreenOffTime failed, System permission intercept");
        return PowerErrors::ERR_SYSTEM_API_DENIED;
    }
    int32_t version = static_cast<int32_t>(strtol(apiVersion.c_str(), nullptr, 10));
    if (version >= API19 && !Permission::IsPermissionGranted("ohos.permission.POWER_MANAGER")) {
        POWER_HILOGI(FEATURE_SUSPEND, "OverrideScreenOffTime failed, The application does not have the permission");
        return PowerErrors::ERR_PERMISSION_DENIED;
    }
    POWER_HILOGI(COMP_SVC,
        "Try to override screenOffTime, timeout=%{public}" PRId64 ", pid: %{public}d, uid: %{public}d",
        timeout, pid, uid);
    return powerStateMachine_->OverrideScreenOffTimeInner(timeout) ?
        PowerErrors::ERR_OK : PowerErrors::ERR_FAILURE;
}

PowerErrors PowerMgrService::RestoreScreenOffTime(const std::string& apiVersion)
{
    std::lock_guard lock(screenMutex_);
    if (!Permission::IsSystem()) {
        POWER_HILOGI(COMP_SVC, "RestoreScreenOffTime failed, System permission intercept");
        return PowerErrors::ERR_SYSTEM_API_DENIED;
    }
    int32_t version = static_cast<int32_t>(strtol(apiVersion.c_str(), nullptr, 10));
    if (version >= API19 && !Permission::IsPermissionGranted("ohos.permission.POWER_MANAGER")) {
        POWER_HILOGI(FEATURE_SUSPEND, "RestoreScreenOffTime failed, The application does not have the permission");
        return PowerErrors::ERR_PERMISSION_DENIED;
    }
    POWER_HILOGD(COMP_SVC, "Try to restore screen off time");
    return powerStateMachine_->RestoreScreenOffTimeInner() ?
        PowerErrors::ERR_OK : PowerErrors::ERR_FAILURE;
}

PowerState PowerMgrService::GetState()
{
    std::lock_guard lock(stateMutex_);
    auto state = powerStateMachine_->GetState();
    POWER_HILOGD(FEATURE_POWER_STATE, "state: %{public}d", state);
    return state;
}

bool PowerMgrService::IsScreenOn(bool needPrintLog)
{
    std::lock_guard lock(stateMutex_);
    auto isScreenOn = powerStateMachine_->IsScreenOn();
    if (needPrintLog) {
        POWER_HILOGD(COMP_SVC, "isScreenOn: %{public}d", isScreenOn);
    }
    return isScreenOn;
}

bool PowerMgrService::IsFoldScreenOn()
{
    std::lock_guard lock(stateMutex_);
    auto isFoldScreenOn = powerStateMachine_->IsFoldScreenOn();
    POWER_HILOGI(COMP_SVC, "isFoldScreenOn: %{public}d", isFoldScreenOn);
    return isFoldScreenOn;
}

bool PowerMgrService::IsCollaborationScreenOn()
{
    std::lock_guard lock(stateMutex_);
    auto isCollaborationScreenOn = powerStateMachine_->IsCollaborationScreenOn();
    POWER_HILOGI(COMP_SVC, "isCollaborationScreenOn: %{public}d", isCollaborationScreenOn);
    return isCollaborationScreenOn;
}

bool PowerMgrService::IsForceSleeping()
{
    auto suspendController = pms->GetSuspendController();
    if (suspendController == nullptr) {
        POWER_HILOGE(COMP_SVC, "get suspendController fail");
        return false;
    }

    bool isForceSleeping = false;
#ifdef POWER_MANAGER_ENABLE_FORCE_SLEEP_BROADCAST
    isForceSleeping = suspendController->GetForceSleepingFlag();
#endif
    POWER_HILOGD(COMP_SVC, "isForceSleeping: %{public}d", isForceSleeping);
    return isForceSleeping;
}

PowerErrors PowerMgrService::ForceSuspendDevice(int64_t callTimeMs, const std::string& apiVersion)
{
    std::lock_guard lock(suspendMutex_);
    pid_t pid = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    if (!Permission::IsSystem()) {
        POWER_HILOGI(FEATURE_SUSPEND, "ForceSuspendDevice failed, System permission intercept");
        return PowerErrors::ERR_SYSTEM_API_DENIED;
    }
    int32_t version = static_cast<int32_t>(strtol(apiVersion.c_str(), nullptr, 10));
    if (version >= API19 && !Permission::IsPermissionGranted("ohos.permission.POWER_MANAGER")) {
        POWER_HILOGI(FEATURE_SUSPEND, "ForceSuspendDevice failed, The application does not have the permission");
        return PowerErrors::ERR_PERMISSION_DENIED;
    }
    if (shutdownController_->IsShuttingDown()) {
        POWER_HILOGI(FEATURE_SUSPEND, "System is shutting down, can't force suspend");
        return PowerErrors::ERR_FAILURE;
    }
#ifdef HAS_HIVIEWDFX_HISYSEVENT_PART
    powerStateMachine_->ReportSuspendStart(
        uid, static_cast<int32_t>(SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION), true);
#endif
    POWER_HILOGI(FEATURE_SUSPEND, "[UL_POWER] Try to force suspend device, pid: %{public}d, uid: %{public}d", pid, uid);
#ifdef POWER_MANAGER_ENABLE_CHARGING_TYPE_SETTING
    if (suspendController_) {
        suspendController_->StopSleep();
    }
#endif
    powerStateMachine_->ForceSuspendDeviceInner(pid, callTimeMs);
    return PowerErrors::ERR_OK;
}

PowerErrors PowerMgrService::Hibernate(bool clearMemory, const std::string& reason, const std::string& apiVersion)
{
    POWER_HILOGI(FEATURE_SUSPEND, "power mgr service hibernate begin.");
    if (!Permission::IsSystem()) {
        POWER_HILOGI(FEATURE_SUSPEND, "Hibernate failed, System permission intercept");
        return PowerErrors::ERR_SYSTEM_API_DENIED;
    }
    int32_t version = static_cast<int32_t>(strtol(apiVersion.c_str(), nullptr, 10));
    if (version >= API19 && !Permission::IsPermissionGranted("ohos.permission.POWER_MANAGER")) {
        POWER_HILOGI(FEATURE_SUSPEND, "Hibernate failed, The application does not have the permission");
        return PowerErrors::ERR_PERMISSION_DENIED;
    }
#ifdef POWER_MANAGER_POWER_ENABLE_S4
    std::lock_guard lock(hibernateMutex_);
    pid_t pid = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    if (shutdownController_->IsShuttingDown()) {
        POWER_HILOGI(FEATURE_SUSPEND, "System is shutting down, can't hibernate");
        return PowerErrors::ERR_FAILURE;
    }
    POWER_HILOGI(FEATURE_SUSPEND,
        "[UL_POWER] Try to hibernate, pid: %{public}d, uid: %{public}d, clearMemory: %{public}d, reason: %{public}s",
        pid, uid, static_cast<int>(clearMemory), reason.c_str());
#ifdef POWER_MANAGER_ENABLE_CHARGING_TYPE_SETTING
    if (suspendController_) {
        suspendController_->StopSleep();
    }
#endif
    BackgroundRunningLock hibernateGuard(
        "hibernateGuard", HIBERNATE_GUARD_TIMEOUT_MS); // avoid hibernate breaked by S3/ULSR
    HibernateControllerInit();
#ifdef HAS_HIVIEWDFX_HISYSEVENT_PART
    powerStateMachine_->ReportHibernateStart(
        static_cast<int32_t>(uid), reason, clearMemory);
#endif
    bool ret = powerStateMachine_->HibernateInner(clearMemory, reason);
    return ret ? PowerErrors::ERR_OK : PowerErrors::ERR_FAILURE;
#else
    POWER_HILOGI(FEATURE_SUSPEND, "Hibernate interface not supported.");
    return PowerErrors::ERR_FAILURE;
#endif
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
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "get B for U=%{public}d,Err:%{public}d",
            uid, static_cast<int32_t>(res));
    }
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "U=%{public}d,B=%{public}s", uid, tempBundleName.c_str());
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

bool PowerMgrService::ReleaseRunningLock(const sptr<IRemoteObject>& remoteObj, const std::string& name)
{
    std::lock_guard lock(lockMutex_);
    bool result = false;
    if (!Permission::IsPermissionGranted("ohos.permission.RUNNING_LOCK")) {
        return result;
    }

    result = runningLockMgr_->ReleaseLock(remoteObj, name);
    return result;
}

bool PowerMgrService::IsDeviceSupportedTypeUserIdle()
{
#ifdef POWER_MANAGER_RUNNINGLOCK_BACKGROUND_USER_IDLE_PERMISSION_PERMISSIVE_MODE
    return true;
#else
    return false;
#endif
}

bool PowerMgrService::IsRunningLockTypeSupported(RunningLockType type)
{
    if (Permission::IsHap()) {
        if (Permission::IsSystem()) {
            return type <= RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL ||
                type == RunningLockType::RUNNINGLOCK_BACKGROUND_USER_IDLE;
        }
        return type == RunningLockType::RUNNINGLOCK_BACKGROUND ||
            type == RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL ||
            (type == RunningLockType::RUNNINGLOCK_BACKGROUND_USER_IDLE && IsDeviceSupportedTypeUserIdle());
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

bool PowerMgrService::UpdateWorkSource(const sptr<IRemoteObject>& remoteObj,
    const std::vector<int32_t>& workSources)
{
    if (!Permission::IsPermissionGranted("ohos.permission.RUNNING_LOCK")) {
        return false;
    }
    std::map<int32_t, std::string> wks;
    for (const auto& uid : workSources) {
        std::string bundleName = GetBundleNameByUid(uid);
        wks.emplace(std::make_pair(uid, bundleName));
    }
    std::lock_guard lock(lockMutex_);
    runningLockMgr_->UpdateWorkSource(remoteObj, wks);
    return true;
}

PowerErrors PowerMgrService::Lock(const sptr<IRemoteObject>& remoteObj, int32_t timeOutMs)
{
#ifdef HAS_HIVIEWDFX_HISYSEVENT_PART
    constexpr int32_t RUNNINGLOCK_TIMEOUT_MS = 50;
    int32_t beginTimeMs = GetTickCount();
    pid_t pid = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
#endif
    if (!Permission::IsPermissionGranted("ohos.permission.RUNNING_LOCK")) {
        return PowerErrors::ERR_PERMISSION_DENIED;
    }
    std::lock_guard lock(lockMutex_);
    runningLockMgr_->Lock(remoteObj);
    if (timeOutMs > 0) {
        std::function<void()> task = [this, remoteObj, timeOutMs]() {
            std::lock_guard lock(lockMutex_);
            RunningLockTimerHandler::GetInstance().UnregisterRunningLockTimer(remoteObj);
            runningLockMgr_->UpdateWorkSource(remoteObj, {});
            runningLockMgr_->UnLock(remoteObj);
        };
        RunningLockTimerHandler::GetInstance().RegisterRunningLockTimer(remoteObj, task, timeOutMs);
    }
#ifdef HAS_HIVIEWDFX_HISYSEVENT_PART
    int32_t endTimeMs = GetTickCount();
    if (endTimeMs - beginTimeMs > RUNNINGLOCK_TIMEOUT_MS) {
        POWER_HILOGI(FEATURE_RUNNING_LOCK, "Lock interface timeout=%{public}d", (endTimeMs - beginTimeMs));
        HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::POWER, "INTERFACE_CONSUMING_TIMEOUT",
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR, "PID", pid, "UID", uid, "TYPE",
            static_cast<int32_t>(InterfaceTimeoutType::INTERFACE_TIMEOUT_TYPE_RUNNINGLOCK_LOCK), "REASON", "",
            "TIME", (endTimeMs - beginTimeMs));
    }
#endif
    return PowerErrors::ERR_OK;
}

PowerErrors PowerMgrService::UnLock(const sptr<IRemoteObject>& remoteObj, const std::string& name)
{
#ifdef HAS_HIVIEWDFX_HISYSEVENT_PART
    constexpr int32_t RUNNINGLOCK_TIMEOUT_MS = 50;
    int32_t beginTimeMs = GetTickCount();
    pid_t pid = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
#endif
    if (!Permission::IsPermissionGranted("ohos.permission.RUNNING_LOCK")) {
        return PowerErrors::ERR_PERMISSION_DENIED;
    }
    std::lock_guard lock(lockMutex_);
    RunningLockTimerHandler::GetInstance().UnregisterRunningLockTimer(remoteObj);
    runningLockMgr_->UnLock(remoteObj, name);
#ifdef HAS_HIVIEWDFX_HISYSEVENT_PART
    int32_t endTimeMs = GetTickCount();
    if (endTimeMs - beginTimeMs > RUNNINGLOCK_TIMEOUT_MS) {
        POWER_HILOGI(FEATURE_RUNNING_LOCK, "UnLock interface timeout=%{public}d", (endTimeMs - beginTimeMs));
        HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::POWER, "INTERFACE_CONSUMING_TIMEOUT",
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR, "PID", pid, "UID", uid, "TYPE",
            static_cast<int32_t>(InterfaceTimeoutType::INTERFACE_TIMEOUT_TYPE_RUNNINGLOCK_UNLOCK), "REASON", "",
            "TIME", (endTimeMs - beginTimeMs));
    }
#endif
    return PowerErrors::ERR_OK;
}

bool PowerMgrService::QueryRunningLockLists(std::map<std::string, RunningLockInfo>& runningLockLists)
{
    std::lock_guard lock(lockMutex_);
    if (!Permission::IsPermissionGranted("ohos.permission.RUNNING_LOCK")) {
        return false;
    }
    QueryRunningLockListsInner(runningLockLists);
    return true;
}

void PowerMgrService::QueryRunningLockListsInner(std::map<std::string, RunningLockInfo>& runningLockLists)
{
    runningLockMgr_->QueryRunningLockLists(runningLockLists);
}

bool PowerMgrService::IsExistAudioStream(pid_t uid)
{
    std::lock_guard lock(lockMutex_);
    return runningLockMgr_->IsExistAudioStream(uid);
}

bool PowerMgrService::RegisterRunningLockCallback(const sptr<IPowerRunninglockCallback>& callback)
{
    std::lock_guard lock(lockMutex_);
    pid_t pid = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    if (!Permission::IsPermissionGranted("ohos.permission.POWER_MANAGER")) {
        return false;
    }
    POWER_HILOGI(FEATURE_RUNNING_LOCK, "%{public}s: pid: %{public}d, uid: %{public}d", __func__, pid, uid);
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
    POWER_HILOGI(FEATURE_RUNNING_LOCK, "%{public}s: pid: %{public}d, uid: %{public}d", __func__, pid, uid);
    runningLockMgr_->RegisterRunningLockCallback(callback);
    return true;
}

void PowerMgrService::ForceUnLock(const sptr<IRemoteObject>& remoteObj)
{
    PowerXCollie powerXCollie("PowerMgrService::ForceUnLock", true);
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

bool PowerMgrService::RegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback, bool isSync)
{
    std::lock_guard lock(stateMutex_);
    pid_t pid = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    if (!Permission::IsPermissionGranted("ohos.permission.POWER_MANAGER")) {
        return false;
    }
    POWER_HILOGI(FEATURE_POWER_STATE, "%{public}s: pid: %{public}d, uid: %{public}d, isSync: %{public}u", __func__, pid,
        uid, static_cast<uint32_t>(isSync));
    powerStateMachine_->RegisterPowerStateCallback(callback, isSync);
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
    POWER_HILOGI(FEATURE_POWER_STATE, "%{public}s: pid: %{public}d, uid: %{public}d", __func__, pid, uid);
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
    POWER_HILOGI(FEATURE_SUSPEND, "%{public}s: pid: %{public}d, uid: %{public}d", __func__, pid, uid);
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
    POWER_HILOGI(FEATURE_SUSPEND, "%{public}s: pid: %{public}d, uid: %{public}d", __func__, pid, uid);
    suspendController_->RemoveCallback(callback);
    return true;
}

bool PowerMgrService::RegisterSuspendTakeoverCallback(
    const sptr<ITakeOverSuspendCallback>& callback, TakeOverSuspendPriority priority)
{
#ifdef POWER_MANAGER_TAKEOVER_SUSPEND
    std::lock_guard lock(suspendMutex_);
    pid_t pid = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    if (!Permission::IsPermissionGranted("ohos.permission.POWER_MANAGER")) {
        POWER_HILOGE(FEATURE_SUSPEND, "Permission deny");
        return false;
    }
    POWER_HILOGI(FEATURE_SUSPEND, "Func %{public}s: pid: %{public}d, uid: %{public}d", __func__, pid, uid);
    if (suspendController_ == nullptr) {
        POWER_HILOGE(FEATURE_SUSPEND, "suspendController_ is nullptr");
        return false;
    }
    suspendController_->AddCallback(callback, priority);
    return true;
#else
    return true;
#endif
}

bool PowerMgrService::UnRegisterSuspendTakeoverCallback(const sptr<ITakeOverSuspendCallback>& callback)
{
#ifdef POWER_MANAGER_TAKEOVER_SUSPEND
    std::lock_guard lock(suspendMutex_);
    pid_t pid = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    if (!Permission::IsPermissionGranted("ohos.permission.POWER_MANAGER")) {
        POWER_HILOGE(FEATURE_SUSPEND, "Permission deny");
        return false;
    }
    POWER_HILOGI(FEATURE_SUSPEND, "Func %{public}s: pid: %{public}d, uid: %{public}d", __func__, pid, uid);
    suspendController_->RemoveCallback(callback);
    return true;
#else
    return true;
#endif
}

bool PowerMgrService::RegisterSyncHibernateCallback(const sptr<ISyncHibernateCallback>& callback)
{
#ifdef POWER_MANAGER_POWER_ENABLE_S4
    POWER_HILOGI(FEATURE_SUSPEND, "RegisterSyncHibernateCallback begin.");
    std::lock_guard lock(hibernateMutex_);
    HibernateControllerInit();
    hibernateController_->RegisterSyncHibernateCallback(callback);
    return true;
#else
    POWER_HILOGI(FEATURE_SUSPEND, "RegisterSyncHibernateCallback interface not supported.");
    return false;
#endif
}

bool PowerMgrService::UnRegisterSyncHibernateCallback(const sptr<ISyncHibernateCallback>& callback)
{
#ifdef POWER_MANAGER_POWER_ENABLE_S4
    POWER_HILOGI(FEATURE_SUSPEND, "UnRegisterSyncHibernateCallback begin.");
    std::lock_guard lock(hibernateMutex_);
    HibernateControllerInit();
    hibernateController_->UnregisterSyncHibernateCallback(callback);
    return true;
#else
    POWER_HILOGI(FEATURE_SUSPEND, "UnRegisterSyncHibernateCallback interface not supported.");
    return false;
#endif
}

PowerErrors PowerMgrService::RegisterUlsrCallback(const sptr<IAsyncUlsrCallback>& callback)
{
    if (!Permission::IsSystem()) {
        POWER_HILOGE(FEATURE_WAKEUP, "RegisterUlsrCallback failed, System permission intercept");
        return PowerErrors::ERR_SYSTEM_API_DENIED;
    }
#ifdef POWER_MANAGER_ENABLE_SUSPEND_WITH_TAG
    pid_t pid = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    POWER_HILOGI(FEATURE_WAKEUP, "RegisterUlsrCallback pid:%{public}d, uid:%{public}d", pid, uid);
    std::lock_guard lock(ulsrMutex_);
    if (ulsrCallbackHolder_ == nullptr) {
        ulsrCallbackHolder_ = new UlsrCallbackHolder();
    }
    ulsrCallbackHolder_->AddCallback(callback, std::make_pair(pid, uid));
    return PowerErrors::ERR_OK;
#else
    POWER_HILOGW(FEATURE_WAKEUP, "RegisterUlsrCallback interface not supported.");
    return PowerErrors::ERR_PARAM_INVALID;
#endif
}

PowerErrors PowerMgrService::UnRegisterUlsrCallback(const sptr<IAsyncUlsrCallback>& callback)
{
    if (!Permission::IsSystem()) {
        POWER_HILOGE(FEATURE_WAKEUP, "UnRegisterUlsrCallback failed, System permission intercept");
        return PowerErrors::ERR_SYSTEM_API_DENIED;
    }
#ifdef POWER_MANAGER_ENABLE_SUSPEND_WITH_TAG
    pid_t pid = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    POWER_HILOGI(FEATURE_WAKEUP, "UnRegisterUlsrCallback pid:%{public}d, uid:%{public}d", pid, uid);
    std::lock_guard lock(ulsrMutex_);
    if (ulsrCallbackHolder_ == nullptr) {
        ulsrCallbackHolder_ = new UlsrCallbackHolder();
    }
    ulsrCallbackHolder_->RemoveCallback(callback);
    return PowerErrors::ERR_OK;
#else
    POWER_HILOGW(FEATURE_WAKEUP, "UnRegisterUlsrCallback interface not supported.");
    return PowerErrors::ERR_PARAM_INVALID;
#endif
}

#ifdef POWER_MANAGER_ENABLE_SUSPEND_WITH_TAG
void PowerMgrService::TriggerUlsrWakeupCallback()
{
    std::lock_guard lock(ulsrMutex_);
    if (ulsrCallbackHolder_ == nullptr) {
        POWER_HILOGW(FEATURE_WAKEUP, "ulsrCallbackHolder null");
        return;
    }
    ulsrCallbackHolder_->WakeupNotify();
}
#endif

bool PowerMgrService::RegisterPowerModeCallback(const sptr<IPowerModeCallback>& callback)
{
    std::lock_guard lock(modeMutex_);
    pid_t pid = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    if (!Permission::IsSystem()) {
        return false;
    }
    POWER_HILOGI(FEATURE_POWER_MODE, "%{public}s: pid: %{public}d, uid: %{public}d", __func__, pid, uid);
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
    POWER_HILOGI(FEATURE_POWER_MODE, "%{public}s: pid: %{public}d, uid: %{public}d", __func__, pid, uid);
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
    POWER_HILOGI(FEATURE_SCREEN_OFF_PRE, "%{public}s: pid: %{public}d, uid: %{public}d", __func__, pid, uid);
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
    POWER_HILOGI(FEATURE_SCREEN_OFF_PRE, "%{public}s: pid: %{public}d, uid: %{public}d", __func__, pid, uid);
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
    POWER_HILOGI(FEATURE_SUSPEND, "%{public}s: pid: %{public}d, uid: %{public}d, enable: %{public}d",
        __func__, pid, uid, enable);
    powerStateMachine_->SetDisplaySuspend(enable);
    return true;
}

PowerErrors PowerMgrService::SetDeviceMode(const PowerMode& mode)
{
    std::lock_guard lock(modeMutex_);
    pid_t pid = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    POWER_HILOGI(FEATURE_POWER_MODE, "%{public}s: pid: %{public}d, uid: %{public}d, mode: %{public}u",
        __func__, pid, uid, mode);
    if (!Permission::IsSystem()) {
        return PowerErrors::ERR_SYSTEM_API_DENIED;
    }
    if (!Permission::IsPermissionGranted("ohos.permission.POWER_OPTIMIZATION")) {
        return PowerErrors::ERR_PERMISSION_DENIED;
    }
    return powerModeModule_.SetModeItem(mode);
}

PowerMode PowerMgrService::GetDeviceMode()
{
    std::lock_guard lock(modeMutex_);
    pid_t pid = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    auto mode = powerModeModule_.GetModeItem();
    POWER_HILOGI(FEATURE_POWER_MODE, "%{public}s: pid: %{public}d, uid: %{public}d, mode: %{public}u",
        __func__, pid, uid, mode);
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

    std::string result;
    bool ret = PowerMgrDumper::Dump(args, result);
    POWER_HILOGI(COMP_SVC, "%{public}s: pid: %{public}d, uid: %{public}d ret :%{public}d", __func__, pid, uid, ret);
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

PowerMgrService::BackgroundRunningLock::BackgroundRunningLock(std::string name, int32_t timeOutMs)
{
    token_ = new (std::nothrow) RunningLockTokenStub();
    if (token_ == nullptr) {
        POWER_HILOGE(COMP_SVC, "create runninglock token failed");
        return;
    }
    RunningLockInfo info = {name, OHOS::PowerMgr::RunningLockType::RUNNINGLOCK_BACKGROUND};
    pms->CreateRunningLock(token_, info);
    pms->Lock(token_, timeOutMs);
}

PowerMgrService::BackgroundRunningLock::~BackgroundRunningLock()
{
    if (token_ == nullptr) {
        POWER_HILOGE(COMP_SVC, "token_ is nullptr");
        return;
    }
    pms->ReleaseRunningLock(token_);
}

void PowerMgrService::SuspendControllerInit()
{
    if (!suspendController_) {
        suspendController_ = std::make_shared<SuspendController>(shutdownController_, powerStateMachine_, ffrtTimer_);
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

#ifdef POWER_MANAGER_POWER_ENABLE_S4
void PowerMgrService::HibernateControllerInit()
{
    if (!hibernateController_) {
        hibernateController_ = std::make_shared<HibernateController>();
    }
}
#endif

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

#ifdef POWER_MANAGER_ENABLE_CHARGING_TYPE_SETTING
void PowerMgrService::PowerConnectStatusInit()
{
    auto pluggedType = BatterySrvClient::GetInstance().GetPluggedType();
    if (pluggedType == BatteryPluggedType::PLUGGED_TYPE_BUTT) {
        POWER_HILOGE(COMP_SVC, "BatterySrvClient GetPluggedType error");
        SetPowerConnectStatus(PowerConnectStatus::POWER_CONNECT_INVALID);
    } else if ((pluggedType == BatteryPluggedType::PLUGGED_TYPE_AC) ||
        (pluggedType == BatteryPluggedType::PLUGGED_TYPE_USB) ||
        (pluggedType == BatteryPluggedType::PLUGGED_TYPE_WIRELESS)) {
        SetPowerConnectStatus(PowerConnectStatus::POWER_CONNECT_AC);
    } else {
        SetPowerConnectStatus(PowerConnectStatus::POWER_CONNECT_DC);
    }
}

bool PowerMgrService::IsPowerConnected()
{
    if (GetPowerConnectStatus() == PowerConnectStatus::POWER_CONNECT_INVALID) {
        PowerConnectStatusInit(); // try to init again if invalid
    }
    return GetPowerConnectStatus() == PowerConnectStatus::POWER_CONNECT_AC;
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
    return PowerErrors::ERR_FAILURE;
#else
    return PowerErrors::ERR_OK;
#endif
}

void PowerMgrService::InvokerDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    POWER_HILOGI(COMP_SVC, "OnRemoteDied Called");
    if (!remote.promote()) {
        POWER_HILOGI(COMP_SVC, "proxy no longer exists, return early");
        return;
    }
    POWER_HILOGI(COMP_SVC, "the last client using %{public}s has died", interfaceName_.c_str());
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (!pms) {
        POWER_HILOGE(COMP_SVC, "cannot get PowerMgrService, return early");
        return;
    }
    callback_(pms);
}

PowerErrors PowerMgrService::SetForceTimingOut(bool enabled, const sptr<IRemoteObject>& token)
{
    static sptr<IRemoteObject> thisInterfaceInvoker = nullptr;
    static std::mutex localMutex;
    static sptr<InvokerDeathRecipient> drt =
        sptr<InvokerDeathRecipient>::MakeSptr(__func__, [](const sptr<PowerMgrService>& pms) {
            auto stateMachine = pms->GetPowerStateMachine();
            if (!stateMachine) {
                POWER_HILOGE(COMP_SVC, "cannot get PowerStateMachine, return early");
                return;
            }
            stateMachine->SetForceTimingOut(false);
            POWER_HILOGI(COMP_SVC, "the variables related to SetForceTimingOut has been reset");
        });

    if (!Permission::IsSystem()) {
        return PowerErrors::ERR_SYSTEM_API_DENIED;
    }

    // even if drt is nullptr(unlikely), it will be checked in IPCObjectProxy::SendObituary()
    localMutex.lock();
    if (token && token->IsProxyObject() && token != thisInterfaceInvoker) {
        // The localMutex only ensures that the "remove, assign, add" actions for THIS drt are thread safe.
        // AddDeathRecipient/RemoveDeathRecipient are thread safe theirselves.
        // Different remote objects(invokers) do not interfere wich each other
        // Different DeathRecipients for the same invoker do not interfere wich each other
        // Only one RemoteObject may hold the death recipient defined in this method and only once.
        if (thisInterfaceInvoker) {
            thisInterfaceInvoker->RemoveDeathRecipient(drt);
        } // removed from the old invoker
        thisInterfaceInvoker = token;
        thisInterfaceInvoker->AddDeathRecipient(drt); // added to the new invoker
    }
    localMutex.unlock();
    powerStateMachine_->SetForceTimingOut(enabled);
    return PowerErrors::ERR_OK;
}

PowerErrors PowerMgrService::LockScreenAfterTimingOutWithAppid(
    pid_t appid, bool lockScreen, const sptr<IRemoteObject>& token)
{
    if (!Permission::IsSystem()) {
        return PowerErrors::ERR_SYSTEM_API_DENIED;
    }
    if (!Permission::IsPermissionGranted("ohos.permission.TIMEOUT_SCREENOFF_DISABLE_LOCK")) {
        return PowerErrors::ERR_PERMISSION_DENIED;
    }
    POWER_HILOGI(COMP_SVC,
        "LockScreenAfterTimingOutWithAppid called, appid:%{public}d, lockScreen:%{public}d, remotePid:%{public}d",
        appid, lockScreen, IPCSkeleton::GetCallingPid());
    if (lockScreen) {
        return LockScreenAfterTimingOut(true, false, true, token, appid);
    } else {
        return LockScreenAfterTimingOut(false, false, false, token, appid);
    }
}

PowerErrors PowerMgrService::LockScreenAfterTimingOut(
    bool enabledLockScreen, bool checkLock, bool sendScreenOffEvent, const sptr<IRemoteObject>& token, pid_t appid)
{
    constexpr std::bitset<MAX_PARAM_NUMBER> defaultParams = 0b101;
    constexpr size_t paramNumber = 3;
    constexpr size_t firtParamPos = 2;   // index of enabledLockScreen
    constexpr size_t secondParamPos = 1; // index of checkLock
    constexpr size_t thirdParamPos = 0;  // index of sendScreenOffEvent
    static sptr<MultiInvokerHelper> multiInvokerhelper = sptr<MultiInvokerHelper>::MakeSptr(
        paramNumber, defaultParams, [this](const std::bitset<MAX_PARAM_NUMBER>& input) {
            auto stateMachine = powerStateMachine_;
            if (!powerStateMachine_) {
                POWER_HILOGE(COMP_SVC, "%{public}s: powerstatemachine is nullptr", __func__);
                return;
            }
            powerStateMachine_->LockScreenAfterTimingOut(
                input[firtParamPos], input[secondParamPos], input[thirdParamPos]);
        });
    if (!Permission::IsSystem()) {
        return PowerErrors::ERR_SYSTEM_API_DENIED;
    }
    pid_t callingPid = IPCSkeleton::GetCallingPid();
    POWER_HILOGI(COMP_SVC,
        "LockScreenAfterTimingOut called, pid: %{public}d, input parameters: %{public}d, %{public}d, %{public}d",
        callingPid, enabledLockScreen, checkLock, sendScreenOffEvent);
    std::bitset<MAX_PARAM_NUMBER> input =
        MultiInvokerHelper::ToBitset(enabledLockScreen, checkLock, sendScreenOffEvent);
    multiInvokerhelper->Set(token, callingPid, appid, input);
    return PowerErrors::ERR_OK;
}

void PowerMgrService::SetEnableDoze(bool enable)
{
    auto stateMachine = pms->GetPowerStateMachine();
    if (stateMachine == nullptr) {
        return;
    }
    stateMachine->SetEnableDoze(enable);
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
    if (keyEvent->GetDeviceId() == COLLABORATION_REMOTE_DEVICE_ID &&
        stateMachine->IsRunningLockEnabled(RunningLockType::RUNNINGLOCK_COORDINATION) &&
        stateMachine->GetState() == PowerState::AWAKE) {
        stateMachine->SetState(PowerState::DIM, StateChangeReason::STATE_CHANGE_REASON_COORDINATION, true);
        POWER_HILOGD(FEATURE_INPUT, "remote key event in coordinated state, override screen off time");
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
    auto action = pointerEvent->GetPointerAction();
    if (action == PointerEvent::POINTER_ACTION_ENTER_WINDOW ||
        action == PointerEvent::POINTER_ACTION_LEAVE_WINDOW ||
        action == PointerEvent::POINTER_ACTION_PULL_IN_WINDOW ||
        action == PointerEvent::POINTER_ACTION_PULL_OUT_WINDOW) {
        return;
    }
    if (pointerEvent->GetDeviceId() == COLLABORATION_REMOTE_DEVICE_ID &&
        stateMachine->IsRunningLockEnabled(RunningLockType::RUNNINGLOCK_COORDINATION) &&
        stateMachine->GetState() == PowerState::AWAKE) {
        stateMachine->SetState(PowerState::DIM, StateChangeReason::STATE_CHANGE_REASON_COORDINATION, true);
        POWER_HILOGD(FEATURE_INPUT, "remote pointer event in coordinated state, override screen off time");
    }
}

void PowerMgrInputMonitor::OnInputEvent(std::shared_ptr<AxisEvent> axisEvent) const {};
#endif

#ifdef POWER_MANAGER_ENABLE_EXTERNAL_SCREEN_MANAGEMENT
void PowerMgrService::ExternalScreenInit()
{
    auto stateMachine = pms->GetPowerStateMachine();
    auto suspendController = pms->GetSuspendController();
    auto wakeupController = pms->GetWakeupController();
    if (stateMachine == nullptr || suspendController == nullptr || wakeupController == nullptr) {
        POWER_HILOGE(COMP_SVC, "Get important instance error");
        return;
    }

    std::vector<uint64_t> screenIds;
    auto ret = Rosen::ScreenManagerLite::GetInstance().GetPhysicalScreenIds(screenIds);
    if (ret != Rosen::DMError::DM_OK) {
        POWER_HILOGE(COMP_SVC, "Failed to get physical screen ids");
        return;
    }
    POWER_HILOGI(COMP_SVC, "Number of current physical screen is %{public}u", static_cast<uint32_t>(screenIds.size()));
    if (screenIds.size() <= 1) { // there's at least a main screen, we only care about external screen
        return;
    }

    bool isSwitchOpen = stateMachine->IsSwitchOpen();
    bool isScreenOn = stateMachine->IsScreenOn();
    stateMachine->SetExternalScreenNumber(static_cast<int32_t>(screenIds.size()) - 1);
    POWER_HILOGI(
        COMP_SVC, "Init external screen, isSwitchOpen: %{public}d, isScreenOn: %{public}d", isSwitchOpen, isScreenOn);
    if (isScreenOn) {
        wakeupController->PowerOnAllScreens(WakeupDeviceType::WAKEUP_DEVICE_EX_SCREEN_INIT);
        if (!isSwitchOpen) {
            suspendController->PowerOffInternalScreen(SuspendDeviceType::SUSPEND_DEVICE_REASON_SWITCH);
        }
    } else {
        suspendController->PowerOffAllScreens(SuspendDeviceType::SUSPEND_DEVICE_REASON_EX_SCREEN_INIT);
    }
}

void PowerMgrService::RegisterExternalScreenListener()
{
    auto ret = Rosen::DMError::DM_OK;
    if (externalScreenListener_ == nullptr) {
        externalScreenListener_ = sptr<ExternalScreenListener>::MakeSptr();
        ret = Rosen::ScreenManagerLite::GetInstance().RegisterScreenListener(externalScreenListener_);
        POWER_HILOGI(COMP_SVC, "Register external screen listener, ret: %{public}d", static_cast<int32_t>(ret));
#ifdef HAS_HIVIEWDFX_HISYSEVENT_PART
        if (ret != Rosen::DMError::DM_OK) {
            std::string eventReason = "Register external screen listener failed";
            HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::POWER, "ABNORMAL_FAULT",
                HiviewDFX::HiSysEvent::EventType::FAULT, "TYPE", "EXTERNAL_SCREEN", "REASON", eventReason);
        }
#endif
    }

    if (abnormalExScreenListener_ == nullptr) {
        abnormalExScreenListener_ = sptr<AbnormalExternalScreenConnectListener>::MakeSptr();
        ret = Rosen::ScreenManagerLite::GetInstance().RegisterAbnormalScreenConnectChangeListener(
            abnormalExScreenListener_);
        POWER_HILOGI(
            COMP_SVC, "Register abnormal external screen listener, ret: %{public}d", static_cast<int32_t>(ret));
#ifdef HAS_HIVIEWDFX_HISYSEVENT_PART
        if (ret != Rosen::DMError::DM_OK) {
            std::string eventReason = "Register abnormal external screen listener failed";
            HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::POWER, "ABNORMAL_FAULT",
                HiviewDFX::HiSysEvent::EventType::FAULT, "TYPE", "EXTERNAL_SCREEN", "REASON", eventReason);
        }
#endif
    }
}

void PowerMgrService::UnRegisterExternalScreenListener()
{
    auto ret = Rosen::DMError::DM_OK;
    if (externalScreenListener_ != nullptr) {
        ret = Rosen::ScreenManagerLite::GetInstance().UnregisterScreenListener(externalScreenListener_);
        externalScreenListener_ = nullptr;
        POWER_HILOGI(COMP_SVC, "Unregister external screen listener, ret: %{public}d", static_cast<int32_t>(ret));
    }

    if (abnormalExScreenListener_ != nullptr) {
        ret = Rosen::ScreenManagerLite::GetInstance().UnregisterAbnormalScreenConnectChangeListener(
            abnormalExScreenListener_);
        abnormalExScreenListener_ = nullptr;
        POWER_HILOGI(
            COMP_SVC, "Unregister abnormal external screen listener, ret: %{public}d", static_cast<int32_t>(ret));
    }
}

void PowerMgrService::ExternalScreenListener::OnConnect(uint64_t screenId)
{
    if (screenId >= VIRTUAL_SCREEN_START_ID) {
        POWER_HILOGI(
            COMP_SVC, "Ignore virtual screen connecting event, screenId: %{public}u", static_cast<uint32_t>(screenId));
        return;
    }
    auto powerStateMachine = pms->GetPowerStateMachine();
    auto suspendController = pms->GetSuspendController();
    auto wakeupController = pms->GetWakeupController();
    if (powerStateMachine == nullptr || suspendController == nullptr || wakeupController == nullptr) {
        POWER_HILOGE(COMP_SVC, "Get important instance error, screenId: %{public}u", static_cast<uint32_t>(screenId));
        return;
    }

    int32_t curExternalScreenNum = powerStateMachine->GetExternalScreenNumber() + 1;
    powerStateMachine->SetExternalScreenNumber(curExternalScreenNum);
    bool isSwitchOpen = powerStateMachine->IsSwitchOpen();
    bool isScreenOn = powerStateMachine->IsScreenOnAcqLock();
    POWER_HILOGI(COMP_SVC,
        "External screen is connected, screenId: %{public}u, externalScreenNumber: %{public}d, isSwitchOpen: "
        "%{public}d, isScreenOn: %{public}d",
        static_cast<uint32_t>(screenId), curExternalScreenNum, isSwitchOpen, isScreenOn);

    if (isSwitchOpen && isScreenOn) {
        wakeupController->PowerOnAllScreens(WakeupDeviceType::WAKEUP_DEVICE_SCREEN_CONNECT);
        pms->RefreshActivity(GetTickCount(), UserActivityType::USER_ACTIVITY_TYPE_CABLE, false);
    } else if (isSwitchOpen && !isScreenOn) {
        POWER_HILOGI(FEATURE_SUSPEND, "[UL_POWER] Skip wakeup by external screen connected when screen off");
    } else if (!isSwitchOpen && !isScreenOn) {
        suspendController->PowerOffAllScreens(SuspendDeviceType::SUSPEND_DEVICE_REASON_SWITCH);
    } else {
        if (curExternalScreenNum > 1) {
            // When the power state is ON and there are 2 external screens or more, power off closed internal screen
            POWER_HILOGI(FEATURE_SUSPEND, "[UL_POWER] Power on all screens except for the closed internal screen");
            wakeupController->PowerOnAllScreens(WakeupDeviceType::WAKEUP_DEVICE_SCREEN_CONNECT);
            suspendController->PowerOffInternalScreen(SuspendDeviceType::SUSPEND_DEVICE_REASON_SWITCH);
        }
    }
}

void PowerMgrService::ExternalScreenListener::OnDisconnect(uint64_t screenId)
{
    if (screenId >= VIRTUAL_SCREEN_START_ID) {
        POWER_HILOGI(COMP_SVC, "Ignore virtual screen disconnecting event, screenId: %{public}u",
            static_cast<uint32_t>(screenId));
        return;
    }
    auto powerStateMachine = pms->GetPowerStateMachine();
    auto suspendController = pms->GetSuspendController();
    if (powerStateMachine == nullptr || suspendController == nullptr) {
        POWER_HILOGE(COMP_SVC, "Get important instance error, screenId:%{public}u", static_cast<uint32_t>(screenId));
        return;
    }

    int32_t curExternalScreenNum = std::max(powerStateMachine->GetExternalScreenNumber() - 1, 0);
    powerStateMachine->SetExternalScreenNumber(curExternalScreenNum);
    bool isSwitchOpen = powerStateMachine->IsSwitchOpen();
    bool isScreenOn = powerStateMachine->IsScreenOn();
    POWER_HILOGI(COMP_SVC,
        "External screen is disconnected, screenId: %{public}u, externalScreenNumber: %{public}d, isSwitchOpen: "
        "%{public}d, isScreenOn: %{public}d",
        static_cast<uint32_t>(screenId), curExternalScreenNum, isSwitchOpen, isScreenOn);

    if (isSwitchOpen && isScreenOn) {
        pms->RefreshActivity(GetTickCount(), UserActivityType::USER_ACTIVITY_TYPE_CABLE, false);
    } else if (!isSwitchOpen && isScreenOn) {
        // When there's no external screen, we should suspend the device, oterwise do nothing
        if (curExternalScreenNum == 0) {
            POWER_HILOGI(
                FEATURE_SUSPEND, "[UL_POWER] Suspend device when external screen is disconnected and switch is closed");
            suspendController->ExecSuspendMonitorByReason(SuspendDeviceType::SUSPEND_DEVICE_REASON_SWITCH);
        } else {
            POWER_HILOGI(FEATURE_SUSPEND,
                "[UL_POWER] Refresh device rather than suspend device when there's still external screen");
            pms->RefreshActivity(GetTickCount(), UserActivityType::USER_ACTIVITY_TYPE_CABLE, false);
        }
    }
}

void PowerMgrService::AbnormalExternalScreenConnectListener::NotifyAbnormalScreenConnectChange(uint64_t screenId)
{
    if (screenId >= VIRTUAL_SCREEN_START_ID) {
        POWER_HILOGI(COMP_SVC, "Ignore virtual screen abnormal connecting event, screenId: %{public}u",
            static_cast<uint32_t>(screenId));
        return;
    }
    auto powerStateMachine = pms->GetPowerStateMachine();
    auto suspendController = pms->GetSuspendController();
    auto wakeupController = pms->GetWakeupController();
    if (powerStateMachine == nullptr || suspendController == nullptr || wakeupController == nullptr) {
        POWER_HILOGE(COMP_SVC, "Get important instance error, screenId: %{public}u", static_cast<uint32_t>(screenId));
        return;
    }

    bool isSwitchOpen = powerStateMachine->IsSwitchOpen();
    bool isScreenOn = powerStateMachine->IsScreenOn();
    POWER_HILOGI(COMP_SVC,
        "Received abnormal external screen connecting event, screenId: %{public}u, isSwitchOpen: %{public}d, "
        "isScreenOn: %{public}d",
        static_cast<uint32_t>(screenId), isSwitchOpen, isScreenOn);
    if (isScreenOn) {
        wakeupController->PowerOnAllScreens(WakeupDeviceType::WAKEUP_DEVICE_ABNORMAL_SCREEN_CONNECT);
        if (!isSwitchOpen) {
            suspendController->PowerOffInternalScreen(SuspendDeviceType::SUSPEND_DEVICE_REASON_SWITCH);
        }
    }
}
#endif

void PowerMgrService::SubscribeCommonEvent()
{
    using namespace OHOS::EventFwk;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_USER_SWITCHED);
#ifdef POWER_MANAGER_ENABLE_CHARGING_TYPE_SETTING
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_POWER_CONNECTED);
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_POWER_DISCONNECTED);
#endif
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetThreadMode(CommonEventSubscribeInfo::ThreadMode::COMMON);
    if (!subscriberPtr_) {
        subscriberPtr_ = std::make_shared<PowerCommonEventSubscriber>(subscribeInfo);
    }
    bool result = CommonEventManager::SubscribeCommonEvent(subscriberPtr_);
    if (!result) {
        POWER_HILOGE(COMP_SVC, "Subscribe COMMON_EVENT failed");
    }
    runningLockMgr_->SubscribeCommonEvent();
}

void PowerMgrService::UnregisterAllSettingObserver()
{
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        POWER_HILOGI(COMP_SVC, "get PowerMgrService fail");
        return;
    }
    auto stateMachine = pms->GetPowerStateMachine();
    auto suspendController = pms->GetSuspendController();
    auto wakeupController = pms->GetWakeupController();
    if (stateMachine == nullptr || suspendController == nullptr || wakeupController == nullptr) {
        POWER_HILOGE(COMP_SVC, "get stateMachine, suspendController or wakeupController fail");
        return;
    }

#ifdef POWER_DOUBLECLICK_ENABLE
    SettingHelper::UnregisterSettingWakeupDoubleObserver();
#endif
#ifdef POWER_PICKUP_ENABLE
    SettingHelper::UnregisterSettingWakeupPickupObserver();
#endif
    wakeupController->UnregisterSettingsObserver();
#ifdef POWER_MANAGER_ENABLE_CHARGING_TYPE_SETTING
    suspendController->UnregisterSettingsObserver();
    stateMachine->UnregisterDisplayOffTimeObserver();
#endif
}

void PowerMgrService::RegisterAllSettingObserver()
{
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        POWER_HILOGI(COMP_SVC, "get PowerMgrService fail");
        return;
    }

#ifdef POWER_DOUBLECLICK_ENABLE
    pms->RegisterSettingWakeupDoubleClickObservers();
#endif
#ifdef POWER_PICKUP_ENABLE
    pms->RegisterSettingWakeupPickupGestureObserver();
#endif
    pms->WakeupControllerInit();
#ifdef POWER_MANAGER_ENABLE_CHARGING_TYPE_SETTING
    pms->SuspendControllerInit();
    pms->UpdateSettingInvalidDisplayOffTime(); // update setting value if invalid before register
    auto stateMachine = pms->GetPowerStateMachine();
    if (stateMachine == nullptr) {
        POWER_HILOGE(COMP_SVC, "get PowerStateMachine fail");
        return;
    }
    stateMachine->RegisterDisplayOffTimeObserver();
#endif
}

int64_t PowerMgrService::GetSettingDisplayOffTime(int64_t defaultTime)
{
    int64_t settingTime = defaultTime;
#ifdef POWER_MANAGER_ENABLE_CHARGING_TYPE_SETTING
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        POWER_HILOGE(FEATURE_POWER_MODE, "get PowerMgrService fail");
        return settingTime;
    }
    if (pms->IsPowerConnected()) {
        settingTime = SettingHelper::GetSettingDisplayAcScreenOffTime(defaultTime);
    } else {
        settingTime = SettingHelper::GetSettingDisplayDcScreenOffTime(defaultTime);
    }
#else
    settingTime = SettingHelper::GetSettingDisplayOffTime(defaultTime);
#endif
    return settingTime;
}

#ifdef POWER_MANAGER_ENABLE_CHARGING_TYPE_SETTING
void PowerMgrService::UpdateSettingInvalidDisplayOffTime()
{
    if (!SettingHelper::IsSettingDisplayAcScreenOffTimeValid()) {
        SettingHelper::SetSettingDisplayAcScreenOffTime(PowerStateMachine::DEFAULT_AC_DISPLAY_OFF_TIME_MS);
    }
    if (!SettingHelper::IsSettingDisplayDcScreenOffTimeValid()) {
        SettingHelper::SetSettingDisplayDcScreenOffTime(PowerStateMachine::DEFAULT_DC_DISPLAY_OFF_TIME_MS);
    }
}

void PowerCommonEventSubscriber::OnPowerConnectStatusChanged(PowerConnectStatus status)
{
    auto power = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (power == nullptr) {
        POWER_HILOGE(COMP_SVC, "get PowerMgrService fail");
        return;
    }
    power->SetPowerConnectStatus(status);

    auto suspendController = power->GetSuspendController();
    if (suspendController == nullptr) {
        POWER_HILOGE(COMP_SVC, "get suspendController fail");
        return;
    }
    suspendController->UpdateSuspendSources();

    auto stateMachine = power->GetPowerStateMachine();
    if (stateMachine == nullptr) {
        POWER_HILOGE(COMP_SVC, "get PowerStateMachine fail");
        return;
    }
    stateMachine->DisplayOffTimeUpdateFunc();

    power->OnChargeStateChanged(); // should be called after suspend sources settings updated
}
#endif

void PowerCommonEventSubscriber::OnReceiveEvent(const OHOS::EventFwk::CommonEventData &data)
{
    std::string action = data.GetWant().GetAction();
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        POWER_HILOGI(COMP_SVC, "get PowerMgrService fail");
        return;
    }
    if (action == OHOS::EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED) {
        pms->UnregisterAllSettingObserver();    // unregister old user observer
        SettingHelper::UpdateCurrentUserId();   // update user Id
        pms->RegisterAllSettingObserver();      // register new user observer
#ifdef POWER_MANAGER_ENABLE_CHARGING_TYPE_SETTING
    } else if (action == OHOS::EventFwk::CommonEventSupport::COMMON_EVENT_POWER_CONNECTED) {
        OnPowerConnectStatusChanged(PowerConnectStatus::POWER_CONNECT_AC);
    } else if (action == OHOS::EventFwk::CommonEventSupport::COMMON_EVENT_POWER_DISCONNECTED) {
        OnPowerConnectStatusChanged(PowerConnectStatus::POWER_CONNECT_DC);
#endif
    }
}

PowerErrors PowerMgrService::IsRunningLockEnabled(const RunningLockType type, bool& result)
{
    if (!Permission::IsPermissionGranted("ohos.permission.RUNNING_LOCK")) {
        return PowerErrors::ERR_PERMISSION_DENIED;
    }
    std::lock_guard lock(lockMutex_);
    uint32_t num = runningLockMgr_->GetValidRunningLockNum(type);
    POWER_HILOGI(COMP_SVC, "Hold running lock type:%{public}u, num:%{public}u", static_cast<uint32_t>(type), num);
    result = num > 0 ? true : false;
    return PowerErrors::ERR_OK;
}

PowerErrors PowerMgrService::RefreshActivity(
    int64_t callTimeMs, UserActivityType type, const std::string& refreshReason)
{
    if (!Permission::IsSystem()) {
        POWER_HILOGI(FEATURE_ACTIVITY, "RefreshActivity failed, System permission intercept");
        return PowerErrors::ERR_SYSTEM_API_DENIED;
    }
    if (!Permission::IsPermissionGranted("ohos.permission.REFRESH_USER_ACTION")) {
        POWER_HILOGI(FEATURE_ACTIVITY, "RefreshActivity failed, The caller does not have the permission");
        return PowerErrors::ERR_PERMISSION_DENIED;
    }
    pid_t pid = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    POWER_HILOGI(FEATURE_ACTIVITY,
        "Try to refresh activity, pid: %{public}d, uid: %{public}d, activity type: %{public}u, reason: %{public}s",
        pid, uid, type, refreshReason.c_str());
    return RefreshActivityInner(callTimeMs, type, true) ? PowerErrors::ERR_OK :
        PowerErrors::ERR_FREQUENT_FUNCTION_CALL;
}

PowerErrors PowerMgrService::SetPowerKeyFilteringStrategy(PowerKeyFilteringStrategy strategy)
{
    if (!Permission::IsSystem()) {
        POWER_HILOGI(FEATURE_INPUT, "SetPowerKeyFilteringStrategy failed, System permission intercept");
        return PowerErrors::ERR_SYSTEM_API_DENIED;
    }
    if (!Permission::IsPermissionGranted("ohos.permission.POWER_MANAGER")) {
        POWER_HILOGI(FEATURE_INPUT, "SetPowerKeyFilteringStrategy failed, The caller does not have the permission");
        return PowerErrors::ERR_PERMISSION_DENIED;
    }
    pid_t pid = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    POWER_HILOGI(FEATURE_INPUT,
        "SetPowerKeyFilteringStrategy pid: %{public}d, uid: %{public}d, strategy: %{public}d", pid, uid, strategy);
    switch (strategy) {
        case PowerKeyFilteringStrategy::DISABLE_LONG_PRESS_FILTERING:
            shutdownDialog_.SetShutdownDialogForbid(false);
            break;
        case PowerKeyFilteringStrategy::LONG_PRESS_FILTERING_ONCE:
            shutdownDialog_.SetShutdownDialogForbid(true);
            break;
        default:
            POWER_HILOGW(FEATURE_INPUT, "SetPowerKeyFilteringStrategy out of range");
            break;
    }
    return PowerErrors::ERR_OK;
}

PowerErrors PowerMgrService::RegisterAsyncShutdownCallback(
    const sptr<IAsyncShutdownCallback>& callback, ShutdownPriority priority)
{
    if (callback == nullptr) {
        POWER_HILOGW(FEATURE_SHUTDOWN, "RegisterAsyncShutdownCallback callback is null");
        return PowerErrors::ERR_PARAM_INVALID;
    }
    
    if (!Permission::IsSystem()) {
        POWER_HILOGW(FEATURE_SHUTDOWN, "RegisterAsyncShutdownCallback failed, System permission intercept");
        return PowerErrors::ERR_SYSTEM_API_DENIED;
    }
    if (!Permission::IsPermissionGranted("ohos.permission.REBOOT")) {
        POWER_HILOGW(FEATURE_SHUTDOWN, "RegisterAsyncShutdownCallback failed, The caller does not have the permission");
        return PowerErrors::ERR_PERMISSION_DENIED;
    }
    pid_t pid = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    POWER_HILOGI(FEATURE_SHUTDOWN,
        "RegisterAsyncShutdownCallback pid: %{public}d, uid: %{public}d, priority: %{public}d", pid, uid, priority);
    std::lock_guard lock(shutdownMutex_);
    shutdownController_->AddCallback(callback, priority);
    return PowerErrors::ERR_OK;
}

PowerErrors PowerMgrService::UnRegisterAsyncShutdownCallback(const sptr<IAsyncShutdownCallback>& callback)
{
    if (callback == nullptr) {
        POWER_HILOGW(FEATURE_SHUTDOWN, "UnRegisterAsyncShutdownCallback callback is null");
        return PowerErrors::ERR_PARAM_INVALID;
    }
    
    if (!Permission::IsSystem()) {
        POWER_HILOGW(FEATURE_SHUTDOWN, "UnRegisterAsyncShutdownCallback failed, System permission intercept");
        return PowerErrors::ERR_SYSTEM_API_DENIED;
    }
    if (!Permission::IsPermissionGranted("ohos.permission.REBOOT")) {
        POWER_HILOGW(FEATURE_SHUTDOWN,
            "UnRegisterAsyncShutdownCallback failed, The caller does not have the permission");
        return PowerErrors::ERR_PERMISSION_DENIED;
    }
    pid_t pid = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    POWER_HILOGI(FEATURE_SHUTDOWN,
        "UnRegisterAsyncShutdownCallback pid: %{public}d, uid: %{public}d", pid, uid);

    std::lock_guard lock(shutdownMutex_);
    shutdownController_->RemoveCallback(callback);
    return PowerErrors::ERR_OK;
}
} // namespace PowerMgr
} // namespace OHOS