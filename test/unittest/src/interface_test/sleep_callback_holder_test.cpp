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
    SleepCallbackHolder::SleepCallbackContainerType defaultCallbacks =
        sleepCallbackHolder.GetDefaultPriorityCallbacks();
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
    auto pidUid = sleepCallbackHolder.FindCallbackPidUid(callback);
    EXPECT_EQ(pidUid.first, IPCSkeleton::GetCallingPid());
    EXPECT_EQ(pidUid.second, IPCSkeleton::GetCallingUid());

    sleepCallbackHolder.RemoveCallback(callback);
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

/**
 * @tc.name: SleepCallbackHolderTest009
 * @tc.desc: Test Add same callback to different priorities
 * @tc.type: FUNC
 */
HWTEST_F(SleepCallbackHolderTest, SleepCallbackHolderTest009, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "SleepCallbackHolderTest009 function start!");
    SleepCallbackHolder& sleepCallbackHolder = SleepCallbackHolder::GetInstance();
    sptr<TestSleepCallbackHolder> callback = new TestSleepCallbackHolder();

    sleepCallbackHolder.AddCallback(callback, SleepPriority::HIGH);
    EXPECT_EQ(sleepCallbackHolder.GetHighPriorityCallbacks().size(), 1);
    EXPECT_EQ(sleepCallbackHolder.GetDefaultPriorityCallbacks().size(), 0);
    EXPECT_EQ(sleepCallbackHolder.GetLowPriorityCallbacks().size(), 0);

    sleepCallbackHolder.AddCallback(callback, SleepPriority::DEFAULT);
    EXPECT_EQ(sleepCallbackHolder.GetHighPriorityCallbacks().size(), 1);
    EXPECT_EQ(sleepCallbackHolder.GetDefaultPriorityCallbacks().size(), 1);
    EXPECT_EQ(sleepCallbackHolder.GetLowPriorityCallbacks().size(), 0);

    sleepCallbackHolder.AddCallback(callback, SleepPriority::LOW);
    EXPECT_EQ(sleepCallbackHolder.GetHighPriorityCallbacks().size(), 1);
    EXPECT_EQ(sleepCallbackHolder.GetDefaultPriorityCallbacks().size(), 1);
    EXPECT_EQ(sleepCallbackHolder.GetLowPriorityCallbacks().size(), 1);

    sleepCallbackHolder.RemoveCallback(callback);
    EXPECT_EQ(sleepCallbackHolder.GetHighPriorityCallbacks().size(), 0);
    EXPECT_EQ(sleepCallbackHolder.GetDefaultPriorityCallbacks().size(), 0);
    EXPECT_EQ(sleepCallbackHolder.GetLowPriorityCallbacks().size(), 0);

    POWER_HILOGI(LABEL_TEST, "SleepCallbackHolderTest009 function end!");
}

/**
 * @tc.name: SleepCallbackHolderTest010
 * @tc.desc: Test AddCallback with invalid priority
 * @tc.type: FUNC
 */
HWTEST_F(SleepCallbackHolderTest, SleepCallbackHolderTest010, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "SleepCallbackHolderTest010 function start!");
    SleepCallbackHolder& sleepCallbackHolder = SleepCallbackHolder::GetInstance();
    sptr<TestSleepCallbackHolder> callback = new TestSleepCallbackHolder();

    sleepCallbackHolder.AddCallback(callback, static_cast<SleepPriority>(100));
    auto highCallbacks = sleepCallbackHolder.GetHighPriorityCallbacks();
    auto defaultCallbacks = sleepCallbackHolder.GetDefaultPriorityCallbacks();
    auto lowCallbacks = sleepCallbackHolder.GetLowPriorityCallbacks();
    EXPECT_EQ(highCallbacks.size(), 0);
    EXPECT_EQ(defaultCallbacks.size(), 0);
    EXPECT_EQ(lowCallbacks.size(), 0);

    POWER_HILOGI(LABEL_TEST, "SleepCallbackHolderTest010 function end!");
}

/**
 * @tc.name: SleepCallbackHolderTest011
 * @tc.desc: Test FindCallbackPidUid with non-existent callback
 * @tc.type: FUNC
 */
HWTEST_F(SleepCallbackHolderTest, SleepCallbackHolderTest011, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "SleepCallbackHolderTest011 function start!");
    SleepCallbackHolder& sleepCallbackHolder = SleepCallbackHolder::GetInstance();
    sptr<TestSleepCallbackHolder> callback = new TestSleepCallbackHolder();

    auto pidUid = sleepCallbackHolder.FindCallbackPidUid(callback);
    EXPECT_EQ(pidUid.first, 0);
    EXPECT_EQ(pidUid.second, 0);

    sleepCallbackHolder.AddCallback(callback, SleepPriority::HIGH);
    pidUid = sleepCallbackHolder.FindCallbackPidUid(callback);
    EXPECT_EQ(pidUid.first, IPCSkeleton::GetCallingPid());
    EXPECT_EQ(pidUid.second, IPCSkeleton::GetCallingUid());

    sleepCallbackHolder.RemoveCallback(callback);
    pidUid = sleepCallbackHolder.FindCallbackPidUid(callback);
    EXPECT_EQ(pidUid.first, 0);
    EXPECT_EQ(pidUid.second, 0);

    POWER_HILOGI(LABEL_TEST, "SleepCallbackHolderTest011 function end!");
}

/**
 * @tc.name: SleepCallbackHolderTest012
 * @tc.desc: Test RemoveCallback with null callback
 * @tc.type: FUNC
 */
HWTEST_F(SleepCallbackHolderTest, SleepCallbackHolderTest012, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "SleepCallbackHolderTest012 function start!");
    SleepCallbackHolder& sleepCallbackHolder = SleepCallbackHolder::GetInstance();

    auto highCallbacks = sleepCallbackHolder.GetHighPriorityCallbacks();
    auto defaultCallbacks = sleepCallbackHolder.GetDefaultPriorityCallbacks();
    auto lowCallbacks = sleepCallbackHolder.GetLowPriorityCallbacks();
    EXPECT_EQ(highCallbacks.size(), 0);
    EXPECT_EQ(defaultCallbacks.size(), 0);
    EXPECT_EQ(lowCallbacks.size(), 0);

    POWER_HILOGI(LABEL_TEST, "SleepCallbackHolderTest012 function end!");
}

/**
 * @tc.name: SleepCallbackHolderTest013
 * @tc.desc: Test all priorities get empty after all callbacks removed
 * @tc.type: FUNC
 */
HWTEST_F(SleepCallbackHolderTest, SleepCallbackHolderTest013, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "SleepCallbackHolderTest013 function start!");
    SleepCallbackHolder& sleepCallbackHolder = SleepCallbackHolder::GetInstance();

    sptr<TestSleepCallbackHolder> h1 = new TestSleepCallbackHolder();
    sptr<TestSleepCallbackHolder> h2 = new TestSleepCallbackHolder();
    sptr<TestSleepCallbackHolder> d1 = new TestSleepCallbackHolder();
    sptr<TestSleepCallbackHolder> d2 = new TestSleepCallbackHolder();
    sptr<TestSleepCallbackHolder> l1 = new TestSleepCallbackHolder();
    sptr<TestSleepCallbackHolder> l2 = new TestSleepCallbackHolder();

    sleepCallbackHolder.AddCallback(h1, SleepPriority::HIGH);
    sleepCallbackHolder.AddCallback(h2, SleepPriority::HIGH);
    sleepCallbackHolder.AddCallback(d1, SleepPriority::DEFAULT);
    sleepCallbackHolder.AddCallback(d2, SleepPriority::DEFAULT);
    sleepCallbackHolder.AddCallback(l1, SleepPriority::LOW);
    sleepCallbackHolder.AddCallback(l2, SleepPriority::LOW);

    EXPECT_EQ(sleepCallbackHolder.GetHighPriorityCallbacks().size(), 2);
    EXPECT_EQ(sleepCallbackHolder.GetDefaultPriorityCallbacks().size(), 2);
    EXPECT_EQ(sleepCallbackHolder.GetLowPriorityCallbacks().size(), 2);

    sleepCallbackHolder.RemoveCallback(h1);
    sleepCallbackHolder.RemoveCallback(h2);
    sleepCallbackHolder.RemoveCallback(d1);
    sleepCallbackHolder.RemoveCallback(d2);
    sleepCallbackHolder.RemoveCallback(l1);
    sleepCallbackHolder.RemoveCallback(l2);

    EXPECT_EQ(sleepCallbackHolder.GetHighPriorityCallbacks().size(), 0);
    EXPECT_EQ(sleepCallbackHolder.GetDefaultPriorityCallbacks().size(), 0);
    EXPECT_EQ(sleepCallbackHolder.GetLowPriorityCallbacks().size(), 0);

    POWER_HILOGI(LABEL_TEST, "SleepCallbackHolderTest013 function end!");
}

/**
 * @tc.name: SleepCallbackHolderTest014
 * @tc.desc: Test callback with same object but different priority add sequence
 * @tc.type: FUNC
 */
HWTEST_F(SleepCallbackHolderTest, SleepCallbackHolderTest014, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "SleepCallbackHolderTest014 function start!");
    SleepCallbackHolder& sleepCallbackHolder = SleepCallbackHolder::GetInstance();
    sptr<TestSleepCallbackHolder> callback = new TestSleepCallbackHolder();

    sleepCallbackHolder.AddCallback(callback, SleepPriority::LOW);
    sleepCallbackHolder.AddCallback(callback, SleepPriority::DEFAULT);
    sleepCallbackHolder.AddCallback(callback, SleepPriority::HIGH);

    EXPECT_EQ(sleepCallbackHolder.GetHighPriorityCallbacks().size(), 1);
    EXPECT_EQ(sleepCallbackHolder.GetDefaultPriorityCallbacks().size(), 1);
    EXPECT_EQ(sleepCallbackHolder.GetLowPriorityCallbacks().size(), 1);

    sleepCallbackHolder.RemoveCallback(callback);
    EXPECT_EQ(sleepCallbackHolder.GetHighPriorityCallbacks().size(), 0);
    EXPECT_EQ(sleepCallbackHolder.GetDefaultPriorityCallbacks().size(), 0);
    EXPECT_EQ(sleepCallbackHolder.GetLowPriorityCallbacks().size(), 0);

    POWER_HILOGI(LABEL_TEST, "SleepCallbackHolderTest014 function end!");
}

/**
 * @tc.name: SleepCallbackHolderTest015
 * @tc.desc: Test batch add and remove operations
 * @tc.type: FUNC
 */
HWTEST_F(SleepCallbackHolderTest, SleepCallbackHolderTest015, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "SleepCallbackHolderTest015 function start!");
    SleepCallbackHolder& sleepCallbackHolder = SleepCallbackHolder::GetInstance();

    std::vector<sptr<TestSleepCallbackHolder>> highCbs;
    std::vector<sptr<TestSleepCallbackHolder>> defaultCbs;
    std::vector<sptr<TestSleepCallbackHolder>> lowCbs;

    for (int i = 0; i < 5; ++i) {
        highCbs.push_back(new TestSleepCallbackHolder());
        defaultCbs.push_back(new TestSleepCallbackHolder());
        lowCbs.push_back(new TestSleepCallbackHolder());
    }

    for (auto& cb : highCbs) {
        sleepCallbackHolder.AddCallback(cb, SleepPriority::HIGH);
    }
    for (auto& cb : defaultCbs) {
        sleepCallbackHolder.AddCallback(cb, SleepPriority::DEFAULT);
    }
    for (auto& cb : lowCbs) {
        sleepCallbackHolder.AddCallback(cb, SleepPriority::LOW);
    }

    EXPECT_EQ(sleepCallbackHolder.GetHighPriorityCallbacks().size(), 5);
    EXPECT_EQ(sleepCallbackHolder.GetDefaultPriorityCallbacks().size(), 5);
    EXPECT_EQ(sleepCallbackHolder.GetLowPriorityCallbacks().size(), 5);

    for (auto& cb : highCbs) {
        sleepCallbackHolder.RemoveCallback(cb);
    }
    EXPECT_EQ(sleepCallbackHolder.GetHighPriorityCallbacks().size(), 0);
    EXPECT_EQ(sleepCallbackHolder.GetDefaultPriorityCallbacks().size(), 5);
    EXPECT_EQ(sleepCallbackHolder.GetLowPriorityCallbacks().size(), 5);

    for (auto& cb : defaultCbs) {
        sleepCallbackHolder.RemoveCallback(cb);
    }
    EXPECT_EQ(sleepCallbackHolder.GetHighPriorityCallbacks().size(), 0);
    EXPECT_EQ(sleepCallbackHolder.GetDefaultPriorityCallbacks().size(), 0);
    EXPECT_EQ(sleepCallbackHolder.GetLowPriorityCallbacks().size(), 5);

    for (auto& cb : lowCbs) {
        sleepCallbackHolder.RemoveCallback(cb);
    }
    EXPECT_EQ(sleepCallbackHolder.GetHighPriorityCallbacks().size(), 0);
    EXPECT_EQ(sleepCallbackHolder.GetDefaultPriorityCallbacks().size(), 0);
    EXPECT_EQ(sleepCallbackHolder.GetLowPriorityCallbacks().size(), 0);

    POWER_HILOGI(LABEL_TEST, "SleepCallbackHolderTest015 function end!");
}

/**
 * @tc.name: SleepCallbackHolderTest016
 * @tc.desc: Test SyncSleepCallbackStub with different force sleep values
 * @tc.type: FUNC
 */
HWTEST_F(SleepCallbackHolderTest, SleepCallbackHolderTest016, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "SleepCallbackHolderTest016 function start!");
    TestSleepCallbackHolder callback;
    MessageParcel reply;
    MessageOption opt;

    g_isOnSyncSleep = false;
    g_forceSleep = false;

    MessageParcel data1;
    data1.WriteInterfaceToken(TestSleepCallbackHolder::GetDescriptor());
    data1.WriteBool(false);
    int32_t ret1 = callback.OnRemoteRequest(
        static_cast<uint32_t>(PowerMgr::SyncSleepCallbackInterfaceCode::CMD_ON_SYNC_SLEEP),
        data1, reply, opt);
    EXPECT_EQ(ret1, ERR_OK);
    EXPECT_TRUE(g_isOnSyncSleep);
    EXPECT_FALSE(g_forceSleep);

    g_isOnSyncWakeup = false;
    g_forceWakeup = false;

    MessageParcel data2;
    data2.WriteInterfaceToken(TestSleepCallbackHolder::GetDescriptor());
    data2.WriteBool(false);
    int32_t ret2 = callback.OnRemoteRequest(
        static_cast<uint32_t>(PowerMgr::SyncSleepCallbackInterfaceCode::CMD_ON_SYNC_WAKEUP),
        data2, reply, opt);
    EXPECT_EQ(ret2, ERR_OK);
    EXPECT_TRUE(g_isOnSyncWakeup);
    EXPECT_FALSE(g_forceWakeup);

    POWER_HILOGI(LABEL_TEST, "SleepCallbackHolderTest016 function end!");
}

/**
 * @tc.name: SleepCallbackHolderTest017
 * @tc.desc: Test SyncSleepCallbackStub with invalid parcel data
 * @tc.type: FUNC
 */
HWTEST_F(SleepCallbackHolderTest, SleepCallbackHolderTest017, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "SleepCallbackHolderTest017 function start!");
    TestSleepCallbackHolder callback;
    MessageParcel reply;
    MessageOption opt;

    MessageParcel data1;
    data1.WriteInterfaceToken(TestSleepCallbackHolder::GetDescriptor());
    int32_t ret1 = callback.OnRemoteRequest(
        static_cast<uint32_t>(PowerMgr::SyncSleepCallbackInterfaceCode::CMD_ON_SYNC_SLEEP),
        data1, reply, opt);
    EXPECT_NE(ret1, ERR_OK);

    MessageParcel data2;
    data2.WriteInterfaceToken(TestSleepCallbackHolder::GetDescriptor());
    int32_t ret2 = callback.OnRemoteRequest(
        static_cast<uint32_t>(PowerMgr::SyncSleepCallbackInterfaceCode::CMD_ON_SYNC_WAKEUP),
        data2, reply, opt);
    EXPECT_NE(ret2, ERR_OK);

    POWER_HILOGI(LABEL_TEST, "SleepCallbackHolderTest017 function end!");
}
} // namespace PowerMgr
} // namespace OHOS