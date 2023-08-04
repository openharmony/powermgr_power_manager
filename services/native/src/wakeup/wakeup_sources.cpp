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

#include "wakeup_sources.h"
#include "power_log.h"
#include <string>
namespace OHOS {
namespace PowerMgr {
std::mutex WakeupSources::sourceKeysMutex_;

WakeupDeviceType WakeupSources::mapWakeupDeviceType(const std::string& key, uint32_t click)
{
    if (key == WakeupSources::POWERKEY_KEY) {
        return WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON;
    }

    if (key == WakeupSources::MOUSE_KEY) {
        return WakeupDeviceType::WAKEUP_DEVICE_MOUSE;
    }

    if (key == WakeupSources::KEYBOARD_KEY) {
        return WakeupDeviceType::WAKEUP_DEVICE_KEYBOARD;
    }

    if (key == WakeupSources::PEN_KEY) {
        return WakeupDeviceType::WAKEUP_DEVICE_PEN;
    }

    if (key == WakeupSources::TOUCHPAD_KEY) {
        return WakeupDeviceType::WAKEUP_DEVICE_TOUCHPAD;
    }

    if (key == WakeupSources::LID_KEY) {
        return WakeupDeviceType::WAKEUP_DEVICE_LID;
    }

    if (key == WakeupSources::SWITCH_KEY) {
        return WakeupDeviceType::WAKEUP_DEVICE_SWITCH;
    }

    if (key == WakeupSources::TOUCHSCREEN_KEY) {
        if (click == WakeupSources::SINGLE_CLICK) {
            return WakeupDeviceType::WAKEUP_DEVICE_SINGLE_CLICK;
        }
        return WakeupDeviceType::WAKEUP_DEVICE_DOUBLE_CLICK;
    }

    return WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN;
}

std::vector<std::string> WakeupSources::getSourceKeys()
{
    std::lock_guard<std::mutex> lock(sourceKeysMutex_);
    std::vector<std::string> sourceKeys;
    sourceKeys.push_back(WakeupSources::POWERKEY_KEY);
    sourceKeys.push_back(WakeupSources::MOUSE_KEY);
    sourceKeys.push_back(WakeupSources::KEYBOARD_KEY);
    sourceKeys.push_back(WakeupSources::TOUCHSCREEN_KEY);
    sourceKeys.push_back(WakeupSources::TOUCHPAD_KEY);
    sourceKeys.push_back(WakeupSources::PEN_KEY);
    sourceKeys.push_back(WakeupSources::LID_KEY);
    sourceKeys.push_back(WakeupSources::SWITCH_KEY);
    return sourceKeys;
}

void WakeupSources::PutSource(WakeupSource& source)
{
    std::lock_guard<std::mutex> lock(sourceListMutex_);
    sourceList_.push_back(source);
}

std::vector<WakeupSource> WakeupSources::GetSourceList()
{
    std::lock_guard<std::mutex> lock(sourceListMutex_);
    return sourceList_;
}

} // namespace PowerMgr
} // namespace OHOS