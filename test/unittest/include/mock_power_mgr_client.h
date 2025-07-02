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

#ifndef MOCK_POWER_MGR_CLIENT_H
#define MOCK_POWER_MGR_CLIENT_H

#include "power_mgr_client.h"

namespace OHOS {
namespace PowerMgr {
class MockPowerMgrClient {
public:
    PowerErrors WakeupDevice(WakeupDeviceType reason = WakeupDeviceType::WAKEUP_DEVICE_APPLICATION,
        const std::string& detail = std::string("app call"))
    {
        if (static_cast<int32_t>(reason) <= static_cast<int32_t>(WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN) ||
            static_cast<int32_t>(reason) >= static_cast<int32_t>(WakeupDeviceType::WAKEUP_DEVICE_MAX)) {
            state_ = false;
            return  PowerErrors::ERR_OK;
        }
        if (detail == "pre_bright") {
            state_ = false;
        } else if (detail == "pre_bright_auth_success") {
            state_ = true;
        } else if (detail == "pre_bright_auth_fail_screen_off") {
            state_ = false;
        } else if (detail == "pre_bright_auth_fail_screen_on") {
            state_ = false;
        } else {
            state_ = true;
        }
        return PowerMgrClient::GetInstance().WakeupDevice(reason, detail);
    }

    PowerErrors SuspendDevice()
    {
        state_ = false;
        return PowerMgrClient::GetInstance().SuspendDevice();
    }

    PowerErrors OverrideScreenOffTime(int64_t timeout)
    {
        return PowerMgrClient::GetInstance().OverrideScreenOffTime(timeout);
    }

    PowerErrors RestoreScreenOffTime()
    {
        return PowerMgrClient::GetInstance().RestoreScreenOffTime();
    }

    bool IsScreenOn()
    {
        return state_;
    }

    static MockPowerMgrClient& GetInstance()
    {
        static MockPowerMgrClient instance;
        return static_cast<MockPowerMgrClient&>(instance);
    }
private:
    bool state_;
};
} // PowerMgr
} // OHOS
#endif // MOCK_POWER_MGR_CLIENT_H