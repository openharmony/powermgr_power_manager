/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "actions/irunning_lock_action.h"

#include "power_log.h"

using namespace std;

namespace OHOS {
namespace PowerMgr {
const array<string, ToUnderlying(RunningLockType::RUNNINGLOCK_BUTT)> IRunningLockAction::LOCK_TAGS {
    "OHOSPowerMgr.Screen", "OHOSPowerMgr.Background", "OHOSPowerMgr.Proximity",
};

void IRunningLockAction::Acquire(RunningLockType type)
{
    if (!IsValidType(type)) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Invalid runninglock type");
        return;
    }

    auto t = ToUnderlying(type);
    RunningLockDesc& desc = lockDescs_[t];
    if (desc.IsRefNone()) {
        Lock(type, GetLockTag(type));
    }
    desc.IncRef();
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "Acquire runninglock, type: %{public}u, refCnt: %{public}u", t,
        desc.GetRefCnt());
}

void IRunningLockAction::Release(RunningLockType type)
{
    if (!IsValidType(type)) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Invalid runninglock type");
        return;
    }

    auto t = ToUnderlying(type);
    RunningLockDesc& desc = lockDescs_[t];
    if (desc.IsRefNone()) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Invalid refCnt of wakelock: %{public}u", t);
        return;
    }

    desc.DecRef();
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "Release runninglock, type: %{public}u, refCnt: %{public}u", t,
        desc.GetRefCnt());
    if (desc.IsRefNone()) {
        Unlock(type, GetLockTag(type));
    }
}
} // namespace PowerMgr
} // namespace OHOS
