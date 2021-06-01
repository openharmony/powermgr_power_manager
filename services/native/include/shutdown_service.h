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

#ifndef SHUTDOWN_REBOOT_THREAD_H
#define SHUTDOWN_REBOOT_THREAD_H

#include <mutex>
#include <set>
#include <string>

#include <ohos/aafwk/content/want.h>

#include "device_power_action.h"
#include "ishutdown_callback.h"

namespace OHOS {
namespace PowerMgr {
class ShutdownService {
public:
    ShutdownService();
    ~ShutdownService() = default;
    void Reboot(const std::string& reason);
    void Shutdown(const std::string& reason);
    void AddShutdownCallback(const sptr<IShutdownCallback>& callback);
    void DelShutdownCallback(const sptr<IShutdownCallback>& callback);

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
    void PublishShutdownEvent() const;

    CallbackManager callbackMgr_;
    std::atomic<bool> started_;
    std::unique_ptr<IDevicePowerAction> devicePowerAction_;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // SHUTDOWN_REBOOT_THREAD_H
