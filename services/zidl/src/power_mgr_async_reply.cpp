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

#include "power_mgr_async_reply.h"
#include "power_common.h"

namespace OHOS {
namespace PowerMgr {
int PowerMgrStubAsync::OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    POWER_HILOGI(COMP_FWK, "code=%{public}u, flags=%{public}d", code, option.GetFlags());
    int result = ERR_OK;
    switch (code) {
        case SEND_ASYNC_REPLY: {
            int32_t replyData = data.ReadInt32();
            SendAsyncReply(replyData);
            break;
        }
        default:
            result = IPCObjectStub::OnRemoteRequest(code, data, reply, option);
            break;
    }

    return result;
}

void PowerMgrStubAsync::SendAsyncReply(int replyValue)
{
    std::unique_lock<std::mutex> lck(mutex_);
    asyncReply_ = replyValue;
    notified = true;
    lck.unlock();
    cv_.notify_all();
}

int PowerMgrStubAsync::WaitForAsyncReply(int timeout)
{
    std::unique_lock<std::mutex> lck(mutex_);
    cv_.wait_for(lck, std::chrono::milliseconds(timeout), [this]() {
        return notified;
    });
    return asyncReply_;
}

void PowerMgrProxyAsync::SendAsyncReply(int replyValue)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF(remote == nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option = { MessageOption::TF_ASYNC };
    data.WriteInt32(replyValue);
    remote->SendRequest(SEND_ASYNC_REPLY, data, reply, option);
}
} // namespace PowerMgr
} // namespace OHOS