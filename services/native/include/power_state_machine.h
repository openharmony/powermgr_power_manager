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

#ifndef POWERMGR_POWER_STATE_MACHINE_H
#define POWERMGR_POWER_STATE_MACHINE_H

#include <set>
#include <map>
#include <singleton.h>

#include "actions/idevice_state_action.h"
#include "ffrt_utils.h"
#include "ipower_state_callback.h"
#include "power_common.h"
#include "power_state_machine_info.h"
#include "running_lock_info.h"
#include "hibernate_controller.h"

#define DEFAULT_DISPLAY_OFF_TIME 30000
#define DEFAULT_SLEEP_TIME       5000

namespace OHOS {
namespace PowerMgr {
class RunningLockMgr;
class PowerMgrService;

struct ScreenState {
    DisplayState state;
    int64_t lastOnTime;
    int64_t lastOffTime;
};

struct DevicePowerState {
    ScreenState screenState;
    // record the last time when get wakeup event from A side
    int64_t lastWakeupEventTime;
    // record the last time when calling func RefreshActivityInner
    int64_t lastRefreshActivityTime;
    // record the last time when calling func WakeupDeviceInner
    int64_t lastWakeupDeviceTime;
    // record the last time when calling func SuspendDeviceInner
    int64_t lastSuspendDeviceTime;
};

enum class TransitResult {
    SUCCESS = 0,
    ALREADY_IN_STATE = 1,
    LOCKING = 2,
    HDI_ERR = 3,
    DISPLAY_ON_ERR = 4,
    DISPLAY_OFF_ERR = 5,
    FORBID_TRANSIT = 6,
    OTHER_ERR = 99
};

class PowerStateMachine : public std::enable_shared_from_this<PowerStateMachine> {
public:
    explicit PowerStateMachine(const wptr<PowerMgrService>& pms);
    ~PowerStateMachine();

    enum {
        CHECK_USER_ACTIVITY_TIMEOUT_MSG = 0,
        CHECK_USER_ACTIVITY_OFF_TIMEOUT_MSG,
    };

    static void onSuspend();
    static void onWakeup();

    bool Init();
    void InitState();
    void SuspendDeviceInner(
        pid_t pid, int64_t callTimeMs, SuspendDeviceType type, bool suspendImmed, bool ignoreScreenState = false);
    void WakeupDeviceInner(
        pid_t pid, int64_t callTimeMs, WakeupDeviceType type, const std::string& details, const std::string& pkgName);
    void HandlePreBrightWakeUp(
        int64_t callTimeMs, WakeupDeviceType type, const std::string& details, const std::string& pkgName);
    void RefreshActivityInner(pid_t pid, int64_t callTimeMs, UserActivityType type, bool needChangeBacklight);
    bool CheckRefreshTime();
    bool OverrideScreenOffTimeInner(int64_t timeout);
    bool RestoreScreenOffTimeInner();
    void ReceiveScreenEvent(bool isScreenOn);
    bool IsScreenOn();
    void Reset();
    int64_t GetSleepTime();

    PowerState GetState()
    {
        return currentState_;
    };
    const std::shared_ptr<IDeviceStateAction>& GetStateAction()
    {
        return stateAction_;
    };
    bool ForceSuspendDeviceInner(pid_t pid, int64_t callTimeMs);
    bool HibernateInner(bool clearMemory);
    void RegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback);
    void UnRegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback);
    void SetDelayTimer(int64_t delayTime, int32_t event);
    void CancelDelayTimer(int32_t event);
    void ResetInactiveTimer();
    void ResetSleepTimer();
    void SetAutoSuspend(SuspendDeviceType type, uint32_t delay);
    bool SetState(PowerState state, StateChangeReason reason, bool force = false);
    void SetDisplaySuspend(bool enable);
    StateChangeReason GetReasonByUserActivity(UserActivityType type);
    StateChangeReason GetReasonByWakeType(WakeupDeviceType type);
    StateChangeReason GetReasionBySuspendType(SuspendDeviceType type);
    WakeupDeviceType ParseWakeupDeviceType(const std::string& details);

    // only use for test
    int64_t GetLastSuspendDeviceTime() const
    {
        return mDeviceState_.lastSuspendDeviceTime;
    }
    int64_t GetLastWakeupDeviceTime() const
    {
        return mDeviceState_.lastWakeupDeviceTime;
    }
    int64_t GetLastRefreshActivityTime() const
    {
        return mDeviceState_.lastRefreshActivityTime;
    }
    int64_t GetLastWakeupEventTime() const
    {
        return mDeviceState_.lastWakeupEventTime;
    }
    void SetSwitchState(bool switchOpen)
    {
        switchOpen_ = switchOpen;
    }
    bool IsSwitchOpen() const
    {
        return switchOpen_;
    }
    int64_t GetLastOnTime() const
    {
        return mDeviceState_.screenState.lastOnTime;
    }
    class PowerStateCallbackDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        PowerStateCallbackDeathRecipient() = default;
        virtual void OnRemoteDied(const wptr<IRemoteObject>& remote);
        virtual ~PowerStateCallbackDeathRecipient() = default;
    };
    void DumpInfo(std::string& result);
    void EnableMock(IDeviceStateAction* mockAction);
    int64_t GetDisplayOffTime();
    int64_t GetDimTime(int64_t displayOffTime);
    static constexpr int64_t OFF_TIMEOUT_FACTOR = 4;
    static constexpr int64_t MAX_DIM_TIME_MS = 7500;
    static constexpr int64_t COORDINATED_STATE_SCREEN_OFF_TIME_MS = 10000;
    static constexpr uint32_t SCREEN_CHANGE_TIMEOUT_MS = 10000;
    static constexpr uint32_t SCREEN_CHANGE_REPORT_INTERVAL_MS = 600000;
    void SetDisplayOffTime(int64_t time, bool needUpdateSetting = true);
    static void RegisterDisplayOffTimeObserver();
    static void UnregisterDisplayOffTimeObserver();
    void SetSleepTime(int64_t time);
    bool IsRunningLockEnabled(RunningLockType type);
    void SetForceTimingOut(bool enabled);
    void LockScreenAfterTimingOut(bool enabled, bool checkScreenOnLock);
    bool IsSettingState(PowerState state);

private:
    static std::string GetTransitResultString(TransitResult result);
    class SettingStateFlag {
    public:
        SettingStateFlag(PowerState state, std::shared_ptr<PowerStateMachine> owner) : owner_(owner)
        {
            std::shared_ptr<PowerStateMachine> stateMachine = owner_.lock();
            stateMachine->settingStateFlag_ = static_cast<int64_t>(state);
        }
        SettingStateFlag(const SettingStateFlag&) = delete;
        SettingStateFlag& operator=(const SettingStateFlag&) = delete;
        SettingStateFlag(SettingStateFlag&&) = delete;
        SettingStateFlag& operator=(SettingStateFlag&&) = delete;
        ~SettingStateFlag()
        {
            std::shared_ptr<PowerStateMachine> stateMachine = owner_.lock();
            stateMachine->settingStateFlag_ = -1;
        }

    protected:
        std::weak_ptr<PowerStateMachine> owner_;
    };
    class StateController {
    public:
        StateController(PowerState state, std::shared_ptr<PowerStateMachine> owner,
            std::function<TransitResult(StateChangeReason)> action)
            : state_(state),
            owner_(owner), action_(action)
        {
        }
        ~StateController() = default;
        PowerState GetState()
        {
            return state_;
        }
        TransitResult TransitTo(StateChangeReason reason, bool ignoreLock = false);
        void RecordFailure(PowerState from, StateChangeReason trigger, TransitResult failReason);
        static bool IsReallyFailed(StateChangeReason reason);
        StateChangeReason lastReason_;
        int64_t lastTime_ {0};
        PowerState failFrom_;
        StateChangeReason failTrigger_;
        std::string failReason_;
        int64_t failTime_ {0};

    protected:
        bool CheckState();
        void MatchState(PowerState& currentState, DisplayState state);
        void CorrectState(PowerState& currentState, PowerState correctState, DisplayState state);
        PowerState state_;
        std::weak_ptr<PowerStateMachine> owner_;
        std::function<TransitResult(StateChangeReason)> action_;
    };

    struct classcomp {
        bool operator()(const sptr<IPowerStateCallback>& l, const sptr<IPowerStateCallback>& r) const
        {
            return l->AsObject() < r->AsObject();
        }
    };

    std::shared_ptr<FFRTTimer> ffrtTimer_;
    class ScreenChangeCheck {
    public:
        ScreenChangeCheck(std::shared_ptr<FFRTTimer> ffrtTimer, PowerState state, StateChangeReason reason);
        void Finish(TransitResult result);
    private:
        void Report(const std::string &msg);
        std::shared_ptr<FFRTTimer> timer_;
        pid_t pid_;
        pid_t uid_;
        PowerState state_;
        StateChangeReason reason_;
    };

    void InitStateMap();
    void EmplaceAwake();
    void EmplaceFreeze();
    void EmplaceInactive();
    void EmplaceStandBy();
    void EmplaceDoze();
    void EmplaceSleep();
    void EmplaceHibernate();
    void EmplaceShutdown();
    void EmplaceDim();
    void InitTransitMap();
    bool CanTransitTo(PowerState to, StateChangeReason reason);
    void NotifyPowerStateChanged(PowerState state);
    void SendEventToPowerMgrNotify(PowerState state, int64_t callTime);
    bool CheckRunningLock(PowerState state);
    void HandleActivityTimeout();
    void HandleActivitySleepTimeout();
    void HandleSystemWakeup();
    void AppendDumpInfo(std::string& result, std::string& reason, std::string& time);
    std::shared_ptr<StateController> GetStateController(PowerState state);
    void ResetScreenOffPreTimeForSwing(int64_t displayOffTime);

    const wptr<PowerMgrService> pms_;
    PowerState currentState_;
    std::map<PowerState, std::shared_ptr<std::vector<RunningLockType>>> lockMap_;
    std::map<PowerState, std::shared_ptr<StateController>> controllerMap_;
    std::mutex mutex_;
    // all change to currentState_ should be inside stateMutex_
    std::mutex stateMutex_;
    DevicePowerState mDeviceState_;
    sptr<IRemoteObject::DeathRecipient> powerStateCBDeathRecipient_;
    std::set<const sptr<IPowerStateCallback>, classcomp> powerStateListeners_;
    std::shared_ptr<IDeviceStateAction> stateAction_;

private:
    std::atomic<int64_t> displayOffTime_ {DEFAULT_DISPLAY_OFF_TIME};
    int64_t sleepTime_ {DEFAULT_SLEEP_TIME};
    bool enableDisplaySuspend_ {false};
    bool isScreenOffTimeOverride_ {false};
    bool IsPreBrightWakeUp(WakeupDeviceType type);
    std::unordered_map<PowerState, std::set<PowerState>> forbidMap_;
    std::atomic<bool> switchOpen_ {true};
    std::unordered_map<StateChangeReason, std::unordered_map<PowerState, std::set<PowerState>>> allowMapByReason_;
    std::atomic<bool> forceTimingOut_ {false};
    std::atomic<bool> enabledTimingOutLockScreen_ {true};
    std::atomic<bool> enabledTimingOutLockScreenCheckLock_ {false};
    std::atomic<int64_t> settingStateFlag_ {-1};
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_POWER_STATE_MACHINE_H
