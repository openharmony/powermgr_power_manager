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

#include <linux/reboot.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>
#include "parameters.h"
#include "securec.h"

#include "hilog_wrapper.h"
#include "device_power_action.h"
#ifdef __cplusplus
extern "C"
{
#endif
#include "init_reboot.h"
#ifdef __cplusplus
}
#endif

namespace OHOS {
namespace PowerMgr {
void DevicePowerAction::Reboot(const std::string& reason)
{
    int32_t propertyMaxSize = PROPERTY_MAX_SIZE;
    char updateCmd[propertyMaxSize];
    if (snprintf_s(updateCmd, propertyMaxSize, propertyMaxSize - 1, "reboot,%s", reason.c_str()) == 0) {
        return;
    }
    POWER_HILOGI(MODULE_SERVICE, "Reboot executing.");
    DoReboot(updateCmd);
}

void DevicePowerAction::Shutdown(const std::string& reason)
{
    int32_t propertyMaxSize = PROPERTY_MAX_SIZE;
    char updateCmd[propertyMaxSize];
    if (snprintf_s(updateCmd, propertyMaxSize, propertyMaxSize - 1, "shutdown,%s", reason.c_str()) == 0) {
        return;
    }
    POWER_HILOGI(MODULE_SERVICE, "Shutdown executing.");
    DoReboot(updateCmd);
}
} // namespace PowerMgr
} // namespace OHOS
