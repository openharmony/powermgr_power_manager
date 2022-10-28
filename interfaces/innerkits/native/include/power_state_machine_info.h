/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

/**
 * PowerState of Device.
 */
enum class PowerState : uint32_t {
    /**
     * Power State: screen on and cpu on.
     */
    AWAKE = 0,

    /**
     * Power State: screen off and cpu on.
     */
    INACTIVE,

    /**
     * Power State: screen off and cpu off.
     */
    SLEEP,

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
    DISPLAY_UNKNOWN = 4,
};

// UserActivityType list, must sync with A_PMS
enum class UserActivityType : uint32_t {
    USER_ACTIVITY_TYPE_OTHER = 0,
    USER_ACTIVITY_TYPE_BUTTON = 1,
    USER_ACTIVITY_TYPE_TOUCH = 2,
    USER_ACTIVITY_TYPE_ACCESSIBILITY = 3,
    USER_ACTIVITY_TYPE_ATTENTION = 4,
    USER_ACTIVITY_TYPE_SOFTWARE = 5,
    USER_ACTIVITY_TYPE_MAX = USER_ACTIVITY_TYPE_SOFTWARE,
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
    WAKEUP_DEVICE_MAX = WAKEUP_DEVICE_MOUSE,
};

// SuspendDeviceType list
enum class SuspendDeviceType : uint32_t {
    SUSPEND_DEVICE_REASON_MIN = 0,
    SUSPEND_DEVICE_REASON_APPLICATION = SUSPEND_DEVICE_REASON_MIN,
    SUSPEND_DEVICE_REASON_DEVICE_ADMIN = 1,
    SUSPEND_DEVICE_REASON_TIMEOUT = 2,
    SUSPEND_DEVICE_REASON_LID_SWITCH = 3,
    SUSPEND_DEVICE_REASON_POWER_BUTTON = 4,
    SUSPEND_DEVICE_REASON_HDMI = 5,
    SUSPEND_DEVICE_REASON_SLEEP_BUTTON = 6,
    SUSPEND_DEVICE_REASON_ACCESSIBILITY = 7,
    SUSPEND_DEVICE_REASON_FORCE_SUSPEND = 8,
    SUSPEND_DEVICE_REASON_MAX = SUSPEND_DEVICE_REASON_FORCE_SUSPEND,
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
    STATE_CHANGE_REASON_REMOTE = 100,
    STATE_CHANGE_REASON_UNKNOWN = 1000,
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_POWER_STATE_MACHINE_INFO_H
