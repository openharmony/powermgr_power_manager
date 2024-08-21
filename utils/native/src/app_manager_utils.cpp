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

#include "power_log.h"
#include <ability_manager_proxy.h>
#include <app_mgr_interface.h>
#include <if_system_ability_manager.h>
#include <iservice_registry.h>

namespace OHOS {
namespace PowerMgr {
static constexpr uint32_t APP_MGR_SERVICE_ID = 501;
sptr<OHOS::AppExecFwk::IAppMgr> AppManagerUtils::appManagerInstance_ = nullptr;

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
        FEATURE_UTIL, "GetForegroundApplications, ret: %{public}u, num of apps: %{public}u", ret, appsData.size());
}

bool AppManagerUtils::IsForegroundApplication(const std::string& appName)
{
    if (appName.empty()) {
        POWER_HILOGW(FEATURE_UTIL, "IsForegroundApplication: app name is empty");
        return false;
    }

    bool IsForeground = false;
    std::vector<OHOS::AppExecFwk::AppStateData> appsData;
    GetForegroundApplications(appsData);
    for (const auto& curApp : appsData) {
        if (curApp.bundleName == appName) {
            IsForeground = true;
            break;
        }
    }

    POWER_HILOGI(FEATURE_UTIL, "IsForegroundApplication, ret: %{public}zu", static_cast<uint32_t>(IsForeground));
    return IsForeground;
}

} // namespace PowerMgr
} // namespace OHOS