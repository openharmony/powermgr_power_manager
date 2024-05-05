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
} // namespace PowerMgr
} // namespace OHOS
