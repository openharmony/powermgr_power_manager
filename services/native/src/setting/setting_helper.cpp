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
    return PowerSettingHelper::GetInstance(POWER_MANAGER_SERVICE_ID).IsValidKey(SETTING_DISPLAY_OFF_TIME_KEY);
}

int64_t SettingHelper::GetSettingDisplayOffTime()
{
    PowerSettingHelper& helper = PowerSettingHelper::GetInstance(POWER_MANAGER_SERVICE_ID);
    int64_t value;
    ErrCode ret = helper.GetLongValue(SETTING_DISPLAY_OFF_TIME_KEY, value);
    if (ret != ERR_OK) {
        POWER_HILOGW(COMP_UTILS, "get setting display off time failed, ret=%{public}d", ret);
    }
    return value;
}

void SettingHelper::SetSettingDisplayOffTime(int64_t time)
{
    PowerSettingHelper& helper = PowerSettingHelper::GetInstance(POWER_MANAGER_SERVICE_ID);
    ErrCode ret = helper.PutLongValue(SETTING_DISPLAY_OFF_TIME_KEY, time);
    if (ret != ERR_OK) {
        POWER_HILOGW(COMP_UTILS, "set setting display off time failed, time=%{public}" PRId64 ", ret=%{public}d",
                     time, ret);
    }
}

sptr<PowerSettingObserver> SettingHelper::RegisterSettingDisplayOffTimeObserver(PowerSettingObserver::UpdateFunc func)
{
    PowerSettingHelper& helper = PowerSettingHelper::GetInstance(POWER_MANAGER_SERVICE_ID);
    auto observer = helper.CreateObserver(SETTING_AUTO_ADJUST_BRIGHTNESS_KEY, func);
    ErrCode ret = helper.RegisterObserver(observer);
    if (ret != ERR_OK) {
        POWER_HILOGW(COMP_UTILS, "register setting brightness observer failed, ret=%{public}d", ret);
        return nullptr;
    }
    return observer;
}

void SettingHelper::UnregisterSettingDisplayOffTimeObserver(sptr<PowerSettingObserver>& observer)
{
    if (observer == nullptr) {
        return;
    }
    PowerSettingHelper& helper = PowerSettingHelper::GetInstance(POWER_MANAGER_SERVICE_ID);
    helper.UnregisterObserver(observer);
}

bool SettingHelper::IsAutoAdjustBrightnessSettingValid()
{
    return PowerSettingHelper::GetInstance(POWER_MANAGER_SERVICE_ID).IsValidKey(SETTING_AUTO_ADJUST_BRIGHTNESS_KEY);
}

void SettingHelper::SetSettingAutoAdjustBrightness(SwitchStatus status)
{
    PowerSettingHelper& helper = PowerSettingHelper::GetInstance(POWER_MANAGER_SERVICE_ID);
    ErrCode ret = helper.PutIntValue(SETTING_AUTO_ADJUST_BRIGHTNESS_KEY, static_cast<int32_t>(status));
    if (ret != ERR_OK) {
        POWER_HILOGW(COMP_UTILS, "set setting auto adjust brightness failed, " \
                    "status=%{public}d, ret=%{public}d", status, ret);
    }
}

bool SettingHelper::IsBrightnessSettingValid()
{
    return PowerSettingHelper::GetInstance(POWER_MANAGER_SERVICE_ID).IsValidKey(SETTING_VIBRATION_KEY);
}

void SettingHelper::SetSettingBrightness(int32_t brightness)
{
    PowerSettingHelper& helper = PowerSettingHelper::GetInstance(POWER_MANAGER_SERVICE_ID);
    ErrCode ret = helper.PutIntValue(SETTING_BRIGHTNESS_KEY, brightness);
    if (ret != ERR_OK) {
        POWER_HILOGW(COMP_UTILS, "set setting brightness failed, brightness=%{public}d, ret=%{public}d",
                     brightness, ret);
    }
}

bool SettingHelper::IsVibrationSettingValid()
{
    return PowerSettingHelper::GetInstance(POWER_MANAGER_SERVICE_ID).IsValidKey(SETTING_VIBRATION_KEY);
}

void SettingHelper::SetSettingVibration(SwitchStatus status)
{
    PowerSettingHelper& helper = PowerSettingHelper::GetInstance(POWER_MANAGER_SERVICE_ID);
    ErrCode ret = helper.PutIntValue(SETTING_VIBRATION_KEY, static_cast<int32_t>(status));
    if (ret != ERR_OK) {
        POWER_HILOGW(COMP_UTILS, "set setting vibration failed, status=%{public}d, ret=%{public}d",
                     status, ret);
    }
}

bool SettingHelper::IsWindowRotationSettingValid()
{
    return PowerSettingHelper::GetInstance(POWER_MANAGER_SERVICE_ID).IsValidKey(SETTING_WINDOW_ROTATION_KEY);
}

void SettingHelper::SetSettingWindowRotation(SwitchStatus status)
{
    PowerSettingHelper& helper = PowerSettingHelper::GetInstance(POWER_MANAGER_SERVICE_ID);
    ErrCode ret = helper.PutIntValue(SETTING_WINDOW_ROTATION_KEY, static_cast<int32_t>(status));
    if (ret != ERR_OK) {
        POWER_HILOGW(COMP_UTILS, "set setting window rotation failed, status=%{public}d, ret=%{public}d",
                     status, ret);
    }
}
} // OHOS
} // PowerMgr
