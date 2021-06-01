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

#include "running_lock.h"

#include "power_common.h"
#include "running_lock_token_stub.h"

using std::lock_guard;

namespace OHOS {
namespace PowerMgr {
RunningLock::RunningLock(const wptr<IPowerMgr>& proxy, const std::string& name, RunningLockType type)
    : proxy_(proxy)
{
    runningLockInfo_.name = name;
    runningLockInfo_.type = type;
}

RunningLock::~RunningLock()
{
    UnLock();
}

bool RunningLock::Init()
{
    lock_guard<std::mutex> lock(mutex_);
    token_ = new (std::nothrow)RunningLockTokenStub();
    if (token_ == nullptr) {
        POWER_HILOGE(MODULE_INNERKIT, "RunningLock::%{public}s :creating RunningLockTokenStub error.", __func__);
        return false;
    }
    return true;
}

ErrCode RunningLock::Lock(uint32_t timeOutMs)
{
    lock_guard<std::mutex> lock(mutex_);
    POWER_HILOGI(MODULE_INNERKIT, "RunningLock::%{public}s :timeOutMs = %u.", __func__, timeOutMs);

    sptr<IPowerMgr> proxy = proxy_.promote();
    if (proxy == nullptr) {
        POWER_HILOGE(MODULE_INNERKIT, "RunningLock::%{public}s :proxy nullptr.", __func__);
        return E_GET_POWER_SERVICE_FAILED;
    }
    POWER_HILOGI(MODULE_INNERKIT, "RunningLock::%{public}s :service lock is called", __func__);
    proxy->Lock(token_, runningLockInfo_, timeOutMs);
    if (timeOutMs != 0) {
        usedState_ = RunningLockState::UNKNOWN;
    } else {
        usedState_ = RunningLockState::USED;
    }
    return ERR_OK;
}

ErrCode RunningLock::UnLock()
{
    lock_guard<std::mutex> lock(mutex_);
    POWER_HILOGI(MODULE_INNERKIT, "RunningLock::%{public}s.", __func__);
    if (!CheckUsedNoLock()) {
        return ERR_OK;
    }
    sptr<IPowerMgr> proxy = proxy_.promote();
    if (proxy == nullptr) {
        POWER_HILOGE(MODULE_INNERKIT, "RunningLock::%{public}s :proxy nullptr.", __func__);
        return E_GET_POWER_SERVICE_FAILED;
    }
    POWER_HILOGI(MODULE_INNERKIT, "RunningLock::%{public}s :really called service UnLock.", __func__);
    proxy->UnLock(token_);
    usedState_ = RunningLockState::UNUSED;
    return ERR_OK;
}

bool RunningLock::CheckUsedNoLock()
{
    if (usedState_ <= RunningLockState::USED) {
        return (usedState_ == RunningLockState::USED);
    }
    sptr<IPowerMgr> proxy = proxy_.promote();
    if (proxy == nullptr) {
        POWER_HILOGE(MODULE_INNERKIT, "RunningLock::%{public}s :proxy nullptr.", __func__);
        return false;
    }
    bool ret = proxy->IsUsed(token_);
    if (!ret) {
        // only ret false can update the unknown state.
        usedState_ = RunningLockState::UNUSED;
        return false;
    }
    POWER_HILOGI(MODULE_INNERKIT, "RunningLock::%{public}s, usedState_ = %d.", __func__, usedState_);
    return true;
}

bool RunningLock::IsUsed()
{
    lock_guard<std::mutex> lock(mutex_);
    POWER_HILOGI(MODULE_INNERKIT, "RunningLock::%{public}s.", __func__);
    return CheckUsedNoLock();
}

ErrCode RunningLock::SetWorkTriggerList(const WorkTriggerList& workTriggerList)
{
    lock_guard<std::mutex> lock(mutex_);
    auto& list = runningLockInfo_.workTriggerlist;
    list.clear();
    for (auto& w : workTriggerList) {
        if (w != nullptr) {
            list.push_back(w);
        }
    }
    if (!CheckUsedNoLock()) {
        // no need to notify service when the lock is not used.
        return ERR_OK;
    }

    sptr<IPowerMgr> proxy = proxy_.promote();
    if (proxy == nullptr) {
        POWER_HILOGE(MODULE_INNERKIT, "RunningLock::%{public}s :proxy nullptr.", __func__);
        return E_GET_POWER_SERVICE_FAILED;
    }
    POWER_HILOGI(MODULE_INNERKIT, "RunningLock::%{public}s :service SetWorkTriggerList is called.", __func__);
    proxy->SetWorkTriggerList(token_, runningLockInfo_.workTriggerlist);
    return ERR_OK;
}

const WorkTriggerList& RunningLock::GetWorkTriggerList() const
{
    return runningLockInfo_.workTriggerlist;
}
} // namespace PowerMgr
} // namespace OHOS
