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

#ifndef POWERMGR_IDEVICE_STATE_ACTION_H
#define POWERMGR_IDEVICE_STATE_ACTION_H

#include <string>

#include "power_state_machine_info.h"

namespace OHOS {
namespace PowerMgr {
class IDeviceStateAction {
public:
    IDeviceStateAction() = default;
    virtual ~IDeviceStateAction() = default;

    virtual void Suspend(int64_t callTimeMs, SuspendDeviceType type, uint32_t flags) = 0;
    virtual void ForceSuspend() = 0;
    virtual void Wakeup(int64_t callTimeMs, WakeupDeviceType type, const std::string& details,
        const std::string& pkgName) = 0;
    virtual void RefreshActivity(int64_t callTimeMs, UserActivityType type, uint32_t flags) = 0;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_IDEVICE_STATE_ACTION_H
