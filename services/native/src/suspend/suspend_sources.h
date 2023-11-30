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

#ifndef POWERMGR_SUSPEND_SOURCES_H
#define POWERMGR_SUSPEND_SOURCES_H

#include <cstdint>
#include <vector>
#include <mutex>

#include "power_state_machine_info.h"

namespace OHOS {
namespace PowerMgr {
enum class SuspendAction : uint32_t {
    ACTION_NONE = 0,      //  no action
    ACTION_AUTO_SUSPEND,  //  automatically enter sleep
    ACTION_FORCE_SUSPEND, //  force enter sleep
    ACTION_HIBERNATE,     //  entry hibernate
    ACTION_SHUTDOWN,      //  shutdown
    ACTION_INVALID,
    ACTION_MAC = ACTION_INVALID
};

class SuspendSource {
public:
    static const constexpr char* ACTION_KEY = "action";
    static const constexpr char* DELAY_KEY = "delayMs";

    SuspendSource(SuspendDeviceType reason, uint32_t action, uint32_t delay)
    {
        reason_ = reason;
        action_ = action;
        delayMs_ = delay;
    }
    ~SuspendSource() = default;
    SuspendDeviceType GetReason() const
    {
        return reason_;
    }
    uint32_t GetAction() const
    {
        return action_;
    }
    uint32_t GetDelay() const
    {
        return delayMs_;
    }

private:
    SuspendDeviceType reason_;
    uint32_t action_;
    uint32_t delayMs_;
};

class SuspendSources {
public:
    static const constexpr char* POWERKEY_KEY = "powerkey";
    static const constexpr char* TIMEOUT_KEY = "timeout";
    static const constexpr char* LID_KEY = "lid";
    static const constexpr char* SWITCH_KEY = "switch";
    SuspendSources() = default;
    ~SuspendSources() = default;
    static SuspendDeviceType mapSuspendDeviceType(const std::string& key);
    static std::vector<std::string> getSourceKeys();
    void PutSource(SuspendSource& source);
    std::vector<SuspendSource> GetSourceList();

private:
    std::vector<SuspendSource> sourceList_;
    std::mutex sourceListMutex_;
    static std::mutex sourceKeysMutex_;
};

} // namespace PowerMgr
} // namespace OHOS

#endif // POWERMGR_SUSPEND_SOURCES_H