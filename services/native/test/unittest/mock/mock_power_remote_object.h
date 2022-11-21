/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#ifndef MOCK_POWER_REMOTE_OBJECT_H
#define MOCK_POWER_REMOTE_OBJECT_H

#include "iremote_object.h"

namespace OHOS {
namespace PowerMgr {
class MockPowerRemoteObject : public IRemoteObject {
public:
    explicit MockPowerRemoteObject(std::u16string descriptor = std::u16string());
    ~MockPowerRemoteObject();

    bool IsProxyObject() const override
    {
        return false;
    };

    int32_t GetObjectRefCount() override;

    int Dump(int fd, const std::vector<std::u16string>& args) override;

    int SendRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override;

    bool AddDeathRecipient(const sptr<DeathRecipient>& recipient) override;

    bool RemoveDeathRecipient(const sptr<DeathRecipient>& recipient) override;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // MOCK_POWER_REMOTE_OBJECT_H
