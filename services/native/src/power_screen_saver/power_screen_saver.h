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

#ifndef POWER_SCREEN_SAVER_H
#define POWER_SCREEN_SAVER_H

#include "screen_saver_timer_manager.h"
#include "setting_helper.h"

#include <string>
#include <unordered_map>
#include <vector>

#include "common_event_subscriber.h"
#include "common_event_manager.h"
#include "common_event_support.h"

namespace OHOS {
namespace PowerMgr {

class PowerScreenSaver : public std::enable_shared_from_this<PowerScreenSaver> {
public:
    PowerScreenSaver() = default;
    virtual ~PowerScreenSaver() = default;
    void Init();
    void RegisterSettingScreenSaverObservers();
    void UpdateScreenSaverTimer();
    void CancelScreenSaverTimer();
    void SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus status);

private:
    void SubscribeCommonEvent();
    std::shared_ptr<ScreenSaverTimerManager> screenSaverTimerManager_;
    ffrt::mutex screenSaverMutex_;
    std::shared_ptr<EventFwk::CommonEventSubscriber> subscriberPtr_ {nullptr};
    sptr<SettingObserver> acScreenSaverTimeObserver_ {nullptr};
    sptr<SettingObserver> dcScreenSaverTimeObserver_ {nullptr};
};

class ScreenSaverEventSubscriber : public EventFwk::CommonEventSubscriber {
public:
    explicit ScreenSaverEventSubscriber(const EventFwk::CommonEventSubscribeInfo& subscribeInfo,
                                        const std::weak_ptr<PowerScreenSaver>& powerScreenSaver)
        : EventFwk::CommonEventSubscriber(subscribeInfo), powerScreenSaver_(powerScreenSaver) {}
    virtual ~ScreenSaverEventSubscriber() = default;
    void OnReceiveEvent(const EventFwk::CommonEventData &data) override;

    // Get all event names for registration (iterates over eventMap_)
    static std::vector<std::string> GetEventNames();

private:
    enum class ReceiveEventType {
        SCREEN_OFF,
        USER_ACTIVITY,
        POWER_CONNECTED,
        POWER_DISCONNECTED,
        RUNNINGLOCK_SCREEN_CHANGE,
        UNKNOWN
    };
    static const std::unordered_map<std::string, ReceiveEventType> eventMap_;
    std::weak_ptr<PowerScreenSaver> powerScreenSaver_;
};

} //namespace PowerMgr
} //namespace OHOS
#endif // POWER_SCREEN_SAVER_H
