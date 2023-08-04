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

#include "suspend_sources.h"
#include "power_log.h"
#include <string>
namespace OHOS {
namespace PowerMgr {
std::mutex SuspendSources::sourceKeysMutex_;

SuspendDeviceType SuspendSources::mapSuspendDeviceType(const std::string& key)
{
    if (key == SuspendSources::POWERKEY_KEY) {
        return SuspendDeviceType::SUSPEND_DEVICE_REASON_POWER_KEY;
    }

    if (key == SuspendSources::TIMEOUT_KEY) {
        return SuspendDeviceType::SUSPEND_DEVICE_REASON_TIMEOUT;
    }

    if (key == SuspendSources::LID_KEY) {
        return SuspendDeviceType::SUSPEND_DEVICE_REASON_LID;
    }

    if (key == SuspendSources::SWITCH_KEY) {
        return SuspendDeviceType::SUSPEND_DEVICE_REASON_SWITCH;
    }

    return SuspendDeviceType::SUSPEND_DEVICE_REASON_MIN;
}

std::vector<std::string> SuspendSources::getSourceKeys()
{
    std::lock_guard<std::mutex> lock(sourceKeysMutex_);
    std::vector<std::string> sourceKeys;
    sourceKeys.push_back(SuspendSources::POWERKEY_KEY);
    sourceKeys.push_back(SuspendSources::TIMEOUT_KEY);
    sourceKeys.push_back(SuspendSources::LID_KEY);
    sourceKeys.push_back(SuspendSources::SWITCH_KEY);
    return sourceKeys;
}

void SuspendSources::PutSource(SuspendSource& source)
{
    std::lock_guard<std::mutex> lock(sourceListMutex_);
    sourceList_.push_back(source);
}

std::vector<SuspendSource> SuspendSources::GetSourceList()
{
    std::lock_guard<std::mutex> lock(sourceListMutex_);
    return sourceList_;
}

} // namespace PowerMgr
} // namespace OHOS