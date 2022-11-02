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

#include "power_mgr_factory.h"

#include "device_power_action.h"
#ifdef HAS_DISPLAY_MANAGER_PART
#include "display/device_state_action.h"
#else
#include "display/default_device_state_action.h"
#endif
#include "running_lock_action.h"

using namespace std;

namespace OHOS {
namespace PowerMgr {
unique_ptr<IDevicePowerAction> PowerMgrFactory::GetDevicePowerAction()
{
    return make_unique<DevicePowerAction>();
}

unique_ptr<IDeviceStateAction> PowerMgrFactory::GetDeviceStateAction()
{
#ifdef HAS_DISPLAY_MANAGER_PART
    return make_unique<DeviceStateAction>();
#else
    return make_unique<DefaultDeviceStateAction>();
#endif
}

unique_ptr<IRunningLockAction> PowerMgrFactory::GetRunningLockAction()
{
    return make_unique<RunningLockAction>();
}
} // namespace PowerMgr
} // namespace OHOS
