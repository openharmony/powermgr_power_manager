/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef POWERMGR_POWER_MANAGER_SLEEP_CALLBACK_HOLER_H
#define POWERMGR_POWER_MANAGER_SLEEP_CALLBACK_HOLER_H

#include <set>
#include <singleton.h>
#include "iremote_object.h"
#include "suspend/isync_sleep_callback.h"

namespace OHOS {
namespace PowerMgr {
class SleepCallbackHolder final : public DelayedRefSingleton<SleepCallbackHolder> {
    DECLARE_DELAYED_REF_SINGLETON(SleepCallbackHolder);
    DISALLOW_COPY_AND_MOVE(SleepCallbackHolder);
public:
    struct SleepCallbackCompare {
        bool operator()(const sptr<ISyncSleepCallback>& lhs, const sptr<ISyncSleepCallback>& rhs) const
        {
            return lhs->AsObject() < rhs->AsObject();
        }
    };

    using SleepCallbackContainerType = std::set<sptr<ISyncSleepCallback>, SleepCallbackCompare>;
    void AddCallback(const sptr<ISyncSleepCallback>& callback, SleepPriority priority);
    void RemoveCallback(const sptr<ISyncSleepCallback>& callback);
    SleepCallbackContainerType GetHighPriorityCallbacks();
    SleepCallbackContainerType GetDefaultPriorityCallbacks();
    SleepCallbackContainerType GetLowPriorityCallbacks();

private:
    static void RemoveCallback(SleepCallbackContainerType& callbacks, const sptr<ISyncSleepCallback>& callback);

    std::mutex mutex_;
    SleepCallbackContainerType highPriorityCallbacks_;
    SleepCallbackContainerType defaultPriorityCallbacks_;
    SleepCallbackContainerType lowPriorityCallbacks_;
};
} // namespace PowerMgr
} // namespace OHOS

#endif // POWERMGR_POWER_MANAGER_SLEEP_CALLBACK_HOLER_H
