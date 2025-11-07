/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "app_manager_utils.h"
#ifdef HAS_ABILITY_RUNTIME_PART
#include "bundle_mgr_interface.h"
#include "system_ability_definition.h"
#endif

#include "power_log.h"
#include <app_mgr_interface.h>
#include "app_mgr_proxy.h"
#include <if_system_ability_manager.h>
#include <iservice_registry.h>

namespace OHOS {
namespace PowerMgr {
static constexpr uint32_t APP_MGR_SERVICE_ID = 501;
sptr<OHOS::AppExecFwk::IAppMgr> AppManagerUtils::appManagerInstance_ = nullptr;
namespace {
const int32_t API_VERSION_MOD = 1000;
}

sptr<OHOS::AppExecFwk::IAppMgr> AppManagerUtils::GetAppManagerInstance()
{
    if (appManagerInstance_) {
        return appManagerInstance_;
    }

    sptr<OHOS::ISystemAbilityManager> abilityMgr =
        OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (abilityMgr == nullptr) {
        POWER_HILOGE(FEATURE_UTIL, "Failed to get ISystemAbilityManager");
        return nullptr;
    }
    sptr<IRemoteObject> remoteObject = abilityMgr->GetSystemAbility(APP_MGR_SERVICE_ID);
    if (remoteObject == nullptr) {
        POWER_HILOGE(FEATURE_UTIL, "Failed to get app manager service, id=%{public}u", APP_MGR_SERVICE_ID);
        return nullptr;
    }
    sptr<OHOS::AppExecFwk::IAppMgr> appMgrProxy = iface_cast<OHOS::AppExecFwk::IAppMgr>(remoteObject);
    if (appMgrProxy == nullptr || !appMgrProxy->AsObject()) {
        POWER_HILOGE(FEATURE_UTIL, "Failed to get app manager proxy");
        return nullptr;
    }
    appManagerInstance_ = appMgrProxy;
    return appManagerInstance_;
}

void AppManagerUtils::GetForegroundApplications(std::vector<OHOS::AppExecFwk::AppStateData>& appsData)
{
    auto appMgr = GetAppManagerInstance();
    if (!appMgr) {
        return;
    }
    int32_t ret = appMgr->GetForegroundApplications(appsData);
    POWER_HILOGI(
        FEATURE_UTIL, "GetForegroundApplications, ret: %{public}u, num of apps: %{public}zu", ret, appsData.size());
}

bool AppManagerUtils::IsForegroundApplication(const std::set<std::string>& appNames)
{
    if (appNames.empty()) {
        POWER_HILOGW(FEATURE_UTIL, "IsForegroundApplication: app name is empty");
        return false;
    }

    bool IsForeground = false;
    std::vector<OHOS::AppExecFwk::AppStateData> appsData;
    GetForegroundApplications(appsData);
    for (const auto& curApp : appsData) {
        if (appNames.find(curApp.bundleName) != appNames.end()) {
            IsForeground = true;
            POWER_HILOGI(FEATURE_UTIL, "Current ForegroundApp is %{public}s", curApp.bundleName.c_str());
            break;
        }
    }
    POWER_HILOGI(FEATURE_UTIL, "IsForegroundApplication, ret: %{public}u", static_cast<uint32_t>(IsForeground));
    return IsForeground;
}

int32_t AppManagerUtils::GetApiTargetVersion()
{
#ifdef HAS_ABILITY_RUNTIME_PART
    static int32_t apiTargetVersion = -1;
    if (apiTargetVersion != -1) {
        return apiTargetVersion;
    }
    sptr<OHOS::ISystemAbilityManager> saManager =
        OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (saManager == nullptr) {
        POWER_HILOGE(FEATURE_UTIL, "Failed to get ISystemAbilityManager");
        return 0;
    }
    sptr<OHOS::IRemoteObject> remoteObject = saManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    sptr<OHOS::AppExecFwk::IBundleMgr> bundleMgrProxy = iface_cast<OHOS::AppExecFwk::IBundleMgr>(remoteObject);
    if (bundleMgrProxy == nullptr) {
        POWER_HILOGE(FEATURE_UTIL, "GetApiTargetVersion: bundleMgrProxy is nullptr");
        return 0;
    }
    OHOS::AppExecFwk::BundleInfo bundleInfo;
    auto ret =
        bundleMgrProxy->GetBundleInfoForSelf(OHOS::AppExecFwk::BundleFlag::GET_BUNDLE_WITH_ABILITIES, bundleInfo);
    if (ret != 0) {
        POWER_HILOGI(FEATURE_UTIL, "GetApiTargetVersion: GetBundleInfoForSelf failed");
        return 0;
    }
    int32_t hapApiVersion = bundleInfo.applicationInfo.apiTargetVersion % API_VERSION_MOD;
    apiTargetVersion = hapApiVersion;
    POWER_HILOGI(FEATURE_UTIL, "GetApiTargetVersion: hapApiVersion is %{public}d", hapApiVersion);
    return hapApiVersion;
#else
    POWER_HILOGI(FEATURE_UTIL, "GetApiTargetVersion not support");
    return -1;
#endif
}

void AppManagerUtils::GetForegroundBundleNames(std::set<std::string>& bundleNames)
{
    std::vector<OHOS::AppExecFwk::AppStateData> appList;
    GetForegroundApplications(appList);
    for (const auto &curApp : appList) {
        bundleNames.emplace(curApp.bundleName);
    }
}

} // namespace PowerMgr
} // namespace OHOS