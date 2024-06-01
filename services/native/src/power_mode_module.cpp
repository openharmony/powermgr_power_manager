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

#include "power_mode_module.h"

#ifdef HAS_DISPLAY_MANAGER
#include "display_power_mgr_client.h"
#endif
#include "power_log.h"
#include "power_mode_policy.h"
#include "power_mgr_service.h"
#include "setting_helper.h"

#include "singleton.h"

using namespace std;
using namespace OHOS;
using namespace OHOS::AAFwk;
using namespace OHOS::EventFwk;

namespace OHOS {
namespace PowerMgr {
namespace {
sptr<SettingObserver> g_autoAdjustBrightnessObserver;
sptr<SettingObserver> g_autoWindowRotationObserver;
sptr<SettingObserver> g_vibratorsStateObserver;
sptr<SettingObserver> g_intellVoiceObserver;
}

PowerModeModule::PowerModeModule()
    : mode_(PowerMode::NORMAL_MODE), lastMode_(LAST_MODE_FLAG), started_(false)
{
    POWER_HILOGI(FEATURE_POWER_MODE, "Instance create");
    callbackMgr_ = new CallbackManager();
    auto policy = DelayedSingleton<PowerModePolicy>::GetInstance();
    PowerModePolicy::ModeAction displayOffTimeAction = [&](bool isInit) { SetDisplayOffTime(isInit); };
    policy->AddAction(PowerModePolicy::ServiceType::DISPLAY_OFFTIME, displayOffTimeAction);
    PowerModePolicy::ModeAction sleepTimeAction = [&](bool isInit) { SetSleepTime(isInit); };
    policy->AddAction(PowerModePolicy::ServiceType::SLEEPTIME, sleepTimeAction);
    PowerModePolicy::ModeAction autoAdjustBrightnessAction = [&](bool isInit) { SetAutoAdjustBrightness(isInit); };
    policy->AddAction(PowerModePolicy::ServiceType::AUTO_ADJUST_BRIGHTNESS, autoAdjustBrightnessAction);
    PowerModePolicy::ModeAction lcdBrightnessAction = [&](bool isInit) { SetLcdBrightness(isInit); };
    policy->AddAction(PowerModePolicy::ServiceType::LCD_BRIGHTNESS, lcdBrightnessAction);
    PowerModePolicy::ModeAction vibrationAction = [&](bool isInit) { SetVibration(isInit); };
    policy->AddAction(PowerModePolicy::ServiceType::VIBRATORS_STATE, vibrationAction);
    PowerModePolicy::ModeAction onOffRotationAction = [&](bool isInit) { SetWindowRotation(isInit); };
    policy->AddAction(PowerModePolicy::ServiceType::AUTO_WINDOWN_RORATION, onOffRotationAction);
    PowerModePolicy::ModeAction intellVoiceAction = [&](bool isInit) { SetIntellVoiceState(isInit); };
    policy->AddAction(PowerModePolicy::ServiceType::INTELL_VOICE, intellVoiceAction);
}

void PowerModeModule::SetModeItem(PowerMode mode)
{
    POWER_HILOGI(FEATURE_POWER_MODE, "mode_: %{public}u, mode: %{public}u", mode_, mode);

    /* Same as the previous mode */
    if (this->mode_ == mode) {
        return;
    }

    /* If it's a valid mode */
    if (mode < PowerMode::POWER_MODE_MIN || mode > PowerMode::POWER_MODE_MAX) {
        POWER_HILOGW(FEATURE_POWER_MODE, "Invalid mode %{public}d", mode);
        return;
    }

    /* unregister setting observer for current mode */
    UnregisterSaveModeObserver();

    /* start set mode thread */
    EnableMode(mode);

    /* register setting observer for save mode */
    RegisterSaveModeObserver();
}

PowerMode PowerModeModule::GetModeItem()
{
    POWER_HILOGD(FEATURE_POWER_MODE, "mode_: %{public}u", mode_);
    /* get power mode */
    return mode_;
}

void PowerModeModule::UnregisterSaveModeObserver()
{
    if (!this->observerRegisted_) {
        POWER_HILOGD(FEATURE_POWER_MODE, "current power mode is normal mode");
        return;
    }

    POWER_HILOGI(FEATURE_POWER_MODE, "unregister setting observer for save mode");
    SettingHelper::UnregisterSettingObserver(g_autoAdjustBrightnessObserver);
    SettingHelper::UnregisterSettingObserver(g_autoWindowRotationObserver);
    SettingHelper::UnregisterSettingObserver(g_vibratorsStateObserver);
    SettingHelper::UnregisterSettingObserver(g_intellVoiceObserver);
    g_autoAdjustBrightnessObserver = nullptr;
    g_autoWindowRotationObserver = nullptr;
    g_vibratorsStateObserver = nullptr;
    g_intellVoiceObserver = nullptr;
    observerRegisted_ = false;
}

void PowerModeModule::RegisterSaveModeObserver()
{
    if (this->mode_ == PowerMode::POWER_SAVE_MODE || this->mode_ == PowerMode::EXTREME_POWER_SAVE_MODE) {
        POWER_HILOGD(FEATURE_POWER_MODE, "register setting observer in save mode");
        RegisterAutoAdjustBrightnessObserver();
        RegisterAutoWindowRotationObserver();
        RegisterVibrateStateObserver();
        RegisterIntellVoiceObserver();
        observerRegisted_ = true;
    }
}

static void AutoAdjustBrightnessUpdateFunc()
{
    auto policy = DelayedSingleton<PowerModePolicy>::GetInstance();
    int32_t switchVal = policy->GetPowerModeValuePolicy(PowerModePolicy::ServiceType::AUTO_ADJUST_BRIGHTNESS);
    auto setVal = SettingHelper::GetSettingAutoAdjustBrightness(switchVal);
    if (setVal == switchVal) {
        return;
    }
    policy->RemoveBackupMapSettingSwitch(PowerModePolicy::ServiceType::AUTO_ADJUST_BRIGHTNESS);
}

void PowerModeModule::RegisterAutoAdjustBrightnessObserver()
{
    if (g_autoAdjustBrightnessObserver) {
        POWER_HILOGD(FEATURE_POWER_MODE, "auto adjust brightness observer already registed");
        return;
    }
    SettingObserver::UpdateFunc updateFunc = [&](const std::string&) {
        AutoAdjustBrightnessUpdateFunc();
    };
    g_autoAdjustBrightnessObserver = SettingHelper::RegisterSettingAutoAdjustBrightnessObserver(updateFunc);
}

static void WindowRotationUpdateFunc()
{
    auto policy = DelayedSingleton<PowerModePolicy>::GetInstance();
    int32_t switchVal = policy->GetPowerModeValuePolicy(PowerModePolicy::ServiceType::AUTO_WINDOWN_RORATION);
    auto setVal = SettingHelper::GetSettingWindowRotation(switchVal);
    if (setVal == switchVal) {
        return;
    }
    policy->RemoveBackupMapSettingSwitch(PowerModePolicy::ServiceType::AUTO_WINDOWN_RORATION);
}

void PowerModeModule::RegisterAutoWindowRotationObserver()
{
    if (g_autoWindowRotationObserver) {
        POWER_HILOGD(FEATURE_POWER_MODE, "auto window rotation observer already registed");
        return;
    }
    SettingObserver::UpdateFunc updateFunc = [&](const std::string&) {
        WindowRotationUpdateFunc();
    };
    g_autoWindowRotationObserver = SettingHelper::RegisterSettingWindowRotationObserver(updateFunc);
}

static void VibrateStateUpdateFunc()
{
    auto policy = DelayedSingleton<PowerModePolicy>::GetInstance();
    int32_t switchVal = policy->GetPowerModeValuePolicy(PowerModePolicy::ServiceType::VIBRATORS_STATE);
    auto setVal = SettingHelper::GetSettingVibration(switchVal);
    if (setVal == switchVal) {
        return;
    }
    policy->RemoveBackupMapSettingSwitch(PowerModePolicy::ServiceType::VIBRATORS_STATE);
}

void PowerModeModule::RegisterVibrateStateObserver()
{
    if (g_vibratorsStateObserver) {
        POWER_HILOGD(FEATURE_POWER_MODE, "vibrate state observer already registed");
        return;
    }
    SettingObserver::UpdateFunc updateFunc = [&](const std::string&) {
        VibrateStateUpdateFunc();
    };
    g_vibratorsStateObserver = SettingHelper::RegisterSettingVibrationObserver(updateFunc);
}

static void IntellVoiceUpdateFunc()
{
    auto policy = DelayedSingleton<PowerModePolicy>::GetInstance();
    int32_t switchVal = policy->GetPowerModeValuePolicy(PowerModePolicy::ServiceType::INTELL_VOICE);
    auto setVal = SettingHelper::GetSettingIntellVoice(switchVal);
    if (setVal == switchVal) {
        return;
    }
    policy->RemoveBackupMapSettingSwitch(PowerModePolicy::ServiceType::INTELL_VOICE);
}

void PowerModeModule::RegisterIntellVoiceObserver()
{
    if (g_intellVoiceObserver) {
        POWER_HILOGD(FEATURE_POWER_MODE, "intell voice observer already registed");
        return;
    }
    SettingObserver::UpdateFunc updateFunc = [&](const std::string&) {
        IntellVoiceUpdateFunc();
    };
    g_intellVoiceObserver = SettingHelper::RegisterSettingIntellVoiceObserver(updateFunc);
}

void PowerModeModule::EnableMode(PowerMode mode, bool isBoot)
{
    if (started_) {
        POWER_HILOGW(FEATURE_POWER_MODE, "Power Mode is already running");
        return;
    }

    started_ = true;
    mode_ = mode;

    /* Update power mode policy */
    UpdateModepolicy();

    /* Send state change */
    Prepare();

    /* Set action */
    RunAction(isBoot);

    this->lastMode_ = static_cast<uint32_t>(mode);
    started_ = false;
}

void PowerModeModule::UpdateModepolicy()
{
    /* update policy */
    DelayedSingleton<PowerModePolicy>::GetInstance()->UpdatePowerModePolicy(static_cast<uint32_t>(this->mode_));
}

void PowerModeModule::AddPowerModeCallback(const sptr<IPowerModeCallback>& callback)
{
    if (callbackMgr_) {
        callbackMgr_->AddCallback(callback);
    }
}

void PowerModeModule::DelPowerModeCallback(const sptr<IPowerModeCallback>& callback)
{
    if (callbackMgr_) {
        callbackMgr_->RemoveCallback(callback);
    }
}

void PowerModeModule::Prepare()
{
    PublishPowerModeEvent();
    if (callbackMgr_) {
        callbackMgr_->WaitingCallback();
    }
}

void PowerModeModule::CallbackManager::AddCallback(const sptr<IPowerModeCallback>& callback)
{
    unique_lock<mutex> lock(mutex_);
    RETURN_IF((callback == nullptr) || (callback->AsObject() == nullptr));
    auto object = callback->AsObject();
    auto retIt = callbacks_.insert(object);
    if (retIt.second) {
        object->AddDeathRecipient(this);
    }
    POWER_HILOGD(FEATURE_POWER_MODE, "callbacks.size = %{public}zu, insertOk = %{public}d",
        callbacks_.size(), retIt.second);
}

void PowerModeModule::CallbackManager::RemoveCallback(const sptr<IPowerModeCallback>& callback)
{
    unique_lock<mutex> lock(mutex_);
    RETURN_IF((callback == nullptr) || (callback->AsObject() == nullptr));
    auto object = callback->AsObject();
    auto it = find(callbacks_.begin(), callbacks_.end(), object);
    if (it != callbacks_.end()) {
        callbacks_.erase(it);
        object->RemoveDeathRecipient(this);
    }
    POWER_HILOGD(FEATURE_POWER_MODE, "callbacks.size = %{public}zu", callbacks_.size());
}

void PowerModeModule::CallbackManager::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    POWER_HILOGW(FEATURE_POWER_MODE, "On remote died");
    RETURN_IF(remote.promote() == nullptr);
    RemoveCallback(iface_cast<IPowerModeCallback>(remote.promote()));
}

void PowerModeModule::CallbackManager::WaitingCallback()
{
    POWER_HILOGD(FEATURE_POWER_MODE, "Mode callback started");
    unique_lock<mutex> lock(mutex_);
    for (auto& obj: callbacks_) {
        sptr<IPowerModeCallback> callback = iface_cast<IPowerModeCallback>(obj);
        if (callback != nullptr) {
            POWER_HILOGD(FEATURE_POWER_MODE, "Call IPowerModeCallback");
            PowerMode mode = PowerMode::NORMAL_MODE;
            callback->OnPowerModeChanged(mode);
        }
    }
}

void PowerModeModule::PublishPowerModeEvent()
{
    POWER_HILOGD(FEATURE_POWER_MODE, "Publish power mode module event");
    /* send event */
    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(false);
    std::string action;
    uint32_t code;
    std::string data;
    switch (mode_) {
        case PowerMode::PERFORMANCE_MODE:
            action = CommonEventSupport::COMMON_EVENT_POWER_SAVE_MODE_CHANGED;
            code = static_cast<uint32_t>(PowerMode::PERFORMANCE_MODE);
            data = ToString(static_cast<uint32_t>(PowerMode::PERFORMANCE_MODE));
            break;
        case PowerMode::NORMAL_MODE:
            action = CommonEventSupport::COMMON_EVENT_POWER_SAVE_MODE_CHANGED;
            code = static_cast<uint32_t>(PowerMode::NORMAL_MODE);
            data = ToString(static_cast<uint32_t>(PowerMode::NORMAL_MODE));
            break;
        case PowerMode::POWER_SAVE_MODE:
            action = CommonEventSupport::COMMON_EVENT_POWER_SAVE_MODE_CHANGED;
            code = static_cast<uint32_t>(PowerMode::POWER_SAVE_MODE);
            data = ToString(static_cast<uint32_t>(PowerMode::POWER_SAVE_MODE));
            break;
        case PowerMode::EXTREME_POWER_SAVE_MODE:
            action = CommonEventSupport::COMMON_EVENT_POWER_SAVE_MODE_CHANGED;
            code = static_cast<uint32_t>(PowerMode::EXTREME_POWER_SAVE_MODE);
            data = ToString(static_cast<uint32_t>(PowerMode::EXTREME_POWER_SAVE_MODE));
            break;
        default:
            POWER_HILOGW(FEATURE_POWER_MODE, "Unknown mode");
            return;
    }
    IntentWant setModeWant;
    setModeWant.SetAction(action);
    CommonEventData event(setModeWant);
    event.SetCode(code);
    event.SetData(data);
    if (!CommonEventManager::PublishCommonEvent(event, publishInfo, nullptr)) {
        POWER_HILOGE(FEATURE_POWER_MODE, "Failed to publish the mode event");
        return;
    }
    POWER_HILOGD(FEATURE_POWER_MODE, "Publish power mode module event end");
}

void PowerModeModule::RunAction(bool isBoot)
{
    POWER_HILOGD(FEATURE_POWER_MODE, "Run action");
    auto policy = DelayedSingleton<PowerModePolicy>::GetInstance();
    policy->TriggerAllActions(isBoot);
}

void PowerModeModule::SetDisplayOffTime(bool isBoot)
{
    if (isBoot && SettingHelper::IsDisplayOffTimeSettingValid()) {
        return;
    }
    int32_t time = DelayedSingleton<PowerModePolicy>::GetInstance()
        ->GetPowerModeValuePolicy(PowerModePolicy::ServiceType::DISPLAY_OFFTIME);
    if (time == INIT_VALUE_FALSE) {
        return;
    }
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    POWER_HILOGD(FEATURE_POWER_MODE, "Set display off timeout: %{public}d", time);
    bool needUpdateSetting = time > 0;
    pms->GetPowerStateMachine()->SetDisplayOffTime(static_cast<int64_t>(time), needUpdateSetting);
}

void PowerModeModule::SetSleepTime([[maybe_unused]] bool isBoot)
{
    int32_t time = DelayedSingleton<PowerModePolicy>::GetInstance()
        ->GetPowerModeValuePolicy(PowerModePolicy::ServiceType::SLEEPTIME);
    if (time == INIT_VALUE_FALSE) {
        return;
    }
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    POWER_HILOGD(FEATURE_POWER_MODE, "Set sleep timeout: %{public}d", time);
    pms->GetPowerStateMachine()->SetSleepTime(static_cast<int64_t>(time));
}

void PowerModeModule::SetAutoAdjustBrightness(bool isBoot)
{
    if (isBoot && SettingHelper::IsAutoAdjustBrightnessSettingValid()) {
        return;
    }
    int32_t value = DelayedSingleton<PowerModePolicy>::GetInstance()
        ->GetPowerModeValuePolicy(PowerModePolicy::ServiceType::AUTO_ADJUST_BRIGHTNESS);
    auto status = static_cast<SettingHelper::SwitchStatus>(value);
    POWER_HILOGD(FEATURE_POWER_MODE, "Set auto adjust brightness status: %{public}d", status);
    if (value == INIT_VALUE_FALSE) {
        return;
    }
    SettingHelper::SetSettingAutoAdjustBrightness(status);
}

void PowerModeModule::SetLcdBrightness(bool isBoot)
{
    if (isBoot && SettingHelper::IsBrightnessSettingValid()) {
        return;
    }
    int32_t lcdBrightness = DelayedSingleton<PowerModePolicy>::GetInstance()
        ->GetPowerModeValuePolicy(PowerModePolicy::ServiceType::LCD_BRIGHTNESS);
    POWER_HILOGD(FEATURE_POWER_MODE, "Set lcd Brightness: %{public}d", lcdBrightness);
    if (lcdBrightness == INIT_VALUE_FALSE) {
        return;
    }
    SettingHelper::SetSettingBrightness(lcdBrightness);
#ifdef HAS_DISPLAY_MANAGER
    OHOS::DisplayPowerMgr::DisplayPowerMgrClient::GetInstance().SetBrightness(lcdBrightness);
#endif
}

void PowerModeModule::SetVibration(bool isBoot)
{
    if (isBoot && SettingHelper::IsVibrationSettingValid()) {
        return;
    }
    int32_t vibration = DelayedSingleton<PowerModePolicy>::GetInstance()
        ->GetPowerModeValuePolicy(PowerModePolicy::ServiceType::VIBRATORS_STATE);
    POWER_HILOGD(FEATURE_POWER_MODE, "Set vibrate state %{public}d", vibration);
    if (vibration == INIT_VALUE_FALSE) {
        return;
    }
    SettingHelper::SetSettingVibration(static_cast<SettingHelper::SwitchStatus>(vibration));
}

void PowerModeModule::SetWindowRotation(bool isBoot)
{
    if (isBoot && SettingHelper::IsWindowRotationSettingValid()) {
        return;
    }
    int32_t rotation = DelayedSingleton<PowerModePolicy>::GetInstance()
        ->GetPowerModeValuePolicy(PowerModePolicy::ServiceType::AUTO_WINDOWN_RORATION);
    POWER_HILOGD(FEATURE_POWER_MODE, "Set window rotation state %{public}d", rotation);
    if (rotation == INIT_VALUE_FALSE) {
        return;
    }
    SettingHelper::SetSettingWindowRotation(static_cast<SettingHelper::SwitchStatus>(rotation));
}

void PowerModeModule::SetIntellVoiceState(bool isBoot)
{
    if (isBoot && SettingHelper::IsIntellVoiceSettingValid()) {
        return;
    }
    int32_t state = DelayedSingleton<PowerModePolicy>::GetInstance()
        ->GetPowerModeValuePolicy(PowerModePolicy::ServiceType::INTELL_VOICE);
    POWER_HILOGD(FEATURE_POWER_MODE, "Set intell voice state %{public}d", state);
    if (state == INIT_VALUE_FALSE) {
        return;
    }
    SettingHelper::SetSettingIntellVoice(static_cast<SettingHelper::SwitchStatus>(state));
}
} // namespace PowerMgr
} // namespace OHOS
