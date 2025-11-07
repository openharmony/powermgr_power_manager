/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef POWERMGR_SCREEN_COMMON_EVENT_CONTROLLER_H
#define POWERMGR_SCREEN_COMMON_EVENT_CONTROLLER_H

#include "singleton.h"
#include "power_mgr_service.h"
#ifdef POWER_MANAGER_ENABLE_WATCH_CUSTOMIZED_SCREEN_COMMON_EVENT_RULES
#include "app_manager_utils.h"
#include "power_hookmgr.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "power_log.h"
#include "app_mgr_interface.h"
#include <mutex>
#endif

namespace OHOS {
namespace PowerMgr {
class ScreenCommonEventController : public DelayedSingleton<ScreenCommonEventController> {
public:
    const uint32_t ERROR_OK = 1;
    const uint32_t ERROR_FAIL = 0;

    ScreenCommonEventController() = default;

#ifdef POWER_MANAGER_ENABLE_WATCH_CUSTOMIZED_SCREEN_COMMON_EVENT_RULES
    uint32_t InitScreenCommonEventController();
    uint32_t SetScreenOnCommonEventRules(StateChangeReason reason);
    uint32_t NotifyOperateEventAfterScreenOn();
    std::set<std::string> GetForegroundBundleNames();
#endif
    uint32_t SendCustomizedScreenEvent(std::shared_ptr<PowerMgrNotify> notify, PowerState state,
        int64_t callTime, const std::string& reason);
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_SCREEN_COMMON_EVENT_CONTROLLER_H