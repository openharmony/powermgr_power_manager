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

#include "power_common.h"
#include "power_log.h"
#include "suspend/running_lock_hub.h"
#include "suspend/suspend_controller.h"

using namespace OHOS::HDI::Power::V1_0;

namespace {
const std::string HDI_SERVICE_NAME = "power_interface_service";
constexpr uint32_t RETRY_TIME = 1000;
}

namespace OHOS {
namespace PowerMgr {
SystemSuspendController::SystemSuspendController()
{
}

SystemSuspendController::~SystemSuspendController() = default;

void SystemSuspendController::RegisterHdiStatusListener(const std::shared_ptr<PowermsEventHandler>& handler)
{
#ifndef POWER_SUSPEND_NO_HDI
    POWER_HILOGD(COMP_SVC, "power rigister Hdi status listener");
    hdiServiceMgr_ = OHOS::HDI::ServiceManager::V1_0::IServiceManager::Get();
    if (hdiServiceMgr_ == nullptr) {
        handler->SendEvent(PowermsEventHandler::RETRY_REGISTER_HDI_STATUS_LISTENER, 0, RETRY_TIME);
        POWER_HILOGW(COMP_SVC, "hdi service manager is nullptr");
        return;
    }

    hdiServStatListener_ = new HdiServiceStatusListener(HdiServiceStatusListener::StatusCallback(
        [&](const OHOS::HDI::ServiceManager::V1_0::ServiceStatus &status) {
            RETURN_IF(status.serviceName != HDI_SERVICE_NAME || status.deviceClass != DEVICE_CLASS_DEFAULT);

            if (status.status == SERVIE_STATUS_START) {
                handler->SendEvent(PowermsEventHandler::REGISTER_POWER_HDI_CALLBACK, 0, 0);
                POWER_HILOGI(COMP_SVC, "power interface service start");
            } else if (status.status == SERVIE_STATUS_STOP && powerInterface_) {
                powerInterface_ = nullptr;
                POWER_HILOGW(COMP_SVC, "power interface service stop, unregister interface");
            }
        }
    ));

    int32_t status = hdiServiceMgr_->RegisterServiceStatusListener(hdiServStatListener_, DEVICE_CLASS_DEFAULT);
    if (status != ERR_OK) {
        handler->SendEvent(PowermsEventHandler::RETRY_REGISTER_HDI_STATUS_LISTENER, 0, RETRY_TIME);
        POWER_HILOGW(COMP_SVC, "Register hdi failed");
    }
#else
    sc_ = std::make_shared<Suspend::SuspendController>();
#endif
}

void SystemSuspendController::RegisterPowerHdiCallback()
{
    POWER_HILOGD(COMP_SVC, "register power hdi callback");
    if (powerInterface_ == nullptr) {
        powerInterface_ = IPowerInterface::Get();
        RETURN_IF_WITH_LOG(powerInterface_ == nullptr, "failed to get power hdi interface");
    }
    sptr<IPowerHdiCallback> callback = new PowerHdiCallback();
    powerInterface_->RegisterCallback(callback);
    POWER_HILOGD(COMP_SVC, "register power hdi callback end");
}

void SystemSuspendController::UnRegisterPowerHdiCallback()
{
    POWER_HILOGD(COMP_SVC, "unregister power hdi callback");
    if (powerInterface_ == nullptr) {
        powerInterface_ = IPowerInterface::Get();
        RETURN_IF_WITH_LOG(powerInterface_ == nullptr, "failed to get power hdi interface");
    }
    sptr<IPowerHdiCallback> callback = nullptr;
    powerInterface_->RegisterCallback(callback);
    POWER_HILOGD(COMP_SVC, "unregister power hdi callback end");
}

void SystemSuspendController::Suspend(const std::function<void()>& onSuspend,
    const std::function<void()>& onWakeup, bool force)
{
#ifndef POWER_SUSPEND_NO_HDI
    if (powerInterface_ == nullptr) {
        POWER_HILOGE(COMP_SVC, "The hdf interface is null");
        return;
    }
    if (force) {
        powerInterface_->ForceSuspend();
    } else {
        powerInterface_->StartSuspend();
    }
#else
    sc_->Suspend(onSuspend, onWakeup, force);
#endif
}

void SystemSuspendController::Wakeup()
{
#ifndef POWER_SUSPEND_NO_HDI
    if (powerInterface_ == nullptr) {
        POWER_HILOGE(COMP_SVC, "The hdf interface is null");
        return;
    }
    powerInterface_->StopSuspend();
#else
    sc_->Wakeup();
#endif
}

void SystemSuspendController::AcquireRunningLock(const std::string& name)
{
#ifndef POWER_SUSPEND_NO_HDI
    if (powerInterface_ == nullptr) {
        POWER_HILOGE(COMP_SVC, "The hdf interface is null");
        return;
    }
    powerInterface_->SuspendBlock(name);
#endif
}

void SystemSuspendController::ReleaseRunningLock(const std::string& name)
{
#ifndef POWER_SUSPEND_NO_HDI
    if (powerInterface_ == nullptr) {
        POWER_HILOGE(COMP_SVC, "The hdf interface is null");
        return;
    }
    powerInterface_->SuspendUnblock(name);
#endif
}

void SystemSuspendController::Dump(std::string& info)
{
#ifndef POWER_SUSPEND_NO_HDI
    if (powerInterface_ == nullptr) {
        POWER_HILOGE(COMP_SVC, "The hdf interface is null");
        return;
    }
    powerInterface_->PowerDump(info);
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
