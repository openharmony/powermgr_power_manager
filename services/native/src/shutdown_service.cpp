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

#include "shutdown_service.h"

#include <algorithm>
#include <atomic>
#include <future>
#include <thread>
#include <cinttypes>
#include <common_event_data.h>
#include <common_event_manager.h>
#include <common_event_publish_info.h>
#include <common_event_support.h>
#include <datetime_ex.h>

#include "power_log.h"
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
    POWER_HILOGD(FEATURE_SHUTDOWN, "Instance create");
    devicePowerAction_ = PowerMgrFactory::GetDevicePowerAction();
    deviceStateAction_ = PowerMgrFactory::GetDeviceStateAction();
    highCallbackMgr_ = new CallbackManager();
    defaultCallbackMgr_ = new CallbackManager();
    lowCallbackMgr_ = new CallbackManager();
}

void ShutdownService::Reboot(const std::string& reason)
{
    RebootOrShutdown(reason, true);
}

void ShutdownService::Shutdown(const std::string& reason)
{
    RebootOrShutdown(reason, false);
}

void ShutdownService::AddShutdownCallback(IShutdownCallback::ShutdownPriority priority,
    const sptr<IShutdownCallback>& callback)
{
    switch (priority) {
        case IShutdownCallback::ShutdownPriority::POWER_SHUTDOWN_PRIORITY_HIGH:
            highCallbackMgr_->AddCallback(callback);
            break;
        case IShutdownCallback::ShutdownPriority::POWER_SHUTDOWN_PRIORITY_DEFAULT:
            defaultCallbackMgr_->AddCallback(callback);
            break;
        case IShutdownCallback::ShutdownPriority::POWER_SHUTDOWN_PRIORITY_LOW:
            lowCallbackMgr_->AddCallback(callback);
            break;
        default:
            return;
    }
}

void ShutdownService::DelShutdownCallback(const sptr<IShutdownCallback>& callback)
{
    highCallbackMgr_->RemoveCallback(callback);
    defaultCallbackMgr_->RemoveCallback(callback);
    lowCallbackMgr_->RemoveCallback(callback);
}

bool ShutdownService::IsShuttingDown()
{
    return started_;
}

void ShutdownService::RebootOrShutdown(const std::string& reason, bool isReboot)
{
    if (started_) {
        POWER_HILOGW(FEATURE_SHUTDOWN, "Shutdown is already running");
        return;
    }
    started_ = true;
    POWER_HILOGI(FEATURE_SHUTDOWN, "Start to detach shutdown thread");
    make_unique<thread>([=] {
        Prepare();
        TurnOffScreen();
        POWER_HILOGD(FEATURE_SHUTDOWN, "reason = %{public}s, reboot = %{public}d", reason.c_str(), isReboot);
        if (devicePowerAction_ != nullptr) {
            isReboot ? devicePowerAction_->Reboot(reason) : devicePowerAction_->Shutdown(reason);
        }
        started_ = false;
    })->detach();
}

void ShutdownService::Prepare()
{
    PublishShutdownEvent();
    POWER_HILOGD(FEATURE_SHUTDOWN, "High priority shutdown callback started");
    highCallbackMgr_->WaitingCallback();
    POWER_HILOGD(FEATURE_SHUTDOWN, "Default priority shutdown callback started");
    defaultCallbackMgr_->WaitingCallback();
    POWER_HILOGD(FEATURE_SHUTDOWN, "Low priority shutdown callback started");
    lowCallbackMgr_->WaitingCallback();
}

void ShutdownService::PublishShutdownEvent() const
{
    POWER_HILOGD(FEATURE_SHUTDOWN, "Start of publishing shutdown event");
    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(false);
    IntentWant shutdownWant;
    shutdownWant.SetAction(CommonEventSupport::COMMON_EVENT_SHUTDOWN);
    CommonEventData event(shutdownWant);
    if (!CommonEventManager::PublishCommonEvent(event, publishInfo, nullptr)) {
        POWER_HILOGE(FEATURE_SHUTDOWN, "Publish the shutdown event fail");
        return;
    }
    POWER_HILOGD(FEATURE_SHUTDOWN, "End of publishing shutdown event");
}

void ShutdownService::TurnOffScreen()
{
    POWER_HILOGD(FEATURE_SHUTDOWN, "Turn off screen before shutdown");
    deviceStateAction_->SetDisplayState(DisplayState::DISPLAY_OFF, StateChangeReason::STATE_CHANGE_REASON_INIT);
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
    POWER_HILOGD(FEATURE_SHUTDOWN, "callbacks.size = %{public}zu, insertOk = %{public}d",
        callbacks_.size(), retIt.second);
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
    POWER_HILOGD(FEATURE_SHUTDOWN, "callbacks.size = %{public}zu", callbacks_.size());
}

void ShutdownService::CallbackManager::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    RETURN_IF(remote.promote() == nullptr);
    RemoveCallback(iface_cast<IShutdownCallback>(remote.promote()));
}

void ShutdownService::CallbackManager::WaitingCallback()
{
    auto callbackStart = [&]() {
        unique_lock<mutex> lock(mutex_);
        for (auto &obj : callbacks_) {
            sptr<IShutdownCallback> callback = iface_cast<IShutdownCallback>(obj);
            if (callback != nullptr) {
                int64_t start = GetTickCount();
                callback->ShutdownCallback();
                int64_t cost = GetTickCount() - start;
                POWER_HILOGD(FEATURE_SHUTDOWN, "Callback finished, cost=%{public}" PRId64 "", cost);
            }
        }
    };

    packaged_task<void()> callbackTask(callbackStart);
    future<void> fut = callbackTask.get_future();
    make_unique<thread>(std::move(callbackTask))->detach();

    POWER_HILOGI(FEATURE_SHUTDOWN, "Waiting for the callback execution complete...");
    future_status status = fut.wait_for(std::chrono::seconds(MAX_TIMEOUT_SEC));
    if (status == future_status::timeout) {
        POWER_HILOGW(FEATURE_SHUTDOWN, "Shutdown callback execution timeout");
    }
    POWER_HILOGI(FEATURE_SHUTDOWN, "The callback execution is complete");
}
} // namespace PowerMgr
} // namespace OHOS
