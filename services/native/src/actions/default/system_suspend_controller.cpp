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

#include "system_suspend_controller.h"

#include "power_log.h"
#include "suspend/running_lock_hub.h"
#include "suspend/suspend_controller.h"
#include "v1_0/power_interface_proxy.h"

using namespace OHOS::HDI::Power::V1_0;

namespace OHOS {
namespace PowerMgr {
sptr<IPowerInterface> powerInterface = nullptr;
SystemSuspendController::SystemSuspendController()
{
#ifndef POWER_SUSPEND_NO_HDI
    sptr<IPowerHdiCallback> g_callback = new PowerHdiCallbackImpl();
    powerInterface = IPowerInterface::Get();
    if (powerInterface == nullptr) {
        POWER_HILOGE(COMP_SVC, "The hdf interface is null");
        return;
    }
    powerInterface->RegisterCallback(g_callback);
#else
    sc_ = std::make_shared<Suspend::SuspendController>();
#endif
}

SystemSuspendController::~SystemSuspendController() = default;

void SystemSuspendController::Suspend(const std::function<void()>& onSuspend,
    const std::function<void()>& onWakeup, bool force)
{
#ifndef POWER_SUSPEND_NO_HDI
    if (powerInterface == nullptr) {
        POWER_HILOGE(COMP_SVC, "The hdf interface is null");
        return;
    }
    if (force) {
        powerInterface->ForceSuspend();
    } else {
        powerInterface->StartSuspend();
    }
#else
    sc_->Suspend(onSuspend, onWakeup, force);
#endif
}

void SystemSuspendController::Wakeup()
{
#ifndef POWER_SUSPEND_NO_HDI
    if (powerInterface == nullptr) {
        POWER_HILOGE(COMP_SVC, "The hdf interface is null");
        return;
    }
    powerInterface->StopSuspend();
#else
    sc_->Wakeup();
#endif
}

void SystemSuspendController::AcquireRunningLock(const std::string& name)
{
#ifndef POWER_SUSPEND_NO_HDI
    if (powerInterface == nullptr) {
        POWER_HILOGE(COMP_SVC, "The hdf interface is null");
        return;
    }
    powerInterface->SuspendBlock(name);
#endif
}

void SystemSuspendController::ReleaseRunningLock(const std::string& name)
{
#ifndef POWER_SUSPEND_NO_HDI
    if (powerInterface == nullptr) {
        POWER_HILOGE(COMP_SVC, "The hdf interface is null");
        return;
    }
    powerInterface->SuspendUnblock(name);
#endif
}

void SystemSuspendController::Dump(std::string& info)
{
#ifndef POWER_SUSPEND_NO_HDI
    if (powerInterface == nullptr) {
        POWER_HILOGE(COMP_SVC, "The hdf interface is null");
        return;
    }
    powerInterface->PowerDump(info);
#endif
}

int32_t SystemSuspendController::PowerHdfCallback::OnSuspend()
{
    if (onSuspend_ != nullptr) {
        onSuspend_();
    }
    return 0;
}

int32_t SystemSuspendController::PowerHdfCallback::OnWakeup()
{
    if (onWakeup_ != nullptr) {
        onWakeup_();
    }
    return 0;
}

void SystemSuspendController::PowerHdfCallback::SetListener(
    std::function<void()>& suspend,
    std::function<void()>& wakeup)
{
    onSuspend_ = suspend;
    onWakeup_ = wakeup;
}
} // namespace PowerMgr
} // namespace OHOS
