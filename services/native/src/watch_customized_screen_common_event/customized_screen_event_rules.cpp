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

#include "customized_screen_event_rules.h"
#ifdef POWER_MANAGER_ENABLE_WATCH_CUSTOMIZED_SCREEN_COMMON_EVENT_RULES
#include "json/json.h"
#include "power_ext_intf_wrapper.h"
#endif

namespace OHOS {
namespace PowerMgr {
#ifdef POWER_MANAGER_ENABLE_WATCH_CUSTOMIZED_SCREEN_COMMON_EVENT_RULES
static std::vector<std::string> bundleNames{};

std::vector<std::string> CustomizedScreenEventRules::GetForegroundBundleNames()
{
    if (!bundleNames.empty()) {
        return bundleNames;
    }
    std::vector<OHOS::AppExecFwk::AppStateData> appList;
    AppManagerUtils::GetForegroundApplications(appList);
    for (const auto &curApp : appList) {
        bundleNames.push_back(curApp.bundleName);
    }
    return bundleNames;
}

void CustomizedScreenEventRules::SetScreenOnEventRules(StateChangeReason reason)
{
    PowerExtIntfWrapper::Instance().SetScreenOnEventRules(reason);
}

void CustomizedScreenEventRules::PublishCustomizedScreenEvent(PowerState state)
{
    PowerExtIntfWrapper::Instance().PublishCustomizedScreenEvent(state, GetForegroundBundleNames());
}

bool CustomizedScreenEventRules::NotifyScreenOnEventAgain(WakeupDeviceType reason)
{
    return PowerExtIntfWrapper::Instance().NotifyScreenOnEventAgain(reason, GetForegroundBundleNames());
}

void CustomizedScreenEventRules::NotifyOperateEventAfterScreenOn()
{
    PowerExtIntfWrapper::Instance().NotifyOperateEventAfterScreenOn(GetForegroundBundleNames());
}
#endif

void CustomizedScreenEventRules::SendCustomizedScreenEvent(
    std::shared_ptr<PowerMgrNotify> notify, PowerState state, int64_t callTime)
{
#ifdef POWER_MANAGER_ENABLE_WATCH_CUSTOMIZED_SCREEN_COMMON_EVENT_RULES
    if (state == PowerState::AWAKE) {
        PublishCustomizedScreenEvent(PowerState::AWAKE);
    } else if (state == PowerState::INACTIVE) {
        PublishCustomizedScreenEvent(PowerState::INACTIVE);
    }
#else
    if (state == PowerState::AWAKE) {
        notify->PublishScreenOnEvents(callTime);
    } else if (state == PowerState::INACTIVE) {
        notify->PublishScreenOffEvents(callTime);
    }
#endif
}

}  //namespace PowerMgr
}  //namespace OHOS