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

#include <fstream>
#include <unistd.h>

#include "config_policy_utils.h"
#include "power_log.h"
#include "setting_helper.h"
#include "wakeup_source_parser.h"
#include "json/reader.h"
#include "json/value.h"

namespace OHOS {
namespace PowerMgr {

namespace {
static const std::string POWER_WAKEUP_CONFIG_FILE = "etc/power_config/power_wakeup.json";
static const std::string VENDOR_POWER_WAKEUP_CONFIG_FILE = "/vendor/etc/power_config/power_wakeup.json";
static const std::string SYSTEM_POWER_WAKEUP_CONFIG_FILE = "/system/etc/power_config/power_wakeup.json";
static const uint32_t SINGLE_CLICK = static_cast<uint32_t>(WakeUpAction::CLICK_SINGLE);
static const uint32_t DOUBLE_CLICK = static_cast<uint32_t>(WakeUpAction::CLICK_DOUBLE);
} // namespace

std::shared_ptr<WakeupSources> WakeupSourceParser::ParseSources()
{
    std::shared_ptr<WakeupSources> parseSources;
    bool isSettingUpdated = SettingHelper::IsWakeupSourcesSettingValid();
    POWER_HILOGI(FEATURE_WAKEUP, "ParseSources setting=%{public}d", isSettingUpdated);
    std::string configJsonStr;
    if (isSettingUpdated) {
        configJsonStr = SettingHelper::GetSettingWakeupSources();
    } else {
        std::string targetPath;
        bool ret = GetTargetPath(targetPath);
        if (ret == false) {
            return parseSources;
        }

        POWER_HILOGI(FEATURE_WAKEUP, "use targetPath=%{public}s", targetPath.c_str());
        std::ifstream inputStream(targetPath.c_str(), std::ios::in | std::ios::binary);
        std::string fileStringStr(std::istreambuf_iterator<char> {inputStream}, std::istreambuf_iterator<char> {});
        configJsonStr = fileStringStr;
    }
    parseSources = ParseSources(configJsonStr);
    if (parseSources != nullptr) {
        SettingHelper::SetSettingWakeupSources(configJsonStr);
    }
    return parseSources;
}

bool WakeupSourceParser::GetTargetPath(std::string& targetPath)
{
    targetPath.clear();
    char buf[MAX_PATH_LEN];
    char* path = GetOneCfgFile(POWER_WAKEUP_CONFIG_FILE.c_str(), buf, MAX_PATH_LEN);
    if (path != nullptr && *path != '\0') {
        POWER_HILOGI(FEATURE_WAKEUP, "use policy path=%{public}s", path);
        targetPath = path;
        return true;
    }

    if (access(VENDOR_POWER_WAKEUP_CONFIG_FILE.c_str(), F_OK | R_OK) == -1) {
        POWER_HILOGE(FEATURE_WAKEUP, "vendor wakeup config is not exist or permission denied");
        if (access(SYSTEM_POWER_WAKEUP_CONFIG_FILE.c_str(), F_OK | R_OK) == -1) {
            POWER_HILOGE(FEATURE_WAKEUP, "system wakeup config is not exist or permission denied");
            return false;
        } else {
            targetPath = SYSTEM_POWER_WAKEUP_CONFIG_FILE;
        }
    } else {
        targetPath = VENDOR_POWER_WAKEUP_CONFIG_FILE;
    }

    return true;
}

std::shared_ptr<WakeupSources> WakeupSourceParser::ParseSources(const std::string& jsonStr)
{
    std::shared_ptr<WakeupSources> parseSources = std::make_shared<WakeupSources>();
    Json::Reader reader;
    Json::Value root;
    std::string errors;
    if (!reader.parse(jsonStr.data(), jsonStr.data() + jsonStr.size(), root)) {
        POWER_HILOGE(FEATURE_WAKEUP, "json parse error");
        return parseSources;
    }

    Json::Value::Members members = root.getMemberNames();
    for (auto iter = members.begin(); iter != members.end(); iter++) {
        std::string key = *iter;
        Json::Value valueObj = root[key];

        POWER_HILOGI(FEATURE_WAKEUP, "key=%{public}s", key.c_str());

        bool ret = ParseSourcesProc(parseSources, valueObj, key);
        if (ret == false) {
            POWER_HILOGI(FEATURE_WAKEUP, "lost map config key");
            continue;
        }
    }

    return parseSources;
}

bool WakeupSourceParser::ParseSourcesProc(
    std::shared_ptr<WakeupSources>& parseSources, Json::Value& valueObj, std::string& key)
{
    bool enable = true;
    uint32_t click = DOUBLE_CLICK;
    WakeupDeviceType wakeupDeviceType = WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN;
    if (valueObj.isObject()) {
        Json::Value enableValue = valueObj[WakeupSource::ENABLE_KEY];
        Json::Value clickValue = valueObj[WakeupSource::KEYS_KEY];
        if (!clickValue.isNull() && clickValue.isUInt()) {
            POWER_HILOGI(FEATURE_WAKEUP, "clickValue=%{public}u", clickValue.asUInt());
            click = (clickValue.asUInt() == SINGLE_CLICK || clickValue.asUInt() == DOUBLE_CLICK) ? clickValue.asUInt() :
                                                                                                   DOUBLE_CLICK;
        }
        if (enableValue.isBool()) {
            enable = enableValue.asBool();
            POWER_HILOGD(FEATURE_WAKEUP, "enable=%{public}d", enable);
        }
    }

    wakeupDeviceType = WakeupSources::mapWakeupDeviceType(key, click);
    POWER_HILOGI(FEATURE_WAKEUP, "key map type=%{public}u", wakeupDeviceType);

    if (wakeupDeviceType == WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN) {
        return false;
    }
    if (enable == true) {
        WakeupSource wakeupSource = WakeupSource(wakeupDeviceType, enable, click);
        parseSources->PutSource(wakeupSource);
    }

    return true;
}

} // namespace PowerMgr
} // namespace OHOS