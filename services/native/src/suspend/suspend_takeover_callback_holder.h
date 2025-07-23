/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#ifndef POWERMGR_POWER_MANAGER_TAKEOVER_CALLBACK_HOLDER_H
#define POWERMGR_POWER_MANAGER_TAKEOVER_CALLBACK_HOLDER_H

#include <set>
#include <map>
#include <singleton.h>
#include "ipc_skeleton.h"
#include "iremote_object.h"
#include "suspend/itake_over_suspend_callback.h"

namespace OHOS {
namespace PowerMgr {
class TakeOverSuspendCallbackHolder final : public DelayedRefSingleton<TakeOverSuspendCallbackHolder> {
    DECLARE_DELAYED_REF_SINGLETON(TakeOverSuspendCallbackHolder);
    DISALLOW_COPY_AND_MOVE(TakeOverSuspendCallbackHolder);
public:
    struct TakeoverSuspendCallbackCompare {
        bool operator()(const sptr<ITakeOverSuspendCallback>& lhs, const sptr<ITakeOverSuspendCallback>& rhs) const
        {
            return lhs->AsObject() < rhs->AsObject();
        }
    };

    using TakeoverSuspendCallbackContainerType = std::set<sptr<ITakeOverSuspendCallback>,
        TakeoverSuspendCallbackCompare>;
    using TakeoverSuspendCallbackCachedRegister =
        std::map<sptr<ITakeOverSuspendCallback>, std::pair<int32_t, int32_t>, TakeoverSuspendCallbackCompare>;
    void AddCallback(const sptr<ITakeOverSuspendCallback>& callback, TakeOverSuspendPriority priority);
    void RemoveCallback(const sptr<ITakeOverSuspendCallback>& callback);
    TakeoverSuspendCallbackContainerType GetHighPriorityCallbacks();
    TakeoverSuspendCallbackContainerType GetDefaultPriorityCallbacks();
    TakeoverSuspendCallbackContainerType GetLowPriorityCallbacks();
    std::pair<int32_t, int32_t> FindCallbackPidUid(const sptr<ITakeOverSuspendCallback>& callback);
private:
    static void RemoveCallback(
        TakeoverSuspendCallbackContainerType& callbacks, const sptr<ITakeOverSuspendCallback>& callback);
    void AddCallbackPidUid(const sptr<ITakeOverSuspendCallback>& callback);
    void RemoveCallbackPidUid(const sptr<ITakeOverSuspendCallback>& callback);

    std::mutex mutex_;
    TakeoverSuspendCallbackContainerType highPriorityCallbacks_;
    TakeoverSuspendCallbackContainerType defaultPriorityCallbacks_;
    TakeoverSuspendCallbackContainerType lowPriorityCallbacks_;
    TakeoverSuspendCallbackCachedRegister cachedRegister_;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_POWER_MANAGER_TAKEOVER_CALLBACK_HOLDER_H