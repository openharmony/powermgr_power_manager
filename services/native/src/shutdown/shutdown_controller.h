/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef POWERMGR_POWER_MANAGER_SHUTDOWN_CONTROLLER_H
#define POWERMGR_POWER_MANAGER_SHUTDOWN_CONTROLLER_H

#include "actions/idevice_state_action.h"
#include "device_power_action.h"
#include "want.h"
#include <atomic>
#include <string>

#include "shutdown/iasync_shutdown_callback.h"
#include "shutdown/isync_shutdown_callback.h"
#include "shutdown/itakeover_shutdown_callback.h"
#include "shutdown_callback_holer.h"

namespace OHOS {
namespace PowerMgr {
class ShutdownController {
public:
    ShutdownController();
    virtual ~ShutdownController() = default;

    virtual void Reboot(const std::string& reason);
    virtual void Shutdown(const std::string& reason);
    bool IsShuttingDown();
    void EnableMock(IDevicePowerAction* mockPowerAction, IDeviceStateAction* mockStateAction)
    {
        std::unique_ptr<IDevicePowerAction> mockPower(mockPowerAction);
        devicePowerAction_ = std::move(mockPower);
        std::unique_ptr<IDeviceStateAction> mockState(mockStateAction);
        deviceStateAction_ = std::move(mockState);
        started_ = false;
    }

    void AddCallback(const sptr<ITakeOverShutdownCallback>& callback, ShutdownPriority priority);
    void AddCallback(const sptr<IAsyncShutdownCallback>& callback, ShutdownPriority priority);
    void AddCallback(const sptr<ISyncShutdownCallback>& callback, ShutdownPriority priority);
    void RemoveCallback(const sptr<ITakeOverShutdownCallback>& callback);
    void RemoveCallback(const sptr<IAsyncShutdownCallback>& callback);
    void RemoveCallback(const sptr<ISyncShutdownCallback>& callback);

    bool TriggerTakeOverShutdownCallback(bool isReboot);
    void TriggerAsyncShutdownCallback();
    void TriggerSyncShutdownCallback();

private:
    using IntentWant = OHOS::AAFwk::Want;
    void RebootOrShutdown(const std::string& reason, bool isReboot);
    void Prepare();
    void TurnOffScreen();
    void PublishShutdownEvent() const;

    static bool TriggerTakeOverShutdownCallbackInner(std::set<sptr<IRemoteObject>>& callbacks, bool isReboot);
    static void TriggerAsyncShutdownCallbackInner(std::set<sptr<IRemoteObject>>& callbacks);
    static void TriggerSyncShutdownCallbackInner(std::set<sptr<IRemoteObject>>& callbacks);

    sptr<ShutdownCallbackHolder> takeoverShutdownCallbackHolder_;
    sptr<ShutdownCallbackHolder> asyncShutdownCallbackHolder_;
    sptr<ShutdownCallbackHolder> syncShutdownCallbackHolder_;

    std::atomic<bool> started_;
    std::unique_ptr<IDevicePowerAction> devicePowerAction_;
    std::shared_ptr<IDeviceStateAction> deviceStateAction_;
};
} // namespace PowerMgr
} // namespace OHOS

#endif // POWERMGR_POWER_MANAGER_SHUTDOWN_CONTROLLER_H
