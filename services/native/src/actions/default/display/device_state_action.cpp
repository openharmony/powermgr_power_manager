/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include <ipc_skeleton.h>
#include "display_power_mgr_client.h"
#include "power_log.h"
#include "power_state_machine_info.h"
#include "system_suspend_controller.h"

using namespace std;

namespace OHOS {
namespace PowerMgr {
using namespace DisplayPowerMgr;
using namespace Rosen;

DeviceStateAction::DeviceStateAction()
{
    dispCallback_ = new DisplayPowerCallback();
}

DeviceStateAction::~DeviceStateAction()
{
    dispCallback_ = nullptr;
}

void DeviceStateAction::Suspend(int64_t callTimeMs, SuspendDeviceType type, uint32_t flags)
{
    // Display is controlled by PowerStateMachine
    // Don't suspend until GoToSleep is called
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
    POWER_HILOGD(FEATURE_POWER_STATE, "Get display state: %{public}d", state);
    DisplayState ret = DisplayState::DISPLAY_UNKNOWN;
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
        case DisplayPowerMgr::DisplayState::DISPLAY_UNKNOWN:
            ret = DisplayState::DISPLAY_UNKNOWN;
            break;
        default:
            break;
    }
    return ret;
}

PowerStateChangeReason DeviceStateAction::GetDmsReasonByPowerReason(StateChangeReason reason)
{
    PowerStateChangeReason dmsReason = static_cast<PowerStateChangeReason>(reason);
    switch (reason) {
        case StateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT:
            dmsReason = PowerStateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT;
            break;
        case StateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_SUCCESS:
            dmsReason = PowerStateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_SUCCESS;
            break;
        case StateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_FAIL_SCREEN_ON:
            dmsReason = PowerStateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_FAIL_SCREEN_ON;
            break;
        case StateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_FAIL_SCREEN_OFF:
            dmsReason = PowerStateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_FAIL_SCREEN_OFF;
            break;
        case StateChangeReason::STATE_CHANGE_REASON_POWER_KEY:
            dmsReason = PowerStateChangeReason::STATE_CHANGE_REASON_POWER_KEY;
            break;
        default:
            break;
    }
    POWER_HILOGI(FEATURE_POWER_STATE, "The reason to DMS is = %{public}d", static_cast<uint32_t>(dmsReason));
    return dmsReason;
}

uint32_t DeviceStateAction::SetDisplayState(DisplayState state, StateChangeReason reason)
{
    DisplayState currentState = GetDisplayState();
    if (state == currentState) {
        POWER_HILOGD(FEATURE_POWER_STATE, "Already in state: %{public}d", static_cast<uint32_t>(state));
        return ActionResult::SUCCESS;
    }
    if (!isRegister_) {
        isRegister_ = DisplayPowerMgrClient::GetInstance().RegisterCallback(dispCallback_);
    }
    if (reason == StateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_FAIL_SCREEN_OFF) {
        state = DisplayState::DISPLAY_OFF;
        currentState = DisplayState::DISPLAY_ON;
    }
    DisplayPowerMgr::DisplayState dispState = DisplayPowerMgr::DisplayState::DISPLAY_ON;
    PowerStateChangeReason dispReason = (reason == StateChangeReason::STATE_CHANGE_REASON_TIMEOUT_NO_SCREEN_LOCK ?
            PowerStateChangeReason::STATE_CHANGE_REASON_COLLABORATION : GetDmsReasonByPowerReason(reason));
    switch (state) {
        case DisplayState::DISPLAY_ON: {
            dispState = DisplayPowerMgr::DisplayState::DISPLAY_ON;
            if (currentState == DisplayState::DISPLAY_OFF) {
                std::string identity = IPCSkeleton::ResetCallingIdentity();
                DisplayManager::GetInstance().WakeUpBegin(dispReason);
                IPCSkeleton::SetCallingIdentity(identity);
            }
            break;
        }
        case DisplayState::DISPLAY_DIM:
            dispState = DisplayPowerMgr::DisplayState::DISPLAY_DIM;
            break;
        case DisplayState::DISPLAY_OFF: {
            dispState = DisplayPowerMgr::DisplayState::DISPLAY_OFF;
            if ((currentState == DisplayState::DISPLAY_ON || currentState == DisplayState::DISPLAY_DIM) &&
                reason != StateChangeReason::STATE_CHANGE_REASON_SENSOR) {
                std::string identity = IPCSkeleton::ResetCallingIdentity();
                DisplayManager::GetInstance().SuspendBegin(dispReason);
                IPCSkeleton::SetCallingIdentity(identity);
            }
            break;
        }
        case DisplayState::DISPLAY_SUSPEND:
            dispState = DisplayPowerMgr::DisplayState::DISPLAY_SUSPEND;
            break;
        default:
            break;
    }
    dispCallback_->notify_ = actionCallback_;
    bool ret = DisplayPowerMgrClient::GetInstance().SetDisplayState(dispState, reason);
    POWER_HILOGI(FEATURE_POWER_STATE, "Set display state finished, ret=%{public}d", ret);
    return ret ? ActionResult::SUCCESS : ActionResult::FAILED;
}

void DeviceStateAction::SetCoordinated(bool coordinated)
{
    coordinated_ = coordinated;
    bool ret = DisplayPowerMgrClient::GetInstance().SetCoordinated(coordinated_);
    POWER_HILOGI(FEATURE_POWER_STATE, "Set coordinated=%{public}d, ret=%{public}d", coordinated_, ret);
}

uint32_t DeviceStateAction::GoToSleep(const std::function<void()> onSuspend,
    const std::function<void()> onWakeup, bool force)
{
    SystemSuspendController::GetInstance().Suspend(onSuspend, onWakeup, force);
    return ActionResult::SUCCESS;
}

void DeviceStateAction::RegisterCallback(std::function<void(uint32_t)>& callback)
{
    actionCallback_ = callback;
}

void DeviceStateAction::DisplayPowerCallback::OnDisplayStateChanged(uint32_t displayId,
    DisplayPowerMgr::DisplayState state, uint32_t reason)
{
    POWER_HILOGD(FEATURE_POWER_STATE, "Callback: OnDisplayStateChanged");
    int32_t mainDisp = DisplayPowerMgrClient::GetInstance().GetMainDisplayId();
    if (mainDisp < 0 || static_cast<uint32_t>(mainDisp) != displayId) {
        POWER_HILOGI(FEATURE_POWER_STATE, "[UL_POWER] It's not main display, skip!");
        return;
    }
    switch (state) {
        case DisplayPowerMgr::DisplayState::DISPLAY_ON: {
            std::string identity = IPCSkeleton::ResetCallingIdentity();
            DisplayManager::GetInstance().WakeUpEnd();
            IPCSkeleton::SetCallingIdentity(identity);
            NotifyDisplayActionDone(DISPLAY_ON_DONE);
            break;
        }
        case DisplayPowerMgr::DisplayState::DISPLAY_OFF: {
            if (StateChangeReason(reason) != StateChangeReason::STATE_CHANGE_REASON_SENSOR) {
                std::string identity = IPCSkeleton::ResetCallingIdentity();
                DisplayManager::GetInstance().SuspendEnd();
                IPCSkeleton::SetCallingIdentity(identity);
            }
            NotifyDisplayActionDone(DISPLAY_OFF_DONE);
            break;
        }
        default:
            break;
    }
    return;
}

void DeviceStateAction::DisplayPowerCallback::NotifyDisplayActionDone(uint32_t event)
{
    std::lock_guard lock(notifyMutex_);
    if (notify_ != nullptr) {
        notify_(event);
    }
}
} // namespace PowerMgr
} // namespace OHOS
