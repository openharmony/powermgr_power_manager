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

#include "suspend/suspend_controller.h"

#include <file_ex.h>
#include <sys/eventfd.h>

#include "errors.h"
#include "pubdef.h"

#include "hilog_wrapper.h"

namespace OHOS {
namespace PowerMgr {
namespace Suspend {
SuspendController::SuspendController()
{
    auto f = std::bind(&SuspendController::WaitingSuspendCondition, this);
    suspend_ = std::make_unique<AutoSuspend>(f);
}

void SuspendController::AutoSuspend::AutoSuspendLoop()
{
    while (true) {
        std::this_thread::sleep_for(waitTime_);
        const std::string wakeupCount = WaitWakeupCount();
        if (wakeupCount.empty()) {
            continue;
        }
        waitingFunc_();
        if (!WriteWakeupCount(wakeupCount)) {
            continue;
        }
        bool success = SuspendEnter();
        if (!success) {
            POWER_HILOGE(MODULE_SERVICE, "Start suspend failed!");
        }
    }
}

void SuspendController::AutoSuspend::Start()
{
    static bool started = false;
    if (started) {
        return;
    }
    daemon_ = std::make_unique<std::thread>(&AutoSuspend::AutoSuspendLoop, this);
    daemon_->detach();
    started = true;
}

bool SuspendController::AutoSuspend::SuspendEnter()
{
    static bool inited = false;
    static UniqueFd suspendStateFd(TEMP_FAILURE_RETRY(open(SUSPEND_STATE_PATH, O_RDWR | O_CLOEXEC)));
    if (!inited) {
        if (suspendStateFd < 0) {
            POWER_HILOGE(MODULE_SERVICE, "Failed to open the suspending state fd!");
            return false;
        }
        inited = true;
    }
    bool ret = SaveStringToFd(suspendStateFd, SUSPEND_STATE);
    if (!ret) {
        POWER_HILOGE(MODULE_SERVICE, "Failed to write the suspending state!");
    }
    return ret;
}

std::string SuspendController::AutoSuspend::WaitWakeupCount()
{
    if (wakeupCountFd < 0) {
        wakeupCountFd = UniqueFd(TEMP_FAILURE_RETRY(open(WAKEUP_COUNT_PATH, O_RDWR | O_CLOEXEC)));
    }
    std::string wakeupCount;
    bool ret = LoadStringFromFd(wakeupCountFd, wakeupCount);
    if (!ret) {
        POWER_HILOGW(MODULE_SERVICE, "Read wakeup count failed!");
        return std::string();
    }
    return wakeupCount;
}

bool SuspendController::AutoSuspend::WriteWakeupCount(std::string wakeupCount)
{
    if (wakeupCountFd < 0) {
        return false;
    }
    bool ret = SaveStringToFd(wakeupCountFd, wakeupCount.c_str());
    if (!ret) {
        POWER_HILOGE(MODULE_SERVICE, "Failed to write the wakeup count!");
    }
    return ret;
}

void SuspendController::EnableSuspend()
{
    suspend_->Start();
    POWER_HILOGI(MODULE_SERVICE, "AutoSuspend enabled");
}

void SuspendController::ForceSuspend()
{
    std::lock_guard lock(suspendMutex_);
    bool success = suspend_->SuspendEnter();
    POWER_HILOGI(MODULE_SERVICE, "Forced suspend %{public}s", success ? "succeeded." : "failed!");
}

void SuspendController::IncSuspendBlockCounter()
{
    std::lock_guard lock(suspendMutex_);
    suspendBlockCounter_++;
    POWER_HILOGD(MODULE_SERVICE, "Running Lock Counter = %{public}d", suspendBlockCounter_);
}

void SuspendController::DecSuspendBlockCounter()
{
    std::lock_guard lock(suspendMutex_);
    suspendBlockCounter_--;
    POWER_HILOGD(MODULE_SERVICE, "Running Lock Counter = %{public}d", suspendBlockCounter_);
    if (SuspendConditionSatisfied()) {
        suspendCv_.notify_one();
    }
}

bool SuspendController::SuspendConditionSatisfied()
{
    return suspendBlockCounter_ == 0;
}

void SuspendController::WaitingSuspendCondition()
{
    auto suspendLock = std::unique_lock(suspendMutex_);
    suspendCv_.wait(suspendLock, [this] { return this->SuspendConditionSatisfied(); });
}
} // namespace Suspend
} // namespace PowerMgr
} // namespace OHOS
