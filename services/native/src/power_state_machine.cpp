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

#include "power_state_machine.h"

#include <algorithm>
#include <cinttypes>
#include <datetime_ex.h>
#ifdef HAS_HIVIEWDFX_HISYSEVENT_PART
#include <hisysevent.h>
#endif
#include <ipc_skeleton.h>
#include "power_ext_intf_wrapper.h"
#ifdef HAS_HIVIEWDFX_HITRACE_PART
#include "power_hitrace.h"
#endif
#include "power_mode_policy.h"
#include "power_mgr_factory.h"
#include "power_mgr_service.h"
#include "power_utils.h"
#include "setting_helper.h"
#include "system_suspend_controller.h"
#ifdef POWER_MANAGER_POWER_ENABLE_S4
#include "os_account_manager.h"
#include "parameters.h"
#endif
#ifdef MSDP_MOVEMENT_ENABLE
#include <dlfcn.h>
#endif
#include "customized_screen_event_rules.h"

namespace OHOS {
namespace PowerMgr {
namespace {
#ifdef POWER_MANAGER_ENABLE_CHARGING_TYPE_SETTING
sptr<SettingObserver> g_displayOffTimeAcObserver;
sptr<SettingObserver> g_displayOffTimeDcObserver;
#else
sptr<SettingObserver> g_displayOffTimeObserver;
#endif
static int64_t g_beforeOverrideTime {-1};
#ifdef HAS_HIVIEWDFX_HISYSEVENT_PART
constexpr int32_t DISPLAY_OFF = 0;
constexpr int32_t DISPLAY_ON = 2;
#endif
const std::string POWERMGR_STOPSERVICE = "persist.powermgr.stopservice";
constexpr uint32_t PRE_BRIGHT_AUTH_TIMER_DELAY_MS = 3000;
#ifdef POWER_MANAGER_POWER_ENABLE_S4
constexpr uint32_t POST_HIBERNATE_CLEARMEM_DELAY_US = 2000000;
constexpr uint32_t HIBERNATE_DELAY_MS = 3500;
static int64_t g_preHibernateStart = 0;
#endif
pid_t g_callSetForceTimingOutPid = 0;
pid_t g_callSetForceTimingOutUid = 0;
}
PowerStateMachine::PowerStateMachine(const wptr<PowerMgrService>& pms, const std::shared_ptr<FFRTTimer>& ffrtTimer)
    : pms_(pms), ffrtTimer_(ffrtTimer), currentState_(PowerState::UNKNOWN)
{
    POWER_HILOGD(FEATURE_POWER_STATE, "Instance start");
    // NOTICE Need get screen state when device startup,
    // rightnow we set screen is on as default
    mDeviceState_.screenState.lastOnTime = GetTickCount();
    mDeviceState_.screenState.lastOffTime = 0;
    mDeviceState_.lastWakeupEventTime = 0;
    mDeviceState_.lastRefreshActivityTime = 0;
    mDeviceState_.lastWakeupDeviceTime = 0;
    mDeviceState_.lastSuspendDeviceTime = 0;

    // init lock map which will block state transit
    std::vector<RunningLockType> awakeBlocker {};
    std::vector<RunningLockType> freezeBlocker {};
    std::vector<RunningLockType> inactiveBlocker {RunningLockType::RUNNINGLOCK_SCREEN};
    std::vector<RunningLockType> standByBlocker {};
    std::vector<RunningLockType> dozeBlocker {};
    std::vector<RunningLockType> sleepBlocker {
        RunningLockType::RUNNINGLOCK_COORDINATION
    };
    std::vector<RunningLockType> hibernateBlocker {};
    std::vector<RunningLockType> shutdownBlocker {};

    lockMap_.emplace(PowerState::AWAKE, std::make_shared<std::vector<RunningLockType>>(awakeBlocker));
    lockMap_.emplace(PowerState::FREEZE, std::make_shared<std::vector<RunningLockType>>(freezeBlocker));
    lockMap_.emplace(PowerState::INACTIVE, std::make_shared<std::vector<RunningLockType>>(inactiveBlocker));
    lockMap_.emplace(PowerState::STAND_BY, std::make_shared<std::vector<RunningLockType>>(standByBlocker));
    lockMap_.emplace(PowerState::DOZE, std::make_shared<std::vector<RunningLockType>>(dozeBlocker));
    lockMap_.emplace(PowerState::SLEEP, std::make_shared<std::vector<RunningLockType>>(sleepBlocker));
    lockMap_.emplace(PowerState::HIBERNATE, std::make_shared<std::vector<RunningLockType>>(hibernateBlocker));
    lockMap_.emplace(PowerState::SHUTDOWN, std::make_shared<std::vector<RunningLockType>>(shutdownBlocker));

    POWER_HILOGD(FEATURE_POWER_STATE, "Instance end");
}

PowerStateMachine::~PowerStateMachine()
{
    ffrtTimer_.reset();
}

bool PowerStateMachine::Init()
{
    POWER_HILOGD(FEATURE_POWER_STATE, "Start init");
    stateAction_ = PowerMgrFactory::GetDeviceStateAction();
    InitTransitMap();
    InitStateMap();

    if (powerStateCBDeathRecipient_ == nullptr) {
        powerStateCBDeathRecipient_ = new PowerStateCallbackDeathRecipient();
    }
    POWER_HILOGD(FEATURE_POWER_STATE, "Init success");
    return true;
}

void PowerStateMachine::InitTransitMap()
{
#ifdef POWER_MANAGER_POWER_ENABLE_S4
    std::vector<PowerState> awake { PowerState::SLEEP };
    std::vector<PowerState> hibernate { PowerState::SLEEP };
#else
    std::vector<PowerState> awake { PowerState::SLEEP, PowerState::HIBERNATE };
#endif
    std::vector<PowerState> inactive { PowerState::DIM };
    std::vector<PowerState> dim { PowerState::SLEEP };
    std::vector<PowerState> sleep { PowerState::DIM };

    forbidMap_.emplace(PowerState::AWAKE, std::set<PowerState>(awake.begin(), awake.end()));
    forbidMap_.emplace(PowerState::INACTIVE, std::set<PowerState>(inactive.begin(), inactive.end()));
    forbidMap_.emplace(PowerState::DIM, std::set<PowerState>(dim.begin(), dim.end()));
    forbidMap_.emplace(PowerState::SLEEP, std::set<PowerState>(sleep.begin(), sleep.end()));
#ifdef POWER_MANAGER_POWER_ENABLE_S4
    forbidMap_.emplace(PowerState::HIBERNATE, std::set<PowerState>(hibernate.begin(), hibernate.end()));
#endif

    allowMapByReason_.insert({
        {
            StateChangeReason::STATE_CHANGE_REASON_REFRESH,
            {
                {PowerState::DIM, {PowerState::AWAKE}},
                {PowerState::AWAKE, {PowerState::AWAKE}}
            }
        },
        {
            StateChangeReason::STATE_CHANGE_REASON_TIMEOUT,
            {
                // allow AWAKE to INACTIVE without going to DIM for UTs to pass
                {PowerState::AWAKE, {PowerState::DIM, PowerState::INACTIVE}},
                {PowerState::DIM, {PowerState::INACTIVE}},
                {PowerState::INACTIVE, {PowerState::SLEEP}}
            }
        },
        {
            StateChangeReason::STATE_CHANGE_REASON_TIMEOUT_NO_SCREEN_LOCK,
            {
                {PowerState::DIM, {PowerState::INACTIVE}},
                // allow AWAKE to INACTIVE without going to DIM for UTs to pass
                {PowerState::AWAKE, {PowerState::INACTIVE}}
            }
        },
    });
}

#ifdef MSDP_MOVEMENT_ENABLE
static const char* MOVEMENT_STATE_CONFIG = "GetMovementState";
static const char* POWER_MANAGER_EXT_PATH = "libpower_manager_ext.z.so";
typedef bool(*FuncMovementState)();

bool PowerStateMachine::IsMovementStateOn()
{
    POWER_HILOGD(FEATURE_POWER_STATE, "Start to GetMovementState");
    void *stateHandler = dlopen(POWER_MANAGER_EXT_PATH, RTLD_LAZY | RTLD_NODELETE);
    if (stateHandler == nullptr) {
        POWER_HILOGE(FEATURE_POWER_STATE, "Dlopen GetMovementState failed, reason : %{public}s", dlerror());
        return false;
    }

    FuncMovementState MovementStateFlag = reinterpret_cast<FuncMovementState>(dlsym(stateHandler,
        MOVEMENT_STATE_CONFIG));
    if (MovementStateFlag == nullptr) {
        POWER_HILOGE(FEATURE_POWER_STATE, "GetMovementState is null, reason : %{public}s", dlerror());
        dlclose(stateHandler);
        stateHandler = nullptr;
        return false;
    }
    bool ret = MovementStateFlag();
    dlclose(stateHandler);
    stateHandler = nullptr;
    return ret;
}
#endif

bool PowerStateMachine::CanTransitTo(PowerState from, PowerState to, StateChangeReason reason)
{
    bool isForbidden = forbidMap_.count(currentState_) && forbidMap_[currentState_].count(to);
    if (isForbidden) {
        return false;
    }
    // prevent the double click or pickup to light up the screen when calling or sporting or proximity is close
    if ((reason == StateChangeReason::STATE_CHANGE_REASON_DOUBLE_CLICK ||
             reason == StateChangeReason::STATE_CHANGE_REASON_PICKUP) && to == PowerState::AWAKE) {
#ifdef HAS_SENSORS_SENSOR_PART
        if (IsProximityClose()) {
            POWER_HILOGI(FEATURE_POWER_STATE,
                "Double-click or pickup isn't allowed to wakeup device when proximity is close during calling.");
            StartSleepTimer(from);
            return false;
        }
        // prevent the pickup to light up the screen when proximity is close out of calling
        if (ProximityNormalController::IsInactiveClose() && reason == StateChangeReason::STATE_CHANGE_REASON_PICKUP) {
            POWER_HILOGI(FEATURE_POWER_STATE,
                "Pickup isn't allowed to wakeup device when proximity is close out of calling.");
            StartSleepTimer(from);
            return false;
        }
        // prevent the pickup to light up the screen when lid is close
        if (PowerMgrService::isInLidMode_ == true && reason == StateChangeReason::STATE_CHANGE_REASON_PICKUP) {
            POWER_HILOGI(FEATURE_POWER_STATE, "Pickup isn't allowed to wakeup device when lid is close.");
            StartSleepTimer(from);
            return false;
        }
#endif
#ifdef MSDP_MOVEMENT_ENABLE
        if (IsMovementStateOn()) {
            POWER_HILOGI(FEATURE_POWER_STATE,
                "Double-click or pickup isn't allowed to wakeup device when movement state is on.");
            StartSleepTimer(from);
            return false;
        }
#endif
    }
    if (reason == StateChangeReason::STATE_CHANGE_REASON_PROXIMITY && to == PowerState::INACTIVE &&
        !IsRunningLockEnabled(RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL)) {
        POWER_HILOGI(FEATURE_POWER_STATE, "this proximity task is invalidated, directly return");
        return false;
    }
    bool isAllowed = (!allowMapByReason_.count(reason) ||
        (allowMapByReason_[reason].count(currentState_) && allowMapByReason_[reason][currentState_].count(to)));
    return isAllowed;
}

void PowerStateMachine::StartSleepTimer(PowerState from)
{
    if (from == PowerState::SLEEP) {
        uint32_t delay = 0;
        POWER_HILOGI(FEATURE_POWER_STATE, "Double-click or pickup isn't allowed to wakeup device, SetAutoSuspend");
        SetAutoSuspend(SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, delay);
    }
}

void PowerStateMachine::InitState()
{
    POWER_HILOGD(FEATURE_POWER_STATE, "Init power state");
    if (IsScreenOn()) {
#ifdef HAS_HIVIEWDFX_HISYSEVENT_PART
        HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::DISPLAY, "SCREEN_STATE",
            HiviewDFX::HiSysEvent::EventType::STATISTIC, "STATE", DISPLAY_ON);
#endif
        SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_INIT, true);
    } else {
#ifdef HAS_HIVIEWDFX_HISYSEVENT_PART
        HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::DISPLAY, "SCREEN_STATE",
            HiviewDFX::HiSysEvent::EventType::STATISTIC, "STATE", DISPLAY_OFF);
#endif
        SetState(PowerState::INACTIVE, StateChangeReason::STATE_CHANGE_REASON_INIT, true);
    }
}

void PowerStateMachine::EmplaceAwake()
{
    controllerMap_.emplace(PowerState::AWAKE,
        std::make_shared<StateController>(PowerState::AWAKE, shared_from_this(), [this](StateChangeReason reason) {
            POWER_HILOGD(FEATURE_POWER_STATE, "[UL_POWER] StateController_AWAKE lambda start, reason=%{public}s",
                PowerUtils::GetReasonTypeString(reason).c_str());
#ifdef HAS_HIVIEWDFX_HISYSEVENT_PART
            HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::POWER_UE, "SCREEN_ON",
                HiviewDFX::HiSysEvent::EventType::BEHAVIOR, "PNAMEID", "PowerManager", "PVERSIONID", "1.0",
                "REASON", PowerUtils::GetReasonTypeString(reason).c_str());
            HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::POWER, "SCREEN_ON",
                HiviewDFX::HiSysEvent::EventType::BEHAVIOR, "REASON", PowerUtils::GetReasonTypeString(reason).c_str());
#endif
#ifdef POWER_MANAGER_ENABLE_WATCH_CUSTOMIZED_SCREEN_COMMON_EVENT_RULES
            DelayedSingleton<CustomizedScreenEventRules>::GetInstance()->SetScreenOnEventRules(reason);
#endif
            mDeviceState_.screenState.lastOnTime = GetTickCount();
            auto targetState = DisplayState::DISPLAY_ON;
            if (reason == StateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_FAIL_SCREEN_OFF) {
                if (isDozeEnabled_) {
                    targetState = DisplayState::DISPLAY_DOZE;
                } else {
                    targetState = DisplayState::DISPLAY_OFF;
                }
            }
#ifdef POWER_MANAGER_ENABLE_EXTERNAL_SCREEN_MANAGEMENT
            if (reason == StateChangeReason::STATE_CHANGE_REASON_SWITCH && IsSwitchOpen()) {
                this->stateAction_->SetInternalScreenDisplayPower(DisplayState::DISPLAY_ON, reason);
            }
#endif
            uint32_t ret = this->stateAction_->SetDisplayState(targetState, reason);
            if (ret != ActionResult::SUCCESS) {
                POWER_HILOGE(FEATURE_POWER_STATE, "Failed to go to AWAKE, display error, ret: %{public}u", ret);
                return TransitResult::DISPLAY_ON_ERR;
            }
            if (reason != StateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT) {
                ResetInactiveTimer();
            }
            SystemSuspendController::GetInstance().DisallowAutoSleep();
            SystemSuspendController::GetInstance().Wakeup();
            return TransitResult::SUCCESS;
        }));
}

void PowerStateMachine::EmplaceFreeze()
{
    controllerMap_.emplace(PowerState::FREEZE,
        std::make_shared<StateController>(PowerState::FREEZE, shared_from_this(), [this](StateChangeReason reason) {
            POWER_HILOGI(FEATURE_POWER_STATE, "StateController_FREEZE lambda start");
            // Subsequent added functions
            return TransitResult::SUCCESS;
        }));
}

void PowerStateMachine::EmplaceInactive()
{
    controllerMap_.emplace(PowerState::INACTIVE,
        std::make_shared<StateController>(PowerState::INACTIVE, shared_from_this(), [this](StateChangeReason reason) {
            POWER_HILOGD(FEATURE_POWER_STATE, "[UL_POWER] StateController_INACTIVE lambda start");
#ifdef HAS_HIVIEWDFX_HISYSEVENT_PART
            HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::POWER_UE, "SCREEN_OFF",
                HiviewDFX::HiSysEvent::EventType::BEHAVIOR, "PNAMEID", "PowerManager", "PVERSIONID", "1.0",
                "REASON", PowerUtils::GetReasonTypeString(reason).c_str());
#endif
            mDeviceState_.screenState.lastOffTime = GetTickCount();
            DisplayState state = DisplayState::DISPLAY_OFF;
            if (isDozeEnabled_.load(std::memory_order_relaxed)) {
                state = DisplayState::DISPLAY_DOZE;
            }
            uint32_t ret = this->stateAction_->SetDisplayState(state, reason);
            if (ret != ActionResult::SUCCESS) {
                POWER_HILOGE(FEATURE_POWER_STATE, "Failed to go to INACTIVE, display error, ret: %{public}u", ret);
                return TransitResult::DISPLAY_OFF_ERR;
            }
            CancelDelayTimer(PowerStateMachine::CHECK_USER_ACTIVITY_TIMEOUT_MSG);
            CancelDelayTimer(PowerStateMachine::CHECK_USER_ACTIVITY_OFF_TIMEOUT_MSG);
            return TransitResult::SUCCESS;
        }));
}

void PowerStateMachine::EmplaceStandBy()
{
    controllerMap_.emplace(PowerState::STAND_BY,
        std::make_shared<StateController>(PowerState::STAND_BY, shared_from_this(), [this](StateChangeReason reason) {
            POWER_HILOGI(FEATURE_POWER_STATE, "StateController_STAND_BY lambda start");
            mDeviceState_.screenState.lastOffTime = GetTickCount();
            // Subsequent added functions
            return TransitResult::SUCCESS;
        }));
}

void PowerStateMachine::EmplaceDoze()
{
    controllerMap_.emplace(PowerState::DOZE,
        std::make_shared<StateController>(PowerState::DOZE, shared_from_this(), [this](StateChangeReason reason) {
            POWER_HILOGI(FEATURE_POWER_STATE, "StateController_DOZE lambda start");
            mDeviceState_.screenState.lastOffTime = GetTickCount();
            // Subsequent added functions
            return TransitResult::SUCCESS;
        }));
}

void PowerStateMachine::EmplaceSleep()
{
    controllerMap_.emplace(PowerState::SLEEP,
        std::make_shared<StateController>(PowerState::SLEEP, shared_from_this(), [this](StateChangeReason reason) {
            POWER_HILOGI(FEATURE_POWER_STATE, "StateController_SLEEP lambda start");
            SystemSuspendController::GetInstance().AllowAutoSleep();
            return TransitResult::SUCCESS;
        }));
}

void PowerStateMachine::EmplaceHibernate()
{
    controllerMap_.emplace(PowerState::HIBERNATE,
        std::make_shared<StateController>(PowerState::HIBERNATE, shared_from_this(), [this](StateChangeReason reason) {
            POWER_HILOGI(FEATURE_POWER_STATE, "StateController_HIBERNATE lambda start");
            return TransitResult::SUCCESS;
        }));
}

void PowerStateMachine::EmplaceShutdown()
{
    controllerMap_.emplace(PowerState::SHUTDOWN,
        std::make_shared<StateController>(PowerState::SHUTDOWN, shared_from_this(), [this](StateChangeReason reason) {
            POWER_HILOGI(FEATURE_POWER_STATE, "StateController_SHUTDOWN lambda start");
            // Subsequent added functions
            return TransitResult::SUCCESS;
        }));
}

void PowerStateMachine::EmplaceDim()
{
    controllerMap_.emplace(PowerState::DIM,
        std::make_shared<StateController>(PowerState::DIM, shared_from_this(), [this](StateChangeReason reason) {
            POWER_HILOGD(FEATURE_POWER_STATE, "[UL_POWER] StateController_DIM lambda start");
            if (GetDisplayOffTime() < 0) {
                POWER_HILOGD(FEATURE_ACTIVITY, "Auto display off is disabled");
                return TransitResult::OTHER_ERR;
            }
            int64_t dimTime = GetDimTime(GetDisplayOffTime());
            if (reason == StateChangeReason::STATE_CHANGE_REASON_COORDINATION) {
                dimTime = COORDINATED_STATE_SCREEN_OFF_TIME_MS;
            }
            uint32_t ret = stateAction_->SetDisplayState(DisplayState::DISPLAY_DIM, reason);
            if (ret != ActionResult::SUCCESS) {
                // failed but not return, still need to set screen off
                POWER_HILOGE(FEATURE_POWER_STATE, "Failed to go to DIM, display error, ret: %{public}u", ret);
            }
            CancelDelayTimer(PowerStateMachine::CHECK_USER_ACTIVITY_TIMEOUT_MSG);
            CancelDelayTimer(PowerStateMachine::CHECK_USER_ACTIVITY_OFF_TIMEOUT_MSG);
            // Set a timer without checking runninglock, but the actual timeout event can still be blocked.
            // Theoretically, this timer is always cancelable before the current task is finished.
            SetDelayTimer(dimTime, PowerStateMachine::CHECK_USER_ACTIVITY_OFF_TIMEOUT_MSG);
            // in case a refresh action occurs, change display state back to on
            if (settingStateFlag_.load() ==
                static_cast<int64_t>(SettingStateFlag::StateFlag::SETTING_DIM_INTERRUPTED)) {
                stateAction_->SetDisplayState(DisplayState::DISPLAY_ON, StateChangeReason::STATE_CHANGE_REASON_REFRESH);
                ResetInactiveTimer();
                POWER_HILOGW(FEATURE_POWER_STATE, "Setting DIM interrupted!");
                return TransitResult::OTHER_ERR;
            }
            return ret == ActionResult::SUCCESS ? TransitResult::SUCCESS : TransitResult::OTHER_ERR;
        }));
}

void PowerStateMachine::InitStateMap()
{
    EmplaceAwake();
    EmplaceFreeze();
    EmplaceInactive();
    EmplaceStandBy();
    EmplaceDoze();
    EmplaceSleep();
    EmplaceHibernate();
    EmplaceShutdown();
    EmplaceDim();
}

void PowerStateMachine::onSuspend()
{
    POWER_HILOGI(FEATURE_SUSPEND, "System is suspending");
}

void PowerStateMachine::onWakeup()
{
    POWER_HILOGI(FEATURE_WAKEUP, "System is awaking");
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        return;
    }
    FFRTTask task = [&pms] {
        pms->GetPowerStateMachine()->HandleSystemWakeup();
    };
}

void PowerStateMachine::SuspendDeviceInner(
    pid_t pid, int64_t callTimeMs, SuspendDeviceType type, bool suspendImmed, bool ignoreScreenState)
{
#ifdef HAS_HIVIEWDFX_HITRACE_PART
    PowerHitrace powerHitrace("SuspendDevice");
#endif
    if (type > SuspendDeviceType::SUSPEND_DEVICE_REASON_MAX) {
        POWER_HILOGW(FEATURE_SUSPEND, "Invalid type: %{public}d", type);
        return;
    }
    // Check the screen state
    if (!ignoreScreenState) {
        if (stateAction_ != nullptr) {
            stateAction_->Suspend(
                callTimeMs, type, suspendImmed ? SUSPEND_DEVICE_IMMEDIATELY : SUSPEND_DEVICE_NEED_DOZE);
        }
        mDeviceState_.lastSuspendDeviceTime = callTimeMs;
        POWER_HILOGD(FEATURE_SUSPEND, "Suspend device success");
    } else {
        POWER_HILOGD(FEATURE_SUSPEND, "Do not suspend device, screen state is ignored");
    }

    if (SetState(PowerState::INACTIVE, GetReasonBySuspendType(type), true)) {
        uint32_t delay = 0;
        SetAutoSuspend(type, delay);
    }
    POWER_HILOGD(FEATURE_SUSPEND, "Suspend device finish");
}

bool PowerStateMachine::IsPreBrightAuthReason(StateChangeReason reason)
{
    bool ret = false;
    switch (reason) {
        case StateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_SUCCESS:
            ret = true;
            break;
        case StateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_FAIL_SCREEN_ON:
            ret = true;
            break;
        case StateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_FAIL_SCREEN_OFF:
            ret = true;
            break;
        default:
            break;
    }
    return ret;
}

bool PowerStateMachine::IsPreBrightWakeUp(WakeupDeviceType type)
{
    bool ret = false;
    switch (type) {
        case WakeupDeviceType::WAKEUP_DEVICE_PRE_BRIGHT:
            ret = true;
            break;
        case WakeupDeviceType::WAKEUP_DEVICE_PRE_BRIGHT_AUTH_SUCCESS:
            ret = true;
            break;
        case WakeupDeviceType::WAKEUP_DEVICE_PRE_BRIGHT_AUTH_FAIL_SCREEN_ON:
            ret = true;
            break;
        case WakeupDeviceType::WAKEUP_DEVICE_PRE_BRIGHT_AUTH_FAIL_SCREEN_OFF:
            ret = true;
            break;
        default:
            break;
    }
    return ret;
}

void PowerStateMachine::HandlePreBrightWakeUp(int64_t callTimeMs, WakeupDeviceType type, const std::string& details,
    const std::string& pkgName, bool timeoutTriggered)
{
    POWER_HILOGD(FEATURE_WAKEUP, "This wakeup event is trigged by %{public}s.", details.c_str());

    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    auto suspendController = pms->GetSuspendController();
    if (suspendController != nullptr) {
        suspendController->StopSleep();
    } else {
        POWER_HILOGI(FEATURE_WAKEUP, "suspendController is nullptr, can't stop sleep ffrt task");
    }
    if (stateAction_ != nullptr) {
        stateAction_->Wakeup(callTimeMs, type, details, pkgName);
    }
    mDeviceState_.lastWakeupDeviceTime = callTimeMs;

    StateChangeReason reason = GetReasonByWakeType(type);
    if (!timeoutTriggered && IsPreBrightAuthReason(reason)) {
        POWER_HILOGI(FEATURE_WAKEUP, "Cancel pre-bright-auth timer, rason=%{public}s",
            PowerUtils::GetReasonTypeString(reason).c_str());
        CancelDelayTimer(PowerStateMachine::CHECK_PRE_BRIGHT_AUTH_TIMEOUT_MSG);
    }
    SetState(PowerState::AWAKE, reason, true);

    switch (type) {
        case WakeupDeviceType::WAKEUP_DEVICE_PRE_BRIGHT: {
            break;
        }
        case WakeupDeviceType::WAKEUP_DEVICE_PRE_BRIGHT_AUTH_SUCCESS: // fall through
        case WakeupDeviceType::WAKEUP_DEVICE_PRE_BRIGHT_AUTH_FAIL_SCREEN_ON:
            if (suspendController != nullptr) {
                POWER_HILOGD(FEATURE_WAKEUP, "HandlePreBrightWakeUp. TriggerSyncSleepCallback start.");
                suspendController->TriggerSyncSleepCallback(true);
            } else {
                POWER_HILOGI(FEATURE_WAKEUP, "HandlePreBrightWakeUp. suspendController is nullptr");
            }
            break;
        case WakeupDeviceType::WAKEUP_DEVICE_PRE_BRIGHT_AUTH_FAIL_SCREEN_OFF:
            if (suspendController != nullptr) {
                suspendController->StartSleepTimer(
                    SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION,
                    static_cast<uint32_t>(SuspendAction::ACTION_AUTO_SUSPEND), 0);
            } else {
                POWER_HILOGI(FEATURE_WAKEUP, "suspendController is nullptr, can't restore sleep ffrt task");
            }
            break;
        default:
            break;
    }

    return;
}

void PowerStateMachine::WakeupDeviceInner(
    pid_t pid, int64_t callTimeMs, WakeupDeviceType type, const std::string& details, const std::string& pkgName)
{
#ifdef HAS_HIVIEWDFX_HITRACE_PART
    PowerHitrace powerHitrace("WakeupDevice");
#endif
    if (type > WakeupDeviceType::WAKEUP_DEVICE_MAX) {
        POWER_HILOGW(FEATURE_WAKEUP, "Invalid type: %{public}d", type);
        return;
    }

#ifdef POWER_MANAGER_POWER_ENABLE_S4
    if (!IsSwitchOpen() || IsHibernating()) {
#else
    if (!IsSwitchOpen()) {
#endif
        POWER_HILOGI(FEATURE_WAKEUP, "Switch is closed or hibernating, wakeup device do nothing.");
        return;
    }

    if (type == WakeupDeviceType::WAKEUP_DEVICE_APPLICATION) {
        type = PowerUtils::ParseWakeupDeviceType(details);
    }

    if (IsPreBrightWakeUp(type)) {
        HandlePreBrightWakeUp(callTimeMs, type, details, pkgName);
        return;
    }

    // Call legacy wakeup, Check the screen state
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    auto suspendController = pms->GetSuspendController();
    if (suspendController != nullptr) {
        POWER_HILOGI(FEATURE_WAKEUP, "Stop sleep ffrt task");
        suspendController->StopSleep();
    }

    if (stateAction_ != nullptr) {
        stateAction_->Wakeup(callTimeMs, type, details, pkgName);
    }
    mDeviceState_.lastWakeupDeviceTime = callTimeMs;

    SetState(PowerState::AWAKE, GetReasonByWakeType(type), true);

    if (suspendController != nullptr) {
        POWER_HILOGD(FEATURE_WAKEUP, "WakeupDeviceInner. TriggerSyncSleepCallback start.");
        suspendController->TriggerSyncSleepCallback(true);
    } else {
        POWER_HILOGI(FEATURE_WAKEUP, "WakeupDeviceInner. suspendController is nullptr");
    }

    POWER_HILOGD(FEATURE_WAKEUP, "Wakeup device finish");
}

void PowerStateMachine::RefreshActivityInner(
    pid_t pid, int64_t callTimeMs, UserActivityType type, bool needChangeBacklight)
{
    if (type > UserActivityType::USER_ACTIVITY_TYPE_MAX) {
        POWER_HILOGW(FEATURE_ACTIVITY, "Invalid type: %{public}d", type);
        return;
    }
    // Check the screen state
    if (IsScreenOn() && !IsSettingState(PowerState::INACTIVE)) {
        if (stateAction_ != nullptr) {
            stateAction_->RefreshActivity(callTimeMs, type,
                needChangeBacklight ? REFRESH_ACTIVITY_NEED_CHANGE_LIGHTS : REFRESH_ACTIVITY_NO_CHANGE_LIGHTS);
            mDeviceState_.screenState.lastOnTime = GetTickCount();
        }
        if (GetState() == PowerState::DIM || IsSettingState(PowerState::DIM)) {
            // Inactive to Awake will be blocked for this reason in CanTransitTo()
            SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_REFRESH, true);
        } else {
            // There is a small chance that the last "if" statement occurs before the (already started) ffrt task
            // is actually trying to set DIM state.
            // In that case we may still (not guaranteed) interrupt it.
            ResetInactiveTimer(false);
        }
    } else {
        POWER_HILOGD(FEATURE_ACTIVITY, "Ignore refresh activity, screen is off");
    }
}

bool PowerStateMachine::CheckRefreshTime()
{
    // The minimum refreshactivity interval is 100ms!!
    int64_t now = GetTickCount();
    if ((mDeviceState_.lastRefreshActivityTime + MIN_TIME_MS_BETWEEN_USERACTIVITIES) > now) {
        return true;
    }
    mDeviceState_.lastRefreshActivityTime = now;
    return false;
}

bool PowerStateMachine::OverrideScreenOffTimeInner(int64_t timeout)
{
    if (!isScreenOffTimeOverride_) {
        int64_t beforeOverrideTime = this->GetDisplayOffTime();
        isScreenOffTimeOverride_ = true;
        g_beforeOverrideTime = beforeOverrideTime;
    }
    this->SetDisplayOffTime(timeout, false);
    POWER_HILOGD(COMP_SVC, "Override screenOffTime finish");
    return true;
}

bool PowerStateMachine::RestoreScreenOffTimeInner()
{
    if (!isScreenOffTimeOverride_) {
        POWER_HILOGD(COMP_SVC, "RestoreScreenOffTime is not override, no need to restore");
        return false;
    }
    this->SetDisplayOffTime(g_beforeOverrideTime, false);
    isScreenOffTimeOverride_ = false;
    POWER_HILOGD(COMP_SVC, "Restore screenOffTime finish");
    return true;
}

bool PowerStateMachine::ForceSuspendDeviceInner(pid_t pid, int64_t callTimeMs)
{
    SetState(
        PowerState::INACTIVE, GetReasonBySuspendType(SuspendDeviceType::SUSPEND_DEVICE_REASON_FORCE_SUSPEND), true);
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    auto suspendController = pms->GetSuspendController();
    if (suspendController != nullptr) {
        POWER_HILOGI(FEATURE_SUSPEND, "ForceSuspendDeviceInner StartSleepTimer start.");
        suspendController->StartSleepTimer(
            SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION,
            static_cast<uint32_t>(SuspendAction::ACTION_FORCE_SUSPEND), 0);
    }

    POWER_HILOGI(FEATURE_SUSPEND, "Force suspend finish");
    return true;
}

#ifdef POWER_MANAGER_POWER_ENABLE_S4
bool PowerStateMachine::PrepareHibernate(bool clearMemory)
{
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    auto hibernateController = pms->GetHibernateController();
    if (hibernateController == nullptr) {
        POWER_HILOGE(FEATURE_SUSPEND, "hibernateController is nullptr.");
        return false;
    }
    SystemSuspendController::GetInstance().Wakeup();
    bool ret = true;
    if (!SetState(PowerState::INACTIVE, StateChangeReason::STATE_CHANGE_REASON_HIBERNATE, true)) {
        POWER_HILOGE(FEATURE_POWER_STATE, "failed to set state to inactive.");
    }
    
    g_preHibernateStart = GetTickCount();
    if (clearMemory) {
        PowerExtIntfWrapper::Instance().SubscribeScreenLockCommonEvent();
        if (AccountSA::OsAccountManager::DeactivateAllOsAccounts() != ERR_OK) {
            POWER_HILOGE(FEATURE_SUSPEND, "deactivate all os accounts failed.");
            return false;
        }
        int32_t id;
        if (AccountSA::OsAccountManager::GetDefaultActivatedOsAccount(id) != ERR_OK) {
            POWER_HILOGE(FEATURE_SUSPEND, "get default activated os account failed.");
            return false;
        }
        if (AccountSA::OsAccountManager::ActivateOsAccount(id) != ERR_OK) {
            POWER_HILOGE(FEATURE_SUSPEND, "activate os account failed.");
            return false;
        }
        if (!OHOS::system::SetParameter(POWERMGR_STOPSERVICE.c_str(), "true")) {
            POWER_HILOGE(FEATURE_SUSPEND, "set parameter POWERMGR_STOPSERVICE true failed.");
            return false;
        }
        PowerExtIntfWrapper::Instance().BlockHibernateUntilScrLckReady();
        PowerExtIntfWrapper::Instance().UnSubscribeScreenLockCommonEvent();
    }
    hibernateController->PreHibernate();

    if (!SetState(PowerState::HIBERNATE, StateChangeReason::STATE_CHANGE_REASON_SYSTEM, true)) {
        POWER_HILOGE(FEATURE_POWER_STATE, "failed to set state to hibernate.");
        ret = false;
    }
    if (ret && clearMemory) {
        usleep(POST_HIBERNATE_CLEARMEM_DELAY_US);
    }
    return ret;
}

uint32_t PowerStateMachine::GetPreHibernateDelay()
{
    int64_t preHibernateEnd = GetTickCount();
    uint32_t preHibernateDelay = static_cast<uint32_t>(preHibernateEnd - g_preHibernateStart);
    preHibernateDelay = preHibernateDelay > HIBERNATE_DELAY_MS ? 0 : HIBERNATE_DELAY_MS - preHibernateDelay;
    POWER_HILOGI(FEATURE_SUSPEND, "preHibernateDelay = %{public}u", preHibernateDelay);
    return preHibernateDelay;
}

void PowerStateMachine::RestoreHibernate(bool clearMemory, HibernateStatus status,
    std::shared_ptr<HibernateController>& hibernateController, std::shared_ptr<PowerMgrNotify>& notify)
{
    // hibernateController and notify already judge empty
    bool hibernateRes = (status == HibernateStatus::HIBERNATE_SUCCESS);
    if (hibernateRes) {
        switchOpen_ = true;
    }
    hibernating_ = false;

    int64_t exitTime = GetTickCount();
    notify->PublishExitHibernateEvent(exitTime);

    if (!SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_SYSTEM, true)) {
        POWER_HILOGE(FEATURE_POWER_STATE, "failed to set state to awake when hibernate.");
    }
    if (clearMemory) {
        if (!OHOS::system::SetParameter(POWERMGR_STOPSERVICE.c_str(), "false")) {
            POWER_HILOGE(FEATURE_SUSPEND, "set parameter POWERMGR_STOPSERVICE false failed.");
        }
    }
    hibernateController->PostHibernate(hibernateRes);
}

FFRTTask PowerStateMachine::CreateHibernateFfrtTask(bool clearMemory, sptr<PowerMgrService>& pms,
    std::shared_ptr<HibernateController>& hibernateController, std::shared_ptr<PowerMgrNotify>& notify)
{
    // pms, hibernateController and notify already judge empty
    FFRTTask task = [this, clearMemory, pms, hibernateController, notify]() {
        HibernateStatus status = hibernateController->Hibernate(clearMemory);
        if (status != HibernateStatus::HIBERNATE_SUCCESS && clearMemory) {
            POWER_HILOGE(FEATURE_SUSPEND, "hibernate failed, shutdown begin.");
            pms->ShutDownDevice("HibernateFail");
            hibernating_ = false;
            return;
        }
        RestoreHibernate(clearMemory, status, hibernateController, notify);
        POWER_HILOGI(FEATURE_SUSPEND, "power mgr machine hibernate end.");
    };
    return task;
}

bool PowerStateMachine::HibernateInner(bool clearMemory)
{
    POWER_HILOGI(FEATURE_POWER_STATE, "HibernateInner begin.");
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        POWER_HILOGE(FEATURE_SUSPEND, "PowerMgr service is nullptr");
        return false;
    }
    auto shutdownController = pms->GetShutdownController();
    auto hibernateController = pms->GetHibernateController();
    auto notify = pms->GetPowerMgrNotify();
    if (shutdownController == nullptr || hibernateController == nullptr ||
        notify == nullptr || ffrtTimer_ == nullptr) {
        POWER_HILOGE(FEATURE_SUSPEND,
            "shutdown controller or hibernate controller or notify or ffrtTimer_ is nullptr.");
        return false;
    }

    if (clearMemory) {
        // do takeover only when user presses "shutdown" menu
        bool takenOver = shutdownController->TriggerTakeOverHibernateCallback(TakeOverInfo("hibernate", clearMemory));
        if (takenOver) {
            POWER_HILOGE(FEATURE_SUSPEND, "Hibernating is taken over by OnTakeOverHibernate callback");
            return true;
        }
    }
    if (hibernating_) {
        POWER_HILOGE(FEATURE_SUSPEND, "the device is hibernating, please try again later.");
        return false;
    }
    hibernating_ = true;

    int64_t enterTime = GetTickCount();
    notify->PublishEnterHibernateEvent(enterTime);

    if (!PrepareHibernate(clearMemory) && clearMemory) {
        POWER_HILOGE(FEATURE_SUSPEND, "prepare hibernate failed, shutdown begin.");
        pms->ShutDownDevice("HibernateFail");
        hibernating_ = false;
        return true;
    }

    FFRTTask task = CreateHibernateFfrtTask(clearMemory, pms, hibernateController, notify);
    ffrtTimer_->SetTimer(TIMER_ID_HIBERNATE, task, 0);
    return true;
}
#endif

bool PowerStateMachine::IsScreenOn(bool needPrintLog)
{
#ifdef POWER_MANAGER_ENABLE_EXTERNAL_SCREEN_MANAGEMENT
    bool isScreenOn {false};
    // When there's external screen, the original way to get screen state is inaccurate,
    // so use PowerState instead
    if (GetExternalScreenNumber() > 0) {
        PowerState powerState = GetState();
        isScreenOn = (powerState == PowerState::AWAKE) || (powerState == PowerState::DIM);
    } else {
        DisplayState displayState = stateAction_->GetDisplayState();
        isScreenOn = (displayState == DisplayState::DISPLAY_ON) || (displayState == DisplayState::DISPLAY_DIM);
    }
#else
    DisplayState state = stateAction_->GetDisplayState();
    bool isScreenOn = (state == DisplayState::DISPLAY_ON) || (state == DisplayState::DISPLAY_DIM);
#endif

    if (needPrintLog) {
        POWER_HILOGD(FEATURE_POWER_STATE, "Current screen is %{public}s", isScreenOn ? "ON" : "OFF");
    }
    return isScreenOn;
}

bool PowerStateMachine::IsFoldScreenOn()
{
    POWER_HILOGI(FEATURE_POWER_STATE,
        "IsFoldScreenOn settingOnStateFlag_ is %{public}d and settingOffStateFlag_ is %{public}d",
        settingOnStateFlag_.load(), settingOffStateFlag_.load());

    if (settingOnStateFlag_ == true) {
        POWER_HILOGI(FEATURE_POWER_STATE, "Current fold screen is going on");
        return true;
    }
    if (settingOffStateFlag_ == true) {
        POWER_HILOGI(FEATURE_POWER_STATE, "Current fold screen is going off");
        return false;
    }
    DisplayState state = stateAction_->GetDisplayState();
    if (state == DisplayState::DISPLAY_ON || state == DisplayState::DISPLAY_DIM) {
        POWER_HILOGI(FEATURE_POWER_STATE, "Current fold screen is on or going on");
        return true;
    }
    POWER_HILOGI(FEATURE_POWER_STATE, "Current fold screen state is off, state: %{public}u", state);
    return false;
}

bool PowerStateMachine::IsCollaborationScreenOn()
{
    return isAwakeNotified_.load(std::memory_order_relaxed);
}

void PowerStateMachine::ReceiveScreenEvent(bool isScreenOn)
{
    POWER_HILOGD(FEATURE_POWER_STATE, "Enter");
    std::lock_guard lock(mutex_);
    auto prestate = mDeviceState_.screenState.state;
    if (isScreenOn) {
        mDeviceState_.screenState.lastOnTime = GetTickCount();
    } else {
        mDeviceState_.screenState.lastOffTime = GetTickCount();
    }
    if (prestate != mDeviceState_.screenState.state) {
        NotifyPowerStateChanged(isScreenOn ? PowerState::AWAKE : PowerState::INACTIVE);
    }
}

void PowerStateMachine::RegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback, bool isSync)
{
    std::lock_guard lock(mutex_);
    RETURN_IF(callback == nullptr);
    auto object = callback->AsObject();
    RETURN_IF(object == nullptr);

    bool result = false;
    if (isSync) {
        auto retIt = syncPowerStateListeners_.insert(callback);
        result = retIt.second;
        POWER_HILOGD(FEATURE_POWER_STATE, "sync listeners.size = %{public}d, insertOk = %{public}d",
            static_cast<unsigned int>(syncPowerStateListeners_.size()), retIt.second);
    } else {
        auto retIt = asyncPowerStateListeners_.insert(callback);
        result = retIt.second;
        POWER_HILOGD(FEATURE_POWER_STATE, "async listeners.size = %{public}d, insertOk = %{public}d",
            static_cast<unsigned int>(asyncPowerStateListeners_.size()), retIt.second);
    }
    if (result) {
        object->AddDeathRecipient(powerStateCBDeathRecipient_);
    }
}

void PowerStateMachine::UnRegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback)
{
    std::lock_guard lock(mutex_);
    RETURN_IF(callback == nullptr);
    auto object = callback->AsObject();
    RETURN_IF(object == nullptr);
    size_t eraseNum = 0;
    if (syncPowerStateListeners_.find(callback) != syncPowerStateListeners_.end()) {
        eraseNum = syncPowerStateListeners_.erase(callback);
        if (eraseNum != 0) {
            object->RemoveDeathRecipient(powerStateCBDeathRecipient_);
        }
        POWER_HILOGD(FEATURE_POWER_STATE, "sync listeners.size = %{public}d, eraseNum = %{public}zu",
            static_cast<unsigned int>(syncPowerStateListeners_.size()), eraseNum);
    } else {
        eraseNum = asyncPowerStateListeners_.erase(callback);
        if (eraseNum != 0) {
            object->RemoveDeathRecipient(powerStateCBDeathRecipient_);
        }
        POWER_HILOGD(FEATURE_POWER_STATE, "async listeners.size = %{public}d, eraseNum = %{public}zu",
            static_cast<unsigned int>(asyncPowerStateListeners_.size()), eraseNum);
    }
}

void PowerStateMachine::EnableMock(IDeviceStateAction* mockAction)
{
    std::lock_guard lock(mutex_);
    displayOffTime_ = DEFAULT_DISPLAY_OFF_TIME_MS;
    sleepTime_ = DEFAULT_SLEEP_TIME_MS;
    ResetInactiveTimer();

    std::unique_ptr<IDeviceStateAction> mock(mockAction);
    if (stateAction_ != nullptr) {
        stateAction_.reset();
    }
    stateAction_ = std::move(mock);
}

void PowerStateMachine::NotifyPowerStateChanged(PowerState state, StateChangeReason reason)
{
    if (GetState() == PowerState::INACTIVE &&
        !enabledScreenOffEvent_.load(std::memory_order_relaxed) &&
        reason == StateChangeReason::STATE_CHANGE_REASON_TIMEOUT_NO_SCREEN_LOCK) {
        POWER_HILOGI(FEATURE_POWER_STATE, "[UL_POWER] not notify inactive power state");
        return;
    }
    POWER_HILOGD(
        FEATURE_POWER_STATE, "state=%{public}u, listeners.size=%{public}zu", state, syncPowerStateListeners_.size());
#ifdef HAS_HIVIEWDFX_HISYSEVENT_PART
    HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::POWER, "STATE", HiviewDFX::HiSysEvent::EventType::STATISTIC, "STATE",
        static_cast<uint32_t>(state));
#endif
    std::lock_guard lock(mutex_);
    int64_t now = GetTickCount();
    // Send Notification event
    SendEventToPowerMgrNotify(state, now);

    // Call back all native function
    for (auto& listener : asyncPowerStateListeners_) {
        listener->OnAsyncPowerStateChanged(state);
    }
    for (auto& listener : syncPowerStateListeners_) {
        listener->OnPowerStateChanged(state);
    }
}

void PowerStateMachine::SendEventToPowerMgrNotify(PowerState state, int64_t callTime)
{
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        POWER_HILOGE(FEATURE_POWER_STATE, "Pms is nullptr");
        return;
    }
    auto notify = pms->GetPowerMgrNotify();
    if (notify == nullptr) {
        POWER_HILOGE(FEATURE_POWER_STATE, "Notify is null");
        return;
    }

    switch (state) {
        case PowerState::AWAKE: {
            DelayedSingleton<CustomizedScreenEventRules>::GetInstance()->SendCustomizedScreenEvent(
                notify, PowerState::AWAKE, callTime);
            notify->PublishScreenOnEvents(callTime);
            isAwakeNotified_.store(true, std::memory_order_relaxed);
#ifdef POWER_MANAGER_ENABLE_FORCE_SLEEP_BROADCAST
            auto suspendController = pms->GetSuspendController();
            if (suspendController != nullptr && suspendController->GetForceSleepingFlag()) {
                notify->PublishExitForceSleepEvents(callTime);
                POWER_HILOGI(FEATURE_POWER_STATE, "Set flag of force sleeping to false");
                suspendController->SetForceSleepingFlag(false);
            }
#endif
            break;
        }
        case PowerState::INACTIVE: {
            DelayedSingleton<CustomizedScreenEventRules>::GetInstance()->SendCustomizedScreenEvent(
                notify, PowerState::INACTIVE, callTime);
            notify->PublishScreenOffEvents(callTime);
            isAwakeNotified_.store(false, std::memory_order_relaxed);
            break;
        }
        case PowerState::SLEEP: {
#ifdef POWER_MANAGER_ENABLE_FORCE_SLEEP_BROADCAST
            auto suspendController = pms->GetSuspendController();
            if (suspendController != nullptr && suspendController->GetForceSleepingFlag()) {
                notify->PublishEnterForceSleepEvents(callTime);
            }
            break;
#endif
            POWER_HILOGI(FEATURE_POWER_STATE, "No need to publish EnterForceSleepEvent, state:%{public}u", state);
            break;
        }
        default:
            POWER_HILOGI(FEATURE_POWER_STATE, "No need to publish event, state:%{public}u", state);
    }
}

void PowerStateMachine::PowerStateCallbackDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    if (remote == nullptr || remote.promote() == nullptr) {
        return;
    }
    sptr<IPowerStateCallback> callback = iface_cast<IPowerStateCallback>(remote.promote());
    FFRTTask unRegFunc = [callback] {
        auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
        if (pms == nullptr) {
            POWER_HILOGE(FEATURE_POWER_STATE, "Pms is nullptr");
            return;
        }
        pms->UnRegisterPowerStateCallback(callback);
    };
    FFRTUtils::SubmitTask(unRegFunc);
}

void PowerStateMachine::SetDelayTimer(int64_t delayTime, int32_t event)
{
    if (!ffrtTimer_) {
        POWER_HILOGE(FEATURE_ACTIVITY, "Failed to set delay timer, the timer pointer is null");
        return;
    }
    POWER_HILOGD(FEATURE_ACTIVITY, "Set delay timer, delayTime=%{public}s, event=%{public}d",
        std::to_string(delayTime).c_str(), event);

    switch (event) {
        case CHECK_USER_ACTIVITY_TIMEOUT_MSG: {
            FFRTTask task = [this] { this->HandleActivityTimeout(); };
            ffrtTimer_->SetTimer(TIMER_ID_USER_ACTIVITY_TIMEOUT, task, delayTime);
            break;
        }
        case CHECK_USER_ACTIVITY_OFF_TIMEOUT_MSG: {
            auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
            auto suspendController = pms->GetSuspendController();
            if (suspendController == nullptr) {
                POWER_HILOGW(FEATURE_ACTIVITY, "suspendController is nullptr");
                return;
            }
            suspendController->HandleEvent(delayTime);
            break;
        }
        case CHECK_PROXIMITY_SCREEN_OFF_MSG: {
            FFRTTask delayScreenOffTask = [this] {
                POWER_HILOGI(FEATURE_POWER_STATE, "proximity-screen-off timer task is triggered");
                proximityScreenOffTimerStarted_.store(false, std::memory_order_relaxed);
                auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
                auto suspendController = pms->GetSuspendController();
                if (suspendController == nullptr) {
                    POWER_HILOGW(
                        FEATURE_POWER_STATE, "suspendController is nullptr, exit proximity-screen-off timer task");
                    return;
                }
                bool ret = SetState(PowerState::INACTIVE, StateChangeReason::STATE_CHANGE_REASON_PROXIMITY, true);
                if (ret) {
                    suspendController->StartSleepTimer(SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION,
                        static_cast<uint32_t>(SuspendAction::ACTION_AUTO_SUSPEND), 0);
                }
            };
            ffrtTimer_->SetTimer(TIMER_ID_PROXIMITY_SCREEN_OFF, delayScreenOffTask, delayTime);
            proximityScreenOffTimerStarted_.store(true, std::memory_order_relaxed);
            break;
        }
        default: {
            break;
        }
    }
}

void PowerStateMachine::CancelDelayTimer(int32_t event)
{
    if (!ffrtTimer_) {
        POWER_HILOGE(FEATURE_ACTIVITY, "Failed to cancel delay timer, the timer pointer is null");
        return;
    }
    POWER_HILOGD(FEATURE_ACTIVITY, "Cancel delay timer, event: %{public}d", event);

    switch (event) {
        case CHECK_USER_ACTIVITY_TIMEOUT_MSG: {
            ffrtTimer_->CancelTimer(TIMER_ID_USER_ACTIVITY_TIMEOUT);
            break;
        }
        case CHECK_USER_ACTIVITY_OFF_TIMEOUT_MSG: {
            auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
            auto suspendController = pms->GetSuspendController();
            if (suspendController == nullptr) {
                POWER_HILOGW(FEATURE_ACTIVITY, "suspendController is nullptr");
                return;
            }
            suspendController->CancelEvent();
            break;
        }
        case CHECK_PRE_BRIGHT_AUTH_TIMEOUT_MSG: {
            ffrtTimer_->CancelTimer(TIMER_ID_PRE_BRIGHT_AUTH);
            break;
        }
        case CHECK_PROXIMITY_SCREEN_OFF_MSG: {
            ffrtTimer_->CancelTimer(TIMER_ID_PROXIMITY_SCREEN_OFF);
            proximityScreenOffTimerStarted_.store(false, std::memory_order_relaxed);
            break;
        }
        default: {
            break;
        }
    }
}

void PowerStateMachine::ResetInactiveTimer(bool needPrintLog)
{
    // change the flag to notify the thread which is setting DIM
    int64_t expectedFlag = static_cast<int64_t>(PowerState::DIM);
    settingStateFlag_.compare_exchange_strong(
        expectedFlag, static_cast<int64_t>(SettingStateFlag::StateFlag::SETTING_DIM_INTERRUPTED));
    CancelDelayTimer(PowerStateMachine::CHECK_USER_ACTIVITY_TIMEOUT_MSG);
    CancelDelayTimer(PowerStateMachine::CHECK_USER_ACTIVITY_OFF_TIMEOUT_MSG);
    if (this->GetDisplayOffTime() < 0) {
        if (needPrintLog) {
            POWER_HILOGI(FEATURE_ACTIVITY, "Auto display off is disabled");
        }
        return;
    }

    int64_t displayOffTime = this->GetDisplayOffTime();
    ResetScreenOffPreTimeForSwing(displayOffTime);
    this->SetDelayTimer(
        displayOffTime - this->GetDimTime(displayOffTime), PowerStateMachine::CHECK_USER_ACTIVITY_TIMEOUT_MSG);
    if (needPrintLog) {
        POWER_HILOGI(FEATURE_ACTIVITY, "reset inactive timer: %{public}" PRId64, displayOffTime);
    }
}

void PowerStateMachine::ResetScreenOffPreTimeForSwing(int64_t displayOffTime)
{
    int64_t now = GetTickCount();
    int64_t nextTimeOut = now + displayOffTime - this->GetDimTime(displayOffTime);
    POWER_HILOGD(FEATURE_SCREEN_OFF_PRE,
        "now=%{public}lld,displayOffTime=%{public}lld,nextTimeOut=%{public}lld",
        static_cast<long long>(now), static_cast<long long>(displayOffTime), static_cast<long long>(nextTimeOut));
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    auto screenOffPreController = pms->GetScreenOffPreController();
    if (screenOffPreController != nullptr && screenOffPreController->IsRegistered()) {
        screenOffPreController->SchedulEyeDetectTimeout(nextTimeOut, now);
    }
}

void PowerStateMachine::ResetSleepTimer()
{
    CancelDelayTimer(PowerStateMachine::CHECK_USER_ACTIVITY_TIMEOUT_MSG);
    CancelDelayTimer(PowerStateMachine::CHECK_USER_ACTIVITY_OFF_TIMEOUT_MSG);
    if (this->GetSleepTime() < 0) {
        POWER_HILOGD(FEATURE_ACTIVITY, "Auto sleep is disabled");
        return;
    }
}

void PowerStateMachine::SetAutoSuspend(SuspendDeviceType type, uint32_t delay)
{
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        POWER_HILOGE(FEATURE_SUSPEND, "Pms is nullptr");
        return;
    }
    auto suspendController = pms->GetSuspendController();
    if (suspendController == nullptr) {
        POWER_HILOGE(FEATURE_SUSPEND, "Suspend controller is nullptr");
        return;
    }
    suspendController->StartSleepTimer(type, static_cast<uint32_t>(SuspendAction::ACTION_AUTO_SUSPEND), delay);
    POWER_HILOGD(FEATURE_SUSPEND, "Set auto suspend finish");
}

void PowerStateMachine::ShowCurrentScreenLocks()
{
    auto pms = pms_.promote();
    if (pms == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Pms is nullptr");
        return;
    }
    std::map<std::string, RunningLockInfo> screenOnLockLists;
    pms_->QueryRunningLockListsInner(screenOnLockLists);
    std::string message;
    uint32_t mapSize = screenOnLockLists.size();
    uint32_t counter = 0;
    for (auto it : screenOnLockLists) {
        counter++;
        message.append(std::to_string(counter)).append(". ")
            .append("bundleName=").append(it.second.bundleName)
            .append(" name=").append(it.second.name)
            .append(" pid=").append(std::to_string(it.second.pid))
            .append(". ");
    }
    if (counter == 0) {
        return;
    }
    POWER_HILOGI(FEATURE_RUNNING_LOCK, "%{public}d screen on locks as follows: %{public}s", mapSize, message.c_str());
}

#ifdef HAS_SENSORS_SENSOR_PART
bool PowerStateMachine::IsProximityClose()
{
    auto pms = pms_.promote();
    if (pms == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Pms is nullptr");
        return false;
    }
    auto runningLockMgr = pms->GetRunningLockMgr();
    if (runningLockMgr == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "RunningLockMgr is nullptr");
        return false;
    }
    return runningLockMgr->IsProximityClose();
}
#endif

void PowerStateMachine::HandleActivityTimeout()
{
    POWER_HILOGD(FEATURE_ACTIVITY, "Enter, displayState = %{public}d", stateAction_->GetDisplayState());
    SetState(PowerState::DIM, StateChangeReason::STATE_CHANGE_REASON_TIMEOUT);
}

void PowerStateMachine::HandleActivitySleepTimeout()
{
    POWER_HILOGD(FEATURE_ACTIVITY, "Enter, displayState = %{public}d", stateAction_->GetDisplayState());
    if (!this->CheckRunningLock(PowerState::SLEEP)) {
        POWER_HILOGW(FEATURE_POWER_STATE, "RunningLock is blocking to transit to SLEEP");
        return;
    }
    DisplayState dispState = stateAction_->GetDisplayState();
    if (dispState == DisplayState::DISPLAY_OFF) {
        SetState(PowerState::SLEEP, StateChangeReason::STATE_CHANGE_REASON_TIMEOUT);
    } else {
        POWER_HILOGW(FEATURE_ACTIVITY, "Display is on, ignore activity sleep timeout, state = %{public}d", dispState);
    }
}

void PowerStateMachine::HandleSystemWakeup()
{
    POWER_HILOGD(FEATURE_WAKEUP, "Enter, displayState = %{public}d", stateAction_->GetDisplayState());
    if (IsScreenOn()) {
        SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_SYSTEM, true);
    } else {
        SetState(PowerState::INACTIVE, StateChangeReason::STATE_CHANGE_REASON_SYSTEM, true);
    }
}

void PowerStateMachine::SetForceTimingOut(bool enabled)
{
    bool isScreenOnLockActive = IsRunningLockEnabled(RunningLockType::RUNNINGLOCK_SCREEN);
    bool currentValue = forceTimingOut_.exchange(enabled);
    pid_t pid = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    PowerState curState = GetState();
    if (!enabled) {
        g_callSetForceTimingOutPid = 0;
        g_callSetForceTimingOutUid = 0;
    } else {
        g_callSetForceTimingOutPid = pid;
        g_callSetForceTimingOutUid = uid;
    }
    POWER_HILOGI(FEATURE_RUNNING_LOCK,
        "SetForceTimingOut: %{public}s -> %{public}s, screenOnLockActive=%{public}s, PowerState=%{public}u, "
        "PID=%{public}d, UID=%{public}d",
        currentValue ? "TRUE" : "FALSE", enabled ? "TRUE" : "FALSE", isScreenOnLockActive ? "TRUE" : "FALSE", curState,
        pid, uid);
    if (currentValue == enabled || !isScreenOnLockActive || IsSettingState(PowerState::DIM)) {
        // no need to interact with screen state or timer
        return;
    }
    if (enabled) {
        // In case the PowerState is AWAKE, we need to reset the timer since there is no existing one.
        // Only reset the timer if no SetState operation is currently in progress, and if any,
        // make sure this ResetInactiveTimer operation does not interfere with it.
        // Because the goal here is to ensure that there exist some Timer, regardless who sets the timer.
        // I call it "weak" ResetInactiveTimer to distinguish it from the "strong" one invoked by RefreshActivity,
        // which (should) invalidates any time-out timer previously set.
        if (stateMutex_.try_lock()) {
            if (GetState() == PowerState::AWAKE) {
                ResetInactiveTimer();
            }
            stateMutex_.unlock();
        }
    } else {
        // SetForceTimingOut from TRUE to FALSE, with screen-on-lock active.
        // Need to exit DIM and/or reset(cancel) timer
        SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_REFRESH);
    }
}

void PowerStateMachine::LockScreenAfterTimingOut(bool enabled, bool checkScreenOnLock, bool sendScreenOffEvent)
{
    pid_t pid = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    POWER_HILOGI(FEATURE_RUNNING_LOCK,
        "LockScreenAfterTimingOut: %{public}u, %{public}u, %{public}u, PID=%{public}d, UID=%{public}d",
        static_cast<uint32_t>(enabled), static_cast<uint32_t>(checkScreenOnLock),
        static_cast<uint32_t>(sendScreenOffEvent), pid, uid);
    enabledTimingOutLockScreen_.store(enabled, std::memory_order_relaxed);
    enabledTimingOutLockScreenCheckLock_.store(checkScreenOnLock, std::memory_order_relaxed);
    enabledScreenOffEvent_.store(sendScreenOffEvent, std::memory_order_relaxed);
}

void PowerStateMachine::SetEnableDoze(bool enable)
{
    isDozeEnabled_.store(enable, std::memory_order_relaxed);
}

bool PowerStateMachine::SetDozeMode(DisplayState state)
{
    std::lock_guard<std::mutex> lock(stateMutex_);
    if (IsScreenOn()) {
        POWER_HILOGW(FEATURE_POWER_STATE, "the screen is on, not allowed to set doze mode");
        return false;
    }
    uint32_t ret =
        this->stateAction_->SetDisplayState(state, StateChangeReason::STATE_CHANGE_REASON_SWITCHING_DOZE_MODE);
    return ret == ActionResult::SUCCESS;
}

bool PowerStateMachine::CheckRunningLock(PowerState state)
{
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "Enter, state = %{public}u", state);
    auto pms = pms_.promote();
    if (pms == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Pms is nullptr");
        return false;
    }
    auto runningLockMgr = pms->GetRunningLockMgr();
    if (runningLockMgr == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "RunningLockMgr is nullptr");
        return false;
    }
    if (state == PowerState::DIM) {
        // screen on lock need to block DIM state as well
        state = PowerState::INACTIVE;
        POWER_HILOGI(FEATURE_RUNNING_LOCK, "check Screen on Lock for DIM state");
    }
    auto iterator = lockMap_.find(state);
    if (iterator == lockMap_.end()) {
        POWER_HILOGI(FEATURE_RUNNING_LOCK, "No specific lock in lockMap_ for state: %{public}u", state);
        return true;
    }

    std::shared_ptr<std::vector<RunningLockType>> pLock = iterator->second;
    for (std::vector<RunningLockType>::const_iterator iter = pLock->begin(); iter != pLock->end(); ++iter) {
        uint32_t count = runningLockMgr->GetValidRunningLockNum(*iter);
        if (count > 0) {
            POWER_HILOGD(FEATURE_POWER_STATE,
                "RunningLock %{public}s is locking (count=%{public}d), blocking %{public}s",
                PowerUtils::GetRunningLockTypeString(*iter).c_str(), count,
                PowerUtils::GetPowerStateString(state).c_str());
            return false;
        }
    }

    POWER_HILOGD(FEATURE_RUNNING_LOCK, "No specific lock for state: %{public}u", state);
    return true;
}

bool PowerStateMachine::IsRunningLockEnabled(RunningLockType type)
{
    auto pms = pms_.promote();
    if (pms == nullptr) {
        POWER_HILOGE(FEATURE_POWER_STATE, "Pms is nullptr");
        return false;
    }
    auto runningLockMgr = pms->GetRunningLockMgr();
    if (runningLockMgr == nullptr) {
        POWER_HILOGE(FEATURE_POWER_STATE, "RunningLockMgr is nullptr");
        return false;
    }
    if (runningLockMgr->GetValidRunningLockNum(type) > 0) {
        return true;
    }
    return false;
}

void PowerStateMachine::SetDisplayOffTime(int64_t time, bool needUpdateSetting)
{
    POWER_HILOGI(FEATURE_POWER_STATE, "set display off time %{public}" PRId64 " -> %{public}" PRId64 "",
        displayOffTime_.load(), time);
    displayOffTime_ = time;
    if (currentState_ == PowerState::AWAKE) {
        if (isScreenOffTimeOverride_ == true) {
            ResetInactiveTimer(false);
        } else {
            ResetInactiveTimer();
        }
    }
    if (needUpdateSetting) {
#ifdef POWER_MANAGER_ENABLE_CHARGING_TYPE_SETTING
        SettingHelper::SetSettingDisplayAcScreenOffTime(displayOffTime_);
        SettingHelper::SetSettingDisplayDcScreenOffTime(displayOffTime_);
#else
        SettingHelper::SetSettingDisplayOffTime(displayOffTime_);
#endif
    }
}

void PowerStateMachine::DisplayOffTimeUpdateFunc()
{
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        POWER_HILOGE(FEATURE_POWER_STATE, "get PowerMgrService fail");
        return;
    }
    auto stateMachine = pms->GetPowerStateMachine();
    if (stateMachine == nullptr) {
        POWER_HILOGE(FEATURE_POWER_STATE, "get PowerStateMachine fail");
        return;
    }

    int64_t systemTime = stateMachine->GetDisplayOffTime();
    int64_t settingTime = pms->GetSettingDisplayOffTime(systemTime);
    if (settingTime == systemTime) {
        POWER_HILOGI(FEATURE_POWER_STATE, "setting display off time %{public}" PRId64 " already worked", settingTime);
        return;
    }
    POWER_HILOGI(FEATURE_POWER_STATE, "setting update display off time %{public}" PRId64 " -> %{public}" PRId64 "",
        systemTime, settingTime);
    g_beforeOverrideTime = settingTime;
    auto policy = DelayedSingleton<PowerModePolicy>::GetInstance();
    if (policy == nullptr) {
        POWER_HILOGE(FEATURE_POWER_STATE, "get PowerModePolicy fail");
        return;
    }
    policy->RemoveBackupMapSettingSwitch(PowerModePolicy::ServiceType::DISPLAY_OFFTIME);
    stateMachine->SetDisplayOffTime(settingTime, false);
}

void PowerStateMachine::RegisterDisplayOffTimeObserver()
{
#ifdef POWER_MANAGER_ENABLE_CHARGING_TYPE_SETTING
    if (g_displayOffTimeAcObserver && g_displayOffTimeDcObserver) {
#else
    if (g_displayOffTimeObserver) {
#endif
        POWER_HILOGI(FEATURE_POWER_STATE, "setting display off time observer is already registered");
        return;
    }
    DisplayOffTimeUpdateFunc();
    SettingObserver::UpdateFunc updateFunc = [&](const std::string&) {
        PowerStateMachine::DisplayOffTimeUpdateFunc();
    };
#ifdef POWER_MANAGER_ENABLE_CHARGING_TYPE_SETTING
    if (g_displayOffTimeAcObserver == nullptr) {
        g_displayOffTimeAcObserver = SettingHelper::RegisterSettingDisplayAcScreenOffTimeObserver(updateFunc);
    }
    if (g_displayOffTimeDcObserver == nullptr) {
        g_displayOffTimeDcObserver = SettingHelper::RegisterSettingDisplayDcScreenOffTimeObserver(updateFunc);
    }
#else
    g_displayOffTimeObserver = SettingHelper::RegisterSettingDisplayOffTimeObserver(updateFunc);
#endif
}

void PowerStateMachine::UnregisterDisplayOffTimeObserver()
{
#ifdef POWER_MANAGER_ENABLE_CHARGING_TYPE_SETTING
    if (g_displayOffTimeAcObserver) {
        SettingHelper::UnregisterSettingObserver(g_displayOffTimeAcObserver);
        g_displayOffTimeAcObserver = nullptr;
    }
    if (g_displayOffTimeDcObserver) {
        SettingHelper::UnregisterSettingObserver(g_displayOffTimeDcObserver);
        g_displayOffTimeDcObserver = nullptr;
    }
#else
    if (g_displayOffTimeObserver == nullptr) {
        POWER_HILOGD(FEATURE_POWER_STATE, "g_displayOffTimeObserver is nullptr, no need to unregister");
        return;
    }
    SettingHelper::UnregisterSettingObserver(g_displayOffTimeObserver);
    g_displayOffTimeObserver = nullptr;
#endif
}

void PowerStateMachine::SetSleepTime(int64_t time)
{
    sleepTime_ = time;
}

int64_t PowerStateMachine::GetDisplayOffTime()
{
    return displayOffTime_;
}

int64_t PowerStateMachine::GetDimTime(int64_t displayOffTime)
{
    int64_t dimTime = displayOffTime / OFF_TIMEOUT_FACTOR;
    return std::clamp(dimTime, static_cast<int64_t>(0), MAX_DIM_TIME_MS);
}

bool PowerStateMachine::IsSettingState(PowerState state)
{
    int64_t flag = settingStateFlag_.load();
    bool matched = flag == static_cast<int64_t>(state);
    if (matched) {
        return true;
    } else {
        return (
            state == PowerState::DIM && flag == static_cast<int64_t>(SettingStateFlag::StateFlag::FORCE_SETTING_DIM));
    }
}

int64_t PowerStateMachine::GetSleepTime()
{
    return sleepTime_;
}

PowerStateMachine::ScreenChangeCheck::ScreenChangeCheck(
    std::shared_ptr<FFRTTimer> ffrtTimer, PowerState state, StateChangeReason reason) :
    ffrtTimer_(ffrtTimer),
    state_(state), reason_(reason)
{
    // only check for screen on/off event
    if (state != PowerState::INACTIVE && state != PowerState::AWAKE) {
        return;
    }

    if (!ffrtTimer_) {
        POWER_HILOGE(FEATURE_POWER_STATE, "ScreenChangeCheck failed: invalid timer");
        return;
    }

    pid_ = IPCSkeleton::GetCallingPid();
    uid_ = IPCSkeleton::GetCallingUid();

    FFRTTask task = [checker = (*this)]() {
        checker.ReportSysEvent("TIMEOUT");
        checker.SetReportTimerStartFlag(false);
    };

    ffrtTimer_->SetTimer(TIMER_ID_SCREEN_TIMEOUT_CHECK, task, SCREEN_CHANGE_TIMEOUT_MS);
    isReportTimerStarted_ = true;
}

PowerStateMachine::ScreenChangeCheck::~ScreenChangeCheck() noexcept
{
    if (!isReportTimerStarted_) {
        return;
    }
    ffrtTimer_->CancelTimer(TIMER_ID_SCREEN_TIMEOUT_CHECK);
}

void PowerStateMachine::ScreenChangeCheck::SetReportTimerStartFlag(bool flag) const
{
    isReportTimerStarted_ = flag;
}

void PowerStateMachine::ScreenChangeCheck::ReportSysEvent(const std::string& msg) const
{
    const char* eventName = (state_ == PowerState::INACTIVE) ? "SCREEN_OFF_TIMEOUT" : "SCREEN_ON_TIMEOUT";
    POWER_HILOGE(FEATURE_POWER_STATE,
        "event=%{public}s, reason=%{public}s, msg=%{public}s, pid=%{public}d,"
        " uid=%{public}d",
        eventName, PowerUtils::GetReasonTypeString(reason_).c_str(), msg.c_str(), pid_, uid_);

    static int64_t lastReportTime = -1;
    int64_t now = GetTickCount();
    int64_t nextReportTime = lastReportTime + SCREEN_CHANGE_REPORT_INTERVAL_MS;
    if (nextReportTime > SCREEN_CHANGE_REPORT_INTERVAL_MS && now < nextReportTime) {
        POWER_HILOGD(FEATURE_POWER_STATE, "Will skip report for another %{public}s ms",
            std::to_string(nextReportTime - now).c_str());
        return;
    }
    lastReportTime = now;
#ifdef HAS_HIVIEWDFX_HISYSEVENT_PART
    HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::POWER, eventName, HiviewDFX::HiSysEvent::EventType::FAULT, "PID",
        pid_, "UID", uid_, "PACKAGE_NAME", "", "PROCESS_NAME", "", "MSG", msg.c_str(), "REASON",
        PowerUtils::GetReasonTypeString(reason_).c_str());
#endif
}

std::shared_ptr<PowerStateMachine::StateController> PowerStateMachine::GetStateController(PowerState state)
{
    auto iterator = controllerMap_.find(state);
    if (iterator == controllerMap_.end()) {
        return nullptr;
    }
    return iterator->second;
}

bool PowerStateMachine::NeedShowScreenLocks(PowerState state)
{
    return state == PowerState::AWAKE ||
        state == PowerState::INACTIVE || state == PowerState::DIM;
}

void PowerStateMachine::UpdateSettingStateFlag(PowerState state, StateChangeReason reason)
{
    if (reason == StateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT ||
        reason == StateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_FAIL_SCREEN_OFF) {
        settingOnStateFlag_ = false;
        settingOffStateFlag_ = true;
        return;
    }
    settingOnStateFlag_ = (state == PowerState::AWAKE);
    settingOffStateFlag_ = (state == PowerState::INACTIVE);
}

void PowerStateMachine::RestoreSettingStateFlag()
{
    settingOnStateFlag_ = false;
    settingOffStateFlag_ = false;
}

void PowerStateMachine::HandleProximityScreenOffTimer(PowerState state, StateChangeReason reason)
{
#ifdef HAS_SENSORS_SENSOR_PART
    if (!proximityScreenOffTimerStarted_.load()) {
        return;
    }
    if ((reason == StateChangeReason::STATE_CHANGE_REASON_DOUBLE_CLICK ||
            reason == StateChangeReason::STATE_CHANGE_REASON_PICKUP) &&
        IsProximityClose() && state == PowerState::AWAKE) {
        POWER_HILOGI(FEATURE_POWER_STATE, "Double-click or pickup is not allowed to cancel proximity-screen-off timer");
        return;
    }
    if (reason != StateChangeReason::STATE_CHANGE_REASON_PROXIMITY) {
        POWER_HILOGI(FEATURE_POWER_STATE, "Cancel proximity-screen-off timer, reason:%{public}s",
            PowerUtils::GetReasonTypeString(reason).c_str());
        CancelDelayTimer(PowerStateMachine::CHECK_PROXIMITY_SCREEN_OFF_MSG);
        return;
    }
    if (state == PowerState::AWAKE) {
        POWER_HILOGI(FEATURE_POWER_STATE, "Cancel proximity-screen-off timer, reason:%{public}s(away)",
            PowerUtils::GetReasonTypeString(reason).c_str());
        CancelDelayTimer(PowerStateMachine::CHECK_PROXIMITY_SCREEN_OFF_MSG);
    }
#endif
}

bool PowerStateMachine::HandlePreBrightState(PowerState targetState, StateChangeReason reason)
{
    bool ret = false;
    PowerStateMachine::PreBrightState curState = preBrightState_.load();
    if (reason == StateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT) {
        if (ffrtTimer_ != nullptr) {
            FFRTTask authFailTask = [this] {
                POWER_HILOGI(FEATURE_WAKEUP, "Auth result of PRE_BRIGHT isn't received within %{public}u ms",
                    PRE_BRIGHT_AUTH_TIMER_DELAY_MS);
                const std::string detail = "pre_bright_auth_fail_screen_off";
                const std::string pkgName = "pre_bright_auth_time";
                HandlePreBrightWakeUp(GetTickCount(), WakeupDeviceType::WAKEUP_DEVICE_PRE_BRIGHT_AUTH_FAIL_SCREEN_OFF,
                    detail, pkgName, true);
            };
            if (curState == PowerStateMachine::PRE_BRIGHT_STARTED) {
                POWER_HILOGI(FEATURE_WAKEUP, "Cancel pre-bright-auth timer, rason=%{public}s",
                    PowerUtils::GetReasonTypeString(reason).c_str());
                CancelDelayTimer(PowerStateMachine::CHECK_PRE_BRIGHT_AUTH_TIMEOUT_MSG);
            }
            POWER_HILOGI(FEATURE_POWER_STATE, "Start pre-bright-auth timer");
            ffrtTimer_->SetTimer(TIMER_ID_PRE_BRIGHT_AUTH, authFailTask, PRE_BRIGHT_AUTH_TIMER_DELAY_MS);
            preBrightState_.store(PowerStateMachine::PRE_BRIGHT_STARTED, std::memory_order_relaxed);
            ret = true;
        }
    } else if (IsPreBrightAuthReason(reason)) {
        if (curState == PowerStateMachine::PRE_BRIGHT_STARTED) {
            preBrightState_.store(PowerStateMachine::PRE_BRIGHT_FINISHED, std::memory_order_relaxed);
            ret = true;
        }
        POWER_HILOGW(
            FEATURE_POWER_STATE, "prebright first stage is not triggered, skip handling prebright auth result");
    } else {
        if (targetState != PowerState::SLEEP && curState == PowerStateMachine::PRE_BRIGHT_STARTED) {
            POWER_HILOGI(FEATURE_WAKEUP, "Cancel pre-bright-auth timer, rason=%{public}s",
                PowerUtils::GetReasonTypeString(reason).c_str());
            CancelDelayTimer(PowerStateMachine::CHECK_PRE_BRIGHT_AUTH_TIMEOUT_MSG);
        }
        if (targetState != PowerState::SLEEP) {
            preBrightState_.store(PowerStateMachine::PRE_BRIGHT_UNSTART, std::memory_order_relaxed);
        }
        ret = true;
    }
    POWER_HILOGD(FEATURE_WAKEUP, "Pre bright state: %{public}u", static_cast<uint32_t>(preBrightState_.load()));
    return ret;
}

bool PowerStateMachine::CheckFFRTTaskAvailability(PowerState state, StateChangeReason reason) const
{
    if (!IsTimeoutReason(reason)) {
        return true;
    }
    void* curTask = ffrt_get_cur_task();
    if (curTask == nullptr) {
        // not actually an ffrt task;
        return true;
    }
    if (!ffrtTimer_) {
        POWER_HILOGE(FEATURE_POWER_STATE, "ffrtTimer_ is nullptr");
        return false;
    }
    const void* pendingTask = nullptr;
    switch (state) {
        case PowerState::DIM:
            pendingTask = ffrtTimer_->GetTaskHandlePtr(TIMER_ID_USER_ACTIVITY_TIMEOUT);
            break;
        case PowerState::INACTIVE:
            pendingTask = ffrtTimer_->GetTaskHandlePtr(TIMER_ID_USER_ACTIVITY_OFF);
            break;
        default:
            pendingTask = ffrtTimer_->GetTaskHandlePtr(TIMER_ID_SLEEP);
            break;
    }
    return curTask == pendingTask;
}

bool PowerStateMachine::IsTimeoutReason(StateChangeReason reason) const
{
    return reason == StateChangeReason::STATE_CHANGE_REASON_TIMEOUT_NO_SCREEN_LOCK ||
        reason == StateChangeReason::STATE_CHANGE_REASON_TIMEOUT;
}

bool PowerStateMachine::SetState(PowerState state, StateChangeReason reason, bool force)
{
    POWER_HILOGD(FEATURE_POWER_STATE, "state=%{public}s, reason=%{public}s, force=%{public}d",
        PowerUtils::GetPowerStateString(state).c_str(), PowerUtils::GetReasonTypeString(reason).c_str(), force);
    std::lock_guard<std::mutex> lock(stateMutex_);
    if (!CheckFFRTTaskAvailability(state, reason)) {
        POWER_HILOGI(FEATURE_POWER_STATE, "this timeout task is invalidated, directly return");
        return false;
    }
    ScreenChangeCheck timeoutCheck(ffrtTimer_, state, reason);
    SettingStateFlag flag(state, shared_from_this(), reason);

    if (NeedShowScreenLocks(state)) {
        ShowCurrentScreenLocks();
    }

    HandleProximityScreenOffTimer(state, reason);
    std::shared_ptr<StateController> pController = GetStateController(state);
    if (pController == nullptr) {
        POWER_HILOGW(FEATURE_POWER_STATE, "StateController is not init");
        return false;
    }
    if (IsTimeoutReason(reason) && forceTimingOut_.load()) {
        force = true;
        POWER_HILOGI(FEATURE_POWER_STATE, "Call SetForceTimingOut PID=%{public}d, UID=%{public}d",
            g_callSetForceTimingOutPid, g_callSetForceTimingOutUid);
    }
    UpdateSettingStateFlag(state, reason);
    TransitResult ret = pController->TransitTo(reason, force);
    POWER_HILOGI(FEATURE_POWER_STATE, "[UL_POWER] StateController::TransitTo %{public}s ret: %{public}d",
        PowerUtils::GetPowerStateString(state).c_str(), ret);
    RestoreSettingStateFlag();
    return (ret == TransitResult::SUCCESS || ret == TransitResult::ALREADY_IN_STATE);
}

void PowerStateMachine::SetDisplaySuspend(bool enable)
{
    POWER_HILOGD(FEATURE_POWER_STATE, "enable: %{public}d", enable);
    std::lock_guard<std::mutex> lock(stateMutex_);
    enableDisplaySuspend_ = enable;
    if (GetState() == PowerState::INACTIVE) {
        POWER_HILOGI(FEATURE_POWER_STATE, "Change display state");
        if (enable) {
            stateAction_->SetDisplayState(
                DisplayState::DISPLAY_SUSPEND, StateChangeReason::STATE_CHANGE_REASON_APPLICATION);
        } else {
            stateAction_->SetDisplayState(
                DisplayState::DISPLAY_OFF, StateChangeReason::STATE_CHANGE_REASON_APPLICATION);
        }
    }
}

bool PowerStateMachine::TryToCancelScreenOff()
{
    return stateAction_->TryToCancelScreenOff();
}

void PowerStateMachine::BeginPowerkeyScreenOff()
{
    stateAction_->BeginPowerkeyScreenOff();
}

void PowerStateMachine::EndPowerkeyScreenOff()
{
    stateAction_->EndPowerkeyScreenOff();
}

StateChangeReason PowerStateMachine::GetReasonByWakeType(WakeupDeviceType type)
{
    POWER_HILOGD(FEATURE_WAKEUP, "WakeupDeviceType :%{public}u", type);
    StateChangeReason ret = StateChangeReason::STATE_CHANGE_REASON_UNKNOWN;
    switch (type) {
        case WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON:
            ret = StateChangeReason::STATE_CHANGE_REASON_POWER_KEY;
            break;
        case WakeupDeviceType::WAKEUP_DEVICE_WAKE_KEY:
            ret = StateChangeReason::STATE_CHANGE_REASON_HARD_KEY;
            break;
        case WakeupDeviceType::WAKEUP_DEVICE_APPLICATION:
            ret = StateChangeReason::STATE_CHANGE_REASON_APPLICATION;
            break;
        case WakeupDeviceType::WAKEUP_DEVICE_PLUGGED_IN: // fall through
        case WakeupDeviceType::WAKEUP_DEVICE_HDMI:
            ret = StateChangeReason::STATE_CHANGE_REASON_CABLE;
            break;
        case WakeupDeviceType::WAKEUP_DEVICE_GESTURE:
            ret = StateChangeReason::STATE_CHANGE_REASON_TOUCH;
            break;
        case WakeupDeviceType::WAKEUP_DEVICE_CAMERA_LAUNCH:
            ret = StateChangeReason::STATE_CHANGE_REASON_CAMERA;
            break;
        case WakeupDeviceType::WAKEUP_DEVICE_WAKE_MOTION:
            ret = StateChangeReason::STATE_CHANGE_REASON_SENSOR;
            break;
        case WakeupDeviceType::WAKEUP_DEVICE_LID:
            ret = StateChangeReason::STATE_CHANGE_REASON_LID;
            break;
        case WakeupDeviceType::WAKEUP_DEVICE_DOUBLE_CLICK:
            ret = StateChangeReason::STATE_CHANGE_REASON_DOUBLE_CLICK;
            break;
        case WakeupDeviceType::WAKEUP_DEVICE_PEN:
            ret = StateChangeReason::STATE_CHANGE_REASON_PEN;
            break;
        case WakeupDeviceType::WAKEUP_DEVICE_KEYBOARD:
            ret = StateChangeReason::STATE_CHANGE_REASON_KEYBOARD;
            break;
        case WakeupDeviceType::WAKEUP_DEVICE_MOUSE:
            ret = StateChangeReason::STATE_CHANGE_REASON_MOUSE;
            break;
        case WakeupDeviceType::WAKEUP_DEVICE_SWITCH:
            ret = StateChangeReason::STATE_CHANGE_REASON_SWITCH;
            break;
        case WakeupDeviceType::WAKEUP_DEVICE_PRE_BRIGHT:
            ret = StateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT;
            break;
        case WakeupDeviceType::WAKEUP_DEVICE_PRE_BRIGHT_AUTH_SUCCESS:
            ret = StateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_SUCCESS;
            break;
        case WakeupDeviceType::WAKEUP_DEVICE_PRE_BRIGHT_AUTH_FAIL_SCREEN_ON:
            ret = StateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_FAIL_SCREEN_ON;
            break;
        case WakeupDeviceType::WAKEUP_DEVICE_PRE_BRIGHT_AUTH_FAIL_SCREEN_OFF:
            ret = StateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_FAIL_SCREEN_OFF;
            break;
        case WakeupDeviceType::WAKEUP_DEVICE_AOD_SLIDING:
            ret = StateChangeReason::STATE_CHANGE_REASON_AOD_SLIDING;
            break;
        case WakeupDeviceType::WAKEUP_DEVICE_INCOMING_CALL:
            ret = StateChangeReason::STATE_CHANGE_REASON_INCOMING_CALL;
            break;
        case WakeupDeviceType::WAKEUP_DEVICE_SHELL:
            ret = StateChangeReason::STATE_CHANGE_REASON_SHELL;
            break;
        case WakeupDeviceType::WAKEUP_DEVICE_PICKUP:
            ret = StateChangeReason::STATE_CHANGE_REASON_PICKUP;
            break;
        case WakeupDeviceType::WAKEUP_DEVICE_EXIT_SYSTEM_STR:
            ret = StateChangeReason::STATE_CHANGE_REASON_EXIT_SYSTEM_STR;
            break;
        case WakeupDeviceType::WAKEUP_DEVICE_SCREEN_CONNECT:
            ret = StateChangeReason::STATE_CHANGE_REASON_SCREEN_CONNECT;
            break;
        case WakeupDeviceType::WAKEUP_DEVICE_TP_TOUCH:
            ret = StateChangeReason::STATE_CHANGE_REASON_TP_TOUCH;
            break;
        case WakeupDeviceType::WAKEUP_DEVICE_EX_SCREEN_INIT:
            ret = StateChangeReason::STATE_CHANGE_REASON_EX_SCREEN_INIT;
            break;
        case WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN: // fall through
        default:
            break;
    }
    POWER_HILOGD(FEATURE_WAKEUP, "StateChangeReason: %{public}u", ret);
    return ret;
}

StateChangeReason PowerStateMachine::GetReasonBySuspendType(SuspendDeviceType type)
{
    POWER_HILOGD(FEATURE_SUSPEND, "SuspendDeviceType: %{public}u", type);
    StateChangeReason ret = StateChangeReason::STATE_CHANGE_REASON_UNKNOWN;
    switch (type) {
        case SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION:
            ret = StateChangeReason::STATE_CHANGE_REASON_APPLICATION;
            break;
        case SuspendDeviceType::SUSPEND_DEVICE_REASON_DEVICE_ADMIN:
            ret = StateChangeReason::STATE_CHANGE_REASON_REMOTE;
            break;
        case SuspendDeviceType::SUSPEND_DEVICE_REASON_TIMEOUT:
            ret = (enabledTimingOutLockScreen_.load() &&
                      (!enabledTimingOutLockScreenCheckLock_.load() ||
                          CheckRunningLock(PowerState::INACTIVE))) ?
                StateChangeReason::STATE_CHANGE_REASON_TIMEOUT :
                StateChangeReason::STATE_CHANGE_REASON_TIMEOUT_NO_SCREEN_LOCK;
            break;
        case SuspendDeviceType::SUSPEND_DEVICE_REASON_LID:
            ret = StateChangeReason::STATE_CHANGE_REASON_LID;
            break;
        case SuspendDeviceType::SUSPEND_DEVICE_REASON_SWITCH:
            ret = StateChangeReason::STATE_CHANGE_REASON_SWITCH;
            break;
        case SuspendDeviceType::SUSPEND_DEVICE_REASON_POWER_KEY: // fall through
        case SuspendDeviceType::SUSPEND_DEVICE_REASON_SLEEP_KEY:
            ret = StateChangeReason::STATE_CHANGE_REASON_HARD_KEY;
            break;
        case SuspendDeviceType::SUSPEND_DEVICE_REASON_HDMI:
            ret = StateChangeReason::STATE_CHANGE_REASON_CABLE;
            break;
        case SuspendDeviceType::SUSPEND_DEVICE_REASON_ACCESSIBILITY:
            ret = StateChangeReason::STATE_CHANGE_REASON_ACCESSIBILITY;
            break;
        case SuspendDeviceType::SUSPEND_DEVICE_REASON_FORCE_SUSPEND:
            ret = StateChangeReason::STATE_CHANGE_REASON_SYSTEM;
            break;
        case SuspendDeviceType::SUSPEND_DEVICE_REASON_TP_COVER:
            ret = StateChangeReason::STATE_CHANGE_REASON_TP_COVER;
            break;
        case SuspendDeviceType::SUSPEND_DEVICE_REASON_EX_SCREEN_INIT:
            ret = StateChangeReason::STATE_CHANGE_REASON_EX_SCREEN_INIT;
            break;
        default:
            break;
    }
    POWER_HILOGD(FEATURE_SUSPEND, "StateChangeReason: %{public}u", ret);
    return ret;
}

void PowerStateMachine::AppendDumpInfo(std::string& result, std::string& reason, std::string& time)
{
    result.append("POWER STATE DUMP:\n");
    result.append("Current State: ")
        .append(PowerUtils::GetPowerStateString(GetState()))
        .append("  Reason: ")
        .append(reason)
        .append("  Time: ")
        .append(time)
        .append("\n");

    result.append("ScreenOffTime: Timeout=");
    if (isScreenOffTimeOverride_) {
        result.append((ToString(g_beforeOverrideTime)))
            .append("ms  OverrideTimeout=")
            .append((ToString(GetDisplayOffTime())))
            .append("ms\n");
    } else {
        result.append((ToString(GetDisplayOffTime()))).append("ms\n");
    }

    result.append("DUMP DETAILS:\n");
    result.append("Last Screen On: ").append(ToString(mDeviceState_.screenState.lastOnTime)).append("\n");
    result.append("Last Screen Off: ").append(ToString(mDeviceState_.screenState.lastOffTime)).append("\n");
    result.append("Last SuspendDevice: ").append(ToString(mDeviceState_.lastSuspendDeviceTime)).append("\n");
    result.append("Last WakeupDevice: ").append(ToString(mDeviceState_.lastWakeupDeviceTime)).append("\n");
    result.append("Last Refresh: ").append(ToString(mDeviceState_.lastRefreshActivityTime)).append("\n");

    result.append("DUMP EACH STATES:\n");
    for (auto it = controllerMap_.begin(); it != controllerMap_.end(); it++) {
        result.append("State: ")
            .append(PowerUtils::GetPowerStateString(it->second->GetState()))
            .append("   Reason: ")
            .append(PowerUtils::GetReasonTypeString(it->second->lastReason_).c_str())
            .append("   Time: ")
            .append(ToString(it->second->lastTime_))
            .append("\n")
            .append("   Failure: ")
            .append(PowerUtils::GetReasonTypeString(it->second->failTrigger_).c_str())
            .append("   Reason: ")
            .append(it->second->failReason_)
            .append("   From: ")
            .append(PowerUtils::GetPowerStateString(it->second->failFrom_))
            .append("   Time: ")
            .append(ToString(it->second->failTime_))
            .append("\n\n");
    }
}

void PowerStateMachine::DumpInfo(std::string& result)
{
    std::string reason = "UNKNOWN";
    std::string time = "UNKNOWN";
    auto it = controllerMap_.find(GetState());
    if (it != controllerMap_.end() && it->second != nullptr) {
        reason = ToString(static_cast<uint32_t>(it->second->lastReason_));
        time = ToString(it->second->lastTime_);
    }
    AppendDumpInfo(result, reason, time);
}

bool PowerStateMachine::StateController::NeedNotify(PowerState currentState)
{
#ifdef POWER_MANAGER_ENABLE_FORCE_SLEEP_BROADCAST
    // need notify when SLEEP->SLEEP if force sleeping flag is set
    if (currentState == PowerState::SLEEP && GetState() == PowerState::SLEEP) {
        auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
        auto suspendController = (pms != nullptr ? pms->GetSuspendController() : nullptr);

        if (suspendController == nullptr) {
            POWER_HILOGE(FEATURE_POWER_STATE, "suspendController is nullptr, can't get force sleeping flag");
        } else if (suspendController->GetForceSleepingFlag()) {
            return true;
        }
    }
#endif

    if (currentState == GetState()) {
        return false;
    }
    if (currentState == PowerState::DIM && GetState() == PowerState::AWAKE) {
        return false;
    }
    return true;
}

TransitResult PowerStateMachine::StateController::TransitTo(StateChangeReason reason, bool ignoreLock)
{
    POWER_HILOGD(FEATURE_POWER_STATE, "Start");
    std::shared_ptr<PowerStateMachine> owner = owner_.lock();
    if (owner == nullptr) {
        POWER_HILOGW(FEATURE_POWER_STATE, "owner is nullptr");
        return TransitResult::OTHER_ERR;
    }
    POWER_HILOGI(FEATURE_POWER_STATE,
        "[UL_POWER] Transit from %{public}s to %{public}s for %{public}s ignoreLock=%{public}d",
        PowerUtils::GetPowerStateString(owner->currentState_).c_str(),
        PowerUtils::GetPowerStateString(this->state_).c_str(),
        PowerUtils::GetReasonTypeString(reason).c_str(), ignoreLock);
    MatchState(owner->currentState_, owner->stateAction_->GetDisplayState());
    if (!CheckState()) {
        POWER_HILOGD(FEATURE_POWER_STATE, "Already in state: %{public}d", owner->currentState_);
        RecordFailure(owner->currentState_, reason, TransitResult::ALREADY_IN_STATE);
        return TransitResult::ALREADY_IN_STATE;
    }

    if (reason != StateChangeReason::STATE_CHANGE_REASON_INIT &&
        !owner->CanTransitTo(owner->currentState_, state_, reason)) {
        POWER_HILOGD(FEATURE_POWER_STATE, "Block Transit from %{public}s to %{public}s",
            PowerUtils::GetPowerStateString(owner->currentState_).c_str(),
            PowerUtils::GetPowerStateString(state_).c_str());
        RecordFailure(owner->currentState_, reason, TransitResult::FORBID_TRANSIT);
        return TransitResult::FORBID_TRANSIT;
    }

    if (!ignoreLock && !owner->CheckRunningLock(GetState())) {
        POWER_HILOGD(FEATURE_POWER_STATE, "Running lock block");
        RecordFailure(owner->currentState_, reason, TransitResult::LOCKING);
        return TransitResult::LOCKING;
    }

    if (!owner->HandlePreBrightState(state_, reason)) {
        return TransitResult::PRE_BRIGHT_ERR;
    }

    TransitResult ret = action_(reason);
    if (ret == TransitResult::SUCCESS) {
        bool needNotify = NeedNotify(owner->currentState_);
        lastReason_ = reason;
        lastTime_ = GetTickCount();
        owner->currentState_ = GetState();
        if (needNotify) {
            owner->NotifyPowerStateChanged(owner->currentState_, reason);
        }
        owner->proximityNormalController_.ActivateValidProximitySensor(owner->currentState_);
    } else if (IsReallyFailed(reason)) {
        RecordFailure(owner->currentState_, reason, ret);
    }

    POWER_HILOGD(FEATURE_POWER_STATE, "Finish, result: %{public}d", ret);
    return ret;
}

bool PowerStateMachine::StateController::CheckState()
{
    std::shared_ptr<PowerStateMachine> owner = owner_.lock();
    if (owner == nullptr) {
        POWER_HILOGW(FEATURE_POWER_STATE, "Owner is nullptr");
        return false;
    }
    auto state = GetState();
    if (state == PowerState::DIM ||
#ifdef POWER_MANAGER_ENABLE_FORCE_SLEEP_BROADCAST
        state == PowerState::SLEEP ||
#endif
        state == PowerState::AWAKE) {
        return true;
    }
    POWER_HILOGD(FEATURE_POWER_STATE, "state: %{public}u, currentState_: %{public}u", state, owner->currentState_);
    return state != owner->currentState_;
}

void PowerStateMachine::StateController::CorrectState(
    PowerState& currentState, PowerState correctState, DisplayState state)
{
    std::string msg = "[UL_POWER] Correct power state errors from ";
    msg.append(PowerUtils::GetPowerStateString(currentState))
        .append(" to ")
        .append(PowerUtils::GetPowerStateString(correctState))
        .append(" due to current display state is ")
        .append(PowerUtils::GetDisplayStateString(state));
    POWER_HILOGW(FEATURE_POWER_STATE, "%{public}s", msg.c_str());
#ifdef HAS_HIVIEWDFX_HISYSEVENT_PART
    HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::POWER, "STATE_CORRECTION", HiviewDFX::HiSysEvent::EventType::FAULT,
        "ERROR_STATE", static_cast<uint32_t>(currentState), "CORRECTION_STATE", static_cast<uint32_t>(correctState),
        "DISPLAY_STATE", static_cast<uint32_t>(state), "MSG", msg);
#endif
    currentState = correctState;
}

void PowerStateMachine::StateController::MatchState(PowerState& currentState, DisplayState state)
{
    if (GetState() == PowerState::SLEEP || currentState == PowerState::SLEEP || GetState() == PowerState::HIBERNATE ||
        currentState == PowerState::HIBERNATE || GetState() == PowerState::SHUTDOWN ||
        currentState == PowerState::SHUTDOWN) {
        return;
    }

    // Keep the state of display consistent with the state of power
    switch (state) {
        case DisplayState::DISPLAY_OFF:
            if (currentState == PowerState::AWAKE || currentState == PowerState::FREEZE) {
                CorrectState(currentState, PowerState::INACTIVE, state);
            }
            break;
        case DisplayState::DISPLAY_DIM:
            if (currentState == PowerState::INACTIVE || currentState == PowerState::STAND_BY ||
                currentState == PowerState::DOZE) {
                CorrectState(currentState, PowerState::DIM, state);
            }
            break;
        case DisplayState::DISPLAY_ON:
            if (currentState == PowerState::INACTIVE || currentState == PowerState::STAND_BY ||
                currentState == PowerState::DOZE) {
                CorrectState(currentState, PowerState::AWAKE, state);
            }
            break;
        case DisplayState::DISPLAY_SUSPEND:
        case DisplayState::DISPLAY_UNKNOWN:
        default:
            break;
    }
}

void PowerStateMachine::Reset()
{
    ffrtTimer_.reset();
}

std::string PowerStateMachine::GetTransitResultString(TransitResult result)
{
    switch (result) {
        case TransitResult::ALREADY_IN_STATE:
            return "Already in the state";
        case TransitResult::LOCKING:
            return "Blocked by running lock";
        case TransitResult::HDI_ERR:
            return "Power HDI error";
        case TransitResult::DISPLAY_ON_ERR:
            return "SetDisplayState(ON) error";
        case TransitResult::DISPLAY_OFF_ERR:
            return "SetDisplayState(OFF) error";
        case TransitResult::FORBID_TRANSIT:
            return "Forbid transit";
        case TransitResult::OTHER_ERR:
            return "Other error";
        default:
            break;
    }
    return "Unknown error";
}

void PowerStateMachine::StateController::RecordFailure(
    PowerState from, StateChangeReason trigger, TransitResult failReason)
{
    failFrom_ = from;
    failTrigger_ = trigger;
    failTime_ = GetTickCount();
    failReason_ = GetTransitResultString(failReason);
    std::string message = "State Transit Failed from ";
    message.append(PowerUtils::GetPowerStateString(failFrom_))
        .append(" to ")
        .append(PowerUtils::GetPowerStateString(GetState()))
        .append(" by ")
        .append(PowerUtils::GetReasonTypeString(failTrigger_).c_str())
        .append("   Reason:")
        .append(failReason_)
        .append("   Time:")
        .append(ToString(failTime_))
        .append("\n");
#ifdef HAS_HIVIEWDFX_HISYSEVENT_PART
    const int logLevel = 2;
    const std::string tag = "TAG_POWER";
    HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::POWER, "SCREEN", HiviewDFX::HiSysEvent::EventType::FAULT,
        "LOG_LEVEL", logLevel, "TAG", tag, "MESSAGE", message);
    POWER_HILOGI(FEATURE_POWER_STATE, "RecordFailure: %{public}s", message.c_str());
#endif
}

bool PowerStateMachine::StateController::IsReallyFailed(StateChangeReason reason)
{
    if (reason == StateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT ||
        reason == StateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_FAIL_SCREEN_OFF) {
        return false;
    }
    return true;
}
} // namespace PowerMgr
} // namespace OHOS
