/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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

#include "device_power_action.h"

#include <string>
#include <init_reboot.h>
#include <list.h>
#include "power_hookmgr.h"
#include "power_log.h"

namespace {
const std::string INVALID_CMD = "invalid_cmd";
const std::string UPDATER_CMD = "updater";
const std::string REBOOT_CMD = "";
const std::string SHUTDOWN_CMD = "shutdown";
const std::string FLASH_CMD = "flash";
}

namespace OHOS {
namespace PowerMgr {
void DevicePowerAction::Reboot(const std::string& reason)
{
    std::string rebootCmd;
    RebootCmdInfo rebootInfo = {.rebootReason = reason, .rebootCmd = INVALID_CMD};
    HOOK_EXEC_OPTIONS options {TRAVERSE_STOP_WHEN_ERROR, nullptr, nullptr};
    HOOK_MGR* hookMgr = GetPowerHookMgr();
    if (HookMgrExecute(hookMgr, static_cast<int32_t>(PowerHookStage::POWER_PRE_DO_REBOOT),
        &rebootInfo, &options) == 0) {
        rebootCmd = rebootInfo.rebootCmd;
    } else {
        rebootCmd = Updater(reason);
    }
    POWER_KHILOGI(FEATURE_SHUTDOWN,
        "Reboot reason: %{public}s, command: %{public}s", reason.c_str(), rebootCmd.c_str());
    DoRebootExt(rebootCmd.c_str(), reason.c_str());
}

void DevicePowerAction::Shutdown(const std::string& reason)
{
#ifdef POWER_MANAGER_POWEROFF_CHARGE
    HOOK_MGR* hookMgr = GetPowerHookMgr();
    std::string hookContext = reason;
    HookMgrExecute(hookMgr, static_cast<int32_t>(PowerHookStage::POWER_PRE_DO_SHUTDOWN), &hookContext, nullptr);
#endif
    POWER_KHILOGI(FEATURE_SHUTDOWN, "Shutdown executing");
    DoRebootExt(SHUTDOWN_CMD.c_str(), reason.c_str());
}

std::string DevicePowerAction::Updater(const std::string& reason)
{
    return (reason.find(UPDATER_CMD) != std::string::npos) ? UPDATER_CMD : REBOOT_CMD;
}
} // namespace PowerMgr
} // namespace OHOS
