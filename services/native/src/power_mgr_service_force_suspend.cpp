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

#include "power_mgr_service.h"

#include <ipc_skeleton.h>

#include "permission.h"
#include "power_log.h"
#include "suspend_controller.h"

namespace OHOS {
namespace PowerMgr {
bool PowerMgrService::IsCockpitLegacySuspendDenied() const
{
#ifdef POWER_MANAGER_ENABLE_FORCE_SUSPEND_IGNORING_WAKELOCK
    POWER_HILOGI(FEATURE_SUSPEND, "this function is not supported when force suspend ignoring wakelock is enabled");
    return true;
#else
    return false;
#endif
}

PowerErrors PowerMgrService::ForceSuspendDeviceIgnoringWakelock(const std::string& suspendTag)
{
    if (!Permission::IsSystem()) {
        POWER_HILOGI(FEATURE_SUSPEND, "ForceSuspendDeviceIgnoringWakelock failed, System permission intercept");
        return PowerErrors::ERR_SYSTEM_API_DENIED;
    }
    if (!Permission::IsPermissionGranted("ohos.permission.POWER_MANAGER")) {
        POWER_HILOGI(FEATURE_SUSPEND, "ForceSuspendDeviceIgnoringWakelock failed, no POWER_MANAGER permission");
        return PowerErrors::ERR_PERMISSION_DENIED;
    }
#if !defined(POWER_MANAGER_ENABLE_FORCE_SUSPEND_IGNORING_WAKELOCK) || !defined(POWER_MANAGER_ENABLE_SUSPEND_WITH_TAG)
    (void)suspendTag;
    POWER_HILOGI(FEATURE_SUSPEND,
        "ForceSuspendDeviceIgnoringWakelock failed, force suspend ignoring wakelock or suspend-with-tag is not enabled");
    return PowerErrors::ERR_CAPABILITY_NOT_SUPPORTED;
#else
    std::lock_guard lock(suspendMutex_);
    pid_t pid = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    if (shutdownController_->IsShuttingDown()) {
        POWER_HILOGI(FEATURE_SUSPEND, "System is shutting down, skip ForceSuspendDeviceIgnoringWakelock");
        return PowerErrors::ERR_FAILURE;
    }
    POWER_HILOGI(FEATURE_SUSPEND,
        "[UL_POWER] Try to force suspend ignoring wakelock, pid=%{public}d, uid=%{public}d, tag=%{public}s",
        pid, uid, suspendTag.c_str());
#ifdef POWER_MANAGER_ENABLE_CHARGING_TYPE_SETTING
    if (suspendController_) {
        suspendController_->StopSleep();
    }
#endif
#ifdef HAS_HIVIEWDFX_HISYSEVENT_PART
    powerStateMachine_->ReportSuspendStart(
        uid, static_cast<int32_t>(SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION), true);
#endif
    if (suspendController_ == nullptr) {
        POWER_HILOGE(FEATURE_SUSPEND, "SuspendController is null");
        return PowerErrors::ERR_FAILURE;
    }
    bool ret = suspendController_->HandleForceSuspendIgnoringWakelock(
        SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, suspendTag);
    return ret ? PowerErrors::ERR_OK : PowerErrors::ERR_FAILURE;
#endif
}
} // namespace PowerMgr
} // namespace OHOS
