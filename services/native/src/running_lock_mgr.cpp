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

#ifdef HAS_DISPLAY_MANAGER_PART
#include "dm_common.h"
#endif
#include "hitrace_meter.h"
#include "ffrt_utils.h"
#include "power_log.h"
#include "power_mgr_factory.h"
#include "power_mgr_service.h"
#include "system_suspend_controller.h"
#ifdef HAS_DISPLAY_MANAGER_PART
#include "screen_manager.h"
#endif

using namespace std;

namespace OHOS {
namespace PowerMgr {
namespace {
const string TASK_RUNNINGLOCK_FORCEUNLOCK = "RunningLock_ForceUnLock";
constexpr int32_t VALID_PID_LIMIT = 1;
FFRTQueue g_queue("power_running_lock_mgr");
FFRTHandle g_runningLockTimeoutHandle;
}

RunningLockMgr::~RunningLockMgr() {}

bool RunningLockMgr::Init()
{
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "Init start");
    std::lock_guard<std::mutex> lock(mutex_);
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

    if (backgroundLock_ == nullptr) {
        backgroundLock_ = std::make_shared<SystemLock>(
            runningLockAction_, RunningLockType::RUNNINGLOCK_BACKGROUND, RUNNINGLOCK_TAG_BACKGROUND);
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
    InitLocksTypeProximity();
    return true;
}

void RunningLockMgr::InitLocksTypeScreen()
{
    lockCounters_.emplace(RunningLockType::RUNNINGLOCK_SCREEN,
        std::make_shared<LockCounter>(
        RunningLockType::RUNNINGLOCK_SCREEN, [this](bool active) {
            POWER_HILOGD(FEATURE_RUNNING_LOCK, "RUNNINGLOCK_SCREEN action start");
            auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
            if (pms == nullptr) {
                return;
            }
            auto stateMachine = pms->GetPowerStateMachine();
            if (stateMachine == nullptr) {
                return;
            }
            if (active) {
                POWER_HILOGI(FEATURE_RUNNING_LOCK, "RUNNINGLOCK_SCREEN active");
                SystemSuspendController::GetInstance().Wakeup();
                stateMachine->SetState(PowerState::AWAKE,
                    StateChangeReason::STATE_CHANGE_REASON_RUNNING_LOCK);
                stateMachine->CancelDelayTimer(
                    PowerStateMachine::CHECK_USER_ACTIVITY_TIMEOUT_MSG);
                stateMachine->CancelDelayTimer(
                    PowerStateMachine::CHECK_USER_ACTIVITY_OFF_TIMEOUT_MSG);
            } else {
                POWER_HILOGI(FEATURE_RUNNING_LOCK, "RUNNINGLOCK_SCREEN inactive");
                if (stateMachine->GetState() == PowerState::AWAKE) {
                    stateMachine->ResetInactiveTimer();
                } else {
                    POWER_HILOGD(FEATURE_RUNNING_LOCK, "Screen unlock in state: %{public}d",
                        stateMachine->GetState());
                }
            }
        })
    );
}

void RunningLockMgr::InitLocksTypeBackground()
{
    lockCounters_.emplace(RunningLockType::RUNNINGLOCK_BACKGROUND,
        std::make_shared<LockCounter>(
        RunningLockType::RUNNINGLOCK_BACKGROUND, [this](bool active) {
            POWER_HILOGD(FEATURE_RUNNING_LOCK, "RUNNINGLOCK_BACKGROUND action start");
            auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
            if (pms == nullptr) {
                return;
            }
            auto stateMachine = pms->GetPowerStateMachine();
            if (stateMachine == nullptr) {
                return;
            }
            if (active) {
                POWER_HILOGI(FEATURE_RUNNING_LOCK, "RUNNINGLOCK_BACKGROUND active");
                backgroundLock_->Lock();
            } else {
                POWER_HILOGI(FEATURE_RUNNING_LOCK, "RUNNINGLOCK_BACKGROUND inactive");
                if (stateMachine->GetState() == PowerState::INACTIVE) {
                    stateMachine->ResetSleepTimer();
                } else {
                    POWER_HILOGD(FEATURE_RUNNING_LOCK, "Background unlock in state: %{public}d",
                        stateMachine->GetState());
                }
                backgroundLock_->Unlock();
            }
        })
    );
}

void RunningLockMgr::InitLocksTypeProximity()
{
    lockCounters_.emplace(RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL,
        std::make_shared<LockCounter>(
        RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL, [this](bool active) {
            POWER_HILOGD(FEATURE_RUNNING_LOCK, "RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL action start");
            auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
            if (pms == nullptr) {
                return;
            }
            auto stateMachine = pms->GetPowerStateMachine();
            if (stateMachine == nullptr) {
                return;
            }
            if (active) {
                POWER_HILOGI(FEATURE_RUNNING_LOCK, "RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL active");
                proximityController_.Enable();
                if (proximityController_.IsClose()) {
                    POWER_HILOGI(FEATURE_RUNNING_LOCK, "INACTIVE when proximity is closed");
                    stateMachine->SetState(PowerState::INACTIVE,
                        StateChangeReason::STATE_CHANGE_REASON_RUNNING_LOCK, true);
                } else {
                    POWER_HILOGI(FEATURE_RUNNING_LOCK, "AWAKE when proximity is away");
                    stateMachine->SetState(PowerState::AWAKE,
                        StateChangeReason::STATE_CHANGE_REASON_RUNNING_LOCK, true);
                }
                stateMachine->CancelDelayTimer(
                    PowerStateMachine::CHECK_USER_ACTIVITY_TIMEOUT_MSG);
                stateMachine->CancelDelayTimer(
                    PowerStateMachine::CHECK_USER_ACTIVITY_OFF_TIMEOUT_MSG);
                backgroundLock_->Lock();
            } else {
                POWER_HILOGI(FEATURE_RUNNING_LOCK, "RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL inactive");
                stateMachine->SetState(PowerState::AWAKE,
                    StateChangeReason::STATE_CHANGE_REASON_RUNNING_LOCK);
                stateMachine->ResetInactiveTimer();
                backgroundLock_->Unlock();
                proximityController_.Disable();
            }
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
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "Create lock success, name=%{public}s, type=%{public}d",
        runningLockParam.name.c_str(), runningLockParam.type);

    mutex_.lock();
    runningLocks_.emplace(remoteObj, lockInner);
    runninglockProxy_->AddRunningLock(lockInner->GetPid(), lockInner->GetUid(), remoteObj);
    mutex_.unlock();
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

    UnLock(remoteObj);

    mutex_.lock();
    runningLocks_.erase(remoteObj);
    runninglockProxy_->RemoveRunningLock(lockInner->GetPid(), lockInner->GetUid(), remoteObj);
    mutex_.unlock();
    result = remoteObj->RemoveDeathRecipient(runningLockDeathRecipient_);
    return result;
}

void RunningLockMgr::RemoveAndPostUnlockTask(const sptr<IRemoteObject>& remoteObj, int32_t timeOutMS)
{
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "timeOutMS=%{public}d", timeOutMS);
    FFRTUtils::CancelTask(g_runningLockTimeoutHandle, g_queue);
    if (timeOutMS > 0) {
        FFRTTask task = [this, &remoteObj] {
            UnLock(remoteObj);
        };
        g_runningLockTimeoutHandle = FFRTUtils::SubmitDelayTask(task, timeOutMS, g_queue);
    }
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

void RunningLockMgr::Lock(const sptr<IRemoteObject>& remoteObj, int32_t timeOutMS)
{
    StartTrace(HITRACE_TAG_POWER, "RunningLock_Lock");

    auto lockInner = GetRunningLockInner(remoteObj);
    if (lockInner == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "LockInner is nullptr");
        return;
    }
    if (lockInner->IsProxied()) {
        POWER_HILOGW(FEATURE_RUNNING_LOCK, "Runninglock is proxied");
        return;
    }
    if (runninglockProxy_->IsProxied(lockInner->GetPid(), lockInner->GetUid())) {
        POWER_HILOGW(FEATURE_RUNNING_LOCK, "Runninglock is proxied, do not allow lock");
        return;
    }
    lockInner->SetTimeOutMs(timeOutMS);
    RunningLockParam lockInnerParam = lockInner->GetParam();
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "name=%{public}s, type=%{public}d, timeoutMs=%{public}d",
        lockInnerParam.name.c_str(), lockInnerParam.type, timeOutMS);
    if (IsSceneRunningLockType(lockInnerParam.type)) {
        runningLockAction_->Lock(lockInnerParam);
        NotifyRunningLockChanged(remoteObj, lockInner, NOTIFY_RUNNINGLOCK_ADD);
        return;
    }

    if (lockInner->GetState() == RunningLockState::RUNNINGLOCK_STATE_ENABLE) {
        POWER_HILOGD(FEATURE_RUNNING_LOCK, "Lock is already enabled");
        return;
    }
    auto iterator = lockCounters_.find(lockInnerParam.type);
    if (iterator == lockCounters_.end()) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Lock failed unsupported type, type=%{public}d", lockInnerParam.type);
        return;
    }
    lockInner->SetState(RunningLockState::RUNNINGLOCK_STATE_ENABLE);
    std::shared_ptr<LockCounter> counter = iterator->second;
    counter->Increase(remoteObj, lockInner);
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "LockCounter type=%{public}d, count=%{public}d", lockInnerParam.type,
        counter->GetCount());
    if (timeOutMS > 0) {
        RemoveAndPostUnlockTask(remoteObj, timeOutMS);
    }
    FinishTrace(HITRACE_TAG_POWER);
}

void RunningLockMgr::UnLock(const sptr<IRemoteObject> remoteObj)
{
    StartTrace(HITRACE_TAG_POWER, "RunningLock_Unlock");

    auto lockInner = GetRunningLockInner(remoteObj);
    if (lockInner == nullptr) {
        return;
    }
    if (lockInner->IsProxied()) {
        POWER_HILOGW(FEATURE_RUNNING_LOCK, "Runninglock is proxied");
        return;
    }
    auto lockInnerParam = lockInner->GetParam();
    POWER_HILOGD(
        FEATURE_RUNNING_LOCK, "name=%{public}s, type=%{public}d", lockInnerParam.name.c_str(), lockInnerParam.type);
    if (IsSceneRunningLockType(lockInnerParam.type)) {
        runningLockAction_->Unlock(lockInnerParam);
        NotifyRunningLockChanged(remoteObj, lockInner, NOTIFY_RUNNINGLOCK_REMOVE);
        return;
    }

    if (lockInner->GetState() == RunningLockState::RUNNINGLOCK_STATE_DISABLE) {
        POWER_HILOGD(FEATURE_RUNNING_LOCK, "Lock is not enabled, name=%{public}s", lockInner->GetName().c_str());
        return;
    }

    auto iterator = lockCounters_.find(lockInnerParam.type);
    if (iterator == lockCounters_.end()) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Unlock failed unsupported type, type=%{public}d",
            lockInnerParam.type);
        return;
    }
    RemoveAndPostUnlockTask(remoteObj);
    lockInner->SetState(RunningLockState::RUNNINGLOCK_STATE_DISABLE);
    std::shared_ptr<LockCounter> counter = iterator->second;
    counter->Decrease(remoteObj, lockInner);
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "LockCounter type=%{public}d, count=%{public}d", lockInnerParam.type,
        counter->GetCount());
    FinishTrace(HITRACE_TAG_POWER);
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

void RunningLockMgr::NotifyHiViewRunningLockInfo(const RunningLockInner& lockInner,
    RunningLockChangedType changeType) const
{
    NotifyHiView(changeType, lockInner);
}

void RunningLockMgr::CheckOverTime()
{
    FFRTUtils::CancelTask(g_runningLockTimeoutHandle, g_queue);
    if (runningLocks_.empty()) {
        return;
    }
    int64_t curTime = GetTickCount();
    int64_t detectTime = curTime - CHECK_TIMEOUT_INTERVAL_MS;
    POWER_HILOGI(FEATURE_RUNNING_LOCK, "curTime=%{public}" PRId64 " detectTime=%{public}" PRId64 "", curTime,
        detectTime);
    if (detectTime < 0) {
        return;
    }
    int64_t nextDetectTime = INT_MAX;
    for (auto& it : runningLocks_) {
        auto lockInner = it.second;
        if (lockInner->GetState() == RunningLockState::RUNNINGLOCK_STATE_ENABLE &&
            (!lockInner->GetOverTimeFlag())) {
            if (lockInner->GetLockTimeMs() < detectTime) {
                lockInner->SetOverTimeFlag(true);
                NotifyRunningLockChanged(it.first, lockInner, NOTIFY_RUNNINGLOCK_OVERTIME);
            } else {
                if (lockInner->GetLockTimeMs() < nextDetectTime) {
                    nextDetectTime = lockInner->GetLockTimeMs();
                }
            }
        }
    }
    if (nextDetectTime != INT_MAX) {
        detectTime = nextDetectTime - curTime + CHECK_TIMEOUT_INTERVAL_MS;
        SendCheckOverTimeMsg(detectTime);
    }
}

void RunningLockMgr::SendCheckOverTimeMsg(int64_t delayTime)
{
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "delayTime=%{public}" PRId64 "", delayTime);
    FFRTTask task = [this] {
        CheckOverTime();
    };
    FFRTUtils::SubmitDelayTask(task, delayTime, g_queue);
}

void RunningLockMgr::NotifyRunningLockChanged(const sptr<IRemoteObject>& remoteObj,
    std::shared_ptr<RunningLockInner>& lockInner, RunningLockChangedType changeType)
{
    if (changeType >= RUNNINGLOCK_CHANGED_BUTT) {
        return;
    }
    const string& remoteObjStr = to_string(reinterpret_cast<uintptr_t>(remoteObj.GetRefPtr()));
    switch (changeType) {
        case NOTIFY_RUNNINGLOCK_ADD: {
            NotifyHiViewRunningLockInfo(*lockInner, changeType);
            SendCheckOverTimeMsg(CHECK_TIMEOUT_INTERVAL_MS);
            break;
        }
        case NOTIFY_RUNNINGLOCK_REMOVE: {
            NotifyHiView(changeType, *lockInner);
            break;
        }
        case NOTIFY_RUNNINGLOCK_OVERTIME: {
            break;
        }
        default: {
            break;
        }
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
    for (auto& it : runningLocks_) {
        LockInnerByProxy(it.first, it.second);
    }
    runninglockProxy_->ResetRunningLocks();
}

void RunningLockMgr::LockInnerByProxy(const sptr<IRemoteObject>& remoteObj,
    std::shared_ptr<RunningLockInner>& lockInner)
{
    if (!lockInner->IsProxied()) {
        return;
    }
    RunningLockState lastState = lockInner->GetState();
    lockInner->SetState(RunningLockState::RUNNINGLOCK_STATE_DISABLE);
    if (lastState == RunningLockState::RUNNINGLOCK_STATE_UNPROXIED_RESTORE) {
        Lock(remoteObj, lockInner->GetTimeOutMs());
    }
}

void RunningLockMgr::UnlockInnerByProxy(const sptr<IRemoteObject>& remoteObj,
    std::shared_ptr<RunningLockInner>& lockInner)
{
    RunningLockState lastState = lockInner->GetState();
    lockInner->SetState(RunningLockState::RUNNINGLOCK_STATE_PROXIED);
    auto lockParam = lockInner->GetParam();
    if (IsSceneRunningLockType(lockInner->GetType()) &&
        runningLockAction_->Unlock(lockParam) == RUNNINGLOCK_SUCCESS) {
        lockInner->SetState(RunningLockState::RUNNINGLOCK_STATE_UNPROXIED_RESTORE);
        NotifyRunningLockChanged(remoteObj, lockInner, NOTIFY_RUNNINGLOCK_REMOVE);
        return;
    }
    if (lastState == RunningLockState::RUNNINGLOCK_STATE_DISABLE) {
        return;
    }
    auto counterIter = lockCounters_.find(lockParam.type);
    if (counterIter == lockCounters_.end()) {
        return;
    }
    lockInner->SetState(RunningLockState::RUNNINGLOCK_STATE_UNPROXIED_RESTORE);
    RemoveAndPostUnlockTask(remoteObj);
    counterIter->second->Decrease(remoteObj, lockInner);
}

void RunningLockMgr::NotifyHiView(RunningLockChangedType changeType, const RunningLockInner& lockInner) const
{
    int32_t pid = lockInner.GetPid();
    int32_t uid = lockInner.GetUid();
    int32_t state = static_cast<int32_t>(lockInner.GetState());
    int32_t type = static_cast <int32_t>(lockInner.GetType());
    string name = lockInner.GetName();
    const int logLevel = 2;
    const string &tag = runninglockNotifyStr_.at(changeType);
    HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::POWER, "RUNNINGLOCK",
        HiviewDFX::HiSysEvent::EventType::STATISTIC,
        "PID", pid, "UID", uid, "STATE", state, "TYPE", type, "NAME", name,
        "LOG_LEVEL", logLevel, "TAG", tag);
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "pid = %{public}d, uid= %{public}d, tag=%{public}s",
        pid, uid, tag.c_str());
}

void RunningLockMgr::EnableMock(IRunningLockAction* mockAction)
{
    // reset lock list
    runningLocks_.clear();
    for (auto it = lockCounters_.begin(); it != lockCounters_.end(); it++) {
        it->second->Clear();
    }
    runninglockProxy_->Clear();
    proximityController_.Clear();
    std::shared_ptr<IRunningLockAction> mock(mockAction);
    backgroundLock_->EnableMock(mock);
    runningLockAction_ = mock;
}

static const std::string GetRunningLockTypeString(RunningLockType type)
{
    switch (type) {
        case RunningLockType::RUNNINGLOCK_SCREEN:
            return "SCREEN";
        case RunningLockType::RUNNINGLOCK_BACKGROUND:
            return "BACKGROUND";
        case RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL:
            return "PROXIMITY_SCREEN_CONTROL";
        case RunningLockType::RUNNINGLOCK_BACKGROUND_PHONE:
            return "BACKGROUND_PHONE";
        case RunningLockType::RUNNINGLOCK_BACKGROUND_NOTIFICATION:
            return "BACKGROUND_NOTIFICATION";
        case RunningLockType::RUNNINGLOCK_BACKGROUND_AUDIO:
            return "BACKGROUND_AUDIO";
        case RunningLockType::RUNNINGLOCK_BACKGROUND_SPORT:
            return "BACKGROUND_SPORT";
        case RunningLockType::RUNNINGLOCK_BACKGROUND_NAVIGATION:
            return "BACKGROUND_NAVIGATION";
        case RunningLockType::RUNNINGLOCK_BACKGROUND_TASK:
            return "BACKGROUND_TASK";
        case RunningLockType::RUNNINGLOCK_BUTT:
            return "BUTT";
        default:
            break;
    }

    return "UNKNOWN";
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
            .append(GetRunningLockTypeString(it->first))
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
            .append(" type=").append(GetRunningLockTypeString(lockParam.type))
            .append(" name=").append(lockParam.name)
            .append(" uid=").append(ToString(lockInner->GetUid()))
            .append(" pid=").append(ToString(lockInner->GetPid()))
            .append(" state=").append(ToString(static_cast<uint32_t>(lockInner->GetState())))
            .append("\n");
    }

    result.append("Dump Proxy List: \n");
    result.append(runninglockProxy_->DumpProxyInfo());

    result.append("Peripherals Info: \n")
            .append("  Proximity: ")
            .append("Enabled=")
            .append(ToString(proximityController_.IsEnabled()))
            .append(" Status=")
            .append(ToString(proximityController_.GetStatus()))
            .append("\n");
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
    FFRTTask task = [&] {
        pms->ForceUnLock(remote.promote());
    };
    FFRTUtils::SubmitTask(task);
}

RunningLockMgr::SystemLock::SystemLock(std::shared_ptr<IRunningLockAction> action, RunningLockType type,
    const std::string& name) : action_(action), locking_(false)
{
    param_.type = type;
    param_.name = name;
}

void RunningLockMgr::SystemLock::Lock()
{
    if (!locking_) {
        action_->Lock(param_);
        locking_ = true;
    }
}

void RunningLockMgr::SystemLock::Unlock()
{
    if (locking_) {
        action_->Unlock(param_);
        locking_ = false;
    }
}

uint32_t RunningLockMgr::LockCounter::Increase(const sptr<IRemoteObject>& remoteObj,
    std::shared_ptr<RunningLockInner>& lockInner)
{
    ++counter_;
    if (counter_ == 1) {
        activate_(true);
    }
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        POWER_HILOGW(FEATURE_RUNNING_LOCK, "No power service instance");
        return counter_;
    }
    pms->GetRunningLockMgr()->NotifyRunningLockChanged(remoteObj, lockInner, NOTIFY_RUNNINGLOCK_ADD);
    return counter_;
}

uint32_t RunningLockMgr::LockCounter::Decrease(const sptr<IRemoteObject> remoteObj,
    std::shared_ptr<RunningLockInner>& lockInner)
{
    --counter_;
    if (counter_ == 0) {
        activate_(false);
    }
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        POWER_HILOGW(FEATURE_RUNNING_LOCK, "No power service instance");
        return counter_;
    }
    pms->GetRunningLockMgr()->NotifyRunningLockChanged(remoteObj, lockInner, NOTIFY_RUNNINGLOCK_REMOVE);
    return counter_;
}

void RunningLockMgr::LockCounter::Clear()
{
    counter_ = 0;
}

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
    if (stateMachine == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "state machine is nullptr");
        return;
    }
    isClose = true;
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "PROXIMITY is closed");
    auto runningLock = pms->GetRunningLockMgr();
    if (runningLock->GetValidRunningLockNum(
        RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL) > 0) {
        POWER_HILOGD(FEATURE_RUNNING_LOCK, "Change state to INACITVE when holding PROXIMITY LOCK");
#ifdef HAS_DISPLAY_MANAGER_PART
        Rosen::ScreenManager::GetInstance().SetScreenPowerForAll(Rosen::ScreenPowerState::POWER_OFF,
            Rosen::PowerStateChangeReason::POWER_BUTTON);
#endif
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
#ifdef HAS_DISPLAY_MANAGER_PART
        Rosen::ScreenManager::GetInstance().SetScreenPowerForAll(Rosen::ScreenPowerState::POWER_ON,
            Rosen::PowerStateChangeReason::POWER_BUTTON);
#endif
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
} // namespace PowerMgr
} // namespace OHOS
