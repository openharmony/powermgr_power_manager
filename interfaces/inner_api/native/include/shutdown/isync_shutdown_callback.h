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

#ifndef POWERMGR_POWER_MANAGER_ISYNC_SHUTDOWN_CALLBACK_H
#define POWERMGR_POWER_MANAGER_ISYNC_SHUTDOWN_CALLBACK_H

#include <iremote_broker.h>
#include "shutdown_priority.h"

namespace OHOS {
namespace PowerMgr {
class ISyncShutdownCallback : public IRemoteBroker {
public:
    /**
     * Synchronous shutdown callback interface.
     * <p>
     * Execute the shutdown callback synchronously and wait for the callback method to return.
     * Execute from high priority to low priority.
     */
    virtual void OnSyncShutdown() = 0;
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.powermgr.ISyncShutdownCallback");
};
} // namespace PowerMgr
} // namespace OHOS

#endif // POWERMGR_POWER_MANAGER_ISYNC_SHUTDOWN_CALLBACK_H
