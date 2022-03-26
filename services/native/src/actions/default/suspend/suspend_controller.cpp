/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "power_log.h"

namespace OHOS {
namespace PowerMgr {
namespace Suspend {
bool SuspendController::AutoSuspend::started_ = false;

SuspendController::SuspendController()
{
    auto f = std::bind(&SuspendController::WaitingSuspendCondition, this);
    suspend_ = std::make_unique<AutoSuspend>(f);
}

void SuspendController::AutoSuspend::AutoSuspendLoop()
{
    POWER_HILOGD(FEATURE_SUSPEND, "AutoSuspendLoop start");
    while (true) {
        std::this_thread::sleep_for(waitTime_);
        if (!started_) {
            POWER_HILOGW(FEATURE_SUSPEND, "AutoSuspend is stopped");
            break;
        }
        const std::string wakeupCount = WaitWakeupCount();
        if (wakeupCount.empty()) {
            POWER_HILOGD(FEATURE_SUSPEND, "Can't read wake count, continue");
            continue;
        }
        waitingFunc_();
        if (!WriteWakeupCount(wakeupCount)) {
            POWER_HILOGD(FEATURE_SUSPEND, "Can't write wake count, continue");
            continue;
        }
        if (onSuspend_ != nullptr) {
            onSuspend_();
        }
        bool success = SuspendEnter();
        if (!success) {
            POWER_HILOGE(FEATURE_SUSPEND, "Start suspend failed");
        }
        if (onWakeup_ != nullptr) {
            onWakeup_();
        }
        break;
    }
    started_ = false;

    POWER_HILOGD(FEATURE_SUSPEND, "AutoSuspendLoop end");
}

void SuspendController::AutoSuspend::Start(SuspendCallback onSuspend, SuspendCallback onWakeup)
{
    POWER_HILOGD(FEATURE_SUSPEND, "AutoSuspend Start");
    onSuspend_ = onSuspend;
    onWakeup_ = onWakeup;
    if (started_) {
        POWER_HILOGW(FEATURE_SUSPEND, "AutoSuspend is already started");
        return;
    }
    daemon_ = std::make_unique<std::thread>(&AutoSuspend::AutoSuspendLoop, this);
    daemon_->detach();
    POWER_HILOGD(FEATURE_SUSPEND, "AutoSuspend Start detach");
    started_ = true;
}

void SuspendController::AutoSuspend::Stop()
{
    POWER_HILOGD(FEATURE_SUSPEND, "AutoSuspend Stop");
    if (started_ && daemon_.get() != nullptr) {
        POWER_HILOGD(FEATURE_SUSPEND, "daemon join start");
        started_ = false;
        daemon_->join();
        POWER_HILOGD(FEATURE_SUSPEND, "daemon join end");
    }
}

bool SuspendController::AutoSuspend::SuspendEnter()
{
    static bool inited = false;
    static UniqueFd suspendStateFd(TEMP_FAILURE_RETRY(open(SUSPEND_STATE_PATH, O_RDWR | O_CLOEXEC)));
    if (!inited) {
        if (suspendStateFd < 0) {
            POWER_HILOGE(FEATURE_SUSPEND, "Failed to open the suspending state fd");
            return false;
        }
        inited = true;
    }
    POWER_HILOGD(FEATURE_SUSPEND, "Before suspend");
    bool ret = SaveStringToFd(suspendStateFd, SUSPEND_STATE);
    POWER_HILOGD(FEATURE_SUSPEND, "After suspend");
    if (!ret) {
        POWER_HILOGE(FEATURE_SUSPEND, "Failed to write the suspending state");
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
        POWER_HILOGW(FEATURE_SUSPEND, "Read wakeup count failed");
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
        POWER_HILOGE(FEATURE_SUSPEND, "Failed to write the wakeup count");
    }
    return ret;
}

void SuspendController::Suspend(SuspendCallback onSuspend, SuspendCallback onWakeup, bool force)
{
    if (force) {
        POWER_HILOGI(FEATURE_SUSPEND, "SuspendController Suspend: force");
        if (onSuspend != nullptr) {
            onSuspend();
        }
        suspend_->SuspendEnter();
        if (onWakeup != nullptr) {
            onWakeup();
        }
    } else {
        POWER_HILOGI(FEATURE_SUSPEND, "SuspendController Suspend: not force");
        suspend_->Start(onSuspend, onWakeup);
    }
}

void SuspendController::Wakeup()
{
    suspend_->Stop();
}

void SuspendController::IncSuspendBlockCounter()
{
    std::lock_guard lock(suspendMutex_);
    suspendBlockCounter_++;
    POWER_HILOGD(FEATURE_SUSPEND, "Running Lock Counter = %{public}d", suspendBlockCounter_);
}

void SuspendController::DecSuspendBlockCounter()
{
    std::lock_guard lock(suspendMutex_);
    suspendBlockCounter_--;
    POWER_HILOGD(FEATURE_SUSPEND, "Running Lock Counter = %{public}d", suspendBlockCounter_);
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
