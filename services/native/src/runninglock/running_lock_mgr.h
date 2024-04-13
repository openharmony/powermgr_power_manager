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

#ifndef POWERMGR_RUNNING_LOCK_MGR_H
#define POWERMGR_RUNNING_LOCK_MGR_H

#include <array>
#include <map>

#include <iremote_object.h>

#include "actions/irunning_lock_action.h"
#include "running_lock_inner.h"
#include "running_lock_proxy.h"
#include "running_lock_token_stub.h"
#include "running_lock_info.h"
#include "ipower_runninglock_callback.h"
#ifdef HAS_SENSORS_SENSOR_PART
#include "sensor_agent.h"
#endif

namespace OHOS {
namespace PowerMgr {
class PowerMgrService;
class PowerStateMachine;
using RunningLockMap = std::map<const sptr<IRemoteObject>, std::shared_ptr<RunningLockInner>>;

class RunningLockMgr {
public:
    enum {
        PROXIMITY_AWAY = 0,
        PROXIMITY_CLOSE
    };
    explicit RunningLockMgr(const wptr<PowerMgrService>& pms) : pms_(pms) {}
    ~RunningLockMgr();

    std::shared_ptr<RunningLockInner> CreateRunningLock(const sptr<IRemoteObject>& remoteObj,
        const RunningLockParam& runningLockParam);
    bool ReleaseLock(const sptr<IRemoteObject> remoteObj);
    bool Lock(const sptr<IRemoteObject>& remoteObj);
    bool UnLock(const sptr<IRemoteObject> remoteObj);
    void RegisterRunningLockCallback(const sptr<IPowerRunninglockCallback>& callback);
    void UnRegisterRunningLockCallback(const sptr<IPowerRunninglockCallback>& callback);
    void QueryRunningLockLists(std::map<std::string, RunningLockInfo>& runningLockLists);
    uint32_t GetRunningLockNum(RunningLockType type = RunningLockType::RUNNINGLOCK_BUTT);
    uint32_t GetValidRunningLockNum(RunningLockType type = RunningLockType::RUNNINGLOCK_BUTT);
    bool Init();
    bool ExistValidRunningLock();
    std::shared_ptr<RunningLockInner> GetRunningLockInner(const sptr<IRemoteObject>& remoteObj);
    const RunningLockMap& GetRunningLockMap() const
    {
        return runningLocks_;
    }
    bool ProxyRunningLock(bool isProxied, pid_t pid, pid_t uid);
    void ProxyRunningLocks(bool isProxied, const std::vector<std::pair<pid_t, pid_t>>& processInfos);
    void ResetRunningLocks();
    bool IsUsed(const sptr<IRemoteObject>& remoteObj);
    static constexpr uint32_t CHECK_TIMEOUT_INTERVAL_MS = 60 * 1000;
    void SetProximity(uint32_t status);
    void DumpInfo(std::string& result);
    void EnableMock(IRunningLockAction* mockAction);
private:

    void InitLocksTypeScreen();
    void InitLocksTypeBackground();
    void InitLocksTypeProximity();
    void InitLocksTypeCoordination();
    void ProxyRunningLockInner(bool isProxied, pid_t pid, pid_t uid);

    class LockCounter {
    public:
        LockCounter(RunningLockType type, std::function<int32_t(bool, RunningLockParam)> activate)
            : type_(type), activate_(activate), counter_(0) {}
        ~LockCounter() = default;
        int32_t Increase(const RunningLockParam& lockInnerParam);
        int32_t Decrease(const RunningLockParam& lockInnerParam);
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
        std::function<int32_t(bool, RunningLockParam)> activate_;
        uint32_t counter_;
    };

#ifdef HAS_SENSORS_SENSOR_PART
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
    ProximityController proximityController_;
#endif

    class RunningLockDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        RunningLockDeathRecipient() = default;
        virtual void OnRemoteDied(const wptr<IRemoteObject>& remote);
        virtual ~RunningLockDeathRecipient() = default;
    };
    bool InitLocks();
    void LockInnerByProxy(const sptr<IRemoteObject>& remoteObj, std::shared_ptr<RunningLockInner>& lockInner);
    void UnlockInnerByProxy(const sptr<IRemoteObject>& remoteObj, std::shared_ptr<RunningLockInner>& lockInner);
    static bool IsSceneRunningLockType(RunningLockType type);
    bool IsValidType(RunningLockType type);
    void PreprocessBeforeAwake();
    void ProximityLockOn();
    const wptr<PowerMgrService> pms_;
    std::mutex mutex_;
    RunningLockMap runningLocks_;
    std::map<RunningLockType, std::shared_ptr<LockCounter>> lockCounters_;
    std::shared_ptr<RunningLockProxy> runninglockProxy_;
    sptr<IRemoteObject::DeathRecipient> runningLockDeathRecipient_;
    std::shared_ptr<IRunningLockAction> runningLockAction_;
    static void NotifyRunningLockChanged(const RunningLockParam& lockInnerParam, const std::string &tag);
    RunningLockInfo FillAppRunningLockInfo(const RunningLockParam& info);
    void UpdateUnSceneLockLists(RunningLockParam& singleLockParam, bool fill);
    std::map<std::string, RunningLockInfo> unSceneLockLists_;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_RUNNING_LOCK_MGR_H
