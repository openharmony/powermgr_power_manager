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

#ifndef POWERMGR_RUNNING_LOCK_INNER_H
#define POWERMGR_RUNNING_LOCK_INNER_H

#include <mutex>

#include "power_common.h"
#include "running_lock_info.h"

namespace OHOS {
namespace PowerMgr {
struct UserIPCInfo {
    pid_t uid = INVALID_UID;
    pid_t pid = INVALID_PID;

    bool operator==(const UserIPCInfo& other) const
    {
        return (((uid == other.uid) && (pid == other.pid)) ||
                ((uid == other.uid) && (other.pid == INVALID_PID)) ||
                ((other.uid == INVALID_PID) && (pid == other.pid)) ||
                ((other.uid == INVALID_PID) && (other.pid == INVALID_PID)));
    }
};

class RunningLockInner {
public:
    RunningLockInner(const RunningLockInfo& runningLockInfo, const UserIPCInfo &userIPCinfo);
    ~RunningLockInner() = default;

    static std::shared_ptr<RunningLockInner> CreateRunningLockInner(const RunningLockInfo& runningLockInfo,
        const UserIPCInfo &userIPCinfo);
    void SetWorkTriggerList(const WorkTriggerList& workTriggerList);
    void DumpInfo(const std::string& description);

    RunningLockType GetRunningLockType() const
    {
        return runningLockInfo_.type;
    }
    const std::string& GetRunningLockName() const
    {
        return runningLockInfo_.name;
    }
    const RunningLockInfo& GetRunningLockInfo() const
    {
        return runningLockInfo_;
    }
    const UserIPCInfo& GetUserIPCInfo() const
    {
        return userIPCinfo_;
    }
    void SetDisabled(bool disabled)
    {
        disabled_ = disabled;
    }
    bool GetDisabled() const
    {
        return disabled_;
    }
    void SetReallyLocked(bool reallyLocked)
    {
        reallyLocked_ = reallyLocked;
    }
    bool GetReallyLocked() const
    {
        return reallyLocked_;
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
    RunningLockInfo runningLockInfo_;
    UserIPCInfo userIPCinfo_;
    bool disabled_ {true};
    bool reallyLocked_ {false};
    bool overTimeFlag_ {false};
    int64_t lockTimeMs_ {0};
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_RUNNING_LOCK_INNER_H
