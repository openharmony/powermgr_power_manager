/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "mock_accesstoken_kit.h"

using namespace OHOS::Security::AccessToken;
namespace OHOS {
namespace PowerMgr {
namespace {
ATokenTypeEnum g_mockGetTokenTypeFlagRet = ATokenTypeEnum::TOKEN_INVALID;
PermissionState g_mockState = PermissionState::PERMISSION_DENIED;
bool g_isSystemApp = false;
} // namespace

void MockAccesstokenKit::MockSetTokenTypeFlag(ATokenTypeEnum mockRet)
{
    g_mockGetTokenTypeFlagRet = mockRet;
}

void MockAccesstokenKit::MockSetPermissionState(PermissionState mockState)
{
    g_mockState = mockState;
}

void MockAccesstokenKit::MockSetSystemApp(bool isSystemApp)
{
    g_isSystemApp = isSystemApp;
}
} // namespace PowerMgr
} // namespace OHOS

namespace OHOS {
namespace Security {
namespace AccessToken {
int AccessTokenKit::VerifyAccessToken(AccessTokenID tokenID, const std::string& permissionName)
{
    return PowerMgr::g_mockState;
}

ATokenTypeEnum AccessTokenKit::GetTokenTypeFlag(AccessTokenID tokenID)
{
    return PowerMgr::g_mockGetTokenTypeFlagRet;
}

bool TokenIdKit::IsSystemAppByFullTokenID(uint64_t tokenId)
{
    return PowerMgr::g_isSystemApp;
}
} // namespace AccessToken
} // namespace Security
} // namespace OHOS
