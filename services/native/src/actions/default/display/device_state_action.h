/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include "display_power_callback_stub.h"

namespace OHOS {
namespace PowerMgr {
class DeviceStateAction : public IDeviceStateAction {
public:
    DeviceStateAction();
    ~DeviceStateAction();
    void Suspend(int64_t callTimeMs, SuspendDeviceType type, uint32_t flags) override;
    void ForceSuspend() override;
    void Wakeup(int64_t callTimeMs, WakeupDeviceType type, const std::string& details,
        const std::string& pkgName) override;
    void RefreshActivity(const int64_t callTimeMs, UserActivityType type,
        const uint32_t flags) override {}
    DisplayState GetDisplayState() override;
    uint32_t SetDisplayState(const DisplayState state,
        StateChangeReason reason = StateChangeReason::STATE_CHANGE_REASON_UNKNOWN) override;
    uint32_t GoToSleep(std::function<void()> onSuspend, std::function<void()> onWakeup, bool force) override;
    void RegisterCallback(std::function<void(uint32_t)>& callback) override;

private:
    static constexpr const char * const LOCK_TAG_DISPLAY_POWER = "display_power_lock";
    class DisplayPowerCallback : public DisplayPowerMgr::DisplayPowerCallbackStub {
        friend DeviceStateAction;
    public:
        void OnDisplayStateChanged(uint32_t displayId, DisplayPowerMgr::DisplayState state) override;
    private:
        void NotifyDisplayActionDone(uint32_t event);
        std::function<void(uint32_t)> notify_ {nullptr};
    };
    bool isRegister_ {false};
    sptr<DisplayPowerCallback> dispCallback_ {nullptr};
    std::function<void(uint32_t)> actionCallback_ {nullptr};
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_DEVICE_STATE_ACTION_H
