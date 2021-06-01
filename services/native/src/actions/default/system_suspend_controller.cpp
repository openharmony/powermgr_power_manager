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
    rlh_ = std::make_unique<Suspend::RunningLockHub>(sc_);
    rlh_->Acquire(WAKEUP_HOLDER);
}

SystemSuspendController::~SystemSuspendController() = default;

void SystemSuspendController::EnableSuspend()
{
    std::lock_guard lock(mutex_);
    sc_->EnableSuspend();
    if (!suspendEnabled_) {
        rlh_->Release(WAKEUP_HOLDER);
        suspendEnabled_ = true;
    }
}

void SystemSuspendController::ForceSuspend()
{
    sc_->ForceSuspend();
}

void SystemSuspendController::DisableSuspend()
{
    std::lock_guard lock(mutex_);
    if (suspendEnabled_) {
        rlh_->Acquire(WAKEUP_HOLDER);
        suspendEnabled_ = false;
    }
}

void SystemSuspendController::AcquireRunningLock(const std::string& name)
{
    rlh_->Acquire(name);
}

void SystemSuspendController::ReleaseRunningLock(const std::string& name)
{
    rlh_->Release(name);
}
} // namespace PowerMgr
} // namespace OHOS
