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

#ifndef POWERMGR_CUSTOMIZED_SCREEN_EVENT_RULES_H
#define POWERMGR_CUSTOMIZED_SCREEN_EVENT_RULES_H

#include "singleton.h"
#include "app_mgr_interface.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "power_mgr_service.h"
#include "power_log.h"
#include <chrono>
#include <ctime>
#include <dlfcn.h>
#ifdef POWER_MANAGER_ENABLE_WATCH_CUSTOMIZED_SCREEN_COMMON_EVENT_RULES
#include "app_manager_utils.h"
#endif
namespace OHOS {
namespace PowerMgr {
class CustomizedScreenEventRules : public DelayedSingleton<CustomizedScreenEventRules> {
public:
    CustomizedScreenEventRules() = default;
#ifdef POWER_MANAGER_ENABLE_WATCH_CUSTOMIZED_SCREEN_COMMON_EVENT_RULES
    static void SetScreenOnEventRules(StateChangeReason reason);
    static void PublishCustomizedScreenEvent(PowerState state);
    static bool NotifyScreenOnEventAgain(WakeupDeviceType reason);
    static void NotifyOperateEventAfterScreenOn();
    static std::vector<std::string> GetForegroundBundleNames();
#endif
    void SendCustomizedScreenEvent(std::shared_ptr<PowerMgrNotify> notify, PowerState state, int64_t callTime);
};
}  //namespace PowerMgr
}  //namespace OHOS
#endif