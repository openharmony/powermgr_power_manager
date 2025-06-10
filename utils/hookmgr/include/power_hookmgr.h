/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef POWERMGR_UTILS_HOOKMGR_POWER_HOOKMGR_H
#define POWERMGR_UTILS_HOOKMGR_POWER_HOOKMGR_H

#include <cstdint>
#include <string>
#include <hookmgr.h>
#include "modulemgr.h"

namespace OHOS {
namespace PowerMgr {
enum class PowerHookStage : int32_t {
    POWER_HDI_CALLBACK_WAKEUP = 0,
    POWER_PRE_SWITCH_ACCOUNT,
    POWER_POST_SWITCH_ACCOUNT,
    POWER_PRE_DO_REBOOT,
    POWER_PRE_DO_SHUTDOWN,
    POWER_HOOK_STAGE_MAX = 1000,
};

struct RebootCmdInfo {
    std::string rebootReason;
    std::string rebootCmd;
};

HOOK_MGR* GetPowerHookMgr();
} // namespace PowerMgr
} // namespace OHOS

#endif // POWERMGR_UTILS_HOOKMGR_POWER_HOOKMGR_H