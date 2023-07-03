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

#ifndef POWERMGR_POWER_MANAGER_IASYNC_SHUTDOWN_CALLBACK_H
#define POWERMGR_POWER_MANAGER_IASYNC_SHUTDOWN_CALLBACK_H

#include <iremote_broker.h>
#include "shutdown_priority.h"

namespace OHOS {
namespace PowerMgr {
class IAsyncShutdownCallback : public IRemoteBroker {
public:
    /**
     * Asynchronous shutdown callback interface.
     * <p>
     * Asynchronously execute the shutdown callback. Wait for a maximum of 30 seconds.
     * If this timeout occurs, the asynchronous task will be interrupted and the shutdown will be forcibly executed.
     * Execute from high priority to low priority.
     */
    virtual void OnAsyncShutdown() = 0;
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.powermgr.IAsyncShutdownCallback");
};
} // namespace PowerMgr
} // namespace OHOS

#endif // POWERMGR_POWER_MANAGER_IASYNC_SHUTDOWN_CALLBACK_H
