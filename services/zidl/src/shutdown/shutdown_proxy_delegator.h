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

#ifndef POWERMGR_POWER_MANAGER_SHUTDOWN_PROXY_H
#define POWERMGR_POWER_MANAGER_SHUTDOWN_PROXY_H

#include <utility>

#include "ipower_mgr.h"
#include "iremote_object.h"

namespace OHOS {
namespace PowerMgr {
class ShutdownProxyDelegator {
public:
    ShutdownProxyDelegator(const sptr<IRemoteObject>& remote, std::u16string descriptor)
        : remote_(remote), descriptor_(std::move(descriptor))
    {
    }
    virtual ~ShutdownProxyDelegator() = default;

    void RegisterShutdownCallback(const sptr<ITakeOverShutdownCallback>& callback, ShutdownPriority priority);
    void UnRegisterShutdownCallback(const sptr<ITakeOverShutdownCallback>& callback);

    void RegisterShutdownCallback(const sptr<IAsyncShutdownCallback>& callback, ShutdownPriority priority);
    void UnRegisterShutdownCallback(const sptr<IAsyncShutdownCallback>& callback);
    void RegisterShutdownCallback(const sptr<ISyncShutdownCallback>& callback, ShutdownPriority priority);
    void UnRegisterShutdownCallback(const sptr<ISyncShutdownCallback>& callback);

private:
    sptr<IRemoteObject> remote_;
    std::u16string descriptor_;
};
} // namespace PowerMgr
} // namespace OHOS

#endif // POWERMGR_POWER_MANAGER_SHUTDOWN_PROXY_H
