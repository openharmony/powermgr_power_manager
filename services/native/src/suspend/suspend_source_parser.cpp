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
        configJsonStr = SettingHelper::GetSettingSuspendSources();
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
    Json::Reader reader;
    Json::Value root;
    std::string errors;
    if (!reader.parse(jsonStr.data(), jsonStr.data() + jsonStr.size(), root)) {
        POWER_HILOGE(FEATURE_SUSPEND, "json parse error");
        parseSources->SetParseErrorFlag(true);
        return parseSources;
    }

    if (root.isNull() || !root.isObject()) {
        POWER_HILOGE(FEATURE_SUSPEND, "json root invalid[%{public}s]", jsonStr.c_str());
        parseSources->SetParseErrorFlag(true);
        return parseSources;
    }

    Json::Value::Members members = root.getMemberNames();
    for (auto iter = members.begin(); iter != members.end(); iter++) {
        std::string key = *iter;
        Json::Value valueObj = root[key];
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
    if (suspendDeviceType == SuspendDeviceType::SUSPEND_DEVICE_REASON_MIN) {
        return false;
    }

    uint32_t action = 0;
    uint32_t delayMs = 0;
    if (!valueObj.isNull() && valueObj.isObject()) {
        Json::Value actionValue = valueObj[SuspendSource::ACTION_KEY];
        Json::Value delayValue = valueObj[SuspendSource::DELAY_KEY];
        if (actionValue.isUInt() && delayValue.isUInt()) {
            action = actionValue.asUInt();
            delayMs = delayValue.asUInt();
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