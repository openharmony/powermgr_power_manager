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

#ifndef POWERMGR_MOCK_STATE_ACTION_H
#define POWERMGR_MOCK_STATE_ACTION_H

#include <gmock/gmock.h>
#include "actions/idevice_state_action.h"

namespace OHOS {
namespace PowerMgr {
class MockStateAction : public IDeviceStateAction {
public:
    MockStateAction() = default;
    virtual ~MockStateAction() = default;
    MOCK_METHOD3(Suspend, void(int64_t callTimeMs, SuspendDeviceType type, uint32_t flags));
    MOCK_METHOD0(ForceSuspend, void());
    MOCK_METHOD4(Wakeup, void(int64_t callTimeMs, WakeupDeviceType type,
        const std::string& details, const std::string& pkgName));
    MOCK_METHOD3(RefreshActivity, void(int64_t callTimeMs, UserActivityType type, uint32_t flags));
    MOCK_METHOD0(GetDisplayState, DisplayState());
    MOCK_METHOD2(SetDisplayState, uint32_t(DisplayState state, StateChangeReason reason));
    MOCK_METHOD3(GoToSleep, uint32_t(std::function<void()> onSuspend, std::function<void()> onWakeup, bool force));
    MOCK_METHOD1(RegisterCallback, void(std::function<void(uint32_t)>& callback));
};
} // namespace PowerMgr
} // namespace OHOS

#endif // POWERMGR_MOCK_STATE_ACTION_H