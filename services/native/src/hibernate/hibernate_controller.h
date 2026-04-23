/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef POWERMGR_POWER_MANAGER_HIBERNATE_CONTROLLER_H
#define POWERMGR_POWER_MANAGER_HIBERNATE_CONTROLLER_H

#include <set>
#include <map>
#include <mutex>

#include "ipc_skeleton.h"
#include "power_common.h"
#include "hibernate/isync_hibernate_callback.h"
#include "hibernate/hibernate_callback_priority.h"

namespace OHOS {
namespace PowerMgr {
enum class HibernateStatus {
    HIBERNATE_SUCCESS = 0,
    HIBERNATE_FAILURE,
    HIBERNATE_INVALID_STATUS,
};
class HibernateController : public IRemoteObject::DeathRecipient {
public:
    HibernateController() {};
    virtual ~HibernateController() = default;

    void OnRemoteDied(const wptr<IRemoteObject>& remote) override;
    virtual HibernateStatus Hibernate(bool clearMemory);
    virtual void RegisterSyncHibernateCallback(const sptr<ISyncHibernateCallback>& cb,
        HibernateCallbackPriority priority);
    virtual void UnregisterSyncHibernateCallback(const sptr<ISyncHibernateCallback>& cb);
    virtual void PreHibernate();
    virtual void PostHibernate(bool hibernateResult = false);

private:
    struct HibernateCallbackCompare {
        bool operator()(const sptr<ISyncHibernateCallback>& lhs, const sptr<ISyncHibernateCallback>& rhs) const
        {
            return lhs->AsObject() < rhs->AsObject();
        }
    };
    using CallbackContainerType = std::set<sptr<ISyncHibernateCallback>, HibernateCallbackCompare>;
    void AddCallbackToHolder(const sptr<ISyncHibernateCallback>& cb, HibernateCallbackPriority priority);
    void RemoveCallbackFromHolder(const sptr<ISyncHibernateCallback>& cb);
    void AddCallbackPidUid(const sptr<ISyncHibernateCallback>& cb);
    void RemoveCallbackPidUid(const sptr<ISyncHibernateCallback>& cb);
    void TriggerCallbacks(const CallbackContainerType& callbacks, bool isPreHibernate,
        bool hibernateResult = false);
    CallbackContainerType GetHighPriorityCallbacks();
    CallbackContainerType GetDefaultPriorityCallbacks();
    CallbackContainerType GetLowPriorityCallbacks();

    bool prepared_ {false};
    std::mutex mutex_;
    CallbackContainerType highPriorityCallbacks_;
    CallbackContainerType defaultPriorityCallbacks_;
    CallbackContainerType lowPriorityCallbacks_;
    std::map<sptr<ISyncHibernateCallback>, std::pair<int32_t, int32_t>> cachedRegister_;
};
} // namespace PowerMgr
} // namespace OHOS

#endif // POWERMGR_POWER_MANAGER_HIBERNATE_CONTROLLER_H
