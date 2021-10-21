/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include <string_ex.h>

#include "power_common.h"
#include "power_mgr_factory.h"
#include "power_mgr_service.h"

using namespace std;

namespace OHOS {
namespace PowerMgr {
namespace {
const string TASK_RUNNINGLOCK_FORCEUNLOCK = "RunningLock_ForceUnLock";
}

RunningLockMgr::~RunningLockMgr() {}

bool RunningLockMgr::Init()
{
    POWER_HILOGI(MODULE_SERVICE, "RunningLockMgr::Init start");
    std::lock_guard<std::mutex> lock(mutex_);
    if (runningLockDeathRecipient_ == nullptr) {
        runningLockDeathRecipient_ = new RunningLockDeathRecipient();
    }
    if (runningLockAction_ == nullptr) {
        runningLockAction_ = PowerMgrFactory::GetRunningLockAction();
        if (!runningLockAction_) {
            POWER_HILOGE(MODULE_SERVICE,
                "RunningLockMgr::Init create RunningLockMgr fail");
            return false;
        }
    }
    auto pmsptr = pms_.promote();
    if (pmsptr == nullptr) {
        return false;
    }
    handler_ = pmsptr->GetHandler();

    systemLocks_.emplace(SystemLockType::SYSTEM_LOCK_APP,
        std::make_shared<SystemLock>(runningLockAction_, LOCK_TAG_APP));
    systemLocks_.emplace(SystemLockType::SYSTEM_LOCK_DISPLAY,
        std::make_shared<SystemLock>(runningLockAction_, LOCK_TAG_DISPLAY));

    bool ret = InitLocks();

    POWER_HILOGI(MODULE_SERVICE, "RunningLockMgr::Init success");
    return ret;
}

bool RunningLockMgr::InitLocks()
{
    InitLocksTypeScreen();
    InitLocksTypeBackground();
    nitLocksTypeProximity();
    return true;
}

void RunningLockMgr::InitLocksTypeScreen()
{
    lockCounters_.emplace(RunningLockType::RUNNINGLOCK_SCREEN,
        std::make_shared<LockCounter>(
        RunningLockType::RUNNINGLOCK_SCREEN, [this](bool active) {
            POWER_HILOGI(MODULE_SERVICE, "RUNNINGLOCK_SCREEN action start!");
            auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
            if (pms == nullptr) {
                return;
            }
            auto stateMachine = pms->GetPowerStateMachine();
            if (stateMachine == nullptr) {
                return;
            }
            if (active) {
                POWER_HILOGI(MODULE_SERVICE, "RUNNINGLOCK_SCREEN active!");
                stateMachine->SetState(PowerState::AWAKE,
                    StateChangeReason::STATE_CHANGE_REASON_RUNNING_LOCK);
                stateMachine->CancelDelayTimer(
                    PowermsEventHandler::CHECK_USER_ACTIVITY_TIMEOUT_MSG);
                stateMachine->CancelDelayTimer(
                    PowermsEventHandler::CHECK_USER_ACTIVITY_OFF_TIMEOUT_MSG);
            } else {
                POWER_HILOGI(MODULE_SERVICE, "RUNNINGLOCK_SCREEN inactive!");
                if (stateMachine->GetState() == PowerState::AWAKE) {
                    stateMachine->ResetInactiveTimer();
                } else {
                    POWER_HILOGD(MODULE_SERVICE,
                        "Screen On unlock in state: %{public}d",
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
            POWER_HILOGI(MODULE_SERVICE, "RUNNINGLOCK_BACKGROUND action start!");
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
                POWER_HILOGI(MODULE_SERVICE, "RUNNINGLOCK_BACKGROUND active!");
                stateMachine->CancelDelayTimer(
                    PowermsEventHandler::CHECK_USER_ACTIVITY_SLEEP_TIMEOUT_MSG);
                pSysLock->Lock();
            } else {
                POWER_HILOGI(MODULE_SERVICE, "RUNNINGLOCK_BACKGROUND inactive!");
                if (stateMachine->GetState() == PowerState::INACTIVE) {
                    stateMachine->ResetSleepTimer();
                } else {
                    POWER_HILOGD(MODULE_SERVICE,
                        "Background unlock in state: %{public}d",
                        stateMachine->GetState());
                }
                pSysLock->Unlock();
            }
        })
    );
}

void RunningLockMgr::nitLocksTypeProximity()
{
    lockCounters_.emplace(RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL,
        std::make_shared<LockCounter>(
        RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL, [this](bool active) {
            POWER_HILOGI(MODULE_SERVICE,
                "RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL action start!");
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
                POWER_HILOGI(MODULE_SERVICE,
                    "RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL active!");
                proximityController_.Enable();
                if (proximityController_.IsClose()) {
                    POWER_HILOGI(MODULE_SERVICE, "INACTIVE when close");
                    stateMachine->SetState(PowerState::INACTIVE,
                        StateChangeReason::STATE_CHANGE_REASON_RUNNING_LOCK, true);
                } else {
                    POWER_HILOGI(MODULE_SERVICE, "AWAKE when away");
                    stateMachine->SetState(PowerState::AWAKE,
                        StateChangeReason::STATE_CHANGE_REASON_RUNNING_LOCK, true);
                }
                stateMachine->CancelDelayTimer(
                    PowermsEventHandler::CHECK_USER_ACTIVITY_TIMEOUT_MSG);
                stateMachine->CancelDelayTimer(
                    PowermsEventHandler::CHECK_USER_ACTIVITY_OFF_TIMEOUT_MSG);
                pSysLock->Lock();
            } else {
                POWER_HILOGI(MODULE_SERVICE,
                    "RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL inactive!");
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
    const sptr<IRemoteObject>& token)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto iterator = runningLocks_.find(token);
    if (iterator != runningLocks_.end()) {
        POWER_HILOGD(MODULE_SERVICE, "RunningLockMgr::%{public}s :find by token.", __func__);
        return  iterator->second;
    }
    return nullptr;
}

std::shared_ptr<RunningLockInner> RunningLockMgr::CreateRunningLock(
    const sptr<IRemoteObject>& token,
    const RunningLockInfo& runningLockInfo,
    const UserIPCInfo& userIPCinfo)
{
    auto lockInner = RunningLockInner::CreateRunningLockInner(runningLockInfo, userIPCinfo);
    if (lockInner == nullptr) {
        return nullptr;
    }
    POWER_HILOGD(MODULE_SERVICE,
        "RunningLockMgr::%{public}s : ok,name = %{public}s,type = %{public}d",
        __func__,
        runningLockInfo.name.c_str(),
        runningLockInfo.type);

    mutex_.lock();
    runningLocks_.emplace(token, lockInner);
    mutex_.unlock();
    token->AddDeathRecipient(runningLockDeathRecipient_);
    return lockInner;
}

void RunningLockMgr::ReleaseLock(const sptr<IRemoteObject> token)
{
    POWER_HILOGI(MODULE_SERVICE, "RunningLockMgr::%{public}s :token = %{public}p",
        __func__, token.GetRefPtr());
    auto lockInner = GetRunningLockInner(token);
    if (lockInner == nullptr) {
        return;
    }
    if (!lockInner->GetDisabled()) {
        UnLock(token);
    }
    mutex_.lock();
    runningLocks_.erase(token);
    mutex_.unlock();
    token->RemoveDeathRecipient(runningLockDeathRecipient_);
}

void RunningLockMgr::RemoveAndPostUnlockTask(
    const sptr<IRemoteObject>& token, uint32_t timeOutMS)
{
    auto handler = handler_.lock();
    if (handler == nullptr) {
        return;
    }
    const string& tokenStr = to_string(reinterpret_cast<uintptr_t>(token.GetRefPtr()));
    POWER_HILOGI(MODULE_SERVICE,
        "RunningLockMgr::%{public}s :token = %p,tokenStr = %s,timeOutMS = %d",
        __func__,
        token.GetRefPtr(),
        tokenStr.c_str(),
        timeOutMS);
    handler->RemoveTask(tokenStr);
    if (timeOutMS != 0) {
        std::function<void()> unLockFunc = std::bind(&RunningLockMgr::UnLock, this,  token);
        handler->PostTask(unLockFunc, tokenStr, timeOutMS);
    }
}

void RunningLockMgr::Lock(const sptr<IRemoteObject>& token,
    const RunningLockInfo& runningLockInfo,
    const UserIPCInfo& userIPCinfo, uint32_t timeOutMS)
{
    POWER_HILOGI(MODULE_SERVICE,
        "RunningLockMgr::%{public}s :token = %p,name = %s,type = %d",
        __func__,
        token.GetRefPtr(),
        runningLockInfo.name.c_str(),
        runningLockInfo.type);

    auto lockInner = GetRunningLockInner(token);
    if (lockInner == nullptr) {
        POWER_HILOGD(MODULE_SERVICE, "RunningLockMgr::Lock failed, can't find %{public}p",
            token.GetRefPtr());
        return;
    }
    if (!lockInner->GetDisabled()) {
        POWER_HILOGD(MODULE_SERVICE, "RunningLockMgr::Lock, this lock(%s) is already on",
            lockInner->GetRunningLockName().c_str());
        return;
    }
    lockInner->SetDisabled(false);

    auto iterator = lockCounters_.find(lockInner->GetRunningLockType());
    if (iterator == lockCounters_.end()) {
        POWER_HILOGD(MODULE_SERVICE,
            "RunningLockMgr::Lock failed, unsupported type %{public}d",
            lockInner->GetRunningLockType());
        return;
    }
    std::shared_ptr<LockCounter> counter = iterator->second;
    counter->Increase();
    POWER_HILOGD(MODULE_SERVICE,
        "LockCounter: %{public}d, %{public}d",
        lockInner->GetRunningLockType(),
        counter->GetCount());
    if (timeOutMS > 0) {
        RemoveAndPostUnlockTask(token, timeOutMS);
    }
}

void RunningLockMgr::UnLock(const sptr<IRemoteObject> token)
{
    POWER_HILOGI(MODULE_SERVICE, "RunningLockMgr::%{public}s :token = %p",
        __func__, token.GetRefPtr());

    auto lockInner = GetRunningLockInner(token);
    if (lockInner == nullptr) {
        return;
    }
    if (lockInner->GetDisabled()) {
        POWER_HILOGD(MODULE_SERVICE,
            "RunningLockMgr::Unlock, this lock(%{public}s) is already off",
            lockInner->GetRunningLockName().c_str());
        return;
    }
    lockInner->SetDisabled(true);
    RemoveAndPostUnlockTask(token);

    auto iterator = lockCounters_.find(lockInner->GetRunningLockType());
    if (iterator == lockCounters_.end()) {
        POWER_HILOGD(MODULE_SERVICE,
            "RunningLockMgr::Unlock failed, unsupported type %{public}d",
            lockInner->GetRunningLockType());
        return;
    }
    std::shared_ptr<LockCounter> counter = iterator->second;
    counter->Decrease();
}

bool RunningLockMgr::IsUsed(const sptr<IRemoteObject>& token)
{
    auto lockInner = GetRunningLockInner(token);
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
        POWER_HILOGD(MODULE_SERVICE,
            "GetValidRunningLockNum failed, unsupported type %d", type);
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

void RunningLockMgr::SetWorkTriggerList(const sptr<IRemoteObject>& token,
    const WorkTriggerList& workTriggerList)
{
    POWER_HILOGI(MODULE_SERVICE, "RunningLockMgr::%{public}s :token = %p",
        __func__,
        token.GetRefPtr());

    auto lockInner = GetRunningLockInner(token);
    if (lockInner == nullptr) {
        return;
    }
    lockInner->SetWorkTriggerList(workTriggerList);
    NotifyRunningLockChanged(token, lockInner, NOTIFY_RUNNINGLOCK_WORKTRIGGER_CHANGED);
    // After update triggerlist, maybe need disabled the lock or enable the lock.
    SetRunningLockDisableFlag(lockInner);
    // If enabled, we really lock here.
    LockReally(token, lockInner);
    // If disabled, we really unlock here.
    UnLockReally(token, lockInner);
}

void RunningLockMgr::NotifyHiViewRunningLockInfo(const string& tokenStr,
    const RunningLockInner& lockInner,
    RunningLockChangedType changeType) const
{
    string lockName = lockInner.GetRunningLockName().empty()
        ? "NULL" : lockInner.GetRunningLockName();
    string msg = "token=" + tokenStr + " lockName=" + lockName + " type=" +
        to_string(ToUnderlying(lockInner.GetRunningLockType()));
    auto MakeNotifyInfo = [](int uid, int pid, const string& tag) {
        return (" uid=" + to_string(uid) + " pid=" + to_string(pid) + " tag=" + tag);
    };

    auto& list = lockInner.GetRunningLockInfo().workTriggerlist;
    if (list.empty()) {
        const UserIPCInfo& ipcInfo = lockInner.GetUserIPCInfo();
        msg += MakeNotifyInfo(ipcInfo.uid, ipcInfo.pid, lockName);
    } else {
        for (auto& worker : list) {
            string workName = worker->GetName().empty() ? "NULL" : worker->GetName();
            msg += MakeNotifyInfo(worker->GetUid(), worker->GetPid(), workName);
        }
    }
    NotifyHiView(changeType, msg);
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
    POWER_HILOGI(MODULE_SERVICE,
        "RunningLockMgr::%{public}s :cur = %" PRId64 " detectTime=%" PRId64 "",
        __func__,
        curTime,
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
    POWER_HILOGI(MODULE_SERVICE, "RunningLockMgr::%{public}s ,delay = %" PRId64 "",
        __func__,
        delayTime);
    handler->SendEvent(PowermsEventHandler::CHECK_RUNNINGLOCK_OVERTIME_MSG, 0, delayTime);
}

void RunningLockMgr::NotifyRunningLockChanged(const sptr<IRemoteObject>& token,
    std::shared_ptr<RunningLockInner>& lockInner, RunningLockChangedType changeType)
{
    if (changeType >= RUNNINGLOCK_CHANGED_BUTT) {
        return;
    }
    const string& tokenStr = to_string(reinterpret_cast<uintptr_t>(token.GetRefPtr()));
    switch (changeType) {
        case NOTIFY_RUNNINGLOCK_ADD: {
            NotifyHiViewRunningLockInfo(tokenStr, *lockInner, changeType);
            SendCheckOverTimeMsg(CHECK_TIMEOUT_INTERVAL_MS);
            break;
        }
        case NOTIFY_RUNNINGLOCK_REMOVE: {
            string str = "token=" + tokenStr;
            NotifyHiView(changeType, str);
            break;
        }
        case NOTIFY_RUNNINGLOCK_WORKTRIGGER_CHANGED: {
            NotifyHiViewRunningLockInfo(tokenStr, *lockInner, changeType);
            break;
        }
        case NOTIFY_RUNNINGLOCK_OVERTIME: {
            POWER_HILOGI(MODULE_SERVICE, "RunningLockMgr::%{public}s :%s token=%s",
                __func__,
                runninglockNotifyStr_.at(changeType).c_str(),
                tokenStr.c_str());
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
        POWER_HILOGD(MODULE_SERVICE,
            "%{public}s ret false by proxyMap_.empty(), useripcinfo uid = %d pid = %d.",
            __func__,
            userIPCinfo.uid,
            userIPCinfo.pid);
        return false;
    }
    auto it = proxyMap_.find(userIPCinfo.uid);
    // 1. Find pidset by uid.
    if (it == proxyMap_.end()) {
        POWER_HILOGD(MODULE_SERVICE,
            "%{public}s not find uidmap, useripcinfo uid = %d pid = %d.",
            __func__,
            userIPCinfo.uid,
            userIPCinfo.pid);
        return false;
    }
    auto& pidset = it->second;
    // 2. Count by owner pid.
    if (pidset.count(userIPCinfo.pid) > 0) {
        POWER_HILOGD(MODULE_SERVICE,
            "%{public}s find uidmap and count pid > 1, useripcinfo uid = %d pid = %d.",
            __func__,
            userIPCinfo.uid,
            userIPCinfo.pid);
        return true;
    }
    POWER_HILOGD(MODULE_SERVICE,
        "%{public}s find uidmap and count pid = 0,count(-1) = %d, useripcinfo uid = %d "
        "pid = %d.", __func__,
        static_cast<unsigned int>(pidset.count(INVALID_PID)),
        userIPCinfo.uid,
        userIPCinfo.pid);
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
        POWER_HILOGD(MODULE_SERVICE,
            "%{public}s ret false by proxyMap_.empty() and forceRefresh = false", __func__);
        lockInner->SetDisabled(false);
        return;
    }
    const UserIPCInfo& userIPCinfo = lockInner->GetUserIPCInfo();
    bool matched = MatchProxyMap(userIPCinfo);
    if (matched) {
        // Matched the lock owner useripcinfo, set disabled directly.
        lockInner->SetDisabled(true);
        POWER_HILOGD(MODULE_SERVICE,
            "%{public}s MatchProxyMap matched by useripcinfo uid = %d pid = %d.",
            __func__, userIPCinfo.uid, userIPCinfo.pid);
        return;
    }
    const RunningLockInfo& runningLockInfo = lockInner->GetRunningLockInfo();
    const WorkTriggerList& list = runningLockInfo.workTriggerlist;
    if (list.empty()) {
        // Not matched, and no trigger list.
        lockInner->SetDisabled(false);
        POWER_HILOGD(MODULE_SERVICE,
            "%{public}s useripcinfo not matched and list is empty().", __func__);
        return;
    }
    bool triggerMatched = true;
    // Have trigger list, need to judge whether or not all of the list matched,
    // otherwise we should hold the lock.
    for (auto& workTrigger : list) {
        UserIPCInfo triggerIPCInfo {workTrigger->GetUid(), workTrigger->GetPid()};
        if (!MatchProxyMap(triggerIPCInfo)) {
            triggerMatched = false;
            POWER_HILOGD(MODULE_SERVICE,
                "%{public}s workTrigger not matched uid = %d, pid = %d.",
                __func__,
                triggerIPCInfo.uid,
                triggerIPCInfo.pid);
            break;
        }
    }
    lockInner->SetDisabled(triggerMatched);
    POWER_HILOGD(MODULE_SERVICE,
        "%{public}s useripcinfo uid = %d pid = %d, triggerMatched = %d.",
        __func__,
        userIPCinfo.uid,
        userIPCinfo.pid,
        triggerMatched);
}

void RunningLockMgr::LockReally(const sptr<IRemoteObject>& token,
    std::shared_ptr<RunningLockInner>& lockInner)
{
    if (lockInner->GetReallyLocked()) {
        POWER_HILOGD(MODULE_SERVICE,
            "%{public}s :return by lockInner->GetReallyLocked() == true.", __func__);
        return;
    }
    if (lockInner->GetDisabled()) {
        POWER_HILOGD(MODULE_SERVICE,
            "%{public}s :return by lockInner->GetDisabled() == true.", __func__);
        return;
    }
    runningLockAction_->Acquire(lockInner->GetRunningLockType());
    lockInner->SetReallyLocked(true);
    NotifyRunningLockChanged(token, lockInner, NOTIFY_RUNNINGLOCK_ADD);
    POWER_HILOGD(MODULE_SERVICE, "%{public}s :called end.", __func__);
}
void RunningLockMgr::UnLockReally(const sptr<IRemoteObject>& token,
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
    if (!lockInner->GetReallyLocked()) {
        POWER_HILOGD(MODULE_SERVICE,
            "%{public}s :return by lockInner->GetReallyLocked() == false.", __func__);
        return;
    }
    // If disabled, unlock to the kernel.
    if (!lockInner->GetDisabled()) {
        POWER_HILOGD(MODULE_SERVICE,
            "%{public}s :return by lockInner->GetDisabled() == false.", __func__);
        return;
    }
    runningLockAction_->Release(lockInner->GetRunningLockType());
    lockInner->SetReallyLocked(false);
    NotifyRunningLockChanged(token, lockInner, NOTIFY_RUNNINGLOCK_REMOVE);
    POWER_HILOGD(MODULE_SERVICE, "%{public}s :called end.", __func__);
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
    POWER_HILOGD(MODULE_SERVICE,
        "%{public}s :proxyLock = %d, uid = %d, pid = %d", __func__,
        proxyLock, uid, pid);
    auto it = proxyMap_.find(uid);
    if (proxyLock) {
        // PGManager insert proxy info.
        if (it == proxyMap_.end()) {
            unordered_set<pid_t> pidset({pid});
            proxyMap_.emplace(uid, pidset);
            POWER_HILOGD(MODULE_SERVICE,
                "%{public}s :proxyLock = true first emplace {uid = %d, pid = %d}",
                __func__, uid, pid);
        } else {
            if (pid == INVALID_PID) {
                // Insert uid, pid = -1,remove other pid
                proxyMap_.erase(uid);
                unordered_set<pid_t> pidset({pid});
                proxyMap_.emplace(uid, pidset);
            } else {
                auto& pidset = it->second;
                pidset.insert(pid);
            }
            POWER_HILOGD(MODULE_SERVICE,
                "%{public}s :proxyLock = true uid = %d exist insert pid = %d",
                __func__, uid, pid);
        }
        // 1. Set the matched runninglock inner disabled flag.
    } else {
        if (it == proxyMap_.end()) {
            // No insert proxy info, nothing to erase.
            POWER_HILOGD(MODULE_SERVICE,
                "%{public}s :proxyLock = false not find by uid = %d",
                __func__, uid);
            return;
        }
        // 1. Clear the runninglock inner disabled flag 2.removed from proxyMap_
        if (pid == INVALID_PID) {
            proxyMap_.erase(uid);
            POWER_HILOGD(MODULE_SERVICE,
                "%{public}s :proxyLock = false pid = -1 rmv uid = %d map",
                __func__, uid);
        } else {
            auto& pidset = it->second;
            pidset.erase(pid);
            POWER_HILOGD(MODULE_SERVICE,
                "%{public}s :proxyLock = false uid = %d erase single pid = %d",
                __func__, uid, pid);
            if (pidset.size() == 0) {
                proxyMap_.erase(uid);
                POWER_HILOGD(MODULE_SERVICE,
                    "%{public}s :pidset.size()=0 erase uid keymap", __func__);
            }
        }
    }
    ProxyRunningLockInner(proxyLock);
}

void RunningLockMgr::NotifyHiView(RunningLockChangedType changeType,
    const std::string& msg) const
{
    if (msg.empty()) {
        return;
    }
    const int logLevel = 2;
    const string &tag = runninglockNotifyStr_.at(changeType);
    HiviewDFX::HiSysEvent::Write(HiviewDFX::HiSysEvent::Domain::POWERMGR, "Lock",
        HiviewDFX::HiSysEvent::EventType::FAULT,
        "LOG_LEVEL",
        logLevel,
        "TAG",
        tag,
        "MESSAGE",
        msg);
    POWER_HILOGI(MODULE_SERVICE, "RunningLockMgr::%{public}s: %s %s",
        __func__,
        tag.c_str(),
        msg.c_str());
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

static const char* GetRunningLockTypeString(RunningLockType type)
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

    result.append("POWER MANAGER DUMP (hidumper -runninglock):\n");
    result.append("  totalSize=").append(ToString(runningLocks_.size()))
            .append(" validSize=").append(ToString(validSize)).append("\n");
    result.append("Summary By Type: \n");
    for (auto it = lockCounters_.begin(); it != lockCounters_.end(); it++) {
        result.append(GetRunningLockTypeString(it->first))
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
        if (index++ == MAX_DUMP_NUM) {
            break;
        }
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
            .append("Proximity: ")
            .append("Enabled: ")
            .append(ToString(proximityController_.IsEnabled()))
            .append("Status: ")
            .append(ToString(proximityController_.GetStatus()))
            .append("\n");
}

void RunningLockMgr::RunningLockDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    if (remote.promote() == nullptr) {
        return;
    }
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        return;
    }
    auto handler = pms->GetHandler();
    if (handler == nullptr) {
        return;
    }
    std::function<void()> forceUnLockFunc = std::bind(&PowerMgrService::ForceUnLock, pms,
        remote.promote());
    POWER_HILOGI(MODULE_SERVICE,
        "RunningLockDeathRecipient::%{public}s :remote.promote() = %p",
        __func__,
        remote.promote().GetRefPtr());
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

uint32_t RunningLockMgr::LockCounter::Increase()
{
    ++counter_;
    if (counter_ == 1) {
        activate_(true);
    }
    return counter_;
}

uint32_t RunningLockMgr::LockCounter::Decrease()
{
    --counter_;
    if (counter_ == 0) {
        activate_(false);
    }
    return counter_;
}

void RunningLockMgr::LockCounter::Clear()
{
    counter_ = 0;
}

RunningLockMgr::ProximityController::ProximityController()
    : enabled_(false), status_(0)
{
}

RunningLockMgr::ProximityController::~ProximityController()
{
}

void RunningLockMgr::ProximityController::Enable()
{
    enabled_ = true;
}

void RunningLockMgr::ProximityController::Disable()
{
    enabled_ = false;
}

bool RunningLockMgr::ProximityController::IsClose()
{
    POWER_HILOGD(MODULE_SERVICE, "IsClose:%{public}d", isClose);
    return isClose;
}

void RunningLockMgr::ProximityController::OnClose()
{
    if (!enabled_) {
        return;
    }
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        return;
    }
    auto stateMachine = pms->GetPowerStateMachine();
    if (stateMachine == nullptr) {
        return;
    }
    isClose = true;
    auto runningLock = pms->GetRunningLockMgr();
    if (runningLock->GetValidRunningLockNum(
        RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL) > 0) {
        POWER_HILOGD(MODULE_SERVICE, "Change to INACITVE when come close");
        stateMachine->SetState(PowerState::INACTIVE,
            StateChangeReason::STATE_CHANGE_REASON_SENSOR,
            true);
    }
}

void RunningLockMgr::ProximityController::OnAway()
{
    if (!enabled_) {
        return;
    }
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        return;
    }
    auto stateMachine = pms->GetPowerStateMachine();
    if (stateMachine == nullptr) {
        return;
    }
    isClose = false;
    auto runningLock = pms->GetRunningLockMgr();
    if (runningLock->GetValidRunningLockNum(
        RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL) > 0) {
        POWER_HILOGD(MODULE_SERVICE, "Change to AWAKE when go away");
        stateMachine->SetState(PowerState::AWAKE,
            StateChangeReason::STATE_CHANGE_REASON_SENSOR,
            true);
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
