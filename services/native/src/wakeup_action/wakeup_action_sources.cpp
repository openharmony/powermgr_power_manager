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

#include "wakeup_action_sources.h"

#include <string>
#include "power_log.h"

namespace OHOS {
namespace PowerMgr {

SuspendDeviceType WakeupActionSources::mapSuspendDeviceType(const std::string& reason)
{
    if (reason == WakeupActionSources::LOW_CAPACITY_KEY) {
        return SuspendDeviceType::SUSPEND_DEVICE_LOW_CAPACITY;
    }

    return SuspendDeviceType::SUSPEND_DEVICE_REASON_MIN;
}

void WakeupActionSources::PutSource(const std::string& key, std::shared_ptr<WakeupActionSource>& source)
{
    std::lock_guard<std::mutex> lock(sourceMapMutex_);
    sourceMap_.emplace(key, source);
}

std::map<std::string, std::shared_ptr<WakeupActionSource>> WakeupActionSources::GetSourceMap()
{
    std::lock_guard<std::mutex> lock(sourceMapMutex_);
    return sourceMap_;
}

} // namespace PowerMgr
} // namespace OHOS