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

#include "wakeup_controller.h"

#include <datetime_ex.h>
#ifdef POWER_MANAGER_ENABLE_EXTERNAL_SCREEN_MANAGEMENT
#include <display_manager_lite.h>
#endif
#include <dlfcn.h>
#ifdef HAS_HIVIEWDFX_HISYSEVENT_PART
#include <hisysevent.h>
#endif
#include <input_manager.h>
#include <ipc_skeleton.h>
#include <json/json.h>
#include <securec.h>
#include "permission.h"
#include "power_errors.h"
#include "power_log.h"
#include "power_mgr_service.h"
#include "power_state_callback_stub.h"
#include "power_utils.h"
#include "setting_helper.h"
#include "suspend_controller.h"
#include "system_suspend_controller.h"
#include "customized_screen_event_rules.h"
#include "singleton.h"

namespace OHOS {
namespace PowerMgr {
using namespace OHOS::MMI;
namespace {
sptr<SettingObserver> g_wakeupSourcesKeyObserver = nullptr;
#ifdef POWER_DOUBLECLICK_ENABLE
const int32_t ERR_FAILED = -1;
#endif

constexpr int32_t WAKEUP_LOCK_TIMEOUT_MS = 5000;
constexpr int32_t COLLABORATION_REMOTE_DEVICE_ID = 0xAAAAAAFF;
constexpr int32_t OTHER_SYSTEM_DEVICE_ID = 0xAAAAAAFE;
}
std::mutex WakeupController::sourceUpdateMutex_;

/** WakeupController Implement */
WakeupController::WakeupController(std::shared_ptr<PowerStateMachine>& stateMachine)
{
    stateMachine_ = stateMachine;
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
    std::shared_ptr<InputCallback> callback = std::make_shared<InputCallback>();
    if (monitorId_ < 0) {
        monitorId_ = InputManager::GetInstance()->AddMonitor(std::static_pointer_cast<IInputEventConsumer>(callback));
    }
#endif
    eventHandleMap_.emplace(WakeupDeviceType::WAKEUP_DEVICE_KEYBOARD, 0);
    eventHandleMap_.emplace(WakeupDeviceType::WAKEUP_DEVICE_MOUSE, 0);
    eventHandleMap_.emplace(WakeupDeviceType::WAKEUP_DEVICE_TOUCHPAD, 0);
    eventHandleMap_.emplace(WakeupDeviceType::WAKEUP_DEVICE_PEN, 0);
    eventHandleMap_.emplace(WakeupDeviceType::WAKEUP_DEVICE_TOUCH_SCREEN, 0);
    eventHandleMap_.emplace(WakeupDeviceType::WAKEUP_DEVICE_SINGLE_CLICK, 0);
}

WakeupController::~WakeupController()
{
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
    InputManager* inputManager = InputManager::GetInstance();
    if (monitorId_ >= 0) {
        inputManager->RemoveMonitor(monitorId_);
    }
#endif
    UnregisterSettingsObserver();
}

void WakeupController::Init()
{
    std::lock_guard lock(monitorMutex_);
    std::shared_ptr<WakeupSources> sources = WakeupSourceParser::ParseSources();
    sourceList_ = sources->GetSourceList();
    if (sourceList_.empty()) {
        POWER_HILOGE(FEATURE_WAKEUP, "InputManager is null");
        return;
    }

    for (auto source = sourceList_.begin(); source != sourceList_.end(); source++) {
        POWER_HILOGI(FEATURE_WAKEUP, "registered type=%{public}u", (*source).GetReason());
        SetOriginSettingValue((*source));
        std::shared_ptr<WakeupMonitor> monitor = WakeupMonitor::CreateMonitor(*source);
        if (monitor != nullptr && monitor->Init()) {
            POWER_HILOGI(FEATURE_WAKEUP, "monitor init success, type=%{public}u", (*source).GetReason());
            monitor->RegisterListener([this](WakeupDeviceType reason) { this->ControlListener(reason); });
            monitorMap_.emplace(monitor->GetReason(), monitor);
        }
    }
    RegisterSettingsObserver();
}

void WakeupController::Cancel()
{
    for (auto monitor = monitorMap_.begin(); monitor != monitorMap_.end(); monitor++) {
        monitor->second->Cancel();
    }
    monitorMap_.clear();
}

void WakeupController::RegisterSettingsObserver()
{
    if (g_wakeupSourcesKeyObserver) {
        POWER_HILOGE(FEATURE_POWER_STATE, "wakeup sources key observer is already registered");
        return;
    }
    SettingObserver::UpdateFunc updateFunc = [&](const std::string&) {
        std::lock_guard lock(monitorMutex_);
        POWER_HILOGI(COMP_SVC, "start setting string update");
        std::string jsonStr = SettingHelper::GetSettingWakeupSources();
        std::shared_ptr<WakeupSources> sources = WakeupSourceParser::ParseSources(jsonStr);
        if (sources->GetParseErrorFlag()) {
            POWER_HILOGI(FEATURE_WAKEUP, "Parse failed, call GetWakeupSourcesByConfig again");
            sources = WakeupSourceParser::ParseSources(WakeupSourceParser::GetWakeupSourcesByConfig());
        }
        std::vector<WakeupSource> updateSourceList = sources->GetSourceList();
        if (updateSourceList.size() == 0) {
            return;
        }
        sourceList_ = updateSourceList;
        POWER_HILOGI(COMP_SVC, "start updateListener");
        Cancel();
        uint32_t id = 0;
        for (auto source = sourceList_.begin(); source != sourceList_.end(); source++, id++) {
            std::shared_ptr<WakeupMonitor> monitor = WakeupMonitor::CreateMonitor(*source);
            POWER_HILOGI(FEATURE_WAKEUP, "UpdateFunc CreateMonitor[%{public}u] reason=%{public}d",
                id, source->GetReason());
            if (monitor != nullptr && monitor->Init()) {
                monitor->RegisterListener([this](WakeupDeviceType reason) { this->ControlListener(reason); });
                monitorMap_.emplace(monitor->GetReason(), monitor);
            }
        }
    };
    g_wakeupSourcesKeyObserver = SettingHelper::RegisterSettingWakeupSourcesObserver(updateFunc);
    POWER_HILOGI(FEATURE_POWER_STATE, "register setting observer fin");
}

void WakeupController::UnregisterSettingsObserver()
{
    if (g_wakeupSourcesKeyObserver) {
        SettingHelper::UnregisterSettingObserver(g_wakeupSourcesKeyObserver);
        g_wakeupSourcesKeyObserver = nullptr;
    }
}

void WakeupController::SetOriginSettingValue(WakeupSource& source)
{
#ifdef POWER_DOUBLECLICK_ENABLE
    if (source.GetReason() == WakeupDeviceType::WAKEUP_DEVICE_DOUBLE_CLICK) {
        if (SettingHelper::IsWakeupDoubleSettingValid() == false) {
            POWER_HILOGI(COMP_SVC, "the origin doubleClick_enable is: %{public}d", source.IsEnable());
            SettingHelper::SetSettingWakeupDouble(source.IsEnable());
            SetWakeupDoubleClickSensor(source.IsEnable());
            return;
        }

        auto enable = SettingHelper::GetSettingWakeupDouble();
        SetWakeupDoubleClickSensor(enable);
    }
#endif
#ifdef POWER_PICKUP_ENABLE
    if (source.GetReason() == WakeupDeviceType::WAKEUP_DEVICE_PICKUP) {
        if (!SettingHelper::IsWakeupPickupSettingValid()) {
            POWER_HILOGI(FEATURE_WAKEUP, "GetReason_WAKEUP_DEVICE_PICKUP,source enable=%{public}d", source.IsEnable());
            SettingHelper::SetSettingWakeupPickup(source.IsEnable());
            PickupConnectMotionConfig(source.IsEnable());
            return;
        }

        auto enable = SettingHelper::GetSettingWakeupPickup();
        PickupConnectMotionConfig(enable);
    }
#endif
}
#ifdef POWER_DOUBLECLICK_ENABLE
void WakeupController::ChangeWakeupSourceConfig(bool updateEnable)
{
    std::lock_guard lock(sourceUpdateMutex_);
    std::string jsonStr = SettingHelper::GetSettingWakeupSources();
    if (jsonStr.empty()) {
        POWER_HILOGE(COMP_SVC, "there is no such configuration file available");
        return;
    }
    POWER_HILOGI(COMP_SVC, "the origin ccmJson is: %{public}s", jsonStr.c_str());
    Json::Value root;
    Json::Reader reader;
    if (!reader.parse(jsonStr.data(), jsonStr.data() + jsonStr.size(), root)) {
        POWER_HILOGE(COMP_SVC, "json parse error");
        return;
    }
    if (root["touchscreen"].isNull()) {
        POWER_HILOGE(COMP_SVC, "this touchscreenNode is empty");
        return;
    }
    if (root["touchscreen"]["enable"].isNull()) {
        POWER_HILOGE(COMP_SVC, "the touchscreenNode is empty");
        return;
    }
    if (!root["touchscreen"]["enable"].isBool()) {
        POWER_HILOGE(COMP_SVC, "the origin touchscreenEnable value is invalid");
        return;
    }
    bool originEnable = root["touchscreen"]["enable"].asBool();
    if (originEnable == updateEnable) {
        POWER_HILOGI(COMP_SVC, "no need change jsonConfig value");
        return;
    }

    root["touchscreen"]["enable"] = updateEnable;
    POWER_HILOGI(COMP_SVC, "the new doubleJsonConfig is: %{public}s", root.toStyledString().c_str());
    SettingHelper::SetSettingWakeupSources(root.toStyledString());
}

static const char* POWER_MANAGER_EXT_PATH = "libpower_manager_ext.z.so";
static const char* SET_WAKEUP_DOUBLE_CLICK_SENSOR = "SetWakeupDoubleClickSensor";
typedef int32_t(*Func)(bool);
int32_t WakeupController::SetWakeupDoubleClickSensor(bool enable)
{
    POWER_HILOGI(COMP_SVC, "enter SetWakeupDoubleClickSensor");
    void *handler = dlopen(POWER_MANAGER_EXT_PATH, RTLD_LAZY | RTLD_NODELETE);
    if (handler == nullptr) {
        POWER_HILOGE(COMP_SVC, "Dlopen failed, reason : %{public}s", dlerror());
        return ERR_FAILED;
    }

    Func PowerDoubleClickFlag = (Func)dlsym(handler, SET_WAKEUP_DOUBLE_CLICK_SENSOR);
    if (PowerDoubleClickFlag == nullptr) {
        POWER_HILOGE(COMP_SVC, "find function failed, reason : %{public}s", dlerror());
        dlclose(handler);
        return ERR_FAILED;
    }
    auto resCode = PowerDoubleClickFlag(enable);
    dlclose(handler);
    return resCode;
}
#endif
#ifdef POWER_PICKUP_ENABLE
static const char* SET_WAKEUP_MOTION_SUBSCRIBER_CONFIG = "PickupMotionSubscriber";
static const char* SET_WAKEUP_MOTION_UNSUBSCRIBER_CONFIG = "PickupMotionUnsubscriber";
typedef void(*FuncSubscriber)();
typedef void(*FuncUnsubscriber)();

void WakeupController::PickupConnectMotionConfig(bool databaseSwitchValue)
{
    POWER_HILOGI(COMP_SVC, "open enter PickupConnectMotionConfig");
    if (databaseSwitchValue) {
        void *subscriberHandler = dlopen(POWER_MANAGER_EXT_PATH, RTLD_LAZY | RTLD_NODELETE);
        if (subscriberHandler == nullptr) {
            POWER_HILOGE(COMP_SVC, "Dlopen failed, reason : %{public}s", dlerror());
            return;
        }
        FuncSubscriber powerPickupMotionSubscriberFlag = reinterpret_cast<FuncSubscriber>(dlsym(subscriberHandler,
            SET_WAKEUP_MOTION_SUBSCRIBER_CONFIG));
        if (powerPickupMotionSubscriberFlag == nullptr) {
            POWER_HILOGE(COMP_SVC, "find Subscriber function failed, reason : %{public}s", dlerror());
            dlclose(subscriberHandler);
            return;
        }
        powerPickupMotionSubscriberFlag();
        POWER_HILOGI(COMP_SVC, "powerservice enable powerPickupMotionSubscriberFlag isSettingEnable=%{public}d",
            databaseSwitchValue);
        dlclose(subscriberHandler);
        POWER_HILOGI(COMP_SVC, "open to close PickupMotionSubscriberConfig");
    } else {
        void *unsubscriberHandler = dlopen(POWER_MANAGER_EXT_PATH, RTLD_LAZY | RTLD_NODELETE);
        if (unsubscriberHandler == nullptr) {
            POWER_HILOGE(COMP_SVC, "Dlopen failed, reason : %{public}s", dlerror());
            return;
        }
        FuncUnsubscriber powerPickupMotionUnsubscriberFlag = reinterpret_cast<FuncUnsubscriber>(dlsym(
            unsubscriberHandler, SET_WAKEUP_MOTION_UNSUBSCRIBER_CONFIG));
        if (powerPickupMotionUnsubscriberFlag == nullptr) {
            POWER_HILOGE(COMP_SVC, "find Unsubscriber function failed, reason : %{public}s", dlerror());
            dlclose(unsubscriberHandler);
            return;
        }
        powerPickupMotionUnsubscriberFlag();
        POWER_HILOGI(COMP_SVC, "powerservice disable powerPickupMotionUnsubscriberFlag isSettingEnable=%{public}d",
            databaseSwitchValue);
        dlclose(unsubscriberHandler);
        POWER_HILOGI(COMP_SVC, "open to close PickupMotionSubscriberConfig");
    }
}

void WakeupController::ChangePickupWakeupSourceConfig(bool updataEnable)
{
    std::lock_guard lock(sourceUpdateMutex_);
    std::string jsonStr = SettingHelper::GetSettingWakeupSources();
    if (jsonStr.empty()) {
        POWER_HILOGE(COMP_SVC, "there is no such configuration file available");
        return;
    }
    POWER_HILOGI(COMP_SVC, "%{public}s(%{public}d)", __func__, updataEnable);
    Json::Value root;
    Json::Reader reader;
    reader.parse(jsonStr, root);
    if (!reader.parse(jsonStr, root)) {
        POWER_HILOGE(COMP_SVC, "Failed to parse json string");
        return;
    }
    if (root["pickup"].isNull()) {
        POWER_HILOGE(COMP_SVC, "this pickNode is empty");
        return;
    }
    if (root["pickup"]["enable"].isNull()) {
        POWER_HILOGE(COMP_SVC, "the pickupNode is empty");
        return;
    }
    if (!root["pickup"]["enable"].isBool()) {
        POWER_HILOGE(COMP_SVC, "the origin pickupEnable value is invalid");
        return;
    }
    bool originEnable = root["pickup"]["enable"].asBool();
    if (originEnable == updataEnable) {
        POWER_HILOGI(COMP_SVC, "no need change jsonconfig_value");
        return;
    }
    root["pickup"]["enable"] = updataEnable;
    POWER_HILOGI(COMP_SVC, "the new pickupJsonConfig is: %{public}s", root.toStyledString().c_str());
    SettingHelper::SetSettingWakeupSources(root.toStyledString());
}
#endif

void WakeupController::ChangeLidWakeupSourceConfig(bool updataEnable)
{
    std::lock_guard lock(sourceUpdateMutex_);
    std::string jsonStr = SettingHelper::GetSettingWakeupSources();
    POWER_HILOGI(FEATURE_POWER_STATE, "%{public}s", jsonStr.c_str());
    Json::Value root;
    Json::Reader reader;
    reader.parse(jsonStr, root);
    if (!reader.parse(jsonStr, root)) {
        POWER_HILOGE(FEATURE_POWER_STATE, "Failed to parse json string");
        return;
    }
    bool originEnable = true;
    if (root["lid"]["enable"].isBool()) {
        originEnable = root["lid"]["enable"].asBool();
    }

    if (originEnable == updataEnable) {
        POWER_HILOGI(FEATURE_POWER_STATE, "no need change jsonConfig value");
        return;
    }
    if (root["lid"]["enable"].isBool()) {
        root["lid"]["enable"] = updataEnable;
    }
    SettingHelper::SetSettingWakeupSources(root.toStyledString());
}


void WakeupController::ExecWakeupMonitorByReason(WakeupDeviceType reason)
{
    FFRTUtils::SubmitTask([this, reason] {
        std::lock_guard lock(monitorMutex_);
        if (monitorMap_.find(reason) != monitorMap_.end()) {
            auto monitor = monitorMap_[reason];
            monitor->Notify();
        }
    });
}

void WakeupController::Wakeup()
{
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        POWER_HILOGE(FEATURE_WAKEUP, "get powerMgrService instance error");
        return;
    }
    auto suspendController = pms->GetSuspendController();
    if (suspendController == nullptr) {
        POWER_HILOGE(FEATURE_WAKEUP, "get suspendController instance error");
        return;
    }
    suspendController->StopSleep();
}

WakeupController::SleepGuard::SleepGuard(const sptr<PowerMgrService>& pms) : pms_(pms)
{
    token_ = new (std::nothrow) RunningLockTokenStub();
    if (token_ == nullptr) {
        POWER_HILOGE(COMP_SVC, "create runninglock token failed");
        return;
    }
    RunningLockInfo info = {"SleepGuard", OHOS::PowerMgr::RunningLockType::RUNNINGLOCK_BACKGROUND_TASK};
    pms_->CreateRunningLock(token_, info);
    pms_->Lock(token_, WAKEUP_LOCK_TIMEOUT_MS);
}

WakeupController::SleepGuard::~SleepGuard()
{
    if (token_ == nullptr) {
        POWER_HILOGE(COMP_SVC, "dtor: token_ is nullptr, direct return ");
        return;
    }
    pms_->ReleaseRunningLock(token_);
}

#ifdef POWER_MANAGER_WAKEUP_ACTION
bool WakeupController::IsLowCapacityWakeup(WakeupDeviceType reason)
{
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        POWER_HILOGE(FEATURE_WAKEUP, "[UL_POWER] pms is nullptr");
        return false;
    }
    auto wakeupActionController = pms->GetWakeupActionController();
    if (wakeupActionController == nullptr) {
        POWER_HILOGE(FEATURE_WAKEUP, "[UL_POWER] wakeupActionController is nullptr.");
        return false;
    }
    return (reason == WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON) &&
        (wakeupActionController->IsLowCapacityWakeup());
}

void WakeupController::ProcessLowCapacityWakeup()
{
    POWER_HILOGI(FEATURE_WAKEUP, "[UL_POWER] processing low capacity wake up begins.");
    if (stateMachine_->GetState() != PowerState::SLEEP) {
        POWER_HILOGE(FEATURE_WAKEUP, "[UL_POWER] the current power state is not sleep.");
        return;
    }
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        POWER_HILOGE(FEATURE_WAKEUP, "[UL_POWER] pms is nullptr");
        return;
    }
    auto wakeupActionController = pms->GetWakeupActionController();
    if (wakeupActionController == nullptr) {
        POWER_HILOGE(FEATURE_WAKEUP, "[UL_POWER] wakeupActionController is nullptr");
        return;
    }
    SleepGuard sleepGuard(pms);
    Wakeup();
    auto suspendController = pms->GetSuspendController();
    if (suspendController != nullptr) {
        POWER_HILOGI(FEATURE_WAKEUP, "ControlListener TriggerSyncSleepCallback start.");
        suspendController->TriggerSyncSleepCallback(true);
    }
    wakeupActionController->ExecuteByGetReason();
}
#endif

void WakeupController::HandleWakeup(const sptr<PowerMgrService>& pms, WakeupDeviceType reason)
{
#ifdef POWER_MANAGER_ENABLE_EXTERNAL_SCREEN_MANAGEMENT
    if (IsPowerOnInernalScreenOnlyScene(reason)) {
        ProcessPowerOnInternalScreenOnly(pms, reason);
        return;
    }
#endif

    pid_t pid = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    POWER_HILOGI(FEATURE_WAKEUP, "[UL_POWER] Try to wakeup device, pid=%{public}d, uid=%{public}d", pid, uid);
    if (stateMachine_->GetState() != PowerState::AWAKE || reason == WakeupDeviceType::WAKEUP_DEVICE_SWITCH ||
        reason == WakeupDeviceType::WAKEUP_DEVICE_LID) {
        SleepGuard sleepGuard(pms);
        Wakeup();
        SystemSuspendController::GetInstance().Wakeup();
        POWER_HILOGI(FEATURE_WAKEUP, "wakeup Request: %{public}d", reason);
        if (!stateMachine_->SetState(PowerState::AWAKE, stateMachine_->GetReasonByWakeType(reason), true)) {
            POWER_HILOGI(FEATURE_WAKEUP, "[UL_POWER] setstate wakeup error");
        }
        auto suspendController = pms->GetSuspendController();
        if (suspendController != nullptr && stateMachine_->GetState() == PowerState::AWAKE) {
            POWER_HILOGI(FEATURE_WAKEUP, "WakeupController::ControlListener TriggerSyncSleepCallback start.");
            suspendController->TriggerSyncSleepCallback(true);
        }
#ifdef POWER_MANAGER_ENABLE_EXTERNAL_SCREEN_MANAGEMENT
        if (suspendController != nullptr && stateMachine_->GetExternalScreenNumber() > 0 &&
            !stateMachine_->IsSwitchOpenByPath()) {
            suspendController->PowerOffInternalScreen(SuspendDeviceType::SUSPEND_DEVICE_REASON_SWITCH);
        }
#endif
    } else {
        POWER_HILOGI(FEATURE_WAKEUP, "[UL_POWER] state=%{public}u no transitor", stateMachine_->GetState());
    }
}

void WakeupController::ControlListener(WakeupDeviceType reason)
{
    std::lock_guard lock(mutex_);
    if (!Permission::IsSystem()) {
        return;
    }

    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        POWER_HILOGE(FEATURE_WAKEUP, "[UL_POWER] get PowerMgrService instance error");
        return;
    }

    if (NeedToSkipCurrentWakeup(pms, reason)) {
        return;
    }

    HandleWakeup(pms, reason);
}

#ifdef HAS_MULTIMODALINPUT_INPUT_PART
/* InputCallback achieve */
bool InputCallback::isRemoteEvent(std::shared_ptr<InputEvent> event) const
{
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    bool isCollaborationEvent =
        pms && pms->IsCollaborationState() && event->GetDeviceId() == COLLABORATION_REMOTE_DEVICE_ID;
    bool isFromOtherSource = event->GetDeviceId() == OTHER_SYSTEM_DEVICE_ID;
    return isCollaborationEvent || isFromOtherSource;
}

bool InputCallback::isKeyboardKeycode(int32_t keyCode) const
{
    if ((keyCode >= KeyEvent::KEYCODE_0 && keyCode <= KeyEvent::KEYCODE_NUMPAD_RIGHT_PAREN
        && keyCode != KeyEvent::KEYCODE_F1)
        || (keyCode == KeyEvent::KEYCODE_BRIGHTNESS_DOWN) // F1 hotkey
        || (keyCode == KeyEvent::KEYCODE_BRIGHTNESS_UP) // F2 hotkey
        || (keyCode == KeyEvent::KEYCODE_FN) // F3 hotkey
        || (keyCode == KeyEvent::KEYCODE_VOLUME_MUTE) // F4 hotkey
        || (keyCode == KeyEvent::KEYCODE_SOUND) // sound
        || (keyCode == KeyEvent::KEYCODE_MUTE) // F7 hotkey
        || (keyCode == KeyEvent::KEYCODE_SWITCHVIDEOMODE) // F8 hotkey
        || (keyCode == KeyEvent::KEYCODE_SEARCH) // F9 hotkey
        || (keyCode == KeyEvent::KEYCODE_ASSISTANT)) { // assistant
        return true;
    }
    return false;
}

void InputCallback::OnInputEvent(std::shared_ptr<KeyEvent> keyEvent) const
{
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        POWER_HILOGE(FEATURE_WAKEUP, "get powerMgrService instance error");
        return;
    }
    // ignores remote event
    if (isRemoteEvent(keyEvent)) {
        return;
    }
    int64_t now = static_cast<int64_t>(time(nullptr));
    pms->RefreshActivityInner(now, UserActivityType::USER_ACTIVITY_TYPE_BUTTON, false);

    PowerState state = pms->GetState();
    if (state == PowerState::AWAKE || state == PowerState::FREEZE) {
        return;
    }
    std::shared_ptr<WakeupController> wakeupController = pms->GetWakeupController();
    if (wakeupController == nullptr) {
        POWER_HILOGE(FEATURE_WAKEUP, "wakeupController is not init");
        return;
    }

    int32_t keyCode = keyEvent->GetKeyCode();
    WakeupDeviceType wakeupType = WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN;
    if (keyCode == KeyEvent::KEYCODE_F1) {
        wakeupType = WakeupDeviceType::WAKEUP_DEVICE_DOUBLE_CLICK;
    } else if (keyCode == KeyEvent::KEYCODE_STYLUS_SCREEN) {
        wakeupType = WakeupDeviceType::WAKEUP_DEVICE_PEN;
    } else if (keyCode == KeyEvent::KEYCODE_WAKEUP) {
        wakeupType = WakeupDeviceType::WAKEUP_DEVICE_TP_TOUCH;
    }

    if (isKeyboardKeycode(keyCode)) {
        wakeupType = WakeupDeviceType::WAKEUP_DEVICE_KEYBOARD;
        if (wakeupController->CheckEventReciveTime(wakeupType) ||
            keyEvent->GetKeyAction() == KeyEvent::KEY_ACTION_UP) {
            return;
        }
    }

    POWER_HILOGD(FEATURE_WAKEUP, "[UL_POWER] KeyEvent wakeupType=%{public}u, keyCode=%{public}d", wakeupType, keyCode);
    if (wakeupType != WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN) {
        wakeupController->ExecWakeupMonitorByReason(wakeupType);
    }
}

bool InputCallback::TouchEventAfterScreenOn(std::shared_ptr<PointerEvent> pointerEvent, PowerState state) const
{
    if (state == PowerState::AWAKE || state == PowerState::FREEZE) {
#ifdef POWER_MANAGER_ENABLE_WATCH_CUSTOMIZED_SCREEN_COMMON_EVENT_RULES
        int32_t sourceType1 = pointerEvent->GetSourceType();
        if (sourceType1 == PointerEvent::POINTER_ACTION_DOWN) {
            DelayedSingleton<CustomizedScreenEventRules>::GetInstance()->NotifyOperateEventAfterScreenOn();
        }
#endif
        return true;
    }
    return false;
}

void InputCallback::OnInputEvent(std::shared_ptr<PointerEvent> pointerEvent) const
{
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        return;
    }
    if (!NonWindowEvent(pointerEvent)) {
        return;
    }
    if (isRemoteEvent(pointerEvent)) {
        return;
    }
    int64_t now = static_cast<int64_t>(time(nullptr));
    pms->RefreshActivityInner(now, UserActivityType::USER_ACTIVITY_TYPE_TOUCH, false);

    PowerState state = pms->GetState();
    if (TouchEventAfterScreenOn(pointerEvent, state)) {
        return;
    }
    std::shared_ptr<WakeupController> wakeupController = pms->GetWakeupController();
    WakeupDeviceType wakeupType = WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN;
    PointerEvent::PointerItem pointerItem;
    if (!pointerEvent->GetPointerItem(pointerEvent->GetPointerId(), pointerItem)) {
        POWER_HILOGI(FEATURE_WAKEUP, "GetPointerItem false");
    }
    int32_t deviceType = pointerItem.GetToolType();
    int32_t sourceType = pointerEvent->GetSourceType();
    if (deviceType == PointerEvent::TOOL_TYPE_PEN) {
        wakeupType = WakeupDeviceType::WAKEUP_DEVICE_PEN;
    } else {
        switch (sourceType) {
            case PointerEvent::SOURCE_TYPE_MOUSE:
                wakeupType = WakeupDeviceType::WAKEUP_DEVICE_MOUSE;
                break;
            case PointerEvent::SOURCE_TYPE_TOUCHPAD:
                wakeupType = WakeupDeviceType::WAKEUP_DEVICE_TOUCHPAD;
                break;
            case PointerEvent::SOURCE_TYPE_TOUCHSCREEN:
                wakeupType = WakeupDeviceType::WAKEUP_DEVICE_SINGLE_CLICK;
                break;
            default:
                break;
        }
    }
    if (wakeupController->CheckEventReciveTime(wakeupType)) {
        return;
    }

    if (wakeupType != WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN) {
        POWER_HILOGI(FEATURE_WAKEUP, "[UL_POWER] PointerEvent wakeupType=%{public}u", wakeupType);
        wakeupController->ExecWakeupMonitorByReason(wakeupType);
    }
}

bool InputCallback::NonWindowEvent(const std::shared_ptr<PointerEvent>& pointerEvent) const
{
    auto action = pointerEvent->GetPointerAction();
    if (action == PointerEvent::POINTER_ACTION_ENTER_WINDOW ||
        action == PointerEvent::POINTER_ACTION_LEAVE_WINDOW ||
        action == PointerEvent::POINTER_ACTION_PULL_IN_WINDOW ||
        action == PointerEvent::POINTER_ACTION_PULL_OUT_WINDOW) {
        return false;
    }
    return true;
}

void InputCallback::OnInputEvent(std::shared_ptr<AxisEvent> axisEvent) const
{
    POWER_HILOGD(FEATURE_WAKEUP, "AxisEvent");
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        return;
    }
    int64_t now = static_cast<int64_t>(time(nullptr));
    pms->RefreshActivityInner(now, UserActivityType::USER_ACTIVITY_TYPE_ACCESSIBILITY, false);
}
#endif

bool WakeupController::CheckEventReciveTime(WakeupDeviceType wakeupType)
{
    // The minimum refreshactivity interval is 100ms!!
    std::lock_guard lock(eventHandleMutex_);
    int64_t now = GetTickCount();
    if (eventHandleMap_.find(wakeupType) != eventHandleMap_.end()) {
        if ((eventHandleMap_[wakeupType] + MIN_TIME_MS_BETWEEN_MULTIMODEACTIVITIES) > now) {
            return true;
        }
        eventHandleMap_[wakeupType] = now;
        return false;
    }
    return false;
}

#ifdef POWER_MANAGER_ENABLE_EXTERNAL_SCREEN_MANAGEMENT
void WakeupController::PowerOnInternalScreen(WakeupDeviceType type)
{
#ifdef POWER_MANAGER_POWER_ENABLE_S4
    if (stateMachine_->IsHibernating()) {
        POWER_HILOGI(FEATURE_SUSPEND, "[UL_POWER] Do not power the internal screen while hibernating");
        return;
    }
#endif
    if (!stateMachine_->IsSwitchOpen()) {
        POWER_HILOGI(FEATURE_SUSPEND, "[UL_POWER] Do not power the internal screen while switch is close");
        return;
    }

    auto changeReason = stateMachine_->GetReasonByWakeType(type);
    auto dmsReason = PowerUtils::GetDmsReasonByPowerReason(changeReason);
    uint64_t screenId = Rosen::DisplayManagerLite::GetInstance().GetInternalScreenId();
    bool ret = Rosen::DisplayManagerLite::GetInstance().SetScreenPowerById(
        screenId, Rosen::ScreenPowerState::POWER_ON, dmsReason);
    stateMachine_->SetInternalScreenBrightness();
    POWER_HILOGI(FEATURE_WAKEUP,
        "[UL_POWER] Power on internal screen, reason = %{public}u, screenId = %{public}u, ret = %{public}d", dmsReason,
        static_cast<uint32_t>(screenId), ret);
}

void WakeupController::PowerOnAllScreens(WakeupDeviceType type)
{
#ifdef POWER_MANAGER_POWER_ENABLE_S4
    if (stateMachine_->IsHibernating()) {
        POWER_HILOGI(FEATURE_SUSPEND, "[UL_POWER] Do not power all the screen while hibernating");
        return;
    }
#endif

    auto changeReason = stateMachine_->GetReasonByWakeType(type);
    auto dmsReason = PowerUtils::GetDmsReasonByPowerReason(changeReason);
    bool ret =
        Rosen::ScreenManagerLite::GetInstance().SetScreenPowerForAll(Rosen::ScreenPowerState::POWER_ON, dmsReason);
    POWER_HILOGI(FEATURE_WAKEUP, "Power on all screens, reason = %{public}u, ret = %{public}d", dmsReason, ret);
}

bool WakeupController::IsPowerOnInernalScreenOnlyScene(WakeupDeviceType reason) const
{
    // switch action 0 menas only doing poweroff when closing switch
    if (reason == WakeupDeviceType::WAKEUP_DEVICE_SWITCH && stateMachine_->GetSwitchAction() == 0 &&
        stateMachine_->GetExternalScreenNumber() > 0 && stateMachine_->IsScreenOn()) {
        return true;
    }
    return false;
}

void WakeupController::ProcessPowerOnInternalScreenOnly(const sptr<PowerMgrService>& pms, WakeupDeviceType reason)
{
    POWER_HILOGI(FEATURE_WAKEUP, "[UL_POWER] Power on internal screen only when external screen is on");
    PowerOnInternalScreen(reason);
    pms->RefreshActivity(GetTickCount(), UserActivityType::USER_ACTIVITY_TYPE_SWITCH, false);
}
#endif

bool WakeupController::NeedToSkipCurrentWakeup(const sptr<PowerMgrService>& pms, WakeupDeviceType reason) const
{
    bool skipWakeup = !stateMachine_->IsSwitchOpen();
#ifdef POWER_MANAGER_ENABLE_EXTERNAL_SCREEN_MANAGEMENT
    skipWakeup = skipWakeup && (stateMachine_->GetExternalScreenNumber() <= 0);
#endif
    if (skipWakeup) {
        POWER_HILOGI(FEATURE_WAKEUP, "[UL_POWER] Switch is closed, skip current wakeup reason: %{public}u", reason);
#ifdef POWER_MANAGER_ENABLE_WATCH_CUSTOMIZED_SCREEN_COMMON_EVENT_RULES
        DelayedSingleton<CustomizedScreenEventRules>::GetInstance()->NotifyScreenOnEventAgain(reason);
#endif
        return true;
    }

#ifdef POWER_MANAGER_POWER_ENABLE_S4
    skipWakeup = stateMachine_->IsHibernating();
    if (skipWakeup) {
        POWER_HILOGI(
            FEATURE_WAKEUP, "[UL_POWER] Device is hibernating, skip current wakeup reason: %{public}u", reason);
        return true;
    }
#endif

    skipWakeup = (pms->IsScreenOn()) && (reason != WakeupDeviceType::WAKEUP_DEVICE_SWITCH) &&
        (reason != WakeupDeviceType::WAKEUP_DEVICE_LID);
    if (skipWakeup) {
        POWER_HILOGI(FEATURE_WAKEUP, "[UL_POWER] Screen is on, skip current wakeup reason: %{public}u", reason);
        return true;
    }

    return false;
}

/* WakeupMonitor Implement */

std::shared_ptr<WakeupMonitor> WakeupMonitor::CreateMonitor(WakeupSource& source)
{
    WakeupDeviceType reason = source.GetReason();
    std::shared_ptr<WakeupMonitor> monitor = nullptr;
    switch (reason) {
        case WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON:
            monitor = std::static_pointer_cast<WakeupMonitor>(std::make_shared<PowerkeyWakeupMonitor>(source));
            break;
        case WakeupDeviceType::WAKEUP_DEVICE_MOUSE:
            monitor = std::static_pointer_cast<WakeupMonitor>(std::make_shared<MousekeyWakeupMonitor>(source));
            break;
        case WakeupDeviceType::WAKEUP_DEVICE_KEYBOARD:
            monitor = std::static_pointer_cast<WakeupMonitor>(std::make_shared<KeyboardWakeupMonitor>(source));
            break;
        case WakeupDeviceType::WAKEUP_DEVICE_PEN:
            monitor = std::static_pointer_cast<WakeupMonitor>(std::make_shared<PenWakeupMonitor>(source));
            break;
        case WakeupDeviceType::WAKEUP_DEVICE_TOUCHPAD:
            monitor = std::static_pointer_cast<WakeupMonitor>(std::make_shared<TouchpadWakeupMonitor>(source));
            break;
        case WakeupDeviceType::WAKEUP_DEVICE_SINGLE_CLICK:
            monitor = std::static_pointer_cast<WakeupMonitor>(std::make_shared<SingleClickWakeupMonitor>(source));
            break;
        case WakeupDeviceType::WAKEUP_DEVICE_DOUBLE_CLICK:
            monitor = std::static_pointer_cast<WakeupMonitor>(std::make_shared<DoubleClickWakeupMonitor>(source));
            break;
        case WakeupDeviceType::WAKEUP_DEVICE_LID:
            monitor = std::static_pointer_cast<WakeupMonitor>(std::make_shared<LidWakeupMonitor>(source));
            break;
        case WakeupDeviceType::WAKEUP_DEVICE_SWITCH:
            monitor = std::static_pointer_cast<WakeupMonitor>(std::make_shared<SwitchWakeupMonitor>(source));
            break;
        case WakeupDeviceType::WAKEUP_DEVICE_PICKUP:
            monitor = std::static_pointer_cast<WakeupMonitor>(std::make_shared<PickupWakeupMonitor>(source));
            break;
        case WakeupDeviceType::WAKEUP_DEVICE_TP_TOUCH:
            monitor = std::static_pointer_cast<WakeupMonitor>(std::make_shared<TPTouchWakeupMonitor>(source));
            break;
        default:
            POWER_HILOGE(FEATURE_WAKEUP, "CreateMonitor : Invalid reason=%{public}d", reason);
            break;
    }
    return monitor;
}

/** PowerkeyWakeupMonitor Implement */
bool PowerkeyWakeupMonitor::Init()
{
    if (powerkeyShortPressId_ >= 0) {
        return true;
    }
    std::shared_ptr<OHOS::MMI::KeyOption> keyOption = std::make_shared<OHOS::MMI::KeyOption>();
    std::set<int32_t> preKeys;
    keyOption.reset();
    keyOption = std::make_shared<OHOS::MMI::KeyOption>();
    keyOption->SetPreKeys(preKeys);
    keyOption->SetFinalKey(OHOS::MMI::KeyEvent::KEYCODE_POWER);
    keyOption->SetFinalKeyDown(true);
    keyOption->SetFinalKeyDownDuration(0);
    std::weak_ptr<PowerkeyWakeupMonitor> weak = weak_from_this();
    powerkeyShortPressId_ = InputManager::GetInstance()->SubscribeKeyEvent(
        keyOption, [weak](std::shared_ptr<OHOS::MMI::KeyEvent> keyEvent) {
            std::shared_ptr<PowerkeyWakeupMonitor> strong = weak.lock();
            if (!strong) {
                POWER_HILOGI(FEATURE_WAKEUP, "[UL_POWER] PowerkeyWakeupMonitor is invaild, return");
                return;
            }

            strong->ReceivePowerkeyCallback(keyEvent);
        });

    POWER_HILOGI(FEATURE_WAKEUP, "powerkey register powerkeyShortPressId_=%{public}d", powerkeyShortPressId_);
    return powerkeyShortPressId_ >= 0 ? true : false;
}

void PowerkeyWakeupMonitor::ReceivePowerkeyCallback(std::shared_ptr<OHOS::MMI::KeyEvent> keyEvent)
{
#ifndef POWER_MANAGER_ALLOW_INTERRUPTING_POWERKEY_OFF
    ffrt::wait({&PowerKeySuspendMonitor::powerkeyScreenOff_});
#endif
    POWER_HILOGI(FEATURE_WAKEUP, "[UL_POWER] Received powerkey down");

    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        POWER_HILOGE(FEATURE_WAKEUP, "[UL_POWER] PowerMgrService is nullptr");
        return;
    }
    int64_t now = static_cast<int64_t>(time(nullptr));
    pms->RefreshActivityInner(now, UserActivityType::USER_ACTIVITY_TYPE_BUTTON, false);

    std::shared_ptr<WakeupController> wakeupController = pms->GetWakeupController();
    if (wakeupController == nullptr) {
        POWER_HILOGE(FEATURE_WAKEUP, "[UL_POWER] wakeupController is nullptr");
        return;
    }
    std::shared_ptr<SuspendController> suspendController = pms->GetSuspendController();
    if (suspendController == nullptr) {
        POWER_HILOGE(FEATURE_WAKEUP, "[UL_POWER] suspendController is nullptr");
        return;
    }

#ifdef POWER_MANAGER_WAKEUP_ACTION
    if (wakeupController->IsLowCapacityWakeup(reason)) {
        wakeupController->ProcessLowCapacityWakeup();
        suspendController->SetLowCapacityPowerKeyFlag(true);
        return;
    }
    suspendController->SetLowCapacityPowerKeyFlag(false);
#endif

    bool poweroffInterrupted = false;
    if (PowerKeySuspendMonitor::powerkeyScreenOff_.load()) {
        auto stateMachine = pms->GetPowerStateMachine();
        if (!stateMachine) {
            POWER_HILOGE(FEATURE_WAKEUP, "TryToCancelScreenOff, state machine is nullptr");
        } else {
            poweroffInterrupted = stateMachine->TryToCancelScreenOff();
        }
    }
    // sync with the end of powerkey screen off task
    ffrt::wait({&PowerKeySuspendMonitor::powerkeyScreenOff_});
    suspendController->RecordPowerKeyDown(poweroffInterrupted);
    Notify();
}

void PowerkeyWakeupMonitor::Cancel()
{
    if (powerkeyShortPressId_ >= 0) {
        POWER_HILOGI(FEATURE_WAKEUP, "UnsubscribeKeyEvent: PowerkeyWakeupMonitor");
        InputManager::GetInstance()->UnsubscribeKeyEvent(powerkeyShortPressId_);
    }
}

/** Keyboard Implement */

bool KeyboardWakeupMonitor::Init()
{
    return true;
}

void KeyboardWakeupMonitor::Cancel() {}

/** Mouse Implement */

bool MousekeyWakeupMonitor::Init()
{
    return true;
}

void MousekeyWakeupMonitor::Cancel() {}

/** Mouse Implement */

bool TouchpadWakeupMonitor::Init()
{
    return true;
}

void TouchpadWakeupMonitor::Cancel() {}

/** Pen Implement */

bool PenWakeupMonitor::Init()
{
    return true;
}

void PenWakeupMonitor::Cancel() {}

/** SingleClickWakeupMonitor Implement */

bool SingleClickWakeupMonitor::Init()
{
    return true;
}

void SingleClickWakeupMonitor::Cancel() {}

/** DoubleClickWakeupMonitor Implement */

bool DoubleClickWakeupMonitor::Init()
{
    return true;
}

void DoubleClickWakeupMonitor::Cancel() {}

/** SwitchWakeupMonitor Implement */

bool SwitchWakeupMonitor::Init()
{
    return true;
}

void SwitchWakeupMonitor::Cancel() {}

/** LidWakeupMonitor Implement */

bool LidWakeupMonitor::Init()
{
    return true;
}

void LidWakeupMonitor::Cancel() {}

/** PickupWakeupMonitor Implement */

bool PickupWakeupMonitor::Init()
{
    return true;
}

void PickupWakeupMonitor::Cancel() {}

/** TPTouchWakeupMonitor Implement */

bool TPTouchWakeupMonitor::Init()
{
    return true;
}

void TPTouchWakeupMonitor::Cancel() {}
} // namespace PowerMgr
} // namespace OHOS
