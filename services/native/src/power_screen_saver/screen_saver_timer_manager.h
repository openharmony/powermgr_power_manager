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

#ifndef SCREEN_SAVER_TIMER_MANAGER_H
#define SCREEN_SAVER_TIMER_MANAGER_H

#include "power_plug_status_manager.h"
#include "ffrt_utils.h"

namespace OHOS {
namespace PowerMgr {

class ScreenSaverTimerManager {
public:
    ScreenSaverTimerManager() = default;
    void Init();
    void UpdateScreenSaverTimer();
    void CancelScreenSaverTimer();
    void SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus status);

private:
    void SetScreenSaverTimer(int32_t screenSaverTime);
    void CancelScreenSaverTimerLocked();
    int32_t GetSettingScreenSaverTime();
    int32_t GetSettingScreenOffTime();
    bool IsPowerPluged();
    std::shared_ptr<PowerPlugStatusManager> plugStatsToScreenSaverTimer_;
    FFRTHandle screenSaverHandle_;
    std::shared_ptr<FFRTQueue> screenSaverQueue_;
    static constexpr int32_t DEFAULT_AC_DISPLAY_OFF_TIME_MS = 600000;
    static constexpr int32_t DEFAULT_DC_DISPLAY_OFF_TIME_MS = 300000;
    static constexpr int32_t DISABLE_SCREEN_SAVER_TIME = -1;
    static constexpr int32_t NEVER_AUTO_SCREEN_OFF = -1;
    int32_t currentSettingTime_ = 0;
    ffrt::mutex timerMutex_;
    ffrt::mutex plugMutex_;
    bool IsScreenSaverTimeChanged(int32_t settingTme)
    {
        return settingTme != currentSettingTime_;
    }
};

} //namespace PowerMgr
} //namespace OHOS
#endif // SCREEN_SAVER_TIMER_MANAGER_H