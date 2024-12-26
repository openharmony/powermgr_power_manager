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

#ifndef POWERMGR_PROXIMITY_CONTROLLER_BASE_H
#define POWERMGR_PROXIMITY_CONTROLLER_BASE_H

#ifdef HAS_SENSORS_SENSOR_PART
#include <mutex>
#endif
#include "sensor_agent.h"
#include "power_state_machine_info.h"

namespace OHOS {
namespace PowerMgr {
typedef void (*SensorCallbackFunc)(SensorEvent *event);

class ProximityControllerBase {
public:
    ProximityControllerBase(const std::string& name, SensorCallbackFunc callback);
    virtual ~ProximityControllerBase();
#ifdef HAS_SENSORS_SENSOR_PART
    void Enable();
    void Disable();
    bool IsEnabled()
    {
        return enabled_;
    }
    bool IsSupported()
    {
        return support_;
    }
    bool IsClose();
    uint32_t GetStatus()
    {
        return status_;
    }
    void Clear();
#endif

protected:
#ifdef HAS_SENSORS_SENSOR_PART
    static const int32_t PROXIMITY_CLOSE_SCALAR = 0;
    static const int32_t PROXIMITY_AWAY_SCALAR = 5;
    static const uint32_t SAMPLING_RATE =  100000000;
    bool support_ {false};
    bool enabled_ {false};
    bool isClose_ {false};
    uint32_t status_ {0};
    SensorUser user_ {};
#endif
};

class ProximityNormalController : public ProximityControllerBase {
public:
    ProximityNormalController(const std::string& name = "ProximityNormalController", SensorCallbackFunc
        callback = &ProximityNormalController::RecordSensorCallback) : ProximityControllerBase(name, callback) {}
    ~ProximityNormalController() {}
    void ActivateValidProximitySensor(PowerState state);
#ifdef HAS_SENSORS_SENSOR_PART
    static void RecordSensorCallback(SensorEvent *event);
    static bool isInactiveClose_;
#endif
    
private:
#ifdef HAS_SENSORS_SENSOR_PART
    bool proximitySensorEnabled_ {false};
    static std::mutex userMutex_;
#endif
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_PROXIMITY_CONTROLLER_BASE_H