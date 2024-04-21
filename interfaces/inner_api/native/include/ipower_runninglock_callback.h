/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef IPOWER_RUNNINGLOCK_CALLBACK_H
#define IPOWER_RUNNINGLOCK_CALLBACK_H

#include <iremote_object.h>
#include <iremote_proxy.h>
#include <iremote_stub.h>

namespace OHOS {
namespace PowerMgr {
class IPowerRunninglockCallback : public IRemoteBroker {
public:
    virtual void HandleRunningLockMessage(std::string message) = 0;

    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.powermgr.IPowerRunninglockCallback");
};
} // namespace PowerMgr
} // namespace OHOS
#endif // IPOWER_RUNNINGLOCK_CALLBACK_H