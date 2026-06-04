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

#ifndef POWERMGR_SWITCH_ACTION_H
#define POWERMGR_SWITCH_ACTION_H

#include <cstdint>

namespace OHOS {
namespace PowerMgr {

enum class SwitchActionType : uint32_t {
    WAKEUP_IN_CLOSED_STATE = 0,
    SWITCH_OPEN,
    SWITCH_CLOSE,
    REPORT_SWITCH_STATE,
};

enum class SwitchActionRet : int32_t {
    DEFAULT = 0,
    HANDLED = 1,
};

class ISwitchAction {
public:
    ISwitchAction() = default;
    virtual ~ISwitchAction() = default;

    SwitchActionRet HandleSwitchAction(SwitchActionType type)
    {
        switch (type) {
            case SwitchActionType::WAKEUP_IN_CLOSED_STATE:
                return DoWakeupInClosedState();
            case SwitchActionType::SWITCH_OPEN:
                return DoSwitchOpen();
            case SwitchActionType::SWITCH_CLOSE:
                return DoSwitchClose();
            case SwitchActionType::REPORT_SWITCH_STATE:
                return DoReportSwitchState();
            default:
                return SwitchActionRet::DEFAULT;
        }
    }

private:
    virtual SwitchActionRet DoWakeupInClosedState() { return SwitchActionRet::DEFAULT; }
    virtual SwitchActionRet DoSwitchOpen() { return SwitchActionRet::DEFAULT; }
    virtual SwitchActionRet DoSwitchClose() { return SwitchActionRet::DEFAULT; }
    virtual SwitchActionRet DoReportSwitchState() { return SwitchActionRet::DEFAULT; }
};

class DualScreenSwitchAction : public ISwitchAction {
private:
    virtual SwitchActionRet DoWakeupInClosedState() override;
    virtual SwitchActionRet DoSwitchOpen() override;
    virtual SwitchActionRet DoSwitchClose() override;
    virtual SwitchActionRet DoReportSwitchState() override;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_ISWITCH_ACTION_H
