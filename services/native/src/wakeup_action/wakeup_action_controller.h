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

#ifndef POWERMGR_WAKEUP_ACTION_CONTROLLER_H
#define POWERMGR_WAKEUP_ACTION_CONTROLLER_H

#include <functional>
#include <memory>
#include <map>

#include "event_handler.h"
#include "power_state_machine.h"
#include "shutdown_controller.h"
#include "wakeup_action_source_parser.h"
#include "wakeup_action_sources.h"

namespace OHOS {
namespace PowerMgr {

class WakeupActionController : public std::enable_shared_from_this<WakeupActionController> {
public:
    WakeupActionController(
        std::shared_ptr<ShutdownController>& shutdownController, std::shared_ptr<PowerStateMachine>& stateMachine);
    ~WakeupActionController();
    void Init();
    bool ExecuteByGetReason();

private:
    void HandleAction(const std::string& reason);
    void HandleHibernate(SuspendDeviceType reason);
    void HandleShutdown(const std::string& scene);

    std::map<std::string, std::shared_ptr<WakeupActionSource>> sourceMap_;
    std::shared_ptr<ShutdownController> shutdownController_;
    std::shared_ptr<PowerStateMachine> stateMachine_;
    std::mutex mutex_;
};

} // namespace PowerMgr
} // namespace OHOS

#endif // POWERMGR_WAKEUP_ACTION_CONTROLLER_H
