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

#include "hilog_wrapper.h"

namespace OHOS {
namespace PowerMgr {
void DevicePowerAction::Reboot(const std::string& reason)
{
    POWER_HILOGI(MODULE_SERVICE, "Reboot executing.");
    syscall(__NR_reboot, LINUX_REBOOT_MAGIC1, LINUX_REBOOT_MAGIC2, LINUX_REBOOT_CMD_RESTART2, reason.c_str());
}

void DevicePowerAction::Shutdown(const std::string& reason)
{
    POWER_HILOGI(MODULE_SERVICE, "Shutdown executing.");
    syscall(__NR_reboot, LINUX_REBOOT_MAGIC1, LINUX_REBOOT_MAGIC2, LINUX_REBOOT_CMD_POWER_OFF, reason.c_str());
}
} // namespace PowerMgr
} // namespace OHOS
