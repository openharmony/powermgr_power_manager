/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef POWERMGR_IRUNNING_LOCK_ACTION_INFO_H
#define POWERMGR_IRUNNING_LOCK_ACTION_INFO_H

#include "running_lock_info.h"

namespace OHOS {
namespace PowerMgr {
/**
 * Enumerates runninglock operation return status
 */
enum RunningLockStatus : int32_t {
    /**
     * RunningLock status: runninglock operation is successful
     */
    RUNNINGLOCK_SUCCESS  = 0,
    /**
     * RunningLock status: runninglock operation is failed
     */
    RUNNINGLOCK_FAILURE = -1,
    /**
     * RunningLock status: runninglock operation is not supported, not found the runninglock
     */
    RUNNINGLOCK_NOT_SUPPORT = -2,
    /**
     * RunningLock status: invalid input parameter for runninglock operation
     */
    RUNNINGLOCK_INVALID_PARAM = -3,
};

/**
 * Runninglock parameter: used to convert to HDI RunningLockInfo
 */
struct RunningLockParam {
    uint64_t lockid = 0;
    std::string name;
    std::string bundleName;
    RunningLockType type = RunningLockType::RUNNINGLOCK_BUTT;
    int32_t timeoutMs = -1;
    int32_t pid = 0;
    int32_t uid = 0;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_IRUNNING_LOCK_ACTION_INFO_H
