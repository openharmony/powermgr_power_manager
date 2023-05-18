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

#ifndef POWERMGR_POWER_MANAGER_SHUTDOWN_CALLBACK_HOLER_H
#define POWERMGR_POWER_MANAGER_SHUTDOWN_CALLBACK_HOLER_H

#include <set>

#include "iremote_object.h"
#include "shutdown/shutdown_priority.h"

namespace OHOS {
namespace PowerMgr {
class ShutdownCallbackHolder : public IRemoteObject::DeathRecipient {
public:
    void OnRemoteDied(const wptr<IRemoteObject>& object) override;
    void AddCallback(const sptr<IRemoteObject>& callback, ShutdownPriority priority);
    void RemoveCallback(const sptr<IRemoteObject>& callback);

    std::set<sptr<IRemoteObject>> GetHighPriorityCallbacks();
    std::set<sptr<IRemoteObject>> GetDefaultPriorityCallbacks();
    std::set<sptr<IRemoteObject>> GetLowPriorityCallbacks();

private:
    static void RemoveCallback(std::set<sptr<IRemoteObject>>& callbacks, const sptr<IRemoteObject>& callback);

    std::mutex mutex_;
    std::set<sptr<IRemoteObject>> highPriorityCallbacks_;
    std::set<sptr<IRemoteObject>> defaultPriorityCallbacks_;
    std::set<sptr<IRemoteObject>> lowPriorityCallbacks_;
};
} // namespace PowerMgr
} // namespace OHOS

#endif // POWERMGR_POWER_MANAGER_SHUTDOWN_CALLBACK_HOLER_H
