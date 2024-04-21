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

#ifndef POWERMGR_RUNNING_LOCK_INNER_H
#define POWERMGR_RUNNING_LOCK_INNER_H

#include <mutex>

#include "actions/running_lock_action_info.h"
#include "power_common.h"
#include "running_lock_info.h"

namespace OHOS {
namespace PowerMgr {
enum class RunningLockState : uint32_t {
    RUNNINGLOCK_STATE_DISABLE  = 0,
    RUNNINGLOCK_STATE_ENABLE,
    RUNNINGLOCK_STATE_PROXIED,
    RUNNINGLOCK_STATE_UNPROXIED_RESTORE,
};

class RunningLockInner {
public:
    explicit RunningLockInner(const RunningLockParam& runningLockParam);
    ~RunningLockInner() = default;

    static std::shared_ptr<RunningLockInner> CreateRunningLockInner(const RunningLockParam& runningLockParam);
    void DumpInfo(const std::string& description);

    const std::string& GetName() const
    {
        return runningLockParam_.name;
    }

    const std::string& GetBundleName() const
    {
        return runningLockParam_.bundleName;
    }

    RunningLockType GetType() const
    {
        return runningLockParam_.type;
    }
    int32_t GetPid() const
    {
        return runningLockParam_.pid;
    }
    int32_t GetUid() const
    {
        return runningLockParam_.uid;
    }
    void SetTimeOutMs(int32_t timeoutMs)
    {
        runningLockParam_.timeoutMs = timeoutMs;
    }
    int32_t GetTimeOutMs() const
    {
        return runningLockParam_.timeoutMs;
    }
    const RunningLockParam& GetParam() const
    {
        return runningLockParam_;
    }
    void SetState(RunningLockState state)
    {
        state_ = state;
    }
    RunningLockState GetState() const
    {
        return state_;
    }
    bool IsProxied() const
    {
        return state_ == RunningLockState::RUNNINGLOCK_STATE_PROXIED ||
            state_ == RunningLockState::RUNNINGLOCK_STATE_UNPROXIED_RESTORE;
    }
    int64_t GetLockTimeMs() const
    {
        return lockTimeMs_;
    }
    uint64_t GetLockId() const
    {
        return runningLockParam_.lockid;
    }

private:
    std::mutex mutex_;
    RunningLockParam runningLockParam_;
    RunningLockState state_ = RunningLockState::RUNNINGLOCK_STATE_DISABLE;
    int64_t lockTimeMs_ = 0;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_RUNNING_LOCK_INNER_H
