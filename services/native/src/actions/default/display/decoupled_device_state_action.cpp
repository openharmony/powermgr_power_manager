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

#include "decoupled_device_state_action.h"

#include "system_suspend_controller.h"

namespace OHOS {
namespace PowerMgr {
void DecoupledDeviceStateAction::ForceSuspend()
{
    GoToSleep(nullptr, nullptr, true);
}

void DecoupledDeviceStateAction::Wakeup(int64_t callTimeMs, WakeupDeviceType type, const std::string& details,
    const std::string& pkgName)
{
    SystemSuspendController::GetInstance().Wakeup();
}

uint32_t DecoupledDeviceStateAction::GoToSleep(const std::function<void()> onSuspend,
    const std::function<void()> onWakeup, bool force)
{
    SystemSuspendController::GetInstance().Suspend(onSuspend, onWakeup, force);
    return ActionResult::SUCCESS;
}

DisplayState DecoupledDeviceStateAction::GetDisplayState()
{
    return DisplayState::DISPLAY_UNKNOWN;
}

uint32_t DecoupledDeviceStateAction::SetDisplayState(const DisplayState state, StateChangeReason reason)
{
    return ActionResult::SUCCESS;
}

bool DecoupledDeviceStateAction::TryToCancelScreenOff()
{
    return false;
}
} // namespace PowerMgr
} // namespace OHOS
