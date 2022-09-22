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

#ifndef POWERMGR_RUNNING_LOCK_HUB_H
#define POWERMGR_RUNNING_LOCK_HUB_H

#include <map>
#include <memory>
#include <mutex>
#include <string>

#include <unique_fd.h>

#include "suspend/irunning_lock_hub.h"
#include "suspend/isuspend_controller.h"

namespace OHOS {
namespace PowerMgr {
namespace Suspend {
class RunningLockHub : public IRunningLockHub {
public:
    explicit RunningLockHub(const std::shared_ptr<ISuspendController>& sc) : sc_(sc) {};
    ~RunningLockHub() override {};

    void Acquire(const std::string& name) override;
    void Release(const std::string& name) override;

private:
    bool InitFd();
    bool SaveLockFile(const std::string& name, bool isAcquire);
    void NotifySuspendCounter(bool isAcquire);

    static constexpr const char * const LOCK_PATH = "/sys/power/wake_lock";
    static constexpr const char * const UNLOCK_PATH = "/sys/power/wake_unlock";

    UniqueFd lockFd_{-1};
    UniqueFd unlockFd_{-1};
    std::shared_ptr<ISuspendController> sc_;
    std::mutex mutex_;
    std::map<std::string, uint32_t> runningLockMap_;
};
} // namespace Suspend
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_RUNNING_LOCK_HUB_H
