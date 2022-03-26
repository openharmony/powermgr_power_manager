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

#include "power_mgr_notify.h"

#include <common_event_data.h>
#include <common_event_manager.h>
#include <common_event_support.h>

#include "power_log.h"

using namespace OHOS::AAFwk;
using namespace OHOS::EventFwk;

namespace OHOS {
namespace PowerMgr {
void PowerMgrNotify::RegisterPublishEvents()
{
    if (publishInfo_ != nullptr) {
        return;
    }
    publishInfo_ = new (std::nothrow)CommonEventPublishInfo();
    publishInfo_->SetOrdered(false);
    screenOffWant_ = new (std::nothrow)IntentWant();
    screenOffWant_->SetAction(CommonEventSupport::COMMON_EVENT_SCREEN_OFF);
    screenOnWant_ = new (std::nothrow)IntentWant();
    screenOnWant_->SetAction(CommonEventSupport::COMMON_EVENT_SCREEN_ON);
}

void PowerMgrNotify::PublishEvents(int64_t eventTime, sptr<IntentWant> want)
{
    if ((want == nullptr) || (publishInfo_ == nullptr)) {
        POWER_HILOGE(COMP_SVC, "Invalid parameter");
        return;
    }
    CommonEventData event(*want);
    CommonEventManager::PublishCommonEvent(event, *publishInfo_, nullptr);
}

void PowerMgrNotify::PublishScreenOffEvents(int64_t eventTime)
{
    POWER_HILOGI(FEATURE_SUSPEND, "Start to publish event %{public}s at %{public}lld",
        screenOffWant_->GetAction().c_str(), static_cast<long long>(eventTime));
    PublishEvents(eventTime, screenOffWant_);
    POWER_HILOGI(FEATURE_SUSPEND, "Publish event %{public}s done", screenOffWant_->GetAction().c_str());
}

void PowerMgrNotify::PublishScreenOnEvents(int64_t eventTime)
{
    POWER_HILOGI(FEATURE_WAKEUP, "Start to publish event %{public}s at %{public}lld",
        screenOnWant_->GetAction().c_str(), static_cast<long long>(eventTime));
    PublishEvents(eventTime, screenOnWant_);
    POWER_HILOGI(FEATURE_WAKEUP, "Publish event %{public}s done", screenOnWant_->GetAction().c_str());
}
} // namespace PowerMgr
} // namespace OHOS
