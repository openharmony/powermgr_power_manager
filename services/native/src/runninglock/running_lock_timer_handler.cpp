/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#include "running_lock_timer_handler.h"

#include "common_timer_errors.h"
#include "power_log.h"

namespace OHOS {
namespace PowerMgr {
namespace {
const std::string RUNNINGLOCK_TIMER_HANDLER_NAME = "RunningLockTimer";
}
RunningLockTimerHandler::~RunningLockTimerHandler()
{
    CleanTimer();
}

bool RunningLockTimerHandler::RegisterRunningLockTimer(
    const sptr<IRemoteObject> &token, const std::function<void()> &callback, int32_t timeoutMs)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (handlerTimer_ == nullptr) {
        handlerTimer_ = std::make_unique<OHOS::Utils::Timer>(RUNNINGLOCK_TIMER_HANDLER_NAME);
        handlerTimer_->Setup();
    }
    uint32_t lastTimerId = GetRunningLockTimerId(token);
    if (lastTimerId != OHOS::Utils::TIMER_ERR_DEAL_FAILED) {
        POWER_HILOGW(FEATURE_RUNNING_LOCK, "RunningLockTimer exist, unregister timerId=%{public}d", lastTimerId);
        UnregisterTimer(lastTimerId);
    }
    bool once = true;
    uint32_t curTimerId = handlerTimer_->Register(callback, timeoutMs, once);
    if (curTimerId == OHOS::Utils::TIMER_ERR_DEAL_FAILED) {
        POWER_HILOGW(FEATURE_RUNNING_LOCK, "RegisterRunningLockTimer failed");
        if (lastTimerId != OHOS::Utils::TIMER_ERR_DEAL_FAILED) {
            RemoveRunningLockTimerMap(token);
        }
        return false;
    }
    POWER_HILOGI(FEATURE_RUNNING_LOCK, "AddRunningLockTimer timerid=%{public}u", curTimerId);
    AddRunningLockTimerMap(token, curTimerId);
    return true;
}

bool RunningLockTimerHandler::UnregisterRunningLockTimer(const sptr<IRemoteObject> &token)
{
    std::lock_guard<std::mutex> lock(mutex_);
    uint32_t timerId = GetRunningLockTimerId(token);
    if (timerId != OHOS::Utils::TIMER_ERR_DEAL_FAILED) {
        POWER_HILOGI(FEATURE_RUNNING_LOCK, "Running lock timer is exist, unregister timerId=%{public}d", timerId);
        UnregisterTimer(timerId);
        RemoveRunningLockTimerMap(token);
    }
    return true;
}

uint32_t RunningLockTimerHandler::GetRunningLockTimerId(const sptr<IRemoteObject> &token)
{
    uint32_t timerId = OHOS::Utils::TIMER_ERR_DEAL_FAILED;
    auto iter = runninglockTimerMap_.find(token);
    if (iter != runninglockTimerMap_.end()) {
        timerId = iter->second;
    }
    return timerId;
}

void RunningLockTimerHandler::AddRunningLockTimerMap(const sptr<IRemoteObject> &token, uint32_t timerId)
{
    auto iter = runninglockTimerMap_.find(token);
    if (iter == runninglockTimerMap_.end()) {
        runninglockTimerMap_.emplace(token, timerId);
        return;
    }
    iter->second = timerId;
}

void RunningLockTimerHandler::RemoveRunningLockTimerMap(const sptr<IRemoteObject> &token)
{
    auto iter = runninglockTimerMap_.find(token);
    if (iter != runninglockTimerMap_.end()) {
        runninglockTimerMap_.erase(token);
    }
}

void RunningLockTimerHandler::UnregisterTimer(uint32_t timerId)
{
    if (handlerTimer_ != nullptr) {
        handlerTimer_->Unregister(timerId);
    }
}

void RunningLockTimerHandler::CleanTimer()
{
    if (handlerTimer_ != nullptr) {
        for (auto& iter : runninglockTimerMap_) {
            UnregisterTimer(iter.second);
        }
        runninglockTimerMap_.clear();
        handlerTimer_->Shutdown();
    }
}
} // namespace PowerMgr
} // namespace OHOS
