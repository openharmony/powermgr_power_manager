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

#include "suspend_takeover_callback_holder_test.h"
#include <ipc_skeleton.h>
#include "power_common.h"
#include "power_log.h"
#include "takeover_suspend_callback_stub.h"
#include "takeover_suspend_priority.h"
#include "suspend/suspend_takeover_callback_holder.h"
#include "suspend/take_over_callback_ipc_interface_code.h"

namespace OHOS {
namespace PowerMgr {

using namespace testing;
using namespace testing::ext;
using CallbackContainer = TakeOverSuspendCallbackHolder::TakeoverSuspendCallbackContainerType;

namespace {
    bool g_isTakeOverSuspend = false;
    constexpr uint32_t INVALID_CMD_CODE = 100;
} // namespace

class TestTakeOverSuspendCallback : public TakeOverSuspendCallbackStub {
    public:
        TestTakeOverSuspendCallback() = default;
        virtual ~TestTakeOverSuspendCallback() = default;

        bool OnTakeOverSuspend(SuspendDeviceType type) override
        {
            g_isTakeOverSuspend = true;
            return true;
        }
};

void SuspendTakeoverCallbackHolderTest::SetUpTestCase()
{
}

void SuspendTakeoverCallbackHolderTest::TearDownTestCase()
{
}

void SuspendTakeoverCallbackHolderTest::SetUp()
{
}

void SuspendTakeoverCallbackHolderTest::TearDown()
{
}

/**
 * @tc.name: SuspendTakeoverCallbackHolderTest001
 * @tc.desc: Test AddCallback and RemoveCallback
 * @tc.type: FUNC
 */
HWTEST_F(SuspendTakeoverCallbackHolderTest, SuspendTakeoverCallbackHolderTest001, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "SuspendTakeoverCallbackHolderTest001 function start!");
    TakeOverSuspendCallbackHolder& suspendTakeoverCallbackHolder = TakeOverSuspendCallbackHolder::GetInstance();

    sptr<TestTakeOverSuspendCallback> callbackHigh = new TestTakeOverSuspendCallback();
    sptr<TestTakeOverSuspendCallback> callbackDefault = new TestTakeOverSuspendCallback();
    sptr<TestTakeOverSuspendCallback> callbackLow = new TestTakeOverSuspendCallback();

    suspendTakeoverCallbackHolder.AddCallback(callbackHigh, TakeOverSuspendPriority::HIGH);
    suspendTakeoverCallbackHolder.AddCallback(callbackDefault, TakeOverSuspendPriority::DEFAULT);
    suspendTakeoverCallbackHolder.AddCallback(callbackLow, TakeOverSuspendPriority::LOW);

    CallbackContainer highCallbacks = suspendTakeoverCallbackHolder.GetHighPriorityCallbacks();
    CallbackContainer defaultCallbacks = suspendTakeoverCallbackHolder.GetDefaultPriorityCallbacks();
    CallbackContainer lowCallbacks = suspendTakeoverCallbackHolder.GetLowPriorityCallbacks();
    EXPECT_EQ(highCallbacks.size(), 1);
    EXPECT_EQ(defaultCallbacks.size(), 1);
    EXPECT_EQ(lowCallbacks.size(), 1);

    suspendTakeoverCallbackHolder.RemoveCallback(callbackHigh);
    suspendTakeoverCallbackHolder.RemoveCallback(callbackDefault);
    suspendTakeoverCallbackHolder.RemoveCallback(callbackLow);
    highCallbacks = suspendTakeoverCallbackHolder.GetHighPriorityCallbacks();
    defaultCallbacks = suspendTakeoverCallbackHolder.GetDefaultPriorityCallbacks();
    lowCallbacks = suspendTakeoverCallbackHolder.GetLowPriorityCallbacks();
    EXPECT_EQ(highCallbacks.size(), 0);
    EXPECT_EQ(defaultCallbacks.size(), 0);
    EXPECT_EQ(lowCallbacks.size(), 0);

    POWER_HILOGI(LABEL_TEST, "SuspendTakeoverCallbackHolderTest001 function end!");
}

/**
 * @tc.name: SuspendTakeoverCallbackHolderTest002
 * @tc.desc: Test FindCallbackPidUid
 * @tc.type: FUNC
 */
HWTEST_F(SuspendTakeoverCallbackHolderTest, SuspendTakeoverCallbackHolderTest002, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "SuspendTakeoverCallbackHolderTest002 function start!");
    TakeOverSuspendCallbackHolder& suspendTakeoverCallbackHolder = TakeOverSuspendCallbackHolder::GetInstance();
    sptr<TestTakeOverSuspendCallback> callback = new TestTakeOverSuspendCallback();

    suspendTakeoverCallbackHolder.AddCallback(callback, TakeOverSuspendPriority::HIGH);
    auto pidUid = suspendTakeoverCallbackHolder.FindCallbackPidUid(callback);
    EXPECT_EQ(pidUid.first, IPCSkeleton::GetCallingPid());
    EXPECT_EQ(pidUid.second, IPCSkeleton::GetCallingUid());

    suspendTakeoverCallbackHolder.RemoveCallback(callback);
    pidUid = suspendTakeoverCallbackHolder.FindCallbackPidUid(callback);
    EXPECT_EQ(pidUid.first, 0);
    EXPECT_EQ(pidUid.second, 0);
    POWER_HILOGI(LABEL_TEST, "SuspendTakeoverCallbackHolderTest002 function end!");
}

/**
 * @tc.name: SuspendTakeoverCallbackHolderTest003
 * @tc.desc: Test duplicate callback operations
 * @tc.type: FUNC
 */
HWTEST_F(SuspendTakeoverCallbackHolderTest, SuspendTakeoverCallbackHolderTest003, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "SuspendTakeoverCallbackHolderTest003 function start!");
    TakeOverSuspendCallbackHolder& suspendTakeoverCallbackHolder = TakeOverSuspendCallbackHolder::GetInstance();
    sptr<TestTakeOverSuspendCallback> callback = new TestTakeOverSuspendCallback();

    suspendTakeoverCallbackHolder.AddCallback(callback, TakeOverSuspendPriority::HIGH);
    CallbackContainer highCallbacks = suspendTakeoverCallbackHolder.GetHighPriorityCallbacks();
    EXPECT_EQ(highCallbacks.size(), 1);
    suspendTakeoverCallbackHolder.AddCallback(callback, TakeOverSuspendPriority::HIGH);
    highCallbacks = suspendTakeoverCallbackHolder.GetHighPriorityCallbacks();
    EXPECT_EQ(highCallbacks.size(), 1);

    suspendTakeoverCallbackHolder.RemoveCallback(callback);
    POWER_HILOGI(LABEL_TEST, "SuspendTakeoverCallbackHolderTest003 function end!");
}

/**
 * @tc.name: SuspendTakeoverCallbackHolderTest004
 * @tc.desc: Test removing non-existent callback from specific container
 * @tc.type: FUNC
 */
HWTEST_F(SuspendTakeoverCallbackHolderTest, SuspendTakeoverCallbackHolderTest004, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "SuspendTakeoverCallbackHolderTest004 function start!");
    TakeOverSuspendCallbackHolder& suspendTakeoverCallbackHolder = TakeOverSuspendCallbackHolder::GetInstance();
    sptr<TestTakeOverSuspendCallback> callback = new TestTakeOverSuspendCallback();

    suspendTakeoverCallbackHolder.RemoveCallback(callback);

    auto highCallbacks = suspendTakeoverCallbackHolder.GetHighPriorityCallbacks();
    auto defaultCallbacks = suspendTakeoverCallbackHolder.GetDefaultPriorityCallbacks();
    auto lowCallbacks = suspendTakeoverCallbackHolder.GetLowPriorityCallbacks();

    EXPECT_EQ(highCallbacks.size(), 0);
    EXPECT_EQ(defaultCallbacks.size(), 0);
    EXPECT_EQ(lowCallbacks.size(), 0);

    POWER_HILOGI(LABEL_TEST, "SuspendTakeoverCallbackHolderTest004 function end!");
}

/**
 * @tc.name: SuspendTakeoverCallbackHolderTest005
 * @tc.desc: Test removing the same callback multiple times
 * @tc.type: FUNC
 */
HWTEST_F(SuspendTakeoverCallbackHolderTest, SuspendTakeoverCallbackHolderTest005, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "SuspendTakeoverCallbackHolderTest005 function start!");
    TakeOverSuspendCallbackHolder& suspendTakeoverCallbackHolder = TakeOverSuspendCallbackHolder::GetInstance();
    sptr<TestTakeOverSuspendCallback> callback = new TestTakeOverSuspendCallback();

    suspendTakeoverCallbackHolder.AddCallback(callback, TakeOverSuspendPriority::HIGH);
    EXPECT_EQ(suspendTakeoverCallbackHolder.GetHighPriorityCallbacks().size(), 1);
    suspendTakeoverCallbackHolder.RemoveCallback(callback);
    EXPECT_EQ(suspendTakeoverCallbackHolder.GetHighPriorityCallbacks().size(), 0);
    suspendTakeoverCallbackHolder.RemoveCallback(callback);
    EXPECT_EQ(suspendTakeoverCallbackHolder.GetHighPriorityCallbacks().size(), 0);
    suspendTakeoverCallbackHolder.RemoveCallback(callback);
    EXPECT_EQ(suspendTakeoverCallbackHolder.GetHighPriorityCallbacks().size(), 0);

    POWER_HILOGI(LABEL_TEST, "SuspendTakeoverCallbackHolderTest005 function end!");
}

/**
 * @tc.name: SuspendTakeoverCallbackHolderTest006
 * @tc.desc: Test multiple callbacks with different priorities
 * @tc.type: FUNC
 */
HWTEST_F(SuspendTakeoverCallbackHolderTest, SuspendTakeoverCallbackHolderTest006, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "SuspendTakeoverCallbackHolderTest006 function start!");
    TakeOverSuspendCallbackHolder& suspendTakeoverCallbackHolder = TakeOverSuspendCallbackHolder::GetInstance();

    sptr<TestTakeOverSuspendCallback> high1 = new TestTakeOverSuspendCallback();
    sptr<TestTakeOverSuspendCallback> high2 = new TestTakeOverSuspendCallback();
    sptr<TestTakeOverSuspendCallback> default1 = new TestTakeOverSuspendCallback();
    sptr<TestTakeOverSuspendCallback> default2 = new TestTakeOverSuspendCallback();
    sptr<TestTakeOverSuspendCallback> low1 = new TestTakeOverSuspendCallback();
    sptr<TestTakeOverSuspendCallback> low2 = new TestTakeOverSuspendCallback();

    suspendTakeoverCallbackHolder.AddCallback(high1, TakeOverSuspendPriority::HIGH);
    suspendTakeoverCallbackHolder.AddCallback(high2, TakeOverSuspendPriority::HIGH);
    suspendTakeoverCallbackHolder.AddCallback(default1, TakeOverSuspendPriority::DEFAULT);
    suspendTakeoverCallbackHolder.AddCallback(default2, TakeOverSuspendPriority::DEFAULT);
    suspendTakeoverCallbackHolder.AddCallback(low1, TakeOverSuspendPriority::LOW);
    suspendTakeoverCallbackHolder.AddCallback(low2, TakeOverSuspendPriority::LOW);

    EXPECT_EQ(suspendTakeoverCallbackHolder.GetHighPriorityCallbacks().size(), 2);
    EXPECT_EQ(suspendTakeoverCallbackHolder.GetDefaultPriorityCallbacks().size(), 2);
    EXPECT_EQ(suspendTakeoverCallbackHolder.GetLowPriorityCallbacks().size(), 2);

    suspendTakeoverCallbackHolder.RemoveCallback(high1);
    suspendTakeoverCallbackHolder.RemoveCallback(default2);
    suspendTakeoverCallbackHolder.RemoveCallback(low1);

    EXPECT_EQ(suspendTakeoverCallbackHolder.GetHighPriorityCallbacks().size(), 1);
    EXPECT_EQ(suspendTakeoverCallbackHolder.GetDefaultPriorityCallbacks().size(), 1);
    EXPECT_EQ(suspendTakeoverCallbackHolder.GetLowPriorityCallbacks().size(), 1);

    suspendTakeoverCallbackHolder.RemoveCallback(high2);
    suspendTakeoverCallbackHolder.RemoveCallback(default1);
    suspendTakeoverCallbackHolder.RemoveCallback(low2);

    EXPECT_EQ(suspendTakeoverCallbackHolder.GetHighPriorityCallbacks().size(), 0);
    EXPECT_EQ(suspendTakeoverCallbackHolder.GetDefaultPriorityCallbacks().size(), 0);
    EXPECT_EQ(suspendTakeoverCallbackHolder.GetLowPriorityCallbacks().size(), 0);

    POWER_HILOGI(LABEL_TEST, "SuspendTakeoverCallbackHolderTest006 function end!");
}

/**
 * @tc.name: SuspendTakeoverCallbackHolderTest007
 * @tc.desc: Test TakeOverSuspendCallbackStub
 * @tc.type: FUNC
 */
HWTEST_F(SuspendTakeoverCallbackHolderTest, SuspendTakeoverCallbackHolderTest007, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "SuspendTakeoverCallbackHolderTest007 function start!");
    TestTakeOverSuspendCallback callback;
    MessageParcel reply;
    MessageOption opt;

    MessageParcel data0;
    data0.WriteInterfaceToken(u"test.interface.token");
    int32_t ret0 = callback.OnRemoteRequest(
        static_cast<uint32_t>(PowerMgr::TakeOverSuspendCallbackInterfaceCode::CMD_ON_TAKEOVER_SUSPEND),
        data0, reply, opt);
    EXPECT_EQ(ret0, E_GET_POWER_SERVICE_FAILED);

    MessageParcel data1;
    data1.WriteInterfaceToken(TestTakeOverSuspendCallback::GetDescriptor());
    int32_t ret1 = callback.OnRemoteRequest(
        static_cast<uint32_t>(PowerMgr::TakeOverSuspendCallbackInterfaceCode::CMD_ON_TAKEOVER_SUSPEND),
        data1, reply, opt);
    EXPECT_EQ(ret1, E_READ_PARCEL_ERROR);

    MessageParcel data2;
    data2.WriteInterfaceToken(TestTakeOverSuspendCallback::GetDescriptor());
    SuspendDeviceType type2 = SuspendDeviceType::SUSPEND_DEVICE_REASON_POWER_KEY;
    data2.WriteUint32(static_cast<uint32_t>(type2));
    int32_t ret2 = callback.OnRemoteRequest(
        static_cast<uint32_t>(PowerMgr::TakeOverSuspendCallbackInterfaceCode::CMD_ON_TAKEOVER_SUSPEND),
        data2, reply, opt);
    EXPECT_EQ(ret2, ERR_OK);
    EXPECT_EQ(g_isTakeOverSuspend, true);
    bool retVal = callback.TakeOverSuspendCallbackStub::OnTakeOverSuspend(type2);
    EXPECT_EQ(retVal, false);

    MessageParcel data3;
    data3.WriteInterfaceToken(TestTakeOverSuspendCallback::GetDescriptor());
    data3.WriteUint32(100);
    int32_t ret3 = callback.OnRemoteRequest(
        static_cast<uint32_t>(PowerMgr::TakeOverSuspendCallbackInterfaceCode::CMD_ON_TAKEOVER_SUSPEND),
        data3, reply, opt);
    EXPECT_EQ(ret3, E_INNER_ERR);

    MessageParcel data4;
    data4.WriteInterfaceToken(TestTakeOverSuspendCallback::GetDescriptor());
    SuspendDeviceType type4 = SuspendDeviceType::SUSPEND_DEVICE_REASON_FORCE_SUSPEND;
    data4.WriteUint32(static_cast<uint32_t>(type4));
    int32_t ret4 = callback.OnRemoteRequest(INVALID_CMD_CODE, data4, reply, opt);
    EXPECT_NE(ret4, ERR_OK);

    POWER_HILOGI(LABEL_TEST, "SuspendTakeoverCallbackHolderTest007 function end!");
}

/**
 * @tc.name: SuspendTakeoverCallbackHolderTest009
 * @tc.desc: Test Add same callback to different priorities
 * @tc.type: FUNC
 */
HWTEST_F(SuspendTakeoverCallbackHolderTest, SuspendTakeoverCallbackHolderTest009, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "SuspendTakeoverCallbackHolderTest009 function start!");
    TakeOverSuspendCallbackHolder& suspendTakeoverCallbackHolder = TakeOverSuspendCallbackHolder::GetInstance();
    sptr<TestTakeOverSuspendCallback> callback = new TestTakeOverSuspendCallback();

    suspendTakeoverCallbackHolder.AddCallback(callback, TakeOverSuspendPriority::HIGH);
    EXPECT_EQ(suspendTakeoverCallbackHolder.GetHighPriorityCallbacks().size(), 1);
    EXPECT_EQ(suspendTakeoverCallbackHolder.GetDefaultPriorityCallbacks().size(), 0);
    EXPECT_EQ(suspendTakeoverCallbackHolder.GetLowPriorityCallbacks().size(), 0);

    suspendTakeoverCallbackHolder.AddCallback(callback, TakeOverSuspendPriority::DEFAULT);
    EXPECT_EQ(suspendTakeoverCallbackHolder.GetHighPriorityCallbacks().size(), 1);
    EXPECT_EQ(suspendTakeoverCallbackHolder.GetDefaultPriorityCallbacks().size(), 1);
    EXPECT_EQ(suspendTakeoverCallbackHolder.GetLowPriorityCallbacks().size(), 0);

    suspendTakeoverCallbackHolder.AddCallback(callback, TakeOverSuspendPriority::LOW);
    EXPECT_EQ(suspendTakeoverCallbackHolder.GetHighPriorityCallbacks().size(), 1);
    EXPECT_EQ(suspendTakeoverCallbackHolder.GetDefaultPriorityCallbacks().size(), 1);
    EXPECT_EQ(suspendTakeoverCallbackHolder.GetLowPriorityCallbacks().size(), 1);

    suspendTakeoverCallbackHolder.RemoveCallback(callback);
    EXPECT_EQ(suspendTakeoverCallbackHolder.GetHighPriorityCallbacks().size(), 0);
    EXPECT_EQ(suspendTakeoverCallbackHolder.GetDefaultPriorityCallbacks().size(), 0);
    EXPECT_EQ(suspendTakeoverCallbackHolder.GetLowPriorityCallbacks().size(), 0);

    POWER_HILOGI(LABEL_TEST, "SuspendTakeoverCallbackHolderTest009 function end!");
}

/**
 * @tc.name: SuspendTakeoverCallbackHolderTest010
 * @tc.desc: Test AddCallback with invalid priority
 * @tc.type: FUNC
 */
HWTEST_F(SuspendTakeoverCallbackHolderTest, SuspendTakeoverCallbackHolderTest010, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "SuspendTakeoverCallbackHolderTest010 function start!");
    TakeOverSuspendCallbackHolder& suspendTakeoverCallbackHolder = TakeOverSuspendCallbackHolder::GetInstance();
    sptr<TestTakeOverSuspendCallback> callback = new TestTakeOverSuspendCallback();

    suspendTakeoverCallbackHolder.AddCallback(callback, static_cast<TakeOverSuspendPriority>(100));
    auto highCallbacks = suspendTakeoverCallbackHolder.GetHighPriorityCallbacks();
    auto defaultCallbacks = suspendTakeoverCallbackHolder.GetDefaultPriorityCallbacks();
    auto lowCallbacks = suspendTakeoverCallbackHolder.GetLowPriorityCallbacks();
    EXPECT_EQ(highCallbacks.size(), 0);
    EXPECT_EQ(defaultCallbacks.size(), 0);
    EXPECT_EQ(lowCallbacks.size(), 0);

    POWER_HILOGI(LABEL_TEST, "SuspendTakeoverCallbackHolderTest010 function end!");
}

/**
 * @tc.name: SuspendTakeoverCallbackHolderTest011
 * @tc.desc: Test FindCallbackPidUid with non-existent callback
 * @tc.type: FUNC
 */
HWTEST_F(SuspendTakeoverCallbackHolderTest, SuspendTakeoverCallbackHolderTest011, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "SuspendTakeoverCallbackHolderTest011 function start!");
    TakeOverSuspendCallbackHolder& suspendTakeoverCallbackHolder = TakeOverSuspendCallbackHolder::GetInstance();
    sptr<TestTakeOverSuspendCallback> callback = new TestTakeOverSuspendCallback();

    auto pidUid = suspendTakeoverCallbackHolder.FindCallbackPidUid(callback);
    EXPECT_EQ(pidUid.first, 0);
    EXPECT_EQ(pidUid.second, 0);

    suspendTakeoverCallbackHolder.AddCallback(callback, TakeOverSuspendPriority::HIGH);
    pidUid = suspendTakeoverCallbackHolder.FindCallbackPidUid(callback);
    EXPECT_EQ(pidUid.first, IPCSkeleton::GetCallingPid());
    EXPECT_EQ(pidUid.second, IPCSkeleton::GetCallingUid());

    suspendTakeoverCallbackHolder.RemoveCallback(callback);
    pidUid = suspendTakeoverCallbackHolder.FindCallbackPidUid(callback);
    EXPECT_EQ(pidUid.first, 0);
    EXPECT_EQ(pidUid.second, 0);

    POWER_HILOGI(LABEL_TEST, "SuspendTakeoverCallbackHolderTest011 function end!");
}

/**
 * @tc.name: SuspendTakeoverCallbackHolderTest012
 * @tc.desc: Test RemoveCallback with null callback
 * @tc.type: FUNC
 */
HWTEST_F(SuspendTakeoverCallbackHolderTest, SuspendTakeoverCallbackHolderTest012, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "SuspendTakeoverCallbackHolderTest012 function start!");
    TakeOverSuspendCallbackHolder& suspendTakeoverCallbackHolder = TakeOverSuspendCallbackHolder::GetInstance();

    auto highCallbacks = suspendTakeoverCallbackHolder.GetHighPriorityCallbacks();
    auto defaultCallbacks = suspendTakeoverCallbackHolder.GetDefaultPriorityCallbacks();
    auto lowCallbacks = suspendTakeoverCallbackHolder.GetLowPriorityCallbacks();
    EXPECT_EQ(highCallbacks.size(), 0);
    EXPECT_EQ(defaultCallbacks.size(), 0);
    EXPECT_EQ(lowCallbacks.size(), 0);

    POWER_HILOGI(LABEL_TEST, "SuspendTakeoverCallbackHolderTest012 function end!");
}

/**
 * @tc.name: SuspendTakeoverCallbackHolderTest013
 * @tc.desc: Test all priorities get empty after all callbacks removed
 * @tc.type: FUNC
 */
HWTEST_F(SuspendTakeoverCallbackHolderTest, SuspendTakeoverCallbackHolderTest013, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "SuspendTakeoverCallbackHolderTest013 function start!");
    TakeOverSuspendCallbackHolder& suspendTakeoverCallbackHolder = TakeOverSuspendCallbackHolder::GetInstance();

    sptr<TestTakeOverSuspendCallback> h1 = new TestTakeOverSuspendCallback();
    sptr<TestTakeOverSuspendCallback> h2 = new TestTakeOverSuspendCallback();
    sptr<TestTakeOverSuspendCallback> d1 = new TestTakeOverSuspendCallback();
    sptr<TestTakeOverSuspendCallback> d2 = new TestTakeOverSuspendCallback();
    sptr<TestTakeOverSuspendCallback> l1 = new TestTakeOverSuspendCallback();
    sptr<TestTakeOverSuspendCallback> l2 = new TestTakeOverSuspendCallback();

    suspendTakeoverCallbackHolder.AddCallback(h1, TakeOverSuspendPriority::HIGH);
    suspendTakeoverCallbackHolder.AddCallback(h2, TakeOverSuspendPriority::HIGH);
    suspendTakeoverCallbackHolder.AddCallback(d1, TakeOverSuspendPriority::DEFAULT);
    suspendTakeoverCallbackHolder.AddCallback(d2, TakeOverSuspendPriority::DEFAULT);
    suspendTakeoverCallbackHolder.AddCallback(l1, TakeOverSuspendPriority::LOW);
    suspendTakeoverCallbackHolder.AddCallback(l2, TakeOverSuspendPriority::LOW);

    EXPECT_EQ(suspendTakeoverCallbackHolder.GetHighPriorityCallbacks().size(), 2);
    EXPECT_EQ(suspendTakeoverCallbackHolder.GetDefaultPriorityCallbacks().size(), 2);
    EXPECT_EQ(suspendTakeoverCallbackHolder.GetLowPriorityCallbacks().size(), 2);

    suspendTakeoverCallbackHolder.RemoveCallback(h1);
    suspendTakeoverCallbackHolder.RemoveCallback(h2);
    suspendTakeoverCallbackHolder.RemoveCallback(d1);
    suspendTakeoverCallbackHolder.RemoveCallback(d2);
    suspendTakeoverCallbackHolder.RemoveCallback(l1);
    suspendTakeoverCallbackHolder.RemoveCallback(l2);

    EXPECT_EQ(suspendTakeoverCallbackHolder.GetHighPriorityCallbacks().size(), 0);
    EXPECT_EQ(suspendTakeoverCallbackHolder.GetDefaultPriorityCallbacks().size(), 0);
    EXPECT_EQ(suspendTakeoverCallbackHolder.GetLowPriorityCallbacks().size(), 0);

    POWER_HILOGI(LABEL_TEST, "SuspendTakeoverCallbackHolderTest013 function end!");
}

/**
 * @tc.name: SuspendTakeoverCallbackHolderTest014
 * @tc.desc: Test callback with same object but different priority add sequence
 * @tc.type: FUNC
 */
HWTEST_F(SuspendTakeoverCallbackHolderTest, SuspendTakeoverCallbackHolderTest014, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "SuspendTakeoverCallbackHolderTest014 function start!");
    TakeOverSuspendCallbackHolder& suspendTakeoverCallbackHolder = TakeOverSuspendCallbackHolder::GetInstance();
    sptr<TestTakeOverSuspendCallback> callback = new TestTakeOverSuspendCallback();

    suspendTakeoverCallbackHolder.AddCallback(callback, TakeOverSuspendPriority::LOW);
    suspendTakeoverCallbackHolder.AddCallback(callback, TakeOverSuspendPriority::DEFAULT);
    suspendTakeoverCallbackHolder.AddCallback(callback, TakeOverSuspendPriority::HIGH);

    EXPECT_EQ(suspendTakeoverCallbackHolder.GetHighPriorityCallbacks().size(), 1);
    EXPECT_EQ(suspendTakeoverCallbackHolder.GetDefaultPriorityCallbacks().size(), 1);
    EXPECT_EQ(suspendTakeoverCallbackHolder.GetLowPriorityCallbacks().size(), 1);

    suspendTakeoverCallbackHolder.RemoveCallback(callback);
    EXPECT_EQ(suspendTakeoverCallbackHolder.GetHighPriorityCallbacks().size(), 0);
    EXPECT_EQ(suspendTakeoverCallbackHolder.GetDefaultPriorityCallbacks().size(), 0);
    EXPECT_EQ(suspendTakeoverCallbackHolder.GetLowPriorityCallbacks().size(), 0);

    POWER_HILOGI(LABEL_TEST, "SuspendTakeoverCallbackHolderTest014 function end!");
}

/**
 * @tc.name: SuspendTakeoverCallbackHolderTest015
 * @tc.desc: Test batch add and remove operations
 * @tc.type: FUNC
 */
HWTEST_F(SuspendTakeoverCallbackHolderTest, SuspendTakeoverCallbackHolderTest015, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "SuspendTakeoverCallbackHolderTest015 function start!");
    TakeOverSuspendCallbackHolder& suspendTakeoverCallbackHolder = TakeOverSuspendCallbackHolder::GetInstance();

    std::vector<sptr<TestTakeOverSuspendCallback>> highCbs;
    std::vector<sptr<TestTakeOverSuspendCallback>> defaultCbs;
    std::vector<sptr<TestTakeOverSuspendCallback>> lowCbs;

    for (int i = 0; i < 5; ++i) {
        highCbs.push_back(new TestTakeOverSuspendCallback());
        defaultCbs.push_back(new TestTakeOverSuspendCallback());
        lowCbs.push_back(new TestTakeOverSuspendCallback());
    }

    for (auto& cb : highCbs) {
        suspendTakeoverCallbackHolder.AddCallback(cb, TakeOverSuspendPriority::HIGH);
    }
    for (auto& cb : defaultCbs) {
        suspendTakeoverCallbackHolder.AddCallback(cb, TakeOverSuspendPriority::DEFAULT);
    }
    for (auto& cb : lowCbs) {
        suspendTakeoverCallbackHolder.AddCallback(cb, TakeOverSuspendPriority::LOW);
    }

    EXPECT_EQ(suspendTakeoverCallbackHolder.GetHighPriorityCallbacks().size(), 5);
    EXPECT_EQ(suspendTakeoverCallbackHolder.GetDefaultPriorityCallbacks().size(), 5);
    EXPECT_EQ(suspendTakeoverCallbackHolder.GetLowPriorityCallbacks().size(), 5);

    for (auto& cb : highCbs) {
        suspendTakeoverCallbackHolder.RemoveCallback(cb);
    }
    EXPECT_EQ(suspendTakeoverCallbackHolder.GetHighPriorityCallbacks().size(), 0);
    EXPECT_EQ(suspendTakeoverCallbackHolder.GetDefaultPriorityCallbacks().size(), 5);
    EXPECT_EQ(suspendTakeoverCallbackHolder.GetLowPriorityCallbacks().size(), 5);

    for (auto& cb : defaultCbs) {
        suspendTakeoverCallbackHolder.RemoveCallback(cb);
    }
    EXPECT_EQ(suspendTakeoverCallbackHolder.GetHighPriorityCallbacks().size(), 0);
    EXPECT_EQ(suspendTakeoverCallbackHolder.GetDefaultPriorityCallbacks().size(), 0);
    EXPECT_EQ(suspendTakeoverCallbackHolder.GetLowPriorityCallbacks().size(), 5);

    for (auto& cb : lowCbs) {
        suspendTakeoverCallbackHolder.RemoveCallback(cb);
    }
    EXPECT_EQ(suspendTakeoverCallbackHolder.GetHighPriorityCallbacks().size(), 0);
    EXPECT_EQ(suspendTakeoverCallbackHolder.GetDefaultPriorityCallbacks().size(), 0);
    EXPECT_EQ(suspendTakeoverCallbackHolder.GetLowPriorityCallbacks().size(), 0);

    POWER_HILOGI(LABEL_TEST, "SuspendTakeoverCallbackHolderTest015 function end!");
}

/**
 * @tc.name: SuspendTakeoverCallbackHolderTest016
 * @tc.desc: Test TakeOverSuspendCallbackStub with different device types
 * @tc.type: FUNC
 */
HWTEST_F(SuspendTakeoverCallbackHolderTest, SuspendTakeoverCallbackHolderTest016, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "SuspendTakeoverCallbackHolderTest016 function start!");
    TestTakeOverSuspendCallback callback;
    MessageParcel reply;
    MessageOption opt;

    std::vector<SuspendDeviceType> deviceTypes = {
        SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION,
        SuspendDeviceType::SUSPEND_DEVICE_REASON_DEVICE_ADMIN,
        SuspendDeviceType::SUSPEND_DEVICE_REASON_TIMEOUT,
        SuspendDeviceType::SUSPEND_DEVICE_REASON_LID,
        SuspendDeviceType::SUSPEND_DEVICE_REASON_POWER_KEY,
        SuspendDeviceType::SUSPEND_DEVICE_REASON_HDMI,
        SuspendDeviceType::SUSPEND_DEVICE_REASON_SLEEP_KEY, 7,
        SuspendDeviceType::SUSPEND_DEVICE_REASON_FORCE_SUSPEND,
        SuspendDeviceType::SUSPEND_DEVICE_REASON_STR,
        SuspendDeviceType::SUSPEND_DEVICE_REASON_SWITCH,
        SuspendDeviceType::SUSPEND_DEVICE_LOW_CAPACITY,
        SuspendDeviceType::SUSPEND_DEVICE_REASON_TP_COVER,
        SuspendDeviceType::SUSPEND_DEVICE_REASON_EX_SCREEN_INIT,
        SuspendDeviceType::SUSPEND_DEVICE_SWITCH_SENSORHUB,
    };

    for (auto type : deviceTypes) {
        g_isTakeOverSuspend = false;
        MessageParcel data;
        data.WriteInterfaceToken(TestTakeOverSuspendCallback::GetDescriptor());
        data.WriteUint32(static_cast<uint32_t>(type));
        int32_t ret = callback.OnRemoteRequest(
            static_cast<uint32_t>(PowerMgr::TakeOverSuspendCallbackInterfaceCode::CMD_ON_TAKEOVER_SUSPEND),
            data, reply, opt);
        EXPECT_EQ(ret, ERR_OK);
        EXPECT_TRUE(g_isTakeOverSuspend);
    }

    POWER_HILOGI(LABEL_TEST, "SuspendTakeoverCallbackHolderTest016 function end!");
}

/**
 * @tc.name: SuspendTakeoverCallbackHolderTest017
 * @tc.desc: Test TakeOverSuspendCallbackStub with invalid device type
 * @tc.type: FUNC
 */
HWTEST_F(SuspendTakeoverCallbackHolderTest, SuspendTakeoverCallbackHolderTest017, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "SuspendTakeoverCallbackHolderTest017 function start!");
    TestTakeOverSuspendCallback callback;
    MessageParcel reply;
    MessageOption opt;

    MessageParcel data;
    data.WriteInterfaceToken(TestTakeOverSuspendCallback::GetDescriptor());
    data.WriteUint32(999);
    int32_t ret = callback.OnRemoteRequest(
        static_cast<uint32_t>(PowerMgr::TakeOverSuspendCallbackInterfaceCode::CMD_ON_TAKEOVER_SUSPEND),
        data, reply, opt);
    EXPECT_EQ(ret, E_INNER_ERR);

    POWER_HILOGI(LABEL_TEST, "SuspendTakeoverCallbackHolderTest017 function end!");
}
} // namespace PowerMgr
} // namespace OHOS