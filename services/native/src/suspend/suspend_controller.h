/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include <cinttypes>
#include <functional>
#include <memory>
#include <vector>

#include "event_handler.h"
#include "power_state_machine.h"
#ifdef HAS_SENSORS_SENSOR_PART
#include "sensor_agent.h"
#endif
#include "shutdown_controller.h"
#include "suspend_source_parser.h"
#include "suspend_sources.h"
#include "sleep_callback_holder.h"
#include "ffrt_utils.h"

namespace OHOS {
namespace PowerMgr {

using SuspendListener = std::function<void(SuspendDeviceType, uint32_t, uint32_t)>;

class SuspendMonitor;
class SuspendEventHandler;
class SuspendController : public std::enable_shared_from_this<SuspendController> {
public:
    SuspendController(
        std::shared_ptr<ShutdownController>& shutdownController, std::shared_ptr<PowerStateMachine>& stateMachine);
    ~SuspendController();
    void Init();
    void ExecSuspendMonitorByReason(SuspendDeviceType reason);
    void RegisterSettingsObserver();
    void Execute();
    void Cancel();
    void StopSleep();
    void HandleEvent(int64_t delayTime);
    void CancelEvent();
    void HandleAction(SuspendDeviceType reason, uint32_t action);
    void RecordPowerKeyDown();
    bool GetPowerkeyDownWhenScreenOff();

    void AddCallback(const sptr<ISyncSleepCallback>& callback, SleepPriority priority);
    void RemoveCallback(const sptr<ISyncSleepCallback>& callback);
    void TriggerSyncSleepCallback(bool isWakeup);

    std::shared_ptr<PowerStateMachine> GetStateMachine() const
    {
        return stateMachine_;
    }
    SuspendDeviceType GetLastReason() const
    {
        return sleepReason_;
    }
    uint32_t GetLastAction() const
    {
        return sleepAction_;
    }
    void StartSleepTimer(SuspendDeviceType reason, uint32_t action, uint32_t delay);
    void Reset();

private:
    void ControlListener(SuspendDeviceType reason, uint32_t action, uint32_t delay);
    void HandleAutoSleep(SuspendDeviceType reason);
    void HandleForceSleep(SuspendDeviceType reason);
    void HandleHibernate(SuspendDeviceType reason);
    void HandleShutdown(SuspendDeviceType reason);

    void TriggerSyncSleepCallbackInner(std::set<sptr<ISyncSleepCallback>>& callbacks, bool isWakeup);
    static constexpr int32_t FORCE_SLEEP_DELAY_MS = 8000;
    void SuspendWhenScreenOff(SuspendDeviceType reason, uint32_t action, uint32_t delay);
    std::vector<SuspendSource> sourceList_;
    std::map<SuspendDeviceType, std::shared_ptr<SuspendMonitor>> monitorMap_;
    std::shared_ptr<ShutdownController> shutdownController_;
    std::shared_ptr<PowerStateMachine> stateMachine_;
    uint32_t sleepDuration_ {0};
    int64_t sleepTime_ {-1};
    SuspendDeviceType sleepReason_ {0};
    uint32_t sleepAction_ {0};
    uint32_t sleepType_ {0};
    bool powerkeyDownWhenScreenOff_ = false;
    std::mutex mutex_;
    std::shared_ptr<FFRTTimer> ffrtTimer_;
    FFRTMutexMap ffrtMutexMap_;
};

class SuspendMonitor {
public:
    const static std::shared_ptr<SuspendMonitor> CreateMonitor(SuspendSource& source);

    virtual ~SuspendMonitor() = default;
    virtual bool Init() = 0;
    virtual void Cancel() = 0;
    virtual void HandleEvent()
    {
        // do nothing in base class
    }
    SuspendDeviceType GetReason() const
    {
        return reason_;
    }
    uint32_t GetAction() const
    {
        return action_;
    }
    uint32_t GetDelay() const
    {
        return delayMs_;
    }
    void RegisterListener(SuspendListener listener)
    {
        listener_ = listener;
    }

    void Notify()
    {
        if (listener_ == nullptr) {
            return;
        }
        listener_(reason_, action_, delayMs_);
    }
protected:
    explicit SuspendMonitor(const SuspendSource& source)
    {
        reason_ = source.GetReason();
        action_ = source.GetAction();
        delayMs_ = source.GetDelay();
    }

    SuspendDeviceType reason_;
    uint32_t action_;
    uint32_t delayMs_;
    SuspendListener listener_;
};

class PowerKeySuspendMonitor : public SuspendMonitor {
public:
    explicit PowerKeySuspendMonitor(SuspendSource& source) : SuspendMonitor(source) {}
    ~PowerKeySuspendMonitor() override = default;
    bool Init() override;
    void Cancel() override;

private:
    static constexpr int32_t LONG_PRESS_DELAY_MS = 3000;
    static constexpr int32_t POWER_KEY_PRESS_DELAY_MS = 10000;
    int32_t powerkeyReleaseId_ {-1};
};

class TimeoutSuspendMonitor : public SuspendMonitor {
public:
    explicit TimeoutSuspendMonitor(SuspendSource& source) : SuspendMonitor(source) {}
    ~TimeoutSuspendMonitor() override = default;
    bool Init() override;
    void Cancel() override;
    void HandleEvent() override;
};

class LidSuspendMonitor : public SuspendMonitor {
public:
    explicit LidSuspendMonitor(SuspendSource& source) : SuspendMonitor(source) {}
    ~LidSuspendMonitor() override = default;
    bool Init() override;
    void Cancel() override;
};

class SwitchSuspendMonitor : public SuspendMonitor {
public:
    explicit SwitchSuspendMonitor(SuspendSource& source) : SuspendMonitor(source) {}
    ~SwitchSuspendMonitor() override = default;
    bool Init() override;
    void Cancel() override;
};

} // namespace PowerMgr
} // namespace OHOS

#endif // POWERMGR_SUSPEND_CONTROLLER_H
