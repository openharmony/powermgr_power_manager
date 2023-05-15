/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
static const uint32_t DOUBLE_CLICK = static_cast<uint32_t>(WakeUpAction::CLICK_DOUBLE);
} // namespace

std::shared_ptr<WakeupSources> WakeupSourceParser::ParseSources()
{
    std::shared_ptr<WakeupSources> parseSources;
    bool isSettingUpdated = SettingHelper::IsWakeupSourcesSettingValid();
    POWER_HILOGI(COMP_SVC, "ParseSources setting %{public}d", isSettingUpdated);
    std::string configJsonStr;
    if (isSettingUpdated) {
        configJsonStr = SettingHelper::GetSettingWakeupSources();
    } else {
        char buf[MAX_PATH_LEN];
        char targetPath[MAX_PATH_LEN] = {0};
        char* path = GetOneCfgFile(POWER_WAKEUP_CONFIG_FILE.c_str(), buf, MAX_PATH_LEN);
        if (path != nullptr && *path != '\0') {
            POWER_HILOGI(COMP_SVC, "use policy path %{public}s", path);
            if (strcpy_s(targetPath, sizeof(targetPath), path) != EOK) {
                POWER_HILOGE(COMP_SVC, "strcpy_s error");
                return parseSources;
            }
        } else {
            bool ret = GetTargetPath(targetPath);
            if(ret == false)
            {
                return parseSources;       
            }
        }
        POWER_HILOGI(COMP_SVC, "use targetPath %{public}s", targetPath);
        if (access(SYSTEM_POWER_WAKEUP_CONFIG_FILE.c_str(), F_OK | R_OK) == -1) {
            POWER_HILOGE(COMP_SVC, "system wakeup config is not exist or permission denied");
            return parseSources;
        }
        std::ifstream inputStream(SYSTEM_POWER_WAKEUP_CONFIG_FILE.c_str(), std::ios::in | std::ios::binary);
        std::string fileStringStr(std::istreambuf_iterator<char> {inputStream}, std::istreambuf_iterator<char> {});
        configJsonStr = fileStringStr;
    }
    parseSources = ParseSources(configJsonStr);
    if (parseSources != nullptr) {
        SettingHelper::SetSettingWakeupSources(configJsonStr);
    }
    return parseSources;
}

bool WakeupSourceParser::GetTargetPath(char* targetPath)
{
    bool ret = true;
    if (access(VENDOR_POWER_WAKEUP_CONFIG_FILE.c_str(), F_OK | R_OK) == -1) {
        POWER_HILOGE(COMP_SVC, "vendor suspend config is not exist or permission denied");
        if (access(SYSTEM_POWER_WAKEUP_CONFIG_FILE.c_str(), F_OK | R_OK) == -1) {
            POWER_HILOGE(COMP_SVC, "system suspend config is not exist or permission denied");
            ret = false;
        }

        if (strcpy_s(targetPath, sizeof(targetPath), SYSTEM_POWER_WAKEUP_CONFIG_FILE.c_str()) != EOK) {
            POWER_HILOGE(COMP_SVC, "strcpy_s error");
            ret = false;
        }
    } else {
        if (strcpy_s(targetPath, sizeof(targetPath), VENDOR_POWER_WAKEUP_CONFIG_FILE.c_str()) != EOK) {
            POWER_HILOGE(COMP_SVC, "strcpy_s error");
            ret = false;
        }
    }
    return ret;
}

std::shared_ptr<WakeupSources> WakeupSourceParser::ParseSources(const std::string& jsonStr)
{
    std::shared_ptr<WakeupSources> parseSources = std::make_shared<WakeupSources>();
    Json::Reader reader;
    Json::Value root;
    std::string errors;
    if (!reader.parse(jsonStr.data(), jsonStr.data() + jsonStr.size(), root)) {
        POWER_HILOGE(COMP_SVC, "json parse error");
        return parseSources;
    }

    Json::Value::Members members = root.getMemberNames();
    bool matchSource = false;
    std::vector<std::string> sourceKeys = WakeupSources::getSourceKeys();
    for (auto iter = members.begin(); iter != members.end(); iter++) {
        std::string key = *iter;
        Json::Value valueObj = root[key];
        std::vector<std::string>::iterator it = sourceKeys.begin();
        for (; it != sourceKeys.end(); it++) {
            if (key == *it) {
                matchSource = true;
                sourceKeys.erase(it);
                break;
            }
        }

        if (!matchSource) {
            POWER_HILOGE(COMP_SVC, "invalid key %{public}s", key.c_str());
            return parseSources;
        }
        POWER_HILOGI(COMP_SVC, "key %{public}s", key.c_str());

        bool ret = ParseSourcesProc(parseSources,  valueObj, key );
        if(ret == false)
        {
            return parseSources;       
        }
        matchSource = false;
    }

    if (sourceKeys.size()) {
        POWER_HILOGE(COMP_SVC, "config file configuration item lose");
    }

    return parseSources;
}

bool WakeupSourceParser::ParseSourcesProc(std::shared_ptr<WakeupSources> &parseSources,  Json::Value& valueObj, std::string& key )
{
    bool ret = true;
    if (valueObj.isObject()) {
        Json::Value enableValue = valueObj[WakeupSource::ENABLE_KEY];
        Json::Value clickValue = valueObj[WakeupSource::KEYS_KEY];
        uint32_t click = 0;
        if (clickValue.isNull()) {
            click = clickValue.asUInt() <= DOUBLE_CLICK ? clickValue.asUInt() : 0;
        }
        if (enableValue.isBool()) {
            WakeupDeviceType wakeupDeviceType = WakeupSources::mapWakeupDeviceType(key, click);
            POWER_HILOGI(COMP_SVC, "key map type %{public}u", wakeupDeviceType);
            if (wakeupDeviceType == WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN) {
                ret = false;
            }
            bool enable = enableValue.asBool();
            POWER_HILOGI(COMP_SVC, "enable %{public}u", enable);
            if (enable) {
                WakeupSource wakeupSource = WakeupSource(wakeupDeviceType, enable, click);
                parseSources->PutSource(wakeupSource);
            }
        } else {
            POWER_HILOGE(COMP_SVC, "config file configuration item error");
            ret = false;
        }
    } else {
        POWER_HILOGE(COMP_SVC, "config file configuration item error");
        ret = false;
    }

    return ret;
}

} // namespace PowerMgr
} // namespace OHOS