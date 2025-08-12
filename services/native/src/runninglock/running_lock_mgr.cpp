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

#include "running_lock_mgr.h"

#include <cinttypes>
#include <datetime_ex.h>
#ifdef HAS_HIVIEWDFX_HISYSEVENT_PART
#include <hisysevent.h>
#endif
#include <ipc_skeleton.h>
#include <securec.h>
#ifdef HAS_HIVIEWDFX_HITRACE_PART
#include "hitrace_meter.h"
#endif
#include "ffrt_utils.h"
#include "power_log.h"
#include "power_mgr_factory.h"
#include "power_mgr_service.h"
#include "power_utils.h"
#include "system_suspend_controller.h"
#include "power_hookmgr.h"

using namespace std;

namespace OHOS {
namespace PowerMgr {
namespace {
const string TASK_RUNNINGLOCK_FORCEUNLOCK = "RunningLock_ForceUnLock";
constexpr int32_t VALID_PID_LIMIT = 1;
sptr<IPowerRunninglockCallback> g_runningLockCallback = nullptr;
const string INCALL_APP_BUNDLE_NAME = "com.ohos.callui";
#ifdef HAS_SENSORS_SENSOR_PART
constexpr uint32_t FOREGROUND_INCALL_DELAY_TIME_MS = 300;
constexpr uint32_t BACKGROUND_INCALL_DELAY_TIME_MS = 800;
#endif
}

RunningLockMgr::~RunningLockMgr() {}

bool RunningLockMgr::Init()
{
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "Init start");
    if (runningLockDeathRecipient_ == nullptr) {
        runningLockDeathRecipient_ = new RunningLockDeathRecipient();
    }
    if (runningLockAction_ == nullptr) {
        runningLockAction_ = PowerMgrFactory::GetRunningLockAction();
        if (!runningLockAction_) {
            POWER_HILOGE(FEATURE_RUNNING_LOCK, "Get running lock action fail");
            return false;
        }
    }
    if (runninglockProxy_ == nullptr) {
        runninglockProxy_ = std::make_shared<RunningLockProxy>();
    }
    bool ret = InitLocks();
    POWER_HILOGI(FEATURE_RUNNING_LOCK, "Init success");
    return ret;
}

bool RunningLockMgr::InitLocks()
{
    InitLocksTypeScreen();
    InitLocksTypeBackground();
#ifdef HAS_SENSORS_SENSOR_PART
    InitLocksTypeProximity();
#endif
    InitLocksTypeCoordination();
    return true;
}

void RunningLockMgr::InitLocksTypeScreen()
{
    lockCounters_.emplace(RunningLockType::RUNNINGLOCK_SCREEN,
        std::make_shared<LockCounter>(RunningLockType::RUNNINGLOCK_SCREEN,
            [this](bool active, [[maybe_unused]] RunningLockParam runningLockParam) -> int32_t {
            POWER_HILOGD(FEATURE_RUNNING_LOCK, "RUNNINGLOCK_SCREEN action start");
            auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
            if (pms == nullptr) {
                return RUNNINGLOCK_FAILURE;
            }
            auto stateMachine = pms->GetPowerStateMachine();
            if (stateMachine == nullptr) {
                return RUNNINGLOCK_FAILURE;
            }
            if (active) {
                // RUNNINGLOCK_SCREEN active
                POWER_HILOGI(FEATURE_RUNNING_LOCK, "SL active");
                pms->RefreshActivityInner(GetTickCount(), UserActivityType::USER_ACTIVITY_TYPE_SOFTWARE, true);
            } else {
                // RUNNINGLOCK_SCREEN inactive
                POWER_HILOGI(FEATURE_RUNNING_LOCK, "SL inactive");
                if (stateMachine->GetState() == PowerState::AWAKE) {
                    stateMachine->ResetInactiveTimer();
                } else {
                    POWER_HILOGD(FEATURE_RUNNING_LOCK, "Screen unlock in state: %{public}d",
                        stateMachine->GetState());
                }
            }
            return RUNNINGLOCK_SUCCESS;
        })
    );
}

void RunningLockMgr::InitLocksTypeBackground()
{
    std::function<int32_t(bool, RunningLockParam)> activate =
        [this](bool active, RunningLockParam lockInnerParam) -> int32_t {
        if (active) {
            return runningLockAction_->Lock(lockInnerParam);
        } else {
            return runningLockAction_->Unlock(lockInnerParam);
        }
    };
    lockCounters_.emplace(RunningLockType::RUNNINGLOCK_BACKGROUND_PHONE,
        std::make_shared<LockCounter>(RunningLockType::RUNNINGLOCK_BACKGROUND_PHONE, activate));
    lockCounters_.emplace(RunningLockType::RUNNINGLOCK_BACKGROUND_NOTIFICATION,
        std::make_shared<LockCounter>(RunningLockType::RUNNINGLOCK_BACKGROUND_NOTIFICATION, activate));
    lockCounters_.emplace(RunningLockType::RUNNINGLOCK_BACKGROUND_AUDIO,
        std::make_shared<LockCounter>(RunningLockType::RUNNINGLOCK_BACKGROUND_AUDIO, activate));
    lockCounters_.emplace(RunningLockType::RUNNINGLOCK_BACKGROUND_SPORT,
        std::make_shared<LockCounter>(RunningLockType::RUNNINGLOCK_BACKGROUND_SPORT, activate));
    lockCounters_.emplace(RunningLockType::RUNNINGLOCK_BACKGROUND_NAVIGATION,
        std::make_shared<LockCounter>(RunningLockType::RUNNINGLOCK_BACKGROUND_NAVIGATION, activate));
    lockCounters_.emplace(RunningLockType::RUNNINGLOCK_BACKGROUND_TASK,
        std::make_shared<LockCounter>(RunningLockType::RUNNINGLOCK_BACKGROUND_TASK, activate));
}

#ifdef HAS_SENSORS_SENSOR_PART
bool RunningLockMgr::InitProximityController()
{
    if (proximityController_ != nullptr) {
        return true;
    }
#ifdef POWER_MANAGER_INIT_PROXIMITY_CONTROLLER
    auto action = [](uint32_t status) {
        auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
        if (pms == nullptr) {
            POWER_HILOGE(FEATURE_RUNNING_LOCK, "Power service is nullptr");
            return;
        }
        auto runningLock = pms->GetRunningLockMgr();
        if (runningLock == nullptr) {
            POWER_HILOGE(FEATURE_RUNNING_LOCK, "runningLock is nullptr");
            return;
        }
        if (status == IProximityController::PROXIMITY_CLOSE) {
            runningLock->HandleProximityCloseEvent();
        } else if (status == IProximityController::PROXIMITY_AWAY) {
            runningLock->HandleProximityAwayEvent();
        }
    };
    HOOK_MGR* hookMgr = GetPowerHookMgr();
    ProximityControllerContext context = {.action = action};
    HookMgrExecute(
        hookMgr, static_cast<int32_t>(PowerHookStage::POWER_PROXIMITY_CONTROLLER_INIT), &context, nullptr);
    if (context.controllerPtr != nullptr) {
        proximityController_ = context.controllerPtr;
        POWER_HILOGI(FEATURE_RUNNING_LOCK, "InitProximityController with hook");
    } else {
        proximityController_ = std::make_shared<ProximityController>();
        POWER_HILOGI(FEATURE_RUNNING_LOCK, "InitProximityController without hook");
    }
#else
    proximityController_ = std::make_shared<ProximityController>();
    POWER_HILOGI(FEATURE_RUNNING_LOCK, "InitProximityController Done");
#endif
    return true;
}

void RunningLockMgr::InitLocksTypeProximity()
{
    InitProximityController();
    lockCounters_.emplace(RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL,
        std::make_shared<LockCounter>(RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL,
            [this](bool active, [[maybe_unused]] RunningLockParam runningLockParam) -> int32_t {
            POWER_HILOGD(FEATURE_RUNNING_LOCK, "RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL action start");
            FFRTTask task = [this] {
                auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
                if (pms == nullptr) {
                    return;
                }
                auto stateMachine = pms->GetPowerStateMachine();
                if (stateMachine == nullptr) {
                    return;
                }
                PreprocessBeforeAwake();
                stateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_RUNNING_LOCK);
            };
            if (active) {
                POWER_HILOGI(FEATURE_RUNNING_LOCK, "[UL_POWER] RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL active");
                proximityController_->Enable();
            } else {
                POWER_HILOGI(FEATURE_RUNNING_LOCK, "[UL_POWER] RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL inactive");
                auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
                if (pms == nullptr) {
                    return RUNNINGLOCK_FAILURE;
                }
                auto stateMachine = pms->GetPowerStateMachine();
                if (stateMachine == nullptr) {
                    return RUNNINGLOCK_FAILURE;
                }
                stateMachine->CancelDelayTimer(PowerStateMachine::CHECK_PROXIMITY_SCREEN_SWITCH_TO_SUB_MSG);
                FFRTUtils::SubmitTask(task);
                proximityController_->Disable();
                proximityController_->Clear();
            }
            return RUNNINGLOCK_SUCCESS;
        })
    );
}
#endif

void RunningLockMgr::AsyncWakeup()
{
    FFRTTask asyncWakeup = []() {
        auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
        if (pms == nullptr) {
            POWER_HILOGI(FEATURE_RUNNING_LOCK, "Coordination unlock wakeup device, pms is nullptr");
            return;
        }
        pms->WakeupDevice(GetTickCount(), WakeupDeviceType::WAKEUP_DEVICE_APPLICATION, "EndCollaboration");
    };
    FFRTUtils::SubmitTask(asyncWakeup);
}

void RunningLockMgr::InitLocksTypeCoordination()
{
    lockCounters_.emplace(RunningLockType::RUNNINGLOCK_COORDINATION,
        std::make_shared<LockCounter>(RunningLockType::RUNNINGLOCK_COORDINATION,
            [this](bool active, [[maybe_unused]] RunningLockParam runningLockParam) -> int32_t {
            auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
            if (pms == nullptr) {
                return RUNNINGLOCK_FAILURE;
            }
            auto stateMachine = pms->GetPowerStateMachine();
            if (stateMachine == nullptr) {
                return RUNNINGLOCK_FAILURE;
            }
            auto stateAction = stateMachine->GetStateAction();
            if (stateAction == nullptr) {
                return RUNNINGLOCK_FAILURE;
            }
            POWER_HILOGI(FEATURE_RUNNING_LOCK, "Coordination runninglock action, active=%{public}d, state=%{public}d",
                active, stateMachine->GetState());
            struct RunningLockParam backgroundLockParam = runningLockParam;
            backgroundLockParam.name = PowerUtils::GetRunningLockTypeString(RunningLockType::RUNNINGLOCK_COORDINATION),
            backgroundLockParam.type = RunningLockType::RUNNINGLOCK_BACKGROUND_TASK;
            auto iterator = lockCounters_.find(backgroundLockParam.type);
            if (iterator == lockCounters_.end()) {
                POWER_HILOGE(FEATURE_RUNNING_LOCK, "unsupported type, type=%{public}d", backgroundLockParam.type);
                return RUNNINGLOCK_NOT_SUPPORT;
            }
            std::shared_ptr<LockCounter> counter = iterator->second;
            int32_t result = RUNNINGLOCK_SUCCESS;
            if (active) {
                result = counter->Increase(backgroundLockParam);
                stateAction->SetCoordinated(true);
            } else {
                stateAction->SetCoordinated(false);
                result = counter->Decrease(backgroundLockParam);
            }
            return result;
        })
    );
}

std::shared_ptr<RunningLockInner> RunningLockMgr::GetRunningLockInner(
    const sptr<IRemoteObject>& remoteObj)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto iterator = runningLocks_.find(remoteObj);
    if (iterator != runningLocks_.end()) {
        return iterator->second;
    }
    return nullptr;
}

std::shared_ptr<RunningLockInner> RunningLockMgr::GetRunningLockInnerByName(
    const std::string& name)
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::shared_ptr<RunningLockInner> lockInner = nullptr;
    std::string result = ToString(runningLocks_.size());
    for (auto& iter : runningLocks_) {
        if (iter.second == nullptr) {
            POWER_HILOGE(FEATURE_RUNNING_LOCK, "GetRunningLockInnerByName nullptr");
            continue;
        }
        if (iter.second->GetName() == name) {
            lockInner = iter.second;
        }
        auto& lockParam = iter.second->GetParam();
        result.append(" name=").append(lockParam.name);
    }
    POWER_HILOGI(FEATURE_RUNNING_LOCK, "DumpInfo: %{public}s", result.c_str());
    return lockInner;
}

std::shared_ptr<RunningLockInner> RunningLockMgr::CreateRunningLock(const sptr<IRemoteObject>& remoteObj,
    const RunningLockParam& runningLockParam)
{
    auto lockInner = RunningLockInner::CreateRunningLockInner(runningLockParam);
    if (lockInner == nullptr) {
        return nullptr;
    }
    POWER_HILOGI(FEATURE_RUNNING_LOCK, "CrtN:%{public}s,T:%{public}d",
        lockInner->GetName().c_str(), lockInner->GetType());
    {
        std::lock_guard<std::mutex> lock(mutex_);
        runningLocks_.emplace(remoteObj, lockInner);
    }
    if (lockInner->GetType() != RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL) {
        runninglockProxy_->AddRunningLock(lockInner->GetPid(), lockInner->GetUid(), remoteObj);
    }
    remoteObj->AddDeathRecipient(runningLockDeathRecipient_);
    return lockInner;
}

bool RunningLockMgr::ReleaseLock(const sptr<IRemoteObject> remoteObj, const std::string& name)
{
    bool result = false;
    auto lockInner = GetRunningLockInner(remoteObj);
    if (lockInner == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "RInner=null");
        lockInner = GetRunningLockInnerByName(name);
        if (lockInner == nullptr) {
            POWER_HILOGE(FEATURE_RUNNING_LOCK, "%{public}s:LockInner not existed", __func__);
            return result;
        }
    }
    // ReleaseLock name type bundleName
    POWER_HILOGI(FEATURE_RUNNING_LOCK, "RlsN:%{public}s,T:%{public}d,B:%{public}s",
        lockInner->GetName().c_str(), lockInner->GetType(), lockInner->GetBundleName().c_str());
    UnLock(remoteObj);
    {
        std::lock_guard<std::mutex> lock(mutex_);
        runningLocks_.erase(remoteObj);
    }
    if (lockInner->GetType() != RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL) {
        runninglockProxy_->RemoveRunningLock(lockInner->GetPid(), lockInner->GetUid(), remoteObj);
    }
    result = remoteObj->RemoveDeathRecipient(runningLockDeathRecipient_);
    return result;
}

bool RunningLockMgr::IsSceneRunningLockType(RunningLockType type)
{
    return type == RunningLockType::RUNNINGLOCK_BACKGROUND_PHONE ||
        type == RunningLockType::RUNNINGLOCK_BACKGROUND_NOTIFICATION ||
        type == RunningLockType::RUNNINGLOCK_BACKGROUND_AUDIO ||
        type == RunningLockType::RUNNINGLOCK_BACKGROUND_SPORT ||
        type == RunningLockType::RUNNINGLOCK_BACKGROUND_NAVIGATION ||
        type == RunningLockType::RUNNINGLOCK_BACKGROUND_TASK;
}

bool RunningLockMgr::NeedNotify(RunningLockType type)
{
    return IsSceneRunningLockType(type) ||
        type == RunningLockType::RUNNINGLOCK_SCREEN ||
        type == RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL;
}

void RunningLockMgr::UpdateUnSceneLockLists(RunningLockParam& singleLockParam, bool fill)
{
    std::lock_guard<std::mutex> lock(screenLockListsMutex_);
    auto iterator = unSceneLockLists_.find(std::to_string(singleLockParam.lockid));
    if (fill) {
        if (iterator == unSceneLockLists_.end()) {
            POWER_HILOGD(FEATURE_RUNNING_LOCK, "Add non-scene lock information from lists");
            RunningLockInfo unSceneAppLockInfo = FillAppRunningLockInfo(singleLockParam);
            unSceneLockLists_.insert(
                std::pair<std::string, RunningLockInfo>(std::to_string(singleLockParam.lockid), unSceneAppLockInfo));
        }
        return;
    }
    if (iterator != unSceneLockLists_.end()) {
        POWER_HILOGD(FEATURE_RUNNING_LOCK, "Remove non-scene lock information from lists");
        unSceneLockLists_.erase(iterator);
    }
    return;
}

RunningLockInfo RunningLockMgr::FillAppRunningLockInfo(const RunningLockParam& info)
{
    RunningLockInfo tempAppRunningLockInfo = {};
    tempAppRunningLockInfo.name = info.name;
    tempAppRunningLockInfo.bundleName = info.bundleName;
    tempAppRunningLockInfo.type = info.type;
    tempAppRunningLockInfo.pid = info.pid;
    tempAppRunningLockInfo.uid = info.uid;
    return tempAppRunningLockInfo;
}

bool RunningLockMgr::IsValidType(RunningLockType type)
{
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        return true;
    }
    PowerState state = pms->GetState();
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "state=%{public}d, type=%{public}d", state, type);
    switch (state) {
        case PowerState::AWAKE:
        case PowerState::FREEZE:
        case PowerState::INACTIVE:
        case PowerState::STAND_BY:
        case PowerState::DOZE:
            return true;
        case PowerState::SLEEP:
        case PowerState::HIBERNATE:
            return type != RunningLockType::RUNNINGLOCK_COORDINATION;
        default:
            break;
    }
    return true;
}

void RunningLockMgr::PreprocessBeforeAwake()
{
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        return;
    }
    auto stateMachine = pms->GetPowerStateMachine();
    auto suspendController = pms->GetSuspendController();
    if (stateMachine == nullptr || suspendController == nullptr) {
        return;
    }

    suspendController->StopSleep();
    POWER_HILOGI(FEATURE_RUNNING_LOCK, "wake up.");
    SystemSuspendController::GetInstance().Wakeup();
    if (stateMachine->GetState() == PowerState::SLEEP) {
        POWER_HILOGI(FEATURE_RUNNING_LOCK, "TriggerSyncSleepCallback start.");
        suspendController->TriggerSyncSleepCallback(true);
    }
}

bool RunningLockMgr::UpdateWorkSource(const sptr<IRemoteObject>& remoteObj,
    const std::map<int32_t, std::string>& workSources)
{
    auto lockInner = GetRunningLockInner(remoteObj);
    if (lockInner == nullptr) {
        POWER_HILOGD(FEATURE_RUNNING_LOCK, "%{public}s:LockInner is nullptr", __func__);
        return false;
    }
    RunningLockParam lockInnerParam = lockInner->GetParam();
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "try UpdateWorkSource, name: %{public}s, type: %{public}d",
        lockInnerParam.name.c_str(), lockInnerParam.type);
    runninglockProxy_->UpdateWorkSource(lockInner->GetPid(), lockInner->GetUid(), remoteObj, workSources);
    return true;
}

bool RunningLockMgr::Lock(const sptr<IRemoteObject>& remoteObj)
{
#ifdef HAS_HIVIEWDFX_HITRACE_PART
    HitraceScopedEx powerHitrace(HITRACE_LEVEL_COMMERCIAL, HITRACE_TAG_POWER, "RunningLock_Lock");
#endif
    auto lockInner = GetRunningLockInner(remoteObj);
    if (lockInner == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "LInner=null");
        return false;
    }
    RunningLockParam lockInnerParam = lockInner->GetParam();
    // try lock
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "LockN:%{public}s,T:%{public}d",
        lockInnerParam.name.c_str(), lockInnerParam.type);
    if (lockInner->IsProxied()) {
        // Runninglock is proxied
        POWER_HILOGW(FEATURE_RUNNING_LOCK,
            "try lock proxied fail,N:%{public}s,T:%{public}d", lockInnerParam.name.c_str(), lockInnerParam.type);
        return false;
    }
    if (!IsValidType(lockInnerParam.type)) {
        // Type is invalid
        POWER_HILOGE(FEATURE_RUNNING_LOCK,
            "try lock type fail,N:%{public}s,T:%{public}d", lockInnerParam.name.c_str(), lockInnerParam.type);
        return false;
    }
    if (lockInner->GetState() == RunningLockState::RUNNINGLOCK_STATE_ENABLE) {
        // Lock is already enabled
        POWER_HILOGI(FEATURE_RUNNING_LOCK, "Locked N=%{public}s", lockInner->GetName().c_str());
        return false;
    }
    if (lockInnerParam.type == RunningLockType::RUNNINGLOCK_SCREEN) {
        UpdateUnSceneLockLists(lockInnerParam, true);
    }
    auto iterator = lockCounters_.find(lockInnerParam.type);
    if (iterator == lockCounters_.end()) {
        // Lock failed unsupported
        POWER_HILOGE(FEATURE_RUNNING_LOCK,
            "try lock fail,N:%{public}s,T:%{public}d", lockInnerParam.name.c_str(), lockInnerParam.type);
        return false;
    }
    std::shared_ptr<LockCounter> counter = iterator->second;
    if (counter->Increase(lockInnerParam) != RUNNINGLOCK_SUCCESS) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "try lock increase fail,N:%{public}s,T:%{public}d,C:%{public}d",
            lockInnerParam.name.c_str(), counter->GetType(), counter->GetCount());
        return false;
    }
#ifdef HAS_HIVIEWDFX_HISYSEVENT_PART
    lockInner->SetBeginTime(GetTickCount());
#endif
    lockInner->SetState(RunningLockState::RUNNINGLOCK_STATE_ENABLE);
    return true;
}

bool RunningLockMgr::UnLock(const sptr<IRemoteObject> remoteObj, const std::string& name)
{
#ifdef HAS_HIVIEWDFX_HITRACE_PART
    HitraceScopedEx powerHitrace(HITRACE_LEVEL_COMMERCIAL, HITRACE_TAG_POWER, "RunningLock_Unlock");
#endif
    auto lockInner = GetRunningLockInner(remoteObj);
    if (lockInner == nullptr) {
        POWER_HILOGD(FEATURE_RUNNING_LOCK, "UnInner=null");
        lockInner = GetRunningLockInnerByName(name);
        if (lockInner == nullptr) {
            POWER_HILOGD(FEATURE_RUNNING_LOCK, "%{public}s:LockInner not existed", __func__);
            return false;
        }
    }
    if (lockInner->IsProxied()) {
        POWER_HILOGW(FEATURE_RUNNING_LOCK, "Runninglock is proxied, unProxy");
        runninglockProxy_->UpdateProxyState(lockInner->GetPid(), lockInner->GetUid(), remoteObj, false);
    }
    auto lockInnerParam = lockInner->GetParam();
    // try unlock
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "UnLockN:%{public}s,T:%{public}d",
        lockInnerParam.name.c_str(), lockInnerParam.type);
    if (lockInner->GetState() == RunningLockState::RUNNINGLOCK_STATE_DISABLE) {
        // Lock is already disabled
        POWER_HILOGI(FEATURE_RUNNING_LOCK, "UnLocked N=%{public}s", lockInner->GetName().c_str());
        return false;
    }
    if (lockInnerParam.type == RunningLockType::RUNNINGLOCK_SCREEN) {
        UpdateUnSceneLockLists(lockInnerParam, false);
    }
    auto iterator = lockCounters_.find(lockInnerParam.type);
    if (iterator == lockCounters_.end()) {
        // Unlock failed unsupported type
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "try unlock fail,N:%{public}s,T:%{public}d",
            lockInnerParam.name.c_str(), lockInnerParam.type);
        return false;
    }
    std::shared_ptr<LockCounter> counter = iterator->second;
    if (counter->Decrease(lockInnerParam)) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "try unlock decrease fail,N:%{public}s,T:%{public}d,C:%{public}d",
            lockInnerParam.name.c_str(), counter->GetType(), counter->GetCount());
        return false;
    }
    WriteHiSysEvent(lockInner);
    lockInner->SetState(RunningLockState::RUNNINGLOCK_STATE_DISABLE);
    return true;
}

void RunningLockMgr::WriteHiSysEvent(std::shared_ptr<RunningLockInner>& lockInner)
{
#ifdef HAS_HIVIEWDFX_HISYSEVENT_PART
    constexpr int32_t APP_HOLD_RUNNINGLOCK_TIMEOUT = 7200000;
    int32_t endTimeMs = GetTickCount();
    int32_t beginTimeMs = lockInner->GetBeginTime();
    if (endTimeMs - beginTimeMs > APP_HOLD_RUNNINGLOCK_TIMEOUT) {
        POWER_HILOGI(FEATURE_RUNNING_LOCK, "app hold runninglock timeout=%{public}d", (endTimeMs - beginTimeMs));
        HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::POWER, "APP_HOLD_RUNNINGLOCK_TIMEOUT",
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR, "PID", lockInner->GetPid(), "UID", lockInner->GetUid(),
            "TYPE", static_cast<int32_t>(lockInner->GetParam().type), "NAME", lockInner->GetParam().name);
    }
#endif
}

void RunningLockMgr::RegisterRunningLockCallback(const sptr<IPowerRunninglockCallback>& callback)
{
    if (g_runningLockCallback != nullptr) {
        UnRegisterRunningLockCallback(callback);
    }
    g_runningLockCallback = callback;
    POWER_HILOGI(FEATURE_RUNNING_LOCK, "RegisterRunningLockCallback success");
}

void RunningLockMgr::UnRegisterRunningLockCallback(const sptr<IPowerRunninglockCallback>& callback)
{
    g_runningLockCallback = nullptr;
    POWER_HILOGI(FEATURE_RUNNING_LOCK, "UnRegisterRunningLockCallback success");
}

void RunningLockMgr::QueryRunningLockLists(std::map<std::string, RunningLockInfo>& runningLockLists)
{
    std::lock_guard<std::mutex> lock(screenLockListsMutex_);
    for (auto &iter : unSceneLockLists_) {
        runningLockLists.insert(std::pair<std::string, RunningLockInfo>(iter.first, iter.second));
    }
    return;
}

bool RunningLockMgr::IsUsed(const sptr<IRemoteObject>& remoteObj)
{
    auto lockInner = GetRunningLockInner(remoteObj);
    if (lockInner == nullptr || lockInner->GetState() != RunningLockState::RUNNINGLOCK_STATE_ENABLE) {
        return false;
    }
    return true;
}

uint32_t RunningLockMgr::GetRunningLockNum(RunningLockType type)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (type == RunningLockType::RUNNINGLOCK_BUTT) {
        return runningLocks_.size();
    }
    return std::count_if(runningLocks_.begin(), runningLocks_.end(),
        [&type](const auto& pair) {
            return pair.second->GetType() == type;
        });
}

uint32_t RunningLockMgr::GetValidRunningLockNum(RunningLockType type)
{
    auto iterator = lockCounters_.find(type);
    if (iterator == lockCounters_.end()) {
        POWER_HILOGD(FEATURE_RUNNING_LOCK, "No specific lock, type=%{public}d", type);
        return 0;
    }
    std::shared_ptr<LockCounter> counter = iterator->second;
    return counter->GetCount();
}

bool RunningLockMgr::ExistValidRunningLock()
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it = runningLocks_.begin(); it != runningLocks_.end(); it++) {
        auto& lockinner = it->second;
        if (lockinner->GetState() == RunningLockState::RUNNINGLOCK_STATE_ENABLE) {
            return true;
        }
    }
    return false;
}

void RunningLockMgr::NotifyRunningLockChanged(const RunningLockParam& lockInnerParam, const std::string &tag,
    const std::string &logTag)
{
    uint64_t lockid = lockInnerParam.lockid;
    int32_t pid = lockInnerParam.pid;
    int32_t uid = lockInnerParam.uid;
    int32_t type = static_cast <int32_t>(lockInnerParam.type);
    auto pos = lockInnerParam.name.rfind('_');
    string name = lockInnerParam.name.substr(0, pos);
    string bundleName = lockInnerParam.bundleName;
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    std::string message;
    message.append("LOCKID=").append(std::to_string(lockid))
            .append(" PID=").append(std::to_string(pid))
            .append(" UID=").append(std::to_string(uid))
            .append(" TYPE=").append(std::to_string(type))
            .append(" NAME=").append(name)
            .append(" BUNDLENAME=").append(bundleName)
            .append(" TAG=").append(tag)
            .append(" TIMESTAMP=").append(std::to_string(timestamp));
    // runninglock message
    POWER_HILOGI(COMP_LOCK, "P=%{public}dU=%{public}dT=%{public}dN=%{public}sB=%{public}sTA=%{public}s",
        pid, uid, type, lockInnerParam.name.c_str(), bundleName.c_str(), logTag.c_str());
    if (g_runningLockCallback != nullptr) {
        g_runningLockCallback->HandleRunningLockMessage(message);
    }
}

uint64_t RunningLockMgr::TransformLockid(const sptr<IRemoteObject>& remoteObj)
{
    uintptr_t remoteObjPtr = reinterpret_cast<uintptr_t>(remoteObj.GetRefPtr());
    uint64_t lockid = std::hash<uintptr_t>()(remoteObjPtr);
    return lockid;
}

bool RunningLockMgr::ProxyRunningLock(bool isProxied, pid_t pid, pid_t uid)
{
    if (pid < VALID_PID_LIMIT) {
        POWER_HILOGW(FEATURE_RUNNING_LOCK, "Proxy runninglock failed, pid=%{public}d is invalid", pid);
        return false;
    }

    if (isProxied) {
        runninglockProxy_->IncreaseProxyCnt(pid, uid);
    } else {
        runninglockProxy_->DecreaseProxyCnt(pid, uid);
    }
    return true;
}

void RunningLockMgr::ProxyRunningLocks(bool isProxied, const std::vector<std::pair<pid_t, pid_t>>& processInfos)
{
    for (const auto& [pid, uid] : processInfos) {
        ProxyRunningLock(isProxied, pid, uid);
    }
}

void RunningLockMgr::ResetRunningLocks()
{
    POWER_HILOGI(FEATURE_RUNNING_LOCK, "Reset runninglock proxy");
    runninglockProxy_->ResetRunningLocks();
}

void RunningLockMgr::LockInnerByProxy(const sptr<IRemoteObject>& remoteObj,
    std::shared_ptr<RunningLockInner>& lockInner)
{
    if (!lockInner->IsProxied()) {
        // LockInnerByProxy failed, runninglock UnProxied
        POWER_HILOGW(FEATURE_RUNNING_LOCK, "Lock UnProxied");
        return;
    }
    RunningLockParam lockInnerParam = lockInner->GetParam();
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "try LockInnerByProxy, name: %{public}s, type: %{public}d",
        lockInnerParam.name.c_str(), lockInnerParam.type);
    RunningLockState lastState = lockInner->GetState();
    if (lastState == RunningLockState::RUNNINGLOCK_STATE_PROXIED) {
        lockInner->SetState(RunningLockState::RUNNINGLOCK_STATE_DISABLE);
        Lock(remoteObj);
    }
}

void RunningLockMgr::UnlockInnerByProxy(const sptr<IRemoteObject>& remoteObj,
    std::shared_ptr<RunningLockInner>& lockInner)
{
    RunningLockState lastState = lockInner->GetState();
    if (lastState == RunningLockState::RUNNINGLOCK_STATE_DISABLE) {
        // UnlockInnerByProxy failed, runninglock Disable
        POWER_HILOGW(FEATURE_RUNNING_LOCK, "Unlock Disable");
        return;
    }
    RunningLockParam lockInnerParam = lockInner->GetParam();
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "try UnlockInnerByProxy, name: %{public}s, type: %{public}d",
        lockInnerParam.name.c_str(), lockInnerParam.type);
    UnLock(remoteObj);
    lockInner->SetState(RunningLockState::RUNNINGLOCK_STATE_PROXIED);
}

void RunningLockMgr::EnableMock(IRunningLockAction* mockAction)
{
    // reset lock list
    runningLocks_.clear();
    for (auto it = lockCounters_.begin(); it != lockCounters_.end(); it++) {
        it->second->Clear();
    }
    runninglockProxy_->Clear();
#ifdef HAS_SENSORS_SENSOR_PART
    proximityController_->Clear();
#endif
    std::shared_ptr<IRunningLockAction> mock(mockAction);
    runningLockAction_ = mock;
}

void RunningLockMgr::DumpInfo(std::string& result)
{
    auto validSize = GetValidRunningLockNum();
    std::lock_guard<std::mutex> lock(mutex_);

    result.append("RUNNING LOCK DUMP:\n");
    result.append("  totalSize=").append(ToString(runningLocks_.size()))
            .append(" validSize=").append(ToString(validSize)).append("\n");
    result.append("Summary By Type: \n");
    for (auto it = lockCounters_.begin(); it != lockCounters_.end(); it++) {
        result.append("  ")
            .append(PowerUtils::GetRunningLockTypeString(it->first))
            .append(": ")
            .append(ToString(it->second->GetCount()))
            .append("\n");
    }

    if (runningLocks_.empty()) {
        result.append("Lock List is Empty. \n");
        return;
    }

    result.append("Dump Lock List: \n");
    auto curTick = GetTickCount();
    int index = 0;
    for (const auto& it : runningLocks_) {
        index++;
        auto lockInner = it.second;
        if (lockInner == nullptr) {
            return;
        }
        auto& lockParam = lockInner->GetParam();
        result.append("  index=").append(ToString(index))
            .append(" time=").append(ToString(curTick - lockInner->GetLockTimeMs()))
            .append(" type=").append(PowerUtils::GetRunningLockTypeString(lockParam.type))
            .append(" name=").append(lockParam.name)
            .append(" uid=").append(ToString(lockInner->GetUid()))
            .append(" pid=").append(ToString(lockInner->GetPid()))
            .append(" state=").append(ToString(static_cast<uint32_t>(lockInner->GetState())))
            .append("\n");
    }

    result.append("Dump Proxy List: \n");
    result.append(runninglockProxy_->DumpProxyInfo());
#ifdef HAS_SENSORS_SENSOR_PART
    result.append("Peripherals Info: \n")
            .append("  Proximity: ")
            .append("Enabled=")
            .append(ToString(proximityController_->IsEnabled()))
            .append(" Status=")
            .append(ToString(proximityController_->GetStatus()))
            .append("\n");
#endif
}

void RunningLockMgr::RunningLockDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    if (remote == nullptr || remote.promote() == nullptr) {
        POWER_HILOGW(FEATURE_RUNNING_LOCK, "Remote is nullptr");
        return;
    }
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        POWER_HILOGW(FEATURE_RUNNING_LOCK, "Power service is nullptr");
        return;
    }
    pms->ForceUnLock(remote.promote());
}

int32_t RunningLockMgr::LockCounter::Increase(const RunningLockParam& lockInnerParam)
{
    ++counter_;
    int32_t result = RUNNINGLOCK_SUCCESS;
    if (counter_ == 1) {
        result = activate_(true, lockInnerParam);
        if (result != RUNNINGLOCK_SUCCESS) {
            --counter_;
        }
    }
    if (result == RUNNINGLOCK_SUCCESS  && NeedNotify(lockInnerParam.type)) {
        NotifyRunningLockChanged(lockInnerParam, "DUBAI_TAG_RUNNINGLOCK_ADD", "AD");
    }
    return result;
}

int32_t RunningLockMgr::LockCounter::Decrease(const RunningLockParam& lockInnerParam)
{
    --counter_;
    int32_t result = RUNNINGLOCK_SUCCESS;
    if (counter_ == 0) {
        result = activate_(false, lockInnerParam);
        if (result != RUNNINGLOCK_SUCCESS) {
            ++counter_;
        }
    }
    if (result == RUNNINGLOCK_SUCCESS && NeedNotify(lockInnerParam.type)) {
        NotifyRunningLockChanged(lockInnerParam, "DUBAI_TAG_RUNNINGLOCK_REMOVE", "RE");
    }
    return result;
}

void RunningLockMgr::LockCounter::Clear()
{
    counter_ = 0;
}

#ifdef HAS_SENSORS_SENSOR_PART
void RunningLockMgr::ProximityController::RecordSensorCallback(SensorEvent *event)
{
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "Sensor Callback come in");
    if (event == nullptr) {
        POWER_HILOGW(FEATURE_RUNNING_LOCK, "Sensor event is nullptr");
        return;
    }
    if (event->sensorTypeId != SENSOR_TYPE_ID_PROXIMITY) {
        POWER_HILOGW(FEATURE_RUNNING_LOCK, "Sensor type is not PROXIMITY");
        return;
    }
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Power service is nullptr");
        return;
    }
    auto runningLock = pms->GetRunningLockMgr();
    if (runningLock == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "runningLock is nullptr");
        return;
    }
    ProximityData* data = reinterpret_cast<ProximityData*>(event->data);
    int32_t distance = static_cast<int32_t>(data->distance);

    POWER_HILOGI(FEATURE_RUNNING_LOCK, "SensorD=%{public}d", distance);
    if (distance == PROXIMITY_CLOSE_SCALAR) {
        runningLock->SetProximity(IProximityController::PROXIMITY_CLOSE);
    } else if (distance == PROXIMITY_AWAY_SCALAR) {
        runningLock->SetProximity(IProximityController::PROXIMITY_AWAY);
    }
}

void RunningLockMgr::ProximityController::OnClose()
{
    if (!IsEnabled() || IsClose()) {
        POWER_HILOGI(FEATURE_RUNNING_LOCK, "PROXIMITY is disabled or closed already");
        return;
    }
    SetClose(true);
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "PROXIMITY is closed");
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Power service is nullptr");
        return;
    }
    auto runningLock = pms->GetRunningLockMgr();
    if (runningLock == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "runningLock is nullptr");
        return;
    }
    runningLock->HandleProximityCloseEvent();
}

void RunningLockMgr::ProximityController::OnAway()
{
    if (!IsEnabled() || !IsClose()) {
        POWER_HILOGI(FEATURE_RUNNING_LOCK, "PROXIMITY is disabled or away already");
        return;
    }
    SetClose(false);
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "PROXIMITY is away");
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Power service is nullptr");
        return;
    }
    auto runningLock = pms->GetRunningLockMgr();
    if (runningLock == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "runningLock is nullptr");
        return;
    }
    runningLock->HandleProximityAwayEvent();
}

void RunningLockMgr::HandleProximityCloseEvent()
{
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Power service is nullptr");
        return;
    }
    auto stateMachine = pms->GetPowerStateMachine();
    if (stateMachine == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "state machine is nullptr");
        return;
    }
    if (GetValidRunningLockNum(RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL) > 0) {
        POWER_HILOGI(FEATURE_RUNNING_LOCK, "Change state to INACITVE when holding PROXIMITY LOCK");
        uint32_t delayTime = FOREGROUND_INCALL_DELAY_TIME_MS;
        if (!PowerUtils::IsForegroundApplication(INCALL_APP_BUNDLE_NAME)) {
            delayTime = BACKGROUND_INCALL_DELAY_TIME_MS;
        }
        if (pms->IsDuringCallStateEnable() && stateMachine->IsDuringCall() && stateMachine->IsScreenOn()) {
            POWER_HILOGI(
                FEATURE_RUNNING_LOCK, "Start proximity-screen-switch timer, delay time:%{public}u", delayTime);
            FFRTTask delayScreenSwitchToSubTask = [] {
                auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
                auto stateMachine = pms->GetPowerStateMachine();
                if (stateMachine == nullptr) {
                    POWER_HILOGE(FEATURE_RUNNING_LOCK, "state machine is nullptr");
                    return;
                }
                POWER_HILOGI(FEATURE_POWER_STATE, "proximity-screen-switch timer task is triggered");
                bool ret = stateMachine->HandleDuringCall(true);
                POWER_HILOGI(FEATURE_POWER_STATE, "Proximity close when duringcall mode, ret:%{public}d", ret);
            };
            stateMachine->SetDelayTimer(
                delayTime, FFRTTimerId::TIMER_ID_PROXIMITY_SCREEN_SWITCH_TO_SUB, delayScreenSwitchToSubTask);
            return;
        }
        POWER_HILOGI(FEATURE_RUNNING_LOCK, "Start proximity-screen-off timer, delay time:%{public}u", delayTime);
        stateMachine->SetDelayTimer(delayTime, PowerStateMachine::CHECK_PROXIMITY_SCREEN_OFF_MSG);
    } else {
        POWER_HILOGI(FEATURE_RUNNING_LOCK, "Unholding PROXIMITY LOCK");
    }
}

void RunningLockMgr::HandleProximityAwayEvent()
{
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Power service is nullptr");
        return;
    }
    auto stateMachine = pms->GetPowerStateMachine();
    if (stateMachine == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "state machine is nullptr");
        return;
    }
    auto runningLock = pms->GetRunningLockMgr();
    if (GetValidRunningLockNum(RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL) > 0) {
        if (pms->IsDuringCallStateEnable()) {
            stateMachine->CancelDelayTimer(PowerStateMachine::CHECK_PROXIMITY_SCREEN_SWITCH_TO_SUB_MSG);
            if (stateMachine->HandleDuringCall(false)) {
                POWER_HILOGI(FEATURE_RUNNING_LOCK, "Proximity away when duringcall mode");
                return;
            }
        }
        POWER_HILOGI(FEATURE_RUNNING_LOCK, "Change state to AWAKE when holding PROXIMITY LOCK");
        runningLock->PreprocessBeforeAwake();
        stateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_PROXIMITY, true);
    } else {
        POWER_HILOGI(FEATURE_RUNNING_LOCK, "Unholding PROXIMITY LOCK");
    }
}

void RunningLockMgr::SetProximity(uint32_t status)
{
    switch (status) {
        case IProximityController::PROXIMITY_CLOSE:
            proximityController_->OnClose();
            break;
        case IProximityController::PROXIMITY_AWAY:
            proximityController_->OnAway();
            break;
        default:
            break;
    }
}

bool RunningLockMgr::IsExistAudioStream(pid_t uid)
{
    return runninglockProxy_->IsExistAudioStream(uid);
}

bool RunningLockMgr::IsProximityClose()
{
    return proximityController_->IsClose();
}
#endif

} // namespace PowerMgr
} // namespace OHOS
