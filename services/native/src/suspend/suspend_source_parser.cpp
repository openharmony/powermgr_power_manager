/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "suspend_source_parser.h"

#include <fstream>
#include <securec.h>
#include <unistd.h>

#include <cJSON.h>

#include "config_policy_utils.h"
#include "power_log.h"
#include "setting_helper.h"
#ifdef POWER_MANAGER_ENABLE_CHARGING_TYPE_SETTING
#include "power_mgr_service.h"
#endif

namespace OHOS {
namespace PowerMgr {

namespace {
static const std::string POWER_SUSPEND_CONFIG_FILE = "etc/power_config/power_suspend.json";
static const std::string VENDOR_POWER_SUSPEND_CONFIG_FILE = "/vendor/etc/power_config/power_suspend.json";
static const std::string SYSTEM_POWER_SUSPEND_CONFIG_FILE = "/system/etc/power_config/power_suspend.json";
static const uint32_t ILLEGAL_ACTION = static_cast<uint32_t>(SuspendAction::ACTION_INVALID);
} // namespace

#ifdef POWER_MANAGER_ENABLE_CHARGING_TYPE_SETTING
std::shared_ptr<SuspendSources> SuspendSourceParser::ParseSources()
{
    std::shared_ptr<SuspendSources> parseSources{nullptr};
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        POWER_HILOGE(FEATURE_SUSPEND, "get PowerMgrService fail");
        return parseSources;
    }
    bool isPowerConnected = pms->IsPowerConnected();
    bool isSettingAcValid = SettingHelper::IsSettingAcSuspendSourcesValid();
    bool isSettingDcValid = SettingHelper::IsSettingDcSuspendSourcesValid();
    std::string configJsonStr;

    if (!isSettingAcValid || !isSettingDcValid) {
        std::string fileStringStr = GetSuspendSourcesByConfig();

        if (!isSettingAcValid) {
            SettingHelper::SetSettingAcSuspendSources(fileStringStr);
        }
        if (!isSettingDcValid) {
            SettingHelper::SetSettingDcSuspendSources(fileStringStr);
        }
        configJsonStr = fileStringStr;
    }

    if (isPowerConnected && isSettingAcValid) {
        configJsonStr = SettingHelper::GetSettingAcSuspendSources();
    } else if (!isPowerConnected && isSettingDcValid) {
        configJsonStr = SettingHelper::GetSettingDcSuspendSources();
    }

    parseSources = ParseSources(configJsonStr);
    if (parseSources->GetParseErrorFlag()) {
        POWER_HILOGI(FEATURE_SUSPEND, "call GetSuspendSourcesByConfig again");
        configJsonStr = GetSuspendSourcesByConfig();
        parseSources = ParseSources(configJsonStr);
    }
    return parseSources;
}
#else
std::shared_ptr<SuspendSources> SuspendSourceParser::ParseSources()
{
    std::shared_ptr<SuspendSources> parseSources{nullptr};
    bool isSettingUpdated = SettingHelper::IsSuspendSourcesSettingValid();
    POWER_HILOGI(FEATURE_SUSPEND, "ParseSources setting=%{public}d", isSettingUpdated);
    std::string configJsonStr;
    if (isSettingUpdated) {
        std::string sourcesSettingStr = SettingHelper::GetSettingSuspendSources();
        configJsonStr = sourcesSettingStr;
#ifdef POWER_MANAGER_ENABLE_WATCH_UPDATE_ADAPT
        // this branch means use config file for update scene in watch
        if (sourcesSettingStr.find(SuspendSources::TP_COVER_KEY) == std::string::npos) {
            configJsonStr = GetSuspendSourcesByConfig();
            POWER_HILOGW(FEATURE_SUSPEND, "update scene need use (config file)");
        }
#endif
    } else {
        configJsonStr = GetSuspendSourcesByConfig();
    }
    parseSources = ParseSources(configJsonStr);
    if (parseSources->GetParseErrorFlag()) {
        POWER_HILOGI(FEATURE_SUSPEND, "call GetSuspendSourcesByConfig again");
        configJsonStr = GetSuspendSourcesByConfig();
        parseSources = ParseSources(configJsonStr);
    }
    if (parseSources != nullptr) {
        SettingHelper::SetSettingSuspendSources(configJsonStr);
    }
    return parseSources;
}
#endif

const std::string SuspendSourceParser::GetSuspendSourcesByConfig()
{
    std::string targetPath;
    bool ret = GetTargetPath(targetPath);
    if (ret == false) {
        POWER_HILOGE(FEATURE_SUSPEND, "GetTargetPath fail");
        return "";
    }
    POWER_HILOGI(FEATURE_SUSPEND, "use targetPath=%{public}s", targetPath.c_str());
    std::ifstream inputStream(targetPath.c_str(), std::ios::in | std::ios::binary);
    return std::string(std::istreambuf_iterator<char> {inputStream}, std::istreambuf_iterator<char> {});
}

bool SuspendSourceParser::GetTargetPath(std::string& targetPath)
{
    targetPath.clear();
    bool ret = true;
    char buf[MAX_PATH_LEN];
    char* path = GetOneCfgFile(POWER_SUSPEND_CONFIG_FILE.c_str(), buf, MAX_PATH_LEN);
    if (path != nullptr && *path != '\0') {
        POWER_HILOGI(FEATURE_SUSPEND, "use policy path=%{public}s", path);
        targetPath = path;
        return true;
    }

    if (access(VENDOR_POWER_SUSPEND_CONFIG_FILE.c_str(), F_OK | R_OK) == -1) {
        POWER_HILOGE(FEATURE_SUSPEND, "vendor suspend config is not exist or permission denied");
        if (access(SYSTEM_POWER_SUSPEND_CONFIG_FILE.c_str(), F_OK | R_OK) == -1) {
            POWER_HILOGE(FEATURE_SUSPEND, "system suspend config is not exist or permission denied");
            ret = false;
        } else {
            targetPath = SYSTEM_POWER_SUSPEND_CONFIG_FILE;
        }
    } else {
        targetPath = VENDOR_POWER_SUSPEND_CONFIG_FILE;
    }
    return ret;
}

std::shared_ptr<SuspendSources> SuspendSourceParser::ParseSources(const std::string& jsonStr)
{
    std::shared_ptr<SuspendSources> parseSources = std::make_shared<SuspendSources>();
    cJSON* root = cJSON_Parse(jsonStr.c_str());
    if (!root) {
        POWER_HILOGE(FEATURE_SUSPEND, "json parse error");
        parseSources->SetParseErrorFlag(true);
        return parseSources;
    }
    if (!cJSON_IsObject(root)) {
        POWER_HILOGE(FEATURE_SUSPEND, "json root invalid[%{public}s]", jsonStr.c_str());
        parseSources->SetParseErrorFlag(true);
        cJSON_Delete(root);
        return parseSources;
    }

    cJSON* item = nullptr;
    cJSON_ArrayForEach(item, root) {
        const char* key = item->string;
        if (!key) {
            POWER_HILOGI(FEATURE_SUSPEND, "invalid key in json object");
            continue;
        }
        std::string keyStr = std::string(key);
        bool ret = ParseSourcesProc(parseSources, item, keyStr);
        if (ret == false) {
            POWER_HILOGI(FEATURE_SUSPEND, "lost map config key");
            continue;
        }
    }

    cJSON_Delete(root);
    return parseSources;
}

bool SuspendSourceParser::ParseSourcesProc(
    std::shared_ptr<SuspendSources>& parseSources, cJSON* valueObj, std::string& key)
{
    if (parseSources == nullptr) {
        POWER_HILOGE(FEATURE_SUSPEND, "parseSources is nullptr");
        return false;
    }

    SuspendDeviceType suspendDeviceType = SuspendSources::mapSuspendDeviceType(key);
    if (suspendDeviceType == SuspendDeviceType::SUSPEND_DEVICE_REASON_MIN) {
        return false;
    }

    uint32_t action = 0;
    uint32_t delayMs = 0;
    if (valueObj && cJSON_IsObject(valueObj)) {
        cJSON* actionValue = cJSON_GetObjectItemCaseSensitive(valueObj, SuspendSource::ACTION_KEY);
        cJSON* delayValue = cJSON_GetObjectItemCaseSensitive(valueObj, SuspendSource::DELAY_KEY);
        if (actionValue && cJSON_IsNumber(actionValue) && delayValue && cJSON_IsNumber(delayValue)) {
            action = static_cast<uint32_t>(actionValue->valueint);
            delayMs = static_cast<uint32_t>(delayValue->valueint);
            if (action >= ILLEGAL_ACTION) {
                action = 0;
            }
        }
    }

    POWER_HILOGI(FEATURE_SUSPEND,
        "ParseSourcesProc key=%{public}s, type=%{public}u, action=%{public}u, delayMs=%{public}u",
        key.c_str(), suspendDeviceType, action, delayMs);
    SuspendSource suspendSource = SuspendSource(suspendDeviceType, action, delayMs);
    parseSources->PutSource(suspendSource);
    return true;
}

} // namespace PowerMgr
} // namespace OHOS