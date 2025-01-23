/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef POWERMGR_POWER_STATE_MACHINE_INFO_H
#define POWERMGR_POWER_STATE_MACHINE_INFO_H

#include <string>

#include <parcel.h>

namespace OHOS {
namespace PowerMgr {
/*
 * If already dimmed, extend the dim timeout but do not brighten. This flag is useful for keeping
 * the screen on little longer without causing a visible change such as when the power key is pressed.
 */
constexpr uint32_t REFRESH_ACTIVITY_NO_CHANGE_LIGHTS = 1 << 0;
constexpr uint32_t REFRESH_ACTIVITY_NEED_CHANGE_LIGHTS = 0;

/*
 * Indicate whether device enter suspend immediately
 */
constexpr uint32_t SUSPEND_DEVICE_NEED_DOZE = 0;
constexpr uint32_t SUSPEND_DEVICE_IMMEDIATELY = 1 << 0;

// This is use for judge whether the time is valid.
constexpr int DEFAULT_SYSTEM_START_TIME = 0;

// Max datails string length
constexpr int MAX_STRING_LENGTH = 128;

// Throttling interval for user activity calls.
constexpr int64_t MIN_TIME_MS_BETWEEN_USERACTIVITIES = 100; // 100ms

// Throttling interval for multimode activity calls.
constexpr int64_t MIN_TIME_MS_BETWEEN_MULTIMODEACTIVITIES = 100; // 100ms

// the reason for shut down device fast
constexpr const char* const SHUTDOWN_FAST_REASON = "STR";

/**
 * PowerState of Device.
 */
enum class PowerState : uint32_t {
    /**
     * Power State: screen on and cpu on.
     */
    AWAKE = 0,

    /**
     * Power State: screen on or active and some background user processes are frozen.
     */
    FREEZE,

    /**
     * Power State: screen off and cpu on.
     */
    INACTIVE,

    /**
     * Power State: screen off and cpu on and some user processes are frozen.
     */
    STAND_BY,

    /**
     * Power State: screen off and cpu on and most user processes are frozen.
     */
    DOZE,

    /**
     * Power State: screen off and cpu off and suspend to the ram.
     */
    SLEEP,

    /**
     * Power State: screen off and cpu off and suspend to the disk.
     */
    HIBERNATE,

    /**
     * Power State: shutdown.
     */
    SHUTDOWN,

    /**
     * Power State: screen on and display dim
     */
    DIM,

    /**
     * Power State: unknown.
     */
    UNKNOWN,
};

/**
 * Display State of Device.
 */
enum class DisplayState : uint32_t {
    DISPLAY_OFF = 0,
    DISPLAY_DIM = 1,
    DISPLAY_ON = 2,
    DISPLAY_SUSPEND = 3,
    DISPLAY_DOZE,
    DISPLAY_DOZE_SUSPEND,
    DISPLAY_UNKNOWN,
};

// UserActivityType list, must sync with A_PMS
enum class UserActivityType : uint32_t {
    USER_ACTIVITY_TYPE_OTHER = 0,
    USER_ACTIVITY_TYPE_BUTTON = 1,
    USER_ACTIVITY_TYPE_TOUCH = 2,
    USER_ACTIVITY_TYPE_ACCESSIBILITY = 3,
    USER_ACTIVITY_TYPE_ATTENTION = 4,
    USER_ACTIVITY_TYPE_SOFTWARE = 5,
    USER_ACTIVITY_TYPE_SWITCH = 6,
    USER_ACTIVITY_TYPE_CABLE = 7,
    USER_ACTIVITY_TYPE_MAX = USER_ACTIVITY_TYPE_CABLE,
};

// WakeupReasonType list
enum class WakeupDeviceType : uint32_t {
    WAKEUP_DEVICE_UNKNOWN = 0,
    WAKEUP_DEVICE_POWER_BUTTON = 1,
    WAKEUP_DEVICE_APPLICATION = 2,
    WAKEUP_DEVICE_PLUGGED_IN = 3,
    WAKEUP_DEVICE_GESTURE = 4,
    WAKEUP_DEVICE_CAMERA_LAUNCH = 5,
    WAKEUP_DEVICE_WAKE_KEY = 6,
    WAKEUP_DEVICE_WAKE_MOTION = 7,
    WAKEUP_DEVICE_HDMI = 8,
    WAKEUP_DEVICE_LID = 9,
    WAKEUP_DEVICE_DOUBLE_CLICK = 10,
    WAKEUP_DEVICE_KEYBOARD = 11,
    WAKEUP_DEVICE_MOUSE = 12,
    WAKEUP_DEVICE_TOUCHPAD = 13,
    WAKEUP_DEVICE_PEN = 14,
    WAKEUP_DEVICE_TOUCH_SCREEN = 15,
    WAKEUP_DEVICE_SWITCH = 16,
    WAKEUP_DEVICE_SINGLE_CLICK = 17,
    WAKEUP_DEVICE_PRE_BRIGHT = 18,
    WAKEUP_DEVICE_PRE_BRIGHT_AUTH_SUCCESS = 19,
    WAKEUP_DEVICE_PRE_BRIGHT_AUTH_FAIL_SCREEN_ON = 20,
    WAKEUP_DEVICE_PRE_BRIGHT_AUTH_FAIL_SCREEN_OFF = 21,
    WAKEUP_DEVICE_AOD_SLIDING = 22,
    WAKEUP_DEVICE_INCOMING_CALL = 23,
    WAKEUP_DEVICE_SHELL = 24,
    WAKEUP_DEVICE_PICKUP = 25,
    WAKEUP_DEVICE_EXIT_SYSTEM_STR = 26, // STR: suspend to ram
    WAKEUP_DEVICE_SCREEN_CONNECT = 27,
    WAKEUP_DEVICE_TP_TOUCH = 28,
    WAKEUP_DEVICE_EX_SCREEN_INIT = 29,
    WAKEUP_DEVICE_ABNORMAL_SCREEN_CONNECT = 30,
    WAKEUP_DEVICE_MAX
};

// SuspendDeviceType list
enum class SuspendDeviceType : uint32_t {
    SUSPEND_DEVICE_REASON_MIN = 0,
    SUSPEND_DEVICE_REASON_APPLICATION = SUSPEND_DEVICE_REASON_MIN,
    SUSPEND_DEVICE_REASON_DEVICE_ADMIN = 1,
    SUSPEND_DEVICE_REASON_TIMEOUT = 2,
    SUSPEND_DEVICE_REASON_LID = 3,
    SUSPEND_DEVICE_REASON_POWER_KEY = 4,
    SUSPEND_DEVICE_REASON_HDMI = 5,
    SUSPEND_DEVICE_REASON_SLEEP_KEY = 6,
    SUSPEND_DEVICE_REASON_ACCESSIBILITY = 7,
    SUSPEND_DEVICE_REASON_FORCE_SUSPEND = 8,
    SUSPEND_DEVICE_REASON_STR = 9,
    SUSPEND_DEVICE_REASON_SWITCH = 10,
    SUSPEND_DEVICE_LOW_CAPACITY = 11,
    SUSPEND_DEVICE_REASON_TP_COVER = 12,
    SUSPEND_DEVICE_REASON_EX_SCREEN_INIT = 13,
    SUSPEND_DEVICE_REASON_MAX
};

enum class StateChangeReason : uint32_t {
    STATE_CHANGE_REASON_INIT = 0,
    STATE_CHANGE_REASON_TIMEOUT = 1,
    STATE_CHANGE_REASON_RUNNING_LOCK = 2,
    STATE_CHANGE_REASON_BATTERY = 3,
    STATE_CHANGE_REASON_THERMAL = 4,
    STATE_CHANGE_REASON_WORK = 5,
    STATE_CHANGE_REASON_SYSTEM = 6,
    STATE_CHANGE_REASON_APPLICATION = 10,
    STATE_CHANGE_REASON_SETTINGS = 11,
    STATE_CHANGE_REASON_HARD_KEY = 12,
    STATE_CHANGE_REASON_TOUCH = 13,
    STATE_CHANGE_REASON_CABLE = 14,
    STATE_CHANGE_REASON_SENSOR = 15,
    STATE_CHANGE_REASON_LID = 16,
    STATE_CHANGE_REASON_CAMERA = 17,
    STATE_CHANGE_REASON_ACCESSIBILITY = 18,
    STATE_CHANGE_REASON_RESET = 19,
    STATE_CHANGE_REASON_POWER_KEY = 20,
    STATE_CHANGE_REASON_KEYBOARD = 21,
    STATE_CHANGE_REASON_MOUSE = 22,
    STATE_CHANGE_REASON_DOUBLE_CLICK = 23,
    STATE_CHANGE_REASON_SWITCH = 25,
    STATE_CHANGE_REASON_PRE_BRIGHT = 26,
    STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_SUCCESS = 27,
    STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_FAIL_SCREEN_ON = 28,
    STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_FAIL_SCREEN_OFF = 29,
    STATE_CHANGE_REASON_REFRESH = 30,
    STATE_CHANGE_REASON_COORDINATION = 31,
    STATE_CHANGE_REASON_PROXIMITY = 32,
    STATE_CHANGE_REASON_INCOMING_CALL = 33,
    STATE_CHANGE_REASON_SHELL = 34,
    STATE_CHANGE_REASON_PICKUP = 35,
	STATE_CHANGE_REASON_AOD_SLIDING = 40,
    STATE_CHANGE_REASON_PEN = 41,
    STATE_CHANGE_REASON_SHUT_DOWN = 42,
    STATE_CHANGE_REASON_SCREEN_CONNECT = 43,
    STATE_CHANGE_REASON_SWITCHING_DOZE_MODE = 44,
    STATE_CHANGE_REASON_HIBERNATE = 45,
    STATE_CHANGE_REASON_EX_SCREEN_INIT = 46,
    STATE_CHANGE_REASON_ABNORMAL_SCREEN_CONNECT = 47,
    STATE_CHANGE_REASON_REMOTE = 100,
    STATE_CHANGE_REASON_TIMEOUT_NO_SCREEN_LOCK = 101,
    STATE_CHANGE_REASON_EXIT_SYSTEM_STR = 102,
    STATE_CHANGE_REASON_TP_TOUCH = 103,
    STATE_CHANGE_REASON_TP_COVER = 104,
    STATE_CHANGE_REASON_UNKNOWN = 1000,
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_POWER_STATE_MACHINE_INFO_H
