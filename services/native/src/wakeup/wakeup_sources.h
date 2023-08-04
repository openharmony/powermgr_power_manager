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

#ifndef POWERMGR_WAKEUP_SOURCES_H
#define POWERMGR_WAKEUP_SOURCES_H

#include "power_state_machine_info.h"
#include <cstdint>
#include <string>
#include <vector>
#include <mutex>

namespace OHOS {
namespace PowerMgr {
enum class WakeUpAction : uint32_t { CLICK_SINGLE = 1, CLICK_DOUBLE = 2 };

class WakeupSource {
public:
    static const constexpr char* ENABLE_KEY = "enable";
    static const constexpr char* KEYS_KEY = "click";

    WakeupSource(WakeupDeviceType reason, bool enable, uint32_t click) : reason_(reason), enable_(enable), click_(click)
    {
    }
    ~WakeupSource() = default;

    WakeupDeviceType GetReason() const
    {
        return reason_;
    }

    bool IsEnable() const
    {
        return enable_;
    }

    uint32_t GetClick() const
    {
        return click_;
    }

private:
    WakeupDeviceType reason_;
    bool enable_;
    uint32_t click_;
};

class WakeupSources {
public:
    static const constexpr char* POWERKEY_KEY = "powerkey";
    static const constexpr char* MOUSE_KEY = "mouse";
    static const constexpr char* KEYBOARD_KEY = "keyborad";
    static const constexpr char* TOUCHSCREEN_KEY = "touchscreen";
    static const constexpr char* TOUCHPAD_KEY = "touchpad";
    static const constexpr char* PEN_KEY = "pen";
    static const constexpr char* LID_KEY = "lid";
    static const constexpr char* SWITCH_KEY = "switch";
    static const constexpr uint32_t SINGLE_CLICK = 1;
    static const constexpr uint32_t DOUBLC_CLICK = 2;

    WakeupSources() = default;
    ~WakeupSources() = default;
    static WakeupDeviceType mapWakeupDeviceType(const std::string& key, uint32_t click);
    static std::vector<std::string> getSourceKeys();
    void PutSource(WakeupSource& source);
    std::vector<WakeupSource> GetSourceList();

private:
    std::vector<WakeupSource> sourceList_;
    std::mutex sourceListMutex_;
    static std::mutex sourceKeysMutex_;
};

} // namespace PowerMgr
} // namespace OHOS

#endif // POWERMGR_SUSPEND_SOURCES_H