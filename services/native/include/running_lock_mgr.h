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

#ifndef POWERMGR_RUNNING_LOCK_MGR_H
#define POWERMGR_RUNNING_LOCK_MGR_H

#include <array>
#include <map>
#include <unordered_map>
#include <unordered_set>

#include <iremote_object.h>

#include "actions/irunning_lock_action.h"
#include "running_lock_inner.h"
#include "running_lock_token_stub.h"

namespace OHOS {
namespace PowerMgr {
class PowerMgrService;
class PowermsEventHandler;
using RunningLockMap = std::map<const sptr<IRemoteObject>, std::shared_ptr<RunningLockInner>>;

class RunningLockMgr {
public:
    using RunningLockProxyMap = std::unordered_map<pid_t, std::unordered_set<pid_t>>;
    explicit RunningLockMgr(const wptr<PowerMgrService>& pms) : pms_(pms) {}
    ~RunningLockMgr();

    void Lock(const sptr<IRemoteObject>& token, const RunningLockInfo& runningLockInfo,
        const UserIPCInfo &userIPCinfo, uint32_t timeOutMS = 0);
    void UnLock(const sptr<IRemoteObject> token);
    uint32_t GetRunningLockNum(RunningLockType type = RunningLockType::RUNNINGLOCK_BUTT);
    uint32_t GetValidRunningLockNum(RunningLockType type = RunningLockType::RUNNINGLOCK_BUTT);
    void SetWorkTriggerList(const sptr<IRemoteObject>& token, const WorkTriggerList& workTriggerList);
    bool Init();
    bool ExistValidRunningLock();
    std::shared_ptr<RunningLockInner> GetRunningLockInner(const sptr<IRemoteObject>& token);
    const RunningLockMap& GetRunningLockMap() const
    {
        return runningLocks_;
    }
    const RunningLockProxyMap& GetRunningLockProxyMap() const
    {
        return proxyMap_;
    }
    void ProxyRunningLock(bool proxyLock, pid_t uid, pid_t pid);
    bool IsUsed(const sptr<IRemoteObject>& token);
    static constexpr uint32_t CHECK_TIMEOUT_INTERVAL_MS = 60 * 1000;
    static constexpr uint32_t MAX_DUMP_NUM = 10;
    void CheckOverTime();
    void DumpInfo(std::string& result);

private:
    class RunningLockDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        RunningLockDeathRecipient() = default;
        virtual void OnRemoteDied(const wptr<IRemoteObject>& remote);
        virtual ~RunningLockDeathRecipient() = default;
    };
    bool MatchProxyMap(const UserIPCInfo& userIPCinfo);
    void SetRunningLockDisableFlag(std::shared_ptr<RunningLockInner>& lockInner, bool forceRefresh = false);
    void LockReally(const sptr<IRemoteObject>& token, std::shared_ptr<RunningLockInner>& lockInner);
    void UnLockReally(const sptr<IRemoteObject>& token, std::shared_ptr<RunningLockInner>& lockInner);
    void ProxyRunningLockInner(bool proxyLock);
    void RemoveAndPostUnlockTask(const sptr<IRemoteObject>& token, uint32_t timeOutMS = 0);
    std::shared_ptr<RunningLockInner> CreateRunningLockInner(const sptr<IRemoteObject>& token,
        const RunningLockInfo& runningLockInfo, const UserIPCInfo &userIPCinfo);
    const wptr<PowerMgrService> pms_;
    std::weak_ptr<PowermsEventHandler> handler_;
    std::mutex mutex_;
    RunningLockMap runningLocks_;
    RunningLockProxyMap proxyMap_;
    sptr<IRemoteObject::DeathRecipient> runningLockDeathRecipient_;
    std::shared_ptr<IRunningLockAction> runningLockAction_;
    enum RunningLockChangedType {
        NOTIFY_RUNNINGLOCK_ADD,
        NOTIFY_RUNNINGLOCK_REMOVE,
        NOTIFY_RUNNINGLOCK_WORKTRIGGER_CHANGED,
        NOTIFY_RUNNINGLOCK_OVERTIME,
        RUNNINGLOCK_CHANGED_BUTT
    };
    const std::array<std::string, RUNNINGLOCK_CHANGED_BUTT> runninglockNotifyStr_ {
        "DUBAI_TAG_RUNNINGLOCK_ADD", "DUBAI_TAG_RUNNINGLOCK_REMOVE",
        "DUBAI_TAG_RUNNINGLOCK_WORKTRIGGER_CHANGED", "DUBAI_TAG_RUNNINGLOCK_OVERTIME"
    };
    void NotifyRunningLockChanged(const sptr<IRemoteObject>& token, std::shared_ptr<RunningLockInner>& lockInner,
        RunningLockChangedType changeType);
    void SendCheckOverTimeMsg(int64_t delayTime);
    void NotifyHiViewRunningLockInfo(const std::string& tokenstr, const RunningLockInner& lockInner,
        RunningLockChangedType changeType) const;
    void NotifyHiView(RunningLockChangedType changeType, const std::string& msg) const;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_RUNNING_LOCK_MGR_H
