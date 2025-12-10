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

#include "power_hdi_callback.h"
#include "hdf_base.h"
#include "power_hookmgr.h"
#include "power_mgr_service.h"
#include "power_log.h"

using namespace OHOS::HDI::Power::V1_2;

namespace OHOS {
namespace PowerMgr {
int32_t PowerHdiCallback::OnSuspend()
{
    POWER_HILOGD(FEATURE_SUSPEND, "OnSuspend");
    return HDF_SUCCESS;
}

int32_t PowerHdiCallback::OnWakeup()
{
    POWER_HILOGD(FEATURE_WAKEUP, "OnSuspend");
    return HDF_SUCCESS;
}

int32_t PowerHdiCallbackExt::OnSuspendWithTag(const std::string& tag)
{
    POWER_HILOGD(FEATURE_SUSPEND, "OnSuspendWithTag, tag:%{public}s", tag.c_str());
    return HDF_SUCCESS;
}

int32_t PowerHdiCallbackExt::OnWakeupWithTag(const std::string& tag)
{
    POWER_HILOGD(FEATURE_WAKEUP, "OnWakeupWithTag, tag:%{public}s", tag.c_str());
#ifdef POWER_MANAGER_ENABLE_ULSR_PLUGIN
    if (tag == "ulsr") {
        int ret = HookMgrExecute(GetPowerHookMgr(), static_cast<int32_t>(PowerHookStage::POWER_HDI_CALLBACK_WAKEUP),
            nullptr, nullptr);
        POWER_HILOGI(FEATURE_WAKEUP, "OnWakeupWithTag, HookMgrExecute, stage: %{public}d, ret: %{public}d",
            PowerHookStage::POWER_HDI_CALLBACK_WAKEUP, ret);
    }
#endif
#ifdef POWER_MANAGER_ENABLE_SUSPEND_WITH_TAG
    if (tag == "ulsr") {
        auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
        if (pms == nullptr) {
            POWER_HILOGW(FEATURE_WAKEUP, "OnWakeupWithTag, pms null");
            return HDF_FAILURE;
        }
        pms->TriggerUlsrWakeupCallback();
    }
#endif
    return HDF_SUCCESS;
}
} // OHOS
} // PowerMgr
