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
class RunningLockInner {
public:
    RunningLockInner(const RunningLockParam& runningLockParam);
    ~RunningLockInner() = default;

    static std::shared_ptr<RunningLockInner> CreateRunningLockInner(const RunningLockParam& runningLockParam);
    void DumpInfo(const std::string& description);

    const std::string& GetRunningLockName() const
    {
        return runningLockParam_.name;
    }
    RunningLockType GetRunningLockType() const
    {
        return runningLockParam_.type;
    }
    int32_t GetRunningLockPid() const
    {
        return runningLockParam_.pid;
    }
    int32_t GetRunningLockUid() const
    {
        return runningLockParam_.uid;
    }
    void SetRunningLockTimeOutMs(int32_t timeoutMs)
    {
        runningLockParam_.timeoutMs = timeoutMs;
    }
    int32_t GetRunningLockTimeOutMs() const
    {
        return runningLockParam_.timeoutMs;
    }
    const RunningLockParam& GetRunningLockParam() const
    {
        return runningLockParam_;
    }
    void SetEnabled(bool isEnabled)
    {
        isEnabled_ = isEnabled;
    }
    bool GetEnabled() const
    {
        return isEnabled_;
    }
    void SetProxied(bool isProxied)
    {
        isProxied_ = isProxied;
    }
    bool GetProxied() const
    {
        return isProxied_;
    }
    void SetNeedRestoreLock(bool need)
    {
        needRestoreLock_ = need;
    }
    bool GetNeedRestoreLock() const
    {
        return needRestoreLock_;
    }
    void SetOverTimeFlag(bool overTimeFlag)
    {
        overTimeFlag_ = overTimeFlag;
    }
    bool GetOverTimeFlag() const
    {
        return overTimeFlag_;
    }
    int64_t GetLockTimeMs() const
    {
        return lockTimeMs_;
    }

private:
    std::mutex mutex_;
    RunningLockParam runningLockParam_;
    bool isEnabled_ {false};
    bool isProxied_ {false};
    bool needRestoreLock_ {false};
    bool overTimeFlag_ {false};
    int64_t lockTimeMs_ {0};
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_RUNNING_LOCK_INNER_H
