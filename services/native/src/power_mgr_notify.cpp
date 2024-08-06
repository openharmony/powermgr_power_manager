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
#ifdef POWER_MANAGER_ENABLE_FORCE_SLEEP_BROADCAST
    enterForceSleepWant_ = new (std::nothrow)IntentWant();
    enterForceSleepWant_->SetAction(CommonEventSupport::COMMON_EVENT_ENTER_FORCE_SLEEP);
    exitForceSleepWant_ = new (std::nothrow)IntentWant();
    exitForceSleepWant_->SetAction(CommonEventSupport::COMMON_EVENT_EXIT_FORCE_SLEEP);
#endif
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
    POWER_HILOGI(FEATURE_SUSPEND, "[UL_POWER] Start to publish event %{public}s at %{public}lld",
        screenOffWant_->GetAction().c_str(), static_cast<long long>(eventTime));
    PublishEvents(eventTime, screenOffWant_);
    POWER_HILOGI(FEATURE_SUSPEND, "[UL_POWER] Publish event %{public}s done", screenOffWant_->GetAction().c_str());
}

void PowerMgrNotify::PublishScreenOnEvents(int64_t eventTime)
{
    POWER_HILOGI(FEATURE_WAKEUP, "[UL_POWER] Start to publish event %{public}s at %{public}lld",
        screenOnWant_->GetAction().c_str(), static_cast<long long>(eventTime));
    PublishEvents(eventTime, screenOnWant_);
    POWER_HILOGI(FEATURE_WAKEUP, "[UL_POWER] Publish event %{public}s done", screenOnWant_->GetAction().c_str());
}

#ifdef POWER_MANAGER_ENABLE_FORCE_SLEEP_BROADCAST
void PowerMgrNotify::PublishEnterForceSleepEvents(int64_t eventTime)
{
    POWER_HILOGI(FEATURE_SUSPEND, "[UL_POWER] Start to publish event %{public}s at %{public}lld",
        enterForceSleepWant_->GetAction().c_str(), static_cast<long long>(eventTime));
    PublishEvents(eventTime, enterForceSleepWant_);
    POWER_HILOGI(
        FEATURE_SUSPEND, "[UL_POWER] Publish event %{public}s done", enterForceSleepWant_->GetAction().c_str());
}

void PowerMgrNotify::PublishExitForceSleepEvents(int64_t eventTime)
{
    POWER_HILOGI(FEATURE_WAKEUP, "[UL_POWER] Start to publish event %{public}s at %{public}lld",
        exitForceSleepWant_->GetAction().c_str(), static_cast<long long>(eventTime));
    PublishEvents(eventTime, exitForceSleepWant_);
    POWER_HILOGI(FEATURE_WAKEUP, "[UL_POWER] Publish event %{public}s done", exitForceSleepWant_->GetAction().c_str());
}
#endif
} // namespace PowerMgr
} // namespace OHOS
