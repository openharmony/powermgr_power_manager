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

#include "shutdown_service.h"

#include <algorithm>
#include <atomic>
#include <future>
#include <thread>

#include <common_event_data.h>
#include <common_event_manager.h>
#include <common_event_publish_info.h>
#include <common_event_support.h>

#include "hilog_wrapper.h"
#include "power_mgr_factory.h"

using namespace OHOS::AAFwk;
using namespace OHOS::EventFwk;
using namespace std;

namespace OHOS {
namespace PowerMgr {
namespace {
const time_t MAX_TIMEOUT_SEC = 30;
}
ShutdownService::ShutdownService() : started_(false)
{
    devicePowerAction_ = PowerMgrFactory::GetDevicePowerAction();
}

void ShutdownService::Reboot(const std::string& reason)
{
    RebootOrShutdown(reason, true);
}

void ShutdownService::Shutdown(const std::string& reason)
{
    RebootOrShutdown(reason, false);
}

void ShutdownService::AddShutdownCallback(const sptr<IShutdownCallback>& callback)
{
    callbackMgr_.AddCallback(callback);
}

void ShutdownService::DelShutdownCallback(const sptr<IShutdownCallback>& callback)
{
    callbackMgr_.RemoveCallback(callback);
}

void ShutdownService::RebootOrShutdown(const std::string& reason, bool isReboot)
{
    if (started_) {
        POWER_HILOGE(MODULE_SERVICE, "Shutdown is already running.");
        return;
    }
    started_ = true;
    make_unique<thread>([=] {
        Prepare();
        POWER_HILOGD(MODULE_SERVICE, "reason = %{public}s, reboot = %{public}d", reason.c_str(), isReboot);
        if (devicePowerAction_ != nullptr) {
            isReboot ? devicePowerAction_->Reboot(reason) : devicePowerAction_->Shutdown(reason);
        }
        started_ = false;
    })->detach();
}

void ShutdownService::Prepare()
{
    PublishShutdownEvent();
    callbackMgr_.WaitingCallback();
}

void ShutdownService::PublishShutdownEvent() const
{
    POWER_HILOGD(MODULE_SERVICE, "Start of publishing shutdown event");
    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(false);
    IntentWant shutdownWant;
    shutdownWant.SetAction(CommonEventSupport::COMMON_EVENT_SHUTDOWN);
    CommonEventData event(shutdownWant);
    if (!CommonEventManager::PublishCommonEvent(event, publishInfo, nullptr)) {
        POWER_HILOGE(MODULE_SERVICE, "Failed to publish the shutdown event!");
        return;
    }
    POWER_HILOGD(MODULE_SERVICE, "End of publishing shutdown event");
}

void ShutdownService::CallbackManager::AddCallback(const sptr<IShutdownCallback>& callback)
{
    unique_lock<mutex> lock(mutex_);
    RETURN_IF((callback == nullptr) || (callback->AsObject() == nullptr));
    auto object = callback->AsObject();
    auto retIt = callbacks_.insert(object);
    if (retIt.second) {
        object->AddDeathRecipient(this);
    }
    POWER_HILOGI(MODULE_SERVICE, "object = %{public}p, callback = %{public}p, callbacks.size = %{public}zu,"
        " insertOk = %{public}d", object.GetRefPtr(),
        callback.GetRefPtr(), callbacks_.size(), retIt.second);
}

void ShutdownService::CallbackManager::RemoveCallback(const sptr<IShutdownCallback>& callback)
{
    unique_lock<mutex> lock(mutex_);
    RETURN_IF((callback == nullptr) || (callback->AsObject() == nullptr));
    auto object = callback->AsObject();
    auto it = find(callbacks_.begin(), callbacks_.end(), object);
    if (it != callbacks_.end()) {
        callbacks_.erase(it);
        object->RemoveDeathRecipient(this);
    }
    POWER_HILOGI(MODULE_SERVICE, "object = %{public}p, callback = %{public}p, callbacks.size = %{public}zu,",
        object.GetRefPtr(), callback.GetRefPtr(), callbacks_.size());
}

void ShutdownService::CallbackManager::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    RETURN_IF(remote.promote() == nullptr);
    RemoveCallback(iface_cast<IShutdownCallback>(remote.promote()));
}

void ShutdownService::CallbackManager::WaitingCallback()
{
    POWER_HILOGD(MODULE_SERVICE, "Shutdown callback started.");
    auto callbackStart = [&]() {
        unique_lock<mutex> lock(mutex_);
        for (auto &obj : callbacks_) {
            sptr<IShutdownCallback> callback = iface_cast<IShutdownCallback>(obj);
            if (callback != nullptr) {
                callback->ShutdownCallback();
            }
        }
    };

    packaged_task<void()> callbackTask(callbackStart);
    future<void> fut = callbackTask.get_future();
    make_unique<thread>(std::move(callbackTask))->detach();
    time_t begin = time(nullptr);
    POWER_HILOGI(MODULE_SERVICE, "Waiting for the callback execution is complete..."
        " begin = %{public}ld, callback size: %{public}zu", begin, callbacks_.size());
    future_status status = fut.wait_for(std::chrono::seconds(MAX_TIMEOUT_SEC));
    if (status == future_status::timeout) {
        POWER_HILOGW(MODULE_SERVICE, "Shutdown callback execution timedout!");
    }
    time_t end = time(nullptr);
    POWER_HILOGI(MODULE_SERVICE, "The callback execution is complete. begin = %{public}ld, end = %{public}ld,"
        " Execution Time(s): %{public}ld", begin, end, (end - begin));
}
} // namespace PowerMgr
} // namespace OHOS
