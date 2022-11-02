/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "default_device_state_action.h"
#include "system_suspend_controller.h"

namespace OHOS {
namespace PowerMgr {
using namespace std;

void DefaultDeviceStateAction::Suspend(int64_t callTimeMs, SuspendDeviceType type, uint32_t flags)
{
    (void)callTimeMs;
    (void)type;
    (void)flags;
}

void DefaultDeviceStateAction::ForceSuspend() {}

void DefaultDeviceStateAction::Wakeup(
    int64_t callTimeMs, WakeupDeviceType type, const string& details, const string& pkgName)
{
    (void)callTimeMs;
    (void)type;
    (void)details;
    (void)pkgName;
}

DisplayState DefaultDeviceStateAction::GetDisplayState()
{
    return DisplayState::DISPLAY_UNKNOWN;
}

uint32_t DefaultDeviceStateAction::SetDisplayState(const DisplayState state, StateChangeReason reason)
{
    (void)state;
    (void)reason;
    return ActionResult::SUCCESS;
}

uint32_t DefaultDeviceStateAction::GoToSleep(
    const std::function<void()> onSuspend, const std::function<void()> onWakeup, bool force)
{
    SystemSuspendController::GetInstance().Suspend(onSuspend, onWakeup, force);
    return ActionResult::SUCCESS;
}

void DefaultDeviceStateAction::RegisterCallback(std::function<void(uint32_t)>& callback)
{
    (void)callback;
}
} // namespace PowerMgr
} // namespace OHOS
