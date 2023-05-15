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
#include "suspend_source_parser.h"

#include <fstream>
#include <securec.h>

#include "config_policy_utils.h"
#include "power_log.h"
#include "setting_helper.h"
#include "json/reader.h"
#include "json/value.h"

namespace OHOS {
namespace PowerMgr {

namespace {
static const std::string POWER_SUSPEND_CONFIG_FILE = "etc/power_config/power_suspend.json";
static const std::string VENDOR_POWER_SUSPEND_CONFIG_FILE = "/vendor/etc/power_config/power_suspend.json";
static const std::string SYSTEM_POWER_SUSPEND_CONFIG_FILE = "/system/etc/power_config/power_suspend.json";
static const uint32_t ILLEGAL_ACTION = static_cast<uint32_t>(SuspendAction::ACTION_INVALID);
} // namespace

std::shared_ptr<SuspendSources> SuspendSourceParser::ParseSources()
{
    std::shared_ptr<SuspendSources> parseSources;
    bool isSettingUpdated = SettingHelper::IsSuspendSourcesSettingValid();
    POWER_HILOGI(COMP_SVC, "ParseSources setting %{public}d", isSettingUpdated);
    std::string configJsonStr;
    if (isSettingUpdated) {
        configJsonStr = SettingHelper::GetSettingSuspendSources();
    } else {
        char buf[MAX_PATH_LEN];
        char targetPath[MAX_PATH_LEN] = {0};
        char* path = GetOneCfgFile(POWER_SUSPEND_CONFIG_FILE.c_str(), buf, MAX_PATH_LEN);
        if (path != nullptr && *path != '\0') {
            if (strcpy_s(targetPath, sizeof(targetPath), path) != EOK) {
                POWER_HILOGE(COMP_SVC, "strcpy_s error");
                return parseSources;
            }
        } else {
            bool ret = GetTargetPath(targetPath);
            if (ret == false) {
                return parseSources;
            }
        }
        POWER_HILOGI(COMP_SVC, "use targetPath %{public}s", targetPath);
        std::ifstream inputStream(targetPath, std::ios::in | std::ios::binary);
        std::string fileStringStr(std::istreambuf_iterator<char> {inputStream}, std::istreambuf_iterator<char> {});
        configJsonStr = fileStringStr;
    }
    parseSources = ParseSources(configJsonStr);
    if (parseSources != nullptr) {
        SettingHelper::SetSettingSuspendSources(configJsonStr);
    }
    return parseSources;
}

bool SuspendSourceParser::GetTargetPath(char* targetPath)
{
    bool ret = true;
    if (access(VENDOR_POWER_SUSPEND_CONFIG_FILE.c_str(), F_OK | R_OK) == -1) {
        POWER_HILOGE(COMP_SVC, "vendor suspend config is not exist or permission denied");
        if (access(SYSTEM_POWER_SUSPEND_CONFIG_FILE.c_str(), F_OK | R_OK) == -1) {
            POWER_HILOGE(COMP_SVC, "system suspend config is not exist or permission denied");
            ret = false;
        }

        if (strcpy_s(targetPath, sizeof(targetPath), SYSTEM_POWER_SUSPEND_CONFIG_FILE.c_str()) != EOK) {
            POWER_HILOGE(COMP_SVC, "strcpy_s error");
            ret = false;
        }
    } else {
        if (strcpy_s(targetPath, sizeof(targetPath), VENDOR_POWER_SUSPEND_CONFIG_FILE.c_str()) != EOK) {
            POWER_HILOGE(COMP_SVC, "strcpy_s error");
            ret = false;
        }
    }
    return ret;
}

std::shared_ptr<SuspendSources> SuspendSourceParser::ParseSources(const std::string& jsonStr)
{
    std::shared_ptr<SuspendSources> parseSources = std::make_shared<SuspendSources>();
    Json::Reader reader;
    Json::Value root;
    std::string errors;
    if (!reader.parse(jsonStr.data(), jsonStr.data() + jsonStr.size(), root)) {
        POWER_HILOGE(COMP_SVC, "json parse error");
        return parseSources;
    }

    Json::Value::Members members = root.getMemberNames();
    bool matchSource = false;
    std::vector<std::string> sourceKeys = SuspendSources::getSourceKeys();
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

        bool needContinue = false;
        bool ret = ParseSourcesProc(parseSources, valueObj, matchSource, needContinue, key);
        if (ret == false) {
            return parseSources;
        }
        if (needContinue == true) {
            continue;
        }
        matchSource = false;
    }

    if (sourceKeys.size()) {
        POWER_HILOGE(COMP_SVC, "config file configuration item lose");
    }

    return parseSources;
}

bool SuspendSourceParser::ParseSourcesProc(std::shared_ptr<SuspendSources>& parseSources, Json::Value& valueObj,
    bool& matchSource, bool& needContinue, std::string& key)
{
    bool ret = true;
    if (valueObj.isObject()) {
        Json::Value actionValue = valueObj[SuspendSource::ACTION_KEY];
        Json::Value delayValue = valueObj[SuspendSource::DELAY_KEY];
        if (actionValue.isUInt() && delayValue.isUInt()) {
            SuspendDeviceType suspendDeviceType = SuspendSources::mapSuspendDeviceType(key);
            POWER_HILOGI(COMP_SVC, "key map type %{public}u", suspendDeviceType);
            if (suspendDeviceType == SuspendDeviceType::SUSPEND_DEVICE_REASON_MIN) {
                ret = false;
            }
            uint32_t action = actionValue.asUInt();
            POWER_HILOGI(COMP_SVC, "action %{public}u", action);
            if (action >= ILLEGAL_ACTION) {
                ret = false;
            }
            if (action == 0) {
                matchSource = false;
                needContinue = true;
            }
            uint32_t delayMs = delayValue.asUInt();
            POWER_HILOGI(COMP_SVC, "delayMs %{public}u", delayMs);
            SuspendSource suspendSource = SuspendSource(suspendDeviceType, action, delayMs);
            parseSources->PutSource(suspendSource);
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