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
#include "system_suspend_controller.h"

using namespace OHOS::DisplayMgr;
using namespace std;

namespace OHOS {
namespace PowerMgr {
void DeviceStateAction::Suspend(int64_t callTimeMs, SuspendDeviceType type, uint32_t flags)
{
    DisplayManager::SetScreenState(ScreenState::SCREEN_STATE_OFF);
    SystemSuspendController::GetInstance().EnableSuspend();
}

void DeviceStateAction::ForceSuspend()
{
    DisplayManager::SetScreenState(ScreenState::SCREEN_STATE_OFF);
    SystemSuspendController::GetInstance().ForceSuspend();
}

void DeviceStateAction::Wakeup(int64_t callTimeMs, WakeupDeviceType type, const string& details,
    const string& pkgName)
{
    SystemSuspendController::GetInstance().DisableSuspend();
    DisplayManager::SetScreenState(ScreenState::SCREEN_STATE_ON);
}
} // namespace PowerMgr
} // namespace OHOS
