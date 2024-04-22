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

#include "shutdown_client_test.h"

#include <condition_variable>
#include <future>
#include <mutex>
#include "power_log.h"
#include "power_mgr_client.h"
#include "power_mgr_service.h"
#include "shutdown/shutdown_client.h"

namespace OHOS {
namespace PowerMgr {
namespace UnitTest {
namespace {
MessageParcel g_reply;
MessageOption g_option;
bool g_isOnAsyncShutdown = false;
bool g_isOnSyncShutdown = false;
bool g_isOnTakeOverShutdown = false;
}
using namespace testing::ext;

void ShutdownClientTest::SetUpTestCase()
{
}

void ShutdownClientTest::TearDownTestCase()
{
}

void ShutdownClientTest::SetUp()
{
    g_isOnAsyncShutdown = false;
    g_isOnSyncShutdown = false;
    g_isOnTakeOverShutdown = false;
}

void ShutdownClientTest::TearDown()
{}

void ShutdownClientTest::AsyncShutdownCallback::OnAsyncShutdown()
{
    g_isOnAsyncShutdown = true;
}

void ShutdownClientTest::SyncShutdownCallback::OnSyncShutdown()
{
    g_isOnSyncShutdown = true;
}

bool ShutdownClientTest::TakeOverShutdownCallback::OnTakeOverShutdown(bool isReboot)
{
    g_isOnTakeOverShutdown = true;
    return true;
}

/**
 * @tc.name: RegisterShutdownCallback
 * @tc.desc: Test RegisterShutdownCallback
 * @tc.type: FUNC
 */
HWTEST_F(ShutdownClientTest, RegisterShutdownCallback001, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "RegisterShutdownCallback001 start");
    sptr<IAsyncShutdownCallback> asyncShutdownCallback = new AsyncShutdownCallback();
    sptr<ISyncShutdownCallback> syncShutdownCallback = new SyncShutdownCallback();
    sptr<ITakeOverShutdownCallback> takeOverShutdownCallback = new TakeOverShutdownCallback();
    auto& shutdownClient = ShutdownClient::GetInstance();
    shutdownClient.RegisterShutdownCallback(asyncShutdownCallback);
    shutdownClient.RegisterShutdownCallback(syncShutdownCallback);
    shutdownClient.RegisterShutdownCallback(takeOverShutdownCallback);

    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.RebootDevice("RegisterShutdownCallback001");
    EXPECT_FALSE(g_isOnAsyncShutdown);
    EXPECT_FALSE(g_isOnSyncShutdown);
    EXPECT_TRUE(g_isOnTakeOverShutdown);

    powerMgrClient.ShutDownDevice("RegisterShutdownCallback001");
    EXPECT_FALSE(g_isOnAsyncShutdown);
    EXPECT_FALSE(g_isOnSyncShutdown);
    EXPECT_TRUE(g_isOnTakeOverShutdown);
    POWER_HILOGI(LABEL_TEST, "RegisterShutdownCallback001 end");
}

/**
 * @tc.name: UnRegisterShutdownCallback
 * @tc.desc: Test UnRegisterShutdownCallback
 * @tc.type: FUNC
 */
HWTEST_F(ShutdownClientTest, UnRegisterShutdownCallback002, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "UnRegisterShutdownCallback002 start");
    sptr<IAsyncShutdownCallback> asyncShutdownCallback = new AsyncShutdownCallback();
    sptr<ISyncShutdownCallback> syncShutdownCallback = new SyncShutdownCallback();
    sptr<ITakeOverShutdownCallback> takeOverShutdownCallback = new TakeOverShutdownCallback();
    auto& shutdownClient = ShutdownClient::GetInstance();
    shutdownClient.RegisterShutdownCallback(asyncShutdownCallback);
    shutdownClient.UnRegisterShutdownCallback(asyncShutdownCallback);
    shutdownClient.RegisterShutdownCallback(syncShutdownCallback);
    shutdownClient.UnRegisterShutdownCallback(syncShutdownCallback);
    shutdownClient.RegisterShutdownCallback(takeOverShutdownCallback);
    shutdownClient.UnRegisterShutdownCallback(takeOverShutdownCallback);

    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.RebootDevice("UnRegisterShutdownCallback002");
    EXPECT_FALSE(g_isOnAsyncShutdown);
    EXPECT_FALSE(g_isOnSyncShutdown);
    EXPECT_TRUE(g_isOnTakeOverShutdown);

    powerMgrClient.ShutDownDevice("UnRegisterShutdownCallback002");
    EXPECT_FALSE(g_isOnAsyncShutdown);
    EXPECT_FALSE(g_isOnSyncShutdown);
    EXPECT_TRUE(g_isOnTakeOverShutdown);
    POWER_HILOGI(LABEL_TEST, "UnRegisterShutdownCallback002 end");
}

/**
 * @tc.name: AsyncShutdownCallbackStub
 * @tc.desc: Test AsyncShutdownCallbackStub
 * @tc.type: FUNC
 * @tc.require: issueI7MNRN
 */
HWTEST_F(ShutdownClientTest, AsyncShutdownCallbackStub003, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "AsyncShutdownCallbackStub003 start");
    uint32_t code = 0;
    MessageParcel data;
    AsyncShutdownCallback asyncShutdownCallback;
    asyncShutdownCallback.AsyncShutdownCallbackStub::OnAsyncShutdown();
    EXPECT_FALSE(g_isOnAsyncShutdown);
    data.WriteInterfaceToken(AsyncShutdownCallback::GetDescriptor());
    int32_t ret = asyncShutdownCallback.OnRemoteRequest(code, data, g_reply, g_option);
    EXPECT_EQ(ret, ERR_OK);
    POWER_HILOGI(LABEL_TEST, "AsyncShutdownCallbackStub003 end");
}

/**
 * @tc.name: SyncShutdownCallbackStub
 * @tc.desc: Test SyncShutdownCallbackStub
 * @tc.type: FUNC
 * @tc.require: issueI7MNRN
 */
HWTEST_F(ShutdownClientTest, SyncShutdownCallbackStub004, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "SyncShutdownCallbackStub004 start");
    uint32_t code = 0;
    MessageParcel data;
    SyncShutdownCallback syncShutdownCallback;
    syncShutdownCallback.SyncShutdownCallbackStub::OnSyncShutdown();
    EXPECT_FALSE(g_isOnSyncShutdown);
    data.WriteInterfaceToken(SyncShutdownCallback::GetDescriptor());
    int32_t ret = syncShutdownCallback.OnRemoteRequest(code, data, g_reply, g_option);
    EXPECT_EQ(ret, ERR_OK);
    POWER_HILOGI(LABEL_TEST, "SyncShutdownCallbackStub004 end");
}

/**
 * @tc.name: TakeOverShutdownCallbackStub
 * @tc.desc: Test TakeOverShutdownCallbackStub
 * @tc.type: FUNC
 * @tc.require: issueI7MNRN
 */
HWTEST_F(ShutdownClientTest, TakeOverShutdownCallbackStub005, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "TakeOverShutdownCallbackStub005 start");
    uint32_t code = 0;
    MessageParcel data;
    TakeOverShutdownCallback takeOverShutdownCallback;
    data.WriteInterfaceToken(TakeOverShutdownCallback::GetDescriptor());
    int32_t ret = takeOverShutdownCallback.OnRemoteRequest(code, data, g_reply, g_option);
    EXPECT_EQ(ret, E_READ_PARCEL_ERROR);
    bool retVal = takeOverShutdownCallback.TakeOverShutdownCallbackStub::OnTakeOverShutdown(false);
    EXPECT_EQ(retVal, false);
    POWER_HILOGI(LABEL_TEST, "TakeOverShutdownCallbackStub005 end");
}
} // namespace UnitTest
} // namespace PowerMgr
} // namespace OHOS
