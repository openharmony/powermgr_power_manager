/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "actions/running_lock_action_info.h"
#include "hdi_service_status_listener.h"
#include "suspend/irunning_lock_hub.h"
#include "suspend/isuspend_controller.h"
#include "power_hdi_callback.h"
#include "v1_2/ipower_interface.h"
#include "ffrt_utils.h"

namespace OHOS {
namespace PowerMgr {
class SystemSuspendController : public DelayedRefSingleton<SystemSuspendController> {
public:
    void Suspend(const std::function<void()>& onSuspend, const std::function<void()>& onWakeup, bool force);
    void Wakeup();
    void Hibernate();
    int32_t AcquireRunningLock(const RunningLockParam& param);
    int32_t ReleaseRunningLock(const RunningLockParam& param);
    void Dump(std::string& info);
    void GetWakeupReason(std::string& reason);
    void RegisterHdiStatusListener();
    void RegisterPowerHdiCallback();
    void UnRegisterPowerHdiCallback();
    
private:
    DECLARE_DELAYED_REF_SINGLETON(SystemSuspendController);

    inline static constexpr const char* WAKEUP_HOLDER = "OHOSPowerMgr.WakeupHolder";
    class PowerHdfCallback : public OHOS::HDI::Power::V1_2::IPowerHdiCallback {
    public:
        PowerHdfCallback() = default;
        ~PowerHdfCallback() = default;
        int32_t OnSuspend() override;
        int32_t OnWakeup() override;
        void SetListener(std::function<void()>& suspend, std::function<void()>& wakeup);
    private:
        std::function<void()> onSuspend_;
        std::function<void()> onWakeup_;
    };
    OHOS::HDI::Power::V1_2::RunningLockInfo FillRunningLockInfo(const RunningLockParam& param);
    std::mutex mutex_;
    std::shared_ptr<Suspend::ISuspendController> sc_;
    sptr<OHOS::HDI::Power::V1_2::IPowerInterface> powerInterface_ { nullptr };
    sptr<OHOS::HDI::ServiceManager::V1_0::IServiceManager> hdiServiceMgr_ { nullptr };
    sptr<HdiServiceStatusListener::IServStatListener> hdiServStatListener_ { nullptr };
    FFRTQueue queue_ {"power_system_suspend_controller"};
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_SYSTEM_SUSPEND_CONTROLLER_H
