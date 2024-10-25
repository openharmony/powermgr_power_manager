/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "cj_running_lock_impl.h"
#include "power_mgr_client.h"
#include "running_lock.h"
#include "running_lock_info.h"

namespace OHOS {
namespace PowerMgr {
CJRunningLock::CJRunningLock(std::string name, int32_t num_type)
{
    RunningLockType type = RunningLockType(num_type);
    std::shared_ptr<RunningLock> runningLock = PowerMgrClient::GetInstance().CreateRunningLock(name, type);
    PowerErrors error = PowerMgrClient::GetInstance().GetError();
    runningLockToken_ = runningLock;
    error_ = error;
}

std::shared_ptr<RunningLock> CJRunningLock::GetRunningLock()
{
    return runningLockToken_;
}

PowerErrors CJRunningLock::GetError()
{
    return error_;
}

} // namespace PowerMgr
} // namespace OHOS