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

#include "ipc_skeleton.h"
#include "ability_manager_client.h"
#include "abs_shared_result_set.h"
#include "data_ability_predicates.h"
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
Uri PowerSettingHelper::settingUri_("dataability:///com.ohos.settingsdata.DataAbility");
namespace {
static const std::string SETTING_COLUMN_KEYWORD = "KEYWORD";
static const std::string SETTING_COLUMN_VALUE = "VALUE";
}

PowerSettingHelper::~PowerSettingHelper()
{
    instance_ = nullptr;
    remoteObj_ = nullptr;
}

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
    value = static_cast<int64_t>(strtoll(valueStr.c_str(), nullptr, 10));
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
    std::string callingIdentity = IPCSkeleton::ResetCallingIdentity();
    auto uri = AssembleUri(observer->GetKey());
    auto dataAbility = AcquireDataAbility();
    if (dataAbility == nullptr) {
        IPCSkeleton::SetCallingIdentity(callingIdentity);
        return ERR_NO_INIT;
    }
    if (!dataAbility->ScheduleRegisterObserver(uri, observer)) {
        POWER_HILOGW(COMP_UTILS, "dataAbility->ScheduleRegisterObserver return false, uri=%{public}s",
            uri.ToString().c_str());
        ReleaseDataAbility(dataAbility);
        IPCSkeleton::SetCallingIdentity(callingIdentity);
        return ERR_INVALID_OPERATION;
    }
    ReleaseDataAbility(dataAbility);
    IPCSkeleton::SetCallingIdentity(callingIdentity);
    POWER_HILOGD(COMP_UTILS, "succeed to register observer of uri=%{public}s", uri.ToString().c_str());
    return ERR_OK;
}

ErrCode PowerSettingHelper::UnregisterObserver(const sptr<PowerSettingObserver>& observer)
{
    std::string callingIdentity = IPCSkeleton::ResetCallingIdentity();
    auto uri = AssembleUri(observer->GetKey());
    auto dataAbility = AcquireDataAbility();
    if (dataAbility == nullptr) {
        IPCSkeleton::SetCallingIdentity(callingIdentity);
        return ERR_NO_INIT;
    }
    if (!dataAbility->ScheduleUnregisterObserver(uri, observer)) {
        POWER_HILOGW(COMP_UTILS, "dataAbility->ScheduleUnregisterObserver return false, uri=%{public}s",
            uri.ToString().c_str());
        ReleaseDataAbility(dataAbility);
        IPCSkeleton::SetCallingIdentity(callingIdentity);
        return ERR_INVALID_OPERATION;
    }
    ReleaseDataAbility(dataAbility);
    IPCSkeleton::SetCallingIdentity(callingIdentity);
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
    auto remoteObj = sam->GetSystemAbility(systemAbilityId);
    if (remoteObj == nullptr) {
        POWER_HILOGE(COMP_UTILS, "GetSystemAbility return nullptr, systemAbilityId=%{public}d", systemAbilityId);
        return;
    }
    remoteObj_ = remoteObj;
}

ErrCode PowerSettingHelper::GetStringValue(const std::string& key, std::string& value)
{
    std::string callingIdentity = IPCSkeleton::ResetCallingIdentity();
    auto dataAbility = AcquireDataAbility();
    if (dataAbility == nullptr) {
        IPCSkeleton::SetCallingIdentity(callingIdentity);
        return ERR_NO_INIT;
    }
    std::vector<std::string> columns = {SETTING_COLUMN_VALUE};
    NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo(SETTING_COLUMN_KEYWORD, key);
    POWER_HILOGD(COMP_UTILS, "key=%{public}s", key.c_str());
    auto resultSet = dataAbility->Query(settingUri_, columns, predicates);
    ReleaseDataAbility(dataAbility);
    if (resultSet == nullptr) {
        POWER_HILOGE(COMP_UTILS, "dataAbility->Query return nullptr");
        IPCSkeleton::SetCallingIdentity(callingIdentity);
        return ERR_INVALID_OPERATION;
    }
    int32_t count = 0;
    resultSet->GetRowCount(count);
    if (count == 0) {
        POWER_HILOGW(COMP_UTILS, "not found value, key=%{public}s, count=%{public}d", key.c_str(), count);
        IPCSkeleton::SetCallingIdentity(callingIdentity);
        return ERR_NAME_NOT_FOUND;
    }
    const int32_t INDEX = 0;
    resultSet->GoToRow(INDEX);
    int32_t ret = resultSet->GetString(INDEX, value);
    if (ret != NativeRdb::E_OK) {
        POWER_HILOGW(COMP_UTILS, "resultSet->GetString return not ok, ret=%{public}d", ret);
        IPCSkeleton::SetCallingIdentity(callingIdentity);
        return ERR_INVALID_VALUE;
    }
    resultSet->Close();
    IPCSkeleton::SetCallingIdentity(callingIdentity);
    return ERR_OK;
}

ErrCode PowerSettingHelper::PutStringValue(const std::string& key, const std::string& value)
{
    std::string callingIdentity = IPCSkeleton::ResetCallingIdentity();
    auto dataAbility = AcquireDataAbility();
    if (dataAbility == nullptr) {
        IPCSkeleton::SetCallingIdentity(callingIdentity);
        return ERR_NO_INIT;
    }
    POWER_HILOGD(COMP_UTILS, "key=%{public}s, value=%{public}s", key.c_str(), value.c_str());
    NativeRdb::ValuesBucket bucket;
    bucket.PutString(SETTING_COLUMN_KEYWORD, key);
    bucket.PutString(SETTING_COLUMN_VALUE, value);
    NativeRdb::DataAbilityPredicates predicates;
    predicates.EqualTo(SETTING_COLUMN_KEYWORD, key);
    if (dataAbility->Update(settingUri_, bucket, predicates) <= 0) {
        POWER_HILOGD(COMP_UTILS, "no data exist, insert one row");
        dataAbility->Insert(settingUri_, bucket);
    }
    dataAbility->ScheduleNotifyChange(AssembleUri(key));
    ReleaseDataAbility(dataAbility);
    IPCSkeleton::SetCallingIdentity(callingIdentity);
    return ERR_OK;
}

sptr<AAFwk::IAbilityScheduler> PowerSettingHelper::AcquireDataAbility()
{
    auto abilityManagerClient = AAFwk::AbilityManagerClient::GetInstance();
    auto dataAbility = abilityManagerClient->AcquireDataAbility(settingUri_, false, remoteObj_);
    if (dataAbility == nullptr) {
        POWER_HILOGW(COMP_UTILS, "dataAbility is nullptr, uri=%{public}s, remoteObj_=%{public}p",
            settingUri_.ToString().c_str(), remoteObj_.GetRefPtr());
        return nullptr;
    }
    return dataAbility;
}

bool PowerSettingHelper::ReleaseDataAbility(sptr<AAFwk::IAbilityScheduler>& dataAbility)
{
    auto abilityManagerClient = AAFwk::AbilityManagerClient::GetInstance();
    if (abilityManagerClient->ReleaseDataAbility(dataAbility, remoteObj_) != ERR_OK) {
        POWER_HILOGW(COMP_UTILS, "release dataAbility fail, remoteObj_=%{public}p", remoteObj_.GetRefPtr());
        return false;
    }
    return true;
}

Uri PowerSettingHelper::AssembleUri(const std::string& key)
{
    Uri uri(settingUri_.ToString() + "/" + key);
    return uri;
}
} // OHOS
} // PowerMgr