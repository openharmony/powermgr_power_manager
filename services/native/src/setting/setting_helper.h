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
    static void UpdateCurrentUserId();
#ifdef POWER_PICKUP_ENABLE
    static void CopyDataForUpdateScene();
#endif
    static void UnregisterSettingObserver(sptr<SettingObserver>& observer);
    static bool IsDisplayOffTimeSettingValid();
    static bool IsSuspendSourcesSettingValid();
#ifdef POWER_MANAGER_ENABLE_CHARGING_TYPE_SETTING
    static bool IsSettingDisplayAcScreenOffTimeValid();
    static int64_t GetSettingDisplayAcScreenOffTime(int64_t defaultVal);
    static void SetSettingDisplayAcScreenOffTime(int64_t time);
    static sptr<SettingObserver> RegisterSettingDisplayAcScreenOffTimeObserver(SettingObserver::UpdateFunc& func);
    static bool IsSettingDisplayDcScreenOffTimeValid();
    static int64_t GetSettingDisplayDcScreenOffTime(int64_t defaultVal);
    static void SetSettingDisplayDcScreenOffTime(int64_t time);
    static sptr<SettingObserver> RegisterSettingDisplayDcScreenOffTimeObserver(SettingObserver::UpdateFunc& func);
    static bool IsSettingAcSuspendSourcesValid();
    static const std::string GetSettingAcSuspendSources();
    static void SetSettingAcSuspendSources(const std::string& jsonConfig);
    static sptr<SettingObserver> RegisterSettingAcSuspendSourcesObserver(SettingObserver::UpdateFunc& func);
    static bool IsSettingDcSuspendSourcesValid();
    static const std::string GetSettingDcSuspendSources();
    static void SetSettingDcSuspendSources(const std::string& jsonConfig);
    static sptr<SettingObserver> RegisterSettingDcSuspendSourcesObserver(SettingObserver::UpdateFunc& func);
#else
    static int64_t GetSettingDisplayOffTime(int64_t defaultVal);
    static void SetSettingDisplayOffTime(int64_t time);
    static sptr<SettingObserver> RegisterSettingDisplayOffTimeObserver(SettingObserver::UpdateFunc& func);
    static sptr<SettingObserver> RegisterSettingSuspendSourcesObserver(SettingObserver::UpdateFunc& func);
    static const std::string GetSettingSuspendSources();
    static void SetSettingSuspendSources(const std::string& jsonConfig);
#endif
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
    static sptr<SettingObserver> RegisterSettingWakeupSourcesObserver(SettingObserver::UpdateFunc& func);
    static bool IsWakeupSourcesSettingValid();
    static const std::string GetSettingWakeupSources();
    static void SetSettingWakeupSources(const std::string& jsonConfig);
    static bool IsWakeupDoubleSettingValid();
    static bool GetSettingWakeupDouble(const std::string& key = SETTING_POWER_WAKEUP_DOUBLE_KEY);
    static void SetSettingWakeupDouble(bool enable);
    static void RegisterSettingWakeupDoubleObserver(SettingObserver::UpdateFunc& func);
    static void UnregisterSettingWakeupDoubleObserver();
    static bool IsWakeupPickupSettingValid();
    static bool GetSettingWakeupPickup(const std::string& key = SETTING_POWER_WAKEUP_PICKUP_KEY);
    static void SetSettingWakeupPickup(bool enable);
    static void RegisterSettingWakeupPickupObserver(SettingObserver::UpdateFunc& func);
    static void UnregisterSettingWakeupPickupObserver();
    static void SaveCurrentMode(int32_t mode);
    static int32_t ReadCurrentMode(int32_t defaultMode);
    static const std::string ReadPowerModeRecoverMap();
    static void SavePowerModeRecoverMap(const std::string &jsonConfig);
    static void RegisterSettingPowerModeObserver(SettingObserver::UpdateFunc& func);
    static void UnRegisterSettingPowerModeObserver();
    static void RegisterSettingWakeupLidObserver(SettingObserver::UpdateFunc& func);
    static void UnRegisterSettingWakeupLidObserver();
    static bool GetSettingWakeupLid(const std::string& key = SETTING_POWER_WAKEUP_LID_KEY);
    static void SetSettingWakeupLid(bool enable);
    static bool IsWakeupLidSettingValid();
#ifdef POWER_MANAGER_ENABLE_BLOCK_LONG_PRESS
    static const std::string GetBlockLongPress();
#endif

private:
    static bool IsSettingKeyValid(const std::string& key);
    static int32_t GetSettingIntValue(const std::string& key, int32_t defaultVal);
    static void SetSettingIntValue(const std::string& key, int32_t value);
    static int64_t GetSettingLongValue(const std::string& key, int64_t defaultVal);
    static void SetSettingLongValue(const std::string& key, int64_t value);
    static const std::string GetSettingStringValue(const std::string& key);
    static void SetSettingJsonStringValue(const std::string& key, const std::string& jsonConfig);
    static sptr<SettingObserver> RegisterSettingKeyObserver(const std::string& key, SettingObserver::UpdateFunc& func);

#ifdef POWER_MANAGER_ENABLE_CHARGING_TYPE_SETTING
    // AC for Alternating Current, means charing supply
    // DC for Direct Current, means battery supply
    static constexpr const char* SETTING_DISPLAY_AC_OFF_TIME_KEY {"settings.display.ac.screen_off_timeout"};
    static constexpr const char* SETTING_DISPLAY_DC_OFF_TIME_KEY {"settings.display.dc.screen_off_timeout"};
    static constexpr const char* SETTING_POWER_AC_SUSPEND_SOURCES_KEY {"settings.power.ac.suspend_sources"};
    static constexpr const char* SETTING_POWER_DC_SUSPEND_SOURCES_KEY {"settings.power.dc.suspend_sources"};
#else
    static constexpr const char* SETTING_DISPLAY_OFF_TIME_KEY {"settings.display.screen_off_timeout"};
    static constexpr const char* SETTING_POWER_SUSPEND_SOURCES_KEY {"settings.power.suspend_sources"};
#endif

#ifdef POWER_MANAGER_ENABLE_BLOCK_LONG_PRESS
    static constexpr const char* SETTING_POWER_BLOCK_LONG_PRESS_KEY {"settings.power.block_long_press"};
#endif
    static constexpr const char* SETTING_AUTO_ADJUST_BRIGHTNESS_KEY {"settings.display.auto_screen_brightness"};
    static constexpr const char* SETTING_BRIGHTNESS_KEY {"settings.display.screen_brightness_status"};
    static constexpr const char* SETTING_VIBRATION_KEY {"physic_navi_haptic_feedback_enabled"};
    static constexpr const char* SETTING_WINDOW_ROTATION_KEY {"settings.general.accelerometer_rotation_status"};
    static constexpr const char* SETTING_POWER_WAKEUP_SOURCES_KEY {"settings.power.wakeup_sources"};
    static constexpr const char* SETTING_INTELL_VOICE_KEY {"intell_voice_trigger_enabled"};
    static constexpr const char* SETTING_POWER_WAKEUP_DOUBLE_KEY {"settings.power.wakeup_double_click"};
    static constexpr const char* SETTING_POWER_WAKEUP_PICKUP_KEY {"settings.power.wakeup_pick_up"};
    static constexpr const char* SETTING_POWER_MODE_KEY  {"settings.power.smart_mode_status"};
    static constexpr const char* SETTING_POWER_MODE_BACKUP_KEY  {"settings.power.smart_mode_status.backup"};
    static constexpr const char* SETTING_POWER_WAKEUP_LID_KEY {"settings.power.wakeup_lid"};
    static sptr<SettingObserver> doubleClickObserver_;
    static sptr<SettingObserver> pickUpObserver_;
    static sptr<SettingObserver> powerModeObserver_;
    static sptr<SettingObserver> lidObserver_;
};
} // namespace PowerMgr
} // namespace OHOS

#endif // POWERMGR_POWER_MANAGER_SETTING_HELPER_H
