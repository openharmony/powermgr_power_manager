/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef POWERMGR_POWER_MANAGER_SETTING_HELPER_H
#define POWERMGR_POWER_MANAGER_SETTING_HELPER_H

#include <cstdint>
#include <string>

#include "setting_provider.h"

namespace OHOS {
namespace PowerMgr {
class SettingHelper {
public:
    enum class SwitchStatus : int32_t {
        INVALID = -1,
        DISABLE = 0,
        ENABLE = 1,
    };
    static bool IsDisplayOffTimeSettingValid();
    static int64_t GetSettingDisplayOffTime(int64_t defaultVal);
    static void SetSettingDisplayOffTime(int64_t time);
    static sptr<SettingObserver> RegisterSettingDisplayOffTimeObserver(SettingObserver::UpdateFunc& func);
    static bool IsAutoAdjustBrightnessSettingValid();
    static int32_t GetSettingAutoAdjustBrightness(int32_t defaultVal);
    static void SetSettingAutoAdjustBrightness(SwitchStatus status);
    static sptr<SettingObserver> RegisterSettingAutoAdjustBrightnessObserver(SettingObserver::UpdateFunc& func);
    static bool IsBrightnessSettingValid();
    static void SetSettingBrightness(int32_t brightness);
    static bool IsVibrationSettingValid();
    static int32_t GetSettingVibration(int32_t defaultVal);
    static void SetSettingVibration(SwitchStatus status);
    static sptr<SettingObserver> RegisterSettingVibrationObserver(SettingObserver::UpdateFunc& func);
    static bool IsWindowRotationSettingValid();
    static int32_t GetSettingWindowRotation(int32_t defaultVal);
    static void SetSettingWindowRotation(SwitchStatus status);
    static sptr<SettingObserver> RegisterSettingWindowRotationObserver(SettingObserver::UpdateFunc& func);
    static bool IsIntellVoiceSettingValid();
    static int32_t GetSettingIntellVoice(int32_t defaultVal);
    static void SetSettingIntellVoice(SwitchStatus status);
    static sptr<SettingObserver> RegisterSettingIntellVoiceObserver(SettingObserver::UpdateFunc& func);
    static sptr<SettingObserver> RegisterSettingSuspendSourcesObserver(SettingObserver::UpdateFunc& func);
    static bool IsSuspendSourcesSettingValid();
    static const std::string GetSettingSuspendSources();
    static void SetSettingSuspendSources(const std::string& jsonConfig);
    static sptr<SettingObserver> RegisterSettingWakeupSourcesObserver(SettingObserver::UpdateFunc& func);
    static bool IsWakeupSourcesSettingValid();
    static const std::string GetSettingWakeupSources();
    static void SetSettingWakeupSources(const std::string& jsonConfig);

    static void UnregisterSettingObserver(sptr<SettingObserver>& observer);

private:
    static constexpr const char* SETTING_DISPLAY_OFF_TIME_KEY {"settings.display.screen_off_timeout"};
    static constexpr const char* SETTING_AUTO_ADJUST_BRIGHTNESS_KEY {"settings.display.auto_screen_brightness"};
    static constexpr const char* SETTING_BRIGHTNESS_KEY {"settings.display.screen_brightness_status"};
    static constexpr const char* SETTING_VIBRATION_KEY {"physic_navi_haptic_feedback_enabled"};
    static constexpr const char* SETTING_WINDOW_ROTATION_KEY {"settings.general.accelerometer_rotation_status"};
    static constexpr const char* SETTING_POWER_SUSPEND_SOURCES_KEY {"settings.power.suspend_sources"};
    static constexpr const char* SETTING_POWER_WAKEUP_SOURCES_KEY {"settings.power.wakeup_sources"};
    static constexpr const char* SETTING_INTELL_VOICE_KEY {"intell_voice_trigger_enabled"};
};
} // namespace PowerMgr
} // namespace OHOS

#endif // POWERMGR_POWER_MANAGER_SETTING_HELPER_H
