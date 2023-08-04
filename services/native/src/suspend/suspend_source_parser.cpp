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
    POWER_HILOGI(FEATURE_SUSPEND, "ParseSources setting=%{public}d", isSettingUpdated);
    std::string configJsonStr;
    if (isSettingUpdated) {
        configJsonStr = SettingHelper::GetSettingSuspendSources();
    } else {
        std::string targetPath;
        bool ret = GetTargetPath(targetPath);
        if (ret == false) {
            return parseSources;
        }
        POWER_HILOGI(FEATURE_SUSPEND, "use targetPath=%{public}s", targetPath.c_str());
        std::ifstream inputStream(targetPath.c_str(), std::ios::in | std::ios::binary);
        std::string fileStringStr(std::istreambuf_iterator<char> {inputStream}, std::istreambuf_iterator<char> {});
        configJsonStr = fileStringStr;
    }
    parseSources = ParseSources(configJsonStr);
    if (parseSources != nullptr) {
        SettingHelper::SetSettingSuspendSources(configJsonStr);
    }
    return parseSources;
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
    Json::Reader reader;
    Json::Value root;
    std::string errors;
    if (!reader.parse(jsonStr.data(), jsonStr.data() + jsonStr.size(), root)) {
        POWER_HILOGE(FEATURE_SUSPEND, "json parse error");
        return parseSources;
    }

    Json::Value::Members members = root.getMemberNames();
    for (auto iter = members.begin(); iter != members.end(); iter++) {
        std::string key = *iter;
        Json::Value valueObj = root[key];
        POWER_HILOGI(FEATURE_SUSPEND, "key=%{public}s", key.c_str());
        bool ret = ParseSourcesProc(parseSources, valueObj, key);
        if (ret == false) {
            POWER_HILOGI(FEATURE_SUSPEND, "lost map config key");
            continue;
        }
    }
    return parseSources;
}

bool SuspendSourceParser::ParseSourcesProc(
    std::shared_ptr<SuspendSources>& parseSources, Json::Value& valueObj, std::string& key)
{
    SuspendDeviceType suspendDeviceType = SuspendSources::mapSuspendDeviceType(key);
    POWER_HILOGI(FEATURE_SUSPEND, "key map type=%{public}u", suspendDeviceType);
    if (suspendDeviceType == SuspendDeviceType::SUSPEND_DEVICE_REASON_MIN) {
        return false;
    }

    uint32_t action = 0;
    uint32_t delayMs = 0;
    if (valueObj.isObject()) {
        Json::Value actionValue = valueObj[SuspendSource::ACTION_KEY];
        Json::Value delayValue = valueObj[SuspendSource::DELAY_KEY];
        if (actionValue.isUInt() && delayValue.isUInt()) {
            action = actionValue.asUInt();
            POWER_HILOGI(FEATURE_SUSPEND, "action=%{public}u", action);
            delayMs = delayValue.asUInt();
            POWER_HILOGI(FEATURE_SUSPEND, "delayMs=%{public}u", delayMs);
            if (action >= ILLEGAL_ACTION) {
                action = 0;
            }
        }
    }

    if (action != 0) {
        SuspendSource suspendSource = SuspendSource(suspendDeviceType, action, delayMs);
        parseSources->PutSource(suspendSource);
    }

    return true;
}

} // namespace PowerMgr
} // namespace OHOS