/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
RunningLockInner::RunningLockInner(const RunningLockParam& runningLockParam)
{
    runningLockParam_ = runningLockParam;
    lockTimeMs_ = GetTickCount();
}

std::shared_ptr<RunningLockInner> RunningLockInner::CreateRunningLockInner(const RunningLockParam& runningLockParam)
{
    std::shared_ptr<RunningLockInner> runningLockInner = std::make_shared<RunningLockInner>(runningLockParam);
    if (runningLockInner == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "RunningLockInner is nullptr");
        return nullptr;
    }
    POWER_HILOGI(FEATURE_RUNNING_LOCK, "name: %{public}s, type: %{public}d lockid: %{public}s",
        runningLockParam.name.c_str(), runningLockParam.type, std::to_string(runningLockParam.lockid).c_str());
    return runningLockInner;
}

void RunningLockInner::DumpInfo(const std::string& description)
{
    // this statement used to debug, can't find isDebugEnabled() interface. will be replaced later.
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "description: %{public}s, name: %{public}s, type: %{public}d,",
        description.c_str(), runningLockParam_.name.c_str(), runningLockParam_.type);
}
} // namespace PowerMgr
} // namespace OHOS
