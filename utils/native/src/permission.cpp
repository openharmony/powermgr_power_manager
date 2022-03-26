/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "permission.h"

#include "accesstoken_kit.h"
#include "bundle_mgr_interface.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "power_log.h"

using namespace std;
using namespace OHOS;
using namespace OHOS::AppExecFwk;
using namespace OHOS::Security::AccessToken;

namespace {
static sptr<IBundleMgr> g_bundleMgr = nullptr;
}

namespace OHOS {
namespace PowerMgr {
bool Permission::CheckCallingPermission(const string& perm)
{
    AccessTokenID tokenId = IPCSkeleton::GetCallingTokenID();
    pid_t pid = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();

    int32_t result;
    auto type = AccessTokenKit::GetTokenTypeFlag(tokenId);
    if (type == ATokenTypeEnum::TOKEN_NATIVE) {
        result = AccessTokenKit::VerifyNativeToken(tokenId, perm);
    } else if (type == ATokenTypeEnum::TOKEN_HAP) {
        result = AccessTokenKit::VerifyAccessToken(tokenId, perm);
    } else {
        POWER_HILOGW(COMP_SVC,
            "Access token type error, type=%{public}d, pid=%{public}d, uid=%{public}d, perm=%{public}s",
            type, pid, uid, perm.c_str());
        return false;
    }
    bool isPermissionGranted = (result == PermissionState::PERMISSION_GRANTED);
    POWER_HILOGI(COMP_SVC, "isPermissionGranted=%{public}d, pid=%{public}d, uid=%{public}d, perm=%{public}s",
        isPermissionGranted, pid, uid, perm.c_str());
    return isPermissionGranted;
}

sptr<IBundleMgr> GetBundleMgr()
{
    if (g_bundleMgr != nullptr) {
        return g_bundleMgr;
    }
    auto sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        POWER_HILOGW(COMP_SVC, "GetSystemAbilityManager return nullptr");
        return nullptr;
    }
    auto bundleMgrSa = sam->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (bundleMgrSa == nullptr) {
        POWER_HILOGW(COMP_SVC, "GetSystemAbility return nullptr");
        return nullptr;
    }
    auto bundleMgr = iface_cast<IBundleMgr>(bundleMgrSa);
    if (bundleMgr == nullptr) {
        POWER_HILOGW(COMP_SVC, "iface_cast return nullptr");
    }
    g_bundleMgr = bundleMgr;
    return g_bundleMgr;
}

bool Permission::CheckIsSystemAppByUid(int32_t uid)
{
    auto bundleMgr = GetBundleMgr();
    if (bundleMgr == nullptr) {
        POWER_HILOGW(COMP_SVC, "BundleMgr is nullptr, return false");
        return false;
    }
    return bundleMgr->CheckIsSystemAppByUid(uid);
}
} // namespace PowerMgr
} // namespace OHOS
