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
    publishInfo_ = new (std::nothrow) CommonEventPublishInfo();
    if (publishInfo_ != nullptr) {
        publishInfo_->SetOrdered(false);
    }
    screenOffWant_ = new (std::nothrow) IntentWant();
    if (screenOffWant_ != nullptr) {
        screenOffWant_->SetAction(CommonEventSupport::COMMON_EVENT_SCREEN_OFF);
    }
    screenOnWant_ = new (std::nothrow) IntentWant();
    if (screenOnWant_ != nullptr) {
        screenOnWant_->SetAction(CommonEventSupport::COMMON_EVENT_SCREEN_ON);
    }
#ifdef POWER_MANAGER_ENABLE_FORCE_SLEEP_BROADCAST
    enterForceSleepWant_ = new (std::nothrow) IntentWant();
    if (enterForceSleepWant_ != nullptr) {
        enterForceSleepWant_->SetAction(CommonEventSupport::COMMON_EVENT_ENTER_FORCE_SLEEP);
    }
    exitForceSleepWant_ = new (std::nothrow) IntentWant();
    if (exitForceSleepWant_ != nullptr) {
        exitForceSleepWant_->SetAction(CommonEventSupport::COMMON_EVENT_EXIT_FORCE_SLEEP);
    }
#endif
#ifdef POWER_MANAGER_POWER_ENABLE_S4
    enterHibernateWant_ = new (std::nothrow) IntentWant();
    if (enterHibernateWant_ != nullptr) {
        enterHibernateWant_->SetAction(CommonEventSupport::COMMON_EVENT_ENTER_HIBERNATE);
    }
    exitHibernateWant_ = new (std::nothrow) IntentWant();
    if (exitHibernateWant_ != nullptr) {
        exitHibernateWant_->SetAction(CommonEventSupport::COMMON_EVENT_EXIT_HIBERNATE);
    }
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

void PowerMgrNotify::PublishScreenOffEvents(int64_t eventTime, const std::string& reason)
{
    if (screenOffWant_ == nullptr) {
        POWER_HILOGE(COMP_SVC, "%{public}s: Invalid parameter", __func__);
        return;
    }
    POWER_HILOGI(FEATURE_SUSPEND, "[UL_POWER] Start to publish event %{public}s at %{public}lld",
        screenOffWant_->GetAction().c_str(), static_cast<long long>(eventTime));
    screenOffWant_->SetParam("reason", reason);
    PublishEvents(eventTime, screenOffWant_);
    POWER_HILOGI(FEATURE_SUSPEND, "[UL_POWER] Publish event %{public}s done", screenOffWant_->GetAction().c_str());
}

void PowerMgrNotify::PublishScreenOnEvents(int64_t eventTime, const std::string& reason)
{
    if (screenOnWant_ == nullptr) {
        POWER_HILOGE(COMP_SVC, "%{public}s: Invalid parameter", __func__);
        return;
    }
    POWER_HILOGI(FEATURE_WAKEUP, "[UL_POWER] Start to publish event %{public}s at %{public}lld",
        screenOnWant_->GetAction().c_str(), static_cast<long long>(eventTime));
    screenOnWant_->SetParam("reason", reason);
    PublishEvents(eventTime, screenOnWant_);
    POWER_HILOGI(FEATURE_WAKEUP, "[UL_POWER] Publish event %{public}s done", screenOnWant_->GetAction().c_str());
}

#ifdef POWER_MANAGER_ENABLE_FORCE_SLEEP_BROADCAST
void PowerMgrNotify::PublishEnterForceSleepEvents(int64_t eventTime)
{
    if (enterForceSleepWant_ == nullptr) {
        POWER_HILOGE(COMP_SVC, "%{public}s: Invalid parameter", __func__);
        return;
    }
    POWER_HILOGI(FEATURE_SUSPEND, "[UL_POWER] Start to publish event %{public}s at %{public}lld",
        enterForceSleepWant_->GetAction().c_str(), static_cast<long long>(eventTime));
    PublishEvents(eventTime, enterForceSleepWant_);
    POWER_HILOGI(
        FEATURE_SUSPEND, "[UL_POWER] Publish event %{public}s done", enterForceSleepWant_->GetAction().c_str());
}

void PowerMgrNotify::PublishExitForceSleepEvents(int64_t eventTime)
{
    if (exitForceSleepWant_ == nullptr) {
        POWER_HILOGE(COMP_SVC, "%{public}s: Invalid parameter", __func__);
        return;
    }
    POWER_HILOGI(FEATURE_WAKEUP, "[UL_POWER] Start to publish event %{public}s at %{public}lld",
        exitForceSleepWant_->GetAction().c_str(), static_cast<long long>(eventTime));
    PublishEvents(eventTime, exitForceSleepWant_);
    POWER_HILOGI(FEATURE_WAKEUP, "[UL_POWER] Publish event %{public}s done", exitForceSleepWant_->GetAction().c_str());
}
#endif

#ifdef POWER_MANAGER_POWER_ENABLE_S4
void PowerMgrNotify::PublishEnterHibernateEvent(int64_t eventTime, bool clearMemory)
{
    if (enterHibernateWant_ == nullptr) {
        POWER_HILOGE(COMP_SVC, "%{public}s: Invalid parameter", __func__);
        return;
    }
    POWER_HILOGI(FEATURE_SUSPEND, "[UL_POWER] Start to publish event %{public}s at %{public}lld",
        enterHibernateWant_->GetAction().c_str(), static_cast<long long>(eventTime));
    enterHibernateWant_->SetParam("clearMemory", clearMemory);
    PublishEvents(eventTime, enterHibernateWant_);
    POWER_HILOGI(FEATURE_SUSPEND, "[UL_POWER] Publish event %{public}s done", enterHibernateWant_->GetAction().c_str());
}

void PowerMgrNotify::PublishExitHibernateEvent(int64_t eventTime, bool clearMemory)
{
    if (exitHibernateWant_ == nullptr) {
        POWER_HILOGE(COMP_SVC, "%{public}s: Invalid parameter", __func__);
        return;
    }
    POWER_HILOGI(FEATURE_WAKEUP, "[UL_POWER] Start to publish event %{public}s at %{public}lld",
        exitHibernateWant_->GetAction().c_str(), static_cast<long long>(eventTime));
    exitHibernateWant_->SetParam("clearMemory", clearMemory);
    PublishEvents(eventTime, exitHibernateWant_);
    POWER_HILOGI(FEATURE_WAKEUP, "[UL_POWER] Publish event %{public}s done", exitHibernateWant_->GetAction().c_str());
}
#endif

void PowerMgrNotify::PublishCustomizedEvent(const AAFwk::Want& want)
{
    CommonEventData data;
    data.SetWant(want);
    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(false);
    const std::vector<std::string> permissionVec { "ohos.permission.POWER_MANAGER" };
    publishInfo.SetSubscriberPermissions(permissionVec);
    bool ret = CommonEventManager::PublishCommonEvent(data, publishInfo);
    POWER_HILOGI(COMP_SVC, "PublishCustomizedEvent %{public}s ret=%{public}d", want.GetAction().c_str(), ret);
}
} // namespace PowerMgr
} // namespace OHOS
