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

#ifndef POWERMGR_POWER_MODE_TEST_H
#define POWERMGR_POWER_MODE_TEST_H

#include <gtest/gtest.h>

#include "power_mgr_service.h"
#include "power_state_callback_stub.h"

namespace OHOS {
namespace PowerMgr {
constexpr int SLEEP_WAIT_TIME_S = 6;
class PowerDeviceModeTest : public testing::Test {
public:
    
    class PowerModeTest1Callback : public IRemoteStub<IPowerModeCallback> {
    public:
        PowerModeTest1Callback() {};
        virtual ~PowerModeTest1Callback() {};
        virtual void OnPowerModeChanged(PowerMode mode) override;
    };
    class PowerModeTest2Callback : public IRemoteStub<IPowerModeCallback> {
    public:
        PowerModeTest2Callback() {};
        virtual ~PowerModeTest2Callback() {};
        virtual void OnPowerModeChanged(PowerMode mode) override;
    };
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_POWER_MODE_TEST_H
