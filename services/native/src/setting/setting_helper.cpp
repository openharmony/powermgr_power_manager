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

#include "setting_helper.h"

#include "power_log.h"
#include "power_utils.h"
#include <cinttypes>
#include <system_ability_definition.h>

namespace OHOS {
namespace PowerMgr {
namespace {
constexpr int32_t WAKEUP_SOURCE_OPEN = 1;
constexpr int32_t WAKEUP_SOURCE_CLOSE = 0;
}
sptr<SettingObserver> SettingHelper::doubleClickObserver_ = nullptr;
sptr<SettingObserver> SettingHelper::pickUpObserver_ = nullptr;
sptr<SettingObserver> SettingHelper::powerModeObserver_ = nullptr;
sptr<SettingObserver> SettingHelper::lidObserver_ = nullptr;

void SettingHelper::UpdateCurrentUserId()
{
    SettingProvider& settingProvider = SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID);
    settingProvider.UpdateCurrentUserId();
}

bool SettingHelper::IsSettingKeyValid(const std::string& key)
{
    return SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID).IsValidKey(key);
}

int32_t SettingHelper::GetSettingIntValue(const std::string& key, int32_t defaultVal)
{
    SettingProvider& settingProvider = SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID);
    int32_t value = defaultVal;
    ErrCode ret = settingProvider.GetIntValue(key, value);
    if (ret != ERR_OK) {
        POWER_HILOGW(COMP_UTILS, "get setting key=%{public}s failed, ret=%{public}d", key.c_str(), ret);
    }
    return value;
}

void SettingHelper::SetSettingIntValue(const std::string& key, int32_t value)
{
    SettingProvider& settingProvider = SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID);
    ErrCode ret = settingProvider.PutIntValue(key, value);
    if (ret != ERR_OK) {
        POWER_HILOGW(COMP_UTILS,
            "set setting key=%{public}s value=%{public}d failed, ret=%{public}d",
            key.c_str(), value, ret);
    }
}

int64_t SettingHelper::GetSettingLongValue(const std::string& key, int64_t defaultVal)
{
    SettingProvider& settingProvider = SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID);
    int64_t value = defaultVal;
    ErrCode ret = settingProvider.GetLongValue(key, value);
    if (ret != ERR_OK) {
        POWER_HILOGW(COMP_UTILS, "get setting key=%{public}s failed, ret=%{public}d", key.c_str(), ret);
    }
    return value;
}

void SettingHelper::SetSettingLongValue(const std::string& key, int64_t value)
{
    SettingProvider& settingProvider = SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID);
    ErrCode ret = settingProvider.PutLongValue(key, value);
    if (ret != ERR_OK) {
        POWER_HILOGW(COMP_UTILS,
            "set setting key=%{public}s value=%{public}" PRId64 " failed, ret=%{public}d",
            key.c_str(), value, ret);
    }
}

const std::string SettingHelper::GetSettingStringValue(const std::string& key)
{
    SettingProvider& settingProvider = SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID);
    std::string value;
    ErrCode ret = settingProvider.GetStringValue(key, value);
    if (ret != ERR_OK) {
        POWER_HILOGW(COMP_UTILS, "get setting key=%{public}s failed, ret=%{public}d", key.c_str(), ret);
    }
    return value;
}

void SettingHelper::SetSettingJsonStringValue(const std::string& key, const std::string& jsonConfig)
{
    SettingProvider& settingProvider = SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID);
    ErrCode ret = settingProvider.PutStringValue(key, jsonConfig);
    if (ret != ERR_OK) {
        POWER_HILOGW(COMP_UTILS,
            "set setting key=%{public}s failed, jsonConfig=%{public}s, ret=%{public}d",
            key.c_str(), PowerUtils::JsonToSimpleStr(jsonConfig).c_str(), ret);
    }
}

sptr<SettingObserver> SettingHelper::RegisterSettingKeyObserver(
    const std::string& key, SettingObserver::UpdateFunc& func)
{
    SettingProvider& settingProvider = SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID);
    auto settingObserver = settingProvider.CreateObserver(key, func);
    ErrCode ret = settingProvider.RegisterObserver(settingObserver);
    if (ret != ERR_OK) {
        POWER_HILOGW(COMP_UTILS, "register setting key=%{public}s observer failed, ret=%{public}d",
            key.c_str(), ret);
        return nullptr;
    }
    return settingObserver;
}

void SettingHelper::UnregisterSettingObserver(sptr<SettingObserver>& observer)
{
    if (observer == nullptr) {
        return;
    }
    SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID).UnregisterObserver(observer);
}

bool SettingHelper::IsWakeupPickupSettingValid()
{
    return IsSettingKeyValid(SETTING_POWER_WAKEUP_PICKUP_KEY);
}

bool SettingHelper::GetSettingWakeupPickup(const std::string& key)
{
    SettingProvider& settingProvider = SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID);
    int32_t value = 0;
    ErrCode ret = settingProvider.GetIntValue(key, value);
    if (ret != ERR_OK) {
        POWER_HILOGE(COMP_UTILS, "get setting power wakeup pickup key failed, ret=%{public}d", ret);
    }
    return (value == WAKEUP_SOURCE_OPEN);
}

void SettingHelper::SetSettingWakeupPickup(bool enable)
{
    POWER_HILOGI(COMP_UTILS, "SetSettingWakeupPickup switch, enable=%{public}d", enable);
    int32_t value = enable ? WAKEUP_SOURCE_OPEN : WAKEUP_SOURCE_CLOSE;
    SetSettingIntValue(SETTING_POWER_WAKEUP_PICKUP_KEY, value);
}

void SettingHelper::RegisterSettingWakeupPickupObserver(SettingObserver::UpdateFunc& func)
{
    if (pickUpObserver_) {
        POWER_HILOGI(COMP_UTILS, "setting wakeup pickup observer is already registered");
        return;
    }
    pickUpObserver_ = RegisterSettingKeyObserver(SETTING_POWER_WAKEUP_PICKUP_KEY, func);
}

void SettingHelper::UnregisterSettingWakeupPickupObserver()
{
    if (!pickUpObserver_) {
        POWER_HILOGI(COMP_UTILS, "pickUpObserver_ is nullptr, no need to unregister");
        return;
    }
    UnregisterSettingObserver(pickUpObserver_);
    pickUpObserver_ = nullptr;
}

#ifdef POWER_MANAGER_ENABLE_CHARGING_TYPE_SETTING
bool SettingHelper::IsDisplayOffTimeSettingValid()
{
    return IsSettingKeyValid(SETTING_DISPLAY_AC_OFF_TIME_KEY) &&
        IsSettingKeyValid(SETTING_DISPLAY_DC_OFF_TIME_KEY);
}

bool SettingHelper::IsSettingDisplayAcScreenOffTimeValid()
{
    return IsSettingKeyValid(SETTING_DISPLAY_AC_OFF_TIME_KEY);
}

int64_t SettingHelper::GetSettingDisplayAcScreenOffTime(int64_t defaultVal)
{
    return GetSettingLongValue(SETTING_DISPLAY_AC_OFF_TIME_KEY, defaultVal);
}

void SettingHelper::SetSettingDisplayAcScreenOffTime(int64_t time)
{
    SetSettingLongValue(SETTING_DISPLAY_AC_OFF_TIME_KEY, time);
}

sptr<SettingObserver> SettingHelper::RegisterSettingDisplayAcScreenOffTimeObserver(SettingObserver::UpdateFunc& func)
{
    return RegisterSettingKeyObserver(SETTING_DISPLAY_AC_OFF_TIME_KEY, func);
}

bool SettingHelper::IsSettingDisplayDcScreenOffTimeValid()
{
    return IsSettingKeyValid(SETTING_DISPLAY_DC_OFF_TIME_KEY);
}

int64_t SettingHelper::GetSettingDisplayDcScreenOffTime(int64_t defaultVal)
{
    return GetSettingLongValue(SETTING_DISPLAY_DC_OFF_TIME_KEY, defaultVal);
}

void SettingHelper::SetSettingDisplayDcScreenOffTime(int64_t time)
{
    SetSettingLongValue(SETTING_DISPLAY_DC_OFF_TIME_KEY, time);
}

sptr<SettingObserver> SettingHelper::RegisterSettingDisplayDcScreenOffTimeObserver(SettingObserver::UpdateFunc& func)
{
    return RegisterSettingKeyObserver(SETTING_DISPLAY_DC_OFF_TIME_KEY, func);
}
#else
bool SettingHelper::IsDisplayOffTimeSettingValid()
{
    return IsSettingKeyValid(SETTING_DISPLAY_OFF_TIME_KEY);
}

int64_t SettingHelper::GetSettingDisplayOffTime(int64_t defaultVal)
{
    return GetSettingLongValue(SETTING_DISPLAY_OFF_TIME_KEY, defaultVal);
}

void SettingHelper::SetSettingDisplayOffTime(int64_t time)
{
    SetSettingLongValue(SETTING_DISPLAY_OFF_TIME_KEY, time);
}

sptr<SettingObserver> SettingHelper::RegisterSettingDisplayOffTimeObserver(SettingObserver::UpdateFunc& func)
{
    return RegisterSettingKeyObserver(SETTING_DISPLAY_OFF_TIME_KEY, func);
}
#endif

bool SettingHelper::IsAutoAdjustBrightnessSettingValid()
{
    return IsSettingKeyValid(SETTING_AUTO_ADJUST_BRIGHTNESS_KEY);
}

int32_t SettingHelper::GetSettingAutoAdjustBrightness(int32_t defaultVal)
{
    return GetSettingIntValue(SETTING_AUTO_ADJUST_BRIGHTNESS_KEY, defaultVal);
}

void SettingHelper::SetSettingAutoAdjustBrightness(SwitchStatus status)
{
    SetSettingIntValue(SETTING_AUTO_ADJUST_BRIGHTNESS_KEY, static_cast<int32_t>(status));
}

sptr<SettingObserver> SettingHelper::RegisterSettingAutoAdjustBrightnessObserver(SettingObserver::UpdateFunc& func)
{
    return RegisterSettingKeyObserver(SETTING_AUTO_ADJUST_BRIGHTNESS_KEY, func);
}

bool SettingHelper::IsBrightnessSettingValid()
{
    return IsSettingKeyValid(SETTING_BRIGHTNESS_KEY);
}

void SettingHelper::SetSettingBrightness(int32_t brightness)
{
    SetSettingIntValue(SETTING_BRIGHTNESS_KEY, brightness);
}

bool SettingHelper::IsVibrationSettingValid()
{
    return IsSettingKeyValid(SETTING_VIBRATION_KEY);
}

int32_t SettingHelper::GetSettingVibration(int32_t defaultVal)
{
    return GetSettingIntValue(SETTING_VIBRATION_KEY, defaultVal);
}

void SettingHelper::SetSettingVibration(SwitchStatus status)
{
    SetSettingIntValue(SETTING_VIBRATION_KEY, static_cast<int32_t>(status));
}

sptr<SettingObserver> SettingHelper::RegisterSettingVibrationObserver(SettingObserver::UpdateFunc& func)
{
    return RegisterSettingKeyObserver(SETTING_VIBRATION_KEY, func);
}

bool SettingHelper::IsWindowRotationSettingValid()
{
    return IsSettingKeyValid(SETTING_WINDOW_ROTATION_KEY);
}

int32_t SettingHelper::GetSettingWindowRotation(int32_t defaultVal)
{
    return GetSettingIntValue(SETTING_WINDOW_ROTATION_KEY, defaultVal);
}

void SettingHelper::SetSettingWindowRotation(SwitchStatus status)
{
    SetSettingIntValue(SETTING_WINDOW_ROTATION_KEY, static_cast<int32_t>(status));
}

sptr<SettingObserver> SettingHelper::RegisterSettingWindowRotationObserver(SettingObserver::UpdateFunc& func)
{
    return RegisterSettingKeyObserver(SETTING_WINDOW_ROTATION_KEY, func);
}

bool SettingHelper::IsIntellVoiceSettingValid()
{
    return IsSettingKeyValid(SETTING_INTELL_VOICE_KEY);
}

int32_t SettingHelper::GetSettingIntellVoice(int32_t defaultVal)
{
    return GetSettingIntValue(SETTING_INTELL_VOICE_KEY, defaultVal);
}

void SettingHelper::SetSettingIntellVoice(SwitchStatus status)
{
    SetSettingIntValue(SETTING_INTELL_VOICE_KEY, static_cast<int32_t>(status));
}

sptr<SettingObserver> SettingHelper::RegisterSettingIntellVoiceObserver(SettingObserver::UpdateFunc& func)
{
    return RegisterSettingKeyObserver(SETTING_INTELL_VOICE_KEY, func);
}

#ifdef POWER_MANAGER_ENABLE_CHARGING_TYPE_SETTING
sptr<SettingObserver> SettingHelper::RegisterSettingAcSuspendSourcesObserver(SettingObserver::UpdateFunc& func)
{
    return RegisterSettingKeyObserver(SETTING_POWER_AC_SUSPEND_SOURCES_KEY, func);
}

bool SettingHelper::IsSuspendSourcesSettingValid()
{
    return IsSettingKeyValid(SETTING_POWER_AC_SUSPEND_SOURCES_KEY) &&
        IsSettingKeyValid(SETTING_POWER_DC_SUSPEND_SOURCES_KEY);
}

bool SettingHelper::IsSettingAcSuspendSourcesValid()
{
    return IsSettingKeyValid(SETTING_POWER_AC_SUSPEND_SOURCES_KEY);
}

const std::string SettingHelper::GetSettingAcSuspendSources()
{
    return GetSettingStringValue(SETTING_POWER_AC_SUSPEND_SOURCES_KEY);
}

void SettingHelper::SetSettingAcSuspendSources(const std::string& jsonConfig)
{
    SetSettingJsonStringValue(SETTING_POWER_AC_SUSPEND_SOURCES_KEY, jsonConfig);
}

sptr<SettingObserver> SettingHelper::RegisterSettingDcSuspendSourcesObserver(SettingObserver::UpdateFunc& func)
{
    return RegisterSettingKeyObserver(SETTING_POWER_DC_SUSPEND_SOURCES_KEY, func);
}

bool SettingHelper::IsSettingDcSuspendSourcesValid()
{
    return IsSettingKeyValid(SETTING_POWER_DC_SUSPEND_SOURCES_KEY);
}

const std::string SettingHelper::GetSettingDcSuspendSources()
{
    return GetSettingStringValue(SETTING_POWER_DC_SUSPEND_SOURCES_KEY);
}

void SettingHelper::SetSettingDcSuspendSources(const std::string& jsonConfig)
{
    SetSettingJsonStringValue(SETTING_POWER_DC_SUSPEND_SOURCES_KEY, jsonConfig);
}
#else
sptr<SettingObserver> SettingHelper::RegisterSettingSuspendSourcesObserver(SettingObserver::UpdateFunc& func)
{
    return RegisterSettingKeyObserver(SETTING_POWER_SUSPEND_SOURCES_KEY, func);
}

bool SettingHelper::IsSuspendSourcesSettingValid()
{
    return IsSettingKeyValid(SETTING_POWER_SUSPEND_SOURCES_KEY);
}

const std::string SettingHelper::GetSettingSuspendSources()
{
    return GetSettingStringValue(SETTING_POWER_SUSPEND_SOURCES_KEY);
}

void SettingHelper::SetSettingSuspendSources(const std::string& jsonConfig)
{
    SetSettingJsonStringValue(SETTING_POWER_SUSPEND_SOURCES_KEY, jsonConfig);
}
#endif

sptr<SettingObserver> SettingHelper::RegisterSettingWakeupSourcesObserver(SettingObserver::UpdateFunc& func)
{
    return RegisterSettingKeyObserver(SETTING_POWER_WAKEUP_SOURCES_KEY, func);
}

bool SettingHelper::IsWakeupSourcesSettingValid()
{
    return IsSettingKeyValid(SETTING_POWER_WAKEUP_SOURCES_KEY);
}

const std::string SettingHelper::GetSettingWakeupSources()
{
    return GetSettingStringValue(SETTING_POWER_WAKEUP_SOURCES_KEY);
}

void SettingHelper::SetSettingWakeupSources(const std::string& jsonConfig)
{
    SetSettingJsonStringValue(SETTING_POWER_WAKEUP_SOURCES_KEY, jsonConfig);
}

bool SettingHelper::IsWakeupDoubleSettingValid()
{
    return IsSettingKeyValid(SETTING_POWER_WAKEUP_DOUBLE_KEY);
}

bool SettingHelper::GetSettingWakeupDouble(const std::string& key)
{
    SettingProvider& settingProvider = SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID);
    int32_t value;
    ErrCode ret = settingProvider.GetIntValue(key, value);
    if (ret != ERR_OK) {
        POWER_HILOGE(COMP_UTILS, "get setting power Wakeup double key failed, ret=%{public}d", ret);
    }
    return (value == WAKEUP_SOURCE_OPEN);
}

void SettingHelper::SetSettingWakeupDouble(bool enable)
{
    POWER_HILOGI(COMP_UTILS, "SetSettingWakeupDouble switch, enable=%{public}d", enable);
    int32_t value = enable ? WAKEUP_SOURCE_OPEN : WAKEUP_SOURCE_CLOSE;
    SetSettingIntValue(SETTING_POWER_WAKEUP_DOUBLE_KEY, value);
}

void SettingHelper::RegisterSettingWakeupDoubleObserver(SettingObserver::UpdateFunc& func)
{
    if (doubleClickObserver_) {
        POWER_HILOGI(COMP_UTILS, "setting wakeup double click observer is already registered");
        return;
    }
    doubleClickObserver_ = RegisterSettingKeyObserver(SETTING_POWER_WAKEUP_DOUBLE_KEY, func);
}

void SettingHelper::UnregisterSettingWakeupDoubleObserver()
{
    if (!doubleClickObserver_) {
        POWER_HILOGI(COMP_UTILS, "doubleClickObserver_ is nullptr, no need to unregister");
        return;
    }
    UnregisterSettingObserver(doubleClickObserver_);
    doubleClickObserver_ = nullptr;
}

void SettingHelper::SaveCurrentMode(int32_t mode)
{
    SetSettingIntValue(SETTING_POWER_MODE_KEY, mode);
}

int32_t SettingHelper::ReadCurrentMode(int32_t defaultMode)
{
    return GetSettingIntValue(SETTING_POWER_MODE_KEY, defaultMode);
}

void SettingHelper::RegisterSettingPowerModeObserver(SettingObserver::UpdateFunc& func)
{
    if (powerModeObserver_) {
        POWER_HILOGI(COMP_UTILS, "setting power mode observer is already registered");
        return;
    }
    powerModeObserver_ = RegisterSettingKeyObserver(SETTING_POWER_MODE_KEY, func);
}

void SettingHelper::UnRegisterSettingPowerModeObserver()
{
    if (!powerModeObserver_) {
        POWER_HILOGI(COMP_UTILS, "powerModeObserver_ is nullptr, no need to unregister");
        return;
    }
    auto ret = SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID).UnregisterObserver(powerModeObserver_);
    if (ret != ERR_OK) {
        POWER_HILOGE(COMP_UTILS, "unregister setting power mode observer failed, ret=%{public}d", ret);
    }
    powerModeObserver_ = nullptr;
}

const std::string SettingHelper::ReadPowerModeRecoverMap()
{
    return GetSettingStringValue(SETTING_POWER_MODE_BACKUP_KEY);
}

void SettingHelper::SavePowerModeRecoverMap(const std::string& jsonConfig)
{
    SetSettingJsonStringValue(SETTING_POWER_MODE_BACKUP_KEY, jsonConfig);
}

bool SettingHelper::IsWakeupLidSettingValid()
{
    return IsSettingKeyValid(SETTING_POWER_WAKEUP_LID_KEY);
}

void SettingHelper::RegisterSettingWakeupLidObserver(SettingObserver::UpdateFunc& func)
{
    if (!IsWakeupLidSettingValid()) {
        POWER_HILOGE(COMP_UTILS, "settings.power.wakeup_lid is valid.");
        return;
    }
    lidObserver_ = RegisterSettingKeyObserver(SETTING_POWER_WAKEUP_LID_KEY, func);
}

void SettingHelper::UnRegisterSettingWakeupLidObserver()
{
    if (!lidObserver_) {
        POWER_HILOGI(COMP_UTILS, "lidObserver_ is nullptr, no need to unregister");
        return;
    }
    if (!IsWakeupLidSettingValid()) {
        POWER_HILOGE(COMP_UTILS, "settings.power.wakeup_lid is valid.");
        return;
    }
    auto ret = SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID).UnregisterObserver(lidObserver_);
    if (ret != ERR_OK) {
        POWER_HILOGE(COMP_UTILS, "unregister setting wakeup lid observer failed, ret=%{public}d", ret);
    }
    lidObserver_ = nullptr;
}

bool SettingHelper::GetSettingWakeupLid(const std::string& key)
{
    if (!IsWakeupLidSettingValid()) {
        POWER_HILOGE(COMP_UTILS, "settings.power.wakeup_lid is valid.");
        return false;
    }
    SettingProvider& settingProvider = SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID);
    int32_t value = 0;
    ErrCode ret = settingProvider.GetIntValue(SETTING_POWER_WAKEUP_LID_KEY, value);
    if (ret != ERR_OK) {
        POWER_HILOGE(COMP_UTILS, "get setting power wakeup lid key failed, ret=%{public}d", ret);
    }
    return (value == WAKEUP_SOURCE_OPEN);
}

void SettingHelper::SetSettingWakeupLid(bool enable)
{
    POWER_HILOGI(COMP_UTILS, "SetSettingWakeupLid, enable=%{public}d", enable);
    if (!IsWakeupLidSettingValid()) {
        POWER_HILOGE(COMP_UTILS, "settings.power.wakeup_lid is valid.");
        return;
    }
    int32_t value = enable ? WAKEUP_SOURCE_OPEN : WAKEUP_SOURCE_CLOSE;
    SetSettingIntValue(SETTING_POWER_WAKEUP_LID_KEY, value);
}
} // namespace PowerMgr
} // namespace OHOS
