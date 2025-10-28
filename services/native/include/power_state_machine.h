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
#ifdef POWER_MANAGER_POWER_ENABLE_S4
#include "hibernate_controller.h"
#endif
#include "ipower_state_callback.h"
#include "power_common.h"
#include "power_state_machine_info.h"
#include "running_lock_info.h"
#include "power_mgr_notify.h"
#include "window_manager_lite.h"
#include "suspend/itake_over_suspend_callback.h"
#include "parameters.h"

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
    PRE_BRIGHT_ERR = 7,
    TAKEN_OVER = 8,
    OTHER_ERR = 99
};

class PowerStateMachine : public std::enable_shared_from_this<PowerStateMachine> {
public:
    PowerStateMachine(const wptr<PowerMgrService>& pms, const std::shared_ptr<FFRTTimer>& ffrtTimer = nullptr);
    ~PowerStateMachine();

    enum {
        CHECK_USER_ACTIVITY_TIMEOUT_MSG = 0,
        CHECK_USER_ACTIVITY_OFF_TIMEOUT_MSG,
        CHECK_PRE_BRIGHT_AUTH_TIMEOUT_MSG,
        CHECK_PROXIMITY_SCREEN_OFF_MSG,
        SET_INTERNAL_SCREEN_STATE_MSG,
        CHECK_PROXIMITY_SCREEN_SWITCH_TO_SUB_MSG,
    };

    class PowerStateCallbackDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        PowerStateCallbackDeathRecipient() = default;
        virtual void OnRemoteDied(const wptr<IRemoteObject>& remote);
        virtual ~PowerStateCallbackDeathRecipient() = default;
    };

#ifdef POWER_MANAGER_ENABLE_CHARGING_TYPE_SETTING
    static constexpr int32_t DEFAULT_AC_DISPLAY_OFF_TIME_MS = 600000;
    static constexpr int32_t DEFAULT_DC_DISPLAY_OFF_TIME_MS = 300000
    static constexpr int32_t DEFAULT_DISPLAY_OFF_TIME_MS = DEFAULT_DC_DISPLAY_OFF_TIME_MS;
#else
    static constexpr int32_t DEFAULT_DISPLAY_OFF_TIME_MS = 30000;
#endif
    static constexpr int32_t DEFAULT_SLEEP_TIME_MS = 5000;
    static constexpr int64_t OFF_TIMEOUT_FACTOR = 5;
    static constexpr int64_t MAX_DIM_TIME_MS = 7500;
    static constexpr int64_t COORDINATED_STATE_SCREEN_OFF_TIME_MS = 10000;
    static constexpr uint32_t SCREEN_CHANGE_TIMEOUT_MS = 10000;
    static constexpr uint32_t SCREEN_CHANGE_REPORT_INTERVAL_MS = 600000;

    static void onSuspend();
    static void onWakeup();
    static void DisplayOffTimeUpdateFunc();
    static void RegisterDisplayOffTimeObserver();
    static void UnregisterDisplayOffTimeObserver();

    bool Init();
    void InitState();
    void SuspendDeviceInner(
        pid_t pid, int64_t callTimeMs, SuspendDeviceType type, bool suspendImmed, bool ignoreScreenState = false);
    void WakeupDeviceInner(
        pid_t pid, int64_t callTimeMs, WakeupDeviceType type, const std::string& details, const std::string& pkgName);
    void HandlePreBrightWakeUp(int64_t callTimeMs, WakeupDeviceType type, const std::string& details,
        const std::string& pkgName, bool timeoutTriggered = false);
    void RefreshActivityInner(pid_t pid, int64_t callTimeMs, UserActivityType type, bool needChangeBacklight);
    bool CheckRefreshTime();
    bool OverrideScreenOffTimeInner(int64_t timeout);
    bool RestoreScreenOffTimeInner();
    void ReceiveScreenEvent(bool isScreenOn);
    bool IsScreenOn(bool needPrintLog = true);
    bool IsScreenOnAcqLock();
    bool IsFoldScreenOn();
    bool IsCollaborationScreenOn();
    bool CheckFFRTTaskAvailability(PowerState state, StateChangeReason reason) const;
    bool IsTimeoutReason(StateChangeReason reason) const;
    void Reset();
    int64_t GetSleepTime();
#ifdef MSDP_MOVEMENT_ENABLE
    bool IsMovementStateOn();
#endif
    bool IsWakeupDeviceSkip();

    PowerState GetState()
    {
        return currentState_;
    };
    const std::shared_ptr<IDeviceStateAction>& GetStateAction()
    {
        return stateAction_;
    };
    bool ForceSuspendDeviceInner(pid_t pid, int64_t callTimeMs);
#ifdef POWER_MANAGER_POWER_ENABLE_S4
    bool HibernateInner(bool clearMemory, const std::string& reason);
#endif
    void RegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback, bool isSync = true);
    void UnRegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback);
    void SetDelayTimer(int64_t delayTime, int32_t event);
    void SetDelayTimer(int64_t delayTime, int32_t event, FFRTTask& task);
    void CancelDelayTimer(int32_t event);
    void ResetInactiveTimer(bool needPrintLog = true);
    void ResetSleepTimer();
    void SetAutoSuspend(SuspendDeviceType type, uint32_t delay);
    bool SetState(PowerState state, StateChangeReason reason, bool force = false);
    bool TryToCancelScreenOff();
    void BeginPowerkeyScreenOff();
    void EndPowerkeyScreenOff();
    void SetDisplaySuspend(bool enable);
    void WriteHiSysEvent(TransitResult ret, StateChangeReason reason, int32_t beginTimeMs, PowerState state);
    bool IsTransitFailed(TransitResult ret);
    SuspendDeviceType GetSuspendTypeByReason(StateChangeReason reason);
    StateChangeReason GetReasonByWakeType(WakeupDeviceType type);
    StateChangeReason GetReasonBySuspendType(SuspendDeviceType type);
#ifdef POWER_MANAGER_ENABLE_EXTERNAL_SCREEN_MANAGEMENT
    int32_t GetExternalScreenNumber() const
    {
        return externalScreenNumber_;
    }
    void SetExternalScreenNumber(int32_t exScreenNumber)
    {
        externalScreenNumber_ = exScreenNumber;
    }
    bool IsOnlySecondDisplayModeSupported() const
    {
        int32_t displayMode = system::GetIntParameter("const.product.support_display_mode", 0);
        return (displayMode & DISPLAY_MODE_ONLY_SECOND_SCREEN) > 0;
    }
#endif

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
    void SetSwitchAction(uint32_t value)
    {
        switchActionValue_ = value;
    }
    uint32_t GetSwitchAction()
    {
        return switchActionValue_;
    }
    void SetSwitchState(bool switchOpen)
    {
        switchOpen_ = switchOpen;
    }
    bool IsSwitchOpen() const
    {
        return switchOpen_;
    }
    bool IsSwitchOpenByPath();
#ifdef POWER_MANAGER_POWER_ENABLE_S4
    bool IsHibernating() const
    {
        return hibernating_;
    }
#endif
    int64_t GetLastOnTime() const
    {
        return mDeviceState_.screenState.lastOnTime;
    }
    void SetDuringCallState(bool state)
    {
        isDuringCall_ = state;
    }
    bool IsDuringCall()
    {
        return isDuringCall_;
    }

    void DumpInfo(std::string& result);
    void EnableMock(IDeviceStateAction* mockAction);
    int64_t GetDisplayOffTime();
    int64_t GetDimTime(int64_t displayOffTime);
    void SetDisplayOffTime(int64_t time, bool needUpdateSetting = true);
    void SetSleepTime(int64_t time);
    bool IsRunningLockEnabled(RunningLockType type);
    void SetForceTimingOut(bool enabled);
    void LockScreenAfterTimingOut(bool enabled, bool checkScreenOnLock, bool sendScreenOffEvent);
    bool IsSettingState(PowerState state);
    void SetEnableDoze(bool enable);
    bool SetDozeMode(DisplayState state);
#ifdef POWER_MANAGER_ENABLE_EXTERNAL_SCREEN_MANAGEMENT
    void SetInternalScreenDisplayState(DisplayState state, StateChangeReason reason);
#endif
    bool HandleDuringCall(bool isProximityClose);

private:
    enum PreBrightState : uint32_t {
        PRE_BRIGHT_UNSTART = 0,
        PRE_BRIGHT_STARTED,
        PRE_BRIGHT_FINISHED,
    };

    class SettingStateFlag {
    public:
        SettingStateFlag(PowerState state, std::shared_ptr<PowerStateMachine> owner, StateChangeReason reason)
            : owner_(owner)
        {
            std::shared_ptr<PowerStateMachine> stateMachine = owner_.lock();
            int64_t flag;
            if (state == PowerState::DIM && reason == StateChangeReason::STATE_CHANGE_REASON_COORDINATION) {
                flag = static_cast<int64_t>(StateFlag::FORCE_SETTING_DIM);
            } else {
                flag = static_cast<int64_t>(state);
            }
            stateMachine->settingStateFlag_ = flag;
        }
        SettingStateFlag(const SettingStateFlag&) = delete;
        SettingStateFlag& operator=(const SettingStateFlag&) = delete;
        SettingStateFlag(SettingStateFlag&&) = delete;
        SettingStateFlag& operator=(SettingStateFlag&&) = delete;
        ~SettingStateFlag()
        {
            std::shared_ptr<PowerStateMachine> stateMachine = owner_.lock();
            stateMachine->settingStateFlag_ = static_cast<int64_t>(StateFlag::NONE);
        }
        enum class StateFlag : int64_t {
            FORCE_SETTING_DIM = -3,
            SETTING_DIM_INTERRUPTED = -2,
            NONE = -1
        };
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
        bool NeedNotify(PowerState currentState);
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

    class ScreenChangeCheck {
    public:
        ScreenChangeCheck(std::shared_ptr<FFRTTimer> ffrtTimer, PowerState state, StateChangeReason reason);
        ~ScreenChangeCheck() noexcept;
        void SetReportTimerStartFlag(bool flag) const;
        void ReportSysEvent(const std::string& msg) const;

    private:
        pid_t pid_ {-1};
        pid_t uid_ {-1};
        mutable bool isReportTimerStarted_ {false};
        std::shared_ptr<FFRTTimer> ffrtTimer_ {nullptr};
        PowerState state_;
        StateChangeReason reason_;
    };

    static std::string GetTransitResultString(TransitResult result);

    void UpdateSettingStateFlag(PowerState state, StateChangeReason reason);
    void RestoreSettingStateFlag();
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
    bool CanTransitTo(PowerState from, PowerState to, StateChangeReason reason);
    void NotifyPowerStateChanged(PowerState state,
        StateChangeReason reason = StateChangeReason::STATE_CHANGE_REASON_APPLICATION);
    void SendEventToPowerMgrNotify(PowerState state, int64_t callTime, const std::string& reason);
    bool CheckRunningLock(PowerState state);
    void HandleActivityTimeout();
    void HandleActivitySleepTimeout();
    void HandleSystemWakeup();
    void AppendDumpInfo(std::string& result, std::string& reason, std::string& time);
    std::shared_ptr<StateController> GetStateController(PowerState state);
    void ResetScreenOffPreTimeForSwing(int64_t displayOffTime);
    void ShowCurrentScreenLocks();
    void HandleProximityScreenOffTimer(PowerState state, StateChangeReason reason);
    bool HandlePreBrightState(PowerState targetState, StateChangeReason reason);
    bool IsPreBrightAuthReason(StateChangeReason reason);
    bool IsPreBrightWakeUp(WakeupDeviceType type);
    bool NeedShowScreenLocks(PowerState state);
#ifdef POWER_MANAGER_POWER_ENABLE_S4
    void DelayForHibernateInactive(bool clearMemory);
    bool PrepareHibernate(bool clearMemory);
    bool PrepareHibernateWithTimeout(bool clearMemory);
    void RestoreHibernate(bool clearMemory, HibernateStatus status,
        const std::shared_ptr<HibernateController>& hibernateController, const std::shared_ptr<PowerMgrNotify>& notify);
    void RollbackHibernate(
        PowerState originalState, bool clearMemory, bool needShutdown, const sptr<PowerMgrService>& pms);
    uint32_t GetPreHibernateDelay();
#endif
#ifdef HAS_SENSORS_SENSOR_PART
    bool IsProximityClose();
#endif
    void StartSleepTimer(PowerState from);
#ifdef HAS_HIVIEWDFX_HISYSEVENT_PART
    bool ReportScreenOffInvalidEvent(StateChangeReason reason);
    bool ReportAbnormalScreenOffEvent(StateChangeReason reason);
#endif
#ifdef POWER_MANAGER_TAKEOVER_SUSPEND
    TransitResult TakeOverSuspendAction(StateChangeReason reason);
#endif

    const wptr<PowerMgrService> pms_;
    std::shared_ptr<FFRTTimer> ffrtTimer_ {nullptr};
    PowerState currentState_;
    std::map<PowerState, std::shared_ptr<std::vector<RunningLockType>>> lockMap_;
    std::map<PowerState, std::shared_ptr<StateController>> controllerMap_;
    std::mutex mutex_;
    // all change to currentState_ should be inside stateMutex_
    std::mutex stateMutex_;
    DevicePowerState mDeviceState_;
    sptr<IRemoteObject::DeathRecipient> powerStateCBDeathRecipient_;
    std::set<const sptr<IPowerStateCallback>, classcomp> syncPowerStateListeners_;
    std::set<const sptr<IPowerStateCallback>, classcomp> asyncPowerStateListeners_;
    std::map<sptr<IPowerStateCallback>, std::pair<int32_t, int32_t>, classcomp> cachedRegister_;
    std::shared_ptr<IDeviceStateAction> stateAction_;

    std::atomic<int64_t> displayOffTime_ {DEFAULT_DISPLAY_OFF_TIME_MS};
    int64_t sleepTime_ {DEFAULT_SLEEP_TIME_MS};
    bool enableDisplaySuspend_ {false};
    bool isScreenOffTimeOverride_ {false};
    std::unordered_map<PowerState, std::set<PowerState>> forbidMap_;
    uint32_t switchActionValue_ {2}; // default value is 2 (stand for ACTION_FORCE_SUSPEND)
    std::atomic<bool> switchOpen_ {true};
#ifdef POWER_MANAGER_POWER_ENABLE_S4
    std::atomic<bool> hibernating_ {false};
#endif
    std::unordered_map<StateChangeReason, std::unordered_map<PowerState, std::set<PowerState>>> allowMapByReason_;
    std::atomic<bool> forceTimingOut_ {false};
    std::atomic<bool> enabledTimingOutLockScreen_ {true};
    std::atomic<bool> enabledTimingOutLockScreenCheckLock_ {false};
    std::atomic<bool> enabledScreenOffEvent_{true};
    std::atomic<int64_t> settingStateFlag_ {-1};
    std::atomic<bool> settingOnStateFlag_ {false};
    std::atomic<bool> settingOffStateFlag_ {false};
    std::atomic<bool> isAwakeNotified_ {false};
    std::atomic<PreBrightState> preBrightState_ {PRE_BRIGHT_UNSTART};
    std::atomic<bool> proximityScreenOffTimerStarted_ {false};
    std::atomic<bool> isDozeEnabled_ {false};
#ifdef POWER_MANAGER_ENABLE_EXTERNAL_SCREEN_MANAGEMENT
    std::atomic<int32_t> externalScreenNumber_ {0};
    std::mutex internalScreenStateMutex_;
    static constexpr int32_t DISPLAY_MODE_ONLY_SECOND_SCREEN = 8;
#endif
    std::atomic<bool> isDuringCall_ {false};
    bool SetDreamingState(StateChangeReason reason);
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_POWER_STATE_MACHINE_H
