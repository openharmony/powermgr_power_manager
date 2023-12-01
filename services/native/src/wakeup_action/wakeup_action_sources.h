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

#ifndef POWERMGR_WAKEUP_ACTION_SOURCES_H
#define POWERMGR_WAKEUP_ACTION_SOURCES_H

#include <cstdint>
#include <map>
#include <mutex>

#include "power_state_machine_info.h"

namespace OHOS {
namespace PowerMgr {
enum class WakeupAction : uint32_t {
    ACTION_NONE = 0,      //  no action
    ACTION_HIBERNATE,     //  entry hibernate
    ACTION_SHUTDOWN,      //  shutdown
    ACTION_INVALID
};

class WakeupActionSource {
public:
    static const constexpr char* SCENE_KEY = "scene";
    static const constexpr char* ACTION_KEY = "action";

    explicit WakeupActionSource(std::string scene, uint32_t action)
    {
        scene_ = scene;
        action_ = action;
    }
    ~WakeupActionSource() = default;
    std::string GetScene() const
    {
        return scene_;
    }
    uint32_t GetAction() const
    {
        return action_;
    }

private:
    std::string scene_;
    uint32_t action_;
};

class WakeupActionSources {
public:
    static const constexpr char* LOW_CAPACITY_KEY = "53";
    WakeupActionSources() = default;
    ~WakeupActionSources() = default;
    static SuspendDeviceType mapSuspendDeviceType(const std::string& reason);
    void PutSource(const std::string& key, std::shared_ptr<WakeupActionSource>& source);
    std::map<std::string, std::shared_ptr<WakeupActionSource>> GetSourceMap();

private:
    std::map<std::string, std::shared_ptr<WakeupActionSource>>  sourceMap_;
    std::mutex sourceMapMutex_;
};

} // namespace PowerMgr
} // namespace OHOS

#endif // POWERMGR_WAKEUP_ACTION_SOURCES_H