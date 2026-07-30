/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef POWERMGR_POWER_MANAGER_ULSR_CALLBACK_PROXY_H
#define POWERMGR_POWER_MANAGER_ULSR_CALLBACK_PROXY_H

#include <iremote_proxy.h>
#include <nocopyable.h>
#include <functional>
#include "refbase.h"
#include "iremote_broker.h"
#include "iremote_object.h"

#include "ulsr/iulsr_callback.h"

namespace OHOS {
namespace PowerMgr {
class UlsrCallbackProxy : public IRemoteProxy<IUlsrCallback> {
public:
    explicit UlsrCallbackProxy(const sptr<IRemoteObject>& impl)
        : IRemoteProxy<IUlsrCallback>(impl) {}
    ~UlsrCallbackProxy() = default;
    virtual void OnSyncUlsr() override;
    virtual void OnAsyncWakeup(bool ulsrResult = false) override;

private:
    static inline BrokerDelegator<UlsrCallbackProxy> delegator_;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_POWER_MANAGER_ULSR_CALLBACK_PROXY_H