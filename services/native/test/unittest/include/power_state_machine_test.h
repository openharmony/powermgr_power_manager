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

#ifndef POWERMGR_STATE_MACHINE_TEST_H
#define POWERMGR_STATE_MACHINE_TEST_H

#include <gtest/gtest.h>
#include <thread>

#include "power_mgr_service.h"
#include "power_state_callback_stub.h"
namespace OHOS {
namespace PowerMgr {
constexpr int NEXT_WAIT_TIME_S = 1;
constexpr int SLEEP_WAIT_TIME_S = 6;
constexpr int REFRESHACTIVITY_WAIT_TIME_S = 8;
constexpr int SCREEN_OFF_WAIT_TIME_S = 15;

class PowerStateMachineTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    void PowerClientInit();
    void CheckSuspendBlock();
    void CheckSuspendUnblock();
    void CheckStartSuspend();
    void CheckStopSuspend();
    void WakeUpthread();
    void Suspendthread();
    void Rebootthread();
    void Shutdownthread();
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
private:
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_STATE_MACHINE_TEST_H