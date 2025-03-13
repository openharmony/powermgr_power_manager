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

#include "sync_shutdown_callback_test.h"

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
sptr<PowerMgrService> g_service = nullptr;
MockPowerAction* g_mockPowerAction = nullptr;
MockStateAction* g_mockStateAction = nullptr;
bool g_isHighPriority = false;
bool g_isDefaultPriority = false;
bool g_isLowPriority = false;
}
using namespace testing::ext;

void SyncShutdownCallbackTest::SetUpTestCase()
{
    // create singleton service object at the beginning
    g_service = DelayedSpSingleton<PowerMgrService>::GetInstance();
    g_service->OnStart();
}

void SyncShutdownCallbackTest::TearDownTestCase()
{
    g_service->OnStop();
    DelayedSpSingleton<PowerMgrService>::DestroyInstance();
}

void SyncShutdownCallbackTest::SetUp()
{
    g_isHighPriority = false;
    g_isDefaultPriority = false;
    g_isLowPriority = false;
    g_mockPowerAction = new MockPowerAction();
    g_mockStateAction = new MockStateAction();
    auto shutdownController = g_service->GetShutdownController();
    shutdownController->EnableMock(g_mockPowerAction, g_mockStateAction);
}

void SyncShutdownCallbackTest::TearDown()
{}

void SyncShutdownCallbackTest::SyncShutdownCallback::OnSyncShutdown()
{
    g_isDefaultPriority = true;
}

void SyncShutdownCallbackTest::HighPrioritySyncShutdownCallback::OnSyncShutdown()
{
    g_isHighPriority = true;
}

void SyncShutdownCallbackTest::LowPrioritySyncShutdownCallback::OnSyncShutdown()
{
    g_isLowPriority = true;
}

void SyncShutdownCallbackTest::NotSyncShutdownCallback::OnSyncShutdown()
{
}

void SyncShutdownCallbackTest::SyncShutdownOrRebootCallback::OnSyncShutdownOrReboot(bool isReboot)
{
    g_isDefaultPriority = true;
}

void SyncShutdownCallbackTest::HighPrioritySyncShutdownOrRebootCallback::OnSyncShutdownOrReboot(bool isReboot)
{
    g_isHighPriority = true;
}

void SyncShutdownCallbackTest::LowPrioritySyncShutdownOrRebootCallback::OnSyncShutdownOrReboot(bool isReboot)
{
    g_isLowPriority = true;
}

void SyncShutdownCallbackTest::NotSyncShutdownOrRebootCallback::OnSyncShutdownOrReboot(bool isReboot)
{
}

/**
 * @tc.name: SyncShutdownCallback001
 * @tc.desc: Test synchronous shutdown callback for shutdown and reboot
 * @tc.type: FUNC
 */
HWTEST_F(SyncShutdownCallbackTest, SyncShutdownCallback001, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "SyncShutdownCallback001 function start!");
    auto callback = new SyncShutdownCallback();
    g_service->RegisterShutdownCallback(callback, ShutdownPriority::DEFAULT);

    auto callback2 = new SyncShutdownOrRebootCallback();
    g_service->RegisterShutdownCallback(callback2, ShutdownPriority::DEFAULT);

    g_service->RebootDevice("test_case");
    EXPECT_TRUE(g_isDefaultPriority);

    g_service->ShutDownDevice("test_case");
    EXPECT_TRUE(g_isDefaultPriority);
    POWER_HILOGI(LABEL_TEST, "SyncShutdownCallback001 function end!");
}

/**
 * @tc.name: SyncShutdownCallback002
 * @tc.desc: Test the low and default priority of synchronous shutdown callback
 * @tc.type: FUNC
 */
HWTEST_F(SyncShutdownCallbackTest, SyncShutdownCallback002, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "SyncShutdownCallback002 function start!");
    auto lowPriorityCallback = new LowPrioritySyncShutdownCallback();
    g_service->RegisterShutdownCallback(lowPriorityCallback, ShutdownPriority::LOW);
    auto defaultPriorityCallback = new SyncShutdownCallback();
    g_service->RegisterShutdownCallback(defaultPriorityCallback, ShutdownPriority::DEFAULT);

    g_service->ShutDownDevice("test_case");
    EXPECT_TRUE(g_isDefaultPriority);
    EXPECT_TRUE(g_isLowPriority);
    POWER_HILOGI(LABEL_TEST, "SyncShutdownCallback002 function end!");
}

/**
 * @tc.name: SyncShutdownCallback003
 * @tc.desc: Test the low and high priority of synchronous shutdown callback
 * @tc.type: FUNC
 */
HWTEST_F(SyncShutdownCallbackTest, SyncShutdownCallback003, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "SyncShutdownCallback003 function start!");
    auto lowPriorityCallback = new LowPrioritySyncShutdownCallback();
    g_service->RegisterShutdownCallback(lowPriorityCallback, ShutdownPriority::LOW);
    auto highPriorityCallback = new HighPrioritySyncShutdownCallback();
    g_service->RegisterShutdownCallback(highPriorityCallback, ShutdownPriority::HIGH);

    g_service->ShutDownDevice("test_case");
    EXPECT_TRUE(g_isHighPriority);
    EXPECT_TRUE(g_isLowPriority);
    POWER_HILOGI(LABEL_TEST, "SyncShutdownCallback003 function end!");
}

/**
 * @tc.name: SyncShutdownCallback004
 * @tc.desc: Test the default and high priority of synchronous shutdown callback
 * @tc.type: FUNC
 */
HWTEST_F(SyncShutdownCallbackTest, SyncShutdownCallback004, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "SyncShutdownCallback004 function start!");
    auto defaultPriorityCallback = new SyncShutdownCallback();
    g_service->RegisterShutdownCallback(defaultPriorityCallback, ShutdownPriority::DEFAULT);
    auto highPriorityCallback = new HighPrioritySyncShutdownCallback();
    g_service->RegisterShutdownCallback(highPriorityCallback, ShutdownPriority::HIGH);

    g_service->ShutDownDevice("test_case");
    EXPECT_TRUE(g_isHighPriority);
    EXPECT_TRUE(g_isDefaultPriority);
    POWER_HILOGI(LABEL_TEST, "SyncShutdownCallback004 function end!");
}

/**
 * @tc.name: SyncShutdownCallback005
 * @tc.desc: Test do not synchronous shutdown
 * @tc.type: FUNC
 */
HWTEST_F(SyncShutdownCallbackTest, SyncShutdownCallback005, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "SyncShutdownCallback005 function start!");
    auto notSyncCallback = new NotSyncShutdownCallback();
    g_service->RegisterShutdownCallback(notSyncCallback, ShutdownPriority::DEFAULT);

    EXPECT_CALL(*g_mockPowerAction, Reboot(std::string("test_case"))).Times(::testing::AtLeast(1));
    g_service->RebootDevice("test_case");

    EXPECT_CALL(*g_mockPowerAction, Shutdown(std::string("test_case"))).Times(::testing::AtLeast(1));
    g_service->ShutDownDevice("test_case");
    POWER_HILOGI(LABEL_TEST, "SyncShutdownCallback005 function end!");
}
} // namespace UnitTest
} // namespace PowerMgr
} // namespace OHOS
