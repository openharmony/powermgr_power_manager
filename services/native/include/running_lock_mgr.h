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
#include "sensor_agent.h"

namespace OHOS {
namespace PowerMgr {
class PowerMgrService;
class PowermsEventHandler;
class PowerStateMachine;
using RunningLockMap = std::map<const sptr<IRemoteObject>, std::shared_ptr<RunningLockInner>>;

class RunningLockMgr {
public:
    enum class SystemLockType : uint32_t {
        SYSTEM_LOCK_APP = 0,
        SYSTEM_LOCK_DISPLAY = 1,
        SYSTEM_LOCK_OTHER
    };
    enum {
        PROXIMITY_AWAY = 0,
        PROXIMITY_CLOSE
    };
    using RunningLockProxyMap = std::unordered_map<pid_t, std::unordered_set<pid_t>>;
    explicit RunningLockMgr(const wptr<PowerMgrService>& pms) : pms_(pms) {}
    ~RunningLockMgr();

    void Lock(const sptr<IRemoteObject>& remoteObj, const RunningLockInfo& runningLockInfo,
        const UserIPCInfo &userIPCinfo, uint32_t timeOutMS = 0);
    void UnLock(const sptr<IRemoteObject> remoteObj);
    std::shared_ptr<RunningLockInner> CreateRunningLock(const sptr<IRemoteObject>& remoteObj,
        const RunningLockInfo& runningLockInfo, const UserIPCInfo &userIPCinfo);
    bool ReleaseLock(const sptr<IRemoteObject> remoteObj);
    uint32_t GetRunningLockNum(RunningLockType type = RunningLockType::RUNNINGLOCK_BUTT);
    uint32_t GetValidRunningLockNum(RunningLockType type = RunningLockType::RUNNINGLOCK_BUTT);
    void SetWorkTriggerList(const sptr<IRemoteObject>& remoteObj, const WorkTriggerList& workTriggerList);
    bool Init();
    bool ExistValidRunningLock();
    std::shared_ptr<RunningLockInner> GetRunningLockInner(const sptr<IRemoteObject>& remoteObj);
    const RunningLockMap& GetRunningLockMap() const
    {
        return runningLocks_;
    }
    const RunningLockProxyMap& GetRunningLockProxyMap() const
    {
        return proxyMap_;
    }
    void ProxyRunningLock(bool proxyLock, pid_t uid, pid_t pid);
    bool IsUsed(const sptr<IRemoteObject>& remoteObj);
    static constexpr uint32_t CHECK_TIMEOUT_INTERVAL_MS = 60 * 1000;
    void CheckOverTime();
    void SetProximity(uint32_t status);
    void DumpInfo(std::string& result);
    void EnableMock(IRunningLockAction* mockAction);
private:
    static constexpr const char * const LOCK_TAG_APP = "lock_app";
    static constexpr const char * const LOCK_TAG_DISPLAY = "lock_display";
    static constexpr const char * const LOCK_TAG_OTHER = "lock_other";

    void InitLocksTypeScreen();
    void InitLocksTypeBackground();
    void InitLocksTypeProximity();

    class SystemLock {
    public:
        SystemLock(std::shared_ptr<IRunningLockAction> action, const char * const tag)
            : action_(action), tag_(tag), locking_(false) {};
        ~SystemLock() = default;
        void Lock();
        void Unlock();
        bool IsLocking()
        {
            return locking_;
        };
        void EnableMock(std::shared_ptr<IRunningLockAction>& mock)
        {
            locking_ = false;
            action_ = mock;
        }
    private:
        std::shared_ptr<IRunningLockAction> action_;
        const std::string tag_;
        bool locking_;
    };

    class LockCounter {
    public:
        LockCounter(RunningLockType type, std::function<void(bool)> activate)
            : type_(type), activate_(activate), counter_(0) {}
        ~LockCounter() = default;
        uint32_t Increase(const sptr<IRemoteObject>& remoteObj,
            std::shared_ptr<RunningLockInner>& lockInner);
        uint32_t Decrease(const sptr<IRemoteObject> remoteObj,
            std::shared_ptr<RunningLockInner>& lockInner);
        void Clear();
        uint32_t GetCount()
        {
            return counter_;
        }
        RunningLockType GetType()
        {
            return type_;
        }
    private:
        const RunningLockType type_;
        std::shared_ptr<IRunningLockAction> action_;
        std::function<void(bool)> activate_;
        uint32_t counter_;
    };

    class ProximityController {
    public:
        ProximityController();
        ~ProximityController();
        void Enable();
        void Disable();
        bool IsEnabled()
        {
            return enabled_;
        }
        bool IsSupported()
        {
            return support_;
        }
        bool IsClose();
        void OnClose();
        void OnAway();
        uint32_t GetStatus()
        {
            return status_;
        }
        void Clear();
        static void RecordSensorCallback(SensorEvent *event);
    private:
        static const int32_t PROXIMITY_CLOSE_SCALAR = 0;
        static const int32_t PROXIMITY_AWAY_SCALAR = 5;
        static const uint32_t SAMPLING_RATE =  100000000;
        bool support_ {false};
        bool enabled_ {false};
        bool isClose {false};
        uint32_t status_ {0};
        SensorUser user_;
    };

    class RunningLockDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        RunningLockDeathRecipient() = default;
        virtual void OnRemoteDied(const wptr<IRemoteObject>& remote);
        virtual ~RunningLockDeathRecipient() = default;
    };
    bool InitLocks();
    bool MatchProxyMap(const UserIPCInfo& userIPCinfo);
    void SetRunningLockDisableFlag(std::shared_ptr<RunningLockInner>& lockInner, bool forceRefresh = false);
    void LockReally(const sptr<IRemoteObject>& remoteObj, std::shared_ptr<RunningLockInner>& lockInner);
    void UnLockReally(const sptr<IRemoteObject>& remoteObj, std::shared_ptr<RunningLockInner>& lockInner);
    void ProxyRunningLockInner(bool proxyLock);
    void RemoveAndPostUnlockTask(const sptr<IRemoteObject>& remoteObj, uint32_t timeOutMS = 0);
    const wptr<PowerMgrService> pms_;
    ProximityController proximityController_;
    std::weak_ptr<PowermsEventHandler> handler_;
    std::mutex mutex_;
    RunningLockMap runningLocks_;
    std::map<SystemLockType, std::shared_ptr<SystemLock>> systemLocks_;
    std::map<RunningLockType, std::shared_ptr<LockCounter>> lockCounters_;
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
    void NotifyRunningLockChanged(const sptr<IRemoteObject>& remoteObj, std::shared_ptr<RunningLockInner>& lockInner,
        RunningLockChangedType changeType);
    void SendCheckOverTimeMsg(int64_t delayTime);
    void NotifyHiViewRunningLockInfo(const RunningLockInner& lockInner, RunningLockChangedType changeType) const;
    void NotifyHiView(RunningLockChangedType changeType, const RunningLockInner& lockInner) const;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_RUNNING_LOCK_MGR_H
