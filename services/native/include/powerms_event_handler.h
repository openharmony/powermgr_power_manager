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

#ifndef POWERMGR_POWERMS_EVENT_HANDLER_H
#define POWERMGR_POWERMS_EVENT_HANDLER_H

#include <event_handler.h>
#include <refbase.h>

namespace OHOS {
namespace PowerMgr {
class PowerMgrService;

class PowermsEventHandler : public AppExecFwk::EventHandler {
public:
    enum {
        CHECK_RUNNINGLOCK_OVERTIME_MSG = 1,
        CHECK_USER_ACTIVITY_TIMEOUT_MSG = 2,
    };

    PowermsEventHandler(const std::shared_ptr<AppExecFwk::EventRunner>& runner,
        const wptr<PowerMgrService>& service);
    ~PowermsEventHandler() = default;
    void ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event) override;

private:
    wptr<PowerMgrService> service_;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_POWERMS_EVENT_HANDLER_H
