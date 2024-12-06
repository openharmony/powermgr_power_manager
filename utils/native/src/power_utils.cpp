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

#include <dlfcn.h>
#include <string>
#include "power_log.h"
#include "power_utils.h"

namespace OHOS {
namespace PowerMgr {
const std::string PowerUtils::GetReasonTypeString(StateChangeReason type)
{
    switch (type) {
        case StateChangeReason::STATE_CHANGE_REASON_INIT:
            return std::string("INIT");
        case StateChangeReason::STATE_CHANGE_REASON_TIMEOUT:
            return std::string("TIMEOUT");
        case StateChangeReason::STATE_CHANGE_REASON_RUNNING_LOCK:
            return std::string("RUNNING_LOCK");
        case StateChangeReason::STATE_CHANGE_REASON_BATTERY:
            return std::string("BATTERY");
        case StateChangeReason::STATE_CHANGE_REASON_THERMAL:
            return std::string("THERMAL");
        case StateChangeReason::STATE_CHANGE_REASON_WORK:
            return std::string("WORK");
        case StateChangeReason::STATE_CHANGE_REASON_SYSTEM:
            return std::string("SYSTEM");
        case StateChangeReason::STATE_CHANGE_REASON_APPLICATION:
            return std::string("APPLICATION");
        case StateChangeReason::STATE_CHANGE_REASON_SETTINGS:
            return std::string("SETTINGS");
        case StateChangeReason::STATE_CHANGE_REASON_HARD_KEY:
            return std::string("HARD_KEY");
        case StateChangeReason::STATE_CHANGE_REASON_TOUCH:
            return std::string("TOUCH");
        case StateChangeReason::STATE_CHANGE_REASON_CABLE:
            return std::string("CABLE");
        case StateChangeReason::STATE_CHANGE_REASON_SENSOR:
            return std::string("SENSOR");
        case StateChangeReason::STATE_CHANGE_REASON_LID:
            return std::string("LID");
        case StateChangeReason::STATE_CHANGE_REASON_CAMERA:
            return std::string("CAMERA");
        case StateChangeReason::STATE_CHANGE_REASON_ACCESSIBILITY:
            return std::string("ACCESS");
        case StateChangeReason::STATE_CHANGE_REASON_RESET:
            return std::string("RESET");
        case StateChangeReason::STATE_CHANGE_REASON_POWER_KEY:
            return std::string("POWER_KEY");
        case StateChangeReason::STATE_CHANGE_REASON_KEYBOARD:
            return std::string("KEYBOARD");
        case StateChangeReason::STATE_CHANGE_REASON_MOUSE:
            return std::string("MOUSE");
        case StateChangeReason::STATE_CHANGE_REASON_DOUBLE_CLICK:
            return std::string("DOUBLE_CLICK");
        case StateChangeReason::STATE_CHANGE_REASON_PEN:
            return std::string("PEN");
        case StateChangeReason::STATE_CHANGE_REASON_SWITCH:
            return std::string("SWITCH");
        case StateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT:
            return std::string("PRE_BRIGHT");
        case StateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_SUCCESS:
            return std::string("PRE_BRIGHT_ATUH_SUCCESS");
        case StateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_FAIL_SCREEN_ON:
            return std::string("PRE_BRIGHT_ATUH_FAIL_SCREEN_ON");
        case StateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_FAIL_SCREEN_OFF:
            return std::string("PRE_BRIGHT_ATUH_FAIL_SCREEN_OFF");
        case StateChangeReason::STATE_CHANGE_REASON_AOD_SLIDING:
            return std::string("AOD_SLIDING");
        case StateChangeReason::STATE_CHANGE_REASON_REMOTE:
            return std::string("REMOTE");
        case StateChangeReason::STATE_CHANGE_REASON_REFRESH:
            return std::string("REFRESH");
        case StateChangeReason::STATE_CHANGE_REASON_COORDINATION:
            return std::string("COORDINATION_OVERRIDE");
        case StateChangeReason::STATE_CHANGE_REASON_PROXIMITY:
            return std::string("PROXIMITY");
        case StateChangeReason::STATE_CHANGE_REASON_INCOMING_CALL:
            return std::string("INCOMING_CALL");
        case StateChangeReason::STATE_CHANGE_REASON_SHELL:
            return std::string("SHELL");
        case StateChangeReason::STATE_CHANGE_REASON_TIMEOUT_NO_SCREEN_LOCK:
            return std::string("TIMEOUT_NO_SCREEN_LOCK");
        case StateChangeReason::STATE_CHANGE_REASON_PICKUP:
            return std::string("PICKUP");
        case StateChangeReason::STATE_CHANGE_REASON_SCREEN_CONNECT:
            return std::string("SCREEN_CONNECT");
        case StateChangeReason::STATE_CHANGE_REASON_EXIT_SYSTEM_STR:
            return std::string("EXIT_SYSTEM_STR");
        case StateChangeReason::STATE_CHANGE_REASON_UNKNOWN:
            return std::string("UNKNOWN");
        default:
            break;
    }
    return std::string("UNKNOWN");
}

const std::string PowerUtils::GetPowerStateString(PowerState state)
{
    switch (state) {
        case PowerState::AWAKE:
            return std::string("AWAKE");
        case PowerState::FREEZE:
            return std::string("FREEZE");
        case PowerState::INACTIVE:
            return std::string("INACTIVE");
        case PowerState::STAND_BY:
            return std::string("STAND_BY");
        case PowerState::DOZE:
            return std::string("DOZE");
        case PowerState::SLEEP:
            return std::string("SLEEP");
        case PowerState::HIBERNATE:
            return std::string("HIBERNATE");
        case PowerState::SHUTDOWN:
            return std::string("SHUTDOWN");
        case PowerState::DIM:
            return std::string("DIM");
        case PowerState::UNKNOWN:
            return std::string("UNKNOWN");
        default:
            break;
    }
    return std::string("UNKNOWN");
}

const std::string PowerUtils::GetDisplayStateString(DisplayState state)
{
    switch (state) {
        case DisplayState::DISPLAY_OFF:
            return std::string("DISPLAY_OFF");
        case DisplayState::DISPLAY_DIM:
            return std::string("DISPLAY_DIM");
        case DisplayState::DISPLAY_ON:
            return std::string("DISPLAY_ON");
        case DisplayState::DISPLAY_SUSPEND:
            return std::string("DISPLAY_SUSPEND");
        case DisplayState::DISPLAY_UNKNOWN:
            return std::string("DISPLAY_UNKNOWN");
        default:
            break;
    }
    return std::string("DISPLAY_UNKNOWN");
}

const std::string PowerUtils::GetRunningLockTypeString(RunningLockType type)
{
    switch (type) {
        case RunningLockType::RUNNINGLOCK_SCREEN:
            return "SCREEN";
        case RunningLockType::RUNNINGLOCK_BACKGROUND:
            return "BACKGROUND";
        case RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL:
            return "PROXIMITY_SCREEN_CONTROL";
        case RunningLockType::RUNNINGLOCK_COORDINATION:
            return "RUNNINGLOCK_COORDINATION";
        case RunningLockType::RUNNINGLOCK_BACKGROUND_PHONE:
            return "BACKGROUND_PHONE";
        case RunningLockType::RUNNINGLOCK_BACKGROUND_NOTIFICATION:
            return "BACKGROUND_NOTIFICATION";
        case RunningLockType::RUNNINGLOCK_BACKGROUND_AUDIO:
            return "BACKGROUND_AUDIO";
        case RunningLockType::RUNNINGLOCK_BACKGROUND_SPORT:
            return "BACKGROUND_SPORT";
        case RunningLockType::RUNNINGLOCK_BACKGROUND_NAVIGATION:
            return "BACKGROUND_NAVIGATION";
        case RunningLockType::RUNNINGLOCK_BACKGROUND_TASK:
            return "BACKGROUND_TASK";
        case RunningLockType::RUNNINGLOCK_BUTT:
            return "BUTT";
        default:
            break;
    }
    return "UNKNOWN";
}

Rosen::PowerStateChangeReason PowerUtils::GetDmsReasonByPowerReason(StateChangeReason reason)
{
    using namespace Rosen;
    PowerStateChangeReason dmsReason = static_cast<PowerStateChangeReason>(reason);
    switch (reason) {
        case StateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT:
            dmsReason = PowerStateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT;
            break;
        case StateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_SUCCESS:
            dmsReason = PowerStateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_SUCCESS;
            break;
        case StateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_FAIL_SCREEN_ON:
            dmsReason = PowerStateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_FAIL_SCREEN_ON;
            break;
        case StateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_FAIL_SCREEN_OFF:
            dmsReason = PowerStateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_FAIL_SCREEN_OFF;
            break;
        case StateChangeReason::STATE_CHANGE_REASON_POWER_KEY:
            dmsReason = PowerStateChangeReason::STATE_CHANGE_REASON_POWER_KEY;
            break;
        case StateChangeReason::STATE_CHANGE_REASON_TIMEOUT_NO_SCREEN_LOCK:
            dmsReason = PowerStateChangeReason::STATE_CHANGE_REASON_COLLABORATION;
            break;
        case StateChangeReason::STATE_CHANGE_REASON_SWITCH:
            dmsReason = PowerStateChangeReason::STATE_CHANGE_REASON_SWITCH;
            break;
        case StateChangeReason::STATE_CHANGE_REASON_SCREEN_CONNECT:
            dmsReason = PowerStateChangeReason::STATE_CHANGE_REASON_SCREEN_CONNECT;
            break;
        default:
            break;
    }
    POWER_HILOGI(FEATURE_POWER_STATE, "The reason to DMS is = %{public}d", static_cast<uint32_t>(dmsReason));
    return dmsReason;
}

StateChangeReason PowerUtils::GetReasonByUserActivity(UserActivityType type)
{
    StateChangeReason ret = StateChangeReason::STATE_CHANGE_REASON_UNKNOWN;
    switch (type) {
        case UserActivityType::USER_ACTIVITY_TYPE_BUTTON:
            ret = StateChangeReason::STATE_CHANGE_REASON_HARD_KEY;
            break;
        case UserActivityType::USER_ACTIVITY_TYPE_TOUCH:
            ret = StateChangeReason::STATE_CHANGE_REASON_TOUCH;
            break;
        case UserActivityType::USER_ACTIVITY_TYPE_ACCESSIBILITY:
            ret = StateChangeReason::STATE_CHANGE_REASON_ACCESSIBILITY;
            break;
        case UserActivityType::USER_ACTIVITY_TYPE_SOFTWARE:
            ret = StateChangeReason::STATE_CHANGE_REASON_APPLICATION;
            break;
        case UserActivityType::USER_ACTIVITY_TYPE_SWITCH:
            ret = StateChangeReason::STATE_CHANGE_REASON_SWITCH;
            break;
        case UserActivityType::USER_ACTIVITY_TYPE_CABLE:
            ret = StateChangeReason::STATE_CHANGE_REASON_CABLE;
            break;
        case UserActivityType::USER_ACTIVITY_TYPE_ATTENTION: // fall through
        case UserActivityType::USER_ACTIVITY_TYPE_OTHER:     // fall through
        default:
            break;
    }
    return ret;
}

WakeupDeviceType PowerUtils::ParseWakeupDeviceType(const std::string& details)
{
    WakeupDeviceType parsedType = WakeupDeviceType::WAKEUP_DEVICE_APPLICATION;

    if (strcmp(details.c_str(), "pre_bright") == 0) {
        parsedType = WakeupDeviceType::WAKEUP_DEVICE_PRE_BRIGHT;
    } else if (strcmp(details.c_str(), "pre_bright_auth_success") == 0) {
        parsedType = WakeupDeviceType::WAKEUP_DEVICE_PRE_BRIGHT_AUTH_SUCCESS;
    } else if (strcmp(details.c_str(), "pre_bright_auth_fail_screen_on") == 0) {
        parsedType = WakeupDeviceType::WAKEUP_DEVICE_PRE_BRIGHT_AUTH_FAIL_SCREEN_ON;
    } else if (strcmp(details.c_str(), "pre_bright_auth_fail_screen_off") == 0) {
        parsedType = WakeupDeviceType::WAKEUP_DEVICE_PRE_BRIGHT_AUTH_FAIL_SCREEN_OFF;
    } else if (strcmp(details.c_str(), "incoming call") == 0) {
        parsedType = WakeupDeviceType::WAKEUP_DEVICE_INCOMING_CALL;
    } else if (strcmp(details.c_str(), "fake_str_check_unlock") == 0) {
        parsedType = WakeupDeviceType::WAKEUP_DEVICE_EXIT_SYSTEM_STR;
    } else if (strcmp(details.c_str(), "shell") == 0) {
        parsedType = WakeupDeviceType::WAKEUP_DEVICE_SHELL;
    }

    if (parsedType != WakeupDeviceType::WAKEUP_DEVICE_APPLICATION) {
        POWER_HILOGI(FEATURE_WAKEUP, "Parsed wakeup type is %{public}d", static_cast<uint32_t>(parsedType));
    }
    return parsedType;
}

const std::string PowerUtils::JsonToSimpleStr(const std::string& json)
{
    std::string str;
    for (auto ch : json) {
        if (ch != ' ' && ch != '\n') {
            str += ch;
        }
    }
    return str;
}

bool PowerUtils::IsForegroundApplication(const std::string& appName)
{
    void* handler = dlopen("libpower_ability.z.so", RTLD_NOW);
    if (handler == nullptr) {
        POWER_HILOGE(FEATURE_UTIL, "dlopen libpower_ability.z.so failed, reason : %{public}s", dlerror());
        return false;
    }

    auto powerIsForegroundApplicationFunc =
        reinterpret_cast<bool (*)(const std::string&)>(dlsym(handler, "PowerIsForegroundApplication"));
    if (powerIsForegroundApplicationFunc == nullptr) {
        POWER_HILOGE(FEATURE_UTIL, "find PowerIsForegroundApplication function failed, reason : %{public}s", dlerror());
#ifndef FUZZ_TEST
        dlclose(handler);
#endif
        handler = nullptr;
        return false;
    }
    bool isForeground = powerIsForegroundApplicationFunc(appName);
#ifndef FUZZ_TEST
    dlclose(handler);
#endif
    handler = nullptr;
    return isForeground;
}
} // namespace PowerMgr
} // namespace OHOS
