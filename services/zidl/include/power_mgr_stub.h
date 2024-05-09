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

#ifndef POWERMGR_SERVICES_IPOWER_MANAGER_STUB_H
#define POWERMGR_SERVICES_IPOWER_MANAGER_STUB_H

#include <iremote_stub.h>
#include <nocopyable.h>

#include "ipower_mgr.h"

namespace OHOS {
namespace PowerMgr {
class PowerMgrStub : public IRemoteStub<IPowerMgr> {
public:
    DISALLOW_COPY_AND_MOVE(PowerMgrStub);
    PowerMgrStub() = default;
    ~PowerMgrStub() override = default;
    int OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    static bool IsShutdownCommand(uint32_t code);

    int32_t WakeupDeviceStub(MessageParcel& data, MessageParcel& reply);
    int32_t SuspendDeviceStub(MessageParcel& data, MessageParcel& reply);
    int32_t RefreshActivityStub(MessageParcel& data);
    int32_t OverrideScreenOffTimeStub(MessageParcel& data, MessageParcel& reply);
    int32_t RestoreScreenOffTimeStub(MessageParcel& reply);
    int32_t GetStateStub(MessageParcel& reply);
    int32_t IsScreenOnStub(MessageParcel& reply);
    int32_t ForceSuspendDeviceStub(MessageParcel& data, MessageParcel& reply);
    int32_t ProxyRunningLockStub(MessageParcel& data, MessageParcel& reply);
    int32_t ProxyRunningLocksStub(MessageParcel& data, MessageParcel& reply);
    int32_t ResetAllPorxyStub(MessageParcel& data, MessageParcel& reply);
    int32_t CreateRunningLockStub(MessageParcel& data, MessageParcel& reply);
    int32_t ReleaseRunningLockStub(MessageParcel& data);
    int32_t IsRunningLockTypeSupportedStub(MessageParcel& data, MessageParcel& reply);
    int32_t LockStub(MessageParcel& data);
    int32_t UnLockStub(MessageParcel& data);
    int32_t QueryRunningLockListsStub(MessageParcel& data, MessageParcel& reply);
    int32_t IsUsedStub(MessageParcel& data, MessageParcel& reply);
    int32_t RebootDeviceStub(MessageParcel& data, MessageParcel& reply);
    int32_t RebootDeviceForDeprecatedStub(MessageParcel& data, MessageParcel& reply);
    int32_t ShutDownDeviceStub(MessageParcel& data, MessageParcel& reply);
    int32_t RegisterPowerStateCallbackStub(MessageParcel& data);

    int32_t RegisterSyncSleepCallbackStub(MessageParcel& data);
    int32_t UnRegisterSyncSleepCallbackStub(MessageParcel& data);

    int32_t UnRegisterPowerStateCallbackStub(MessageParcel& data);
    int32_t RegisterPowerModeCallbackStub(MessageParcel& data);
    int32_t UnRegisterPowerModeCallbackStub(MessageParcel& data);
    int32_t RegisterRunningLockCallbackStub(MessageParcel& data);
    int32_t UnRegisterRunningLockCallbackStub(MessageParcel& data);
    int32_t RegisterScreenStateCallbackStub(MessageParcel& data);
    int32_t UnRegisterScreenStateCallbackStub(MessageParcel& data);
    int32_t SetDisplaySuspendStub(MessageParcel& data);
    int32_t HibernateStub(MessageParcel& data);
    int32_t SetDeviceModeStub(MessageParcel& data, MessageParcel& reply);
    int32_t GetDeviceModeStub(MessageParcel& reply);
    int32_t ShellDumpStub(MessageParcel& data, MessageParcel& reply);
    int32_t IsStandbyStub(MessageParcel& data, MessageParcel& reply);
    int32_t SetForceTimingOutStub(MessageParcel& data, MessageParcel& reply);
    int32_t LockScreenAfterTimingOutStub(MessageParcel& data, MessageParcel& reply);
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_SERVICES_IPOWER_MANAGER_STUB_H
