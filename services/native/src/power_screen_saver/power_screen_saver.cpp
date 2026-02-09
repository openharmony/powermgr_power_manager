/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "power_screen_saver.h"
#include "power_log.h"
#include "setting_helper.h"

namespace OHOS {
namespace PowerMgr {

static const int32_t POWER_UID = 5528;

// Static event map definition
const std::unordered_map<std::string, ScreenSaverEventSubscriber::ReceiveEventType>
    ScreenSaverEventSubscriber::eventMap_ = {
        {OHOS::EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF,
            ScreenSaverEventSubscriber::ReceiveEventType::SCREEN_OFF},
        {"usual.event.power.USER_ACTIVITY",
            ScreenSaverEventSubscriber::ReceiveEventType::USER_ACTIVITY},
        {OHOS::EventFwk::CommonEventSupport::COMMON_EVENT_POWER_CONNECTED,
            ScreenSaverEventSubscriber::ReceiveEventType::POWER_CONNECTED},
        {OHOS::EventFwk::CommonEventSupport::COMMON_EVENT_POWER_DISCONNECTED,
            ScreenSaverEventSubscriber::ReceiveEventType::POWER_DISCONNECTED},
        {"usual.event.power.RUNNINGLOCK_SCREEN",
            ScreenSaverEventSubscriber::ReceiveEventType::RUNNINGLOCK_SCREEN_CHANGE}
};

std::vector<std::string> ScreenSaverEventSubscriber::GetEventNames()
{
    std::vector<std::string> eventNames;
    for (const auto& [name, type] : eventMap_) {
        eventNames.push_back(name);
    }
    return eventNames;
}

void PowerScreenSaver::Init()
{
    POWER_HILOGI(COMP_SVC, "Power screen saver init.");
    if (!screenSaverTimerManager_) {
        std::lock_guard<ffrt::mutex> lock(screenSaverMutex_);
        screenSaverTimerManager_ = std::make_shared<ScreenSaverTimerManager>();
        screenSaverTimerManager_->Init();
        screenSaverTimerManager_->UpdateScreenSaverTimer();
    }
    RegisterSettingScreenSaverObservers();
    SubscribeCommonEvent();
    return;
}

void PowerScreenSaver::RegisterSettingScreenSaverObservers()
{
    SettingObserver::UpdateFunc updateFunc = [&](const std::string &key) {
        std::lock_guard<ffrt::mutex> lock(screenSaverMutex_);
        screenSaverTimerManager_->UpdateScreenSaverTimer();
    };

    // Unregister old observers before registering new ones
    if (acScreenSaverTimeObserver_ != nullptr) {
        SettingHelper::UnregisterSettingObserver(acScreenSaverTimeObserver_);
    }
    if (dcScreenSaverTimeObserver_ != nullptr) {
        SettingHelper::UnregisterSettingObserver(dcScreenSaverTimeObserver_);
    }

    acScreenSaverTimeObserver_ = SettingHelper::RegisterSettingAcScreenSaverTimeObserver(updateFunc);
    dcScreenSaverTimeObserver_ = SettingHelper::RegisterSettingDcScreenSaverTimeObserver(updateFunc);
    POWER_HILOGI(COMP_SVC, "Register setting screen saver observer done.");
    return;
}

void PowerScreenSaver::SubscribeCommonEvent()
{
    using namespace OHOS::EventFwk;
    MatchingSkills matchingSkills;
    // Iterate over eventMap_ to ensure registration matches handling
    for (const auto& eventName : ScreenSaverEventSubscriber::GetEventNames()) {
        matchingSkills.AddEvent(eventName);
    }

    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetThreadMode(CommonEventSubscribeInfo::ThreadMode::COMMON);
    subscribeInfo.SetPublisherUid(POWER_UID);
    if (!subscriberPtr_) {
        subscriberPtr_ = std::make_shared<ScreenSaverEventSubscriber>(subscribeInfo, shared_from_this());
    }
    bool result = CommonEventManager::SubscribeCommonEvent(subscriberPtr_);
    if (!result) {
        POWER_HILOGE(COMP_SVC, "Subscribe common event failed.");
        return;
    }
    POWER_HILOGI(COMP_SVC, "The common event on which the screen saver depends has been registered.");
    return;
}

void PowerScreenSaver::UpdateScreenSaverTimer()
{
    std::lock_guard<ffrt::mutex> lock(screenSaverMutex_);
    if (screenSaverTimerManager_ == nullptr) {
        POWER_HILOGE(COMP_SVC, "screenSaverTimerManager_ is nullptr.");
        return;
    }
    screenSaverTimerManager_->UpdateScreenSaverTimer();
}

void PowerScreenSaver::CancelScreenSaverTimer()
{
    std::lock_guard<ffrt::mutex> lock(screenSaverMutex_);
    if (screenSaverTimerManager_ == nullptr) {
        POWER_HILOGE(COMP_SVC, "screenSaverTimerManager_ is nullptr.");
        return;
    }
    screenSaverTimerManager_->CancelScreenSaverTimer();
}

void PowerScreenSaver::SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus status)
{
    std::lock_guard<ffrt::mutex> lock(screenSaverMutex_);
    if (screenSaverTimerManager_ == nullptr) {
        POWER_HILOGE(COMP_SVC, "screenSaverTimerManager_ is nullptr.");
        return;
    }
    screenSaverTimerManager_->SetPowerPlugStatus(status);
}

void ScreenSaverEventSubscriber::OnReceiveEvent(const EventFwk::CommonEventData &data)
{
    std::shared_ptr<PowerScreenSaver> powerScreenSaver = powerScreenSaver_.lock();
    if (powerScreenSaver == nullptr) {
        POWER_HILOGE(COMP_SVC, "powerScreenSaver is nullptr.");
        return;
    }
    const auto& action = data.GetWant().GetAction();
    auto it = eventMap_.find(action);
    if (it == eventMap_.end()) {
        POWER_HILOGE(COMP_SVC, "Power screen saver map has no event.");
        return;
    }
    switch (it->second) {
        case ReceiveEventType::SCREEN_OFF:
            POWER_HILOGI(COMP_SVC, "Power screen saver received screen_off event.");
            powerScreenSaver->CancelScreenSaverTimer();
            break;
        case ReceiveEventType::USER_ACTIVITY:
        case ReceiveEventType::RUNNINGLOCK_SCREEN_CHANGE:
            POWER_HILOGD(COMP_SVC, "Power screen saver received user_activity event.");
            powerScreenSaver->UpdateScreenSaverTimer();
            break;
        case ReceiveEventType::POWER_CONNECTED:
            POWER_HILOGI(COMP_SVC, "Power screen saver received power_connected event.");
            powerScreenSaver->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_AC);
            powerScreenSaver->UpdateScreenSaverTimer();
            break;
        case ReceiveEventType::POWER_DISCONNECTED:
            POWER_HILOGI(COMP_SVC, "Power screen saver received power_disconnected event.");
            powerScreenSaver->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_DC);
            powerScreenSaver->UpdateScreenSaverTimer();
            break;
        default:
            break;
    }
}
} // namespace PowerMgr
} // namespace OHOS