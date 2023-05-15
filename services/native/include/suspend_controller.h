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

#ifndef POWERMGR_SUSPEND_CONTROLLER_H
#define POWERMGR_SUSPEND_CONTROLLER_H

#include "event_handler.h"
#include "power_state_machine.h"
#include "sensor_agent.h"
#include "shutdown_service.h"
#include "suspend_source_parser.h"
#include "suspend_sources.h"
#include <functional>
#include <memory>
#include <vector>

namespace OHOS {
namespace PowerMgr {

using SuspendListener = std::function<void(uint32_t, uint32_t, int64_t)>;

class SuspendMonitor;
class SuspendEventHandler;
class SuspendController : public std::enable_shared_from_this<SuspendController> {
public:
    SuspendController(ShutdownService* shutdownService, std::shared_ptr<PowerStateMachine>& stateMachine,
        std::shared_ptr<AppExecFwk::EventRunner>& runner);
    ~SuspendController();
    void Init();
    void RegisterSettingsObserver();
    void Execute();
    void Cancel();
    void StopSleep();
    void HandleEvent(uint32_t eventId);
    void HandleAction(uint32_t reason, uint32_t action);
    void RecordPowerKeyDown();
    bool GetPowerkeyDownWhenScreenOff();
    std::shared_ptr<PowerStateMachine> GetStateMachine()
    {
        return stateMachine_;
    }
    uint32_t GetLastReason()
    {
        return sleepReason_;
    }
    uint32_t GetLastAction()
    {
        return sleepAction_;
    }

private:
    void ControlListener(uint32_t reason, uint32_t action, int64_t delay);
    void StartSleepTimer(uint32_t reason, uint32_t action, int64_t time);
    void HandleAutoSleep(uint32_t reason);
    void HandleForceSleep(uint32_t reason);
    void HandleHibernate(uint32_t reason);
    void HandleShutdown(uint32_t reason);
    std::vector<SuspendSource> sourceList_;
    std::map<uint32_t, std::shared_ptr<SuspendMonitor>> monitorMap_;
    ShutdownService* shutdownService_;
    std::shared_ptr<PowerStateMachine> stateMachine_;
    std::shared_ptr<AppExecFwk::EventRunner> runner_;
    std::shared_ptr<SuspendEventHandler> handler_ = nullptr;
    int64_t sleepDuration_ {0};
    int64_t sleepTime_ {-1};
    uint32_t sleepReason_ {0};
    uint32_t sleepAction_ {0};
    bool powerkeyDownWhenScreenOff_ = false;
};

class SuspendEventHandler : public AppExecFwk::EventHandler {
public:
    enum {
        SLEEP_TIMEOUT_MSG = 100,
        SCREEN_OFF_TIMEOUT_MSG,
        SUSPEND_MAX_MSG,
    };
    SuspendEventHandler(
        const std::shared_ptr<AppExecFwk::EventRunner>& runner, std::shared_ptr<SuspendController> controller)
        : AppExecFwk::EventHandler(runner),
        controller_(controller)
    {
    }
    ~SuspendEventHandler() = default;
    void ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event) override;

private:
    std::weak_ptr<SuspendController> controller_;
};

class SuspendMonitor {
public:
    static std::shared_ptr<SuspendMonitor> CreateMonitor(SuspendSource& source);

    virtual ~SuspendMonitor() = default;
    virtual bool Init() = 0;
    virtual void Cancel() = 0;
    virtual void HandleEvent(uint32_t eventId)
    {
        // do nothing in base class
    }
    uint32_t GetReason()
    {
        return reason_;
    }
    uint32_t GetAction()
    {
        return action_;
    }
    int64_t GetDelay()
    {
        return delayMs_;
    }
    void RegisterListener(SuspendListener listener)
    {
        listener_ = listener;
    }
    bool IsSupportSensor(SensorTypeId typeId);

protected:
    SuspendMonitor(SuspendSource& source)
    {
        reason_ = static_cast<uint32_t>(source.GetReason());
        action_ = source.GetAction();
        delayMs_ = source.GetDelay();
    }
    void Notify()
    {
        listener_(reason_, action_, delayMs_);
    }
    uint32_t reason_;
    uint32_t action_;
    int64_t delayMs_;
    SuspendListener listener_;
};

class PowerKeySuspendMonitor : public SuspendMonitor {
public:
    PowerKeySuspendMonitor(SuspendSource& source) : SuspendMonitor(source) {}
    ~PowerKeySuspendMonitor() = default;
    bool Init() override;
    void Cancel() override;

private:
    static constexpr int32_t LONG_PRESS_DELAY_MS = 3000;
    static constexpr int32_t POWER_KEY_PRESS_DELAY_MS = 10000;
    int32_t powerkeyReleaseId_ {-1};
};

class TimeoutSuspendMonitor : public SuspendMonitor {
public:
    TimeoutSuspendMonitor(SuspendSource& source) : SuspendMonitor(source) {}
    ~TimeoutSuspendMonitor() = default;
    bool Init() override;
    void Cancel() override;
    void HandleEvent(uint32_t eventId) override;
};

class LidSuspendMonitor : public SuspendMonitor, std::enable_shared_from_this<LidSuspendMonitor> {
public:
    LidSuspendMonitor(SuspendSource& source) : SuspendMonitor(source) {}
    ~LidSuspendMonitor() = default;
    bool Init() override;
    void Cancel() override;
    static void HallSensorCallback(SensorEvent* event);

private:
    static constexpr int32_t HALL_REPORT_INTERVAL = 0;
    static constexpr uint32_t HALL_SAMPLING_RATE = 100000000;

    static std::weak_ptr<LidSuspendMonitor> self_;
    SensorUser sensorUser_;
};

} // namespace PowerMgr
} // namespace OHOS

#endif // POWERMGR_SUSPEND_CONTROLLER_H