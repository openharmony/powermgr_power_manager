/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "takeover_shutdown_callback_test.h"

#include <condition_variable>

#include "power_log.h"
#include "power_mgr_client.h"
#include "power_mgr_service.h"
#include "shutdown/shutdown_client.h"

#include "mock_power_action.h"
#include "mock_state_action.h"

namespace OHOS {
namespace PowerMgr {
namespace UnitTest {
namespace {
sptr<PowerMgrService> g_service;
MockPowerAction* g_mockPowerAction;
MockStateAction* g_mockStateAction;
bool g_isReboot;
bool g_isHighPriority;
bool g_isDefaultPriority;
bool g_isLowPriority;
}
using namespace testing::ext;

void TakeOverShutdownCallbackTest::SetUpTestCase()
{
    // create singleton service object at the beginning
    g_service = DelayedSpSingleton<PowerMgrService>::GetInstance();
    g_service->OnStart();
}

void TakeOverShutdownCallbackTest::TearDownTestCase()
{
    g_service->OnStop();
    DelayedSpSingleton<PowerMgrService>::DestroyInstance();
}

void TakeOverShutdownCallbackTest::SetUp()
{
    g_isReboot = false;
    g_isHighPriority = false;
    g_isDefaultPriority = false;
    g_isLowPriority = false;
    g_mockPowerAction = new MockPowerAction();
    g_mockStateAction = new MockStateAction();
    auto shutdownController = g_service->GetShutdownController();
    shutdownController->EnableMock(g_mockPowerAction, g_mockStateAction);
}

void TakeOverShutdownCallbackTest::TearDown()
{}

bool TakeOverShutdownCallbackTest::TakeOverShutdownCallback::OnTakeOverShutdown(bool isReboot)
{
    POWER_HILOGI(LABEL_TEST, "OnTakeOverShutdown called, isReboot=%{public}d", isReboot);
    g_isReboot = isReboot;
    g_isDefaultPriority = true;
    return true; // Take over the shutdown
}

bool TakeOverShutdownCallbackTest::HighPriorityTakeOverShutdownCallback::OnTakeOverShutdown(bool isReboot)
{
    g_isHighPriority = true;
    return true; // Take over the shutdown
}

bool TakeOverShutdownCallbackTest::LowPriorityTakeOverShutdownCallback::OnTakeOverShutdown(bool isReboot)
{
    g_isLowPriority = true;
    return true; // Take over the shutdown
}

bool TakeOverShutdownCallbackTest::NotTakeOverShutdownCallback::OnTakeOverShutdown(bool isReboot)
{
    POWER_HILOGI(LABEL_TEST, "OnTakeOverShutdown called, isReboot=%{public}d", isReboot);
    return false; // Do not take over the shutdown
}

/**
 * @tc.name: TakeOverShutdownCallback001
 * @tc.desc: Test takeover shutdown callback for shutdown and reboot
 * @tc.type: FUNC
 */
HWTEST_F(TakeOverShutdownCallbackTest, TakeOverShutdownCallback001, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "TakeOverShutdownCallback001 start");
    auto& shutdownClient = ShutdownClient::GetInstance();
    auto callback = new TakeOverShutdownCallback();
    shutdownClient.RegisterShutdownCallback(callback);

    auto& powerMgrClient = PowerMgrClient::GetInstance();

    EXPECT_CALL(*g_mockPowerAction, Reboot(std::string("test_reboot"))).Times(0);
    powerMgrClient.RebootDevice("test_reboot"); // reboot will be taken over
    EXPECT_TRUE(g_isReboot); // The callback param will be true for reboot

    EXPECT_CALL(*g_mockPowerAction, Shutdown(std::string("test_shutdown"))).Times(0);
    powerMgrClient.ShutDownDevice("test_shutdown"); // shutdown will be taken over
    EXPECT_FALSE(g_isReboot); // The callback param will be false for shutdown
    POWER_HILOGI(LABEL_TEST, "TakeOverShutdownCallback001 end");
}

/**
 * @tc.name: ITakeOverShutdownCallback002
 * @tc.desc: Test the low and default priority of takeover shutdown callback
 * @tc.type: FUNC
 */
HWTEST_F(TakeOverShutdownCallbackTest, ITakeOverShutdownCallback002, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "TakeOverShutdownCallback002 start");
    auto& shutdownClient = ShutdownClient::GetInstance();
    auto lowPriorityCallback = new LowPriorityTakeOverShutdownCallback();
    shutdownClient.RegisterShutdownCallback(lowPriorityCallback, ShutdownPriority::LOW);
    auto defaultPriorityCallback = new TakeOverShutdownCallback();
    shutdownClient.RegisterShutdownCallback(defaultPriorityCallback, ShutdownPriority::DEFAULT);

    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.ShutDownDevice("test_shutdown"); // shutdown will be taken over

    EXPECT_TRUE(g_isDefaultPriority); // Default priority callback will be called
    EXPECT_FALSE(g_isLowPriority); // Low Priority callback will not be called
    POWER_HILOGI(LABEL_TEST, "TakeOverShutdownCallback002 end");
}

/**
 * @tc.name: TakeOverShutdownCallback003
 * @tc.desc: Test the low and high priority of takeover shutdown callback
 * @tc.type: FUNC
 */
HWTEST_F(TakeOverShutdownCallbackTest, TakeOverShutdownCallback003, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "TakeOverShutdownCallback003 start");
    auto& shutdownClient = ShutdownClient::GetInstance();
    auto lowPriorityCallback = new LowPriorityTakeOverShutdownCallback();
    shutdownClient.RegisterShutdownCallback(lowPriorityCallback, ShutdownPriority::LOW);
    auto highPriorityCallback = new HighPriorityTakeOverShutdownCallback();
    shutdownClient.RegisterShutdownCallback(highPriorityCallback, ShutdownPriority::HIGH);

    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.ShutDownDevice("test_shutdown"); // shutdown will be taken over

    EXPECT_TRUE(g_isHighPriority); // High priority callback will be called
    EXPECT_FALSE(g_isLowPriority); // Low Priority callback will not be called
    POWER_HILOGI(LABEL_TEST, "TakeOverShutdownCallback003 end");
}

/**
 * @tc.name: TakeOverShutdownCallback004
 * @tc.desc: Test the default and high priority of takeover shutdown callback
 * @tc.type: FUNC
 */
HWTEST_F(TakeOverShutdownCallbackTest, TakeOverShutdownCallback004, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "TakeOverShutdownCallback004 start");
    auto& shutdownClient = ShutdownClient::GetInstance();
    auto defaultPriorityCallback = new TakeOverShutdownCallback();
    shutdownClient.RegisterShutdownCallback(defaultPriorityCallback, ShutdownPriority::DEFAULT);
    auto highPriorityCallback = new HighPriorityTakeOverShutdownCallback();
    shutdownClient.RegisterShutdownCallback(highPriorityCallback, ShutdownPriority::HIGH);

    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.ShutDownDevice("test_shutdown"); // shutdown will be taken over

    EXPECT_TRUE(g_isHighPriority); // High priority callback will be called
    EXPECT_FALSE(g_isDefaultPriority); // Default Priority callback will not be called
    POWER_HILOGI(LABEL_TEST, "TakeOverShutdownCallback004 end");
}

/**
 * @tc.name: TakeOverShutdownCallback005
 * @tc.desc: Test do not takeover the shutdown
 * @tc.type: FUNC
 */
HWTEST_F(TakeOverShutdownCallbackTest, TakeOverShutdownCallback005, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "TakeOverShutdownCallback005 start");
    auto& shutdownClient = ShutdownClient::GetInstance();
    auto notTakeOverCallback = new NotTakeOverShutdownCallback();
    shutdownClient.RegisterShutdownCallback(notTakeOverCallback);

    auto& powerMgrClient = PowerMgrClient::GetInstance();
    EXPECT_CALL(*g_mockPowerAction, Reboot(std::string("test_reboot"))).Times(1);
    powerMgrClient.RebootDevice("test_reboot"); // reboot will not be taken over

    EXPECT_CALL(*g_mockPowerAction, Shutdown(std::string("test_shutdown"))).Times(1);
    powerMgrClient.ShutDownDevice("test_shutdown"); // shutdown will not be taken over
    POWER_HILOGI(LABEL_TEST, "TakeOverShutdownCallback005 end");
}
} // namespace UnitTest
} // namespace PowerMgr
} // namespace OHOS
