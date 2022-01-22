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

#include "device_state_action.h"

#include "display_manager.h"
#include "display_power_mgr_client.h"
#include "system_suspend_controller.h"
#include "power_state_machine_info.h"
#include "hilog_wrapper.h"

using namespace std;

namespace OHOS {
namespace PowerMgr {
using namespace DisplayPowerMgr;
using namespace Rosen;

DeviceStateAction::DeviceStateAction()
{
    dispCallback_ = nullptr;
}

DeviceStateAction::~DeviceStateAction()
{
}

void DeviceStateAction::Suspend(int64_t callTimeMs, SuspendDeviceType type, uint32_t flags)
{
    // Display is controlled by PowerStateMachine
    // Don't suspend until GoToSleep is called
    DisplayManager::GetInstance().SuspendBegin(PowerStateChangeReason::POWER_BUTTON);
}

void DeviceStateAction::ForceSuspend()
{
    GoToSleep(nullptr, nullptr, true);
}

void DeviceStateAction::Wakeup(int64_t callTimeMs, WakeupDeviceType type, const string& details,
    const string& pkgName)
{
    SystemSuspendController::GetInstance().Wakeup();
}

DisplayState DeviceStateAction::GetDisplayState()
{
    DisplayPowerMgr::DisplayState state = DisplayPowerMgrClient::GetInstance().GetDisplayState();
    DisplayState ret = DisplayState::DISPLAY_ON;
    switch (state) {
        case DisplayPowerMgr::DisplayState::DISPLAY_ON:
            ret = DisplayState::DISPLAY_ON;
            break;
        case DisplayPowerMgr::DisplayState::DISPLAY_DIM:
            ret = DisplayState::DISPLAY_DIM;
            break;
        case DisplayPowerMgr::DisplayState::DISPLAY_OFF:
            ret = DisplayState::DISPLAY_OFF;
            break;
        case DisplayPowerMgr::DisplayState::DISPLAY_SUSPEND:
            ret = DisplayState::DISPLAY_SUSPEND;
            break;
        default:
            break;
    }
    return ret;
}

uint32_t DeviceStateAction::SetDisplayState(const DisplayState state, StateChangeReason reason)
{
    POWER_HILOGI(MODULE_SERVICE, "Action: SetDisplayState: %{public}d, %{public}d",
        static_cast<uint32_t>(state), static_cast<uint32_t>(reason));

    if (state == GetDisplayState()) {
        POWER_HILOGI(MODULE_SERVICE, "Already ins state: %{public}d", static_cast<uint32_t>(state));
        return ActionResult::SUCCESS;
    }

    if (dispCallback_ == nullptr) {
        POWER_HILOGI(MODULE_SERVICE, "Register Callback");
        dispCallback_ = new DisplayPowerCallback();
        DisplayPowerMgrClient::GetInstance().RegisterCallback(dispCallback_);
    }

    DisplayPowerMgr::DisplayState dispState = DisplayPowerMgr::DisplayState::DISPLAY_ON;
    switch (state) {
        case DisplayState::DISPLAY_ON:
            dispState = DisplayPowerMgr::DisplayState::DISPLAY_ON;
            DisplayManager::GetInstance().WakeUpBegin(PowerStateChangeReason::POWER_BUTTON);
            break;
        case DisplayState::DISPLAY_DIM:
            dispState = DisplayPowerMgr::DisplayState::DISPLAY_DIM;
            break;
        case DisplayState::DISPLAY_OFF:
            dispState = DisplayPowerMgr::DisplayState::DISPLAY_OFF;
            DisplayManager::GetInstance().SuspendBegin(PowerStateChangeReason::POWER_BUTTON);
            break;
        case DisplayState::DISPLAY_SUSPEND:
            dispState = DisplayPowerMgr::DisplayState::DISPLAY_SUSPEND;
            break;
        default:
            break;
    }

    bool ret = DisplayPowerMgrClient::GetInstance().SetDisplayState(dispState, reason);
    return ret ? ActionResult::SUCCESS : ActionResult::FAILED;
}

uint32_t DeviceStateAction::GoToSleep(const std::function<void()> onSuspend,
    const std::function<void()> onWakeup, bool force)
{
    SystemSuspendController::GetInstance().Suspend(onSuspend, onWakeup, force);
    return ActionResult::SUCCESS;
}

void DeviceStateAction::DisplayPowerCallback::OnDisplayStateChanged(uint32_t displayId,
    DisplayPowerMgr::DisplayState state)
{
    POWER_HILOGI(MODULE_SERVICE, "Callback: OnDisplayStateChanged");
    int32_t mainDisp = DisplayPowerMgrClient::GetInstance().GetMainDisplayId();
    if (mainDisp < 0 || static_cast<uint32_t>(mainDisp) != displayId) {
        POWER_HILOGI(MODULE_SERVICE, "It's not main display, skip!");
        return;
    }
    switch (state)
    {
    case DisplayPowerMgr::DisplayState::DISPLAY_ON:
        DisplayManager::GetInstance().WakeUpEnd();
        break;
    case DisplayPowerMgr::DisplayState::DISPLAY_OFF:
        DisplayManager::GetInstance().SuspendEnd();
        break;
    default:
        break;
    }
    return;
}
} // namespace PowerMgr
} // namespace OHOS
