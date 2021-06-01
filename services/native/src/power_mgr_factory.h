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

#ifndef POWERMGR_POWER_MGR_FACTORY_H
#define POWERMGR_POWER_MGR_FACTORY_H

#include <memory>

#include "actions/idevice_power_action.h"
#include "actions/idevice_state_action.h"
#include "actions/irunning_lock_action.h"

namespace OHOS {
namespace PowerMgr {
class PowerMgrFactory {
public:
    static std::unique_ptr<IDevicePowerAction> GetDevicePowerAction();
    static std::unique_ptr<IDeviceStateAction> GetDeviceStateAction();
    static std::unique_ptr<IRunningLockAction> GetRunningLockAction();
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_POWER_MGR_FACTORY_H
