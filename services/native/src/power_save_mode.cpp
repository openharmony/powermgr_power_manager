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

#include "power_save_mode.h"

#include "libxml/parser.h"

#include "config_policy_utils.h"
#include "power_log.h"
#include "string_ex.h"

namespace OHOS {
namespace PowerMgr {
namespace {
const std::string TAG_ROOT = "switch_policy";
const std::string POWER_MODE_CONFIG_PATH = "etc/power_config/power_mode_config.xml";
const std::string VENDOR_POWER_MODE_CONFIG_PATH = "/vendor/etc/power_config/power_mode_config.xml";
const std::string SYSTEM_POWER_MODE_CONFIG_PATH = "/system/etc/power_config/power_mode_config.xml";
constexpr uint32_t SLEEP_FILTER = SLEEP_FILTER_VALUE;
}

PowerSaveMode::PowerSaveMode()
{
    POWER_HILOGD(FEATURE_POWER_MODE, "Start to parse power_mode_config.xml");

    char buf[MAX_PATH_LEN];
    char* path = GetOneCfgFile(POWER_MODE_CONFIG_PATH.c_str(), buf, MAX_PATH_LEN);
    if (path != nullptr && *path != '\0') {
        if (!StartXMlParse(path)) {
            POWER_HILOGE(FEATURE_POWER_MODE, "policy config file power_mode_config.xml err");
        }
        return;
    }

    if (!StartXMlParse(VENDOR_POWER_MODE_CONFIG_PATH)) {
        POWER_HILOGI(FEATURE_POWER_MODE, "No vendor power_mode_config.xml, start to parse system config");
        StartXMlParse(SYSTEM_POWER_MODE_CONFIG_PATH);
    }
}

bool IsNodeLegal(const xmlNodePtr nodePtr, const std::string& tagName)
{
    return nodePtr != nullptr && nodePtr->type != XML_COMMENT_NODE && nodePtr->name != nullptr &&
    xmlStrEqual(nodePtr->name, BAD_CAST(tagName.c_str())) == 0;
}

bool PowerSaveMode::StartXMlParse(std::string path)
{
    std::unique_ptr<xmlDoc, decltype(&xmlFreeDoc)> docPtr(
        xmlReadFile(path.c_str(), nullptr, XML_PARSE_NOBLANKS), xmlFreeDoc);
    if (docPtr == nullptr) {
        POWER_HILOGE(FEATURE_POWER_MODE, "Parse failed, read file failed.");
        return false;
    }

    auto rootPtr = xmlDocGetRootElement(docPtr.get());
    if (!IsNodeLegal(rootPtr, TAG_ROOT)) {
        POWER_HILOGE(FEATURE_POWER_MODE, "Parse failed, root node is illegal.");
        return false;
    }

    for (auto nodePtr = rootPtr->xmlChildrenNode; nodePtr != nullptr; nodePtr = nodePtr->next) {
        int32_t policyId = 0;
        StrToInt(TrimStr(GetProp(nodePtr, "id")), policyId);
        POWER_HILOGD(FEATURE_POWER_MODE, "policyId: %{public}d.", policyId);
        std::list<ModePolicy> listPolicy;
        for (auto policyNodePtr = nodePtr->xmlChildrenNode;
            policyNodePtr != nullptr; policyNodePtr = policyNodePtr->next) {
            ModePolicy pmp;
            int32_t switchId;
            StrToInt(TrimStr(GetProp(policyNodePtr, "id")), switchId);
            StrToInt(TrimStr(GetProp(policyNodePtr, "recover_flag")), pmp.recover_flag);
            StrToInt(TrimStr(GetProp(policyNodePtr, "value")), pmp.value);
            pmp.id = static_cast<uint32_t>(switchId);
            listPolicy.push_back(pmp);
            POWER_HILOGD(FEATURE_POWER_MODE, "id=%{public}d, value=%{public}d, recover_flag=%{public}d", pmp.id,
                pmp.value, pmp.recover_flag);
        }
        std::pair<uint32_t, std::list<ModePolicy>> policyPair(policyId, listPolicy);
        this->policyCache_.insert(policyPair);
    }
    return true;
}

std::string PowerSaveMode::GetProp(const xmlNodePtr& nodePtr, const std::string& key)
{
    xmlChar* prop = xmlGetProp(nodePtr, BAD_CAST(key.c_str()));
    if (prop == nullptr) {
        return "";
    }
    std::string value = reinterpret_cast<char*>(prop);
    xmlFree(prop);
    return value;
}

int32_t PowerSaveMode::GetSleepTime(int32_t mode)
{
    if (this->policyCache_.size() == 0) {
        return RETURN_FLAG_FALSE;
    }
    std::list<ModePolicy>& modePolicyList = this->policyCache_[mode];
    const auto& itemPolicy = std::find_if(modePolicyList.begin(), modePolicyList.end(), [](const auto& modePolicy) {
        return modePolicy.id == SLEEP_FILTER;
    });

    return (itemPolicy != modePolicyList.end()) ? itemPolicy->value : RETURN_FLAG_FALSE;
}
} // namespace PowerMgr
} // namespace OHOS
