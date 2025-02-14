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

#include "setting_provider.h"
#include <thread>
#include <regex>
#include "datashare_predicates.h"
#include "datashare_result_set.h"
#include "datashare_values_bucket.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "os_account_manager.h"
#include "power_log.h"
#include "rdb_errno.h"
#include "result_set.h"
#include "uri.h"

namespace OHOS {
namespace PowerMgr {
std::atomic<SettingProvider*> SettingProvider::instance_ {nullptr};
std::mutex SettingProvider::settingMutex_;
sptr<IRemoteObject> SettingProvider::remoteObj_;
const int32_t INITIAL_USER_ID = 100;
int32_t SettingProvider::currentUserId_ = INITIAL_USER_ID;
namespace {
const std::string SETTING_COLUMN_KEYWORD = "KEYWORD";
const std::string SETTING_COLUMN_VALUE = "VALUE";
const std::string SETTING_URI_PROXY = "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true";
const std::string SETTING_URI_PROXY_USER = "datashare:///com.ohos.settingsdata/entry/settingsdata/";
const std::string SETTING_URI_PROXY_USER_ADAPT = "USER_SETTINGSDATA_SECURE_##USERID##?Proxy=true";
constexpr const char *USERID_REPLACE = "##USERID##";
constexpr const char *SETTINGS_DATA_EXT_URI = "datashare:///com.ohos.settingsdata.DataAbility";
} // namespace

SettingProvider::~SettingProvider()
{
    instance_ = nullptr;
    remoteObj_ = nullptr;
}

SettingProvider& SettingProvider::GetInstance(int32_t systemAbilityId)
{
    SettingProvider* tmp = instance_.load(std::memory_order_acquire);
    if (tmp == nullptr) {
        std::lock_guard<std::mutex> lock(settingMutex_);
        tmp = instance_.load(std::memory_order_relaxed);
        if (tmp == nullptr) {
            Initialize(systemAbilityId);
            tmp = new SettingProvider();
            instance_.store(tmp, std::memory_order_release);
        }
    }
    return *instance_;
}

void SettingProvider::CopyDataForUpdateScene()
{
    if (IsNeedDataMigrationCopy()) {
        DataMigrationCopy();
    }
}

ErrCode SettingProvider::GetIntValue(const std::string& key, int32_t& value)
{
    int64_t valueLong;
    ErrCode ret = GetLongValue(key, valueLong);
    if (ret != ERR_OK) {
        return ret;
    }
    value = static_cast<int32_t>(valueLong);
    return ERR_OK;
}

ErrCode SettingProvider::GetLongValue(const std::string& key, int64_t& value)
{
    std::string valueStr;
    ErrCode ret = GetStringValue(key, valueStr);
    if (ret != ERR_OK) {
        return ret;
    }
    char* endptr = nullptr;
    int64_t result = static_cast<int64_t>(strtoll(valueStr.c_str(), &endptr, 10));
    if (endptr == nullptr || *endptr != '\0') {
        POWER_HILOGE(COMP_UTILS, "GetLongValue error! key:%{public}s, value:%{public}s",
            key.c_str(), valueStr.c_str());
        return ERR_INVALID_VALUE;
    }
    value = result;
    return ERR_OK;
}

ErrCode SettingProvider::GetBoolValue(const std::string& key, bool& value)
{
    std::string valueStr;
    ErrCode ret = GetStringValue(key, valueStr);
    if (ret != ERR_OK) {
        return ret;
    }
    value = (valueStr == "true");
    return ERR_OK;
}

ErrCode SettingProvider::PutIntValue(const std::string& key, int32_t value, bool needNotify)
{
    return PutStringValue(key, std::to_string(value), needNotify);
}

ErrCode SettingProvider::PutLongValue(const std::string& key, int64_t value, bool needNotify)
{
    return PutStringValue(key, std::to_string(value), needNotify);
}

ErrCode SettingProvider::PutBoolValue(const std::string& key, bool value, bool needNotify)
{
    std::string valueStr = value ? "true" : "false";
    return PutStringValue(key, valueStr, needNotify);
}

bool SettingProvider::IsValidKey(const std::string& key)
{
    std::string value;
    ErrCode ret = GetStringValue(key, value);
    if (!value.empty()) {
        POWER_HILOGI(COMP_UTILS, "the getValue is:%{public}s", value.c_str());
    }
    POWER_HILOGI(COMP_UTILS, "the getRet is:%{public}u", ret);
    return (ret != ERR_NAME_NOT_FOUND) && (!value.empty());
}

bool SettingProvider::IsValidKeyGlobal(const std::string& key)
{
    std::string value;
    ErrCode ret = GetStringValueGlobal(key, value);
    if (!value.empty()) {
        POWER_HILOGI(COMP_UTILS, "the getValueGlobal is:%{public}s", value.c_str());
    }
    POWER_HILOGI(COMP_UTILS, "the getRetGlobal is:%{public}u", ret);
    return (ret != ERR_NAME_NOT_FOUND) && (!value.empty());
}

sptr<SettingObserver> SettingProvider::CreateObserver(const std::string& key, const SettingObserver::UpdateFunc& func)
{
    sptr<SettingObserver> observer = new SettingObserver();
    observer->SetKey(key);
    observer->SetUpdateFunc(func);
    return observer;
}

void SettingProvider::ExecRegisterCb(const sptr<SettingObserver>& observer)
{
    if (observer == nullptr) {
        POWER_HILOGE(COMP_UTILS, "observer is nullptr");
        return;
    }
    observer->OnChange();
}

ErrCode SettingProvider::RegisterObserver(const sptr<SettingObserver>& observer)
{
    std::string callingIdentity = IPCSkeleton::ResetCallingIdentity();
    auto uri = AssembleUri(observer->GetKey());
    auto helper = CreateDataShareHelper(observer->GetKey());
    if (helper == nullptr) {
        IPCSkeleton::SetCallingIdentity(callingIdentity);
        return ERR_NO_INIT;
    }
    helper->RegisterObserver(uri, observer);
    helper->NotifyChange(uri);
    std::thread execCb([this, observer] { this->ExecRegisterCb(observer); });
    execCb.detach();
    ReleaseDataShareHelper(helper);
    IPCSkeleton::SetCallingIdentity(callingIdentity);
    POWER_HILOGD(COMP_UTILS, "succeed to register observer of uri=%{public}s", uri.ToString().c_str());
    return ERR_OK;
}

ErrCode SettingProvider::UnregisterObserver(const sptr<SettingObserver>& observer)
{
    std::string callingIdentity = IPCSkeleton::ResetCallingIdentity();
    auto uri = AssembleUri(observer->GetKey());
    auto helper = CreateDataShareHelper(observer->GetKey());
    if (helper == nullptr) {
        IPCSkeleton::SetCallingIdentity(callingIdentity);
        return ERR_NO_INIT;
    }
    helper->UnregisterObserver(uri, observer);
    ReleaseDataShareHelper(helper);
    IPCSkeleton::SetCallingIdentity(callingIdentity);
    POWER_HILOGD(COMP_UTILS, "succeed to unregister observer of uri=%{public}s", uri.ToString().c_str());
    return ERR_OK;
}

void SettingProvider::Initialize(int32_t systemAbilityId)
{
    auto sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        POWER_HILOGE(COMP_UTILS, "GetSystemAbilityManager return nullptr");
        return;
    }
    auto remoteObj = sam->GetSystemAbility(systemAbilityId);
    if (remoteObj == nullptr) {
        POWER_HILOGE(COMP_UTILS, "GetSystemAbility return nullptr, systemAbilityId=%{public}d", systemAbilityId);
        remoteObj = sptr<IPCObjectStub>::MakeSptr(u"ohos.powermgr.utils.setting_provider");
    }
    remoteObj_ = std::move(remoteObj);
}

ErrCode SettingProvider::GetStringValue(const std::string& key, std::string& value)
{
    std::string callingIdentity = IPCSkeleton::ResetCallingIdentity();
    auto helper = CreateDataShareHelper(key);
    if (helper == nullptr) {
        IPCSkeleton::SetCallingIdentity(callingIdentity);
        return ERR_NO_INIT;
    }
    std::vector<std::string> columns = {SETTING_COLUMN_VALUE};
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(SETTING_COLUMN_KEYWORD, key);
    Uri uri(AssembleUri(key));
    auto resultSet = helper->Query(uri, predicates, columns);
    ReleaseDataShareHelper(helper);
    if (resultSet == nullptr) {
        POWER_HILOGE(COMP_UTILS, "helper->Query return nullptr");
        IPCSkeleton::SetCallingIdentity(callingIdentity);
        return ERR_INVALID_OPERATION;
    }
    int32_t count;
    resultSet->GetRowCount(count);
    if (count == 0) {
        POWER_HILOGW(COMP_UTILS, "not found value, key=%{public}s, count=%{public}d", key.c_str(), count);
        IPCSkeleton::SetCallingIdentity(callingIdentity);
        resultSet->Close();
        return ERR_NAME_NOT_FOUND;
    }
    const int32_t INDEX = 0;
    resultSet->GoToRow(INDEX);
    int32_t ret = resultSet->GetString(INDEX, value);
    if (ret != NativeRdb::E_OK) {
        POWER_HILOGW(COMP_UTILS, "resultSet->GetString return not ok, ret=%{public}d", ret);
        IPCSkeleton::SetCallingIdentity(callingIdentity);
        resultSet->Close();
        return ERR_INVALID_VALUE;
    }
    resultSet->Close();
    IPCSkeleton::SetCallingIdentity(callingIdentity);
    return ERR_OK;
}

ErrCode SettingProvider::PutStringValue(const std::string& key, const std::string& value, bool needNotify)
{
    std::string callingIdentity = IPCSkeleton::ResetCallingIdentity();
    auto helper = CreateDataShareHelper(key);
    if (helper == nullptr) {
        IPCSkeleton::SetCallingIdentity(callingIdentity);
        return ERR_NO_INIT;
    }
    DataShare::DataShareValueObject keyObj(key);
    DataShare::DataShareValueObject valueObj(value);
    DataShare::DataShareValuesBucket bucket;
    bucket.Put(SETTING_COLUMN_KEYWORD, keyObj);
    bucket.Put(SETTING_COLUMN_VALUE, valueObj);
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(SETTING_COLUMN_KEYWORD, key);
    Uri uri(AssembleUri(key));
    if (helper->Update(uri, predicates, bucket) <= 0) {
        POWER_HILOGD(COMP_UTILS, "no data exist, insert one row");
        helper->Insert(uri, bucket);
    }
    if (needNotify) {
        helper->NotifyChange(AssembleUri(key));
    }
    ReleaseDataShareHelper(helper);
    IPCSkeleton::SetCallingIdentity(callingIdentity);
    return ERR_OK;
}

std::shared_ptr<DataShare::DataShareHelper> SettingProvider::CreateDataShareHelper(const std::string& key)
{
    std::lock_guard<std::mutex> lock(settingMutex_);
    std::string uriProxyStr;
    if (IsNeedMultiUser(key)) {
        uriProxyStr = SETTING_URI_PROXY_USER + "USER_SETTINGSDATA_SECURE_" +
            std::to_string(currentUserId_) + "?Proxy=true";
        POWER_HILOGI(COMP_UTILS, "the uriProxyStr is %{public}s", uriProxyStr.c_str());
    } else {
        uriProxyStr = SETTING_URI_PROXY;
    }
    auto helper = DataShare::DataShareHelper::Creator(remoteObj_, uriProxyStr, SETTINGS_DATA_EXT_URI);
    if (helper == nullptr) {
        POWER_HILOGW(COMP_UTILS, "helper is nullptr, uri=%{public}s", uriProxyStr.c_str());
        return nullptr;
    }
    return helper;
}

bool SettingProvider::ReleaseDataShareHelper(std::shared_ptr<DataShare::DataShareHelper>& helper)
{
    if (!helper->Release()) {
        POWER_HILOGW(COMP_UTILS, "release helper fail");
        return false;
    }
    return true;
}

Uri SettingProvider::AssembleUri(const std::string& key)
{
    std::lock_guard<std::mutex> lock(settingMutex_);
    if (IsNeedMultiUser(key)) {
        std::string userSetting = ReplaceUserIdForUri(currentUserId_);
        std::string specialUri = SETTING_URI_PROXY_USER + userSetting + "&key=" + key;
        POWER_HILOGI(COMP_UTILS, "the non-global uri is %{public}s", specialUri.c_str());
        Uri uri(specialUri);
        return uri;
    }
    Uri uri(SETTING_URI_PROXY + "&key=" + key);
    return uri;
}

bool SettingProvider::IsNeedDataMigrationCopy()
{
    bool isNeedMigrationCopy = false;
    do {
        if (!(IsValidKeyGlobal(SETTING_POWER_WAKEUP_PICKUP_KEY) && !IsValidKey(SETTING_POWER_WAKEUP_PICKUP_KEY))) {
            break;
        }
        if (!(IsValidKeyGlobal(SETTING_POWER_WAKEUP_DOUBLE_KEY) && !IsValidKey(SETTING_POWER_WAKEUP_DOUBLE_KEY))) {
            break;
        }
        if (!(IsValidKeyGlobal(SETTING_POWER_WAKEUP_SOURCES_KEY) && !IsValidKey(SETTING_POWER_WAKEUP_SOURCES_KEY))) {
            break;
        }
        isNeedMigrationCopy = true;
    } while (0);
    POWER_HILOGI(COMP_UTILS, "IsNeedDataMigrationCopy:(%{public}d)", isNeedMigrationCopy);
    return isNeedMigrationCopy;
}

void SettingProvider::DataMigrationCopy()
{
    std::string value;
    if (GetStringValueGlobal(SETTING_POWER_WAKEUP_DOUBLE_KEY, value) == ERR_OK) {
        PutStringValue(SETTING_POWER_WAKEUP_DOUBLE_KEY, value);
    }
    if (GetStringValueGlobal(SETTING_POWER_WAKEUP_PICKUP_KEY, value) == ERR_OK) {
        PutStringValue(SETTING_POWER_WAKEUP_PICKUP_KEY, value);
    }
    if (GetStringValueGlobal(SETTING_POWER_WAKEUP_SOURCES_KEY, value) == ERR_OK) {
        PutStringValue(SETTING_POWER_WAKEUP_SOURCES_KEY, value);
    }
}

ErrCode SettingProvider::GetStringValueGlobal(const std::string& key, std::string& value)
{
    std::string callingIdentity = IPCSkeleton::ResetCallingIdentity();
    auto helper = DataShare::DataShareHelper::Creator(remoteObj_, SETTING_URI_PROXY, SETTINGS_DATA_EXT_URI);
    if (helper == nullptr) {
        IPCSkeleton::SetCallingIdentity(callingIdentity);
        return ERR_NO_INIT;
    }
    std::vector<std::string> columns = {SETTING_COLUMN_VALUE};
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(SETTING_COLUMN_KEYWORD, key);
    Uri uri(SETTING_URI_PROXY + "&key=" + key);
    auto resultSet = helper->Query(uri, predicates, columns);
    ReleaseDataShareHelper(helper);
    if (resultSet == nullptr) {
        POWER_HILOGE(COMP_UTILS, "helper->Query return nullptr");
        IPCSkeleton::SetCallingIdentity(callingIdentity);
        return ERR_INVALID_OPERATION;
    }
    int32_t count;
    resultSet->GetRowCount(count);
    if (count == 0) {
        POWER_HILOGW(COMP_UTILS, "not found value, key=%{public}s, count=%{public}d", key.c_str(), count);
        IPCSkeleton::SetCallingIdentity(callingIdentity);
        resultSet->Close();
        return ERR_NAME_NOT_FOUND;
    }
    const int32_t INDEX = 0;
    resultSet->GoToRow(INDEX);
    int32_t ret = resultSet->GetString(INDEX, value);
    if (ret != NativeRdb::E_OK) {
        POWER_HILOGW(COMP_UTILS, "resultSet->GetString return not ok, ret=%{public}d", ret);
        IPCSkeleton::SetCallingIdentity(callingIdentity);
        resultSet->Close();
        return ERR_INVALID_VALUE;
    }
    resultSet->Close();
    IPCSkeleton::SetCallingIdentity(callingIdentity);
    return ERR_OK;
}

bool SettingProvider::IsNeedMultiUser(const std::string& key)
{
    std::vector<std::string> needMultiUserStrVec {
        SETTING_POWER_WAKEUP_DOUBLE_KEY,
        SETTING_POWER_WAKEUP_PICKUP_KEY,
        SETTING_POWER_WAKEUP_SOURCES_KEY,
#ifdef POWER_MANAGER_ENABLE_CHARGING_TYPE_SETTING
        SETTING_DISPLAY_AC_OFF_TIME_KEY,
        SETTING_DISPLAY_DC_OFF_TIME_KEY,
        SETTING_POWER_AC_SUSPEND_SOURCES_KEY,
        SETTING_POWER_DC_SUSPEND_SOURCES_KEY,
#endif
    };
    
    if (std::count(needMultiUserStrVec.begin(), needMultiUserStrVec.end(), key)) {
        return true;
    }
    return false;
}

std::string SettingProvider::ReplaceUserIdForUri(int32_t userId)
{
    std::string tempUri = SETTING_URI_PROXY_USER_ADAPT;
    std::regex pattern(USERID_REPLACE);
    return std::regex_replace(tempUri, pattern, std::to_string(userId));
}

void SettingProvider::UpdateCurrentUserId()
{
    std::lock_guard<std::mutex> lock(settingMutex_);
    std::vector<int> activedIds;
    int ret = AccountSA::OsAccountManager::QueryActiveOsAccountIds(activedIds);
    if (ret != 0) {
        POWER_HILOGE(COMP_UTILS, "QueryActivedOsAccountIds failed, ret is %{public}d", ret);
        return;
    }
    if (activedIds.empty()) {
        POWER_HILOGE(COMP_UTILS, "QueryActivedOsAccountIds is empty");
        return;
    }
    currentUserId_ = activedIds[0];
    POWER_HILOGI(COMP_UTILS, "currentUserId_ is %{public}d", currentUserId_);
}
} // namespace PowerMgr
} // namespace OHOS