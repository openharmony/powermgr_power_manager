/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "screen_off_pre_controller.h"

#include <datetime_ex.h>

#include "power_log.h"
#include "power_mgr_service.h"
#include "setting_helper.h"
#include "sp_singleton.h"

namespace OHOS {
namespace PowerMgr {
namespace {
    const uint32_t SCREEN_OFF_PRE_STATE = 1;
}

ScreenOffPreController::ScreenOffPreController(std::shared_ptr<PowerStateMachine>& stateMachine)
{
    callbackMgr_ = new (std::nothrow) CallbackMgr();
    queue_ = std::make_shared<FFRTQueue>("screen_off_pre_controller");
    powerStateMachine_ = stateMachine;
}

void ScreenOffPreController::Init()
{
    POWER_HILOGI(FEATURE_SCREEN_OFF_PRE, "Init start");
}

bool ScreenOffPreController::IsRegistered()
{
    return isRegistered_;
}

void ScreenOffPreController::AddScreenStateCallback(int32_t remainTime, const sptr<IScreenOffPreCallback>& callback)
{
    POWER_HILOGD(FEATURE_SCREEN_OFF_PRE, "remainTime=%{public}d", remainTime);
    remainTime_ = remainTime;
    if (callbackMgr_ != nullptr) {
        callbackMgr_->AddCallback(callback);
        isRegistered_ = true;
        if (powerStateMachine_ != nullptr) {
            int64_t systemTime = powerStateMachine_->GetDisplayOffTime();
            auto settingTime = SettingHelper::GetSettingDisplayOffTime(systemTime);
            POWER_HILOGD(FEATURE_SCREEN_OFF_PRE,
                "systemTime=%{public}lld,settingTime=%{public}lld",
                static_cast<long long>(systemTime), static_cast<long long>(settingTime));
            SchedulEyeDetectTimeout(powerStateMachine_->GetLastOnTime() + settingTime, GetTickCount());
        }
    }
}

void ScreenOffPreController::DelScreenStateCallback(const sptr<IScreenOffPreCallback>& callback)
{
    POWER_HILOGD(FEATURE_SCREEN_OFF_PRE, "DelScreenStateCallback enter");
    if (callbackMgr_ != nullptr) {
        callbackMgr_->RemoveCallback(callback);
        isRegistered_ = false;
    }
}

void ScreenOffPreController::Reset()
{
    if (queue_) {
        queue_.reset();
    }
}

void ScreenOffPreController::CallbackMgr::AddCallback(const sptr<IScreenOffPreCallback>& callback)
{
    std::unique_lock lock(mutex_);
    RETURN_IF((callback == nullptr) || (callback->AsObject() == nullptr));
    auto object = callback->AsObject();
    auto retIt = callbackSet_.insert(object);
    if (retIt.second) {
        object->AddDeathRecipient(this);
    }
}

void ScreenOffPreController::CallbackMgr::RemoveCallback(const sptr<IScreenOffPreCallback>& callback)
{
    std::unique_lock lock(mutex_);
    RETURN_IF((callback == nullptr) || (callback->AsObject() == nullptr));
    auto object = callback->AsObject();
    auto it = find(callbackSet_.begin(), callbackSet_.end(), object);
    if (it != callbackSet_.end()) {
        callbackSet_.erase(it);
        object->RemoveDeathRecipient(this);
    }
}

void ScreenOffPreController::CallbackMgr::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    POWER_HILOGW(FEATURE_SCREEN_OFF_PRE, "OnRemoteDied");
    RETURN_IF(remote.promote() == nullptr);
    RemoveCallback(iface_cast<IScreenOffPreCallback>(remote.promote()));
}

void ScreenOffPreController::CallbackMgr::Notify()
{
    std::unique_lock lock(mutex_);
    for (auto& obj : callbackSet_) {
        sptr<IScreenOffPreCallback> callback = iface_cast<IScreenOffPreCallback>(obj);
        if (callback != nullptr) {
            callback->OnScreenStateChanged(SCREEN_OFF_PRE_STATE);
        }
    }
}

void ScreenOffPreController::TriggerCallback()
{
    POWER_HILOGD(FEATURE_SCREEN_OFF_PRE, "TriggerCallback");
    if (callbackMgr_) {
        callbackMgr_->Notify();
    }
}

void ScreenOffPreController::SchedulEyeDetectTimeout(int64_t nextTimeOut, int64_t now)
{
    if (remainTime_ <= 0) {
        POWER_HILOGE(FEATURE_SCREEN_OFF_PRE, "remainTime_ is invalid:%{public}d", remainTime_);
        return;
    }
    int64_t nextEyeDetectTime = nextTimeOut - remainTime_ - now;
    if (NeedEyeDetectLocked(nextEyeDetectTime)) {
        CancelEyeDetectTimeout();
        HandleEyeDetectTimeout(nextEyeDetectTime);
    }
}

bool ScreenOffPreController::NeedEyeDetectLocked(int64_t nextEyeDetectTime)
{
    if (nextEyeDetectTime < 0) {
        POWER_HILOGI(FEATURE_SCREEN_OFF_PRE, "nextEyeDetectTime<0");
        return false;
    }
    return true;
}

void ScreenOffPreController::HandleEyeDetectTimeout(int64_t delayTime)
{
    POWER_HILOGD(FEATURE_SCREEN_OFF_PRE,
        "HandleEyeDetectTimeout delayTime=%{public}lld", static_cast<long long>(delayTime));
    std::lock_guard lock(ffrtMutex_);
    FFRTTask handletask = std::bind(&ScreenOffPreController::TriggerCallback, this);
    if (!queue_) {
        POWER_HILOGE(FEATURE_SCREEN_OFF_PRE, "queue_ is nullptr");
        return;
    }
    eyeDetectTimeOutHandle_ = FFRTUtils::SubmitDelayTask(handletask, delayTime, queue_);
}

void ScreenOffPreController::CancelEyeDetectTimeout()
{
    std::lock_guard lock(ffrtMutex_);
    if (!queue_) {
        POWER_HILOGE(FEATURE_SCREEN_OFF_PRE, "queue_ is nullptr");
        return;
    }
    FFRTUtils::CancelTask(eyeDetectTimeOutHandle_, queue_);
}

} // namespace PowerMgr
} // namespace OHOS
