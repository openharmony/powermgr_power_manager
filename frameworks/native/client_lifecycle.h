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

#ifndef POWERMGR_POWER_MANAGER_CLIENT_LIFECYCLE_H
#define POWERMGR_POWER_MANAGER_CLIENT_LIFECYCLE_H

#include "errors.h"
#include "ipower_mgr.h"

namespace OHOS {
namespace PowerMgr {
class ClientLifeCycle {
public:
    static sptr<IPowerMgr> GetProxy();

private:
    class LifeCycleRecipient : public IRemoteObject::DeathRecipient {
    public:
        LifeCycleRecipient() = default;
        ~LifeCycleRecipient() override = default;
        void OnRemoteDied(const wptr<IRemoteObject>& remote) override;

    private:
        DISALLOW_COPY_AND_MOVE(LifeCycleRecipient);
    };

    static ErrCode Connect();
    static void Reset(const wptr<IRemoteObject>& remote);

    static sptr<IPowerMgr> proxy_;
    static sptr<IRemoteObject::DeathRecipient> deathRecipient_;
    static std::mutex mutex_;
};
} // namespace PowerMgr
} // namespace OHOS

#endif // POWERMGR_POWER_MANAGER_CLIENT_LIFECYCLE_H
