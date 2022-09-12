/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef IPOWER_MGR_CHANGE_MODE_CALLBACK_H
#define IPOWER_MGR_CHANGE_MODE_CALLBACK_H

#include <iremote_object.h>
#include <iremote_proxy.h>
#include <iremote_stub.h>
#include <iremote_object.h>
#include "power_mode_info.h"

namespace OHOS {
namespace PowerMgr {
class IPowerModeCallback : public IRemoteBroker {
public:
    enum {
        POWER_MODE_CHANGED = 0,
    };

    virtual void OnPowerModeChanged(PowerMode mode) = 0;

    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.powermgr.IPowerModeCallback");
};
} // namespace PowerMgr
} // namespace OHOS
#endif // IPOWER_MGR_CHANGE_MODE_CALLBACK_H
