/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "shutdown_callback_holder_test.h"
#include <ipc_skeleton.h>
#include "power_common.h"
#include "power_log.h"
#include "sync_shutdown_callback_stub.h"
#include "async_shutdown_callback_stub.h"
#include "takeover_shutdown_callback_stub.h"
#include "shutdown_priority.h"
#include "shutdown/shutdown_callback_holer.h"
#include "shutdown/async_shutdown_callback_ipc_interface_code.h"
#include "shutdown/sync_shutdown_callback_ipc_interface_code.h"
#include "shutdown/takeover_shutdown_callback_ipc_interface_code.h"

namespace OHOS {
namespace PowerMgr {

using namespace testing;
using namespace testing::ext;

namespace {
    bool g_isOnAsyncShutdown = false;
    bool g_isOnSyncShutdown = false;
    bool g_isOnTakeOverShutdown = false;
    bool g_isOnTakeOverHibernate = false;
    bool g_isOnAsyncShutdownOrReboot = false;
    bool g_isOnSyncShutdownOrReboot = false;
    constexpr uint32_t INVALID_CMD_CODE = 100;
} // namespace

class TestAsyncShutdownCallback : public AsyncShutdownCallbackStub {
public:
    ~TestAsyncShutdownCallback() override = default;
    void OnAsyncShutdown() override
    {
        g_isOnAsyncShutdown = true;
        POWER_HILOGI(LABEL_TEST, "TestAsyncShutdownCallback OnAsyncShutdown function start!");
    }
    void OnAsyncShutdownOrReboot(bool isReboot) override
    {
        g_isOnAsyncShutdownOrReboot = isReboot;
        POWER_HILOGI(LABEL_TEST, "TestAsyncShutdownCallback OnAsyncShutdownOrReboot function start!");
    }
};

class TestSyncShutdownCallback : public SyncShutdownCallbackStub {
public:
    ~TestSyncShutdownCallback() override = default;
    void OnSyncShutdown() override
    {
        g_isOnSyncShutdown = true;
        POWER_HILOGI(LABEL_TEST, "TestSyncShutdownCallback OnSyncShutdown function start!");
    }
    void OnSyncShutdownOrReboot(bool isReboot) override
    {
        g_isOnSyncShutdownOrReboot = isReboot;
        POWER_HILOGI(LABEL_TEST, "TestSyncShutdownCallback OnSyncShutdownOrReboot function start!");
    }
};

class TestTakeOverShutdownCallback : public TakeOverShutdownCallbackStub {
public:
    ~TestTakeOverShutdownCallback() override = default;
    bool OnTakeOverShutdown(const TakeOverInfo& info) override
    {
        POWER_HILOGI(LABEL_TEST, "TestTakeOverShutdownCallback OnTakeOverShutdown function start!");
        g_isOnTakeOverShutdown = true;
        return true;
    }
    bool OnTakeOverHibernate(const TakeOverInfo& info) override
    {
        POWER_HILOGI(LABEL_TEST, "TestTakeOverShutdownCallback OnTakeOverHibernate function start!");
        g_isOnTakeOverHibernate = true;
        return true;
    }
};

void ShutdownCallbackHolderTest::SetUpTestCase()
{
}

void ShutdownCallbackHolderTest::TearDownTestCase()
{
}

void ShutdownCallbackHolderTest::SetUp()
{
}

void ShutdownCallbackHolderTest::TearDown()
{
}

/**
 * @tc.name: ShutdownCallbackHolderTest001
 * @tc.desc: Test AddCallback and RemoveCallback
 * @tc.type: FUNC
 */
HWTEST_F(ShutdownCallbackHolderTest, ShutdownCallbackHolderTest001, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "ShutdownCallbackHolderTest001 function start!");
    sptr<ShutdownCallbackHolder> shutdownCallbackHolder = new ShutdownCallbackHolder();

    sptr<TestAsyncShutdownCallback> asyncShutdownCallback = new TestAsyncShutdownCallback();
    sptr<TestSyncShutdownCallback> syncShutdownCallback = new TestSyncShutdownCallback();
    sptr<TestTakeOverShutdownCallback> takeOverShutDownCallback = new TestTakeOverShutdownCallback();

    shutdownCallbackHolder->AddCallback(asyncShutdownCallback->AsObject(), ShutdownPriority::HIGH);
    shutdownCallbackHolder->AddCallback(syncShutdownCallback->AsObject(), ShutdownPriority::DEFAULT);
    shutdownCallbackHolder->AddCallback(takeOverShutDownCallback->AsObject(), ShutdownPriority::LOW);

    std::set<sptr<IRemoteObject>> highCallbacks = shutdownCallbackHolder->GetHighPriorityCallbacks();
    std::set<sptr<IRemoteObject>> defaultCallbacks = shutdownCallbackHolder->GetDefaultPriorityCallbacks();
    std::set<sptr<IRemoteObject>> lowCallbacks = shutdownCallbackHolder->GetLowPriorityCallbacks();
    EXPECT_EQ(highCallbacks.size(), 1);
    EXPECT_EQ(defaultCallbacks.size(), 1);
    EXPECT_EQ(lowCallbacks.size(), 1);

    shutdownCallbackHolder->RemoveCallback(asyncShutdownCallback->AsObject());
    shutdownCallbackHolder->RemoveCallback(syncShutdownCallback->AsObject());
    shutdownCallbackHolder->RemoveCallback(takeOverShutDownCallback->AsObject());
    highCallbacks = shutdownCallbackHolder->GetHighPriorityCallbacks();
    defaultCallbacks = shutdownCallbackHolder->GetDefaultPriorityCallbacks();
    lowCallbacks = shutdownCallbackHolder->GetLowPriorityCallbacks();
    EXPECT_EQ(highCallbacks.size(), 0);
    EXPECT_EQ(defaultCallbacks.size(), 0);
    EXPECT_EQ(lowCallbacks.size(), 0);
    POWER_HILOGI(LABEL_TEST, "ShutdownCallbackHolderTest001 function end!");
}

/**
 * @tc.name: ShutdownCallbackHolderTest002
 * @tc.desc: Test FindCallbackPidUid
 * @tc.type: FUNC
 */
HWTEST_F(ShutdownCallbackHolderTest, ShutdownCallbackHolderTest002, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "ShutdownCallbackHolderTest002 function start!");
    sptr<ShutdownCallbackHolder> shutdownCallbackHolder = new ShutdownCallbackHolder();
    sptr<TestAsyncShutdownCallback> asyncShutdownCallback = new TestAsyncShutdownCallback();
    sptr<TestSyncShutdownCallback> syncShutdownCallback = new TestSyncShutdownCallback();
    sptr<TestTakeOverShutdownCallback> takeOverShutDownCallback = new TestTakeOverShutdownCallback();

    shutdownCallbackHolder->AddCallback(asyncShutdownCallback->AsObject(), ShutdownPriority::HIGH);
    shutdownCallbackHolder->AddCallback(syncShutdownCallback->AsObject(), ShutdownPriority::DEFAULT);
    shutdownCallbackHolder->AddCallback(takeOverShutDownCallback->AsObject(), ShutdownPriority::LOW);
    EXPECT_EQ(shutdownCallbackHolder->cachedRegister_.size(), 3);

    auto pidUid = shutdownCallbackHolder->FindCallbackPidUid(asyncShutdownCallback->AsObject());
    EXPECT_EQ(pidUid.first, IPCSkeleton::GetCallingPid());
    EXPECT_EQ(pidUid.second, IPCSkeleton::GetCallingUid());
    pidUid = shutdownCallbackHolder->FindCallbackPidUid(syncShutdownCallback->AsObject());
    EXPECT_EQ(pidUid.first, IPCSkeleton::GetCallingPid());
    EXPECT_EQ(pidUid.second, IPCSkeleton::GetCallingUid());
    pidUid = shutdownCallbackHolder->FindCallbackPidUid(takeOverShutDownCallback->AsObject());
    EXPECT_EQ(pidUid.first, IPCSkeleton::GetCallingPid());
    EXPECT_EQ(pidUid.second, IPCSkeleton::GetCallingUid());

    shutdownCallbackHolder->RemoveCallback(asyncShutdownCallback->AsObject());
    shutdownCallbackHolder->RemoveCallback(syncShutdownCallback->AsObject());
    shutdownCallbackHolder->RemoveCallback(takeOverShutDownCallback->AsObject());
    EXPECT_EQ(shutdownCallbackHolder->cachedRegister_.size(), 0);
    pidUid = shutdownCallbackHolder->FindCallbackPidUid(asyncShutdownCallback->AsObject());
    EXPECT_EQ(pidUid.first, 0);
    EXPECT_EQ(pidUid.second, 0);
    pidUid = shutdownCallbackHolder->FindCallbackPidUid(syncShutdownCallback->AsObject());
    EXPECT_EQ(pidUid.first, 0);
    EXPECT_EQ(pidUid.second, 0);
    pidUid = shutdownCallbackHolder->FindCallbackPidUid(takeOverShutDownCallback->AsObject());
    EXPECT_EQ(pidUid.first, 0);
    EXPECT_EQ(pidUid.second, 0);
    POWER_HILOGI(LABEL_TEST, "ShutdownCallbackHolderTest002 function end!");
}

/**
 * @tc.name: ShutdownCallbackHolderTest003
 * @tc.desc: Test duplicate callback operations
 * @tc.type: FUNC
 */
HWTEST_F(ShutdownCallbackHolderTest, ShutdownCallbackHolderTest003, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "ShutdownCallbackHolderTest003 function start!");
    sptr<ShutdownCallbackHolder> shutdownCallbackHolder = new ShutdownCallbackHolder();
    sptr<TestAsyncShutdownCallback> asyncShutdownCallback = new TestAsyncShutdownCallback();
    sptr<TestSyncShutdownCallback> syncShutdownCallback = new TestSyncShutdownCallback();
    sptr<TestTakeOverShutdownCallback> takeOverShutDownCallback = new TestTakeOverShutdownCallback();

    shutdownCallbackHolder->AddCallback(asyncShutdownCallback->AsObject(), ShutdownPriority::HIGH);
    std::set<sptr<IRemoteObject>> highCallbacks = shutdownCallbackHolder->GetHighPriorityCallbacks();
    EXPECT_EQ(highCallbacks.size(), 1);
    shutdownCallbackHolder->AddCallback(asyncShutdownCallback->AsObject(), ShutdownPriority::HIGH);
    highCallbacks = shutdownCallbackHolder->GetHighPriorityCallbacks();
    EXPECT_EQ(highCallbacks.size(), 1);

    shutdownCallbackHolder->AddCallback(syncShutdownCallback->AsObject(), ShutdownPriority::DEFAULT);
    std::set<sptr<IRemoteObject>> defaultCallbacks = shutdownCallbackHolder->GetDefaultPriorityCallbacks();
    EXPECT_EQ(defaultCallbacks.size(), 1);
    shutdownCallbackHolder->AddCallback(syncShutdownCallback->AsObject(), ShutdownPriority::DEFAULT);
    defaultCallbacks = shutdownCallbackHolder->GetDefaultPriorityCallbacks();
    EXPECT_EQ(defaultCallbacks.size(), 1);

    shutdownCallbackHolder->AddCallback(takeOverShutDownCallback->AsObject(), ShutdownPriority::LOW);
    std::set<sptr<IRemoteObject>> lowCallbacks = shutdownCallbackHolder->GetLowPriorityCallbacks();
    EXPECT_EQ(lowCallbacks.size(), 1);
    shutdownCallbackHolder->AddCallback(takeOverShutDownCallback->AsObject(), ShutdownPriority::LOW);
    lowCallbacks = shutdownCallbackHolder->GetLowPriorityCallbacks();
    EXPECT_EQ(lowCallbacks.size(), 1);

    shutdownCallbackHolder->RemoveCallback(asyncShutdownCallback->AsObject());
    shutdownCallbackHolder->RemoveCallback(syncShutdownCallback->AsObject());
    shutdownCallbackHolder->RemoveCallback(takeOverShutDownCallback->AsObject());
    POWER_HILOGI(LABEL_TEST, "ShutdownCallbackHolderTest003 function end!");
}

/**
 * @tc.name: ShutdownCallbackHolderTest004
 * @tc.desc: Test removing non-existent callback from specific container
 * @tc.type: FUNC
 */
HWTEST_F(ShutdownCallbackHolderTest, ShutdownCallbackHolderTest004, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "ShutdownCallbackHolderTest004 function start!");
    sptr<ShutdownCallbackHolder> shutdownCallbackHolder = new ShutdownCallbackHolder();
    sptr<TestAsyncShutdownCallback> asyncShutdownCallback = new TestAsyncShutdownCallback();
    sptr<TestSyncShutdownCallback> syncShutdownCallback = new TestSyncShutdownCallback();
    sptr<TestTakeOverShutdownCallback> takeOverShutDownCallback = new TestTakeOverShutdownCallback();

    shutdownCallbackHolder->RemoveCallback(asyncShutdownCallback->AsObject());
    shutdownCallbackHolder->RemoveCallback(syncShutdownCallback->AsObject());
    shutdownCallbackHolder->RemoveCallback(takeOverShutDownCallback->AsObject());

    auto highCallbacks = shutdownCallbackHolder->GetHighPriorityCallbacks();
    auto defaultCallbacks = shutdownCallbackHolder->GetDefaultPriorityCallbacks();
    auto lowCallbacks = shutdownCallbackHolder->GetLowPriorityCallbacks();

    EXPECT_EQ(highCallbacks.size(), 0);
    EXPECT_EQ(defaultCallbacks.size(), 0);
    EXPECT_EQ(lowCallbacks.size(), 0);
    POWER_HILOGI(LABEL_TEST, "ShutdownCallbackHolderTest004 function end!");
}

/**
 * @tc.name: ShutdownCallbackHolderTest005
 * @tc.desc: Test removing the same callback multiple times
 * @tc.type: FUNC
 */
HWTEST_F(ShutdownCallbackHolderTest, ShutdownCallbackHolderTest005, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "ShutdownCallbackHolderTest005 function start!");
    sptr<ShutdownCallbackHolder> shutdownCallbackHolder = new ShutdownCallbackHolder();
    sptr<TestAsyncShutdownCallback> asyncShutdownCallback = new TestAsyncShutdownCallback();
    sptr<TestSyncShutdownCallback> syncShutdownCallback = new TestSyncShutdownCallback();
    sptr<TestTakeOverShutdownCallback> takeOverShutDownCallback = new TestTakeOverShutdownCallback();

    shutdownCallbackHolder->AddCallback(asyncShutdownCallback->AsObject(), ShutdownPriority::HIGH);
    EXPECT_EQ(shutdownCallbackHolder->GetHighPriorityCallbacks().size(), 1);
    shutdownCallbackHolder->RemoveCallback(asyncShutdownCallback->AsObject());
    EXPECT_EQ(shutdownCallbackHolder->GetHighPriorityCallbacks().size(), 0);
    shutdownCallbackHolder->RemoveCallback(asyncShutdownCallback->AsObject());

    shutdownCallbackHolder->AddCallback(syncShutdownCallback->AsObject(), ShutdownPriority::DEFAULT);
    EXPECT_EQ(shutdownCallbackHolder->GetDefaultPriorityCallbacks().size(), 1);
    shutdownCallbackHolder->RemoveCallback(syncShutdownCallback->AsObject());
    EXPECT_EQ(shutdownCallbackHolder->GetDefaultPriorityCallbacks().size(), 0);
    shutdownCallbackHolder->RemoveCallback(syncShutdownCallback->AsObject());

    shutdownCallbackHolder->AddCallback(takeOverShutDownCallback->AsObject(), ShutdownPriority::LOW);
    EXPECT_EQ(shutdownCallbackHolder->GetLowPriorityCallbacks().size(), 1);
    shutdownCallbackHolder->RemoveCallback(takeOverShutDownCallback->AsObject());
    EXPECT_EQ(shutdownCallbackHolder->GetLowPriorityCallbacks().size(), 0);
    shutdownCallbackHolder->RemoveCallback(takeOverShutDownCallback->AsObject());
    POWER_HILOGI(LABEL_TEST, "ShutdownCallbackHolderTest005 function end!");
}

/**
 * @tc.name: ShutdownCallbackHolderTest006
 * @tc.desc: Test multiple callbacks with different priorities
 * @tc.type: FUNC
 */
HWTEST_F(ShutdownCallbackHolderTest, ShutdownCallbackHolderTest006, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "ShutdownCallbackHolderTest006 function start!");
    sptr<ShutdownCallbackHolder> shutdownCallbackHolder = new ShutdownCallbackHolder();

    sptr<TestAsyncShutdownCallback> async1 = new TestAsyncShutdownCallback();
    sptr<TestAsyncShutdownCallback> async2 = new TestAsyncShutdownCallback();
    sptr<TestSyncShutdownCallback> sync1 = new TestSyncShutdownCallback();
    sptr<TestSyncShutdownCallback> sync2 = new TestSyncShutdownCallback();
    sptr<TestTakeOverShutdownCallback> takeOver1 = new TestTakeOverShutdownCallback();
    sptr<TestTakeOverShutdownCallback> takeOver2 = new TestTakeOverShutdownCallback();

    shutdownCallbackHolder->AddCallback(async1->AsObject(), ShutdownPriority::HIGH);
    shutdownCallbackHolder->AddCallback(async2->AsObject(), ShutdownPriority::HIGH);
    shutdownCallbackHolder->AddCallback(sync1->AsObject(), ShutdownPriority::DEFAULT);
    shutdownCallbackHolder->AddCallback(sync2->AsObject(), ShutdownPriority::DEFAULT);
    shutdownCallbackHolder->AddCallback(takeOver1->AsObject(), ShutdownPriority::LOW);
    shutdownCallbackHolder->AddCallback(takeOver2->AsObject(), ShutdownPriority::LOW);

    EXPECT_EQ(shutdownCallbackHolder->GetHighPriorityCallbacks().size(), 2);
    EXPECT_EQ(shutdownCallbackHolder->GetDefaultPriorityCallbacks().size(), 2);
    EXPECT_EQ(shutdownCallbackHolder->GetLowPriorityCallbacks().size(), 2);

    shutdownCallbackHolder->RemoveCallback(async1->AsObject());
    shutdownCallbackHolder->RemoveCallback(sync2->AsObject());
    shutdownCallbackHolder->RemoveCallback(takeOver1->AsObject());

    EXPECT_EQ(shutdownCallbackHolder->GetHighPriorityCallbacks().size(), 1);
    EXPECT_EQ(shutdownCallbackHolder->GetDefaultPriorityCallbacks().size(), 1);
    EXPECT_EQ(shutdownCallbackHolder->GetLowPriorityCallbacks().size(), 1);

    shutdownCallbackHolder->RemoveCallback(async2->AsObject());
    shutdownCallbackHolder->RemoveCallback(sync1->AsObject());
    shutdownCallbackHolder->RemoveCallback(takeOver2->AsObject());

    EXPECT_EQ(shutdownCallbackHolder->GetHighPriorityCallbacks().size(), 0);
    EXPECT_EQ(shutdownCallbackHolder->GetDefaultPriorityCallbacks().size(), 0);
    EXPECT_EQ(shutdownCallbackHolder->GetLowPriorityCallbacks().size(), 0);
    POWER_HILOGI(LABEL_TEST, "ShutdownCallbackHolderTest006 function end!");
}

/**
 * @tc.name: ShutdownCallbackHolderTest007
 * @tc.desc: Test OnRemoteDied
 * @tc.type: FUNC
 */
HWTEST_F(ShutdownCallbackHolderTest, ShutdownCallbackHolderTest007, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "ShutdownCallbackHolderTest007 function start!");
    sptr<ShutdownCallbackHolder> shutdownCallbackHolder = new ShutdownCallbackHolder();
    shutdownCallbackHolder->OnRemoteDied(nullptr);
    sptr<IRemoteObject> obj = nullptr;
    shutdownCallbackHolder->OnRemoteDied(obj);

    sptr<TestAsyncShutdownCallback> asyncCallback = new TestAsyncShutdownCallback();
    shutdownCallbackHolder->AddCallback(asyncCallback->AsObject(), ShutdownPriority::HIGH);
    EXPECT_EQ(shutdownCallbackHolder->GetHighPriorityCallbacks().size(), 1);
    EXPECT_EQ(shutdownCallbackHolder->cachedRegister_.size(), 1);
    shutdownCallbackHolder->OnRemoteDied(asyncCallback->AsObject());
    EXPECT_EQ(shutdownCallbackHolder->GetHighPriorityCallbacks().size(), 0);
    EXPECT_EQ(shutdownCallbackHolder->cachedRegister_.size(), 0);

    sptr<TestSyncShutdownCallback> syncCallback = new TestSyncShutdownCallback();
    shutdownCallbackHolder->AddCallback(syncCallback->AsObject(), ShutdownPriority::DEFAULT);
    EXPECT_EQ(shutdownCallbackHolder->GetDefaultPriorityCallbacks().size(), 1);
    EXPECT_EQ(shutdownCallbackHolder->cachedRegister_.size(), 1);
    shutdownCallbackHolder->OnRemoteDied(syncCallback->AsObject());
    EXPECT_EQ(shutdownCallbackHolder->GetDefaultPriorityCallbacks().size(), 0);
    EXPECT_EQ(shutdownCallbackHolder->cachedRegister_.size(), 0);

    sptr<TestTakeOverShutdownCallback> takeOverCallback = new TestTakeOverShutdownCallback();
    shutdownCallbackHolder->AddCallback(takeOverCallback->AsObject(), ShutdownPriority::LOW);
    EXPECT_EQ(shutdownCallbackHolder->GetLowPriorityCallbacks().size(), 1);
    EXPECT_EQ(shutdownCallbackHolder->cachedRegister_.size(), 1);
    shutdownCallbackHolder->OnRemoteDied(takeOverCallback->AsObject());
    EXPECT_EQ(shutdownCallbackHolder->GetLowPriorityCallbacks().size(), 0);
    EXPECT_EQ(shutdownCallbackHolder->cachedRegister_.size(), 0);

    shutdownCallbackHolder->OnRemoteDied(takeOverCallback->AsObject());
    EXPECT_EQ(shutdownCallbackHolder->cachedRegister_.size(), 0);
    POWER_HILOGI(LABEL_TEST, "ShutdownCallbackHolderTest007 function end!");
}

/**
 * @tc.name: ShutdownCallbackHolderTest008
 * @tc.desc: Test AsyncShutdownCallbackStub
 * @tc.type: FUNC
 */
HWTEST_F(ShutdownCallbackHolderTest, ShutdownCallbackHolderTest008, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "ShutdownCallbackHolderTest008 function start!");
    TestAsyncShutdownCallback asyncCallback;
    MessageParcel reply;
    MessageOption opt;

    MessageParcel data0;
    asyncCallback.AsyncShutdownCallbackStub::OnAsyncShutdown();
    EXPECT_FALSE(g_isOnAsyncShutdown);
    data0.WriteInterfaceToken(u"test.interface.token");
    int32_t ret0 = asyncCallback.OnRemoteRequest(
        static_cast<uint32_t>(PowerMgr::AsyncShutdownCallbackInterfaceCode::CMD_ON_ASYNC_SHUTDOWN),
        data0, reply, opt);
    EXPECT_EQ(ret0, E_GET_POWER_SERVICE_FAILED);

    MessageParcel data1;
    data1.WriteInterfaceToken(TestAsyncShutdownCallback::GetDescriptor());
    int32_t ret1 = asyncCallback.OnRemoteRequest(
        static_cast<uint32_t>(PowerMgr::AsyncShutdownCallbackInterfaceCode::CMD_ON_ASYNC_SHUTDOWN),
        data1, reply, opt);
    EXPECT_TRUE(g_isOnAsyncShutdown);
    EXPECT_EQ(ret1, ERR_OK);

    MessageParcel data2;
    asyncCallback.AsyncShutdownCallbackStub::OnAsyncShutdownOrReboot(false);
    EXPECT_FALSE(g_isOnAsyncShutdownOrReboot);
    data2.WriteInterfaceToken(TestAsyncShutdownCallback::GetDescriptor());
    data2.WriteBool(true);
    int32_t ret2 = asyncCallback.OnRemoteRequest(
        static_cast<uint32_t>(PowerMgr::AsyncShutdownCallbackInterfaceCode::CMD_ON_ASYNC_SHUTDOWN_OR_REBOOT),
        data2, reply, opt);
    EXPECT_TRUE(g_isOnAsyncShutdownOrReboot);
    EXPECT_EQ(ret2, ERR_OK);

    MessageParcel data3;
    data3.WriteInterfaceToken(TestAsyncShutdownCallback::GetDescriptor());
    int32_t ret3 = asyncCallback.OnRemoteRequest(2, data3, reply, opt);
    EXPECT_NE(ret3, ERR_OK);
    POWER_HILOGI(LABEL_TEST, "ShutdownCallbackHolderTest008 function end!");
}

/**
 * @tc.name: ShutdownCallbackHolderTest009
 * @tc.desc: Test SyncShutdownCallbackStub
 * @tc.type: FUNC
 */
HWTEST_F(ShutdownCallbackHolderTest, ShutdownCallbackHolderTest009, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "ShutdownCallbackHolderTest009 function start!");
    TestSyncShutdownCallback syncCallback;
    MessageParcel reply;
    MessageOption opt;

    MessageParcel data0;
    syncCallback.SyncShutdownCallbackStub::OnSyncShutdown();
    EXPECT_FALSE(g_isOnSyncShutdown);
    data0.WriteInterfaceToken(u"test.interface.token");
    int32_t ret0 = syncCallback.OnRemoteRequest(
        static_cast<uint32_t>(PowerMgr::SyncShutdownCallbackInterfaceCode::CMD_ON_SYNC_SHUTDOWN),
        data0, reply, opt);
    EXPECT_EQ(ret0, E_GET_POWER_SERVICE_FAILED);

    MessageParcel data1;
    data1.WriteInterfaceToken(TestSyncShutdownCallback::GetDescriptor());
    int32_t ret1 = syncCallback.OnRemoteRequest(
        static_cast<uint32_t>(PowerMgr::SyncShutdownCallbackInterfaceCode::CMD_ON_SYNC_SHUTDOWN),
        data1, reply, opt);
    EXPECT_TRUE(g_isOnSyncShutdown);
    EXPECT_EQ(ret1, ERR_OK);

    MessageParcel data2;
    syncCallback.SyncShutdownCallbackStub::OnSyncShutdownOrReboot(false);
    EXPECT_FALSE(g_isOnSyncShutdownOrReboot);
    data2.WriteInterfaceToken(TestSyncShutdownCallback::GetDescriptor());
    data2.WriteBool(true);
    int32_t ret2 = syncCallback.OnRemoteRequest(
        static_cast<uint32_t>(PowerMgr::SyncShutdownCallbackInterfaceCode::CMD_ON_SYNC_SHUTDOWN_OR_REBOOT),
        data2, reply, opt);
    EXPECT_TRUE(g_isOnSyncShutdownOrReboot);
    EXPECT_EQ(ret2, ERR_OK);

    MessageParcel data3;
    data3.WriteInterfaceToken(TestSyncShutdownCallback::GetDescriptor());
    int32_t ret3 = syncCallback.OnRemoteRequest(2, data3, reply, opt);
    EXPECT_NE(ret3, ERR_OK);
    POWER_HILOGI(LABEL_TEST, "ShutdownCallbackHolderTest009 function end!");
}

/**
 * @tc.name: ShutdownCallbackHolderTest010
 * @tc.desc: Test TakeOverShutdownCallbackStub
 * @tc.type: FUNC
 */
HWTEST_F(ShutdownCallbackHolderTest, ShutdownCallbackHolderTest010, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "ShutdownCallbackHolderTest010 function start!");
    TestTakeOverShutdownCallback takeOverCallback;
    MessageParcel reply;
    MessageOption opt;

    MessageParcel data0;
    takeOverCallback.TakeOverShutdownCallbackStub::OnTakeOverShutdown({"ShutdownCallbackHolderTest010", false});
    EXPECT_FALSE(g_isOnTakeOverShutdown);
    data0.WriteInterfaceToken(u"test.interface.token");
    int32_t ret0 = takeOverCallback.OnRemoteRequest(
        static_cast<uint32_t>(PowerMgr::TakeoverShutdownCallbackInterfaceCode::CMD_ON_TAKEOVER_SHUTDOWN),
        data0, reply, opt);
    EXPECT_EQ(ret0, E_GET_POWER_SERVICE_FAILED);

    MessageParcel data1;
    data1.WriteInterfaceToken(TestTakeOverShutdownCallback::GetDescriptor());
    TakeOverInfo info1("test_reason", false);
    data1.WriteParcelable(&info1);
    int32_t ret1 = takeOverCallback.OnRemoteRequest(
        static_cast<uint32_t>(PowerMgr::TakeoverShutdownCallbackInterfaceCode::CMD_ON_TAKEOVER_SHUTDOWN),
        data1, reply, opt);
    bool result1 = reply.ReadBool();
    EXPECT_TRUE(result1);
    EXPECT_TRUE(g_isOnTakeOverShutdown);
    EXPECT_EQ(ret1, ERR_OK);

    MessageParcel data2;
    takeOverCallback.TakeOverShutdownCallbackStub::OnTakeOverHibernate({"ShutdownCallbackHolderTest010", false});
    EXPECT_FALSE(g_isOnTakeOverHibernate);
    data2.WriteInterfaceToken(TestTakeOverShutdownCallback::GetDescriptor());
    TakeOverInfo info2("hibernate_reason", true);
    data2.WriteParcelable(&info2);
    int32_t ret2 = takeOverCallback.OnRemoteRequest(
        static_cast<uint32_t>(PowerMgr::TakeoverShutdownCallbackInterfaceCode::CMD_ON_TAKEOVER_HIBERNATE),
        data2, reply, opt);
    bool result2 = reply.ReadBool();
    EXPECT_TRUE(result2);
    EXPECT_TRUE(g_isOnTakeOverHibernate);
    EXPECT_EQ(ret2, ERR_OK);

    MessageParcel data3;
    data3.WriteInterfaceToken(TestTakeOverShutdownCallback::GetDescriptor());
    int32_t ret3 = takeOverCallback.OnRemoteRequest(INVALID_CMD_CODE, data3, reply, opt);
    EXPECT_NE(ret3, ERR_OK);
    POWER_HILOGI(LABEL_TEST, "ShutdownCallbackHolderTest010 function end!");
}
} // namespace PowerMgr
} // namespace OHOS