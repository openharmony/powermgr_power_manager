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

#include "sleep_callback_holder_test.h"
#include <ipc_skeleton.h>
#include "power_common.h"
#include "power_log.h"
#include "sync_sleep_callback_stub.h"
#include "sleep_priority.h"
#include "suspend/sleep_callback_holder.h"
#include "suspend/sync_sleep_callback_ipc_interface_code.h"

namespace OHOS {
namespace PowerMgr {

using namespace testing;
using namespace testing::ext;

namespace {
    bool g_isOnSyncSleep = false;
    bool g_isOnSyncWakeup = false;
    bool g_forceSleep = false;
    bool g_forceWakeup = false;
    constexpr uint32_t INVALID_CMD_CODE = 100;
} // namespace

class TestSleepCallbackHolder : public SyncSleepCallbackStub {
    public:
        TestSleepCallbackHolder() = default;
        virtual ~TestSleepCallbackHolder() = default;

        void OnSyncSleep(bool onForceSleep) override
        {
            g_isOnSyncSleep = true;
            g_forceSleep = onForceSleep;
            POWER_HILOGI(LABEL_TEST, "TestSleepCallbackHolder OnSyncSleep function start!");
        }
        void OnSyncWakeup(bool onForceSleep) override
        {
            g_isOnSyncWakeup = true;
            g_forceWakeup = onForceSleep;
            POWER_HILOGI(LABEL_TEST, "TestSleepCallbackHolder OnSyncWakeup function start!");
        }
};
void SleepCallbackHolderTest::SetUpTestCase()
{
}

void SleepCallbackHolderTest::TearDownTestCase()
{
}

void SleepCallbackHolderTest::SetUp()
{
}

void SleepCallbackHolderTest::TearDown()
{
}

/**
 * @tc.name: SleepCallbackHolderTest001
 * @tc.desc: Test AddCallback and RemoveCallback
 * @tc.type: FUNC
 */
HWTEST_F(SleepCallbackHolderTest, SleepCallbackHolderTest001, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "SleepCallbackHolderTest001 function start!");
    SleepCallbackHolder& sleepCallbackHolder = SleepCallbackHolder::GetInstance();

    sptr<TestSleepCallbackHolder> callbackHigh = new TestSleepCallbackHolder();
    sptr<TestSleepCallbackHolder> callbackDefault = new TestSleepCallbackHolder();
    sptr<TestSleepCallbackHolder> callbackLow = new TestSleepCallbackHolder();

    sleepCallbackHolder.AddCallback(callbackHigh, SleepPriority::HIGH);
    sleepCallbackHolder.AddCallback(callbackDefault, SleepPriority::DEFAULT);
    sleepCallbackHolder.AddCallback(callbackLow, SleepPriority::LOW);

    SleepCallbackHolder::SleepCallbackContainerType highCallbacks = sleepCallbackHolder.GetHighPriorityCallbacks();
    SleepCallbackHolder::SleepCallbackContainerType defaultCallbacks = sleepCallbackHolder.GetDefaultPriorityCallbacks();
    SleepCallbackHolder::SleepCallbackContainerType lowCallbacks = sleepCallbackHolder.GetLowPriorityCallbacks();
    EXPECT_EQ(highCallbacks.size(), 1);
    EXPECT_EQ(defaultCallbacks.size(), 1);
    EXPECT_EQ(lowCallbacks.size(), 1);

    sleepCallbackHolder.RemoveCallback(callbackHigh);
    sleepCallbackHolder.RemoveCallback(callbackDefault);
    sleepCallbackHolder.RemoveCallback(callbackLow);
    highCallbacks = sleepCallbackHolder.GetHighPriorityCallbacks();
    defaultCallbacks = sleepCallbackHolder.GetDefaultPriorityCallbacks();
    lowCallbacks = sleepCallbackHolder.GetLowPriorityCallbacks();
    EXPECT_EQ(highCallbacks.size(), 0);
    EXPECT_EQ(defaultCallbacks.size(), 0);
    EXPECT_EQ(lowCallbacks.size(), 0);

    POWER_HILOGI(LABEL_TEST, "SleepCallbackHolderTest001 function end!");
}

/**
 * @tc.name: SleepCallbackHolderTest002
 * @tc.desc: Test FindCallbackPidUid
 * @tc.type: FUNC
 */
HWTEST_F(SleepCallbackHolderTest, SleepCallbackHolderTest002, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "SleepCallbackHolderTest002 function start!");
    SleepCallbackHolder& sleepCallbackHolder = SleepCallbackHolder::GetInstance();
    sptr<TestSleepCallbackHolder> callback = new TestSleepCallbackHolder();

    sleepCallbackHolder.AddCallback(callback, SleepPriority::HIGH);
    EXPECT_EQ(sleepCallbackHolder.cachedRegister_.size(), 1);
    auto pidUid = sleepCallbackHolder.FindCallbackPidUid(callback);
    EXPECT_EQ(pidUid.first, IPCSkeleton::GetCallingPid());
    EXPECT_EQ(pidUid.second, IPCSkeleton::GetCallingUid());

    sleepCallbackHolder.RemoveCallback(callback);
    EXPECT_EQ(sleepCallbackHolder.cachedRegister_.size(), 0);
    pidUid = sleepCallbackHolder.FindCallbackPidUid(callback);
    EXPECT_EQ(pidUid.first, 0);
    EXPECT_EQ(pidUid.second, 0);
    POWER_HILOGI(LABEL_TEST, "SleepCallbackHolderTest002 function end!");
}

/**
 * @tc.name: SleepCallbackHolderTest003
 * @tc.desc: Test duplicate callback operations
 * @tc.type: FUNC
 */
HWTEST_F(SleepCallbackHolderTest, SleepCallbackHolderTest003, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "SleepCallbackHolderTest003 function start!");
    SleepCallbackHolder& sleepCallbackHolder = SleepCallbackHolder::GetInstance();
    sptr<TestSleepCallbackHolder> callback = new TestSleepCallbackHolder();

    sleepCallbackHolder.AddCallback(callback, SleepPriority::HIGH);
    SleepCallbackHolder::SleepCallbackContainerType highCallbacks = sleepCallbackHolder.GetHighPriorityCallbacks();
    EXPECT_EQ(highCallbacks.size(), 1);
    sleepCallbackHolder.AddCallback(callback, SleepPriority::HIGH);
    highCallbacks = sleepCallbackHolder.GetHighPriorityCallbacks();
    EXPECT_EQ(highCallbacks.size(), 1);

    sleepCallbackHolder.RemoveCallback(callback);
    POWER_HILOGI(LABEL_TEST, "SleepCallbackHolderTest003 function end!");
}

/**
 * @tc.name: SleepCallbackHolderTest004
 * @tc.desc: Test removing non-existent callback from specific container
 * @tc.type: FUNC
 */
HWTEST_F(SleepCallbackHolderTest, SleepCallbackHolderTest004, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "SleepCallbackHolderTest004 function start!");
    SleepCallbackHolder& sleepCallbackHolder = SleepCallbackHolder::GetInstance();
    sptr<TestSleepCallbackHolder> callback = new TestSleepCallbackHolder();

    sleepCallbackHolder.RemoveCallback(callback);

    auto highCallbacks = sleepCallbackHolder.GetHighPriorityCallbacks();
    auto defaultCallbacks = sleepCallbackHolder.GetDefaultPriorityCallbacks();
    auto lowCallbacks = sleepCallbackHolder.GetLowPriorityCallbacks();

    EXPECT_EQ(highCallbacks.size(), 0);
    EXPECT_EQ(defaultCallbacks.size(), 0);
    EXPECT_EQ(lowCallbacks.size(), 0);

    POWER_HILOGI(LABEL_TEST, "SleepCallbackHolderTest004 function end!");
}

/**
 * @tc.name: SleepCallbackHolderTest005
 * @tc.desc: Test removing the same callback multiple times
 * @tc.type: FUNC
 */
HWTEST_F(SleepCallbackHolderTest, SleepCallbackHolderTest005, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "SleepCallbackHolderTest005 function start!");
    SleepCallbackHolder& sleepCallbackHolder = SleepCallbackHolder::GetInstance();
    sptr<TestSleepCallbackHolder> callback = new TestSleepCallbackHolder();

    sleepCallbackHolder.AddCallback(callback, SleepPriority::HIGH);
    EXPECT_EQ(sleepCallbackHolder.GetHighPriorityCallbacks().size(), 1);
    sleepCallbackHolder.RemoveCallback(callback);
    EXPECT_EQ(sleepCallbackHolder.GetHighPriorityCallbacks().size(), 0);
    sleepCallbackHolder.RemoveCallback(callback);
    EXPECT_EQ(sleepCallbackHolder.GetHighPriorityCallbacks().size(), 0);
    sleepCallbackHolder.RemoveCallback(callback);
    EXPECT_EQ(sleepCallbackHolder.GetHighPriorityCallbacks().size(), 0);

    POWER_HILOGI(LABEL_TEST, "SleepCallbackHolderTest005 function end!");
}

/**
 * @tc.name: SleepCallbackHolderTest006
 * @tc.desc: Test multiple callbacks with different priorities
 * @tc.type: FUNC
 */
HWTEST_F(SleepCallbackHolderTest, SleepCallbackHolderTest006, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "SleepCallbackHolderTest006 function start!");
    SleepCallbackHolder& sleepCallbackHolder = SleepCallbackHolder::GetInstance();

    sptr<TestSleepCallbackHolder> high1 = new TestSleepCallbackHolder();
    sptr<TestSleepCallbackHolder> high2 = new TestSleepCallbackHolder();
    sptr<TestSleepCallbackHolder> default1 = new TestSleepCallbackHolder();
    sptr<TestSleepCallbackHolder> default2 = new TestSleepCallbackHolder();
    sptr<TestSleepCallbackHolder> low1 = new TestSleepCallbackHolder();
    sptr<TestSleepCallbackHolder> low2 = new TestSleepCallbackHolder();

    sleepCallbackHolder.AddCallback(high1, SleepPriority::HIGH);
    sleepCallbackHolder.AddCallback(high2, SleepPriority::HIGH);
    sleepCallbackHolder.AddCallback(default1, SleepPriority::DEFAULT);
    sleepCallbackHolder.AddCallback(default2, SleepPriority::DEFAULT);
    sleepCallbackHolder.AddCallback(low1, SleepPriority::LOW);
    sleepCallbackHolder.AddCallback(low2, SleepPriority::LOW);

    EXPECT_EQ(sleepCallbackHolder.GetHighPriorityCallbacks().size(), 2);
    EXPECT_EQ(sleepCallbackHolder.GetDefaultPriorityCallbacks().size(), 2);
    EXPECT_EQ(sleepCallbackHolder.GetLowPriorityCallbacks().size(), 2);

    sleepCallbackHolder.RemoveCallback(high1);
    sleepCallbackHolder.RemoveCallback(default2);
    sleepCallbackHolder.RemoveCallback(low1);

    EXPECT_EQ(sleepCallbackHolder.GetHighPriorityCallbacks().size(), 1);
    EXPECT_EQ(sleepCallbackHolder.GetDefaultPriorityCallbacks().size(), 1);
    EXPECT_EQ(sleepCallbackHolder.GetLowPriorityCallbacks().size(), 1);

    sleepCallbackHolder.RemoveCallback(high2);
    sleepCallbackHolder.RemoveCallback(default1);
    sleepCallbackHolder.RemoveCallback(low2);

    EXPECT_EQ(sleepCallbackHolder.GetHighPriorityCallbacks().size(), 0);
    EXPECT_EQ(sleepCallbackHolder.GetDefaultPriorityCallbacks().size(), 0);
    EXPECT_EQ(sleepCallbackHolder.GetLowPriorityCallbacks().size(), 0);

    POWER_HILOGI(LABEL_TEST, "SleepCallbackHolderTest006 function end!");
}

/**
 * @tc.name: SleepCallbackHolderTest007
 * @tc.desc: Test SyncSleepCallbackStub
 * @tc.type: FUNC
 */
HWTEST_F(SleepCallbackHolderTest, SleepCallbackHolderTest007, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "SleepCallbackHolderTest007 function start!");
    TestSleepCallbackHolder callback;
    MessageParcel reply;
    MessageOption opt;

    MessageParcel data0;
    data0.WriteInterfaceToken(u"test.interface.token");
    int32_t ret0 = callback.OnRemoteRequest(
        static_cast<uint32_t>(PowerMgr::SyncSleepCallbackInterfaceCode::CMD_ON_SYNC_SLEEP),
        data0, reply, opt);
    EXPECT_EQ(ret0, E_GET_POWER_SERVICE_FAILED);

    MessageParcel data1;
    data1.WriteInterfaceToken(TestSleepCallbackHolder::GetDescriptor());
    data1.WriteBool(true);
    int32_t ret1 = callback.OnRemoteRequest(
        static_cast<uint32_t>(PowerMgr::SyncSleepCallbackInterfaceCode::CMD_ON_SYNC_SLEEP),
        data1, reply, opt);
    EXPECT_EQ(ret1, ERR_OK);
    EXPECT_TRUE(g_isOnSyncSleep);
    EXPECT_TRUE(g_forceSleep);

    MessageParcel data2;
    data2.WriteInterfaceToken(TestSleepCallbackHolder::GetDescriptor());
    data2.WriteBool(true);
    int32_t ret2 = callback.OnRemoteRequest(
        static_cast<uint32_t>(PowerMgr::SyncSleepCallbackInterfaceCode::CMD_ON_SYNC_WAKEUP),
        data2, reply, opt);
    EXPECT_EQ(ret2, ERR_OK);
    EXPECT_TRUE(g_isOnSyncWakeup);
    EXPECT_TRUE(g_forceWakeup);

    MessageParcel data3;
    data3.WriteInterfaceToken(TestSleepCallbackHolder::GetDescriptor());
    int32_t ret3 = callback.OnRemoteRequest(INVALID_CMD_CODE, data3, reply, opt);
    EXPECT_NE(ret3, ERR_OK);

    POWER_HILOGI(LABEL_TEST, "SleepCallbackHolderTest007 function end!");
}
} // namespace PowerMgr
} // namespace OHOS