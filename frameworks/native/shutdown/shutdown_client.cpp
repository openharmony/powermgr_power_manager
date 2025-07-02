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

#include "shutdown/shutdown_client.h"

#include "client_lifecycle.h"
#include "power_common.h"

namespace OHOS {
namespace PowerMgr {
ShutdownClient::ShutdownClient() {}
ShutdownClient::~ShutdownClient() {}

ErrCode ShutdownClient::Connect()
{
    proxy_ = ClientLifeCycle::GetProxy();
    return proxy_ != nullptr ? ERR_OK : ERR_NO_INIT;
}

void ShutdownClient::RegisterShutdownCallback(
    const sptr<ITakeOverShutdownCallback>& callback, ShutdownPriority priority)
{
    RETURN_IF(Connect() != ERR_OK);
    proxy_->RegisterShutdownCallback(callback, priority);
}

void ShutdownClient::UnRegisterShutdownCallback(const sptr<ITakeOverShutdownCallback>& callback)
{
    RETURN_IF(Connect() != ERR_OK);
    proxy_->UnRegisterShutdownCallback(callback);
}

void ShutdownClient::RegisterShutdownCallback(
    const sptr<IAsyncShutdownCallback>& callback, ShutdownPriority priority)
{
    RETURN_IF(Connect() != ERR_OK);
    proxy_->RegisterShutdownCallback(callback, priority);
}

void ShutdownClient::UnRegisterShutdownCallback(const sptr<IAsyncShutdownCallback>& callback)
{
    RETURN_IF(Connect() != ERR_OK);
    proxy_->UnRegisterShutdownCallback(callback);
}

void ShutdownClient::RegisterShutdownCallback(
    const sptr<ISyncShutdownCallback>& callback, ShutdownPriority priority)
{
    RETURN_IF(Connect() != ERR_OK);
    proxy_->RegisterShutdownCallback(callback, priority);
}

void ShutdownClient::UnRegisterShutdownCallback(const sptr<ISyncShutdownCallback>& callback)
{
    RETURN_IF(Connect() != ERR_OK);
    proxy_->UnRegisterShutdownCallback(callback);
}
} // namespace PowerMgr
} // namespace OHOS
