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

#ifndef I_RUNNING_LOCK_CHANGED_CALLBACK_H
#define I_RUNNING_LOCK_CHANGED_CALLBACK_H

#include <iremote_broker.h>

namespace OHOS {
namespace PowerMgr {
enum class RunningLockChangeState : uint32_t {
    RUNNINGLOCK_STATE_LOCKED = 0,
    RUNNINGLOCK_STATE_UNLOCKED = 1,
};

class IRunningLockChangedCallback : public IRemoteBroker {
public:
    virtual void OnAsyncScreenRunningLockChanged(RunningLockChangeState state) = 0;

    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.powermgr.IRunningLockChangedCallback");
};
} // namespace PowerMgr
} // namespace OHOS
#endif // I_RUNNING_LOCK_CHANGED_CALLBACK_H
