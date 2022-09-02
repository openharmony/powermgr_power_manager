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

#include "power_mode_policy.h"

#include "power_log.h"
#include "power_save_mode.h"
#include "singleton.h"

using namespace std;

namespace OHOS {
namespace PowerMgr {
int32_t PowerModePolicy::GetPowerModeValuePolicy(uint32_t type)
{
    int32_t ret = INIT_VALUE_FALSE;
    if (IsValidType(type)) {
        ret = GetPolicyFromMap(type);
    }

    return ret;
}

int32_t PowerModePolicy::GetPolicyFromMap(uint32_t type)
{
    int32_t ret = INIT_VALUE_FALSE;
    valueiter = valueModePolicy.find(type);
    if (valueiter != valueModePolicy.end()) {
        ret = valueiter->second;
        return ret;
    } else {
        return ret;
    }
}

int32_t PowerModePolicy::GetRecoverPolicyFromMap(uint32_t type)
{
    int32_t ret = INIT_VALUE_FALSE;
    recoveriter = recoverModePolicy.find(type);
    if (recoveriter != recoverModePolicy.end()) {
        ret = recoveriter->second;
        POWER_HILOGD(FEATURE_POWER_MODE, "Recover value: %{public}d", ret);
    }
    return ret;
}

int32_t PowerModePolicy::GetPowerModeRecoverPolicy(uint32_t type)
{
    int32_t ret = INIT_VALUE_FALSE;
    if (IsValidType(type)) {
        ret = GetRecoverPolicyFromMap(type);
    }

    return ret;
}

void PowerModePolicy::SetPowerModePolicy(uint32_t mode, uint32_t lastMode)
{
    POWER_HILOGD(FEATURE_POWER_MODE, "mode=%{public}d, lastMode=%{public}d", mode, lastMode);
    if (lastMode != LAST_MODE_FLAG) {
        ReadRecoverPolicy(lastMode);
    }

    ReadOpenPolicy(mode);

    CompareModeItem(mode, lastMode);
}

void PowerModePolicy::ReadOpenPolicy(uint32_t mode)
{
    DelayedSpSingleton<PowerSaveMode>::GetInstance()->GetValuePolicy(openPolicy, mode);
}

void PowerModePolicy::ReadRecoverPolicy(uint32_t mode)
{
    DelayedSpSingleton<PowerSaveMode>::GetInstance()->GetRecoverPolicy(recoverPolicy, mode);
}

void PowerModePolicy::CompareModeItem(uint32_t mode, uint32_t lastMode)
{
    recoverModePolicy.clear();
    valueModePolicy.clear();

    for (auto openlit = openPolicy.begin(); openlit != openPolicy.end(); openlit++) {
        valueModePolicy[(*openlit).id] = (*openlit).value;
    }

    for (recoverlit = recoverPolicy.begin(); recoverlit != recoverPolicy.end(); recoverlit++) {
        recoverModePolicy[(*recoverlit).id] = (*recoverlit).value;
        POWER_HILOGD(FEATURE_POWER_MODE,
            "(*recoverlit).id=%{public}d, (*recoverlit).value=%{public}d", (*recoverlit).id, (*recoverlit).value);
    }

    openPolicy.clear();
    recoverPolicy.clear();
}

void PowerModePolicy::AddAction(uint32_t type, ModeAction& action)
{
    POWER_HILOGD(FEATURE_POWER_MODE, "type=%{public}d", type);
    actionMap.emplace(type, action);
}

void PowerModePolicy::TriggerAllActions(bool isBoot)
{
    for (auto iterator = actionMap.begin(); iterator != actionMap.end(); iterator++) {
        POWER_HILOGD(FEATURE_POWER_MODE, "type=%{public}d", iterator->first);
        iterator->second(isBoot);
    }
}

bool PowerModePolicy::IsValidType(uint32_t type)
{
    auto iterator = actionMap.find(type);
    if (iterator == actionMap.end()) {
        POWER_HILOGW(FEATURE_POWER_MODE, "Invalid type: %{public}d", type);
        return false;
    }
    return true;
}
} // namespace PowerMgr
} // namespace OHOS
