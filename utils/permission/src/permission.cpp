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

#include "permission.h"

#include "accesstoken_kit.h"
#include "ipc_skeleton.h"
#include "power_log.h"

using namespace OHOS::Security::AccessToken;

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
    return IsTokenAplMatch(ATokenAplEnum::APL_SYSTEM_CORE);
}

bool Permission::IsSystemBasic()
{
    return IsTokenAplMatch(ATokenAplEnum::APL_SYSTEM_BASIC);
}

bool Permission::IsSystem()
{
    return IsSystemCore() || IsSystemBasic();
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
            result = AccessTokenKit::VerifyNativeToken(tokenId, perm);
            break;
        case ATokenTypeEnum::TOKEN_INVALID:
            break;
    }
    if (result == PermissionState::PERMISSION_DENIED) {
        POWER_HILOGW(COMP_UTILS, "permission denied, perm=%{public}s type=%{public}d, pid=%{public}d, uid=%{public}d",
            perm.c_str(), static_cast<int32_t>(type), pid, uid);
        return false;
    }
    return true;
}

bool Permission::IsSystemBasicPermGranted(const std::string& perm)
{
    return IsSystemBasic() && IsPermissionGranted(perm);
}
} // namespace PowerMgr
} // namespace OHOS
