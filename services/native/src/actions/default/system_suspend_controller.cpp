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

#include "system_suspend_controller.h"

#include "suspend/running_lock_hub.h"
#include "suspend/suspend_controller.h"

namespace OHOS {
namespace PowerMgr {
SystemSuspendController::SystemSuspendController()
{
    sc_ = std::make_shared<Suspend::SuspendController>();
    client_ = std::make_unique<PowerHdfClient>();
}

SystemSuspendController::~SystemSuspendController() = default;

void SystemSuspendController::Suspend(std::function<void()> onSuspend,
    std::function<void()> onWakeup, bool force)
{
    sc_->Suspend(onSuspend, onWakeup, force);
}

void SystemSuspendController::Wakeup()
{
    sc_->Wakeup();
}

void SystemSuspendController::AcquireRunningLock(const std::string& name)
{
    client_->WakeLock(name);
}

void SystemSuspendController::ReleaseRunningLock(const std::string& name)
{
    client_->WakeUnlock(name);
}

void SystemSuspendController::Dump(std::string& info)
{
    client_->Dump(info);
}
} // namespace PowerMgr
} // namespace OHOS
