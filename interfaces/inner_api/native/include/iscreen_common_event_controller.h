/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef POWERMGR_ISCREEN_COMMON_EVENT_CONTROLLER_H
#define POWERMGR_ISCREEN_COMMON_EVENT_CONTROLLER_H

#include "power_state_machine_info.h"
#include <set>

namespace OHOS {
namespace PowerMgr {
class IScreenCommonEventController {
public:
    IScreenCommonEventController() = default;
    virtual ~IScreenCommonEventController() = default;
    virtual uint32_t SetScreenOnCommonEventRules(StateChangeReason reason) = 0;
    virtual uint32_t SendCustomizedScreenEvent(PowerState state, const std::set<std::string>& bundleNames) = 0;
    virtual uint32_t NotifyOperateEventAfterScreenOn() = 0;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_ISCREEN_COMMON_EVENT_CONTROLLER_H