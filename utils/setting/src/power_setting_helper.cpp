/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "power_setting_helper.h"

#include "dataobs_mgr_client.h"
#include "abs_shared_result_set.h"
#include "data_ability_predicates.h"
#include "data_ability_helper.h"
#include "iservice_registry.h"
#include "result_set.h"
#include "values_bucket.h"
#include "rdb_errno.h"
#include "power_log.h"

namespace OHOS {
namespace PowerMgr {
PowerSettingHelper* PowerSettingHelper::instance_;
std::mutex PowerSettingHelper::mutex_;
sptr<IRemoteObject> PowerSettingHelper::remoteObj_;

PowerSettingHelper& PowerSettingHelper::GetInstance(int32_t systemAbilityId)
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (instance_ == nullptr) {
            instance_ = new PowerSettingHelper();
            Initialize(systemAbilityId);
        }
    }
    return *instance_;
}

ErrCode PowerSettingHelper::GetIntValue(const std::string& key, int32_t& value)
{
    int64_t valueLong;
    ErrCode ret = GetLongValue(key, valueLong);
    if (ret != ERR_OK) {
        return ret;
    }
    value = static_cast<int32_t>(valueLong);
    return ERR_OK;
}

ErrCode PowerSettingHelper::GetLongValue(const std::string& key, int64_t& value)
{
    std::string valueStr;
    ErrCode ret = GetStringValue(key, valueStr);
    if (ret != ERR_OK) {
        return ret;
    }
    value = atol(valueStr.c_str());
    return ERR_OK;
}

ErrCode PowerSettingHelper::GetBoolValue(const std::string& key, bool& value)
{
    int32_t valueInt;
    ErrCode ret = GetIntValue(key, valueInt);
    if (ret != ERR_OK) {
        return ret;
    }
    value = static_cast<bool>(valueInt);
    return ERR_OK;
}

ErrCode PowerSettingHelper::PutIntValue(const std::string& key, int32_t value)
{
    return PutStringValue(key, std::to_string(value));
}

ErrCode PowerSettingHelper::PutLongValue(const std::string& key, int64_t value)
{
    return PutStringValue(key, std::to_string(value));
}

ErrCode PowerSettingHelper::PutBoolValue(const std::string& key, bool value)
{
    return PutStringValue(key, std::to_string(value));
}

sptr<PowerSettingObserver> PowerSettingHelper::CreateObserver(const std::string& key,
    PowerSettingObserver::UpdateFunc& func)
{
    sptr<PowerSettingObserver> observer = new PowerSettingObserver();
    observer->SetKey(key);
    observer->SetUpdateFunc(func);
    return observer;
}

ErrCode PowerSettingHelper::RegisterObserver(const sptr<PowerSettingObserver>& observer)
{
    auto uri = AssembleUri(observer->GetKey());
    auto dataAbilityHelper = AppExecFwk::DataAbilityHelper::Creator(remoteObj_);
    if (dataAbilityHelper == nullptr) {
        POWER_HILOGE(COMP_UTILS, "DataAbilityHelper::Creator return nullptr, remoteObj_=%{public}p",
            remoteObj_.GetRefPtr());
        return ERR_NO_INIT;
    }
    dataAbilityHelper->RegisterObserver(uri, observer);
    dataAbilityHelper->Release();
    POWER_HILOGD(COMP_UTILS, "succeed to register observer of uri=%{public}s", uri.ToString().c_str());
    return ERR_OK;
}

ErrCode PowerSettingHelper::UnregisterObserver(const sptr<PowerSettingObserver>& observer)
{
    auto uri = AssembleUri(observer->GetKey());
    auto dataAbilityHelper = AppExecFwk::DataAbilityHelper::Creator(remoteObj_);
    if (dataAbilityHelper == nullptr) {
        POWER_HILOGE(COMP_UTILS, "DataAbilityHelper::Creator return nullptr, remoteObj_=%{public}p",
            remoteObj_.GetRefPtr());
        return ERR_NO_INIT;
    }
    dataAbilityHelper->UnregisterObserver(uri, observer);
    dataAbilityHelper->Release();
    POWER_HILOGD(COMP_UTILS, "succeed to unregister observer of uri=%{public}s", uri.ToString().c_str());
    return ERR_OK;
}

void PowerSettingHelper::Initialize(int32_t systemAbilityId)
{
    auto sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        POWER_HILOGE(COMP_UTILS, "GetSystemAbilityManager return nullptr");
        return;
    }
    auto remoteOjb = sam->GetSystemAbility(systemAbilityId);
    if (remoteOjb == nullptr) {
        POWER_HILOGE(COMP_UTILS, "GetSystemAbility return nullptr, systemAbilityId=%{public}d", systemAbilityId);
        return;
    }
    remoteObj_ = remoteOjb;
    POWER_HILOGW(COMP_UTILS, "initialized remoteObj_=%{public}p", remoteObj_.GetRefPtr());
}

ErrCode PowerSettingHelper::GetStringValue(const std::string& key, std::string& value)
{
    auto dataAbilityHelper = AppExecFwk::DataAbilityHelper::Creator(remoteObj_);
    if (dataAbilityHelper == nullptr) {
        POWER_HILOGE(COMP_UTILS, "DataAbilityHelper::Creator return nullptr, remoteObj_=%{public}p",
            remoteObj_.GetRefPtr());
        return ERR_NO_INIT;
    }
    Uri uri(SETTING_URI);
    std::vector<std::string> columns = {SETTING_COLUMN_VALUE};
    NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo(SETTING_COLUMN_KEYWORD, key);
    POWER_HILOGD(COMP_UTILS, "key=%{public}s", key.c_str());
    auto resultSet = dataAbilityHelper->Query(uri, columns, predicates);
    dataAbilityHelper->Release();
    if (resultSet == nullptr) {
        POWER_HILOGE(COMP_UTILS, "dataAbility->Query return nullptr");
        return ERR_INVALID_OPERATION;
    }
    int32_t count = 0;
    resultSet->GetRowCount(count);
    if (count == 0) {
        POWER_HILOGW(COMP_UTILS, "not found value, key=%{public}s, count=%{public}d", key.c_str(), count);
        return ERR_NAME_NOT_FOUND;
    }
    const int32_t INDEX = 0;
    resultSet->GoToRow(INDEX);
    int32_t ret = resultSet->GetString(INDEX, value);
    if (ret != NativeRdb::E_OK) {
        POWER_HILOGW(COMP_UTILS, "resultSet->GetString return not ok, ret=%{public}d", ret);
        return ERR_INVALID_VALUE;
    }
    resultSet->Close();
    return ERR_OK;
}

ErrCode PowerSettingHelper::PutStringValue(const std::string& key, const std::string& value)
{
    auto dataAbilityHelper = AppExecFwk::DataAbilityHelper::Creator(remoteObj_);
    if (dataAbilityHelper == nullptr) {
        POWER_HILOGE(COMP_UTILS, "DataAbilityHelper::Creator return nullptr, remoteObj_=%{public}p",
            remoteObj_.GetRefPtr());
        return ERR_NO_INIT;
    }
    POWER_HILOGD(COMP_UTILS, "key=%{public}s, value=%{public}s", key.c_str(), value.c_str());
    Uri uri(SETTING_URI);
    NativeRdb::ValuesBucket bucket;
    bucket.PutString(SETTING_COLUMN_KEYWORD, key);
    bucket.PutString(SETTING_COLUMN_VALUE, value);
    NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo(SETTING_COLUMN_KEYWORD, key);
    if (dataAbilityHelper->Update(uri, bucket, predicates) <= 0) {
        POWER_HILOGD(COMP_UTILS, "no data exist, insert one row");
        dataAbilityHelper->Insert(uri, bucket);
    }
    dataAbilityHelper->NotifyChange(AssembleUri(key));
    dataAbilityHelper->Release();
    return ERR_OK;
}

Uri PowerSettingHelper::AssembleUri(const std::string& key)
{
    Uri uri(SETTING_URI + "/" + key);
    return uri;
}
} // OHOS
} // PowerMgr