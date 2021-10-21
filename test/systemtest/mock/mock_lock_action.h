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

#ifndef POWERMGR_MOCK_LOCK_ACTION_H
#define POWERMGR_MOCK_LOCK_ACTION_H

#include <gmock/gmock.h>
#include "actions/irunning_lock_action.h"

namespace OHOS {
namespace PowerMgr {
class MockLockAction : public IRunningLockAction {
public:
    MockLockAction() = default;
    virtual ~MockLockAction() = default;
    MOCK_METHOD1(Acquire, void(RunningLockType type));
    MOCK_METHOD1(Release, void(RunningLockType type));
    MOCK_METHOD2(Lock, void(RunningLockType type, const std::string& tag));
    MOCK_METHOD2(Unlock, void(RunningLockType type, const std::string& tag));
};
} // namespace PowerMgr
} // namespace OHOS

#endif // POWERMGR_MOCK_POWER_ACTION_H