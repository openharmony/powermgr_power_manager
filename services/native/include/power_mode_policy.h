/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include <mutex>
#include <list>
#include "power_save_mode.h"

#define INIT_VALUE_FALSE (-2)
#define LAST_MODE_FLAG 0

namespace OHOS {
namespace PowerMgr {
class PowerModePolicy {
public:
    class ServiceType {
    public:
        static constexpr uint32_t DISPLAY_OFFTIME = 101;
        static constexpr uint32_t SLEEPTIME = 102;
        static constexpr uint32_t AUTO_ADJUST_BRIGHTNESS = 103;
        static constexpr uint32_t INTELL_VOICE = 105;
        static constexpr uint32_t AUTO_WINDOWN_RORATION = 107;
        static constexpr uint32_t LCD_BRIGHTNESS = 115;
        static constexpr uint32_t VIBRATORS_STATE = 120;
    };

    ~PowerModePolicy() = default;
    int32_t GetPowerModeValuePolicy(uint32_t type); // from switchMap_
    void UpdatePowerModePolicy(uint32_t mode);
    void RemoveBackupMapSettingSwitch(uint32_t switchId);
    typedef std::function<void(bool)> ModeAction;
    void AddAction(uint32_t type, ModeAction& action);
    void TriggerAllActions(bool isBoot);
    bool IsValidType(uint32_t type);

private:
    std::map<uint32_t, ModeAction> actionMap_;
    std::map<uint32_t, int32_t> switchMap_;
    std::map<uint32_t, int32_t> recoverMap_;
    std::map<uint32_t, int32_t> backupMap_;

    void ReadPowerModePolicy(uint32_t mode);
    void ComparePowerModePolicy();
    void GetSettingSwitchState(uint32_t& switchId, int32_t& value); // from setting
    int32_t GetPolicyFromMap(uint32_t type);
    std::mutex policyMutex_;
    std::mutex actionMapMutex_;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWER_MODE_POLICY_H
