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

#ifndef POWERMGR_POWER_MGR_MONITOR_H
#define POWERMGR_POWER_MGR_MONITOR_H

#include <functional>
#include <map>
#include <memory>
#include <string>

#include <common_event_subscriber.h>
#include "want.h"

using IntentWant = OHOS::AAFwk::Want;
using Ces = OHOS::EventFwk::CommonEventSubscriber;
using CesInfo = OHOS::EventFwk::CommonEventSubscribeInfo;
using EventHandle = std::function<void(const IntentWant&)>;

namespace OHOS {
namespace PowerMgr {
class PowerMgrMonitor {
public:
    PowerMgrMonitor() = default;
    ~PowerMgrMonitor();

    bool Start();

private:
    typedef void (PowerMgrMonitor::*HandleEventFunc)(const IntentWant&) const;

    class EventSubscriber : public Ces {
    public:
        EventSubscriber(const sptr<CesInfo>& info, const std::map<std::string, EventHandle>& handles)
            : Ces(*info), eventHandles_(handles) {}
        void OnReceiveEvent(const EventFwk::CommonEventData &data) override
        {
            HandleEvent(data.GetWant());
        }
        ~EventSubscriber() override = default;

    private:
        void HandleEvent(const IntentWant& want);

        const std::map<std::string, EventHandle>& eventHandles_;
    };

    void InitEventHandles();
    sptr<CesInfo> GetSubscribeInfo() const;
    bool RegisterSubscriber(const sptr<CesInfo>& info);
    void HandleScreenStateChanged(const IntentWant& want) const;
    void HandleStartUpCompleted(const IntentWant& want) const;

    static const std::map<std::string, HandleEventFunc> EVENT_HANDLES;
    std::shared_ptr<Ces> subscriber_;
    std::map<std::string, EventHandle> eventHandles_;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_POWER_MGR_MONITOR_H
