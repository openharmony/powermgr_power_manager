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
#include <hisysevent.h>
#include <input_manager.h>
#include <ipc_skeleton.h>
#include <securec.h>
#ifdef POWER_WAKEUPDOUBLE_OR_PICKUP_ENABLE
#include <dlfcn.h>
#include "json/json.h"
#endif
#include "permission.h"
#include "power_errors.h"
#include "power_log.h"
#include "power_mgr_service.h"
#include "power_state_callback_stub.h"
#include "setting_helper.h"
#include "suspend_controller.h"
#include "system_suspend_controller.h"

namespace OHOS {
namespace PowerMgr {
using namespace OHOS::MMI;
namespace {
sptr<SettingObserver> g_wakeupSourcesKeyObserver = nullptr;
#ifdef POWER_WAKEUPDOUBLE_OR_PICKUP_ENABLE
const int32_t ERR_FAILED = -1;
#endif
}

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

    if (g_wakeupSourcesKeyObserver) {
        SettingHelper::UnregisterSettingObserver(g_wakeupSourcesKeyObserver);
    }
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
#ifdef POWER_WAKEUPDOUBLE_OR_PICKUP_ENABLE
            SetOriginSettingValue((*source));
#endif
        std::shared_ptr<WakeupMonitor> monitor = WakeupMonitor::CreateMonitor(*source);
        if (monitor != nullptr && monitor->Init()) {
            POWER_HILOGI(FEATURE_WAKEUP, "register type=%{public}u", (*source).GetReason());
            monitor->RegisterListener(std::bind(&WakeupController::ControlListener, this, std::placeholders::_1));
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
        std::vector<WakeupSource> updateSourceList = sources->GetSourceList();
        if (updateSourceList.size() == 0) {
            return;
        }
        sourceList_ = updateSourceList;
        POWER_HILOGI(COMP_SVC, "start updateListener");
        Cancel();
        for (auto source = sourceList_.begin(); source != sourceList_.end(); source++) {
            std::shared_ptr<WakeupMonitor> monitor = WakeupMonitor::CreateMonitor(*source);
            if (monitor != nullptr && monitor->Init()) {
                monitor->RegisterListener(std::bind(&WakeupController::ControlListener, this, std::placeholders::_1));
                monitorMap_.emplace(monitor->GetReason(), monitor);
            }
        }
    };
    g_wakeupSourcesKeyObserver = SettingHelper::RegisterSettingWakeupSourcesObserver(updateFunc);
    POWER_HILOGI(FEATURE_POWER_STATE, "register setting observer fin");
}

#ifdef POWER_WAKEUPDOUBLE_OR_PICKUP_ENABLE
void WakeupController::SetOriginSettingValue(WakeupSource& source)
{
    if (source.GetReason() == WakeupDeviceType::WAKEUP_DEVICE_DOUBLE_CLICK) {
        POWER_HILOGI(COMP_SVC, "the origin doubleClick_enable is: %{public}d", source.IsEnable());
        if (SettingHelper::IsWakeupDoubleSettingValid() == false) {
            SettingHelper::SetSettingWakeupDouble(false);
            SetWakeupDoubleClickSensor(false);
            return;
        }
        auto enable = SettingHelper::GetSettingWakeupDouble();
        SetWakeupDoubleClickSensor(enable);
    } else if (source.GetReason() == WakeupDeviceType::WAKEUP_DEVICE_PICKUP) {
        POWER_HILOGI(FEATURE_WAKEUP, "GetReason_WAKEUP_DEVICE_PICKUP,source enable=%{public}d", source.IsEnable());
        if (!SettingHelper::IsWakeupPickupSettingValid()) {
            POWER_HILOGI(COMP_SVC, "enter WAKEUP_DEVICE_PICKUP inValid");
            SettingHelper::SetSettingWakeupPickup(false);
            PickupConnectMotionConfig(false);
            POWER_HILOGI(COMP_SVC, "WAKEUP_DEVICE_PICKUP inValid done");
            return;
        }
        auto enable = SettingHelper::GetSettingWakeupPickup();
        PickupConnectMotionConfig(enable);
    }
}

void WakeupController::ChangeWakeupSourceConfig(bool updateEnable)
{
    std::string jsonStr = SettingHelper::GetSettingWakeupSources();
    POWER_HILOGI(COMP_SVC, "the origin ccmJson is: %{public}s", jsonStr.c_str());
    Json::Value root;
    Json::Reader reader;
    if (!reader.parse(jsonStr.data(), jsonStr.data() + jsonStr.size(), root)) {
        POWER_HILOGE(COMP_SVC, "json parse error");
        return;
    }

    bool originEnable = root["touchscreen"]["enable"].asBool();
    if (originEnable == updateEnable) {
        POWER_HILOGI(COMP_SVC, "no need change jsonConfig value");
        return;
    }

    root["touchscreen"]["enable"] = updateEnable;
    POWER_HILOGI(COMP_SVC, "the new jsonConfig is: %{public}s", root.toStyledString().c_str());
    SettingHelper::SetSettingWakeupSources(root.toStyledString());
}

static const char* SET_WAKEUP_DOUBLE_CLICK_SENSOR = "SetWakeupDoubleClickSensor";
static const char* POWER_DOUBLE_CLICK_PATH = "/system/lib64/libpower_manager_ext.z.so";
typedef int32_t(*Func)(bool);
int32_t WakeupController::SetWakeupDoubleClickSensor(bool enable)
{
    POWER_HILOGI(COMP_SVC, "enter SetWakeupDoubleClickSensor");
    void *handler = dlopen(POWER_DOUBLE_CLICK_PATH, RTLD_LAZY | RTLD_NODELETE);
    if (handler == nullptr) {
        POWER_HILOGE(FEATURE_SHUTDOWN, "Dlopen failed, reason : %{public}s", dlerror());
        return ERR_FAILED;
    }

    Func PowerDoubleClickFlag = (Func)dlsym(handler, SET_WAKEUP_DOUBLE_CLICK_SENSOR);
    if (PowerDoubleClickFlag == nullptr) {
        POWER_HILOGE(FEATURE_SHUTDOWN, "find function failed, reason : %{public}s", dlerror());
        dlclose(handler);
        return ERR_FAILED;
    }
    auto resCode = PowerDoubleClickFlag(enable);
    dlclose(handler);
    return resCode;
}

static const char* SET_WAKEUP_MOTION_SUBSCRIBER_CONFIG = "PickupMotionSubscriber";
static const char* SET_WAKEUP_MOTION_UNSUBSCRIBER_CONFIG = "PickupMotionUnsubscriber";
static const char* POWER_MANAGER_EXT_PATH = "/system/lib64/libpower_manager_ext.z.so";
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
    std::string jsonStr = SettingHelper::GetSettingWakeupSources();
    POWER_HILOGI(FEATURE_POWER_STATE, "%{public}s", jsonStr.c_str());
    Json::Value root;
    Json::Reader reader;
    reader.parse(jsonStr, root);
    if (!reader.parse(jsonStr, root)) {
        POWER_HILOGE(FEATURE_POWER_STATE, "Failed to parse json string");
        return;
    }
    bool originEnable = root["pickup"]["enable"].asBool();
    if (originEnable == updataEnable) {
        POWER_HILOGI(FEATURE_POWER_STATE, "no need change jsonconfig_value");
        return;
    }
    root["pickup"]["enable"] = updataEnable;
    SettingHelper::SetSettingWakeupSources(root.toStyledString());
}
#endif

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

void WakeupController::ControlListener(WakeupDeviceType reason)
{
    std::lock_guard lock(mutex_);
    if (!Permission::IsSystem()) {
        return;
    }
    if (!stateMachine_->IsSwitchOpen()) {
        POWER_HILOGI(FEATURE_WAKEUP, "Switch is closed, wakeup control listerner do nothing.");
        return;
    }
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr || pms->IsScreenOn()) {
        POWER_HILOGI(FEATURE_WAKEUP, "[UL_POWER] The Screen is on, ignore this powerkey down event.");
        return;
    }
    pid_t pid = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    POWER_HILOGI(FEATURE_WAKEUP, "[UL_POWER] Try to wakeup device, pid=%{public}d, uid=%{public}d", pid, uid);
    if (stateMachine_->GetState() != PowerState::AWAKE) {
        Wakeup();
        SystemSuspendController::GetInstance().Wakeup();
        POWER_HILOGI(FEATURE_WAKEUP, "wakeup Request: %{public}d", reason);
        if (stateMachine_->GetState() == PowerState::SLEEP) {
            auto suspendController = pms->GetSuspendController();
            if (suspendController != nullptr) {
                POWER_HILOGI(FEATURE_WAKEUP, "WakeupController::ControlListener TriggerSyncSleepCallback start.");
                suspendController->TriggerSyncSleepCallback(true);
            }
        }
#ifdef POWER_MANAGER_WAKEUP_ACTION
        POWER_HILOGI(FEATURE_WAKEUP, "start get wakeup action reason");
        if ((reason == WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON) &&
            (pms->GetWakeupActionController()->ExecuteByGetReason())) {
                POWER_HILOGI(FEATURE_WAKEUP, "wakeup action reason avaiable");
                return;
        }
#endif
        if (!stateMachine_->SetState(PowerState::AWAKE,
            stateMachine_->GetReasonByWakeType(reason), true)) {
            POWER_HILOGI(FEATURE_WAKEUP, "[UL_POWER] setstate wakeup error");
        }
    } else {
        POWER_HILOGI(FEATURE_WAKEUP, "[UL_POWER] state=%{public}u no transitor", stateMachine_->GetState());
    }
}

#ifdef HAS_MULTIMODALINPUT_INPUT_PART
/* InputCallback achieve */
void InputCallback::OnInputEvent(std::shared_ptr<KeyEvent> keyEvent) const
{
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        POWER_HILOGE(FEATURE_WAKEUP, "get powerMgrService instance error");
        return;
    }
    if (keyEvent->HasFlag(InputEvent::EVENT_FLAG_SIMULATE) && pms->IsCollaborationState()) {
        return;
    }
    int64_t now = static_cast<int64_t>(time(nullptr));
    pms->RefreshActivity(now, UserActivityType::USER_ACTIVITY_TYPE_BUTTON, false);

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
    }

    if (keyCode >= KeyEvent::KEYCODE_0 && keyCode <= KeyEvent::KEYCODE_NUMPAD_RIGHT_PAREN
        && keyCode != KeyEvent::KEYCODE_F1) {
        wakeupType = WakeupDeviceType::WAKEUP_DEVICE_KEYBOARD;
        if (wakeupController->CheckEventReciveTime(wakeupType) ||
            keyEvent->GetKeyAction() == KeyEvent::KEY_ACTION_UP) {
            return;
        }
    }

    POWER_HILOGI(FEATURE_WAKEUP, "[UL_POWER] KeyEvent wakeupType=%{public}u, keyCode=%{public}d", wakeupType, keyCode);
    if (wakeupType != WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN) {
        wakeupController->ExecWakeupMonitorByReason(wakeupType);
    }
}

void InputCallback::OnInputEvent(std::shared_ptr<PointerEvent> pointerEvent) const
{
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        return;
    }
    if (pointerEvent->HasFlag(InputEvent::EVENT_FLAG_SIMULATE) && pms->IsCollaborationState()) {
        return;
    }
    int64_t now = static_cast<int64_t>(time(nullptr));
    pms->RefreshActivity(now, UserActivityType::USER_ACTIVITY_TYPE_TOUCH, false);
    std::shared_ptr<WakeupController> wakeupController = pms->GetWakeupController();
    if (!NonWindowEvent(pointerEvent, pms)) {
        return;
    }
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

bool InputCallback::NonWindowEvent(std::shared_ptr<PointerEvent>& pointerEvent, sptr<PowerMgrService>& pms) const
{
    PowerState state = pms->GetState();
    if (state == PowerState::AWAKE || state == PowerState::FREEZE) {
        return false;
    }

    if (pointerEvent->GetPointerAction() == PointerEvent::POINTER_ACTION_ENTER_WINDOW
        || pointerEvent->GetPointerAction() == PointerEvent::POINTER_ACTION_LEAVE_WINDOW) {
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
    pms->RefreshActivity(now, UserActivityType::USER_ACTIVITY_TYPE_ACCESSIBILITY, false);
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

/* WakeupMonitor Implement */

std::shared_ptr<WakeupMonitor> WakeupMonitor::CreateMonitor(WakeupSource& source)
{
    WakeupDeviceType reason = source.GetReason();
    POWER_HILOGE(FEATURE_WAKEUP, "CreateMonitor reason=%{public}d", reason);
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
    powerkeyShortPressId_ = InputManager::GetInstance()->SubscribeKeyEvent(
        keyOption, [this](std::shared_ptr<OHOS::MMI::KeyEvent> keyEvent) {
            POWER_HILOGI(FEATURE_WAKEUP, "[UL_POWER] Received powerkey down");
            auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
            if (pms == nullptr) {
                return;
            }
            pms->RefreshActivity(
                static_cast<int64_t>(time(nullptr)), UserActivityType::USER_ACTIVITY_TYPE_BUTTON, false);
            std::shared_ptr<SuspendController> suspendController = pms->GetSuspendController();
            suspendController->RecordPowerKeyDown();
            Notify();
        });

    POWER_HILOGI(FEATURE_WAKEUP, "powerkey register powerkeyShortPressId_=%{public}d", powerkeyShortPressId_);
    return powerkeyShortPressId_ >= 0 ? true : false;
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

} // namespace PowerMgr
} // namespace OHOS
