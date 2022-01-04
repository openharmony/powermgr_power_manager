/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef IPOWER_MGR_SHUTDOWN_CALLBACK_H
#define IPOWER_MGR_SHUTDOWN_CALLBACK_H

#include <iremote_broker.h>
#include <iremote_object.h>
#include <iremote_proxy.h>
#include <iremote_stub.h>

namespace OHOS {
namespace PowerMgr {
class IShutdownCallback : public IRemoteBroker {
public:
    enum {
        POWER_SHUTDOWN_CHANGED = 0,
    };
    enum class ShutdownPriority : uint32_t {
        POWER_SHUTDOWN_PRIORITY_LOW = 0,
        POWER_SHUTDOWN_PRIORITY_DEFAULT,
        POWER_SHUTDOWN_PRIORITY_HIGH,
    };

    virtual void ShutdownCallback() = 0;
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.powermgr.IShutdownCallback");
};
} // namespace PowerMgr
} // namespace OHOS
#endif // IPOWER_MGR_SHUTDOWN_CALLBACK_H
