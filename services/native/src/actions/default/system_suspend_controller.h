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

#include "power_hdf_client.h"
#include "suspend/irunning_lock_hub.h"
#include "suspend/isuspend_controller.h"

namespace OHOS {
namespace PowerMgr {
class SystemSuspendController : public DelayedRefSingleton<SystemSuspendController> {
public:
    void Suspend(std::function<void()> onSuspend, std::function<void()> onWakeup, bool force);
    void Wakeup();
    void AcquireRunningLock(const std::string& name);
    void ReleaseRunningLock(const std::string& name);
    void Dump(std::string& info);

private:
    DECLARE_DELAYED_REF_SINGLETON(SystemSuspendController);

    inline static const std::string WAKEUP_HOLDER = "OHOSPowerMgr.WakeupHolder";
    class PowerHdfCallback : public IPowerHdfCallback {
    public:
        PowerHdfCallback() = default;
        ~PowerHdfCallback() = default;
        void OnSuspend() override;
        void OnWakeup() override;
        void SetListener(std::function<void()> suspend, std::function<void()> wakeup);
    private:
        std::function<void()> onSuspend_;
        std::function<void()> onWakeup_;
    };
    std::mutex mutex_;
    std::shared_ptr<Suspend::ISuspendController> sc_;
    std::unique_ptr<PowerHdfClient> client_;
    sptr<PowerHdfCallback> callback_;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_SYSTEM_SUSPEND_CONTROLLER_H
