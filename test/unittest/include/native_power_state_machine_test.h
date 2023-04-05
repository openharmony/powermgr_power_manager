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

#ifndef NATIVE_POWERMGR_STATE_MACHINE_TEST_H
#define NATIVE_POWERMGR_STATE_MACHINE_TEST_H

#ifdef POWER_GTEST
#define private    public
#define protected  public
#endif

#include <map>
#include <memory>
#include <stdlib.h>

#include <gtest/gtest.h>

#include "power_mgr_client.h"
#include "power_mgr_service.h"
#include "power_state_callback_stub.h"

namespace OHOS {
namespace PowerMgr {
constexpr uint32_t THREE = 3;
constexpr int64_t TIME = -3;
constexpr int64_t CALLTIMEMS = 1;
constexpr pid_t PID = 1;
constexpr uint32_t MAXTYPE = 77;
constexpr int64_t TIMEOUT = 7;

class NativePowerStateMachineTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

class PowerStateTest1Callback : public PowerStateCallbackStub {
public:
    PowerStateTest1Callback() {};
    virtual ~PowerStateTest1Callback() {};
    virtual void OnPowerStateChanged(PowerState state) override;
};

class PowerStateTest2Callback : public PowerStateCallbackStub {
public:
    PowerStateTest2Callback() {};
    virtual ~PowerStateTest2Callback() {};
    virtual void OnPowerStateChanged(PowerState state) override;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // NATIVE_POWERMGR_STATE_MACHINE_TEST_H
