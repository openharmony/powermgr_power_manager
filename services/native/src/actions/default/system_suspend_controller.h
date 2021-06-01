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

#ifndef POWERMGR_SYSTEM_SUSPEND_CONTROLLER_H
#define POWERMGR_SYSTEM_SUSPEND_CONTROLLER_H

#include <memory>
#include <mutex>

#include <singleton.h>

#include "suspend/irunning_lock_hub.h"
#include "suspend/isuspend_controller.h"

namespace OHOS {
namespace PowerMgr {
class SystemSuspendController : public DelayedRefSingleton<SystemSuspendController> {
public:
    void EnableSuspend();
    void ForceSuspend();
    void DisableSuspend();
    void AcquireRunningLock(const std::string& name);
    void ReleaseRunningLock(const std::string& name);

private:
    DECLARE_DELAYED_REF_SINGLETON(SystemSuspendController);

    inline static const std::string WAKEUP_HOLDER = "OHOSPowerMgr.WakeupHolder";
    bool suspendEnabled_ {false};
    std::mutex mutex_;
    std::unique_ptr<Suspend::IRunningLockHub> rlh_;
    std::shared_ptr<Suspend::ISuspendController> sc_;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_SYSTEM_SUSPEND_CONTROLLER_H
