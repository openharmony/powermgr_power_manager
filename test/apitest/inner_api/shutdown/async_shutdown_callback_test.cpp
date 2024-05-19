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

#include "async_shutdown_callback_test.h"

#include <condition_variable>
#include <future>
#include <mutex>
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
std::condition_variable g_cv;
std::mutex g_mtx;
bool g_isHighPriority = false;
bool g_isDefaultPriority = false;
bool g_isLowPriority = false;
const int32_t TIMEOUT_SEC = 5;
}
using namespace testing::ext;
using namespace std;
void AsyncShutdownCallbackTest::SetUpTestCase()
{
    // create singleton service object at the beginning
    g_service = DelayedSpSingleton<PowerMgrService>::GetInstance();
    g_service->OnStart();
}

void AsyncShutdownCallbackTest::TearDownTestCase()
{
    g_service->OnStop();
    DelayedSpSingleton<PowerMgrService>::DestroyInstance();
}

void AsyncShutdownCallbackTest::SetUp()
{
    g_isHighPriority = false;
    g_isDefaultPriority = false;
    g_isLowPriority = false;
    g_mockPowerAction = new MockPowerAction();
    g_mockStateAction = new MockStateAction();
    auto shutdownController = g_service->GetShutdownController();
    if (shutdownController->IsShuttingDown()) {
        // wait for detached threads to finish before next testcase
        sleep(1);
    }
    shutdownController->EnableMock(g_mockPowerAction, g_mockStateAction);
}

void AsyncShutdownCallbackTest::TearDown()
{}

void AsyncShutdownCallbackTest::AsyncShutdownCallback::OnAsyncShutdown()
{
    g_isDefaultPriority = true;
    g_cv.notify_one();
}

void AsyncShutdownCallbackTest::HighPriorityAsyncShutdownCallback::OnAsyncShutdown()
{
    g_isHighPriority = true;
    g_cv.notify_one();
}

void AsyncShutdownCallbackTest::LowPriorityAsyncShutdownCallback::OnAsyncShutdown()
{
    g_isLowPriority = true;
    g_cv.notify_one();
}

void AsyncShutdownCallbackTest::NotAsyncShutdownCallback::OnAsyncShutdown()
{
}

static bool WaitingCallback(bool &isPriority)
{
    std::unique_lock<std::mutex> lck(g_mtx);
    auto callbackStart = [&]() {
        while (!isPriority) {
            g_cv.wait(lck);
        }
    };

    packaged_task<void()> callbackTask(callbackStart);
    future<void> fut = callbackTask.get_future();
    make_unique<thread>(std::move(callbackTask))->detach();
    future_status status = fut.wait_for(std::chrono::seconds(TIMEOUT_SEC));
    if (status == future_status::timeout) {
        return false;
    }
    return true;
}

/**
 * @tc.name: AsyncShutdownCallbackk001
 * @tc.desc: Test asynchronous shutdown callback for shutdown and reboot
 * @tc.type: FUNC
 */
HWTEST_F(AsyncShutdownCallbackTest, AsyncShutdownCallbackk001, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "AsyncShutdownCallbackk001 start");
    auto callback = new AsyncShutdownCallback();
    g_service->RegisterShutdownCallback(callback, ShutdownPriority::DEFAULT);

    g_service->RebootDevice("test_reboot");
    EXPECT_TRUE(WaitingCallback(g_isDefaultPriority));

    g_service->ShutDownDevice("test_shutdown");
    EXPECT_TRUE(WaitingCallback(g_isDefaultPriority));
    POWER_HILOGI(LABEL_TEST, "AsyncShutdownCallback001 end");
}

/**
 * @tc.name: AsyncShutdownCallbackk002
 * @tc.desc: Test the low and default priority of asynchronous shutdown callback
 * @tc.type: FUNC
 */
HWTEST_F(AsyncShutdownCallbackTest, AsyncShutdownCallbackk002, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "AsyncShutdownCallbackk002 start");
    auto lowPriorityCallback = new LowPriorityAsyncShutdownCallback();
    g_service->RegisterShutdownCallback(lowPriorityCallback, ShutdownPriority::LOW);
    auto defaultPriorityCallback = new AsyncShutdownCallback();
    g_service->RegisterShutdownCallback(defaultPriorityCallback, ShutdownPriority::DEFAULT);

    g_service->ShutDownDevice("test_shutdown");
    EXPECT_TRUE(WaitingCallback(g_isDefaultPriority));
    EXPECT_TRUE(WaitingCallback(g_isLowPriority));
    POWER_HILOGI(LABEL_TEST, "AsyncShutdownCallbackk002 end");
}

/**
 * @tc.name: AsyncShutdownCallbackk003
 * @tc.desc: Test the low and high priority of asynchronous shutdown callback
 * @tc.type: FUNC
 */
HWTEST_F(AsyncShutdownCallbackTest, AsyncShutdownCallbackk003, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "AsyncShutdownCallbackk003 start");
    auto lowPriorityCallback = new LowPriorityAsyncShutdownCallback();
    g_service->RegisterShutdownCallback(lowPriorityCallback, ShutdownPriority::LOW);
    auto highPriorityCallback = new HighPriorityAsyncShutdownCallback();
    g_service->RegisterShutdownCallback(highPriorityCallback, ShutdownPriority::HIGH);

    g_service->ShutDownDevice("test_shutdown");
    EXPECT_TRUE(WaitingCallback(g_isHighPriority));
    EXPECT_TRUE(WaitingCallback(g_isLowPriority));
    POWER_HILOGI(LABEL_TEST, "AsyncShutdownCallbackk003 end");
}

/**
 * @tc.name: AsyncShutdownCallback004
 * @tc.desc: Test the default and high priority of asynchronous shutdown callback
 * @tc.type: FUNC
 */
HWTEST_F(AsyncShutdownCallbackTest, AsyncShutdownCallback004, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "AsyncShutdownCallback004 start");
    auto defaultPriorityCallback = new AsyncShutdownCallback();
    g_service->RegisterShutdownCallback(defaultPriorityCallback, ShutdownPriority::DEFAULT);
    auto highPriorityCallback = new HighPriorityAsyncShutdownCallback();
    g_service->RegisterShutdownCallback(highPriorityCallback, ShutdownPriority::HIGH);

    g_service->ShutDownDevice("test_shutdown");
    EXPECT_TRUE(WaitingCallback(g_isHighPriority));
    EXPECT_TRUE(WaitingCallback(g_isDefaultPriority));
    POWER_HILOGI(LABEL_TEST, "AsyncShutdownCallback004 end");
}

/**
 * @tc.name: AsyncShutdownCallback005
 * @tc.desc: Test do not asynchronous shutdown
 * @tc.type: FUNC
 */
HWTEST_F(AsyncShutdownCallbackTest, AsyncShutdownCallback005, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "AsyncShutdownCallback005 start");
    auto notAsyncCallback = new NotAsyncShutdownCallback();
    g_service->RegisterShutdownCallback(notAsyncCallback, ShutdownPriority::DEFAULT);

    EXPECT_CALL(*g_mockPowerAction, Reboot(std::string("test_reboot"))).Times(1);
    g_service->RebootDevice("test_reboot");

    EXPECT_CALL(*g_mockPowerAction, Shutdown(std::string("test_shutdown"))).Times(1);
    g_service->ShutDownDevice("test_shutdown");

    // wait for detached threads to finish
    sleep(1);
    POWER_HILOGI(LABEL_TEST, "AsyncShutdownCallback005 end");
}
} // namespace UnitTest
} // namespace PowerMgr
} // namespace OHOS
