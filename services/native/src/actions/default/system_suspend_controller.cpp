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
#ifndef POWER_SUSPEND_NO_HDI
    callback_ = new PowerHdfCallback();
    client_ = std::make_unique<PowerHdfClient>();
    client_->RegisterCallback(callback_);
#else
    sc_ = std::make_shared<Suspend::SuspendController>();
#endif
}

SystemSuspendController::~SystemSuspendController() = default;

void SystemSuspendController::Suspend(std::function<void()> onSuspend,
    std::function<void()> onWakeup, bool force)
{
#ifndef POWER_SUSPEND_NO_HDI
    if (force) {
        client_->ForceSuspend();
    } else {
        client_->StartSuspend();
    }
#else
    sc_->Suspend(onSuspend, onWakeup, force);
#endif
}

void SystemSuspendController::Wakeup()
{
#ifndef POWER_SUSPEND_NO_HDI
    client_->StopSuspend();
#else
    sc_->Wakeup();
#endif
}

void SystemSuspendController::AcquireRunningLock(const std::string& name)
{
#ifndef POWER_SUSPEND_NO_HDI
    client_->SuspendBlock(name);
#endif
}

void SystemSuspendController::ReleaseRunningLock(const std::string& name)
{
#ifndef POWER_SUSPEND_NO_HDI
    client_->SuspendUnblock(name);
#endif
}

void SystemSuspendController::Dump(std::string& info)
{
#ifndef POWER_SUSPEND_NO_HDI
    client_->Dump(info);
#endif
}

void SystemSuspendController::PowerHdfCallback::OnSuspend()
{
    if (onSuspend_ != nullptr) {
        onSuspend_();
    }
}

void SystemSuspendController::PowerHdfCallback::OnWakeup()
{
    if (onWakeup_ != nullptr) {
        onWakeup_();
    }
}

void SystemSuspendController::PowerHdfCallback::SetListener(
    std::function<void()> suspend,
    std::function<void()> wakeup)
{
    onSuspend_ = suspend;
    onWakeup_ = wakeup;
}
} // namespace PowerMgr
} // namespace OHOS
