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

#include <cJSON.h>
#include "power_mode_policy.h"
#include "power_mgr_service.h"
#include "power_log.h"
#include "power_save_mode.h"
#include "singleton.h"
#include "setting_helper.h"
using namespace std;

namespace OHOS {
namespace PowerMgr {
constexpr int32_t KEY_BASE = 10;

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
    std::lock_guard<std::mutex> lock(policyMutex_);
    auto iter = switchMap_.find(type);
    if (iter != switchMap_.end()) {
        ret = iter->second;
    }
    return ret;
}

void PowerModePolicy::UpdatePowerModePolicy(uint32_t mode)
{
    POWER_HILOGD(FEATURE_POWER_MODE, "update mode policy, mode=%{public}d", mode);
    ReadPowerModePolicy(mode);
    ComparePowerModePolicy();
}

void PowerModePolicy::ComparePowerModePolicy()
{
    std::lock_guard<std::mutex> lock(policyMutex_);
    for (auto [id, value] : recoverMap_) {
        if (switchMap_.find(id) != switchMap_.end()) {
            backupMap_[id] = value;
        }
        switchMap_.emplace(id, value);
    }
    recoverMap_ = backupMap_;
    SavePowerModeRecoverMap();
}

bool PowerModePolicy::InitRecoverMap()
{
    std::string jsonStr = SettingHelper::ReadPowerModeRecoverMap();

    cJSON* recoverJson = cJSON_Parse(jsonStr.c_str());
    if (!recoverJson) {
        POWER_HILOGW(FEATURE_POWER_MODE, "parse recover json str error");
        return false;
    }
    if (!cJSON_IsObject(recoverJson)) {
        POWER_HILOGW(FEATURE_POWER_MODE, "recover json root is not an object");
        cJSON_Delete(recoverJson);
        return false;
    }
    cJSON* item = nullptr;
    cJSON_ArrayForEach(item, recoverJson) {
        const char* keyStr = item->string;
        if (!keyStr || !cJSON_IsNumber(item)) {
            continue;
        }
        errno = 0;
        int32_t key = static_cast<int32_t>(strtol(keyStr, nullptr, KEY_BASE));
        if (errno == ERANGE && (key == INT_MAX || key == INT_MIN)) {
            continue;
        }
        int32_t value = static_cast<int32_t>(item->valueint);
        recoverMap_[key] = value;
    }

    cJSON_Delete(recoverJson);
    POWER_HILOGI(FEATURE_POWER_MODE, "init recover map succeed");
    return true;
}

void PowerModePolicy::ReadPowerModePolicy(uint32_t mode)
{
    auto policyCache = DelayedSpSingleton<PowerSaveMode>::GetInstance()->GetPolicyCache();
    if (policyCache.empty()) {
        POWER_HILOGD(FEATURE_POWER_MODE, "config policy cache is empty");
        return;
    }

    switchMap_.clear();
    backupMap_.clear();
    for (auto [id, value, flag] : policyCache[mode]) {
        switchMap_[id] = value;
        POWER_HILOGD(FEATURE_POWER_MODE, "read switch id: %{public}d, value: %{public}d", id, value);
        if (flag == ValueProp::recover) {
            GetSettingSwitchState(id, backupMap_[id]);
        }
    }
}

int64_t PowerModePolicy::GetSettingDisplayOffTime(int64_t defaultVal)
{
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        POWER_HILOGE(FEATURE_POWER_MODE, "get PowerMgrService fail");
        return defaultVal;
    }
    return pms->GetSettingDisplayOffTime(defaultVal);
}

void PowerModePolicy::GetSettingSwitchState(uint32_t& switchId, int32_t& value)
{
    int32_t defaultVal = INIT_VALUE_FALSE;
    switch (switchId) {
        case PowerModePolicy::ServiceType::AUTO_ADJUST_BRIGHTNESS:
            defaultVal = SettingHelper::GetSettingAutoAdjustBrightness(defaultVal);
            break;
        case PowerModePolicy::ServiceType::AUTO_WINDOWN_RORATION:
            defaultVal = SettingHelper::GetSettingWindowRotation(defaultVal);
            break;
        case PowerModePolicy::ServiceType::VIBRATORS_STATE:
            defaultVal = SettingHelper::GetSettingVibration(defaultVal);
            break;
        case PowerModePolicy::ServiceType::INTELL_VOICE:
            defaultVal = SettingHelper::GetSettingIntellVoice(defaultVal);
            break;
        case PowerModePolicy::ServiceType::DISPLAY_OFFTIME: {
            int64_t displayOfftime = GetSettingDisplayOffTime(static_cast<int64_t>(defaultVal));
            defaultVal = static_cast<int32_t>(displayOfftime);
            break;
        }
        default:
            break;
    }

    if (defaultVal == INIT_VALUE_FALSE) {
        POWER_HILOGW(FEATURE_POWER_MODE, "get setting state invalid, switch id: %{public}d", switchId);
        return;
    }
    value = defaultVal;
    POWER_HILOGD(FEATURE_POWER_MODE, "read switch id: %{public}d, switch value: %{public}d", switchId, value);
}

void PowerModePolicy::AddAction(uint32_t type, ModeAction& action)
{
    POWER_HILOGD(FEATURE_POWER_MODE, "add action, type=%{public}d", type);
    std::lock_guard<std::mutex> lock(actionMapMutex_);
    actionMap_.emplace(type, action);
}

void PowerModePolicy::TriggerAllActions(bool isBoot)
{
    std::vector<ModeAction> allActions;
    {
        std::lock_guard<std::mutex> lock(actionMapMutex_);
        for (auto iterator = actionMap_.begin(); iterator != actionMap_.end(); iterator++) {
            POWER_HILOGD(FEATURE_POWER_MODE, "trigger action, type=%{public}d", iterator->first);
            allActions.emplace_back(iterator->second);
        }
    }
    for (const auto &actions : allActions) {
        actions(isBoot);
    }
}

bool PowerModePolicy::IsValidType(uint32_t type)
{
    std::lock_guard<std::mutex> lock(actionMapMutex_);
    auto iterator = actionMap_.find(type);
    if (iterator == actionMap_.end()) {
        POWER_HILOGW(FEATURE_POWER_MODE, "Invalid type: %{public}d", type);
        return false;
    }
    return true;
}

void PowerModePolicy::RemoveBackupMapSettingSwitch(uint32_t switchId)
{
    auto iter = recoverMap_.find(switchId);
    if (iter != recoverMap_.end()) {
        recoverMap_.erase(iter);
        SavePowerModeRecoverMap();
        POWER_HILOGW(FEATURE_POWER_MODE, "remove backup switch: %{public}d", switchId);
    }
}

void PowerModePolicy::SavePowerModeRecoverMap()
{
    cJSON* recoverJson = cJSON_CreateObject();
    if (!recoverJson) {
        POWER_HILOGE(FEATURE_POWER_MODE, "Failed to create cJSON object");
        return;
    }

    for (const auto& pair : recoverMap_) {
        std::string keyStr = std::to_string(pair.first);
        cJSON_AddNumberToObject(recoverJson, keyStr.c_str(), pair.second);
    }

    char* jsonStr = cJSON_Print(recoverJson);
    if (!jsonStr) {
        POWER_HILOGE(FEATURE_POWER_MODE, "Failed to print cJSON to string");
        cJSON_Delete(recoverJson);
        return;
    }
    std::string jsonConfig = std::string(jsonStr);
    SettingHelper::SavePowerModeRecoverMap(jsonConfig);
    cJSON_free(jsonStr);
    cJSON_Delete(recoverJson);
}
} // namespace PowerMgr
} // namespace OHOS
