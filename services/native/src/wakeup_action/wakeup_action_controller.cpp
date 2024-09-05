/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "wakeup_action_controller.h"

#include <ipc_skeleton.h>
#include "power_log.h"
#include "system_suspend_controller.h"

namespace OHOS {
namespace PowerMgr {

/** WakeupActionController Implement */
WakeupActionController::WakeupActionController(
    std::shared_ptr<ShutdownController>& shutdownController, std::shared_ptr<PowerStateMachine>& stateMachine)
{
    shutdownController_ = shutdownController;
    stateMachine_ = stateMachine;
}

WakeupActionController::~WakeupActionController()
{
}

void WakeupActionController::Init()
{
    std::lock_guard lock(mutex_);
    std::shared_ptr<WakeupActionSources> sources = WakeupActionSourceParser::ParseSources();
    sourceMap_ = sources->GetSourceMap();
    if (sourceMap_.empty()) {
        POWER_HILOGE(FEATURE_WAKEUP_ACTION, "InputManager is null");
    }
}

bool WakeupActionController::ExecuteByGetReason()
{
    std::string reason;
    SystemSuspendController::GetInstance().GetWakeupReason(reason);
    if (reason.empty()) {
        POWER_HILOGI(FEATURE_WAKEUP_ACTION, "WakeupAction reason is empty");
        return false;
    }
    reason.erase(reason.end() - 1);
    POWER_HILOGI(FEATURE_WAKEUP_ACTION, "WakeupAction reason %{public}s", reason.c_str());
    if (sourceMap_.find(reason) != sourceMap_.end()) {
        pid_t pid = IPCSkeleton::GetCallingPid();
        auto uid = IPCSkeleton::GetCallingUid();
        POWER_HILOGI(FEATURE_WAKEUP_ACTION,
            "WakeupAction device, pid=%{public}d, uid=%{public}d, reason=%{public}s, scene=%{public}s, "
            "action=%{public}u",
            pid, uid, reason.c_str(), sourceMap_[reason]->GetScene().c_str(), sourceMap_[reason]->GetAction());
        HandleAction(reason);
        return true;
    }
    POWER_HILOGI(FEATURE_WAKEUP_ACTION, "WakeupAction reason %{public}s doesn't exist", reason.c_str());
    return false;
}

void WakeupActionController::HandleAction(const std::string& reason)
{
    switch (static_cast<WakeupAction>(sourceMap_[reason]->GetAction())) {
        case WakeupAction::ACTION_HIBERNATE:
            HandleHibernate(WakeupActionSources::mapSuspendDeviceType(reason));
            break;
        case WakeupAction::ACTION_SHUTDOWN:
            HandleShutdown(sourceMap_[reason]->GetScene());
            break;
        case WakeupAction::ACTION_NONE:
        default:
            break;
    }
}

void WakeupActionController::HandleHibernate(SuspendDeviceType reason)
{
    if (stateMachine_ == nullptr) {
        POWER_HILOGE(FEATURE_WAKEUP_ACTION, "Can't get PowerStateMachine");
        return;
    }
    bool ret = stateMachine_->SetState(
        PowerState::HIBERNATE, stateMachine_->GetReasionBySuspendType(reason), true);
    if (ret) {
        POWER_HILOGI(FEATURE_WAKEUP_ACTION, "State changed, call hibernate");
    } else {
        POWER_HILOGI(FEATURE_WAKEUP_ACTION, "Hibernate: State change failed");
    }
}

void WakeupActionController::HandleShutdown(const std::string& scene)
{
    POWER_HILOGI(FEATURE_WAKEUP_ACTION, "shutdown by reason=%{public}s", scene.c_str());
    shutdownController_->Shutdown(scene);
}

} // namespace PowerMgr
} // namespace OHOS
