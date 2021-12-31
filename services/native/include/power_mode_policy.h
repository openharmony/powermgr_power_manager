/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef POWER_MODE_POLICY_H
#define POWER_MODE_POLICY_H

#include <string>
#include <map>
#include <list>
#include "power_save_mode.h"

#define INIT_VALUE_FALSE (-1)
#define LAST_MODE_FLAG 0

namespace OHOS {
namespace PowerMgr {
class PowerModePolicy {
public:
    class ServiceType {
    public:
        static const uint32_t displayOffTime = 101;
        static const uint32_t sleepTime = 102;
        static const uint32_t autoAdjustBrightness = 103;
        static const uint32_t autoWindownRoration = 107;
        static const uint32_t smartBacklight = 115;
        static const uint32_t vibratorsState = 120;
    };

    ~PowerModePolicy() = default;
    int32_t GetPowerModeValuePolicy(uint32_t type);
    int32_t GetPowerModeRecoverPolicy(uint32_t type);
    void SetPowerModePolicy(uint32_t mode, uint32_t lastMode);
    void ListenSetting();
    void AddAction(uint32_t type, std::function<void()> action);
    void TriggerAction(uint32_t type);
    void TriggerAllActions();
    bool IsValidType(uint32_t type);

private:
    std::list<ModePolicy> openPolicy;
    std::list<ModePolicy> closePolicy;
    std::list<ModePolicy> recoverPolicy;

    std::map<uint32_t, std::function<void()>> actionMap;
    std::map<uint32_t, int32_t> valueModePolicy;
    std::map<uint32_t, int32_t> recoverModePolicy;
    std::map<uint32_t, int32_t>::iterator valueiter;
    std::map<uint32_t, int32_t>::iterator recoveriter;

    std::list<ModePolicy>::iterator openlit;
    std::list<ModePolicy>::iterator closelit;
    std::list<ModePolicy>::iterator recoverlit;
    void ReadOpenPolicy(uint32_t mode);
    void ReadRecoverPolicy(uint32_t lastMode);
    void CompareModeItem(uint32_t mode, uint32_t lastMode);
    int32_t GetPolicyFromMap(uint32_t type);
    int32_t GetRecoverPolicyFromMap(uint32_t type);
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWER_MODE_POLICY_H
