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
#include "v1_3/ipower_interface.h"
#include "ffrt_utils.h"

namespace OHOS {
namespace PowerMgr {
class SystemSuspendController : public DelayedRefSingleton<SystemSuspendController> {
public:
    void Suspend(const std::function<void()>& onSuspend, const std::function<void()>& onWakeup, bool force);
    void Wakeup();
    bool Hibernate();
    int32_t AcquireRunningLock(const RunningLockParam& param);
    int32_t ReleaseRunningLock(const RunningLockParam& param);
    void Dump(std::string& info);
    void GetWakeupReason(std::string& reason);
    void RegisterHdiStatusListener();
    void RegisterPowerHdiCallback();
    void UnRegisterPowerHdiCallback();
    void AllowAutoSleep();
    void DisallowAutoSleep();
    void SetSuspendTag(const std::string& tag);
    int32_t SetPowerConfig(const std::string& sceneName, const std::string& value);
    int32_t GetPowerConfig(const std::string& sceneName, std::string& value);
    
private:
    DECLARE_DELAYED_REF_SINGLETON(SystemSuspendController);

    inline static constexpr const char* WAKEUP_HOLDER = "OHOSPowerMgr.WakeupHolder";
    class PowerHdfCallback : public OHOS::HDI::Power::V1_3::IPowerHdiCallback {
    public:
        PowerHdfCallback() = default;
        ~PowerHdfCallback() = default;
        int32_t OnSuspend() override;
        int32_t OnWakeup() override;
        int32_t OnWakeupWithTag(int32_t suspendTag) override;
        void SetListener(std::function<void()>& suspend, std::function<void()>& wakeup,
            std::function<void(int32_t)>& onWakeupWithTag);

    private:
        std::function<void()> onSuspend_ {nullptr};
        std::function<void()> onWakeup_ {nullptr};
        std::function<void(int32_t)> onWakeupWithTag_ {nullptr};
    };
    OHOS::HDI::Power::V1_3::RunningLockInfo FillRunningLockInfo(const RunningLockParam& param);
    using IPowerInterface = OHOS::HDI::Power::V1_3::IPowerInterface;
    sptr<IPowerInterface> GetPowerInterface();
    std::mutex mutex_;
    std::mutex interfaceMutex_;
    std::shared_ptr<Suspend::ISuspendController> sc_;
    sptr<IPowerInterface> powerInterface_ { nullptr };
    sptr<OHOS::HDI::ServiceManager::V1_0::IServiceManager> hdiServiceMgr_ { nullptr };
    sptr<HdiServiceStatusListener::IServStatListener> hdiServStatListener_ { nullptr };
    std::atomic<bool> allowSleepTask_ {false};
    FFRTQueue queue_ {"power_system_suspend_controller"};
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_SYSTEM_SUSPEND_CONTROLLER_H
