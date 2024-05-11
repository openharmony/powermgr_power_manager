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
#include <cinttypes>
#include <system_ability_definition.h>

namespace OHOS {
namespace PowerMgr {
bool SettingHelper::IsDisplayOffTimeSettingValid()
{
    return SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID).IsValidKey(SETTING_DISPLAY_OFF_TIME_KEY);
}

int64_t SettingHelper::GetSettingDisplayOffTime(int64_t defaultVal)
{
    SettingProvider& settingProvider = SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID);
    int64_t value = defaultVal;
    ErrCode ret = settingProvider.GetLongValue(SETTING_DISPLAY_OFF_TIME_KEY, value);
    if (ret != ERR_OK) {
        POWER_HILOGW(COMP_UTILS, "get setting display off time failed, ret=%{public}d", ret);
    }
    return value;
}

void SettingHelper::SetSettingDisplayOffTime(int64_t time)
{
    SettingProvider& settingProvider = SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID);
    ErrCode ret = settingProvider.PutLongValue(SETTING_DISPLAY_OFF_TIME_KEY, time);
    if (ret != ERR_OK) {
        POWER_HILOGW(
            COMP_UTILS, "set setting display off time failed, time=%{public}" PRId64 ", ret=%{public}d", time, ret);
    }
}

sptr<SettingObserver> SettingHelper::RegisterSettingDisplayOffTimeObserver(SettingObserver::UpdateFunc& func)
{
    SettingProvider& settingProvider = SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID);
    auto settingObserver = settingProvider.CreateObserver(SETTING_DISPLAY_OFF_TIME_KEY, func);
    ErrCode ret = settingProvider.RegisterObserver(settingObserver);
    if (ret != ERR_OK) {
        POWER_HILOGW(COMP_UTILS, "register setting display off time observer failed, ret=%{public}d", ret);
        return nullptr;
    }
    return settingObserver;
}

bool SettingHelper::IsAutoAdjustBrightnessSettingValid()
{
    return SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID).IsValidKey(SETTING_AUTO_ADJUST_BRIGHTNESS_KEY);
}

int32_t SettingHelper::GetSettingAutoAdjustBrightness(int32_t defaultVal)
{
    SettingProvider& settingProvider = SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID);
    int32_t value = defaultVal;
    ErrCode ret = settingProvider.GetIntValue(SETTING_AUTO_ADJUST_BRIGHTNESS_KEY, value);
    if (ret != ERR_OK) {
        POWER_HILOGW(COMP_UTILS, "get setting auto adjust brightness failed, ret=%{public}d", ret);
    }
    return value;
}

void SettingHelper::SetSettingAutoAdjustBrightness(SwitchStatus status)
{
    SettingProvider& settingProvider = SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID);
    ErrCode ret = settingProvider.PutIntValue(SETTING_AUTO_ADJUST_BRIGHTNESS_KEY, static_cast<int32_t>(status));
    if (ret != ERR_OK) {
        POWER_HILOGW(
            COMP_UTILS, "set setting auto adjust brightness failed, status=%{public}d, ret=%{public}d", status, ret);
    }
}

sptr<SettingObserver> SettingHelper::RegisterSettingAutoAdjustBrightnessObserver(SettingObserver::UpdateFunc& func)
{
    SettingProvider& settingProvider = SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID);
    auto settingObserver = settingProvider.CreateObserver(SETTING_AUTO_ADJUST_BRIGHTNESS_KEY, func);
    ErrCode ret = settingProvider.RegisterObserver(settingObserver);
    if (ret != ERR_OK) {
        POWER_HILOGW(COMP_UTILS, "register setting auto adjust brightness observer failed, ret=%{public}d", ret);
        return nullptr;
    }
    return settingObserver;
}

bool SettingHelper::IsBrightnessSettingValid()
{
    return SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID).IsValidKey(SETTING_BRIGHTNESS_KEY);
}

void SettingHelper::SetSettingBrightness(int32_t brightness)
{
    SettingProvider& settingProvider = SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID);
    ErrCode ret = settingProvider.PutIntValue(SETTING_BRIGHTNESS_KEY, brightness);
    if (ret != ERR_OK) {
        POWER_HILOGW(
            COMP_UTILS, "set setting brightness failed, brightness=%{public}d, ret=%{public}d", brightness, ret);
    }
}

bool SettingHelper::IsVibrationSettingValid()
{
    return SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID).IsValidKey(SETTING_VIBRATION_KEY);
}

int32_t SettingHelper::GetSettingVibration(int32_t defaultVal)
{
    SettingProvider& settingProvider = SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID);
    int32_t value = defaultVal;
    ErrCode ret = settingProvider.GetIntValue(SETTING_VIBRATION_KEY, value);
    if (ret != ERR_OK) {
        POWER_HILOGW(COMP_UTILS, "get setting vibration failed, ret=%{public}d", ret);
    }
    return value;
}

void SettingHelper::SetSettingVibration(SwitchStatus status)
{
    SettingProvider& settingProvider = SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID);
    ErrCode ret = settingProvider.PutIntValue(SETTING_VIBRATION_KEY, static_cast<int32_t>(status));
    if (ret != ERR_OK) {
        POWER_HILOGW(COMP_UTILS, "set setting vibration failed, status=%{public}d, ret=%{public}d", status, ret);
    }
}

sptr<SettingObserver> SettingHelper::RegisterSettingVibrationObserver(SettingObserver::UpdateFunc& func)
{
    SettingProvider& settingProvider = SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID);
    auto settingObserver = settingProvider.CreateObserver(SETTING_VIBRATION_KEY, func);
    ErrCode ret = settingProvider.RegisterObserver(settingObserver);
    if (ret != ERR_OK) {
        POWER_HILOGW(COMP_UTILS, "register setting vibration observer failed, ret=%{public}d", ret);
        return nullptr;
    }
    return settingObserver;
}

bool SettingHelper::IsWindowRotationSettingValid()
{
    return SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID).IsValidKey(SETTING_WINDOW_ROTATION_KEY);
}

int32_t SettingHelper::GetSettingWindowRotation(int32_t defaultVal)
{
    SettingProvider& settingProvider = SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID);
    int32_t value = defaultVal;
    ErrCode ret = settingProvider.GetIntValue(SETTING_WINDOW_ROTATION_KEY, value);
    if (ret != ERR_OK) {
        POWER_HILOGW(COMP_UTILS, "get setting window rotation failed, ret=%{public}d", ret);
    }
    return value;
}

void SettingHelper::SetSettingWindowRotation(SwitchStatus status)
{
    SettingProvider& settingProvider = SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID);
    ErrCode ret = settingProvider.PutIntValue(SETTING_WINDOW_ROTATION_KEY, static_cast<int32_t>(status));
    if (ret != ERR_OK) {
        POWER_HILOGW(COMP_UTILS, "set setting window rotation failed, status=%{public}d, ret=%{public}d", status, ret);
    }
}

sptr<SettingObserver> SettingHelper::RegisterSettingWindowRotationObserver(SettingObserver::UpdateFunc& func)
{
    SettingProvider& settingProvider = SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID);
    auto settingObserver = settingProvider.CreateObserver(SETTING_WINDOW_ROTATION_KEY, func);
    ErrCode ret = settingProvider.RegisterObserver(settingObserver);
    if (ret != ERR_OK) {
        POWER_HILOGW(COMP_UTILS, "register setting window rotation observer failed, ret=%{public}d", ret);
        return nullptr;
    }
    return settingObserver;
}

bool SettingHelper::IsIntellVoiceSettingValid()
{
    return SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID).IsValidKey(SETTING_INTELL_VOICE_KEY);
}

int32_t SettingHelper::GetSettingIntellVoice(int32_t defaultVal)
{
    SettingProvider& settingProvider = SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID);
    int32_t value = defaultVal;
    ErrCode ret = settingProvider.GetIntValue(SETTING_INTELL_VOICE_KEY, value);
    if (ret != ERR_OK) {
        POWER_HILOGW(COMP_UTILS, "get setting intell voice failed, ret=%{public}d", ret);
    }
    return value;
}

void SettingHelper::SetSettingIntellVoice(SwitchStatus status)
{
    SettingProvider& settingProvider = SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID);
    ErrCode ret = settingProvider.PutIntValue(SETTING_INTELL_VOICE_KEY, static_cast<int32_t>(status));
    if (ret != ERR_OK) {
        POWER_HILOGW(COMP_UTILS, "set setting intell voice failed, status=%{public}d, ret=%{public}d", status, ret);
    }
}

sptr<SettingObserver> SettingHelper::RegisterSettingIntellVoiceObserver(SettingObserver::UpdateFunc& func)
{
    SettingProvider& settingProvider = SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID);
    auto settingObserver = settingProvider.CreateObserver(SETTING_INTELL_VOICE_KEY, func);
    ErrCode ret = settingProvider.RegisterObserver(settingObserver);
    if (ret != ERR_OK) {
        POWER_HILOGW(COMP_UTILS, "register setting intell voice observer failed, ret=%{public}d", ret);
        return nullptr;
    }
    return settingObserver;
}

sptr<SettingObserver> SettingHelper::RegisterSettingSuspendSourcesObserver(SettingObserver::UpdateFunc& func)
{
    SettingProvider& settingProvider = SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID);
    auto settingObserver = settingProvider.CreateObserver(SETTING_POWER_SUSPEND_SOURCES_KEY, func);
    ErrCode ret = settingProvider.RegisterObserver(settingObserver);
    if (ret != ERR_OK) {
        POWER_HILOGW(COMP_UTILS, "register setting brightness observer failed, ret=%{public}d", ret);
        return nullptr;
    }
    return settingObserver;
}

bool SettingHelper::IsSuspendSourcesSettingValid()
{
    return SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID).IsValidKey(SETTING_POWER_SUSPEND_SOURCES_KEY);
}

const std::string SettingHelper::GetSettingSuspendSources()
{
    SettingProvider& settingProvider = SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID);
    std::string value;
    ErrCode ret = settingProvider.GetStringValue(SETTING_POWER_SUSPEND_SOURCES_KEY, value);
    if (ret != ERR_OK) {
        POWER_HILOGE(COMP_UTILS, "get setting power suspend sources key failed, ret=%{public}d", ret);
    }
    return value;
}

void SettingHelper::SetSettingSuspendSources(const std::string& jsonConfig)
{
    SettingProvider& settingProvider = SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID);
    ErrCode ret = settingProvider.PutStringValue(SETTING_POWER_SUSPEND_SOURCES_KEY, jsonConfig);
    if (ret != ERR_OK) {
        POWER_HILOGE(COMP_UTILS, "set setting power suspend sources key failed, jsonConfig=%{public}s ret=%{public}d",
            jsonConfig.c_str(), ret);
    }
}

sptr<SettingObserver> SettingHelper::RegisterSettingWakeupSourcesObserver(SettingObserver::UpdateFunc& func)
{
    SettingProvider& settingProvider = SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID);
    auto settingObserver = settingProvider.CreateObserver(SETTING_POWER_WAKEUP_SOURCES_KEY, func);
    ErrCode ret = settingProvider.RegisterObserver(settingObserver);
    if (ret != ERR_OK) {
        POWER_HILOGE(COMP_UTILS, "register setting brightness observer failed, ret=%{public}d", ret);
        return nullptr;
    }
    return settingObserver;
}

bool SettingHelper::IsWakeupSourcesSettingValid()
{
    return SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID).IsValidKey(SETTING_POWER_WAKEUP_SOURCES_KEY);
}

const std::string SettingHelper::GetSettingWakeupSources()
{
    SettingProvider& settingProvider = SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID);
    std::string value;
    ErrCode ret = settingProvider.GetStringValue(SETTING_POWER_WAKEUP_SOURCES_KEY, value);
    if (ret != ERR_OK) {
        POWER_HILOGE(COMP_UTILS, "get setting power Wakeup sources key failed, ret=%{public}d", ret);
    }
    return value;
}

void SettingHelper::SetSettingWakeupSources(const std::string& jsonConfig)
{
    SettingProvider& settingProvider = SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID);
    ErrCode ret = settingProvider.PutStringValue(SETTING_POWER_WAKEUP_SOURCES_KEY, jsonConfig);
    if (ret != ERR_OK) {
        POWER_HILOGE(COMP_UTILS, "set setting power Wakeup sources key failed, jsonConfig=%{public}s ret=%{public}d",
            jsonConfig.c_str(), ret);
    }
}

void SettingHelper::UnregisterSettingObserver(sptr<SettingObserver>& observer)
{
    if (observer == nullptr) {
        return;
    }
    SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID).UnregisterObserver(observer);
}
} // namespace PowerMgr
} // namespace OHOS
