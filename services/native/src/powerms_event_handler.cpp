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

#include "powerms_event_handler.h"

#include "power_log.h"
#include "power_mgr_service.h"
#include "system_suspend_controller.h"

namespace OHOS {
namespace PowerMgr {
PowermsEventHandler::PowermsEventHandler(const std::shared_ptr<AppExecFwk::EventRunner>& runner,
    const wptr<PowerMgrService>& service)
    : AppExecFwk::EventHandler(runner), service_(service)
{
    POWER_HILOGD(COMP_SVC, "Instance created");
}

void PowermsEventHandler::ProcessEvent([[maybe_unused]] const AppExecFwk::InnerEvent::Pointer& event)
{
    auto pmsptr = service_.promote();
    if (pmsptr == nullptr) {
        POWER_HILOGE(COMP_SVC, "Power service is nullptr");
        return;
    }
    POWER_HILOGI(COMP_SVC, "Start to process, eventId: %{public}d", event->GetInnerEventId());
    switch (event->GetInnerEventId()) {
        case INIT_KEY_MONITOR_MSG: {
            pmsptr->KeyMonitorInit();
            pmsptr->HallSensorSubscriberInit();
            break;
        }
        case CHECK_RUNNINGLOCK_OVERTIME_MSG: {
            auto runningLockMgr = pmsptr->GetRunningLockMgr();
            if (runningLockMgr == nullptr) {
                return;
            }
            runningLockMgr->CheckOverTime();
            break;
        }
        case CHECK_USER_ACTIVITY_TIMEOUT_MSG: // fallthrough
        case CHECK_USER_ACTIVITY_OFF_TIMEOUT_MSG: // fallthrough
        case CHECK_USER_ACTIVITY_SLEEP_TIMEOUT_MSG: // fallthrough
        case SYSTEM_WAKE_UP_MSG: {
            auto powerStateMachine = pmsptr->GetPowerStateMachine();
            if (powerStateMachine == nullptr) {
                return;
            }
            powerStateMachine->HandleDelayTimer(event->GetInnerEventId());
            break;
        }
        case SCREEN_ON_TIMEOUT_MSG: {
            pmsptr->HandleScreenOnTimeout();
            break;
        }
        case SHUTDOWN_REQUEST_MSG: {
            pmsptr->HandleShutdownRequest();
            break;
        }
        case RETRY_REGISTER_HDI_STATUS_LISTENER: {
            SystemSuspendController::GetInstance().RegisterHdiStatusListener(pmsptr->GetHandler());
            break;
        }
        case REGISTER_POWER_HDI_CALLBACK: {
            SystemSuspendController::GetInstance().RegisterPowerHdiCallback();
            break;
        }
        default:
            POWER_HILOGW(COMP_SVC, "No matched event id");
    }
}
} // namespace PowerMgr
} // namespace OHOS
