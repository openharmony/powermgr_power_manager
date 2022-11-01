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

#ifndef SHUTDOWN_REBOOT_THREAD_H
#define SHUTDOWN_REBOOT_THREAD_H

#include <mutex>
#include <set>
#include <string>

#include "device_power_action.h"
#include "actions/idevice_state_action.h"
#include "ishutdown_callback.h"
#include "want.h"

namespace OHOS {
namespace PowerMgr {
class ShutdownService {
public:
    ShutdownService();
    ~ShutdownService() = default;
    void Reboot(const std::string& reason);
    void Shutdown(const std::string& reason);
    void AddShutdownCallback(IShutdownCallback::ShutdownPriority priority, const sptr<IShutdownCallback>& callback);
    void DelShutdownCallback(const sptr<IShutdownCallback>& callback);
    bool IsShuttingDown();
    void EnableMock(IDevicePowerAction* mockPowerAction, IDeviceStateAction* mockStateAction)
    {
        std::unique_ptr<IDevicePowerAction> mockPower(mockPowerAction);
        devicePowerAction_ = std::move(mockPower);
        std::unique_ptr<IDeviceStateAction> mockState(mockStateAction);
        deviceStateAction_ = std::move(mockState);
        started_ = false;
    }
private:
    using IntentWant = OHOS::AAFwk::Want;
    class CallbackManager : public IRemoteObject::DeathRecipient {
    public:
        void OnRemoteDied(const wptr<IRemoteObject>& remote) override;
        void AddCallback(const sptr<IShutdownCallback>& callback);
        void RemoveCallback(const sptr<IShutdownCallback>& callback);
        void WaitingCallback();

    private:
        std::mutex mutex_;
        std::set<sptr<IRemoteObject>> callbacks_;
    };
    void RebootOrShutdown(const std::string& reason, bool isReboot);
    void Prepare();
    void TurnOffScreen();
    void PublishShutdownEvent() const;

    sptr<CallbackManager> lowCallbackMgr_;
    sptr<CallbackManager> defaultCallbackMgr_;
    sptr<CallbackManager> highCallbackMgr_;
    std::atomic<bool> started_;
    std::unique_ptr<IDevicePowerAction> devicePowerAction_;
    std::unique_ptr<IDeviceStateAction> deviceStateAction_;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // SHUTDOWN_REBOOT_THREAD_H
