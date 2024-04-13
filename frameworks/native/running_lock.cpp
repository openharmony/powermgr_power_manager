/*
 * Copyright (C) 2021-2023 Huawei Device Co., Ltd.
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
#include <timer.h>
#include "errors.h"
#include "new"
#include "refbase.h"
#include "power_log.h"
#include "power_mgr_errors.h"
#include "running_lock_token_stub.h"
#include "power_mgr_client.h"
#include "running_lock_timer_handler.h"

namespace OHOS {
namespace PowerMgr {
constexpr int32_t DEFAULT_TIMEOUT = 3000;
RunningLock::RunningLock(const std::string& name, RunningLockType type)
{
    runningLockInfo_.name = name;
    runningLockInfo_.type = type;
}

RunningLock::~RunningLock()
{
    if (token_ != nullptr) {
        UnLock();
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
    PowerErrors ret = Create();
    if (ret == PowerErrors::ERR_OK) {
        PowerMgrClient::GetInstance().UpdateRunningLockSet(shared_from_this(), false);
    }
    return ret;
}

PowerErrors RunningLock::Create()
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto& pmc = PowerMgrClient::GetInstance();
    sptr<IPowerMgr> proxy = pmc.GetProxy();
    if (proxy == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Proxy is a null pointer");
        return PowerErrors::ERR_CONNECTION_FAIL;
    }
    return proxy->CreateRunningLock(token_, runningLockInfo_);
}

void RunningLock::Recover()
{
    Create();
    if (state_ == true) {
        Lock(timeOutMs_);
    }
    POWER_HILOGI(FEATURE_RUNNING_LOCK, "recover runningLocks end");
}

ErrCode RunningLock::Lock(int32_t timeOutMs)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto& pmc = PowerMgrClient::GetInstance();
    sptr<IPowerMgr> proxy = pmc.GetProxy();
    if (proxy == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Proxy is a null pointer");
        return E_GET_POWER_SERVICE_FAILED;
    }
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "Service side Lock call, timeOutMs=%{public}d", timeOutMs);
    if (!proxy->Lock(token_)) {
        return E_INNER_ERR;
    }
    state_ = true;
    if (timeOutMs == 0) {
        timeOutMs = DEFAULT_TIMEOUT;
        POWER_HILOGW(FEATURE_RUNNING_LOCK, "use default timeout");
    }
    if (timeOutMs > 0) {
        std::function<void()> task = std::bind(&RunningLock::UnLock, this);
        RunningLockTimerHandler::GetInstance().RegisterRunningLockTimer(token_, task, timeOutMs);
    }
    timeOutMs_ = timeOutMs;
    return ERR_OK;
}

ErrCode RunningLock::UnLock()
{
    std::lock_guard<std::mutex> lock(mutex_);
    RunningLockTimerHandler::GetInstance().UnregisterRunningLockTimer(token_);
    if (state_ == false) {
        POWER_HILOGW(FEATURE_RUNNING_LOCK, "RunningLock is already UnLock");
        return ERR_OK;
    }
    auto& pmc = PowerMgrClient::GetInstance();
    sptr<IPowerMgr> proxy = pmc.GetProxy();
    if (proxy == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Proxy is a null pointer");
        return E_GET_POWER_SERVICE_FAILED;
    }
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "Service side UnLock call");
    if (!proxy->UnLock(token_)) {
        return E_INNER_ERR;
    }
    state_ = false;
    return ERR_OK;
}

bool RunningLock::IsUsed()
{
    auto& pmc = PowerMgrClient::GetInstance();
    sptr<IPowerMgr> proxy = pmc.GetProxy();
    if (proxy == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Proxy is a null pointer");
        return false;
    }
    bool ret = proxy->IsUsed(token_);
    POWER_HILOGI(FEATURE_RUNNING_LOCK, "Is Used: %{public}d", ret);
    return ret;
}

void RunningLock::Release()
{
    auto& pmc = PowerMgrClient::GetInstance();
    sptr<IPowerMgr> proxy = pmc.GetProxy();
    if (proxy == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Proxy is a null pointer");
        return;
    }
    POWER_HILOGI(FEATURE_RUNNING_LOCK, "Service side ReleaseRunningLock call");
    proxy->ReleaseRunningLock(token_);
    pmc.UpdateRunningLockSet(shared_from_this(), true);
}
} // namespace PowerMgr
} // namespace OHOS
