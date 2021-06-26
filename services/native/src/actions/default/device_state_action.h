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

#ifndef POWERMGR_DEVICE_STATE_ACTION_H
#define POWERMGR_DEVICE_STATE_ACTION_H

#include "actions/idevice_state_action.h"

namespace OHOS {
namespace PowerMgr {
class DeviceStateAction : public IDeviceStateAction {
public:
    void Suspend(int64_t callTimeMs, SuspendDeviceType type, uint32_t flags) override;
    void ForceSuspend() override;
    void Wakeup(int64_t callTimeMs, WakeupDeviceType type, const std::string& details,
        const std::string& pkgName) override;
   void RefreshActivity(int64_t callTimeMs __attribute__((__unused__)),
        UserActivityType type __attribute__((__unused__)),
        uint32_t flags __attribute__((__unused__))) override {}
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_DEVICE_STATE_ACTION_H
