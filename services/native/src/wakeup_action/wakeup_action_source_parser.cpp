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
#include "wakeup_action_source_parser.h"

#include <fstream>
#include <securec.h>
#include <unistd.h>

#include <cJSON.h>
#include "config_policy_utils.h"
#include "power_log.h"

namespace OHOS {
namespace PowerMgr {

namespace {
static const std::string POWER_WAKEUP_ACTION_CONFIG_FILE = "etc/power_config/power_wakeup_action.json";
static const std::string VENDOR_POWER_WAKEUP_ACTION_CONFIG_FILE = "/vendor/etc/power_config/power_wakeup_action.json";
static const std::string SYSTEM_POWER_WAKEUP_ACTION_CONFIG_FILE = "/system/etc/power_config/power_wakeup_action.json";
static const uint32_t ILLEGAL_ACTION = static_cast<uint32_t>(WakeupAction::ACTION_INVALID);
} // namespace

std::shared_ptr<WakeupActionSources> WakeupActionSourceParser::ParseSources()
{
    std::shared_ptr<WakeupActionSources> parseSources;
    std::string configJsonStr;
    std::string targetPath;
    bool ret = GetTargetPath(targetPath);
    if (ret == false) {
        return parseSources;
    }
    POWER_HILOGI(FEATURE_WAKEUP_ACTION, "use targetPath=%{public}s", targetPath.c_str());
    std::ifstream inputStream(targetPath.c_str(), std::ios::in | std::ios::binary);
    std::string fileStringStr(std::istreambuf_iterator<char> {inputStream}, std::istreambuf_iterator<char> {});
    configJsonStr = fileStringStr;
    parseSources = ParseSources(configJsonStr);
    return parseSources;
}

bool WakeupActionSourceParser::GetTargetPath(std::string& targetPath)
{
    targetPath.clear();
    bool ret = true;
    char buf[MAX_PATH_LEN];
    char* path = GetOneCfgFile(POWER_WAKEUP_ACTION_CONFIG_FILE.c_str(), buf, MAX_PATH_LEN);
    if (path != nullptr && *path != '\0') {
        POWER_HILOGI(FEATURE_WAKEUP_ACTION, "use policy path=%{public}s", path);
        targetPath = path;
        return true;
    }

    if (access(VENDOR_POWER_WAKEUP_ACTION_CONFIG_FILE.c_str(), F_OK | R_OK) == -1) {
        POWER_HILOGE(FEATURE_WAKEUP_ACTION, "vendor WakeupAction config is not exist or permission denied");
        if (access(SYSTEM_POWER_WAKEUP_ACTION_CONFIG_FILE.c_str(), F_OK | R_OK) == -1) {
            POWER_HILOGE(FEATURE_WAKEUP_ACTION, "system WakeupAction config is not exist or permission denied");
            ret = false;
        } else {
            targetPath = SYSTEM_POWER_WAKEUP_ACTION_CONFIG_FILE;
        }
    } else {
        targetPath = VENDOR_POWER_WAKEUP_ACTION_CONFIG_FILE;
    }
    return ret;
}

std::shared_ptr<WakeupActionSources> WakeupActionSourceParser::ParseSources(const std::string& jsonStr)
{
    std::shared_ptr<WakeupActionSources> parseSources = std::make_shared<WakeupActionSources>();
    cJSON* root = cJSON_Parse(jsonStr.c_str());
    if (!root) {
        POWER_HILOGE(FEATURE_WAKEUP_ACTION, "json parse error[%{public}s]", jsonStr.c_str());
        return parseSources;
    }

    if (!cJSON_IsObject(root)) {
        POWER_HILOGE(FEATURE_WAKEUP_ACTION, "json root invalid");
        cJSON_Delete(root);
        return parseSources;
    }

    cJSON* item = nullptr;
    cJSON_ArrayForEach(item, root) {
        const char* key = item->string;
        if (!key) {
            POWER_HILOGI(FEATURE_WAKEUP_ACTION, "invalid key in json object");
            continue;
        }
        POWER_HILOGI(FEATURE_WAKEUP_ACTION, "key=%{public}s", key);
        std::string keyStr = std::string(key);
        bool ret = ParseSourcesProc(parseSources, item, keyStr);
        if (ret == false) {
            POWER_HILOGI(FEATURE_WAKEUP_ACTION, "lost map config key");
            continue;
        }
    }

    cJSON_Delete(root);
    return parseSources;
}

bool WakeupActionSourceParser::ParseSourcesProc(
    std::shared_ptr<WakeupActionSources>& parseSources, cJSON* valueObj, std::string& key)
{
    std::string scene{""};
    uint32_t action = 0;
    if (valueObj && cJSON_IsObject(valueObj)) {
        cJSON* sceneValue = cJSON_GetObjectItemCaseSensitive(valueObj, WakeupActionSource::SCENE_KEY);
        if (sceneValue && cJSON_IsString(sceneValue)) {
            scene = sceneValue->valuestring;
            POWER_HILOGI(FEATURE_WAKEUP_ACTION, "scene=%{public}s", scene.c_str());
        }
        cJSON* actionValue = cJSON_GetObjectItemCaseSensitive(valueObj, WakeupActionSource::ACTION_KEY);
        if (actionValue && cJSON_IsNumber(actionValue)) {
            action = static_cast<uint32_t>(actionValue->valueint);
            POWER_HILOGI(FEATURE_WAKEUP_ACTION, "action=%{public}u", action);
            if (action >= ILLEGAL_ACTION) {
                action = 0;
            }
        }
    }

    if (action != 0) {
        std::shared_ptr<WakeupActionSource> wakeupActionSource = std::make_shared<WakeupActionSource>(scene, action);
        parseSources->PutSource(key, wakeupActionSource);
    }

    return true;
}

} // namespace PowerMgr
} // namespace OHOS