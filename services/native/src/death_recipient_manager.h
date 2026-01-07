/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef DEATH_RECIPIENT_MANAGER_H
#define DEATH_RECIPIENT_MANAGER_H

#include <map>

#include "ipc_skeleton.h"
#include "iremote_object.h"
#include "power_common.h"

namespace OHOS {
namespace PowerMgr {
class DeathRecipientManager {
public:
    using CallbackType = std::function<void(const sptr<IRemoteObject>&)>;
    struct CBInfo {
        CallbackType func;
        std::string funcName;
        int32_t pid {-1};
        int32_t uid {-1};
    };
    class CommonDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        CommonDeathRecipient(const CBInfo& callbackInfo) : callbackInfo_(callbackInfo) {}
        virtual ~CommonDeathRecipient() = default;
        virtual void OnRemoteDied(const wptr<IRemoteObject>& remote) override;
    private:
        CBInfo callbackInfo_;
    };
    static DeathRecipientManager& GetInstance();
    void AddDeathRecipient(const sptr<IRemoteObject>& invoker, const CBInfo& info);
    void AddDeathRecipient(
        int32_t uid, const sptr<IRemoteObject>& invoker, const sptr<IRemoteObject::DeathRecipient>& recipient);
    void RemoveDeathRecipient(int32_t uid);
private:
    DeathRecipientManager() = default;
    ~DeathRecipientManager() = default;
    std::mutex callbacksMutex_;
    std::unordered_map<int32_t, sptr<IRemoteObject>> clientDeathRecipientMap_;
};
} // namespace PowerMgr
} // namespace OHOS

#endif // DEATH_RECIPIENT_MANAGER_H