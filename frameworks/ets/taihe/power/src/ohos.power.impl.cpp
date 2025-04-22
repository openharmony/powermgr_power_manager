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

#include "ohos.power.proj.hpp"
#include "ohos.power.impl.hpp"
#include "taihe/runtime.hpp"
#include "stdexcept"
#include "power_common.h"
#include "power_log.h"
#include "power_mgr_client.h"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "app_manager_utils.h"

#define SET_REBOOT _IOW(BOOT_DETECTOR_IOCTL_BASE, 109, int)

using namespace taihe;
using namespace ohos::power;
using namespace OHOS::PowerMgr;

namespace {
std::map<PowerErrors, std::string> errorTable = {
    {PowerErrors::ERR_CONNECTION_FAIL,   "Failed to connect to the service."},
    {PowerErrors::ERR_PERMISSION_DENIED, "Permission is denied"             },
    {PowerErrors::ERR_SYSTEM_API_DENIED, "System permission is denied"      },
    {PowerErrors::ERR_PARAM_INVALID,     "Invalid input parameter."         }
};
static PowerMgrClient& g_powerMgrClient = PowerMgrClient::GetInstance();
constexpr int32_t RESTORE_DEFAULT_SCREENOFF_TIME = -1;

static void SetFrameworkBootStage(bool isReboot)
{
    int fd = open("/dev/bbox", O_WRONLY);
    if (fd < 0) {
        POWER_HILOGE(FEATURE_SHUTDOWN, "open /dev/bbox failed!ret:%{public}d errno:%{public}d", fd, errno);
        return;
    }
    int rebootFlag = isReboot ? 1 : 0;
    int ret = ioctl(fd, SET_REBOOT, &rebootFlag);
    if (ret < 0) {
        POWER_HILOGE(FEATURE_SHUTDOWN, "set reboot flag failed!ret:%{public}d errno:%{public}d", ret, errno);
        close(fd);
        return;
    }
    int stage = SHUT_STAGE_FRAMEWORK_START;
    ret = ioctl(fd, SET_SHUT_STAGE, &stage);
    if (ret < 0) {
        POWER_HILOGE(FEATURE_SHUTDOWN, "set shut stage failed!ret:%{public}d errno:%{public}d", ret, errno);
    }
    close(fd);
    return;
}

void Shutdown(string_view reason)
{
    SetFrameworkBootStage(false);
    POWER_HILOGD(FEATURE_SHUTDOWN, "reboot: %{public}d, reason: %{public}s", false, reason.c_str());
    PowerErrors code = g_powerMgrClient.ShutDownDevice(std::string(reason));
    if (code != PowerErrors::ERR_OK) {
        taihe::set_business_error(static_cast<int32_t>(code), errorTable[code]);
    }
}

void Reboot(string_view reason)
{
    SetFrameworkBootStage(true);
    POWER_HILOGD(FEATURE_SHUTDOWN, "reboot: %{public}d, reason: %{public}s", true, reason.c_str());
    PowerErrors code = g_powerMgrClient.RebootDevice(std::string(reason));
    if (code != PowerErrors::ERR_OK) {
        taihe::set_business_error(static_cast<int32_t>(code), errorTable[code]);
    }
}

bool IsActive()
{
    return g_powerMgrClient.IsScreenOn();
}

void Wakeup(string_view detail)
{
    POWER_HILOGD(FEATURE_WAKEUP, "Wakeup type: APPLICATION, reason: %{public}s", detail.c_str());
    int32_t apiVersion = AppManagerUtils::GetApiTargetVersion();
    PowerErrors code = g_powerMgrClient.WakeupDevice(
        WakeupDeviceType::WAKEUP_DEVICE_APPLICATION, std::string(detail), std::to_string(apiVersion));
    if (code != PowerErrors::ERR_OK) {
        taihe::set_business_error(static_cast<int32_t>(code), errorTable[code]);
    }
}

void Suspend(optional_view<bool> isImmediate)
{
    bool isForce = isImmediate.value_or(false);

    PowerErrors code;
    int32_t apiVersion = AppManagerUtils::GetApiTargetVersion();
    if (isForce) {
        code = g_powerMgrClient.ForceSuspendDevice(std::to_string(apiVersion));
    } else {
        code = g_powerMgrClient.SuspendDevice(
            SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false, std::to_string(apiVersion));
    }
    if (code != PowerErrors::ERR_OK) {
        POWER_HILOGE(FEATURE_SUSPEND, "Suspend Device fail, isForce:%{public}d", isForce);
        taihe::set_business_error(static_cast<int32_t>(code), errorTable[code]);
    }
}

DevicePowerMode GetPowerMode()
{
    PowerMode mode = g_powerMgrClient.GetDeviceMode();
    DevicePowerMode deviceMode(DevicePowerMode::key_t::MODE_NORMAL);
    switch (mode) {
        case PowerMode::NORMAL_MODE:
            deviceMode = DevicePowerMode::key_t::MODE_NORMAL;
            break;
        case PowerMode::POWER_SAVE_MODE:
            deviceMode = DevicePowerMode::key_t::MODE_POWER_SAVE;
            break;
        case PowerMode::PERFORMANCE_MODE:
            deviceMode = DevicePowerMode::key_t::MODE_PERFORMANCE;
            break;
        case PowerMode::EXTREME_POWER_SAVE_MODE:
            deviceMode = DevicePowerMode::key_t::MODE_EXTREME_POWER_SAVE;
            break;
        default:
            POWER_HILOGE(FEATURE_POWER_MODE, "Unknown mode");
    }
    return deviceMode;
}

void SetPowerModeSync(DevicePowerMode mode)
{
    PowerErrors code = g_powerMgrClient.SetDeviceMode(PowerMode(static_cast<uint32_t>(mode)));
    if (code != PowerErrors::ERR_OK) {
        taihe::set_business_error(static_cast<int32_t>(code), errorTable[code]);
    }
}

bool IsStandby()
{
    bool isStandby = false;
    PowerErrors code = g_powerMgrClient.IsStandby(isStandby);
    if (code == PowerErrors::ERR_OK) {
        return isStandby;
    }
    taihe::set_business_error(static_cast<int32_t>(code), errorTable[code]);
    return isStandby;
}

void Hibernate(bool clearMemory)
{
    int32_t apiVersion = AppManagerUtils::GetApiTargetVersion();
    PowerErrors code = g_powerMgrClient.Hibernate(clearMemory, std::to_string(apiVersion));
    if (code != PowerErrors::ERR_OK && code != PowerErrors::ERR_FAILURE) {
        POWER_HILOGE(FEATURE_WAKEUP, "Hibernate failed.");
        taihe::set_business_error(static_cast<int32_t>(code), errorTable[code]);
    }
}

void SetScreenOffTime(int32_t timeout)
{
    if (timeout == 0 || (timeout < 0 && timeout != RESTORE_DEFAULT_SCREENOFF_TIME)) {
        POWER_HILOGE(FEATURE_WAKEUP, "timeout is not right.");
        taihe::set_business_error(
            static_cast<int32_t>(PowerErrors::ERR_PARAM_INVALID), errorTable[PowerErrors::ERR_PARAM_INVALID]);
    }

    PowerErrors code;
    int32_t apiVersion = AppManagerUtils::GetApiTargetVersion();
    if (timeout == RESTORE_DEFAULT_SCREENOFF_TIME) {
        code = g_powerMgrClient.RestoreScreenOffTime(std::to_string(apiVersion));
    } else {
        code = g_powerMgrClient.OverrideScreenOffTime(timeout, std::to_string(apiVersion));
    }
    if (code != PowerErrors::ERR_OK) {
        POWER_HILOGE(FEATURE_WAKEUP, "SetScreenOffTime failed.");
        taihe::set_business_error(static_cast<int32_t>(code), errorTable[code]);
    }
}
}  // namespace

TH_EXPORT_CPP_API_Shutdown(Shutdown);
TH_EXPORT_CPP_API_Reboot(Reboot);
TH_EXPORT_CPP_API_IsActive(IsActive);
TH_EXPORT_CPP_API_Wakeup(Wakeup);
TH_EXPORT_CPP_API_Suspend(Suspend);
TH_EXPORT_CPP_API_GetPowerMode(GetPowerMode);
TH_EXPORT_CPP_API_SetPowerModeSync(SetPowerModeSync);
TH_EXPORT_CPP_API_IsStandby(IsStandby);
TH_EXPORT_CPP_API_Hibernate(Hibernate);
TH_EXPORT_CPP_API_SetScreenOffTime(SetScreenOffTime);
