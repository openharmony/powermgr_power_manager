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

#include "hibernate/isync_hibernate_callback.h"

namespace OHOS {
namespace PowerMgr {
enum class HibernateStatus {
    HIBERNATE_SUCCESS = 0,
    HIBERNATE_FAILURE,
    HIBERNATE_INVALID_STATUS,
};
class HibernateController {
public:
    struct HibernateCallbackCompare {
        bool operator()(const sptr<ISyncHibernateCallback>& lhs, const sptr<ISyncHibernateCallback>& rhs) const
        {
            return lhs->AsObject() < rhs->AsObject();
        }
    };
    using HibernateCallbackContainerType = std::set<sptr<ISyncHibernateCallback>, HibernateCallbackCompare>;

    HibernateController() {};
    virtual ~HibernateController() = default;

    virtual HibernateStatus Hibernate(bool clearMemory);
    virtual void RegisterSyncHibernateCallback(const sptr<ISyncHibernateCallback>& cb);
    virtual void UnregisterSyncHibernateCallback(const sptr<ISyncHibernateCallback>& cb);
    virtual void PreHibernate();
    virtual void PostHibernate(bool hibernateResult = false);

private:
    bool prepared_ {false};
    std::mutex mutex_;
    HibernateCallbackContainerType callbacks_;
};
} // namespace PowerMgr
} // namespace OHOS

#endif // POWERMGR_POWER_MANAGER_HIBERNATE_CONTROLLER_H
