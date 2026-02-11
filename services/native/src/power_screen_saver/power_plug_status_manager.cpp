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

#include "power_plug_status_manager.h"

#include "power_log.h"
#ifdef POWER_MANAGER_ENABLE_CHARGING_TYPE_SETTING
#include "battery_srv_client.h"
#endif

namespace OHOS {
namespace PowerMgr {

void PowerPlugStatusManager::Init()
{
#ifdef POWER_MANAGER_ENABLE_CHARGING_TYPE_SETTING
    auto pluggedType = BatterySrvClient::GetInstance().GetPluggedType();
    POWER_HILOGI(COMP_SVC, "Power plug status manager get type is %{public}d.", static_cast<int32_t>(pluggedType));
    switch (pluggedType) {
        case BatteryPluggedType::PLUGGED_TYPE_BUTT:
            POWER_HILOGE(COMP_SVC, "Get plug type error.");
            SetPowerPlugStatus(PowerPlugStatus::POWER_PLUG_IN_INVALID);
            break;
        case BatteryPluggedType::PLUGGED_TYPE_AC:
        case BatteryPluggedType::PLUGGED_TYPE_USB:
        case BatteryPluggedType::PLUGGED_TYPE_WIRELESS:
            SetPowerPlugStatus(PowerPlugStatus::POWER_PLUG_IN_AC);
            break;
        default:
            SetPowerPlugStatus(PowerPlugStatus::POWER_PLUG_IN_DC);
            break;
    }
    return;
#endif
}

void PowerPlugStatusManager::SetPowerPlugStatus(PowerPlugStatus status)
{
    POWER_HILOGI(COMP_SVC, "Set plug status is %{public}d.", static_cast<int32_t>(status));
    powerPlugStatus_ = status;
    return;
}

bool PowerPlugStatusManager::IsPowerPluged()
{
    return powerPlugStatus_ == PowerPlugStatus::POWER_PLUG_IN_AC;
}

} //namespace PowerMgr
} //namespace OHOS