/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef POWERMGR_MGR_SERVICE_NATIVE_TEST_H
#define POWERMGR_MGR_SERVICE_NATIVE_TEST_H

#ifdef POWER_GTEST
#define private   public
#define protected public
#endif

#include "mock_lock_action.h"
#include "mock_power_action.h"
#include "mock_state_action.h"
#include "power_state_callback_stub.h"
#include "sensor_agent.h"
#include <gtest/gtest.h>

namespace OHOS {
namespace PowerMgr {
class PowerMgrServiceNativeTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};

class PowerStateTestCallback : public PowerStateCallbackStub {
public:
    PowerStateTestCallback() {};
    virtual ~PowerStateTestCallback() {};
    virtual void OnPowerStateChanged(PowerState state) override;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_MGR_SERVICE_NATIVE_TEST_H