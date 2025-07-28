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

#include "vibrator_source_parser.h"

#include <fstream>
#include <securec.h>
#include <unistd.h>
#include <cJSON.h>
#include "config_policy_utils.h"
#include "power_cjson_utils.h"
#include "power_log.h"

namespace OHOS {
namespace PowerMgr {
std::vector<VibratorSource> VibratorSourceParser::ParseSources(
    const std::string& etcPath, const std::string& vendorPath, const std::string& systemPath)
{
    std::vector<VibratorSource> sources;
    std::string targetPath;
    GetTargetPath(targetPath, etcPath, vendorPath, systemPath);
    if (targetPath.empty()) {
        POWER_HILOGE(COMP_UTILS, "targetPath is null");
        return sources;
    }
    POWER_HILOGI(COMP_UTILS, "use targetPath=%{public}s", targetPath.c_str());
    std::ifstream inputStream(targetPath.c_str(), std::ios::in | std::ios::binary);
    std::string fileStringStr(std::istreambuf_iterator<char> {inputStream}, std::istreambuf_iterator<char> {});
    targetPath = fileStringStr;
    sources = ParseSources(targetPath);
    return sources;
}

void VibratorSourceParser::GetTargetPath(
    std::string& targetPath, const std::string& etcPath, const std::string& vendorPath, const std::string& systemPath)
{
    targetPath.clear();
    char buf[MAX_PATH_LEN];
    char* path = GetOneCfgFile(etcPath.c_str(), buf, MAX_PATH_LEN);
    if (path != nullptr && *path != '\0') {
        POWER_HILOGI(COMP_UTILS, "use policy path=%{public}s", path);
        targetPath = path;
        return;
    }

    if (access(vendorPath.c_str(), F_OK | R_OK) == -1) {
        POWER_HILOGE(COMP_UTILS, "vendor vibrator config is not exist or permission denied");
        if (access(systemPath.c_str(), F_OK | R_OK) == -1) {
            POWER_HILOGE(COMP_UTILS, "system vibrator config is not exist or permission denied");
            return;
        } else {
            targetPath = systemPath;
        }
    } else {
        targetPath = vendorPath;
    }
}
    
std::vector<VibratorSource> VibratorSourceParser::ParseSources(const std::string& jsonStr)
{
    std::vector<VibratorSource> sources;

    if (jsonStr.empty()) {
        POWER_HILOGE(COMP_UTILS, "Input JSON string is empty");
        return sources;
    }

    cJSON* root = cJSON_Parse(jsonStr.c_str());
    if (!root) {
        POWER_HILOGE(COMP_UTILS, "JSON parse error");
        return sources;
    }
    if (!PowerMgrJsonUtils::IsValidJsonObjectOrJsonArray(root)) {
        POWER_HILOGE(COMP_UTILS, "JSON root is not object");
        cJSON_Delete(root);
        return sources;
    }

    cJSON* item = nullptr;
    cJSON_ArrayForEach(item, root) {
        const char* key = item->string;
        if (!key) {
            POWER_HILOGE(COMP_UTILS, "invalid key in json object");
            continue;
        }
        std::string keyStr = std::string(key);
        POWER_HILOGI(COMP_UTILS, "key=%{public}s", keyStr.c_str());
        ParseSourcesProc(sources, item, keyStr);
    }

    cJSON_Delete(root);
    return sources;
}
   
void VibratorSourceParser::ParseSourcesProc(
    std::vector<VibratorSource>& sources, cJSON* valueObj, std::string& key)
{
    if (!PowerMgrJsonUtils::IsValidJsonObject(valueObj)) {
        POWER_HILOGE(COMP_UTILS, "ValueObj is not a json object.");
        return;
    }

    bool enable = false;
    std::string type;

    cJSON* enableItem = cJSON_GetObjectItemCaseSensitive(valueObj, VibratorSource::ENABLE_KEY);
    if (!PowerMgrJsonUtils::IsValidJsonBool(enableItem)) {
        POWER_HILOGE(COMP_UTILS, "Parse enable error.");
        return;
    }
    enable =  cJSON_IsTrue(enableItem);

    cJSON* typeItem = cJSON_GetObjectItemCaseSensitive(valueObj, VibratorSource::TYPE_KEY);
    if (!PowerMgrJsonUtils::IsValidJsonString(typeItem)) {
        POWER_HILOGE(COMP_UTILS, "Parse type error.");
        return;
    }
    type = typeItem->valuestring;

    if (!enable || type.empty()) {
        return;
    }
    VibratorSource vibratorSource = VibratorSource(key, enable, type);
    sources.emplace_back(vibratorSource);
}

}
}