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

#include "screen_common_event_controller.h"
#ifdef POWER_MANAGER_ENABLE_WATCH_CUSTOMIZED_SCREEN_COMMON_EVENT_RULES
#include "app_manager_utils.h"
#include "power_hookmgr.h"
#endif

namespace OHOS {
namespace PowerMgr {
#ifdef POWER_MANAGER_ENABLE_WATCH_CUSTOMIZED_SCREEN_COMMON_EVENT_RULES
std::shared_ptr<IScreenCommonEventController> g_screenCommonEventController = nullptr;

uint32_t ScreenCommonEventController::InitScreenCommonEventController()
{
    POWER_HILOGI(FEATURE_WAKEUP, "InitScreenCommonEventController start.");
    HOOK_MGR *hookMgr = GetPowerHookMgr();
    if (hookMgr == nullptr) {
        POWER_HILOGE(FEATURE_WAKEUP, "InitScreenCommonEventController hookMgr is null.");
        return ERROR_FAIL;
    }
    ScreenCommonEventCustomizedContext context = {};
    HookMgrExecute(hookMgr, static_cast<int32_t>(PowerHookStage::POWER_SCREEN_COMMON_EVENT_CUSTOMIZED_CONTROLLER_INIT),
        &context, nullptr);
    if (context.controllerPtr == nullptr) {
        POWER_HILOGE(FEATURE_WAKEUP, "InitScreenCommonEventController controllerPtr is null.");
        return ERROR_FAIL;
    }
    g_screenCommonEventController = context.controllerPtr;
    POWER_HILOGI(FEATURE_WAKEUP, "InitScreenCommonEventController finished.");
    return ERROR_OK;
}

uint32_t ScreenCommonEventController::SetScreenOnCommonEventRules(StateChangeReason reason)
{
    if (g_screenCommonEventController == nullptr) {
        uint32_t result = InitScreenCommonEventController();
        if (result == ERROR_FAIL) {
            return ERROR_FAIL;
        }
    }
    return g_screenCommonEventController->SetScreenOnCommonEventRules(reason);
}

uint32_t ScreenCommonEventController::NotifyOperateEventAfterScreenOn()
{
    if (g_screenCommonEventController == nullptr) {
        POWER_HILOGE(FEATURE_WAKEUP, "NotifyOperateEventAfterScreenOn g_screenCommonEventController is null.");
        return ERROR_FAIL;
    }
    return g_screenCommonEventController->NotifyOperateEventAfterScreenOn();
}

std::set<std::string> ScreenCommonEventController::GetForegroundBundleNames()
{
    std::set<std::string> bundleNames {};
    AppManagerUtils::GetForegroundBundleNames(bundleNames);
    return bundleNames;
}
#endif

uint32_t ScreenCommonEventController::SendCustomizedScreenEvent(
    std::shared_ptr<PowerMgrNotify> notify, PowerState state, int64_t callTime, const std::string& reason)
{
#ifdef POWER_MANAGER_ENABLE_WATCH_CUSTOMIZED_SCREEN_COMMON_EVENT_RULES
    if (g_screenCommonEventController == nullptr) {
        POWER_HILOGE(FEATURE_WAKEUP, "SendCustomizedScreenEvent g_screenCommonEventController is null.");
        return ERROR_FAIL;
    }
    return g_screenCommonEventController->SendCustomizedScreenEvent(state, GetForegroundBundleNames());
#else
    if (state == PowerState::AWAKE) {
        notify->PublishScreenOnEvents(callTime, reason);
    }
    if (state == PowerState::INACTIVE) {
        notify->PublishScreenOffEvents(callTime, reason);
    }
#endif
    return ERROR_OK;
}

}  //namespace PowerMgr
}  //namespace OHOS
