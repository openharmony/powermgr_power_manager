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
const char* PowerUtils::GetReasonTypeString(StateChangeReason type)
{
    switch (type) {
        case StateChangeReason::STATE_CHANGE_REASON_INIT:
            return "INIT";
        case StateChangeReason::STATE_CHANGE_REASON_TIMEOUT:
            return "TIMEOUT";
        case StateChangeReason::STATE_CHANGE_REASON_RUNNING_LOCK:
            return "RUNNING_LOCK";
        case StateChangeReason::STATE_CHANGE_REASON_BATTERY:
            return "BATTERY";
        case StateChangeReason::STATE_CHANGE_REASON_THERMAL:
            return "THERMAL";
        case StateChangeReason::STATE_CHANGE_REASON_WORK:
            return "WORK";
        case StateChangeReason::STATE_CHANGE_REASON_SYSTEM:
            return "SYSTEM";
        case StateChangeReason::STATE_CHANGE_REASON_APPLICATION:
            return "APPLICATION";
        case StateChangeReason::STATE_CHANGE_REASON_SETTINGS:
            return "SETTINGS";
        case StateChangeReason::STATE_CHANGE_REASON_HARD_KEY:
            return "HARD_KEY";
        case StateChangeReason::STATE_CHANGE_REASON_TOUCH:
            return "TOUCH";
        case StateChangeReason::STATE_CHANGE_REASON_CABLE:
            return "CABLE";
        case StateChangeReason::STATE_CHANGE_REASON_SENSOR:
            return "SENSOR";
        case StateChangeReason::STATE_CHANGE_REASON_LID:
            return "LID";
        case StateChangeReason::STATE_CHANGE_REASON_CAMERA:
            return "CAMERA";
        case StateChangeReason::STATE_CHANGE_REASON_ACCESSIBILITY:
            return "ACCESS";
        case StateChangeReason::STATE_CHANGE_REASON_RESET:
            return "RESET";
        case StateChangeReason::STATE_CHANGE_REASON_POWER_KEY:
            return "POWER_KEY";
        case StateChangeReason::STATE_CHANGE_REASON_KEYBOARD:
            return "KEYBOARD";
        case StateChangeReason::STATE_CHANGE_REASON_MOUSE:
            return "MOUSE";
        case StateChangeReason::STATE_CHANGE_REASON_DOUBLE_CLICK:
            return "DOUBLE_CLICK";
        case StateChangeReason::STATE_CHANGE_REASON_SWITCH:
            return "SWITCH";
        case StateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT:
            return "PRE_BRIGHT";
        case StateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_SUCCESS:
            return "PRE_BRIGHT_ATUH_SUCCESS";
        case StateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_FAIL_SCREEN_ON:
            return "PRE_BRIGHT_ATUH_FAIL_SCREEN_ON";
        case StateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_FAIL_SCREEN_OFF:
            return "PRE_BRIGHT_ATUH_FAIL_SCREEN_OFF";
        case StateChangeReason::STATE_CHANGE_REASON_AOD_SLIDING:
            return "AOD_SLIDING";
        case StateChangeReason::STATE_CHANGE_REASON_REMOTE:
            return "REMOTE";
        case StateChangeReason::STATE_CHANGE_REASON_REFRESH:
            return "REFRESH";
        case StateChangeReason::STATE_CHANGE_REASON_COORDINATION:
            return "COORDINATION_OVERRIDE";
        case StateChangeReason::STATE_CHANGE_REASON_PROXIMITY:
            return "PROXIMITY";
        case StateChangeReason::STATE_CHANGE_REASON_INCOMING_CALL:
            return "INCOMING_CALL";
        case StateChangeReason::STATE_CHANGE_REASON_SHELL:
            return "SHELL";
        case StateChangeReason::STATE_CHANGE_REASON_TIMEOUT_NO_SCREEN_LOCK:
            return "TIMEOUT_NO_SCREEN_LOCK";
        case StateChangeReason::STATE_CHANGE_REASON_UNKNOWN:
            return "UNKNOWN";
        default:
            break;
    }
    return "UNKNOWN";
}

const char* PowerUtils::GetPowerStateString(PowerState state)
{
    switch (state) {
        case PowerState::AWAKE:
            return "AWAKE";
        case PowerState::FREEZE:
            return "FREEZE";
        case PowerState::INACTIVE:
            return "INACTIVE";
        case PowerState::STAND_BY:
            return "STAND_BY";
        case PowerState::DOZE:
            return "DOZE";
        case PowerState::SLEEP:
            return "SLEEP";
        case PowerState::HIBERNATE:
            return "HIBERNATE";
        case PowerState::SHUTDOWN:
            return "SHUTDOWN";
        case PowerState::DIM:
            return "DIM";
        case PowerState::UNKNOWN:
            return "UNKNOWN";
        default:
            break;
    }
    return "UNKNOWN";
}

const char* PowerUtils::GetDisplayStateString(DisplayState state)
{
    switch (state) {
        case DisplayState::DISPLAY_OFF:
            return "DISPLAY_OFF";
        case DisplayState::DISPLAY_DIM:
            return "DISPLAY_DIM";
        case DisplayState::DISPLAY_ON:
            return "DISPLAY_ON";
        case DisplayState::DISPLAY_SUSPEND:
            return "DISPLAY_SUSPEND";
        case DisplayState::DISPLAY_UNKNOWN:
            return "DISPLAY_UNKNOWN";
        default:
            break;
    }
    return "DISPLAY_UNKNOWN";
}

const char* PowerUtils::GetRunningLockTypeString(RunningLockType type)
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

const char* PowerUtils::GetSuspendDeviceTypeString(SuspendDeviceType type);
{
    switch (type) {
        case SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION:
            return "APPLICATION";
        case SuspendDeviceType::SUSPEND_DEVICE_REASON_DEVICE_ADMIN:
            return "DEVICE_ADMIN";
        case SuspendDeviceType::SUSPEND_DEVICE_REASON_TIMEOUT:
            return "TIMEOUT";
        case SuspendDeviceType::SUSPEND_DEVICE_REASON_LID:
            return "LID";
        case SuspendDeviceType::SUSPEND_DEVICE_REASON_POWER_KEY:
            return "POWER_KEY";
        case SuspendDeviceType::SUSPEND_DEVICE_REASON_HDMI:
            return "HDMI";
        case SuspendDeviceType::SUSPEND_DEVICE_REASON_SLEEP_KEY:
            return "SLEEP_KEY";
        case SuspendDeviceType::SUSPEND_DEVICE_REASON_ACCESSIBILITY:
            return "ACCESSIBILITY";
        case SuspendDeviceType::SUSPEND_DEVICE_REASON_FORCE_SUSPEND:
            return "FORCE_SUSPEND";
        case SuspendDeviceType::SUSPEND_DEVICE_REASON_STR:
            return "STR";
        case SuspendDeviceType::SUSPEND_DEVICE_REASON_SWITCH:
            return "SWITCH";
#ifdef POWER_MANAGER_WAKEUP_ACTION
        case SuspendDeviceType::SUSPEND_DEVICE_REASON_LOW_CAPACITY:
            return "LOW_CAPACITY";
#endif
        default:
            break;
    }
    return "UNKNOWN";
}

const char* PowerUtils::GetWakeupDeviceTypeString(WakeupDeviceType type);
{
    switch (type) {
        case WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN:
            return "UNKNOWN";
        case WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON:
            return "POWER_BUTTON";
        case WakeupDeviceType::WAKEUP_DEVICE_APPLICATION:
            return "APPLICATION";
        case WakeupDeviceType::WAKEUP_DEVICE_PLUGGED_IN:
            return "PLUGGED_IN";
        case WakeupDeviceType::WAKEUP_DEVICE_GESTURE:
            return "GESTURE";
        case WakeupDeviceType::WAKEUP_DEVICE_CAMERA_LAUNCH:
            return "CAMERA_LAUNCH";
        case WakeupDeviceType::WAKEUP_DEVICE_WAKE_KEY:
            return "WAKE_KEY";
        case WakeupDeviceType::WAKEUP_DEVICE_WAKE_MOTION:
            return "WAKE_MOTION";
        case WakeupDeviceType::WAKEUP_DEVICE_HDMI:
            return "HDMI";
        case WakeupDeviceType::WAKEUP_DEVICE_LID:
            return "LID";
        case WakeupDeviceType::WAKEUP_DEVICE_DOUBLE_CLICK:
            return "DOUBLE_CLICK";
        case WakeupDeviceType::WAKEUP_DEVICE_KEYBOARD:
            return "KEYBOARD";
        case WakeupDeviceType::WAKEUP_DEVICE_MOUSE:
            return "MOUSE";
        case WakeupDeviceType::WAKEUP_DEVICE_TOUCHPAD:
            return "TOUCHPAD";
        case WakeupDeviceType::WAKEUP_DEVICE_PEN:
            return "PEN";
        case WakeupDeviceType::WAKEUP_DEVICE_TOUCH_SCREEN:
            return "TOUCH_SCREEN";
        case WakeupDeviceType::WAKEUP_DEVICE_SWITCH:
            return "SWITCH";
        case WakeupDeviceType::WAKEUP_DEVICE_SINGLE_CLICK:
            return "SINGLE_CLICK";
        case WakeupDeviceType::WAKEUP_DEVICE_PRE_BRIGHT:
            return "PRE_BRIGHT";
        case WakeupDeviceType::WAKEUP_DEVICE_PRE_BRIGHT_AUTH_SUCCESS:
            return "PRE_BRIGHT_AUTH_SUCCESS";
        case WakeupDeviceType::WAKEUP_DEVICE_PRE_BRIGHT_AUTH_FAIL_SCREEN_ON:
            return "PRE_BRIGHT_AUTH_FAIL_SCREEN_ON";
        case WakeupDeviceType::WAKEUP_DEVICE_PRE_BRIGHT_AUTH_FAIL_SCREEN_OFF:
            return "PRE_BRIGHT_AUTH_FAIL_SCREEN_OFF";
        case WakeupDeviceType::WAKEUP_DEVICE_AOD_SLIDING:
            return "AOD_SLIDING";
        case WakeupDeviceType::WAKEUP_DEVICE_INCOMING_CALL:
            return "INCOMING_CALL";
        case WakeupDeviceType::WAKEUP_DEVICE_SHELL:
            return "SHELL";
        default:
            break;
    }
}
} // namespace PowerMgr
} // namespace OHOS
