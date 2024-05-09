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
#include <hisysevent.h>
#include <ipc_skeleton.h>
#include <securec.h>

#include "power_hitrace.h"
#include "ffrt_utils.h"
#include "power_log.h"
#include "power_mgr_factory.h"
#include "power_mgr_service.h"
#include "power_utils.h"
#include "system_suspend_controller.h"

using namespace std;

namespace OHOS {
namespace PowerMgr {
namespace {
const string TASK_RUNNINGLOCK_FORCEUNLOCK = "RunningLock_ForceUnLock";
constexpr int32_t VALID_PID_LIMIT = 1;
constexpr uint32_t COORDINATION_LOCK_AUTO_SUSPEND_DELAY_TIME = 0;
sptr<IPowerRunninglockCallback> g_runningLockCallback = nullptr;
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
                POWER_HILOGI(FEATURE_RUNNING_LOCK,
                    "[UL_POWER] RUNNINGLOCK_SCREEN active, and the  currrent power state = %{public}d",
                    stateMachine->GetState());
            } else {
                POWER_HILOGI(FEATURE_RUNNING_LOCK, "[UL_POWER] RUNNINGLOCK_SCREEN inactive");
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
            POWER_HILOGI(FEATURE_RUNNING_LOCK, "Background runningLock active");
            return runningLockAction_->Lock(lockInnerParam);
        } else {
            POWER_HILOGI(FEATURE_RUNNING_LOCK, "Background runningLock inactive");
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
void RunningLockMgr::ProximityLockOn()
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

    POWER_HILOGI(FEATURE_RUNNING_LOCK, "RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL active");
    proximityController_.Enable();
    if (proximityController_.IsClose()) {
        POWER_HILOGI(FEATURE_RUNNING_LOCK, "[UL_POWER] INACTIVE when proximity is closed");
        bool ret = stateMachine->SetState(PowerState::INACTIVE,
            StateChangeReason::STATE_CHANGE_REASON_PROXIMITY, true);
        if (ret) {
            suspendController->StartSleepTimer(SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION,
                static_cast<uint32_t>(SuspendAction::ACTION_AUTO_SUSPEND), 0);
        }
    } else {
        POWER_HILOGI(FEATURE_RUNNING_LOCK, "[UL_POWER] AWAKE when proximity is away");
        PreprocessBeforeAwake();
        stateMachine->SetState(PowerState::AWAKE,
            StateChangeReason::STATE_CHANGE_REASON_PROXIMITY, true);
    }
}

void RunningLockMgr::InitLocksTypeProximity()
{
    lockCounters_.emplace(RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL,
        std::make_shared<LockCounter>(RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL,
            [this](bool active, [[maybe_unused]] RunningLockParam runningLockParam) -> int32_t {
            POWER_HILOGD(FEATURE_RUNNING_LOCK, "RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL action start");
            auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
            if (pms == nullptr) {
                return RUNNINGLOCK_FAILURE;
            }
            auto stateMachine = pms->GetPowerStateMachine();
            if (stateMachine == nullptr) {
                return RUNNINGLOCK_FAILURE;
            }
            if (active) {
                ProximityLockOn();
            } else {
                POWER_HILOGI(FEATURE_RUNNING_LOCK, "[UL_POWER] RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL inactive");
                PreprocessBeforeAwake();
                stateMachine->SetState(PowerState::AWAKE,
                    StateChangeReason::STATE_CHANGE_REASON_RUNNING_LOCK);
                stateMachine->ResetInactiveTimer();
                proximityController_.Disable();
                proximityController_.Clear();
            }
            return RUNNINGLOCK_SUCCESS;
        })
    );
}
#endif

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
                stateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_RUNNING_LOCK);
                stateMachine->ResetInactiveTimer();
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

std::shared_ptr<RunningLockInner> RunningLockMgr::CreateRunningLock(const sptr<IRemoteObject>& remoteObj,
    const RunningLockParam& runningLockParam)
{
    auto lockInner = RunningLockInner::CreateRunningLockInner(runningLockParam);
    if (lockInner == nullptr) {
        return nullptr;
    }
    POWER_HILOGI(FEATURE_RUNNING_LOCK, "CreateRunningLock name:%{public}s, type:%{public}d, bundleName:%{public}s",
        lockInner->GetName().c_str(), lockInner->GetType(), lockInner->GetBundleName().c_str());
    {
        std::lock_guard<std::mutex> lock(mutex_);
        runningLocks_.emplace(remoteObj, lockInner);
    }
    runninglockProxy_->AddRunningLock(lockInner->GetPid(), lockInner->GetUid(), remoteObj);
    remoteObj->AddDeathRecipient(runningLockDeathRecipient_);
    return lockInner;
}

bool RunningLockMgr::ReleaseLock(const sptr<IRemoteObject> remoteObj)
{
    bool result = false;
    auto lockInner = GetRunningLockInner(remoteObj);
    if (lockInner == nullptr) {
        return result;
    }
    POWER_HILOGI(FEATURE_RUNNING_LOCK, "ReleaseLock name:%{public}s, type:%{public}d, bundleName:%{public}s",
        lockInner->GetName().c_str(), lockInner->GetType(), lockInner->GetBundleName().c_str());
    {
        std::lock_guard<std::mutex> lock(mutex_);
        runningLocks_.erase(remoteObj);
    }
    runninglockProxy_->RemoveRunningLock(lockInner->GetPid(), lockInner->GetUid(), remoteObj);
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

void RunningLockMgr::UpdateUnSceneLockLists(RunningLockParam& singleLockParam, bool fill)
{
    auto iterator = unSceneLockLists_.find(singleLockParam.bundleName);
    if (fill) {
        if (iterator == unSceneLockLists_.end()) {
            POWER_HILOGD(FEATURE_RUNNING_LOCK, "Add non-scene lock information from lists");
            RunningLockInfo unSceneAppLockInfo = FillAppRunningLockInfo(singleLockParam);
            unSceneLockLists_.insert(
                std::pair<std::string, RunningLockInfo>(unSceneAppLockInfo.bundleName, unSceneAppLockInfo));
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

bool RunningLockMgr::Lock(const sptr<IRemoteObject>& remoteObj)
{
    PowerHitrace powerHitrace("RunningLock_Lock");
    auto lockInner = GetRunningLockInner(remoteObj);
    if (lockInner == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "LockInner is nullptr");
        return false;
    }
    RunningLockParam lockInnerParam = lockInner->GetParam();
    POWER_HILOGI(FEATURE_RUNNING_LOCK, "try Lock, name: %{public}s, type: %{public}d lockid: %{public}s",
        lockInnerParam.name.c_str(), lockInnerParam.type, std::to_string(lockInnerParam.lockid).c_str());
    if (lockInner->IsProxied() || runninglockProxy_->IsProxied(lockInnerParam.pid, lockInnerParam.uid)) {
        POWER_HILOGW(FEATURE_RUNNING_LOCK, "Runninglock is proxied");
        return false;
    }
    if (!IsValidType(lockInnerParam.type)) {
        POWER_HILOGW(FEATURE_RUNNING_LOCK, "Runninglock type is invalid");
        return false;
    }
    if (lockInner->GetState() == RunningLockState::RUNNINGLOCK_STATE_ENABLE) {
        POWER_HILOGI(FEATURE_RUNNING_LOCK, "Lock is already enabled");
        return false;
    }
    if (lockInnerParam.type == RunningLockType::RUNNINGLOCK_SCREEN) {
        UpdateUnSceneLockLists(lockInnerParam, true);
    }
    auto iterator = lockCounters_.find(lockInnerParam.type);
    if (iterator == lockCounters_.end()) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Lock failed unsupported type, type=%{public}d", lockInnerParam.type);
        return false;
    }
    std::shared_ptr<LockCounter> counter = iterator->second;
    if (counter->Increase(lockInnerParam) != RUNNINGLOCK_SUCCESS) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "LockCounter increase failed");
        return false;
    }
    lockInner->SetState(RunningLockState::RUNNINGLOCK_STATE_ENABLE);
    return true;
}

bool RunningLockMgr::UnLock(const sptr<IRemoteObject> remoteObj)
{
    PowerHitrace powerHitrace("RunningLock_Unlock");
    auto lockInner = GetRunningLockInner(remoteObj);
    if (lockInner == nullptr) {
        return false;
    }
    if (lockInner->IsProxied()) {
        POWER_HILOGW(FEATURE_RUNNING_LOCK, "Runninglock is proxied");
        return false;
    }
    auto lockInnerParam = lockInner->GetParam();
    POWER_HILOGI(FEATURE_RUNNING_LOCK, "try UnLock, name: %{public}s, type: %{public}d lockid: %{public}s",
        lockInnerParam.name.c_str(), lockInnerParam.type, std::to_string(lockInnerParam.lockid).c_str());
    if (lockInner->GetState() == RunningLockState::RUNNINGLOCK_STATE_DISABLE) {
        POWER_HILOGI(FEATURE_RUNNING_LOCK, "Lock is already disabled, name=%{public}s", lockInner->GetName().c_str());
        return false;
    }
    if (lockInnerParam.type == RunningLockType::RUNNINGLOCK_SCREEN) {
        UpdateUnSceneLockLists(lockInnerParam, false);
    }
    auto iterator = lockCounters_.find(lockInnerParam.type);
    if (iterator == lockCounters_.end()) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Unlock failed unsupported type, type=%{public}d",
            lockInnerParam.type);
        return false;
    }
    std::shared_ptr<LockCounter> counter = iterator->second;
    if (counter->Decrease(lockInnerParam)) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "LockCounter decrease failed");
        return false;
    }
    lockInner->SetState(RunningLockState::RUNNINGLOCK_STATE_DISABLE);
    return true;
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

void RunningLockMgr::NotifyRunningLockChanged(const RunningLockParam& lockInnerParam, const std::string &tag)
{
    uint64_t lockid = lockInnerParam.lockid;
    int32_t pid = lockInnerParam.pid;
    int32_t uid = lockInnerParam.uid;
    int32_t type = static_cast <int32_t>(lockInnerParam.type);
    string name = lockInnerParam.name;
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
    POWER_HILOGI(FEATURE_RUNNING_LOCK, "runninglock message: %{public}s", message.c_str());
    if (g_runningLockCallback != nullptr) {
        g_runningLockCallback->HandleRunningLockMessage(message);
    }
}

bool RunningLockMgr::ProxyRunningLock(bool isProxied, pid_t pid, pid_t uid)
{
    bool isSuccess = false;
    if (pid < VALID_PID_LIMIT) {
        POWER_HILOGW(FEATURE_RUNNING_LOCK, "Proxy runninglock failed, pid=%{public}d is invalid", pid);
        return isSuccess;
    }

    if (isProxied) {
        runninglockProxy_->IncreaseProxyCnt(pid, uid, [this, pid, uid] () {
            this->ProxyRunningLockInner(true, pid, uid);
        });
    } else {
        runninglockProxy_->DecreaseProxyCnt(pid, uid, [this, pid, uid] () {
            this->ProxyRunningLockInner(false, pid, uid);
        });
    }
    return true;
}

void RunningLockMgr::ProxyRunningLockInner(bool isProxied, pid_t pid, pid_t uid)
{
    auto remoteObjList = runninglockProxy_->GetRemoteObjectList(pid, uid);
    if (remoteObjList.empty()) {
        POWER_HILOGW(FEATURE_RUNNING_LOCK, "Proxy runninglock failed, no matching runninglock exist");
        return;
    }
    for (auto it : remoteObjList) {
        auto lockInner = GetRunningLockInner(it);
        if (lockInner == nullptr) {
            continue;
        }
        if (isProxied) {
            UnlockInnerByProxy(it, lockInner);
        } else {
            LockInnerByProxy(it, lockInner);
        }
    }
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
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& it : runningLocks_) {
        LockInnerByProxy(it.first, it.second);
    }
    runninglockProxy_->ResetRunningLocks();
}

void RunningLockMgr::LockInnerByProxy(const sptr<IRemoteObject>& remoteObj,
    std::shared_ptr<RunningLockInner>& lockInner)
{
    if (!lockInner->IsProxied()) {
        POWER_HILOGW(FEATURE_RUNNING_LOCK, "LockInnerByProxy failed, runninglock Proxied");
        return;
    }
    RunningLockState lastState = lockInner->GetState();
    lockInner->SetState(RunningLockState::RUNNINGLOCK_STATE_DISABLE);
    if (lastState == RunningLockState::RUNNINGLOCK_STATE_UNPROXIED_RESTORE) {
        Lock(remoteObj);
    }
}

void RunningLockMgr::UnlockInnerByProxy(const sptr<IRemoteObject>& remoteObj,
    std::shared_ptr<RunningLockInner>& lockInner)
{
    RunningLockState lastState = lockInner->GetState();
    if (lastState == RunningLockState::RUNNINGLOCK_STATE_DISABLE) {
        POWER_HILOGW(FEATURE_RUNNING_LOCK, "UnlockInnerByProxy failed, runninglock Disable");
        return;
    }
    UnLock(remoteObj);
    lockInner->SetState(RunningLockState::RUNNINGLOCK_STATE_UNPROXIED_RESTORE);
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
    proximityController_.Clear();
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
            .append(ToString(proximityController_.IsEnabled()))
            .append(" Status=")
            .append(ToString(proximityController_.GetStatus()))
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
    if (result == RUNNINGLOCK_SUCCESS  && IsSceneRunningLockType(lockInnerParam.type)) {
        NotifyRunningLockChanged(lockInnerParam, "DUBAI_TAG_RUNNINGLOCK_ADD");
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
    if (result == RUNNINGLOCK_SUCCESS && IsSceneRunningLockType(lockInnerParam.type)) {
        NotifyRunningLockChanged(lockInnerParam, "DUBAI_TAG_RUNNINGLOCK_REMOVE");
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
    ProximityData* data = reinterpret_cast<ProximityData*>(event->data);
    int32_t distance = static_cast<int32_t>(data->distance);

    POWER_HILOGD(FEATURE_RUNNING_LOCK, "Sensor Callback data->distance=%{public}d", distance);
    if (distance == PROXIMITY_CLOSE_SCALAR) {
        runningLock->SetProximity(PROXIMITY_CLOSE);
    } else if (distance == PROXIMITY_AWAY_SCALAR) {
        runningLock->SetProximity(PROXIMITY_AWAY);
    }
}

RunningLockMgr::ProximityController::ProximityController()
{
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "Instance enter");
    SensorInfo* sensorInfo = nullptr;
    int32_t count;
    int ret = GetAllSensors(&sensorInfo, &count);
    if (ret != 0 || sensorInfo == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Get sensors fail, ret=%{public}d", ret);
        return;
    }
    for (int32_t i = 0; i < count; i++) {
        if (sensorInfo[i].sensorTypeId == SENSOR_TYPE_ID_PROXIMITY) {
            POWER_HILOGD(FEATURE_RUNNING_LOCK, "Support PROXIMITY sensor");
            support_ = true;
            break;
        }
    }
    if (!support_) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "PROXIMITY sensor not support");
        return;
    }
    if (strcpy_s(user_.name, sizeof(user_.name), "RunningLock") != EOK) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "strcpy_s error");
        return;
    }
    user_.userData = nullptr;
    user_.callback = &RecordSensorCallback;
}

RunningLockMgr::ProximityController::~ProximityController()
{
    if (support_) {
        UnsubscribeSensor(SENSOR_TYPE_ID_PROXIMITY, &user_);
    }
}

void RunningLockMgr::ProximityController::Enable()
{
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "Enter");
    enabled_ = true;
    if (!support_) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "PROXIMITY sensor not support");
        return;
    }

    int32_t errorCode = SubscribeSensor(SENSOR_TYPE_ID_PROXIMITY, &user_);
    if (errorCode != ERR_OK) {
        POWER_HILOGW(FEATURE_RUNNING_LOCK, "SubscribeSensor PROXIMITY failed, errorCode=%{public}d", errorCode);
        return;
    }
    SetBatch(SENSOR_TYPE_ID_PROXIMITY, &user_, SAMPLING_RATE, SAMPLING_RATE);
    errorCode = ActivateSensor(SENSOR_TYPE_ID_PROXIMITY, &user_);
    if (errorCode != ERR_OK) {
        POWER_HILOGW(FEATURE_RUNNING_LOCK, "ActivateSensor PROXIMITY failed, errorCode=%{public}d", errorCode);
        return;
    }
    SetMode(SENSOR_TYPE_ID_PROXIMITY, &user_, SENSOR_ON_CHANGE);
}

void RunningLockMgr::ProximityController::Disable()
{
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "Enter");
    enabled_ = false;
    if (!support_) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "PROXIMITY sensor not support");
        return;
    }

    DeactivateSensor(SENSOR_TYPE_ID_PROXIMITY, &user_);
    int32_t errorCode = UnsubscribeSensor(SENSOR_TYPE_ID_PROXIMITY, &user_);
    if (errorCode != ERR_OK) {
        POWER_HILOGW(FEATURE_RUNNING_LOCK, "UnsubscribeSensor PROXIMITY failed, errorCode=%{public}d", errorCode);
    }
}

bool RunningLockMgr::ProximityController::IsClose()
{
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "PROXIMITY IsClose: %{public}d", isClose);
    return isClose;
}

void RunningLockMgr::ProximityController::OnClose()
{
    if (!enabled_ || IsClose()) {
        POWER_HILOGD(FEATURE_RUNNING_LOCK, "PROXIMITY is disabled or closed already");
        return;
    }
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Power service is nullptr");
        return;
    }
    auto stateMachine = pms->GetPowerStateMachine();
    auto suspendController = pms->GetSuspendController();
    if (stateMachine == nullptr || suspendController == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "state machine is nullptr");
        return;
    }
    isClose = true;
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "PROXIMITY is closed");
    auto runningLock = pms->GetRunningLockMgr();
    if (runningLock->GetValidRunningLockNum(
        RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL) > 0) {
        POWER_HILOGD(FEATURE_RUNNING_LOCK, "Change state to INACITVE when holding PROXIMITY LOCK");
        bool ret = stateMachine->SetState(PowerState::INACTIVE, StateChangeReason::STATE_CHANGE_REASON_PROXIMITY, true);
        if (ret) {
            suspendController->StartSleepTimer(SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION,
                static_cast<uint32_t>(SuspendAction::ACTION_AUTO_SUSPEND), 0);
        }
    }
}

void RunningLockMgr::ProximityController::OnAway()
{
    if (!enabled_ || !IsClose()) {
        POWER_HILOGD(FEATURE_RUNNING_LOCK, "PROXIMITY is disabled or away already");
        return;
    }
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
    isClose = false;
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "PROXIMITY is away");
    auto runningLock = pms->GetRunningLockMgr();
    if (runningLock->GetValidRunningLockNum(
        RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL) > 0) {
        POWER_HILOGD(FEATURE_RUNNING_LOCK, "Change state to AWAKE when holding PROXIMITY LOCK");
        runningLock->PreprocessBeforeAwake();
        stateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_PROXIMITY, true);
    }
}

void RunningLockMgr::ProximityController::Clear()
{
    isClose = false;
}

void RunningLockMgr::SetProximity(uint32_t status)
{
    switch (status) {
        case PROXIMITY_CLOSE:
            proximityController_.OnClose();
            break;
        case PROXIMITY_AWAY:
            proximityController_.OnAway();
            break;
        default:
            break;
    }
}
#endif

} // namespace PowerMgr
} // namespace OHOS
