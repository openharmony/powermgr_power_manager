/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "power_log.h"

using namespace OHOS;
using namespace OHOS::AppExecFwk;
using namespace OHOS::Security::AccessToken;

namespace {
static sptr<IBundleMgr> g_bundleMgr = nullptr;
}

namespace OHOS {
namespace PowerMgr {
static bool IsTokenAplMatch(ATokenAplEnum apl)
{
    AccessTokenID tokenId = IPCSkeleton::GetCallingTokenID();
    pid_t pid = IPCSkeleton::GetCallingPid();
    pid_t uid = IPCSkeleton::GetCallingUid();
    ATokenTypeEnum type = AccessTokenKit::GetTokenTypeFlag(tokenId);
    POWER_HILOGD(COMP_UTILS, "checking apl, apl=%{public}d, type=%{public}d, pid=%{public}d, uid=%{public}d",
        static_cast<int32_t>(apl), static_cast<int32_t>(type), pid, uid);
    NativeTokenInfo info;
    AccessTokenKit::GetNativeTokenInfo(tokenId, info);
    if (info.apl == apl) {
        return true;
    }
    POWER_HILOGW(COMP_UTILS, "apl not match, info.apl=%{public}d, type=%{public}d, pid=%{public}d, uid=%{public}d",
        static_cast<int32_t>(info.apl), static_cast<int32_t>(type), pid, uid);
    return false;
}

bool Permission::IsSystemCore()
{
    bool isMatch = IsTokenAplMatch(ATokenAplEnum::APL_SYSTEM_CORE);
    if (!isMatch) {
        POWER_HILOGW(COMP_UTILS, "access token denied");
    }
    return isMatch;
}

bool Permission::IsSystemBasic()
{
    bool isMatch = IsTokenAplMatch(ATokenAplEnum::APL_SYSTEM_BASIC);
    if (!isMatch) {
        POWER_HILOGW(COMP_UTILS, "access token denied");
    }
    return isMatch;
}

bool Permission::IsSystemApl()
{
    return IsSystemBasic() || IsSystemCore();
}

static sptr<IBundleMgr> GetBundleMgr()
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

bool Permission::IsSystemHap()
{
    AccessTokenID tokenId = IPCSkeleton::GetCallingTokenID();
    pid_t pid = IPCSkeleton::GetCallingPid();
    pid_t uid = IPCSkeleton::GetCallingUid();
    ATokenTypeEnum type = AccessTokenKit::GetTokenTypeFlag(tokenId);
    POWER_HILOGD(COMP_UTILS, "checking system hap, type=%{public}d, pid=%{public}d, uid=%{public}d",
        static_cast<int32_t>(type), pid, uid);
    bool result = false;
    switch (type) {
        case ATokenTypeEnum::TOKEN_HAP:
        {
            auto bundleMgr = GetBundleMgr();
            if (bundleMgr == nullptr) {
                POWER_HILOGW(COMP_SVC, "BundleMgr is nullptr, return false");
                return false;
            }
            result = bundleMgr->CheckIsSystemAppByUid(uid);
            break;
        }
        case ATokenTypeEnum::TOKEN_NATIVE:
        case ATokenTypeEnum::TOKEN_SHELL:
            result = true;
            break;
        case ATokenTypeEnum::TOKEN_INVALID:
        case ATokenTypeEnum::TOKEN_TYPE_BUTT:
            break;
        default:
            break;
    }
    if (!result) {
        POWER_HILOGW(COMP_UTILS, "system denied, type=%{public}d, pid=%{public}d, uid=%{public}d",
            static_cast<int32_t>(type), pid, uid);
        return false;
    }
    return true;
}

bool Permission::IsSystem()
{
    return IsSystemApl() || IsSystemHap();
}

bool Permission::IsPermissionGranted(const std::string& perm)
{
    AccessTokenID tokenId = IPCSkeleton::GetCallingTokenID();
    pid_t pid = IPCSkeleton::GetCallingPid();
    pid_t uid = IPCSkeleton::GetCallingUid();
    ATokenTypeEnum type = AccessTokenKit::GetTokenTypeFlag(tokenId);
    POWER_HILOGD(COMP_UTILS, "checking permission, perm=%{public}s type=%{public}d, pid=%{public}d, uid=%{public}d",
        perm.c_str(), static_cast<int32_t>(type), pid, uid);
    int32_t result = PermissionState::PERMISSION_DENIED;
    switch (type) {
        case ATokenTypeEnum::TOKEN_HAP:
            result = AccessTokenKit::VerifyAccessToken(tokenId, perm);
            break;
        case ATokenTypeEnum::TOKEN_NATIVE:
        case ATokenTypeEnum::TOKEN_SHELL:
            result = PermissionState::PERMISSION_GRANTED;
            break;
        case ATokenTypeEnum::TOKEN_INVALID:
        case ATokenTypeEnum::TOKEN_TYPE_BUTT:
            break;
    }
    if (result == PermissionState::PERMISSION_DENIED) {
        POWER_HILOGW(COMP_UTILS, "permission denied, perm=%{public}s type=%{public}d, pid=%{public}d, uid=%{public}d",
            perm.c_str(), static_cast<int32_t>(type), pid, uid);
        return false;
    }
    return true;
}

bool Permission::IsSystemHapPermGranted(const std::string& perm)
{
    return IsSystemHap() && IsPermissionGranted(perm);
}
} // namespace PowerMgr
} // namespace OHOS
