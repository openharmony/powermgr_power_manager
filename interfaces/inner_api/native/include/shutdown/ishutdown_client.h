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

#ifndef POWERMGR_POWER_MANAGER_ISHUTDOWN_H
#define POWERMGR_POWER_MANAGER_ISHUTDOWN_H

#include "iremote_broker.h"
#include "itakeover_shutdown_callback.h"
#include "iasync_shutdown_callback.h"
#include "isync_shutdown_callback.h"

namespace OHOS {
namespace PowerMgr {
class IShutdownClient {
public:
    virtual void RegisterShutdownCallback(
        const sptr<ITakeOverShutdownCallback>& callback, ShutdownPriority priority) = 0;
    virtual void UnRegisterShutdownCallback(const sptr<ITakeOverShutdownCallback>& callback) = 0;

    virtual void RegisterShutdownCallback(
        const sptr<IAsyncShutdownCallback>& callback, ShutdownPriority priority) = 0;
    virtual void UnRegisterShutdownCallback(const sptr<IAsyncShutdownCallback>& callback) = 0;
    virtual void RegisterShutdownCallback(
        const sptr<ISyncShutdownCallback>& callback, ShutdownPriority priority) = 0;
    virtual void UnRegisterShutdownCallback(const sptr<ISyncShutdownCallback>& callback) = 0;
};
} // namespace PowerMgr
} // namespace OHOS

#endif // POWERMGR_POWER_MANAGER_ISHUTDOWN_H
