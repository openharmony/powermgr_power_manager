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

#include "running_lock_inner.h"

#include <datetime_ex.h>

#include "power_log.h"

namespace OHOS {
namespace PowerMgr {
RunningLockInner::RunningLockInner(const RunningLockInfo& runningLockInfo, const UserIPCInfo &userIPCinfo)
{
    runningLockInfo_.name = runningLockInfo.name;
    runningLockInfo_.type = runningLockInfo.type;
    SetWorkTriggerList(runningLockInfo.workTriggerlist);
    userIPCinfo_ = userIPCinfo;
    lockTimeMs_ = GetTickCount();
}

std::shared_ptr<RunningLockInner> RunningLockInner::CreateRunningLockInner(const RunningLockInfo& runningLockInfo,
    const UserIPCInfo &userIPCinfo)
{
    std::shared_ptr<RunningLockInner> runningLockInner = std::make_shared<RunningLockInner>(runningLockInfo,
        userIPCinfo);
    if (runningLockInner == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "RunningLockInner is nullptr");
        return nullptr;
    }
    POWER_HILOGI(FEATURE_RUNNING_LOCK, "name: %{public}s, type: %{public}d", runningLockInfo.name.c_str(),
        runningLockInfo.type);
    return runningLockInner;
}

void RunningLockInner::SetWorkTriggerList(const WorkTriggerList& workTriggerList)
{
    DumpInfo("before");
    runningLockInfo_.workTriggerlist = workTriggerList;
    DumpInfo("after");
}

void RunningLockInner::DumpInfo(const std::string& description)
{
    // this statement used to debug, can't find isDebugEnabled() interface. will be replaced later.
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "description: %{public}s, name: %{public}s, type: %{public}d,",
        description.c_str(), runningLockInfo_.name.c_str(), runningLockInfo_.type);

    auto& list = runningLockInfo_.workTriggerlist;
    for (auto& worker : list) {
        POWER_HILOGD(FEATURE_RUNNING_LOCK, "use_count: %{public}ld, name: %{public}s, uid: %{public}d,\
            pid: %{public}d, abilityId: %{public}d", worker.use_count(), worker->GetName().c_str(),
            worker->GetUid(), worker->GetPid(), worker->GetAbilityId());
    }
}
} // namespace PowerMgr
} // namespace OHOS
