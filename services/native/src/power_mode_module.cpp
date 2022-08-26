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

#include "power_mode_module.h"

#include "display_power_mgr_client.h"
#include "power_log.h"
#include "power_mode_policy.h"
#include "power_mgr_service.h"
#include "power_setting_helper.h"
#include "system_ability_definition.h"

#include "singleton.h"

using namespace std;
using namespace OHOS;
using namespace OHOS::AAFwk;
using namespace OHOS::EventFwk;
using namespace OHOS::DisplayPowerMgr;

namespace OHOS {
namespace PowerMgr {
PowerModeModule::PowerModeModule()
    : mode_(NORMAL_MODE), lastMode_(LAST_MODE_FLAG), started_(false)
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
    policy->AddAction(PowerModePolicy::ServiceType::SMART_BACKLIGHT, lcdBrightnessAction);
    PowerModePolicy::ModeAction vibrationAction = [&](bool isInit) { SetVibration(isInit); };
    policy->AddAction(PowerModePolicy::ServiceType::VIBRATORS_STATE, vibrationAction);
    PowerModePolicy::ModeAction onOffRotationAction = [&](bool isInit) { SetWindowRotation(isInit); };
    policy->AddAction(PowerModePolicy::ServiceType::AUTO_WINDOWN_RORATION, onOffRotationAction);
}

void PowerModeModule::SetModeItem(uint32_t mode)
{
    POWER_HILOGI(FEATURE_POWER_MODE, "mode_: %{public}u, mode: %{public}u", mode_, mode);

    /* Same as the previous mode */
    if (this->mode_ == mode) {
        return;
    }

    /* If it's a valid mode */
    if (mode < POWER_MODE_MIN || mode > POWER_MODE_MAX) {
        POWER_HILOGW(FEATURE_POWER_MODE, "Invalid mode %{public}d", mode);
        return;
    }

    /* start set mode thread */
    EnableMode(mode);
}

uint32_t PowerModeModule::GetModeItem()
{
    POWER_HILOGD(FEATURE_POWER_MODE, "mode_: %{public}u", mode_);
    /* get power mode */
    return mode_;
}

void PowerModeModule::EnableMode(uint32_t mode, bool isBoot)
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

    this->lastMode_ = mode;
    started_ = false;
}

void PowerModeModule::UpdateModepolicy()
{
    /* update policy */
    DelayedSingleton<PowerModePolicy>::GetInstance()->SetPowerModePolicy(this->mode_, this->lastMode_);
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
    POWER_HILOGD(FEATURE_POWER_MODE,
                 "object = %{public}p, callback = %{public}p, callbacks.size = %{public}zu, insertOk = %{public}d",
                 object.GetRefPtr(), callback.GetRefPtr(), callbacks_.size(), retIt.second);
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
    POWER_HILOGD(FEATURE_POWER_MODE, "object = %{public}p, callback = %{public}p, callbacks.size = %{public}zu,",
                 object.GetRefPtr(), callback.GetRefPtr(), callbacks_.size());
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
            POWER_HILOGD(FEATURE_POWER_MODE, "Call IPowerModeCallback: %{public}p", callback.GetRefPtr());
            callback->PowerModeCallback();
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
    switch (mode_) {
        case PowerModeModule::EXTREME_MODE:
            action = CommonEventSupport::COMMON_EVENT_DEVICE_IDLE_MODE_CHANGED;
            code = static_cast<uint32_t>(PowerModeModule::EXTREME_MODE);
            break;
        case PowerModeModule::NORMAL_MODE:
            action = CommonEventSupport::COMMON_EVENT_DEVICE_IDLE_MODE_CHANGED;
            code = static_cast<uint32_t>(PowerModeModule::NORMAL_MODE);
            break;
        case PowerModeModule::SAVE_MODE:
            action = CommonEventSupport::COMMON_EVENT_POWER_SAVE_MODE_CHANGED;
            code = static_cast<uint32_t>(PowerModeModule::SAVE_MODE);
            break;
        case PowerModeModule::LOWPOWER_MODE:
            action = CommonEventSupport::COMMON_EVENT_POWER_SAVE_MODE_CHANGED;
            code = static_cast<uint32_t>(PowerModeModule::LOWPOWER_MODE);
            break;
        default:
            POWER_HILOGW(FEATURE_POWER_MODE, "Unknown mode");
            return;
    }
    IntentWant setModeWant;
    setModeWant.SetAction(action);
    CommonEventData event(setModeWant);
    event.SetCode(code);
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
    if (isBoot && IsDisplayOffTimeSettingValid()) {
        return;
    }
    int32_t time = DelayedSingleton<PowerModePolicy>::GetInstance()
        ->GetPowerModeValuePolicy(PowerModePolicy::ServiceType::DISPLAY_OFFTIME);
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    POWER_HILOGD(FEATURE_POWER_MODE, "Set display off timeout: %{public}d", time);
    // TODO subscribe setting
    pms->GetPowerStateMachine()->SetDisplayOffTime(static_cast<int64_t>(time));
    SetSettingDisplayOffTime(time);
}

void PowerModeModule::SetSleepTime([[maybe_unused]] bool isBoot)
{
    int32_t time = DelayedSingleton<PowerModePolicy>::GetInstance()
        ->GetPowerModeValuePolicy(PowerModePolicy::ServiceType::SLEEPTIME);
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    POWER_HILOGD(FEATURE_POWER_MODE, "Set sleep timeout: %{public}d", time);
    pms->GetPowerStateMachine()->SetSleepTime(static_cast<int64_t>(time));
}

void PowerModeModule::SetAutoAdjustBrightness(bool isBoot)
{
    if (isBoot && IsAutoAdjustBrightnessSettingValid()) {
        return;
    }
    int32_t value = DelayedSingleton<PowerModePolicy>::GetInstance()
        ->GetPowerModeValuePolicy(PowerModePolicy::ServiceType::AUTO_ADJUST_BRIGHTNESS);
    auto status = static_cast<SwitchStatus>(value);
    POWER_HILOGI(FEATURE_POWER_MODE, "status: %{public}d", status);
    SetSettingAutoAdjustBrightness(status);
}

void PowerModeModule::SetLcdBrightness(bool isBoot)
{
    if (isBoot && IsBrightnessSettingValid()) {
        return;
    }
    int32_t lcdBrightness = DelayedSingleton<PowerModePolicy>::GetInstance()
        ->GetPowerModeValuePolicy(PowerModePolicy::ServiceType::SMART_BACKLIGHT);
    POWER_HILOGD(FEATURE_POWER_MODE, "lcdBrightness: %{public}d", lcdBrightness);
    SetSettingBrightness(lcdBrightness);
}

void PowerModeModule::SetVibration(bool isBoot)
{
    if (isBoot && IsVibrationSettingValid()) {
        return;
    }
    int32_t vibration = DelayedSingleton<PowerModePolicy>::GetInstance()
        ->GetPowerModeValuePolicy(PowerModePolicy::ServiceType::VIBRATORS_STATE);
    POWER_HILOGD(FEATURE_POWER_MODE, "GetPowerModeValuePolicy vibrate=%{public}d", vibration);
    SetSettingVibration(static_cast<SwitchStatus>(vibration));
}

void PowerModeModule::SetWindowRotation(bool isBoot)
{
    if (isBoot && IsWindowRotationSettingValid()) {
        return;
    }
    int32_t rotation = DelayedSingleton<PowerModePolicy>::GetInstance()
        ->GetPowerModeValuePolicy(PowerModePolicy::ServiceType::AUTO_WINDOWN_RORATION);
    POWER_HILOGD(FEATURE_POWER_MODE, "GetPowerModeValuePolicy rotation=%{public}d", rotation);
    SetSettingWindowRotation(static_cast<SwitchStatus>(rotation));
}

bool PowerModeModule::IsBrightnessSettingValid()
{
    return PowerSettingHelper::GetInstance(POWER_MANAGER_SERVICE_ID).IsValidKey(SETTING_VIBRATION_KEY);
}

void PowerModeModule::SetSettingBrightness(int32_t brightness)
{
    PowerSettingHelper& helper = PowerSettingHelper::GetInstance(POWER_MANAGER_SERVICE_ID);
    ErrCode ret = helper.PutIntValue(SETTING_BRIGHTNESS_KEY, brightness);
    if (ret != ERR_OK) {
        POWER_HILOGW(FEATURE_POWER_MODE, "set setting brightness failed, brightness=%{public}d, ret=%{public}d",
                     brightness, ret);
    }
}

bool PowerModeModule::IsDisplayOffTimeSettingValid()
{
    return PowerSettingHelper::GetInstance(POWER_MANAGER_SERVICE_ID).IsValidKey(SETTING_DISPLAY_OFF_TIME_KEY);
}

void PowerModeModule::SetSettingDisplayOffTime(int32_t time)
{
    PowerSettingHelper& helper = PowerSettingHelper::GetInstance(POWER_MANAGER_SERVICE_ID);
    ErrCode ret = helper.PutIntValue(SETTING_DISPLAY_OFF_TIME_KEY, time);
    if (ret != ERR_OK) {
        POWER_HILOGW(FEATURE_POWER_MODE, "set setting display off time failed, time=%{public}d, ret=%{public}d",
                     time, ret);
    }
}

bool PowerModeModule::IsAutoAdjustBrightnessSettingValid()
{
    return PowerSettingHelper::GetInstance(POWER_MANAGER_SERVICE_ID).IsValidKey(SETTING_AUTO_ADJUST_BRIGHTNESS_KEY);
}

void PowerModeModule::SetSettingAutoAdjustBrightness(SwitchStatus status)
{
    PowerSettingHelper& helper = PowerSettingHelper::GetInstance(POWER_MANAGER_SERVICE_ID);
    ErrCode ret = helper.PutIntValue(SETTING_AUTO_ADJUST_BRIGHTNESS_KEY, static_cast<int32_t>(status));
    if (ret != ERR_OK) {
        POWER_HILOGW(FEATURE_POWER_MODE, "set setting auto adjust brightness failed, " \
                    "status=%{public}d, ret=%{public}d", status, ret);
    }
}

bool PowerModeModule::IsVibrationSettingValid()
{
    return PowerSettingHelper::GetInstance(POWER_MANAGER_SERVICE_ID).IsValidKey(SETTING_VIBRATION_KEY);
}

void PowerModeModule::SetSettingVibration(SwitchStatus status)
{
    PowerSettingHelper& helper = PowerSettingHelper::GetInstance(POWER_MANAGER_SERVICE_ID);
    ErrCode ret = helper.PutIntValue(SETTING_VIBRATION_KEY, static_cast<int32_t>(status));
    if (ret != ERR_OK) {
        POWER_HILOGW(FEATURE_POWER_MODE, "set setting vibration failed, status=%{public}d, ret=%{public}d",
                     status, ret);
    }
}

bool PowerModeModule::IsWindowRotationSettingValid()
{
    return PowerSettingHelper::GetInstance(POWER_MANAGER_SERVICE_ID).IsValidKey(SETTING_WINDOW_ROTATION_KEY);
}

void PowerModeModule::SetSettingWindowRotation(SwitchStatus status)
{
    PowerSettingHelper& helper = PowerSettingHelper::GetInstance(POWER_MANAGER_SERVICE_ID);
    ErrCode ret = helper.PutIntValue(SETTING_WINDOW_ROTATION_KEY, static_cast<int32_t>(status));
    if (ret != ERR_OK) {
        POWER_HILOGW(FEATURE_POWER_MODE, "set setting window rotation failed, status=%{public}d, ret=%{public}d",
                     status, ret);
    }
}
} // namespace PowerMgr
} // namespace OHOS
