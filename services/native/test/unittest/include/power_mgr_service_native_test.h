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

#ifndef POWERMGR_MGR_SERVICE_NATIVE_TEST_H
#define POWERMGR_MGR_SERVICE_NATIVE_TEST_H

#ifdef POWER_GTEST
#define private    public
#define protected  public
#endif

#include <gtest/gtest.h>
#include "mock_lock_action.h"
#include "mock_power_action.h"
#include "mock_state_action.h"
#include "sensor_agent.h"
#include "power_state_callback_stub.h"

namespace OHOS {
namespace PowerMgr {
constexpr int32_t TIMEOUTMS = 7;
constexpr int64_t CALLTIMEMS = 1;
constexpr int64_t SUSCALLTIMEMS = 3;
constexpr pid_t PID = 1;
constexpr pid_t UID = 1;
constexpr int32_t UNCANCELID = -1;
constexpr int64_t UNCALLTIMEMS = -7;
constexpr uint32_t LID_CLOSED_HALL_FLAG = 0x1;
constexpr uint32_t LID_CLOSED_HALL_FLAG_B = 0x2;

class PowerMgrServiceNativeTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
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
