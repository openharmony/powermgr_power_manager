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

#ifndef POWERMGR_IRUNNING_LOCK_ACTION_H
#define POWERMGR_IRUNNING_LOCK_ACTION_H

#include <cstdint>
#include <iosfwd>
#include <array>
#include <string>

#include "power_common.h"
#include "running_lock_info.h"

namespace OHOS {
namespace PowerMgr {
class IRunningLockAction {
public:
    IRunningLockAction() = default;
    virtual ~IRunningLockAction() = default;

    static inline std::string GetLockTag(RunningLockType type)
    {
        return LOCK_TAGS[ToUnderlying(type)];
    }

    virtual void Acquire(RunningLockType type);
    virtual void Release(RunningLockType type);

    virtual void Lock(RunningLockType type, const std::string& tag) = 0;
    virtual void Unlock(RunningLockType type, const std::string& tag) = 0;

private:
    class RunningLockDesc {
    public:
        inline void IncRef()
        {
            refCnt++;
        }

        inline void DecRef()
        {
            refCnt--;
        }

        inline bool IsRefNone() const
        {
            return refCnt == 0;
        }

        inline uint32_t GetRefCnt() const
        {
            return refCnt;
        }

    private:
        uint32_t refCnt{0};
    };

    static inline bool IsValidType(RunningLockType type)
    {
        return type < RunningLockType::RUNNINGLOCK_BUTT;
    }

    static const std::array<std::string, ToUnderlying(RunningLockType::RUNNINGLOCK_BUTT)> LOCK_TAGS;

    std::array<RunningLockDesc, ToUnderlying(RunningLockType::RUNNINGLOCK_BUTT)> lockDescs_;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_IRUNNING_LOCK_ACTION_H
