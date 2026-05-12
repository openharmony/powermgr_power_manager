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

#include "adapter/iswitch_action.h"

#include "power_mgr_service.h"
#include "power_log.h"
#include <datetime_ex.h>
#include <display_manager_lite.h>
#include <screen_manager_lite.h>

namespace OHOS {
namespace PowerMgr {
SwitchActionRet DualScreenSwitchAction::DoWakeupInClosedState()
{
    POWER_HILOGI(FEATURE_POWER_STATE, "[UL_POWER] %{public}s Enter", __func__);
    return SwitchActionRet::HANDLED;
}

SwitchActionRet DualScreenSwitchAction::DoSwitchOpen()
{
    POWER_HILOGI(FEATURE_POWER_STATE, "[UL_POWER] %{public}s Enter", __func__);
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    bool isScreenOn = pms->IsScreenOn();
    Rosen::DisplayManagerLite::GetInstance().SetScreenSwitchState(Rosen::ScreenClosedState::OPEN, isScreenOn);
    return SwitchActionRet::HANDLED;
}

SwitchActionRet DualScreenSwitchAction::DoSwitchClose()
{
    POWER_HILOGI(FEATURE_POWER_STATE, "[UL_POWER] %{public}s Enter", __func__);
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    bool isScreenOn = pms->IsScreenOn();
    Rosen::DisplayManagerLite::GetInstance().SetScreenSwitchState(Rosen::ScreenClosedState::CLOSE, isScreenOn);
    pms->RefreshActivity(GetTickCount(), UserActivityType::USER_ACTIVITY_TYPE_SWITCH, false);
    return SwitchActionRet::HANDLED;
}

SwitchActionRet DualScreenSwitchAction::DoReportSwitchState()
{
    POWER_HILOGI(FEATURE_POWER_STATE, "[UL_POWER] %{public}s Enter", __func__);
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    bool isScreenOn = pms->IsScreenOn();
    auto stateMachine = pms->GetPowerStateMachine();
    RETURN_IF_WITH_RET(stateMachine == nullptr, SwitchActionRet::DEFAULT);
    bool isSwitchOpen = stateMachine->IsSwitchOpenByPath();
    Rosen::DisplayManagerLite::GetInstance().SetScreenSwitchState(
        isSwitchOpen ? Rosen::ScreenClosedState::OPEN : Rosen::ScreenClosedState::CLOSE, isScreenOn);
    return SwitchActionRet::HANDLED;
}

SwitchActionRet DualScreenSwitchAction::DoIsScreenOn()
{
    POWER_HILOGI(FEATURE_POWER_STATE, "[UL_POWER] %{public}s Enter", __func__);
    std::vector<uint64_t> screenIds;
    Rosen::ScreenManagerLite::GetInstance().GetPhysicalScreenIds(screenIds);
    for (uint64_t id : screenIds) {
        Rosen::ScreenPowerState state = Rosen::ScreenManagerLite::GetInstance().GetScreenPower(id);
        if (state == Rosen::ScreenPowerState::POWER_ON) {
            POWER_HILOGE(FEATURE_POWER_STATE, "[UL_POWER] DoIsScreenOn state:%{public}d, id:%{public}d",
                static_cast<int32_t>(state), static_cast<int32_t>(id));
            return SwitchActionRet::IS_SCREEN_ON;
        }
    }
    return SwitchActionRet::HANDLED;
}

} // namespace PowerMgr
} // namespace OHOS