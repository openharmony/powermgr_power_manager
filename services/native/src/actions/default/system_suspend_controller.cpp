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

#include "system_suspend_controller.h"
#ifdef HAS_HIVIEWDFX_HISYSEVENT_PART
#include "hisysevent.h"
#endif
#include "power_common.h"
#include "power_log.h"
#include "suspend/running_lock_hub.h"

namespace OHOS {
namespace PowerMgr {
namespace {
const std::string HDI_SERVICE_NAME = "power_interface_service";
constexpr uint32_t RETRY_TIME = 1000;
constexpr int32_t ERR_FAILED = -1;
} // namespace
using namespace OHOS::HDI::Power::V1_3;

SystemSuspendController::SystemSuspendController() {}

SystemSuspendController::~SystemSuspendController() = default;

void SystemSuspendController::RegisterHdiStatusListener()
{
    POWER_HILOGD(COMP_SVC, "power rigister Hdi status listener");
    hdiServiceMgr_ = OHOS::HDI::ServiceManager::V1_0::IServiceManager::Get();
    if (hdiServiceMgr_ == nullptr) {
        FFRTTask retryTask = [this] {
            RegisterHdiStatusListener();
        };
        POWER_HILOGW(COMP_SVC, "hdi service manager is nullptr");
        FFRTUtils::SubmitDelayTask(retryTask, RETRY_TIME, queue_);
        return;
    }

    hdiServStatListener_ = new HdiServiceStatusListener(
        HdiServiceStatusListener::StatusCallback([&](const OHOS::HDI::ServiceManager::V1_0::ServiceStatus& status) {
            RETURN_IF(status.serviceName != HDI_SERVICE_NAME || status.deviceClass != DEVICE_CLASS_DEFAULT);

            if (status.status == SERVIE_STATUS_START) {
                FFRTTask task = [this] {
                    RegisterPowerHdiCallback();
                };
                FFRTUtils::SubmitTask(task);
                POWER_HILOGI(COMP_SVC, "power interface service start");
            } else if (status.status == SERVIE_STATUS_STOP) {
                std::lock_guard lock(interfaceMutex_);
                if (powerInterface_) {
                    powerInterface_ = nullptr;
                }
                POWER_HILOGW(COMP_SVC, "power interface service stop, unregister interface");
            }
        }));

    int32_t status = hdiServiceMgr_->RegisterServiceStatusListener(hdiServStatListener_, DEVICE_CLASS_DEFAULT);
    if (status != ERR_OK) {
        FFRTTask retryTask = [this] {
            RegisterHdiStatusListener();
        };
        POWER_HILOGW(COMP_SVC, "Register hdi failed");
        FFRTUtils::SubmitDelayTask(retryTask, RETRY_TIME, queue_);
    }
}

sptr<IPowerInterface> SystemSuspendController::GetPowerInterface()
{
    std::lock_guard lock(interfaceMutex_);
    if (powerInterface_ == nullptr) {
        powerInterface_ = IPowerInterface::Get();
        RETURN_IF_WITH_RET(powerInterface_ == nullptr, nullptr);
    }
    return powerInterface_;
}

void SystemSuspendController::RegisterPowerHdiCallback()
{
    POWER_HILOGD(COMP_SVC, "register power hdi callback");
    sptr<IPowerInterface> powerInterface = GetPowerInterface();
    if (powerInterface == nullptr) {
        POWER_HILOGE(COMP_SVC, "The hdf interface is null");
        return;
    }
    sptr<IPowerHdiCallback> callback = new PowerHdiCallback();
    powerInterface->RegisterCallback(callback);
    POWER_HILOGD(COMP_SVC, "register power hdi callback end");
}

void SystemSuspendController::UnRegisterPowerHdiCallback()
{
    POWER_HILOGD(COMP_SVC, "unregister power hdi callback");
    sptr<IPowerInterface> powerInterface = GetPowerInterface();
    if (powerInterface == nullptr) {
        POWER_HILOGE(COMP_SVC, "The hdf interface is null");
        return;
    }
    sptr<IPowerHdiCallback> callback = nullptr;
    powerInterface->RegisterCallback(callback);
    POWER_HILOGD(COMP_SVC, "unregister power hdi callback end");
}

void SystemSuspendController::SetSuspendTag(const std::string& tag)
{
    sptr<IPowerInterface> powerInterface = GetPowerInterface();
    if (powerInterface == nullptr) {
        POWER_HILOGE(COMP_SVC, "The hdf interface is null");
        return;
    }
#ifdef HAS_HIVIEWDFX_HISYSEVENT_PART
    HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::POWER, "SET_SUSPEND_TAG", HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "TAG", tag);
#endif
    powerInterface->SetSuspendTag(tag);
}

void SystemSuspendController::AllowAutoSleep()
{
    allowSleepTask_ = true;
}

void SystemSuspendController::DisallowAutoSleep()
{
    allowSleepTask_ = false;
}

void SystemSuspendController::Suspend(
    const std::function<void()>& onSuspend, const std::function<void()>& onWakeup, bool force)
{
    std::lock_guard lock(mutex_);
    POWER_HILOGI(COMP_SVC, "The hdf interface, force=%{public}u", static_cast<uint32_t>(force));
    sptr<IPowerInterface> powerInterface = GetPowerInterface();
    if (powerInterface == nullptr) {
        POWER_HILOGE(COMP_SVC, "The hdf interface is null");
        return;
    }
#ifdef HAS_HIVIEWDFX_HISYSEVENT_PART
    HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::POWER, "DO_SUSPEND", HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "TYPE", static_cast<int32_t>(1));
#endif
    if (force) {
        powerInterface->ForceSuspend();
    } else if (allowSleepTask_.load()) {
        powerInterface->StartSuspend();
    }
}

void SystemSuspendController::Wakeup()
{
    std::lock_guard lock(mutex_);
    sptr<IPowerInterface> powerInterface = GetPowerInterface();
    if (powerInterface == nullptr) {
        POWER_HILOGE(COMP_SVC, "The hdf interface is null");
        return;
    }
#ifdef HAS_HIVIEWDFX_HISYSEVENT_PART
    HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::POWER, "DO_SUSPEND", HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "TYPE", static_cast<int32_t>(0));
#endif
    powerInterface->StopSuspend();
}

bool SystemSuspendController::Hibernate()
{
    POWER_HILOGI(COMP_SVC, "SystemSuspendController hibernate begin.");
    sptr<IPowerInterface> powerInterface = GetPowerInterface();
    if (powerInterface == nullptr) {
        POWER_HILOGE(COMP_SVC, "The hdf interface is null");
        return false;
    }
#ifdef HAS_HIVIEWDFX_HISYSEVENT_PART
    HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::POWER, "DO_HIBERNATE",
        HiviewDFX::HiSysEvent::EventType::BEHAVIOR);
#endif
    int32_t ret = powerInterface->Hibernate();
    if (ret != HDF_SUCCESS) {
        POWER_HILOGE(COMP_SVC, "SystemSuspendController hibernate failed.");
        return false;
    }
    POWER_HILOGI(COMP_SVC, "SystemSuspendController hibernate end.");
    return true;
}

OHOS::HDI::Power::V1_3::RunningLockInfo SystemSuspendController::FillRunningLockInfo(const RunningLockParam& param)
{
    OHOS::HDI::Power::V1_3::RunningLockInfo filledInfo {};
    filledInfo.name = param.name;
    filledInfo.type = static_cast<OHOS::HDI::Power::V1_3::RunningLockType>(param.type);
    filledInfo.timeoutMs = param.timeoutMs;
    filledInfo.uid = param.uid;
    filledInfo.pid = param.pid;
    return filledInfo;
}

int32_t SystemSuspendController::AcquireRunningLock(const RunningLockParam& param)
{
    sptr<IPowerInterface> powerInterface = GetPowerInterface();
    int32_t status = RUNNINGLOCK_FAILURE;
    if (powerInterface == nullptr) {
        POWER_HILOGE(COMP_SVC, "The hdf interface is null");
        return status;
    }
    OHOS::HDI::Power::V1_3::RunningLockInfo filledInfo = FillRunningLockInfo(param);
    status = powerInterface->HoldRunningLockExt(filledInfo,
        param.lockid, param.bundleName);
    return status;
}

int32_t SystemSuspendController::ReleaseRunningLock(const RunningLockParam& param)
{
    sptr<IPowerInterface> powerInterface = GetPowerInterface();
    int32_t status = RUNNINGLOCK_FAILURE;
    if (powerInterface == nullptr) {
        POWER_HILOGE(COMP_SVC, "The hdf interface is null");
        return status;
    }
    OHOS::HDI::Power::V1_3::RunningLockInfo filledInfo = FillRunningLockInfo(param);
    status = powerInterface->UnholdRunningLockExt(filledInfo,
        param.lockid, param.bundleName);
    return status;
}

void SystemSuspendController::Dump(std::string& info)
{
    sptr<IPowerInterface> powerInterface = GetPowerInterface();
    if (powerInterface == nullptr) {
        POWER_HILOGE(COMP_SVC, "The hdf interface is null");
        return;
    }
    powerInterface->PowerDump(info);
}

void SystemSuspendController::GetWakeupReason(std::string& reason)
{
    sptr<IPowerInterface> powerInterface = GetPowerInterface();
    if (powerInterface == nullptr) {
        POWER_HILOGE(COMP_SVC, "The hdf interface is null");
        return;
    }
    powerInterface->GetWakeupReason(reason);
}

int32_t SystemSuspendController::SetPowerConfig(const std::string& sceneName, const std::string& value)
{
    sptr<IPowerInterface> powerInterface = GetPowerInterface();
    if (powerInterface == nullptr) {
        POWER_HILOGE(COMP_SVC, "The hdf interface is null");
        return ERR_FAILED;
    }
    return powerInterface->SetPowerConfig(sceneName, value);
}

int32_t SystemSuspendController::GetPowerConfig(const std::string& sceneName, std::string& value)
{
    sptr<IPowerInterface> powerInterface = GetPowerInterface();
    if (powerInterface == nullptr) {
        POWER_HILOGE(COMP_SVC, "The hdf interface is null");
        return ERR_FAILED;
    }
    return powerInterface->GetPowerConfig(sceneName, value);
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

int32_t SystemSuspendController::PowerHdfCallback::OnWakeupWithTag(int32_t suspendTag)
{
    if (onWakeup_ != nullptr) {
        onWakeupWithTag_(suspendTag);
    }
    return 0;
}

void SystemSuspendController::PowerHdfCallback::SetListener(
    std::function<void()>& suspend, std::function<void()>& wakeup, std::function<void(int32_t)>& onWakeupWithTag)
{
    onSuspend_ = suspend;
    onWakeup_ = wakeup;
    onWakeupWithTag_ = onWakeupWithTag;
}
} // namespace PowerMgr
} // namespace OHOS
