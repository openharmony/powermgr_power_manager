/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include "libxml/tree.h"
#include "hilog_wrapper.h"

#include <refbase.h>

namespace OHOS {
namespace PowerMgr {
constexpr auto TAG_ROOT_ = "switch_policy";
constexpr uint32_t SLEEP_FILTER = SLEEP_FILTER_VALUE;

PowerSaveMode::PowerSaveMode()
{
    POWER_HILOGD(MODULE_SERVICE, "Start powersave.xml parse");
    StartXMlParse("/system/etc/power_config/power_mode_config.xml");
}

bool IsNodeLegal(const xmlNodePtr nodePtr, const char* tagName)
{
    return nodePtr != nullptr && nodePtr->type != XML_COMMENT_NODE && nodePtr->name != nullptr &&
    xmlStrEqual(nodePtr->name, BAD_CAST(tagName)) == 0;
}

bool PowerSaveMode::StartXMlParse(std::string path)
{
    std::unique_ptr<xmlDoc, decltype(&xmlFreeDoc)> docPtr(
        xmlReadFile(path.c_str(), nullptr, XML_PARSE_NOBLANKS), xmlFreeDoc);
    if (docPtr == nullptr) {
        POWER_HILOGE(MODULE_SERVICE, "Parse failed, read file failed.");
        return false;
    }

    auto rootPtr = xmlDocGetRootElement(docPtr.get());
    if (!IsNodeLegal(rootPtr, TAG_ROOT_)) {
        POWER_HILOGE(MODULE_SERVICE, "Parse failed, root node is illegal.");
        return false;
    }

    for (auto nodePtr = rootPtr->xmlChildrenNode; nodePtr != nullptr; nodePtr = nodePtr->next) {
        int32_t policyId = atoi((char *)xmlGetProp(nodePtr, BAD_CAST("id")));
        std::list<ModePolicy> listPolicy;
        for (auto policyNodePtr = nodePtr->xmlChildrenNode;
            policyNodePtr != nullptr; policyNodePtr = policyNodePtr->next) {
            ModePolicy pmp;
            pmp.id = atoi((char *)xmlGetProp(policyNodePtr, BAD_CAST("id")));
            pmp.recover_flag = atoi((char *)xmlGetProp(policyNodePtr, BAD_CAST("recover_flag")));
            pmp.value = atoi((char *)xmlGetProp(policyNodePtr, BAD_CAST("value")));
            listPolicy.push_back(pmp);
            POWER_HILOGE(MODULE_SERVICE, "id=%{public}d", pmp.id);
            POWER_HILOGE(MODULE_SERVICE, "value=%{public}d", pmp.value);
            POWER_HILOGE(MODULE_SERVICE, "recover_flag=%{public}d", pmp.recover_flag);
        }
        std::pair<int32_t, std::list<ModePolicy>> policyPair(policyId, listPolicy);
        this->policyCache_.insert(policyPair);
        POWER_HILOGI(MODULE_SERVICE, "policyId = %{public}d.", policyId);
    }
    return true;
}

bool PowerSaveMode::GetValuePolicy(std::list<ModePolicy> &openPolicy, int32_t mode)
{
    bool result = GetFilterPolicy(openPolicy, mode, ValueProp::value);
    return result;
}

bool PowerSaveMode::GetRecoverPolicy(std::list<ModePolicy> &recoverPolicy, int32_t mode)
{
    bool result = GetFilterPolicy(recoverPolicy, mode, ValueProp::recover);
    return result;
}

bool PowerSaveMode::GetFilterPolicy(std::list<ModePolicy> &policyList, int32_t mode, int32_t value)
{
    if (this->policyCache_.size() == 0) {
        return false;
    }
    for (ModePolicy modePolicy :this->policyCache_[mode]) {
        policyList.push_back(modePolicy);
    }
    if (ValueProp::recover == value) {
        policyList.remove_if([&](ModePolicy mp) {return mp.recover_flag != ValueProp::recover;});
    }
    return true;
}

int32_t PowerSaveMode::GetSleepTime(int32_t mode)
{
    if (this->policyCache_.size() == 0) {
        return RETURN_FLAG_FALSE;
    }
    std::list<ModePolicy> modePolicyList = this->policyCache_[mode];
    for (auto modePolicy:modePolicyList) {
        if (modePolicy.id == SLEEP_FILTER) {
            return modePolicy.value;
        }
    }
    return RETURN_FLAG_FALSE;
}
} // namespace PowerMgr
} // namespace OHOS
