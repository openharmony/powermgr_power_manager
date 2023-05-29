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

#ifndef POWERMGR_POWER_MANAGER_SHUTDOWN_CLIENT_H
#define POWERMGR_POWER_MANAGER_SHUTDOWN_CLIENT_H

#include <singleton.h>

#include "ipower_mgr.h"
#include "ishutdown_client.h"
#include "itakeover_shutdown_callback.h"
#include "iasync_shutdown_callback.h"
#include "isync_shutdown_callback.h"
#include "power_errors.h"

namespace OHOS {
namespace PowerMgr {
/**
 * Shutdown or reboot the client with related functions.
 */
class ShutdownClient final : public DelayedRefSingleton<ShutdownClient> {
    DECLARE_DELAYED_REF_SINGLETON(ShutdownClient)

public:
    DISALLOW_COPY_AND_MOVE(ShutdownClient);

    /**
     * Register a callback that takes over a shutdown or reboot.
     * <p>
     * Callbacks are executed in order of highest to lowest priority.
     *
     * @param callback Takes over the shutdown or reboot callbacks.
     * @param priority Change the priority of execution.
     */
    void RegisterShutdownCallback(
        const sptr<ITakeOverShutdownCallback>& callback, ShutdownPriority priority = ShutdownPriority::DEFAULT);

    /**
     * Unregister callbacks to take over shutdown or reboot.
     *
     * @param callback Registered callback to take over shutdown or reboot.
     */
    void UnRegisterShutdownCallback(const sptr<ITakeOverShutdownCallback>& callback);

    /**
     * Register the asynchronous shutdown callback interface
     * <p>
     * Callbacks are executed in order of highest to lowest priority.
     *
     * @param callback Asynchronous shutdown or reboot callbacks.
     * @param priority Change the priority of execution.
     */
    void RegisterShutdownCallback(
        const sptr<IAsyncShutdownCallback>& callback, ShutdownPriority priority = ShutdownPriority::DEFAULT);

    /**
     * Unregister the asynchronous shutdown callback interface.
     *
     * @param callback Registered callback to asynchronous shutdown or reboot.
     */
    void UnRegisterShutdownCallback(const sptr<IAsyncShutdownCallback>& callback);

    /**
     * Register the synchronous shutdown callback interface
     * <p>
     * Callbacks are executed in order of highest to lowest priority.
     *
     * @param callback Synchronous shutdown or reboot callbacks.
     * @param priority Change the priority of execution.
     */
    void RegisterShutdownCallback(
        const sptr<ISyncShutdownCallback>& callback, ShutdownPriority priority = ShutdownPriority::DEFAULT);

    /**
     * Unregister the synchronous shutdown callback interface.
     *
     * @param callback Registered callback to synchronous shutdown or reboot.
     */
    void UnRegisterShutdownCallback(const sptr<ISyncShutdownCallback>& callback);

private:
    ErrCode Connect();

    sptr<IPowerMgr> proxy_ {nullptr};
};
} // namespace PowerMgr
} // namespace OHOS

#endif // POWERMGR_POWER_MANAGER_SHUTDOWN_CLIENT_H
