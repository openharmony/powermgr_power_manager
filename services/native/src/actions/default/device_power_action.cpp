/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include <linux/reboot.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>
#include "parameters.h"
#include "securec.h"

#include "hilog_wrapper.h"
#include "init_reboot.h"

namespace {
const std::string UPDATER_CMD = "updater";
const std::string REBOOT_CMD = "";
const std::string SHUTDOWN_CMD = "shutdown";
const std::string FLASH_CMD = "flash";
}

namespace OHOS {
namespace PowerMgr {
void DevicePowerAction::Reboot(const std::string& reason)
{
    std::string rebootReason = Updater(reason);
    POWER_HILOGI(MODULE_SERVICE, "Reboot executing.");
    DoReboot(rebootReason.c_str());
}

void DevicePowerAction::Shutdown(const std::string& reason)
{
    POWER_HILOGI(MODULE_SERVICE, "Shutdown executing.");
    DoReboot(SHUTDOWN_CMD.c_str());
}

std::string DevicePowerAction::Updater(const std::string& reason)
{
    return (reason.find(UPDATER_CMD) != std::string::npos) ? UPDATER_CMD : REBOOT_CMD;
}
} // namespace PowerMgr
} // namespace OHOS
