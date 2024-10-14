/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
bool g_isOnAsyncShutdownOrReboot = false;
bool g_isOnSyncShutdownOrReboot = false;
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

void ShutdownClientTest::AsyncShutdownCallback::OnAsyncShutdownOrReboot(bool isReboot)
{
    g_isOnAsyncShutdownOrReboot = isReboot;
}

void ShutdownClientTest::SyncShutdownCallback::OnSyncShutdown()
{
    g_isOnSyncShutdown = true;
}

void ShutdownClientTest::SyncShutdownCallback::OnSyncShutdownOrReboot(bool isReboot)
{
    g_isOnSyncShutdownOrReboot = isReboot;
}

bool ShutdownClientTest::TakeOverShutdownCallback::OnTakeOverShutdown(bool isReboot)
{
    g_isOnTakeOverShutdown = true;
    return true;
}

/**
 * @tc.name: RegisterShutdownCallback001
 * @tc.desc: Test RegisterShutdownCallback
 * @tc.type: FUNC
 */
HWTEST_F(ShutdownClientTest, RegisterShutdownCallback001, TestSize.Level0)
{
    POWER_HILOGD(LABEL_TEST, "RegisterShutdownCallback001 start");
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
    POWER_HILOGD(LABEL_TEST, "RegisterShutdownCallback001 end");
}

/**
 * @tc.name: UnRegisterShutdownCallback001
 * @tc.desc: Test UnRegisterShutdownCallback
 * @tc.type: FUNC
 */
HWTEST_F(ShutdownClientTest, UnRegisterShutdownCallback001, TestSize.Level0)
{
    POWER_HILOGD(LABEL_TEST, "UnRegisterShutdownCallback001 start");
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
    powerMgrClient.RebootDevice("UnRegisterShutdownCallback001");
    EXPECT_FALSE(g_isOnAsyncShutdown);
    EXPECT_FALSE(g_isOnSyncShutdown);
    EXPECT_TRUE(g_isOnTakeOverShutdown);

    powerMgrClient.ShutDownDevice("UnRegisterShutdownCallback001");
    EXPECT_FALSE(g_isOnAsyncShutdown);
    EXPECT_FALSE(g_isOnSyncShutdown);
    EXPECT_TRUE(g_isOnTakeOverShutdown);
    POWER_HILOGD(LABEL_TEST, "UnRegisterShutdownCallback001 end");
}

/**
 * @tc.name: AsyncShutdownCallbackStub001
 * @tc.desc: Test AsyncShutdownCallbackStub
 * @tc.type: FUNC
 * @tc.require: issueI7MNRN
 */
HWTEST_F(ShutdownClientTest, AsyncShutdownCallbackStub001, TestSize.Level0)
{
    POWER_HILOGD(LABEL_TEST, "AsyncShutdownCallbackStub001 start");
    uint32_t code = 0;
    MessageParcel data;
    AsyncShutdownCallback asyncShutdownCallback;
    asyncShutdownCallback.AsyncShutdownCallbackStub::OnAsyncShutdown();
    EXPECT_FALSE(g_isOnAsyncShutdown);
    data.WriteInterfaceToken(AsyncShutdownCallback::GetDescriptor());
    int32_t ret = asyncShutdownCallback.OnRemoteRequest(code, data, g_reply, g_option);
    EXPECT_EQ(ret, ERR_OK);
    POWER_HILOGD(LABEL_TEST, "AsyncShutdownCallbackStub001 end");
}

/**
 * @tc.name: SyncShutdownCallbackStub001
 * @tc.desc: Test SyncShutdownCallbackStub
 * @tc.type: FUNC
 * @tc.require: issueI7MNRN
 */
HWTEST_F(ShutdownClientTest, SyncShutdownCallbackStub001, TestSize.Level0)
{
    POWER_HILOGD(LABEL_TEST, "SyncShutdownCallbackStub001 start");
    uint32_t code = 0;
    MessageParcel data;
    SyncShutdownCallback syncShutdownCallback;
    syncShutdownCallback.SyncShutdownCallbackStub::OnSyncShutdown();
    EXPECT_FALSE(g_isOnSyncShutdown);
    data.WriteInterfaceToken(SyncShutdownCallback::GetDescriptor());
    int32_t ret = syncShutdownCallback.OnRemoteRequest(code, data, g_reply, g_option);
    EXPECT_EQ(ret, ERR_OK);
    POWER_HILOGD(LABEL_TEST, "SyncShutdownCallbackStub001 end");
}

/**
 * @tc.name: TakeOverShutdownCallbackStub001
 * @tc.desc: Test TakeOverShutdownCallbackStub
 * @tc.type: FUNC
 * @tc.require: issueI7MNRN
 */
HWTEST_F(ShutdownClientTest, TakeOverShutdownCallbackStub001, TestSize.Level0)
{
    POWER_HILOGD(LABEL_TEST, "TakeOverShutdownCallbackStub001 start");
    uint32_t code = 0;
    MessageParcel data;
    TakeOverShutdownCallback takeOverShutdownCallback;
    data.WriteInterfaceToken(TakeOverShutdownCallback::GetDescriptor());
    int32_t ret = takeOverShutdownCallback.OnRemoteRequest(code, data, g_reply, g_option);
    EXPECT_EQ(ret, E_READ_PARCEL_ERROR);
    bool retVal = takeOverShutdownCallback.TakeOverShutdownCallbackStub::OnTakeOverShutdown(false);
    EXPECT_EQ(retVal, false);
    POWER_HILOGD(LABEL_TEST, "TakeOverShutdownCallbackStub001 end");
}

/**
 * @tc.name: AsyncShutdownOrRebootCallbackStub001
 * @tc.desc: Test AsyncShutdownOrRebootCallbackStub
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ShutdownClientTest, AsyncShutdownOrRebootCallbackStub001, TestSize.Level0)
{
    POWER_HILOGD(LABEL_TEST, "AsyncShutdownOrRebootCallbackStub001 start");
    uint32_t code = 1;
    MessageParcel data;
    AsyncShutdownCallback asyncShutdownCallback;
    asyncShutdownCallback.AsyncShutdownCallbackStub::OnAsyncShutdownOrReboot(true);
    EXPECT_FALSE(g_isOnAsyncShutdownOrReboot);
    data.WriteInterfaceToken(AsyncShutdownCallback::GetDescriptor());
    data.WriteBool(true);
    int32_t ret = asyncShutdownCallback.OnRemoteRequest(code, data, g_reply, g_option);
    EXPECT_EQ(ret, ERR_OK);
    POWER_HILOGD(LABEL_TEST, "AsyncShutdownOrRebootCallbackStub001 end");
}

/**
 * @tc.name: SyncShutdownOrRebootCallbackStub001
 * @tc.desc: Test SyncShutdownOrRebootCallbackStub
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ShutdownClientTest, SyncShutdownOrRebootCallbackStub001, TestSize.Level0)
{
    POWER_HILOGD(LABEL_TEST, "SyncShutdownOrRebootCallbackStub001 start");
    uint32_t code = 1;
    MessageParcel data;
    SyncShutdownCallback syncShutdownCallback;
    syncShutdownCallback.SyncShutdownCallbackStub::OnSyncShutdownOrReboot(true);
    EXPECT_FALSE(g_isOnSyncShutdownOrReboot);
    data.WriteInterfaceToken(SyncShutdownCallback::GetDescriptor());
    data.WriteBool(true);
    int32_t ret = syncShutdownCallback.OnRemoteRequest(code, data, g_reply, g_option);
    EXPECT_EQ(ret, ERR_OK);
    POWER_HILOGD(LABEL_TEST, "SyncShutdownOrRebootCallbackStub001 end");
}
} // namespace UnitTest
} // namespace PowerMgr
} // namespace OHOS
