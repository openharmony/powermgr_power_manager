/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef POWERMGR_RUNNING_LOCK_H
#define POWERMGR_RUNNING_LOCK_H

#include <memory>
#include <mutex>

#include <iremote_object.h>

#include "ipower_mgr.h"
#include "running_lock_info.h"

namespace OHOS {
namespace PowerMgr {
class RunningLock {
public:
    RunningLock(const wptr<IPowerMgr>& proxy, const std::string& name, RunningLockType type);
    ~RunningLock();
    DISALLOW_COPY_AND_MOVE(RunningLock);

    PowerErrors Init();
    bool IsUsed();

    /**
     * Acquires a runninglock.
     * The parameter of last called will replace that of the previous called, and the effective parameter will
     * be that of the last called. Eg : If call Lock(n) first then Lock(), a lasting lock will be hold, and
     * need UnLock() to be called to release it. If else call Lock() first and then Lock(n), the parameter 'n'
     * takes effect and it will be released in n ms automatiocly.
     * @param timeOutMs timeOutMs this runninglock will be released in timeOutMs milliseconds. If it is called without
     *                  parameter or parameter is 0 a lasting runninglock will be hold.
     */
    ErrCode Lock(uint32_t timeOutMs = 0);

    /**
     * Release the runninglock, no matter how many times Lock() was called.
	 * The release can be done by calling UnLock() only once.
     */
    ErrCode UnLock();

    /**
     * Maintain the information about the application that attempts to acquire the runninglock.
     */
    ErrCode SetWorkTriggerList(const WorkTriggerList& list);
    const WorkTriggerList& GetWorkTriggerList() const;
    static constexpr uint32_t MAX_NAME_LEN = 256;
    static constexpr uint32_t CREATE_WITH_SCREEN_ON = 0x10000000;

private:
    PowerErrors Create();
    void Release();
    std::mutex mutex_;
    RunningLockInfo runningLockInfo_;
    sptr<IRemoteObject> token_;
    wptr<IPowerMgr> proxy_;
    bool CheckUsedNoLock();
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_RUNNING_LOCK_H
