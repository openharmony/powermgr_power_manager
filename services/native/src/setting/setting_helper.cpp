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

#include <cinttypes>
#include <system_ability_definition.h>
#include "power_log.h"

namespace OHOS {
namespace PowerMgr {
bool SettingHelper::IsDisplayOffTimeSettingValid()
{
    return SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID).IsValidKey(SETTING_DISPLAY_OFF_TIME_KEY);
}

int64_t SettingHelper::GetSettingDisplayOffTime()
{
    SettingProvider& provider = SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID);
    int64_t value;
    ErrCode ret = provider.GetLongValue(SETTING_DISPLAY_OFF_TIME_KEY, value);
    if (ret != ERR_OK) {
        POWER_HILOGW(COMP_UTILS, "get setting display off time failed, ret=%{public}d", ret);
    }
    return value;
}

void SettingHelper::SetSettingDisplayOffTime(int64_t time)
{
    SettingProvider& provider = SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID);
    ErrCode ret = provider.PutLongValue(SETTING_DISPLAY_OFF_TIME_KEY, time);
    if (ret != ERR_OK) {
        POWER_HILOGW(
            COMP_UTILS, "set setting display off time failed, time=%{public}" PRId64 ", ret=%{public}d", time, ret);
    }
}

sptr<SettingObserver> SettingHelper::RegisterSettingDisplayOffTimeObserver(SettingObserver::UpdateFunc& func)
{
    SettingProvider& provider = SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID);
    auto observer = provider.CreateObserver(SETTING_DISPLAY_OFF_TIME_KEY, func);
    ErrCode ret = provider.RegisterObserver(observer);
    if (ret != ERR_OK) {
        POWER_HILOGW(COMP_UTILS, "register setting brightness observer failed, ret=%{public}d", ret);
        return nullptr;
    }
    return observer;
}

void SettingHelper::UnregisterSettingDisplayOffTimeObserver(sptr<SettingObserver>& observer)
{
    if (observer == nullptr) {
        return;
    }
    SettingProvider& provider = SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID);
    provider.UnregisterObserver(observer);
}

bool SettingHelper::IsAutoAdjustBrightnessSettingValid()
{
    return SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID).IsValidKey(SETTING_AUTO_ADJUST_BRIGHTNESS_KEY);
}

void SettingHelper::SetSettingAutoAdjustBrightness(SwitchStatus status)
{
    SettingProvider& provider = SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID);
    ErrCode ret = provider.PutIntValue(SETTING_AUTO_ADJUST_BRIGHTNESS_KEY, static_cast<int32_t>(status));
    if (ret != ERR_OK) {
        POWER_HILOGW(
            COMP_UTILS, "set setting auto adjust brightness failed, status=%{public}d, ret=%{public}d", status, ret);
    }
}

bool SettingHelper::IsBrightnessSettingValid()
{
    return SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID).IsValidKey(SETTING_BRIGHTNESS_KEY);
}

void SettingHelper::SetSettingBrightness(int32_t brightness)
{
    SettingProvider& provider = SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID);
    ErrCode ret = provider.PutIntValue(SETTING_BRIGHTNESS_KEY, brightness);
    if (ret != ERR_OK) {
        POWER_HILOGW(
            COMP_UTILS, "set setting brightness failed, brightness=%{public}d, ret=%{public}d", brightness, ret);
    }
}

bool SettingHelper::IsVibrationSettingValid()
{
    return SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID).IsValidKey(SETTING_VIBRATION_KEY);
}

void SettingHelper::SetSettingVibration(SwitchStatus status)
{
    SettingProvider& provider = SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID);
    ErrCode ret = provider.PutIntValue(SETTING_VIBRATION_KEY, static_cast<int32_t>(status));
    if (ret != ERR_OK) {
        POWER_HILOGW(COMP_UTILS, "set setting vibration failed, status=%{public}d, ret=%{public}d", status, ret);
    }
}

bool SettingHelper::IsWindowRotationSettingValid()
{
    return SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID).IsValidKey(SETTING_WINDOW_ROTATION_KEY);
}

void SettingHelper::SetSettingWindowRotation(SwitchStatus status)
{
    SettingProvider& provider = SettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID);
    ErrCode ret = provider.PutIntValue(SETTING_WINDOW_ROTATION_KEY, static_cast<int32_t>(status));
    if (ret != ERR_OK) {
        POWER_HILOGW(COMP_UTILS, "set setting window rotation failed, status=%{public}d, ret=%{public}d", status, ret);
    }
}
} // namespace PowerMgr
} // namespace OHOS
