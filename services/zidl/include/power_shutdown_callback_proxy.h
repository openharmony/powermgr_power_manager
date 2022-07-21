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

#ifndef POWERMGR_SERVICES_SHUTDOWN_STATE_CALLBACK_PROXY_H
#define POWERMGR_SERVICES_SHUTDOWN_STATE_CALLBACK_PROXY_H

#include <iremote_proxy.h>
#include <functional>
#include "iremote_broker.h"
#include "refbase.h"

#include "ishutdown_callback.h"

namespace OHOS {
namespace PowerMgr {
class PowerShutdownCallbackProxy : public IRemoteProxy<IShutdownCallback> {
public:
    explicit PowerShutdownCallbackProxy(const sptr<IRemoteObject>& impl)
        : IRemoteProxy<IShutdownCallback>(impl) {}
    virtual ~PowerShutdownCallbackProxy() = default;
    virtual void ShutdownCallback() override;

private:
    static inline BrokerDelegator<PowerShutdownCallbackProxy> delegator_;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_SERVICES_SHUTDOWN_STATE_CALLBACK_PROXY_H
