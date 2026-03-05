/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef POWERMGR_RUNNING_LOCK_CHANGED_CALLBACK_PROXY_H
#define POWERMGR_RUNNING_LOCK_CHANGED_CALLBACK_PROXY_H

#include <iremote_proxy.h>
#include "refbase.h"
#include "iremote_broker.h"
#include "iremote_object.h"

#include "irunning_lock_changed_callback.h"

namespace OHOS {
namespace PowerMgr {
class RunningLockChangedCallbackProxy : public IRemoteProxy<IRunningLockChangedCallback> {
public:
    explicit RunningLockChangedCallbackProxy(const sptr<IRemoteObject>& impl)
        : IRemoteProxy<IRunningLockChangedCallback>(impl) {}
    virtual ~RunningLockChangedCallbackProxy() = default;
    virtual void OnAsyncScreenRunningLockChanged(RunningLockChangeState state) override;

private:
    static inline BrokerDelegator<RunningLockChangedCallbackProxy> delegator_;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_RUNNING_LOCK_CHANGED_CALLBACK_PROXY_H
