/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#ifndef POWERMGR_SERVICE_TEST_PROXY_H
#define POWERMGR_SERVICE_TEST_PROXY_H

#include <cstdint>
#include <sys/types.h>
#include <functional>
#include <iosfwd>
#include <vector>

#include "hilog/log.h"
#include "iremote_object.h"
#include "ipower_mode_callback.h"
#include "ipower_state_callback.h"
#include "ipower_runninglock_callback.h"
#include "ipower_mgr.h"
#include "iscreen_off_pre_callback.h"
#include "running_lock_info.h"
#include "power_errors.h"
#include "power_mgr_service.h"
#include "power_mgr_proxy.h"
#include "power_state_machine_info.h"

namespace OHOS {
namespace PowerMgr {
// Intentionally hide the virtual functions in base class if they differ. Make sure to disable compiler errors
// Only invoke base class method if tests fail due to invalidated mock method to minimize the changes
// Actually we don't need this class here at all.
class PowerMgrServiceTestProxy : public PowerMgrProxy {
public:
    PowerMgrServiceTestProxy(const sptr<PowerMgrService>& service);
    ~PowerMgrServiceTestProxy() = default;

    int32_t CreateRunningLockIpc(
        const sptr<IRemoteObject>& remoteObj, const RunningLockInfo& runningLockInfo, int32_t& powerError);
    int32_t ReleaseRunningLockIpc(const sptr<IRemoteObject>& remoteObj, const std::string& name = "");
    int32_t IsRunningLockTypeSupportedIpc(int32_t lockType, bool& lockSupported);
    int32_t LockIpc(const sptr<IRemoteObject>& remoteObj, int32_t timeOutMs, int32_t& powerError);
    int32_t UnLockIpc(const sptr<IRemoteObject>& remoteObj, const std::string& name, int32_t& powerError);
    int32_t ProxyRunningLockIpc(bool isProxied, pid_t pid, pid_t uid);
    int32_t ProxyRunningLocksIpc(bool isProxied, const VectorPair& vectorPairInfos);
    int32_t ResetRunningLocksIpc();
    int32_t IsUsedIpc(const sptr<IRemoteObject>& remoteObj, bool& isUsed);
    int32_t SuspendDeviceIpc(int64_t callTimeMs, int32_t reasonValue, bool suspendImmed,
        const std::string& apiVersion, int32_t& powerError);
    int32_t WakeupDeviceIpc(int64_t callTimeMs, int32_t reasonValue, const std::string& details,
        const std::string& apiVersion, int32_t& powerError);
    int32_t RefreshActivityIpc(int64_t callTimeMs,
        int32_t activityType = static_cast<int32_t>(UserActivityType::USER_ACTIVITY_TYPE_OTHER),
        bool needChangeBacklight = true);
    int32_t OverrideScreenOffTimeIpc(int64_t timeout, int32_t& powerError);
    int32_t RestoreScreenOffTimeIpc(const std::string& apiVersion, int32_t& powerError);
    int32_t GetStateIpc(int32_t& powerState);
    int32_t IsScreenOnIpc(bool needPrintLog, bool& isScreenOn);
    int32_t ForceSuspendDeviceIpc(int64_t callTimeMs);
    int32_t IsForceSleepingIpc(bool& isForceSleeping);
    int32_t RebootDeviceIpc(const std::string& reason, int32_t& powerError);
    int32_t ShutDownDeviceIpc(const std::string& reason, int32_t& powerError);
    int32_t RegisterPowerStateCallbackIpc(const sptr<IPowerStateCallback>& callback, bool isSync = true);
    int32_t UnRegisterPowerStateCallbackIpc(const sptr<IPowerStateCallback>& callback);
    int32_t RegisterPowerModeCallbackIpc(const sptr<IPowerModeCallback>& callback);
    int32_t UnRegisterPowerModeCallbackIpc(const sptr<IPowerModeCallback>& callback);
    int32_t RegisterScreenStateCallbackIpc(int32_t remainTime, const sptr<IScreenOffPreCallback>& callback);
    int32_t UnRegisterScreenStateCallbackIpc(const sptr<IScreenOffPreCallback>& callback);
    int32_t RegisterRunningLockCallbackIpc(const sptr<IPowerRunninglockCallback>& callback);
    int32_t UnRegisterRunningLockCallbackIpc(const sptr<IPowerRunninglockCallback>& callback);
#ifdef POWER_MANAGER_TAKEOVER_SUSPEND
    bool RegisterSuspendTakeoverCallbackIpc(const sptr<ITakeOverSuspendCallback>& callback);
    bool UnRegisterSuspendTakeoverCallbackIpc(const sptr<ITakeOverSuspendCallback>& callback);
#endif
    int32_t SetDisplaySuspendIpc(bool enable);
    int32_t SetDeviceModeIpc(int32_t modeValue, int32_t& powerError);
    int32_t GetDeviceModeIpc(int32_t& powerMode);
    int32_t ShellDumpIpc(const std::vector<std::string>& args, uint32_t argc, std::string& returnDump);
    int32_t IsStandbyIpc(bool& isStandby, int32_t& powerError);
    int32_t RegisterUlsrCallbackIpc(const sptr<IAsyncUlsrCallback>& callback, int32_t& powerError);
    int32_t UnRegisterUlsrCallbackIpc(const sptr<IAsyncUlsrCallback>& callback, int32_t& powerError);
private:
    static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, 0xD000F00, "PowerTest"};
    sptr<PowerMgrStub> stub_ {nullptr};
    const int VECTOR_MAX_SIZE = 102400;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_SERVICE_TEST_PROXY_H
