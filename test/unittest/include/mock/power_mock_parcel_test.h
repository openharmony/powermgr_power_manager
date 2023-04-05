/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef POWER_MOCK_PARCEL_TEST_H
#define POWER_MOCK_PARCEL_TEST_H

#include <gtest/gtest.h>

#include "power_state_callback_stub.h"
#include "power_shutdown_callback_stub.h"
#include "power_mode_callback_stub.h"

namespace OHOS {
namespace PowerMgr {
class MockParcelTest : public testing::Test {
public:
    class PowerModeTestCallback : public PowerModeCallbackStub {
    public:
        PowerModeTestCallback() {};
        virtual ~PowerModeTestCallback() {};
        virtual void OnPowerModeChanged(PowerMode mode) override;
    };
    class PowerShutdownTestCallback : public PowerShutdownCallbackStub {
    public:
        PowerShutdownTestCallback() {};
        virtual ~PowerShutdownTestCallback() {};
        virtual void ShutdownCallback() override;
    };
    class PowerStateTestCallback : public PowerStateCallbackStub {
    public:
        PowerStateTestCallback() {};
        virtual ~PowerStateTestCallback() {};
        virtual void OnPowerStateChanged(PowerState state) override;
    };
};
} // namespace PowerMgr
} // namespace OHOS

#endif // POWER_MOCK_PARCEL_TEST_H
