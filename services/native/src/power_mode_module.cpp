/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "hilog_wrapper.h"

#include "display_power_mgr_client.h"
#include "power_common.h"
#include "power_mode_policy.h"
#include "power_mgr_service.h"
#include "power_save_mode.h"
#include "power_state_machine.h"

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
    POWER_HILOGI(MODULE_SERVICE, "PowerModeModule create");
    callbackMgr_ = new CallbackManager();
    auto policy = DelayedSingleton<PowerModePolicy>::GetInstance();
    policy->AddAction(PowerModePolicy::ServiceType::DISPLAY_OFFTIME,
        std::bind(&PowerModeModule::SetDisplayOffTime, this));
    policy->AddAction(PowerModePolicy::ServiceType::SLEEPTIME,
        std::bind(&PowerModeModule::SetSleepTime, this));
    policy->AddAction(PowerModePolicy::ServiceType::AUTO_ADJUST_BRIGHTNESS,
        std::bind(&PowerModeModule::SetAutoAdjustBrightness, this));
    policy->AddAction(PowerModePolicy::ServiceType::SMART_BACKLIGHT,
        std::bind(&PowerModeModule::SetLcdBrightness, this));
    policy->AddAction(PowerModePolicy::ServiceType::VIBRATORS_STATE,
        std::bind(&PowerModeModule::SetVibration, this));
    policy->AddAction(PowerModePolicy::ServiceType::AUTO_WINDOWN_RORATION,
        std::bind(&PowerModeModule::OnOffRotation, this));
}

void PowerModeModule::SetModeItem(uint32_t mode)
{
    POWER_HILOGD(MODULE_SERVICE, "Set Mode Item : %{public}u", mode);

    /* Same as the previous mode */
    if (this->mode_ == mode) {
        return;
    }

    /* If it's a valid mode */
    if (mode < POWER_MODE_MIN || mode > POWER_MODE_MAX) {
        POWER_HILOGE(MODULE_SERVICE, "Unknow mode %{public}d", mode);
        return;
    }

    /* start set mode thread */
    EnableMode(mode);
}

uint32_t PowerModeModule::GetModeItem()
{
    POWER_HILOGD(MODULE_SERVICE, "Get Mode Item : %{public}u", mode_);
    /* get power mode */
    return mode_;
}

void PowerModeModule::EnableMode(uint32_t mode)
{
    if (started_) {
        POWER_HILOGE(MODULE_SERVICE, "Power Mode is already running.");
        return;
    }

    started_ = true;
    mode_ = mode;

    /* Update power mode policy */
    UpdateModepolicy();

    /* Send state change */
    Prepare();

    /* Set action */
    RunAction();

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
    POWER_HILOGI(MODULE_SERVICE, "callbacks.size = %{public}zu, insertOk = %{public}d", callbacks_.size(), retIt.second);
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
    POWER_HILOGI(MODULE_SERVICE, "callbacks.size = %{public}zu,", callbacks_.size());
}

void PowerModeModule::CallbackManager::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    POWER_HILOGI(MODULE_SERVICE, "OnRemoteDied");
    RETURN_IF(remote.promote() == nullptr);
    RemoveCallback(iface_cast<IPowerModeCallback>(remote.promote()));
}

void PowerModeModule::CallbackManager::WaitingCallback()
{
    POWER_HILOGD(MODULE_SERVICE, "mode callback started.");
    unique_lock<mutex> lock(mutex_);
    for (auto &obj : callbacks_) {
        sptr<IPowerModeCallback> callback = iface_cast<IPowerModeCallback>(obj);
        if (callback != nullptr) {
            POWER_HILOGD(MODULE_SERVICE, "callback->PowerModeCallback()");
            callback->PowerModeCallback();
        }
    }
}

void PowerModeModule::PublishPowerModeEvent()
{
    POWER_HILOGD(MODULE_SERVICE, "Start of publishing mode event");
    /* send event */
    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(false);
    IntentWant setModeWant;
    CommonEventData event(setModeWant);
    switch (mode_) {
        case PowerModeModule::EXTREME_MODE:
            setModeWant.SetAction(CommonEventSupport::COMMON_EVENT_DEVICE_IDLE_MODE_CHANGED);
            event.SetCode(PowerModeModule::EXTREME_MODE);
            break;
        case PowerModeModule::NORMAL_MODE:
            setModeWant.SetAction(CommonEventSupport::COMMON_EVENT_DEVICE_IDLE_MODE_CHANGED);
            event.SetCode(PowerModeModule::NORMAL_MODE);
            break;
        case PowerModeModule::SAVE_MODE:
            setModeWant.SetAction(CommonEventSupport::COMMON_EVENT_POWER_SAVE_MODE_CHANGED);
            event.SetCode(PowerModeModule::SAVE_MODE);
            break;
        case PowerModeModule::LOWPOWER_MODE:
            setModeWant.SetAction(CommonEventSupport::COMMON_EVENT_POWER_SAVE_MODE_CHANGED);
            event.SetCode(PowerModeModule::LOWPOWER_MODE);
            break;
        default:
            POWER_HILOGE(MODULE_SERVICE, "Unknow mode");
            return;
    }
    if (!CommonEventManager::PublishCommonEvent(event, publishInfo, nullptr)) {
        POWER_HILOGE(MODULE_SERVICE, "Failed to publish the mode event!");
        return;
    }
    POWER_HILOGD(MODULE_SERVICE, "End of publishing mode event");
}

void PowerModeModule::RunAction()
{
    POWER_HILOGI(MODULE_SERVICE, "PowerModeModule::RunAction");
    auto policy = DelayedSingleton<PowerModePolicy>::GetInstance();
    policy->TriggerAllActions();
    return;
}

void PowerModeModule::SetDisplayOffTime()
{
    POWER_HILOGI(MODULE_SERVICE, "PowerModeModule::SetDisplayOffTime");
    int32_t time = DelayedSingleton<PowerModePolicy>::GetInstance()
        ->GetPowerModeValuePolicy(PowerModePolicy::ServiceType::DISPLAY_OFFTIME);
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        POWER_HILOGI(MODULE_SERVICE, "No service instance");
        return;
    }
    pms->GetPowerStateMachine()->SetDisplayOffTime(static_cast<int64_t>(time));
}

void PowerModeModule::SetSleepTime()
{
    POWER_HILOGI(MODULE_SERVICE, "PowerModeModule::SetSleepTime");
    int32_t time = DelayedSingleton<PowerModePolicy>::GetInstance()
        ->GetPowerModeValuePolicy(PowerModePolicy::ServiceType::SLEEPTIME);
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        POWER_HILOGI(MODULE_SERVICE, "No service instance");
        return;
    }
    pms->GetPowerStateMachine()->SetSleepTime(static_cast<int64_t>(time));
}

void PowerModeModule::SetAutoAdjustBrightness()
{
    POWER_HILOGI(MODULE_SERVICE, "PowerModeModule::SetAutoAdjustBrightness");
    bool enable = false;
    int32_t value = DelayedSingleton<PowerModePolicy>::GetInstance()
        ->GetPowerModeValuePolicy(PowerModePolicy::ServiceType::AUTO_ADJUST_BRIGHTNESS);
    if (value != FLAG_FALSE) {
        enable = true;
    }
    bool ret = DisplayPowerMgrClient::GetInstance().AutoAdjustBrightness(enable);
    POWER_HILOGI(MODULE_SERVICE, "SetAutoAdjustBrightness: %{public}d, result=%{public}d", enable, ret);
}

void PowerModeModule::SetLcdBrightness()
{
    POWER_HILOGD(MODULE_SERVICE, "set lcd brightness");
    int32_t lcdBrightness = DelayedSingleton<PowerModePolicy>::GetInstance()
        ->GetPowerModeValuePolicy(PowerModePolicy::ServiceType::SMART_BACKLIGHT);
    POWER_HILOGD(MODULE_SERVICE, "GetPowerModeValuePolicy lcdBrightness=%{public}d", lcdBrightness);
    if (lcdBrightness != FLAG_FALSE) {
        // set lastmode value to recoverValue
        if (lastMode_ == LAST_MODE_FLAG) {
            POWER_HILOGD(MODULE_SERVICE, "first set lcdBrightness=%{public}d", lcdBrightness);
            recoverValue[PowerModePolicy::ServiceType::SMART_BACKLIGHT] = lcdBrightness;
        } else {
            // get value from setting privider value
            POWER_HILOGD(MODULE_SERVICE, "Setting lcdBrightness=%{public}d", SETTINGS_PRIVIDER_VALUE_LCD_BRIGHTNESS);
            recoverValue[PowerModePolicy::ServiceType::SMART_BACKLIGHT] = SETTINGS_PRIVIDER_VALUE_LCD_BRIGHTNESS;
        }
        // set lcd brightness
        int32_t dispId = DisplayPowerMgrClient::GetInstance().GetMainDisplayId();
        bool ret = DisplayPowerMgrClient::GetInstance().SetBrightness(static_cast<uint32_t>(lcdBrightness), dispId);
        POWER_HILOGI(MODULE_SERVICE, "SetBrightness: %{public}d, result=%{public}d", lcdBrightness, ret);
    } else {
        lcdBrightness = DelayedSingleton<PowerModePolicy>::GetInstance()
            ->GetPowerModeRecoverPolicy(PowerModePolicy::ServiceType::SMART_BACKLIGHT);
        POWER_HILOGD(MODULE_SERVICE, "GetPowerModeRecoverPolicy lcdBrightness=%{public}d", lcdBrightness);
        if (lcdBrightness != FLAG_FALSE) {
            // get recoverValue
            std::lock_guard<std::mutex> lock(mutex_);
            recoverValueiter = recoverValue.find(PowerModePolicy::ServiceType::SMART_BACKLIGHT);
            if (recoverValueiter != recoverValue.end()) {
                lcdBrightness = recoverValueiter->second;
                POWER_HILOGD(MODULE_SERVICE, "Get recovervalue lcdBrightness=%{public}d", lcdBrightness);
                // delete map
                recoverValue.erase(recoverValueiter);
            }
            POWER_HILOGD(MODULE_SERVICE, "please set lcdBrightness");
        }
    }
    return;
}

void PowerModeModule::SetVibration()
{
    POWER_HILOGD(MODULE_SERVICE, "set vibration");
    int32_t vibration = DelayedSingleton<PowerModePolicy>::GetInstance()
        ->GetPowerModeValuePolicy(PowerModePolicy::ServiceType::VIBRATORS_STATE);
    POWER_HILOGD(MODULE_SERVICE, "GetPowerModeValuePolicy    vibrate=%{public}d", vibration);
    if (vibration != FLAG_FALSE) {
        // set lastmode value to recoverValue
        if (lastMode_ == LAST_MODE_FLAG) {
            POWER_HILOGD(MODULE_SERVICE, "first set vibration=%{public}d", vibration);
            recoverValue[PowerModePolicy::ServiceType::VIBRATORS_STATE] = vibration;
        } else {
            // get value from setting privider value
            POWER_HILOGD(MODULE_SERVICE, "Setting vibration=%{public}d", SETTINGS_PRIVIDER_VALUE_VIBRATION);
            recoverValue[PowerModePolicy::ServiceType::VIBRATORS_STATE] = SETTINGS_PRIVIDER_VALUE_VIBRATION;
        }
        // set  vibration
        POWER_HILOGD(MODULE_SERVICE, "please set vibration");
    } else {
        vibration = DelayedSingleton<PowerModePolicy>::GetInstance()
            ->GetPowerModeRecoverPolicy(PowerModePolicy::ServiceType::VIBRATORS_STATE);
        POWER_HILOGD(MODULE_SERVICE, "GetPowerModeRecoverPolicy    vibration=%{public}d", vibration);
        if (vibration != FLAG_FALSE) {
            // get recoverValue
            std::lock_guard<std::mutex> lock(mutex_);
            recoverValueiter = recoverValue.find(PowerModePolicy::ServiceType::VIBRATORS_STATE);
            if (recoverValueiter != recoverValue.end()) {
                vibration = recoverValueiter->second;
                POWER_HILOGD(MODULE_SERVICE, "Get recovervalue vibration=%{public}d", vibration);
                // delete map
                recoverValue.erase(recoverValueiter);
            }
            POWER_HILOGD(MODULE_SERVICE, "please set vibration");
        }
    }
    return;
}

void PowerModeModule::OnOffRotation()
{
    POWER_HILOGD(MODULE_SERVICE, "on or off rotation");
    int32_t rotation = DelayedSingleton<PowerModePolicy>::GetInstance()
        ->GetPowerModeValuePolicy(PowerModePolicy::ServiceType::AUTO_WINDOWN_RORATION);
    POWER_HILOGD(MODULE_SERVICE, "GetPowerModeValuePolicy    rotation=%{public}d", rotation);
    if (rotation != FLAG_FALSE) {
        // set lastmode value to recoverValue
        if (lastMode_ == LAST_MODE_FLAG) {
            POWER_HILOGD(MODULE_SERVICE, "first set rotation=%{public}d", rotation);
            recoverValue[PowerModePolicy::ServiceType::AUTO_WINDOWN_RORATION] = rotation;
        } else {
            // get value from setting privider value
            POWER_HILOGD(MODULE_SERVICE, "Setting rotation=%{public}d", SETTINGS_PRIVIDER_VALUE_ROTATION);
            recoverValue[PowerModePolicy::ServiceType::AUTO_WINDOWN_RORATION] = SETTINGS_PRIVIDER_VALUE_ROTATION;
        }
        // set lcd vibrate
        POWER_HILOGD(MODULE_SERVICE, "please on or off rotation");
    } else {
        rotation = DelayedSingleton<PowerModePolicy>::GetInstance()
            ->GetPowerModeRecoverPolicy(PowerModePolicy::ServiceType::AUTO_WINDOWN_RORATION);
        POWER_HILOGD(MODULE_SERVICE, "GetPowerModeRecoverPolicy    rotation=%{public}d", rotation);
        if (rotation != FLAG_FALSE) {
            // get recoverValue
            std::lock_guard<std::mutex> lock(mutex_);
            recoverValueiter = recoverValue.find(PowerModePolicy::ServiceType::VIBRATORS_STATE);
            if (recoverValueiter != recoverValue.end()) {
                rotation = recoverValueiter->second;
                POWER_HILOGD(MODULE_SERVICE, "Get recovervalue rotation=%{public}d", rotation);
                // delete map
                recoverValue.erase(recoverValueiter);
            }
            POWER_HILOGD(MODULE_SERVICE, "please on or off rotation");
        }
    }
    return;
}
} // namespace PowerMgr
} // namespace OHOS
