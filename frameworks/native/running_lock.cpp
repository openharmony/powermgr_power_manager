/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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

#include <cstdint>
#include <list>
#include <iosfwd>
#include <string>
#include "errors.h"
#include "new"
#include "refbase.h"
#include "power_log.h"
#include "power_mgr_errors.h"
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
    if (token_ != nullptr) {
        Release();
    }
}

PowerErrors RunningLock::Init()
{
    token_ = new (std::nothrow)RunningLockTokenStub();
    if (token_ == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Failed to create the RunningLockTokenStub");
        return PowerErrors::ERR_CONNECTION_FAIL;
    }
    return Create();
}

ErrCode RunningLock::Lock(uint32_t timeOutMs)
{
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "Lock timeOutMs: %{public}u", timeOutMs);

    sptr<IPowerMgr> proxy = proxy_.promote();
    if (proxy == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Proxy is a null pointer");
        return E_GET_POWER_SERVICE_FAILED;
    }
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "Service side Lock call");
    proxy->Lock(token_, runningLockInfo_, timeOutMs);

    return ERR_OK;
}

ErrCode RunningLock::UnLock()
{
    if (!CheckUsedNoLock()) {
        return ERR_OK;
    }
    sptr<IPowerMgr> proxy = proxy_.promote();
    if (proxy == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Proxy is a null pointer");
        return E_GET_POWER_SERVICE_FAILED;
    }
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "Service side UnLock call");
    proxy->UnLock(token_);
    return ERR_OK;
}

bool RunningLock::CheckUsedNoLock()
{
    sptr<IPowerMgr> proxy = proxy_.promote();
    if (proxy == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Proxy is a null pointer");
        return false;
    }
    bool ret = proxy->IsUsed(token_);

    POWER_HILOGD(FEATURE_RUNNING_LOCK, "Is Used: %{public}d", ret);
    return ret;
}

bool RunningLock::IsUsed()
{
    return CheckUsedNoLock();
}

ErrCode RunningLock::SetWorkTriggerList(const WorkTriggerList& workTriggerList)
{
    auto& list = runningLockInfo_.workTriggerlist;
    list.clear();
    std::copy_if(workTriggerList.begin(), workTriggerList.end(), std::back_inserter(list), [](auto& work) {
        return work != nullptr;
    });

    if (!CheckUsedNoLock()) {
        // no need to notify service when the lock is not used.
        return ERR_OK;
    }

    sptr<IPowerMgr> proxy = proxy_.promote();
    if (proxy == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Proxy is a null pointer");
        return E_GET_POWER_SERVICE_FAILED;
    }
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "Service side SetWorkTriggerList call");
    proxy->SetWorkTriggerList(token_, runningLockInfo_.workTriggerlist);
    return ERR_OK;
}

const WorkTriggerList& RunningLock::GetWorkTriggerList() const
{
    return runningLockInfo_.workTriggerlist;
}

PowerErrors RunningLock::Create()
{
    sptr<IPowerMgr> proxy = proxy_.promote();
    if (proxy == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Proxy is a null pointer");
        return PowerErrors::ERR_CONNECTION_FAIL;
    }
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "Service side CreateRunningLock call");
    return proxy->CreateRunningLock(token_, runningLockInfo_);
}

void RunningLock::Release()
{
    sptr<IPowerMgr> proxy = proxy_.promote();
    if (proxy == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Proxy is a null pointer");
        return;
    }
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "Service side ReleaseRunningLock call");
    proxy->ReleaseRunningLock(token_);
}
} // namespace PowerMgr
} // namespace OHOS
