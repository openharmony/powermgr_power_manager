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
#include "config_policy_utils.h"
#include "power_log.h"
#include "json/reader.h"
#include "json/value.h"

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
    Json::Reader reader;
    Json::Value root;
    if (!reader.parse(jsonStr.data(), jsonStr.data() + jsonStr.size(), root)) {
        POWER_HILOGE(COMP_UTILS, "json parse error");
        return sources;
    }
    Json::Value::Members members = root.getMemberNames();
    for (auto iter = members.begin(); iter != members.end(); iter++) {
        std::string key = *iter;
        Json::Value valueObj = root[key];
        POWER_HILOGI(COMP_UTILS, "key=%{public}s", key.c_str());
        ParseSourcesProc(sources, valueObj, key);
    }
    return sources;
}
   
void VibratorSourceParser::ParseSourcesProc(
    std::vector<VibratorSource>& sources, Json::Value& valueObj, std::string& key)
{
    if (!valueObj.isObject()) {
        return;
    }
    std::string type;
    bool enable = false;
    Json::Value enableValue = valueObj[VibratorSource::ENABLE_KEY];
    Json::Value typeValue = valueObj[VibratorSource::TYPE_KEY];
    if (!typeValue.isString() || !enableValue.isBool()) {
        return;
    }
    enable = enableValue.asBool();
    type = typeValue.asString();
    if (!enable || type.empty()) {
        return;
    }
    VibratorSource vibratorSource = VibratorSource(key, enable, type);
    sources.emplace_back(vibratorSource);
}

}
}