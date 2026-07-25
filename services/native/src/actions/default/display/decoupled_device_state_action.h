/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef POWERMGR_DECOUPLED_DEVICE_STATE_ACTION_H
#define POWERMGR_DECOUPLED_DEVICE_STATE_ACTION_H

#include "actions/idevice_state_action.h"

namespace OHOS {
namespace PowerMgr {
class DecoupledDeviceStateAction : public IDeviceStateAction {
public:
    DecoupledDeviceStateAction() = default;
    ~DecoupledDeviceStateAction() override = default;

    void Suspend(int64_t callTimeMs, SuspendDeviceType type, uint32_t flags) override {};
    void ForceSuspend() override;
    void Wakeup(int64_t callTimeMs, WakeupDeviceType type, const std::string& details,
        const std::string& pkgName) override;
    uint32_t GoToSleep(std::function<void()> onSuspend, std::function<void()> onWakeup, bool force) override;
    void RefreshActivity(const int64_t callTimeMs, UserActivityType type,
        const uint32_t flags) override {}

    // display-control methods are no-op (screen is managed by external product)
    DisplayState GetDisplayState() override;
    uint32_t SetDisplayState(DisplayState state,
        StateChangeReason reason = StateChangeReason::STATE_CHANGE_REASON_UNKNOWN) override;
    void SetInternalScreenDisplayPower(DisplayState state, StateChangeReason reason) override {};
    void SetInternalScreenBrightness() override {};
    bool TryToCancelScreenOff() override;
    void BeginPowerkeyScreenOff() override {};
    void EndPowerkeyScreenOff() override {};
    void SetCoordinated(bool coordinated) override {};
    void RegisterCallback(std::function<void(uint32_t)>& callback) override {};
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_DECOUPLED_DEVICE_STATE_ACTION_H
