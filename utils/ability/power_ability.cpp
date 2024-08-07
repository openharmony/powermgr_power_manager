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

#include <ability_manager_client.h>
#include "power_log.h"

using namespace OHOS;
using namespace PowerMgr;
using namespace AAFwk;

extern "C" void PowerConnectAbility(const Want &want, const sptr<IAbilityConnection>& connect, int32_t userId)
{
    auto amsClient = AbilityManagerClient::GetInstance();
    if (amsClient == nullptr) {
        POWER_HILOGW(FEATURE_UTIL, "AbilityManagerClient is nullptr");
        return;
    }
    ErrCode result = amsClient->ConnectAbility(want, connect, userId);
    if (result != ERR_OK) {
        POWER_HILOGW(FEATURE_UTIL, "ConnectAbility systemui dialog failed, result = %{public}d", result);
        return;
    }
    POWER_HILOGI(FEATURE_UTIL, "ConnectAbility systemui dialog success.");
}

extern "C" void PowerStartAbility(const Want &want)
{
    auto amsClient = AbilityManagerClient::GetInstance();
    if (amsClient == nullptr) {
        POWER_HILOGW(FEATURE_UTIL, "AbilityManagerClient is nullptr");
        return;
    }
    ErrCode result = amsClient->StartAbility(want);
    if (result != ERR_OK) {
        POWER_HILOGW(FEATURE_UTIL, "StartAbility systemui dialog failed, result = %{public}d", result);
        return;
    }
    POWER_HILOGI(FEATURE_UTIL, "StartAbility systemui dialog success.");
}