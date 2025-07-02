/*
 * Copyright (c) 2024-2024 Huawei Device Co., Ltd.
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

#include <securec.h>
#include "proximity_controller_base.h"
#include "ffrt_utils.h"
#include "power_log.h"
#include "errors.h"
#include "setting_helper.h"

namespace OHOS {
namespace PowerMgr {
ProximityControllerBase::ProximityControllerBase(const std::string& name, SensorCallbackFunc callback)
{
#ifdef HAS_SENSORS_SENSOR_PART
    POWER_HILOGD(FEATURE_INPUT, "Instance enter");
    SensorInfo* sensorInfo = nullptr;
    int32_t count = 0;
    int ret = GetAllSensors(&sensorInfo, &count);
    if (ret != 0 || sensorInfo == nullptr) {
        POWER_HILOGE(FEATURE_INPUT, "Get sensors fail, ret=%{public}d", ret);
        return;
    }
    for (int32_t i = 0; i < count; i++) {
        if (sensorInfo[i].sensorTypeId == SENSOR_TYPE_ID_PROXIMITY) {
            POWER_HILOGI(FEATURE_INPUT, "Support PROXIMITY sensor");
            SetSupported(true);
            break;
        }
    }
    if (!IsSupported()) {
        POWER_HILOGE(FEATURE_INPUT, "PROXIMITY sensor not support");
        return;
    }
    if (strcpy_s(user_.name, sizeof(user_.name), name.c_str()) != EOK) {
        POWER_HILOGE(FEATURE_INPUT, "strcpy_s user_.name=%{public}s error", name.c_str());
        return;
    }
    user_.userData = nullptr;
    user_.callback = callback;
#endif
}

ProximityControllerBase::~ProximityControllerBase()
{
#ifdef HAS_SENSORS_SENSOR_PART
    if (IsSupported()) {
        UnsubscribeSensor(SENSOR_TYPE_ID_PROXIMITY, &user_);
    }
#endif
}

#ifdef HAS_SENSORS_SENSOR_PART
void ProximityControllerBase::Enable()
{
    POWER_HILOGD(FEATURE_INPUT, "Enter");
    SetEnabled(true);
    if (!IsSupported()) {
        POWER_HILOGE(FEATURE_INPUT, "PROXIMITY sensor not support");
        return;
    }

    int32_t errorCode = SubscribeSensor(SENSOR_TYPE_ID_PROXIMITY, &user_);
    if (errorCode != ERR_OK) {
        POWER_HILOGW(FEATURE_INPUT, "SubscribeSensor PROXIMITY failed, errorCode=%{public}d", errorCode);
        return;
    }
    SetBatch(SENSOR_TYPE_ID_PROXIMITY, &user_, SAMPLING_RATE, SAMPLING_RATE);
    errorCode = ActivateSensor(SENSOR_TYPE_ID_PROXIMITY, &user_);
    if (errorCode != ERR_OK) {
        POWER_HILOGW(FEATURE_INPUT, "ActivateSensor PROXIMITY failed, errorCode=%{public}d", errorCode);
        return;
    }
    SetMode(SENSOR_TYPE_ID_PROXIMITY, &user_, SENSOR_ON_CHANGE);
    POWER_HILOGI(FEATURE_INPUT, "ActivateSensor PROXIMITY success");
}

void ProximityControllerBase::Disable()
{
    POWER_HILOGD(FEATURE_INPUT, "Enter");
    SetEnabled(false);
    if (!IsSupported()) {
        POWER_HILOGE(FEATURE_INPUT, "PROXIMITY sensor not support");
        return;
    }

    DeactivateSensor(SENSOR_TYPE_ID_PROXIMITY, &user_);
    int32_t errorCode = UnsubscribeSensor(SENSOR_TYPE_ID_PROXIMITY, &user_);
    if (errorCode != ERR_OK) {
        POWER_HILOGW(FEATURE_INPUT, "UnsubscribeSensor PROXIMITY failed, errorCode=%{public}d", errorCode);
        return;
    }
    POWER_HILOGI(FEATURE_INPUT, "UnsubscribeSensor PROXIMITY success");
}
#endif
} // namespace PowerMgr
} // namespace OHOS