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

#ifndef POWERMGR_POWER_MGR_SERVICE_ADAPTER_H
#define POWERMGR_POWER_MGR_SERVICE_ADAPTER_H

#include <iremote_object.h>
#include "ipower_mgr.h"
#include "power_mgr_stub.h"

namespace OHOS {
namespace PowerMgr {

class PowerMgrServiceAdapter : public PowerMgrStub {
public:
    virtual int32_t RebootDeviceIpc(const std::string& reason, int32_t& powerError) override;
    virtual int32_t RebootDeviceForDeprecatedIpc(const std::string& reason, int32_t& powerError) override;
    virtual int32_t ShutDownDeviceIpc(const std::string& reason, int32_t& powerError) override;
    virtual int32_t SetSuspendTagIpc(const std::string& tag, int32_t& powerError) override;
    virtual int32_t SuspendDeviceIpc(int64_t callTimeMs, int32_t reasonValue, bool suspendImmed,
        const std::string& apiVersion, int32_t& powerError) override;
    virtual int32_t WakeupDeviceIpc(int64_t callTimeMs, int32_t reasonValue, const std::string& details,
        const std::string& apiVersion, int32_t& powerError) override;
    virtual int32_t WakeupDeviceAsyncIpc(int64_t callTimeMs, int32_t reasonValue, const std::string& details,
        const std::string& apiVersion) override;
    virtual int32_t RefreshActivityIpc(int64_t callTimeMs, int32_t activityType, bool needChangeBacklight) override;
    virtual int32_t OverrideScreenOffTimeIpc(
        int64_t timeout, const std::string& apiVersion, int32_t& powerError) override;
    virtual int32_t RestoreScreenOffTimeIpc(const std::string& apiVersion, int32_t& powerError) override;
    virtual int32_t GetStateIpc(int32_t& powerState) override;
    virtual int32_t IsScreenOnIpc(bool needPrintLog, bool& isScreenOn) override;
    virtual int32_t IsFoldScreenOnIpc(bool& isFoldScreenOn) override;
    virtual int32_t IsCollaborationScreenOnIpc(bool& isCollaborationScreenOn) override;
    virtual int32_t IsForceSleepingIpc(bool& isForceSleeping) override;
    virtual int32_t ForceSuspendDeviceIpc(
        int64_t callTimeMs, const std::string& apiVersion, const sptr<IPowerMgrAsync>& powerProxy) override;
    virtual int32_t HibernateIpc(bool clearMemory, const std::string& reason,
        const std::string& apiVersion, const sptr<IPowerMgrAsync>& powerProxy) override;
    virtual int32_t CreateRunningLockIpc(
        const sptr<IRemoteObject>& remoteObj, const RunningLockInfo& runningLockInfo, int32_t& powerError) override;
    virtual int32_t ReleaseRunningLockIpc(const sptr<IRemoteObject>& remoteObj, const std::string& name = "") override;
    virtual int32_t IsRunningLockTypeSupportedIpc(int32_t lockType, bool& lockSupported) override;
    virtual int32_t UpdateWorkSourceIpc(const sptr<IRemoteObject>& remoteObj,
        const std::vector<int32_t>& workSources) override;
    virtual int32_t LockIpc(const sptr<IRemoteObject>& remoteObj, int32_t timeOutMs, int32_t& powerError) override;
    virtual int32_t UnLockIpc(
        const sptr<IRemoteObject>& remoteObj, const std::string& name, int32_t& powerError) override;
    virtual int32_t QueryRunningLockListsIpc(std::map<std::string, RunningLockInfo>& runningLockLists) override;
    virtual int32_t IsUsedIpc(const sptr<IRemoteObject>& remoteObj, bool& isUsed) override;
    virtual int32_t ProxyRunningLockIpc(bool isProxied, pid_t pid, pid_t uid) override;
    virtual int32_t ProxyRunningLocksIpc(bool isProxied, const VectorPair& vectorPairInfos) override;
    virtual int32_t ResetRunningLocksIpc() override;
    virtual int32_t RegisterPowerStateCallbackIpc(const sptr<IPowerStateCallback>& callback, bool isSync) override;
    virtual int32_t UnRegisterPowerStateCallbackIpc(const sptr<IPowerStateCallback>& callback) override;

    virtual int32_t RegisterSyncSleepCallbackIpc(
        const sptr<ISyncSleepCallback>& callback, int32_t priorityValue) override;
    virtual int32_t UnRegisterSyncSleepCallbackIpc(const sptr<ISyncSleepCallback>& callback) override;
    virtual int32_t RegisterSyncHibernateCallbackIpc(const sptr<ISyncHibernateCallback>& callback) override;
    virtual int32_t UnRegisterSyncHibernateCallbackIpc(const sptr<ISyncHibernateCallback>& callback) override;

    virtual int32_t RegisterPowerModeCallbackIpc(const sptr<IPowerModeCallback>& callback) override;
    virtual int32_t UnRegisterPowerModeCallbackIpc(const sptr<IPowerModeCallback>& callback) override;

    virtual int32_t RegisterScreenStateCallbackIpc(
        int32_t remainTime, const sptr<IScreenOffPreCallback>& callback) override;
    virtual int32_t UnRegisterScreenStateCallbackIpc(const sptr<IScreenOffPreCallback>& callback) override;

    virtual int32_t RegisterRunningLockCallbackIpc(const sptr<IPowerRunninglockCallback>& callback) override;
    virtual int32_t UnRegisterRunningLockCallbackIpc(const sptr<IPowerRunninglockCallback>& callback) override;
    virtual int32_t SetDisplaySuspendIpc(bool enable) override;
    virtual int32_t SetDeviceModeIpc(int32_t modeValue, int32_t& powerError) override;
    virtual int32_t GetDeviceModeIpc(int32_t& powerMode) override;
    virtual int32_t ShellDumpIpc(const std::vector<std::string>& args, uint32_t argc, std::string& returnDump) override;
    virtual int32_t IsStandbyIpc(bool& isStandby, int32_t& powerError) override;
    virtual int32_t SetForceTimingOutIpc(bool enabled, const sptr<IRemoteObject>& token, int32_t& powerError) override;
    virtual int32_t LockScreenAfterTimingOutIpc(bool enabledLockScreen, bool checkLock, bool sendScreenOffEvent,
        const sptr<IRemoteObject>& token, int32_t& powerError) override;
    virtual int32_t IsRunningLockEnabledIpc(int32_t lockType, bool& result, int32_t& powerError) override;

    int32_t RegisterShutdownCallbackIpc(
        const sptr<ITakeOverShutdownCallback>& callback, int32_t priorityValue) override;
    int32_t UnRegisterShutdownCallbackIpc(const sptr<ITakeOverShutdownCallback>& callback) override;

    int32_t RegisterShutdownCallbackIpc(const sptr<IAsyncShutdownCallback>& callback, int32_t priorityValue) override;
    int32_t UnRegisterShutdownCallbackIpc(const sptr<IAsyncShutdownCallback>& callback) override;
    int32_t RegisterShutdownCallbackIpc(const sptr<ISyncShutdownCallback>& callback, int32_t priorityValue) override;
    int32_t UnRegisterShutdownCallbackIpc(const sptr<ISyncShutdownCallback>& callback) override;

    virtual PowerErrors RebootDevice(const std::string& reason) = 0;
    virtual PowerErrors RebootDeviceForDeprecated(const std::string& reason) = 0;
    virtual PowerErrors ShutDownDevice(const std::string& reason) = 0;
    virtual PowerErrors SetSuspendTag(const std::string& tag) = 0;
    virtual PowerErrors SuspendDevice(int64_t callTimeMs, SuspendDeviceType reason,
        bool suspendImmed, const std::string& apiVersion = "-1") = 0;
    virtual PowerErrors WakeupDevice(int64_t callTimeMs, WakeupDeviceType reason,
        const std::string& details, const std::string& apiVersion = "-1") = 0;
    virtual void WakeupDeviceAsync(int64_t callTimeMs, WakeupDeviceType reason, const std::string& details) = 0;
    virtual bool RefreshActivity(int64_t callTimeMs, UserActivityType type, bool needChangeBacklight) = 0;
    virtual PowerErrors OverrideScreenOffTime(int64_t timeout, const std::string& apiVersion = "-1") = 0;
    virtual PowerErrors RestoreScreenOffTime(const std::string& apiVersion = "-1") = 0;
    virtual PowerState GetState() = 0;
    virtual bool IsScreenOn(bool needPrintLog = true) = 0;
    virtual bool IsFoldScreenOn() = 0;
    virtual bool IsCollaborationScreenOn() = 0;
    virtual bool IsForceSleeping() = 0;
    virtual PowerErrors ForceSuspendDevice(int64_t callTimeMs, const std::string& apiVersion = "-1") = 0;
    virtual PowerErrors Hibernate(
        bool clearMemory, const std::string& reason = "", const std::string& apiVersion = "-1") = 0;
    virtual PowerErrors CreateRunningLock(
        const sptr<IRemoteObject>& remoteObj, const RunningLockInfo& runningLockInfo) = 0;
    virtual bool ReleaseRunningLock(const sptr<IRemoteObject>& remoteObj, const std::string& name = "") = 0;
    virtual bool IsRunningLockTypeSupported(RunningLockType type) = 0;
    virtual bool UpdateWorkSource(const sptr<IRemoteObject>& remoteObj,
        const std::vector<int32_t>& workSources) = 0;
    virtual PowerErrors Lock(const sptr<IRemoteObject>& remoteObj, int32_t timeOutMs = -1) = 0;
    virtual PowerErrors UnLock(const sptr<IRemoteObject>& remoteObj, const std::string& name = "") = 0;
    virtual bool QueryRunningLockLists(std::map<std::string, RunningLockInfo>& runningLockLists) = 0;
    virtual bool IsUsed(const sptr<IRemoteObject>& remoteObj) = 0;
    virtual bool ProxyRunningLock(bool isProxied, pid_t pid, pid_t uid) = 0;
    virtual bool ProxyRunningLocks(bool isProxied,
        const std::vector<std::pair<pid_t, pid_t>>& processInfos) = 0;
    virtual bool ResetRunningLocks() = 0;
    virtual bool RegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback, bool isSync = true) = 0;
    virtual bool UnRegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback) = 0;

    virtual bool RegisterSyncSleepCallback(const sptr<ISyncSleepCallback>& callback, SleepPriority priority) = 0;
    virtual bool UnRegisterSyncSleepCallback(const sptr<ISyncSleepCallback>& callback) = 0;
    virtual bool RegisterSyncHibernateCallback(const sptr<ISyncHibernateCallback>& callback) = 0;
    virtual bool UnRegisterSyncHibernateCallback(const sptr<ISyncHibernateCallback>& callback) = 0;

    virtual bool RegisterPowerModeCallback(const sptr<IPowerModeCallback>& callback) = 0;
    virtual bool UnRegisterPowerModeCallback(const sptr<IPowerModeCallback>& callback) = 0;

    virtual bool RegisterScreenStateCallback(int32_t remainTime, const sptr<IScreenOffPreCallback>& callback) = 0;
    virtual bool UnRegisterScreenStateCallback(const sptr<IScreenOffPreCallback>& callback) = 0;

    virtual bool RegisterRunningLockCallback(const sptr<IPowerRunninglockCallback>& callback) = 0;
    virtual bool UnRegisterRunningLockCallback(const sptr<IPowerRunninglockCallback>& callback) = 0;
    virtual bool SetDisplaySuspend(bool enable) = 0;
    virtual PowerErrors SetDeviceMode(const PowerMode& mode) = 0;
    virtual PowerMode GetDeviceMode() = 0;
    virtual std::string ShellDump(const std::vector<std::string>& args, uint32_t argc) = 0;
    virtual PowerErrors IsStandby(bool& isStandby) = 0;
    virtual PowerErrors SetForceTimingOut(bool enabled, const sptr<IRemoteObject>& token) = 0;
    virtual PowerErrors LockScreenAfterTimingOut(
        bool enabledLockScreen, bool checkLock, bool sendScreenOffEvent, const sptr<IRemoteObject>& token) = 0;
    virtual PowerErrors IsRunningLockEnabled(const RunningLockType type, bool& result) = 0;

    virtual void RegisterShutdownCallback(
        const sptr<ITakeOverShutdownCallback>& callback, ShutdownPriority priority) = 0;
    virtual void UnRegisterShutdownCallback(const sptr<ITakeOverShutdownCallback>& callback) = 0;

    virtual void RegisterShutdownCallback(const sptr<IAsyncShutdownCallback>& callback, ShutdownPriority priority) = 0;
    virtual void UnRegisterShutdownCallback(const sptr<IAsyncShutdownCallback>& callback) = 0;
    virtual void RegisterShutdownCallback(const sptr<ISyncShutdownCallback>& callback, ShutdownPriority priority) = 0;
    virtual void UnRegisterShutdownCallback(const sptr<ISyncShutdownCallback>& callback) = 0;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_POWER_MGR_SERVICE_ADAPTER_H