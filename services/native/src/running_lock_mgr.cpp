/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include "power_log.h"
#include "power_mgr_factory.h"
#include "power_mgr_service.h"
#ifdef HAS_DISPLAY_MANAGER_PART
#include "screen_manager.h"
#endif

using namespace std;

namespace OHOS {
namespace PowerMgr {
namespace {
const string TASK_RUNNINGLOCK_FORCEUNLOCK = "RunningLock_ForceUnLock";
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
    auto pmsptr = pms_.promote();
    if (pmsptr == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Power manager service is null");
        return false;
    }
    handler_ = pmsptr->GetHandler();

    systemLocks_.emplace(SystemLockType::SYSTEM_LOCK_APP,
        std::make_shared<SystemLock>(runningLockAction_, LOCK_TAG_APP));
    systemLocks_.emplace(SystemLockType::SYSTEM_LOCK_DISPLAY,
        std::make_shared<SystemLock>(runningLockAction_, LOCK_TAG_DISPLAY));

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
                stateMachine->SetState(PowerState::AWAKE,
                    StateChangeReason::STATE_CHANGE_REASON_RUNNING_LOCK);
                stateMachine->CancelDelayTimer(
                    PowermsEventHandler::CHECK_USER_ACTIVITY_TIMEOUT_MSG);
                stateMachine->CancelDelayTimer(
                    PowermsEventHandler::CHECK_USER_ACTIVITY_OFF_TIMEOUT_MSG);
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
            auto iterator = systemLocks_.find(SystemLockType::SYSTEM_LOCK_APP);
            if (iterator == systemLocks_.end()) {
                return;
            }
            std::shared_ptr<SystemLock> pSysLock = iterator->second;
            if (active) {
                POWER_HILOGI(FEATURE_RUNNING_LOCK, "RUNNINGLOCK_BACKGROUND active");
                stateMachine->CancelDelayTimer(
                    PowermsEventHandler::CHECK_USER_ACTIVITY_SLEEP_TIMEOUT_MSG);
                pSysLock->Lock();
            } else {
                POWER_HILOGI(FEATURE_RUNNING_LOCK, "RUNNINGLOCK_BACKGROUND inactive");
                if (stateMachine->GetState() == PowerState::INACTIVE) {
                    stateMachine->ResetSleepTimer();
                } else {
                    POWER_HILOGD(FEATURE_RUNNING_LOCK, "Background unlock in state: %{public}d",
                        stateMachine->GetState());
                }
                pSysLock->Unlock();
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
            auto iterator = systemLocks_.find(SystemLockType::SYSTEM_LOCK_APP);
            if (iterator == systemLocks_.end()) {
                return;
            }
            std::shared_ptr<SystemLock> pSysLock = iterator->second;
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
                    PowermsEventHandler::CHECK_USER_ACTIVITY_TIMEOUT_MSG);
                stateMachine->CancelDelayTimer(
                    PowermsEventHandler::CHECK_USER_ACTIVITY_OFF_TIMEOUT_MSG);
                pSysLock->Lock();
            } else {
                POWER_HILOGI(FEATURE_RUNNING_LOCK, "RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL inactive");
                stateMachine->SetState(PowerState::AWAKE,
                    StateChangeReason::STATE_CHANGE_REASON_RUNNING_LOCK);
                stateMachine->ResetInactiveTimer();
                pSysLock->Unlock();
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

std::shared_ptr<RunningLockInner> RunningLockMgr::CreateRunningLock(
    const sptr<IRemoteObject>& remoteObj,
    const RunningLockInfo& runningLockInfo,
    const UserIPCInfo& userIPCinfo)
{
    auto lockInner = RunningLockInner::CreateRunningLockInner(runningLockInfo, userIPCinfo);
    if (lockInner == nullptr) {
        return nullptr;
    }
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "Create lock success, name=%{public}s, type=%{public}d",
        runningLockInfo.name.c_str(), runningLockInfo.type);

    mutex_.lock();
    runningLocks_.emplace(remoteObj, lockInner);
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
    if (!lockInner->GetDisabled()) {
        UnLock(remoteObj);
    }
    mutex_.lock();
    runningLocks_.erase(remoteObj);
    mutex_.unlock();
    result = remoteObj->RemoveDeathRecipient(runningLockDeathRecipient_);
    return result;
}

void RunningLockMgr::RemoveAndPostUnlockTask(
    const sptr<IRemoteObject>& remoteObj, uint32_t timeOutMS)
{
    auto handler = handler_.lock();
    if (handler == nullptr) {
        POWER_HILOGI(FEATURE_RUNNING_LOCK, "Handler is nullptr");
        return;
    }
    const string& remoteObjStr = to_string(reinterpret_cast<uintptr_t>(remoteObj.GetRefPtr()));
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "timeOutMS=%{public}d", timeOutMS);
    handler->RemoveTask(remoteObjStr);
    if (timeOutMS != 0) {
        std::function<void()> unLockFunc = std::bind(&RunningLockMgr::UnLock, this,  remoteObj);
        handler->PostTask(unLockFunc, remoteObjStr, timeOutMS);
    }
}

void RunningLockMgr::Lock(const sptr<IRemoteObject>& remoteObj,
    const RunningLockInfo& runningLockInfo,
    const UserIPCInfo& userIPCinfo, uint32_t timeOutMS)
{
    StartTrace(HITRACE_TAG_POWER, "RunningLock_Lock");
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "name=%{public}s, type=%{public}d",
        runningLockInfo.name.c_str(), runningLockInfo.type);

    auto lockInner = GetRunningLockInner(remoteObj);
    if (lockInner == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "LockInner is nullptr");
        return;
    }
    if (!lockInner->GetDisabled()) {
        POWER_HILOGD(FEATURE_RUNNING_LOCK, "Lock is already enabled");
        return;
    }

    auto iterator = lockCounters_.find(lockInner->GetRunningLockType());
    if (iterator == lockCounters_.end()) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Lock failed unsupported type, type=%{public}d",
            lockInner->GetRunningLockType());
        return;
    }
    std::shared_ptr<LockCounter> counter = iterator->second;
    counter->Increase(remoteObj, lockInner);
    lockInner->SetDisabled(false);
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "LockCounter type=%{public}d, count=%{public}d", lockInner->GetRunningLockType(),
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
    POWER_HILOGI(FEATURE_RUNNING_LOCK, "LockInner name=%{public}s, type=%{public}d",
        lockInner->GetRunningLockName().c_str(), lockInner->GetRunningLockType());
    if (lockInner->GetDisabled()) {
        POWER_HILOGD(FEATURE_RUNNING_LOCK, "Lock is already disabled name=%{public}s",
            lockInner->GetRunningLockName().c_str());
        return;
    }
    RemoveAndPostUnlockTask(remoteObj);

    auto iterator = lockCounters_.find(lockInner->GetRunningLockType());
    if (iterator == lockCounters_.end()) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Unlock failed unsupported type, type=%{public}d",
            lockInner->GetRunningLockType());
        return;
    }
    std::shared_ptr<LockCounter> counter = iterator->second;
    counter->Decrease(remoteObj, lockInner);
    lockInner->SetDisabled(true);
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "LockCounter type=%{public}d, count=%{public}d", lockInner->GetRunningLockType(),
        counter->GetCount());
    FinishTrace(HITRACE_TAG_POWER);
}

bool RunningLockMgr::IsUsed(const sptr<IRemoteObject>& remoteObj)
{
    auto lockInner = GetRunningLockInner(remoteObj);
    if (lockInner == nullptr || lockInner->GetDisabled()) {
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
            return pair.second->GetRunningLockType() == type;
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
        if (lockinner->GetDisabled() == false) {
            return true;
        }
    }
    return false;
}

void RunningLockMgr::SetWorkTriggerList(const sptr<IRemoteObject>& remoteObj,
    const WorkTriggerList& workTriggerList)
{
    auto lockInner = GetRunningLockInner(remoteObj);
    if (lockInner == nullptr) {
        return;
    }
    lockInner->SetWorkTriggerList(workTriggerList);
    NotifyRunningLockChanged(remoteObj, lockInner, NOTIFY_RUNNINGLOCK_WORKTRIGGER_CHANGED);
    // After update triggerlist, maybe need disabled the lock or enable the lock.
    SetRunningLockDisableFlag(lockInner);
    // If enabled, we really lock here.
    LockReally(remoteObj, lockInner);
    // If disabled, we really unlock here.
    UnLockReally(remoteObj, lockInner);
}

void RunningLockMgr::NotifyHiViewRunningLockInfo(const RunningLockInner& lockInner,
    RunningLockChangedType changeType) const
{
    NotifyHiView(changeType, lockInner);
}

void RunningLockMgr::CheckOverTime()
{
    auto handler = handler_.lock();
    if (handler == nullptr) {
        return;
    }
    handler->RemoveEvent(PowermsEventHandler::CHECK_RUNNINGLOCK_OVERTIME_MSG);
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
        if (!lockInner->GetDisabled() && (!lockInner->GetOverTimeFlag())) {
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
    auto handler = handler_.lock();
    if (handler == nullptr) {
        return;
    }
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "delayTime=%{public}" PRId64 "", delayTime);
    handler->SendEvent(PowermsEventHandler::CHECK_RUNNINGLOCK_OVERTIME_MSG, 0, delayTime);
}

void RunningLockMgr::NotifyRunningLockChanged(const sptr<IRemoteObject>& remoteObj,
    std::shared_ptr<RunningLockInner>& lockInner, RunningLockChangedType changeType)
{
    if (changeType >= RUNNINGLOCK_CHANGED_BUTT) {
        return;
    }
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
        case NOTIFY_RUNNINGLOCK_WORKTRIGGER_CHANGED: {
            NotifyHiViewRunningLockInfo(*lockInner, changeType);
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
bool RunningLockMgr::MatchProxyMap(const UserIPCInfo& userIPCinfo)
{
    if (proxyMap_.empty()) {
        POWER_HILOGW(FEATURE_RUNNING_LOCK, "ProxyMap_ is empty, uid = %{public}d, pid = %{public}d",
            userIPCinfo.uid, userIPCinfo.pid);
        return false;
    }
    auto it = proxyMap_.find(userIPCinfo.uid);
    // 1. Find pidset by uid.
    if (it == proxyMap_.end()) {
        POWER_HILOGW(FEATURE_RUNNING_LOCK, "Pid set not match, uid = %{public}d, pid = %{public}d",
            userIPCinfo.uid, userIPCinfo.pid);
        return false;
    }
    auto& pidset = it->second;
    // 2. Count by owner pid.
    if (pidset.count(userIPCinfo.pid) > 0) {
        POWER_HILOGD(FEATURE_RUNNING_LOCK, "Pid set match and count > 0, uid = %{public}d, pid = %{public}d",
            userIPCinfo.uid, userIPCinfo.pid);
        return true;
    }
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "Pid set match and count(-1) = %{public}d, uid = %{public}d, pid = %{public}d",
        static_cast<unsigned int>(pidset.count(INVALID_PID)), userIPCinfo.uid, userIPCinfo.pid);
    // 3. Count by INVALID_PID, return true when proxy (uid, -1).
    return (pidset.count(INVALID_PID) > 0);
}

void RunningLockMgr::SetRunningLockDisableFlag(
    std::shared_ptr<RunningLockInner>& lockInner,
    bool forceRefresh)
{
    if (proxyMap_.empty() && (!forceRefresh)) {
        /**
         * Generally when proxymap empty keep the default value false.
         * When PGManager cancel proxy uid and pid,
         * because we update the proxy map before, so we should refresh
         * all of the runninglock disable flag always.
         */
        POWER_HILOGD(FEATURE_RUNNING_LOCK, "Set lock enable, proxyMap_ is empty and forceRefresh is false");
        lockInner->SetDisabled(false);
        return;
    }
    const UserIPCInfo& userIPCinfo = lockInner->GetUserIPCInfo();
    bool matched = MatchProxyMap(userIPCinfo);
    if (matched) {
        // Matched the lock owner useripcinfo, set disabled directly.
        lockInner->SetDisabled(true);
        POWER_HILOGD(FEATURE_RUNNING_LOCK, "Lock and ipc matched, uid = %{public}d, pid = %{public}d",
            userIPCinfo.uid, userIPCinfo.pid);
        return;
    }
    const RunningLockInfo& runningLockInfo = lockInner->GetRunningLockInfo();
    const WorkTriggerList& list = runningLockInfo.workTriggerlist;
    if (list.empty()) {
        // Not matched, and no trigger list.
        lockInner->SetDisabled(false);
        POWER_HILOGD(FEATURE_RUNNING_LOCK, "Work trigger list is empty");
        return;
    }
    bool triggerMatched = true;
    // Have trigger list, need to judge whether or not all of the list matched,
    // otherwise we should hold the lock.
    for (auto& workTrigger : list) {
        UserIPCInfo triggerIPCInfo {workTrigger->GetUid(), workTrigger->GetPid()};
        if (!MatchProxyMap(triggerIPCInfo)) {
            triggerMatched = false;
            POWER_HILOGD(FEATURE_RUNNING_LOCK, "WorkTrigger not matched uid = %{public}d, pid = %{public}d",
                triggerIPCInfo.uid, triggerIPCInfo.pid);
            break;
        }
    }
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "TriggerMatched = %{public}d, uid = %{public}d, pid = %{public}d",
        userIPCinfo.uid, userIPCinfo.pid, triggerMatched);
    lockInner->SetDisabled(triggerMatched);
}

void RunningLockMgr::LockReally(const sptr<IRemoteObject>& remoteObj,
    std::shared_ptr<RunningLockInner>& lockInner)
{
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "Start");
    if (lockInner->GetReallyLocked()) {
        POWER_HILOGD(FEATURE_RUNNING_LOCK, "lockInner->GetReallyLocked() is true, return");
        return;
    }
    if (lockInner->GetDisabled()) {
        POWER_HILOGD(FEATURE_RUNNING_LOCK, "lockInner->GetDisabled() is true, return");
        return;
    }
    runningLockAction_->Acquire(lockInner->GetRunningLockType());
    lockInner->SetReallyLocked(true);
    NotifyRunningLockChanged(remoteObj, lockInner, NOTIFY_RUNNINGLOCK_ADD);
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "Finish");
}
void RunningLockMgr::UnLockReally(const sptr<IRemoteObject>& remoteObj,
    std::shared_ptr<RunningLockInner>& lockInner)
{
    /**
     * Case 1: Firstly PGManager ProxyRunningLock by uid and pid,
     * secondly application lock by the same uid and
     * pid, when call Lock() we matched the proxymap, and no really locked to kernel.
     * So we don't need to unlocked to kernel.
     * Case 2: Firstly application create the runninglock and call Lock(),
     * and then PGManager Proxyed it,
     * At this time, lock should be unlocked to kernel because we have locked to kernel for it.
     */
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "Start");
    if (!lockInner->GetReallyLocked()) {
        POWER_HILOGD(FEATURE_RUNNING_LOCK, "lockInner->GetReallyLocked() is false, return");
        return;
    }
    // If disabled, unlock to the kernel.
    if (!lockInner->GetDisabled()) {
        POWER_HILOGD(FEATURE_RUNNING_LOCK, "lockInner->GetDisabled() is false, return");
        return;
    }
    runningLockAction_->Release(lockInner->GetRunningLockType());
    lockInner->SetReallyLocked(false);
    NotifyRunningLockChanged(remoteObj, lockInner, NOTIFY_RUNNINGLOCK_REMOVE);
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "Finish");
}

void RunningLockMgr::ProxyRunningLockInner(bool proxyLock)
{
    if (proxyLock) {
        for (auto& it : runningLocks_) {
            SetRunningLockDisableFlag(it.second);
            UnLockReally(it.first, it.second);
        }
    } else {
        for (auto& it : runningLocks_) {
            SetRunningLockDisableFlag(it.second, true);
            LockReally(it.first, it.second);
        }
    }
}

void RunningLockMgr::ProxyRunningLock(bool proxyLock, pid_t uid, pid_t pid)
{
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "proxyLock = %{public}d, uid = %{public}d, pid = %{public}d",
        proxyLock, uid, pid);
    auto it = proxyMap_.find(uid);
    if (proxyLock) {
        // PGManager insert proxy info.
        if (it == proxyMap_.end()) {
            unordered_set<pid_t> pidset({pid});
            proxyMap_.emplace(uid, pidset);
            POWER_HILOGD(FEATURE_RUNNING_LOCK, "Emplace first proxyMap_ {uid = %{public}d, pid = %{public}d}",
                uid, pid);
        } else {
            if (pid == INVALID_PID) {
                // Insert uid, pid = -1,remove other pid
                proxyMap_.erase(uid);
                unordered_set<pid_t> pidset({pid});
                proxyMap_.emplace(uid, pidset);
                POWER_HILOGD(FEATURE_RUNNING_LOCK, "Re-emplace proxyMap_ {uid = %{public}d, pid = %{public}d}",
                    uid, pid);
            } else {
                auto& pidset = it->second;
                pidset.insert(pid);
                POWER_HILOGD(FEATURE_RUNNING_LOCK, "Insert proxyMap_ {uid = %{public}d, pid = %{public}d}", uid, pid);
            }
        }
        // 1. Set the matched runninglock inner disabled flag.
    } else {
        if (it == proxyMap_.end()) {
            // No insert proxy info, nothing to erase.
            POWER_HILOGD(FEATURE_RUNNING_LOCK, "No uid in proxyMap_, uid = %{public}d", uid);
            return;
        }
        // 1. Clear the runninglock inner disabled flag 2.removed from proxyMap_
        if (pid == INVALID_PID) {
            proxyMap_.erase(uid);
            POWER_HILOGD(FEATURE_RUNNING_LOCK, "pid = -1, erase from proxyMap_, uid = %{public}d", uid);
        } else {
            auto& pidset = it->second;
            pidset.erase(pid);
            POWER_HILOGD(FEATURE_RUNNING_LOCK, "Erase from proxyMap_, uid = %{public}d, pid = %{public}d", uid, pid);
            if (pidset.size() == 0) {
                proxyMap_.erase(uid);
                POWER_HILOGD(FEATURE_RUNNING_LOCK, "Pidset is empty erase uid from proxyMap_, uid = %{public}d", uid);
            }
        }
    }
    ProxyRunningLockInner(proxyLock);
}

void RunningLockMgr::NotifyHiView(RunningLockChangedType changeType, const RunningLockInner& lockInner) const
{
    const UserIPCInfo& ipcInfo = lockInner.GetUserIPCInfo();
    int32_t pid = ipcInfo.pid;
    int32_t uid = ipcInfo.uid;
    bool state = lockInner.GetDisabled();
    int32_t type = static_cast <int32_t>(lockInner.GetRunningLockType());
    string name = lockInner.GetRunningLockName();
    const int logLevel = 2;
    const string &tag = runninglockNotifyStr_.at(changeType);
    HiviewDFX::HiSysEvent::Write("POWER", "POWER_RUNNINGLOCK",
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
    proximityController_.Clear();

    std::shared_ptr<IRunningLockAction> mock(mockAction);
    for (auto it = systemLocks_.begin(); it != systemLocks_.end(); it++) {
        it->second->EnableMock(mock);
    }
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
    for (auto& it : runningLocks_) {
        index++;
        auto lockInner = it.second;
        if (lockInner == nullptr) {
            return;
        }
        auto& lockinfo = lockInner->GetRunningLockInfo();
        auto& ipcinfo = lockInner->GetUserIPCInfo();
        result.append("  index=").append(ToString(index))
            .append(" time=").append(ToString(curTick - lockInner->GetLockTimeMs()))
            .append(" type=").append(GetRunningLockTypeString(lockinfo.type))
            .append(" name=").append(lockinfo.name)
            .append(" uid=").append(ToString(ipcinfo.uid))
            .append(" pid=").append(ToString(ipcinfo.pid))
            .append(" disable=").append(ToString(lockInner->GetDisabled()))
            .append("\n");
    }

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
    if (remote.promote() == nullptr) {
        POWER_HILOGW(FEATURE_RUNNING_LOCK, "Remote is nullptr");
        return;
    }
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        POWER_HILOGW(FEATURE_RUNNING_LOCK, "Power service is nullptr");
        return;
    }
    auto handler = pms->GetHandler();
    if (handler == nullptr) {
        POWER_HILOGW(FEATURE_RUNNING_LOCK, "Handler is nullptr");
        return;
    }
    std::function<void()> forceUnLockFunc = std::bind(&PowerMgrService::ForceUnLock, pms,
        remote.promote());
    handler->PostTask(forceUnLockFunc, TASK_RUNNINGLOCK_FORCEUNLOCK);
}

void RunningLockMgr::SystemLock::Lock()
{
    if (!locking_) {
        action_->Lock(RunningLockType::RUNNINGLOCK_BUTT, tag_.c_str());
        locking_ = true;
    }
}

void RunningLockMgr::SystemLock::Unlock()
{
    if (locking_) {
        action_->Unlock(RunningLockType::RUNNINGLOCK_BUTT, tag_.c_str());
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
    ProximityData* data = (ProximityData*)event->data;
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
