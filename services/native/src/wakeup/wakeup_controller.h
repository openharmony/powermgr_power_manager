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

#ifndef POWERMGR_WAKEUP_CONTROLLER_H
#define POWERMGR_WAKEUP_CONTROLLER_H

#include <functional>
#include <memory>
#include <mutex>
#include <vector>

#include "event_handler.h"
#include "i_input_event_consumer.h"
#include "power_state_machine.h"
#include "system_ability.h"
#include "sensor_agent.h"
#include "wakeup_sources.h"
#include "wakeup_source_parser.h"

namespace OHOS {
namespace PowerMgr {

using WakeupListener = std::function<void(uint32_t)>;
using namespace OHOS::MMI;
class WakeupMonitor;
class WakeupEventHandler;
class WakeupController : public std::enable_shared_from_this<WakeupController> {
public:
    WakeupController(
        std::shared_ptr<PowerStateMachine>& stateMachine, std::shared_ptr<AppExecFwk::EventRunner>& runner);
    ~WakeupController();
    void Init();
    void Cancel();
    void RegisterSettingsObserver();
    void ExecWakeupMonitorByReason(uint32_t reason);
    void Wakeup();
    std::shared_ptr<PowerStateMachine> GetStateMachine()
    {
        return stateMachine_;
    }
    uint32_t GetLastReason()
    {
        return wakeupReason_;
    }

private:
    void ControlListener(uint32_t reason);
    void StartWakeupTimer();
    std::vector<WakeupSource> sourceList_;
    std::map<uint32_t, std::shared_ptr<WakeupMonitor>> monitorMap_;
    std::shared_ptr<PowerStateMachine> stateMachine_;
    std::shared_ptr<AppExecFwk::EventRunner> runner_;
    std::shared_ptr<WakeupEventHandler> handler_;
    uint32_t wakeupReason_ {0};
    std::mutex mutex_;
    int32_t monitorId_ {-1};
};

class InputCallback : public IInputEventConsumer {
    virtual void OnInputEvent(std::shared_ptr<KeyEvent> keyEvent) const;
    virtual void OnInputEvent(std::shared_ptr<PointerEvent> pointerEvent) const;
    virtual void OnInputEvent(std::shared_ptr<AxisEvent> axisEvent) const;
};

class WakeupEventHandler : public AppExecFwk::EventHandler {
public:
    WakeupEventHandler(
        const std::shared_ptr<AppExecFwk::EventRunner>& runner, std::shared_ptr<WakeupController> controller)
        : AppExecFwk::EventHandler(runner),
        controller_(controller)
    {
    }
    ~WakeupEventHandler() = default;
    void ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event) override;

private:
    std::weak_ptr<WakeupController> controller_;
};

class WakeupMonitor {
public:
    static std::shared_ptr<WakeupMonitor> CreateMonitor(WakeupSource& source);
    static constexpr int32_t POWER_KEY_PRESS_DELAY_MS = 10000;

    virtual ~WakeupMonitor() = default;
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

    void RegisterListener(WakeupListener listener)
    {
        listener_ = listener;
    }

    void Notify()
    {
        listener_(reason_);
    }

protected:
    explicit WakeupMonitor(WakeupSource& source)
    {
        reason_ = static_cast<uint32_t>(source.GetReason());
    }

    uint32_t reason_;
    WakeupListener listener_;
};

class PowerkeyWakeupMonitor : public WakeupMonitor {
public:
    explicit PowerkeyWakeupMonitor(WakeupSource& source) : WakeupMonitor(source) {}
    ~PowerkeyWakeupMonitor() override = default;
    bool Init() override;
    void Cancel() override;

private:
    int32_t powerkeyShortPressId_ {-1};
};

class KeyboardWakeupMonitor : public WakeupMonitor {
public:
    explicit KeyboardWakeupMonitor(WakeupSource& source) : WakeupMonitor(source) {}
    ~KeyboardWakeupMonitor() override = default;
    bool Init() override;
    void Cancel() override;
};

class MousekeyWakeupMonitor : public WakeupMonitor {
public:
    explicit MousekeyWakeupMonitor(WakeupSource& source) : WakeupMonitor(source) {}
    ~MousekeyWakeupMonitor() override = default;
    bool Init() override;
    void Cancel() override;
};

class TouchpadWakeupMonitor : public WakeupMonitor {
public:
    explicit TouchpadWakeupMonitor(WakeupSource& source) : WakeupMonitor(source) {}
    ~TouchpadWakeupMonitor() override = default;
    bool Init() override;
    void Cancel() override;
};

class PenWakeupMonitor : public WakeupMonitor {
public:
    explicit PenWakeupMonitor(WakeupSource& source) : WakeupMonitor(source) {}
    ~PenWakeupMonitor() override = default;
    bool Init() override;
    void Cancel() override;
};

class SingleClickWakeupMonitor : public WakeupMonitor {
public:
    explicit SingleClickWakeupMonitor(WakeupSource& source) : WakeupMonitor(source) {}
    ~SingleClickWakeupMonitor() override = default;
    bool Init() override;
    void Cancel() override;
};

class DoubleClickWakeupMonitor : public WakeupMonitor {
public:
    explicit DoubleClickWakeupMonitor(WakeupSource& source) : WakeupMonitor(source) {}
    ~DoubleClickWakeupMonitor() override = default;
    bool Init() override;
    void Cancel() override;
};

class LidWakeupMonitor : public WakeupMonitor {
public:
    explicit LidWakeupMonitor(WakeupSource& source) : WakeupMonitor(source) {}
    ~LidWakeupMonitor() override = default;
    bool Init() override;
    void Cancel() override;
};

class SwitchWakeupMonitor : public WakeupMonitor {
public:
    explicit SwitchWakeupMonitor(WakeupSource& source) : WakeupMonitor(source) {}
    ~SwitchWakeupMonitor() override = default;
    bool Init() override;
    void Cancel() override;
};

} // namespace PowerMgr
} // namespace OHOS

#endif