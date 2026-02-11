/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "screen_saver_timer_manager.h"

#include "power_log.h"
#include "power_mgr_client.h"
#include "power_mgr_notify.h"
#include "setting_helper.h"

namespace OHOS {
namespace PowerMgr {

void ScreenSaverTimerManager::Init()
{
    screenSaverQueue_ = std::make_shared<FFRTQueue>("screenSaverQueue");
    std::lock_guard<ffrt::mutex> lock(plugMutex_);
    if (!plugStatsToScreenSaverTimer_) {
        plugStatsToScreenSaverTimer_ = std::make_shared<PowerPlugStatusManager>();
        plugStatsToScreenSaverTimer_->Init();
    }
    POWER_HILOGI(COMP_SVC, "Init screen saver timer manager done.");
    return;
}

void ScreenSaverTimerManager::UpdateScreenSaverTimer()
{
    int32_t settingTime = GetSettingScreenSaverTime();
    POWER_HILOGD(COMP_SVC, "Get setting screen saver time is %{public}d ms.", settingTime);
    SetScreenSaverTimer(settingTime);
    return;
}

void ScreenSaverTimerManager::CancelScreenSaverTimer()
{
    std::lock_guard<ffrt::mutex> lock(timerMutex_);
    CancelScreenSaverTimerLocked();
    POWER_HILOGD(COMP_SVC, "Cancel screen saver timer done.");
    return;
}

void ScreenSaverTimerManager::CancelScreenSaverTimerLocked()
{
    if (screenSaverHandle_ != nullptr) {
        FFRTUtils::CancelTask(screenSaverHandle_, screenSaverQueue_);
        screenSaverHandle_ = nullptr;
    }
}

void ScreenSaverTimerManager::SetScreenSaverTimer(int32_t screenSaverTime)
{
    if (screenSaverTime <= DISABLE_SCREEN_SAVER_TIME) {
        POWER_HILOGD(COMP_SVC, "Disable screen saver timer.");
        CancelScreenSaverTimer();
        return;
    }
    int32_t screenOffTime = GetSettingScreenOffTime();
    std::lock_guard<ffrt::mutex> lock(timerMutex_);
    if (IsScreenSaverTimeChanged(screenSaverTime)) {
        POWER_HILOGI(COMP_SVC, "The screen saver timer has been changed from %{public}d to %{public}d ms.",
            currentSettingTime_, screenSaverTime);
        currentSettingTime_ = screenSaverTime;
    }

    if (screenOffTime != NEVER_AUTO_SCREEN_OFF && screenOffTime <= screenSaverTime) {
        POWER_HILOGD(COMP_SVC, "ScreenOffTime %{public}d ms is less than screenSaverTime %{public}d ms.",
            screenOffTime, screenSaverTime);
        CancelScreenSaverTimerLocked();
        return;
    }

    CancelScreenSaverTimerLocked();
    POWER_HILOGD(COMP_SVC, "Start screen saver timing at %{public}d ms.", screenSaverTime);
    FFRTTask startScreenSaverTimer = [this] {
        auto& powerMgrClient = PowerMgrClient::GetInstance();
        bool isRunningLock = false;
        powerMgrClient.IsRunningLockEnabled(RunningLockType::RUNNINGLOCK_SCREEN, isRunningLock);
        bool isScreenOn = powerMgrClient.IsScreenOn();
        if (isRunningLock || !isScreenOn) {
            POWER_HILOGW(COMP_SVC, "Enable screen saver fail, isRunningLock %{public}d isScreen %{public}d.",
                isRunningLock, isScreenOn);
        } else {
            POWER_HILOGI(COMP_SVC, "Start to publish usual.event.power.START_DREAM.");
            AAFwk::Want want;
            want.SetAction("usual.event.power.START_DREAM");
            PowerMgrNotify::PublishCustomizedEvent(want);
        }
    };
    screenSaverHandle_ =
        FFRTUtils::SubmitDelayTask(startScreenSaverTimer, screenSaverTime, screenSaverQueue_);
    return;
}

int32_t ScreenSaverTimerManager::GetSettingScreenSaverTime()
{
    return IsPowerPluged() ?
        SettingHelper::GetSettingAcScreenSaverTime(DISABLE_SCREEN_SAVER_TIME) :
        SettingHelper::GetSettingDcScreenSaverTime(DISABLE_SCREEN_SAVER_TIME);
}

int32_t ScreenSaverTimerManager::GetSettingScreenOffTime()
{
#ifdef POWER_MANAGER_ENABLE_CHARGING_TYPE_SETTING
    return IsPowerPluged() ?
        SettingHelper::GetSettingDisplayAcScreenOffTime(DEFAULT_AC_DISPLAY_OFF_TIME_MS) :
        SettingHelper::GetSettingDisplayDcScreenOffTime(DEFAULT_DC_DISPLAY_OFF_TIME_MS);
#endif
    return IsPowerPluged() ? DEFAULT_AC_DISPLAY_OFF_TIME_MS : DEFAULT_DC_DISPLAY_OFF_TIME_MS;
}

bool ScreenSaverTimerManager::IsPowerPluged()
{
    std::lock_guard<ffrt::mutex> lock(plugMutex_);
    if (plugStatsToScreenSaverTimer_ == nullptr) {
        POWER_HILOGE(COMP_SVC, "plugStatsToScreenSaverTimer_ is nullptr.");
        return false;
    }
    return plugStatsToScreenSaverTimer_->IsPowerPluged();
}

void ScreenSaverTimerManager::SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus status)
{
    std::lock_guard<ffrt::mutex> lock(plugMutex_);
    if (plugStatsToScreenSaverTimer_ == nullptr) {
        POWER_HILOGE(COMP_SVC, "plugStatsToScreenSaverTimer_ is nullptr.");
        return;
    }
    plugStatsToScreenSaverTimer_->SetPowerPlugStatus(status);
}

} // namespace PowerMgr
} // namespace OHOS