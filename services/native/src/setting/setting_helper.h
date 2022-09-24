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
    static int64_t GetSettingDisplayOffTime();
    static void SetSettingDisplayOffTime(int64_t time);
    static sptr<SettingObserver> RegisterSettingDisplayOffTimeObserver(SettingObserver::UpdateFunc& func);
    static void UnregisterSettingDisplayOffTimeObserver(sptr<SettingObserver>& observer);
    static bool IsAutoAdjustBrightnessSettingValid();
    static void SetSettingAutoAdjustBrightness(SwitchStatus status);
    static bool IsBrightnessSettingValid();
    static void SetSettingBrightness(int32_t brightness);
    static bool IsVibrationSettingValid();
    static void SetSettingVibration(SwitchStatus status);
    static bool IsWindowRotationSettingValid();
    static void SetSettingWindowRotation(SwitchStatus status);

private:
    static constexpr const char* SETTING_DISPLAY_OFF_TIME_KEY {"settings.display.screen_off_timeout"};
    static constexpr const char* SETTING_AUTO_ADJUST_BRIGHTNESS_KEY {"settings.display.auto_screen_brightness"};
    static constexpr const char* SETTING_BRIGHTNESS_KEY {"settings.display.screen_brightness_status"};
    static constexpr const char* SETTING_VIBRATION_KEY {"settings.sound.vibrate_status"};
    static constexpr const char* SETTING_WINDOW_ROTATION_KEY {"settings.display.default_screen_rotation"};
};
} // namespace PowerMgr
} // namespace OHOS

#endif // POWERMGR_POWER_MANAGER_SETTING_HELPER_H
