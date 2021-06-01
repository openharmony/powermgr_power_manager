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

#ifndef POWERMGR_IPOWER_STATE_CALLBACK_H
#define POWERMGR_IPOWER_STATE_CALLBACK_H

#include <iremote_broker.h>
#include <iremote_object.h>

#include "power_state_machine_info.h"

namespace OHOS {
namespace PowerMgr {
class IPowerStateCallback : public IRemoteBroker {
public:
    enum {
        POWER_STATE_CHANGED = 0,
    };
    virtual void OnPowerStateChanged(PowerState state) = 0;

    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.powermgr.IPowerStateCallback");
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_IPOWER_STATE_CALLBACK_H
