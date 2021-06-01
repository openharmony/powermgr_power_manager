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

#include "powerms_event_handler.h"

#include "power_common.h"
#include "power_mgr_service.h"

namespace OHOS {
namespace PowerMgr {
PowermsEventHandler::PowermsEventHandler(const std::shared_ptr<AppExecFwk::EventRunner>& runner,
    const wptr<PowerMgrService>& service)
    : AppExecFwk::EventHandler(runner), service_(service)
{
    POWER_HILOGD(MODULE_SERVICE, "PowermsEventHandler::PowermsEventHandler instance created.");
}

void PowermsEventHandler::ProcessEvent([[maybe_unused]] const AppExecFwk::InnerEvent::Pointer& event)
{
    auto pmsptr = service_.promote();
    if (pmsptr == nullptr) {
        return;
    }
    POWER_HILOGI(MODULE_SERVICE, "PowermsEventHandler::%{public}s ,eventid = %d", __func__,
        event->GetInnerEventId());
    switch (event->GetInnerEventId()) {
        case CHECK_RUNNINGLOCK_OVERTIME_MSG: {
            auto runningLockMgr = pmsptr->GetRunningLockMgr();
            if (runningLockMgr == nullptr) {
                return;
            }
            runningLockMgr->CheckOverTime();
            break;
        }
        case CHECK_USER_ACTIVITY_TIMEOUT_MSG: {
            auto powerStateMachine = pmsptr->GetPowerStateMachine();
            if (powerStateMachine == nullptr) {
                return;
            }
            powerStateMachine->HandleDelayTimer();
            break;
        }
        default:
        POWER_HILOGD(MODULE_SERVICE, "PowermsEventHandler::no event id matched.");
    }
}
} // namespace PowerMgr
} // namespace OHOS
