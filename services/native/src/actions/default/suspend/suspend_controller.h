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

#ifndef POWERMGR_SUSPEND_CONTROLLER_H
#define POWERMGR_SUSPEND_CONTROLLER_H

#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>

#include "suspend/isuspend_controller.h"

#include "unique_fd.h"

namespace OHOS {
namespace PowerMgr {
namespace Suspend {
using namespace std::chrono_literals;

class SuspendController : public ISuspendController {
public:
    SuspendController();
    ~SuspendController() override {};

    void Suspend(SuspendCallback onSuspend, SuspendCallback onWakeup, bool force) override;
    void Wakeup() override;
    void IncSuspendBlockCounter() override;
    void DecSuspendBlockCounter() override;

private:
    using WaitingSuspendConditionFunc = std::function<void()>;
    class AutoSuspend {
    public:
        explicit AutoSuspend(const WaitingSuspendConditionFunc& waitingFunc) : waitingFunc_(waitingFunc) {}
        ~AutoSuspend() = default;

        void AutoSuspendLoop();
        void Start(SuspendCallback onSuspend, SuspendCallback onWakeup);
        void Stop();
        bool SuspendEnter();

    private:
        static bool started_;
        std::string WaitWakeupCount();
        bool WriteWakeupCount(std::string wakeupCount);

        static constexpr const char * const SUSPEND_STATE = "mem";
        static constexpr const char * const SUSPEND_STATE_PATH = "/sys/power/state";
        static constexpr const char * const WAKEUP_COUNT_PATH = "/sys/power/wakeup_count";
        UniqueFd wakeupCountFd {-1};

        std::chrono::milliseconds waitTime_ {100ms};
        std::unique_ptr<std::thread> daemon_;
        WaitingSuspendConditionFunc waitingFunc_;
        SuspendCallback onSuspend_;
        SuspendCallback onWakeup_;
    };

    bool SuspendConditionSatisfied();
    void WaitingSuspendCondition();

    uint32_t suspendBlockCounter_ {0};
    std::condition_variable suspendCv_;
    std::unique_ptr<AutoSuspend> suspend_;
    std::mutex suspendMutex_;
};
} // namespace Suspend
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_SUSPEND_CONTROLLER_H
