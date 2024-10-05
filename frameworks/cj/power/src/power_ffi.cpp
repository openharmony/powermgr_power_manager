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

#include "power_mgr_client.h"
#include "power_ffi.h"

namespace OHOS {
namespace PowerMgr {
static PowerMgrClient& g_powerMgrClient = PowerMgrClient::GetInstance();
extern "C" {
    bool FfiPowerIsActive()
    {
        bool isScreen = g_powerMgrClient.IsScreenOn();
        return isScreen;
    }

    uint32_t FfiPowerGetPowerMode()
    {
        PowerMode mode = g_powerMgrClient.GetDeviceMode();
        return static_cast<uint32_t>(mode);
    }

    bool FfiPowerIsStandby(int32_t &code)
    {
        bool isStandby = false;
        PowerErrors errCode = g_powerMgrClient.IsStandby(isStandby);
        if (errCode != PowerErrors::ERR_OK) {
            code = static_cast<int32_t>(errCode);
        }
        return isStandby;
    }
}
} // namespace PowerMgr
} // namespace OHOS