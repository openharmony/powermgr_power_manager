/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "ulsr_callback_holder_test.h"

#include <memory>
#include "gtest/gtest.h"
#include "power_common.h"
#include "power_log.h"
#include "ulsr/ulsr_callback_holder.h"
#include "ulsr/ulsr_callback_ipc_interface_code.h"
#include "ulsr_callback_stub.h"
#include "ulsr_callback_proxy.h"
#include "message_option.h"
#include "message_parcel.h"
#include "mock_power_remote_object.h"

namespace OHOS {
namespace PowerMgr {

using namespace testing::ext;

void UlsrCallbackHolderTest::SetUpTestCase() {}
void UlsrCallbackHolderTest::TearDownTestCase() {}
void UlsrCallbackHolderTest::SetUp() {}
void UlsrCallbackHolderTest::TearDown() {}

// Mock callback implementing IUlsrCallback interface
class TestUlsrCallback : public UlsrCallbackStub {
public:
    TestUlsrCallback() = default;
    virtual ~TestUlsrCallback() = default;

    void OnSyncUlsr() override
    {
        isSyncCalled_ = true;
        POWER_HILOGI(LABEL_TEST, "TestUlsrCallback OnSyncUlsr!");
    }

    void OnAsyncWakeup(bool ulsrResult) override
    {
        isAsyncCalled_ = true;
        lastUlsrResult_ = ulsrResult;
        POWER_HILOGI(LABEL_TEST, "TestUlsrCallback OnAsyncWakeup!");
    }

    bool IsSyncCalled() const { return isSyncCalled_; }
    bool IsAsyncCalled() const { return isAsyncCalled_; }
    bool GetLastUlsrResult() const { return lastUlsrResult_; }
    void ResetCallFlags()
    {
        isSyncCalled_ = false;
        isAsyncCalled_ = false;
        lastUlsrResult_ = false;
    }

private:
    bool isSyncCalled_ = false;
    bool isAsyncCalled_ = false;
    bool lastUlsrResult_ = false;
};

// Test callback with null AsObject
class TestUlsrCallbackNullObject : public IUlsrCallback {
public:
    TestUlsrCallbackNullObject() = default;
    virtual ~TestUlsrCallbackNullObject() = default;

    void OnSyncUlsr() override {}
    void OnAsyncWakeup(bool ulsrResult) override {}
    sptr<IRemoteObject> AsObject() override { return nullptr; }
};

// Test UlsrCallbackHolder with accessible internal state for coverage testing
class UlsrCallbackHolderTestWrapper : public UlsrCallbackHolder {
public:
    // Helper method to count callback occurrences in a specific priority queue
    // Uses iteration to avoid hash calculation for nullptr callback
    int32_t CountInPriority(const sptr<IUlsrCallback>& callback, UlsrPriority priority)
    {
        std::lock_guard<std::mutex> lock(callbacksMutex_);
        auto& container = (priority == UlsrPriority::HIGH) ? highPriorityCallbacks_ :
                         (priority == UlsrPriority::DEFAULT) ? defaultPriorityCallbacks_ :
                         lowPriorityCallbacks_;
        int32_t count = 0;
        for (const auto& [key, value] : container) {
            if (callback == nullptr) {
                // Detect nullptr callback record
                if (key == nullptr) {
                    count++;
                }
            } else {
                // Detect normal callback
                if (key == callback) {
                    count++;
                }
            }
        }
        return count;
    }

    // Insert null callback record for testing - uses insert to avoid immediate hash calculation
    void InsertNullCallbackRecord(const std::pair<int32_t, int32_t>& pidUid, UlsrPriority priority)
    {
        std::lock_guard<std::mutex> lock(callbacksMutex_);
        UlsrCallbackRecord record = {nullptr, static_cast<int32_t>(priority), pidUid.first, pidUid.second, -1};
        switch (priority) {
            case UlsrPriority::HIGH:
                highPriorityCallbacks_.insert({nullptr, record});
                break;
            case UlsrPriority::DEFAULT:
                defaultPriorityCallbacks_.insert({nullptr, record});
                break;
            case UlsrPriority::LOW:
                lowPriorityCallbacks_.insert({nullptr, record});
                break;
            default:
                break;
        }
    }
};

// RAII guard to save and restore MockPowerRemoteObject static state for test isolation
class MockRequestValueGuard {
public:
    explicit MockRequestValueGuard(int32_t newValue)
    {
        savedValue_ = MockPowerRemoteObject::GetRequestValue();
        MockPowerRemoteObject::SetRequestValue(newValue);
    }
    ~MockRequestValueGuard()
    {
        MockPowerRemoteObject::SetRequestValue(savedValue_);
    }
    // Delete copy constructor and assignment operator
    MockRequestValueGuard(const MockRequestValueGuard&) = delete;
    MockRequestValueGuard& operator=(const MockRequestValueGuard&) = delete;

private:
    int32_t savedValue_;
};

// ============================================================================
// Test AddCallback
// ============================================================================

/**
 * @tc.name: UlsrCallbackHolderTest001
 * @tc.desc: Test AddCallback with nullptr and null AsObject callbacks - should be rejected
 * @tc.type: FUNC
 */
HWTEST_F(UlsrCallbackHolderTest, UlsrCallbackHolderTest001, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest001 start!");
    sptr<UlsrCallbackHolder> holder = new UlsrCallbackHolder();

    // All nullptr callbacks should be rejected
    holder->AddCallback(nullptr, {6, 6});
    holder->AddCallback(nullptr, {6, 6}, UlsrPriority::HIGH);
    holder->AddCallback(nullptr, {6, 6}, UlsrPriority::LOW);
    EXPECT_EQ(holder->highPriorityCallbacks_.size(), 0);  // All rejected
    EXPECT_EQ(holder->defaultPriorityCallbacks_.size(), 0);
    EXPECT_EQ(holder->lowPriorityCallbacks_.size(), 0);

    // Null AsObject callback should also be rejected
    sptr<TestUlsrCallbackNullObject> nullObjCallback = new TestUlsrCallbackNullObject();
    holder->AddCallback(nullObjCallback, {6, 6});
    holder->AddCallback(nullObjCallback, {6, 6}, UlsrPriority::DEFAULT);
    EXPECT_EQ(holder->defaultPriorityCallbacks_.size(), 0);  // Null AsObject rejected

    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest001 end!");
}

/**
 * @tc.name: UlsrCallbackHolderTest002
 * @tc.desc: Test AddCallback with different priorities - HIGH, DEFAULT, LOW
 * @tc.type: FUNC
 */
HWTEST_F(UlsrCallbackHolderTest, UlsrCallbackHolderTest002, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest002 start!");
    sptr<UlsrCallbackHolder> holder = new UlsrCallbackHolder();

    sptr<TestUlsrCallback> cbHigh = new TestUlsrCallback();
    sptr<TestUlsrCallback> cbDefault = new TestUlsrCallback();
    sptr<TestUlsrCallback> cbLow = new TestUlsrCallback();

    // Add callbacks to different priority queues
    holder->AddCallback(cbHigh, {1, 1}, UlsrPriority::HIGH);
    holder->AddCallback(cbDefault, {2, 2}, UlsrPriority::DEFAULT);
    holder->AddCallback(cbLow, {3, 3}, UlsrPriority::LOW);
    EXPECT_EQ(holder->highPriorityCallbacks_.size(), 1);
    EXPECT_EQ(holder->defaultPriorityCallbacks_.size(), 1);
    EXPECT_EQ(holder->lowPriorityCallbacks_.size(), 1);

    // Same callback added to HIGH again should be rejected (duplicate)
    holder->AddCallback(cbHigh, {4, 4}, UlsrPriority::HIGH);
    EXPECT_EQ(holder->highPriorityCallbacks_.size(), 1);

    // Same callback added to DEFAULT again should be rejected (duplicate)
    holder->AddCallback(cbDefault, {5, 5}, UlsrPriority::DEFAULT);
    EXPECT_EQ(holder->defaultPriorityCallbacks_.size(), 1);

    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest002 end!");
}

// ============================================================================
// Test RemoveCallback
// ============================================================================

/**
 * @tc.name: UlsrCallbackHolderTest003
 * @tc.desc: Test RemoveCallback with nullptr and non-existent callback - should not crash
 * @tc.type: FUNC
 */
HWTEST_F(UlsrCallbackHolderTest, UlsrCallbackHolderTest003, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest003 start!");
    sptr<UlsrCallbackHolder> holder = new UlsrCallbackHolder();

    // Remove nullptr and non-existent callback should not crash
    holder->RemoveCallback(nullptr);  // Should return early
    sptr<TestUlsrCallback> cb = new TestUlsrCallback();
    holder->RemoveCallback(cb);  // Should return early (not in any queue)
    EXPECT_EQ(holder->highPriorityCallbacks_.size(), 0);
    EXPECT_EQ(holder->defaultPriorityCallbacks_.size(), 0);
    EXPECT_EQ(holder->lowPriorityCallbacks_.size(), 0);

    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest003 end!");
}

/**
 * @tc.name: UlsrCallbackHolderTest004
 * @tc.desc: Test RemoveCallback after AddCallback - callbacks should be removed
 * @tc.type: FUNC
 */
HWTEST_F(UlsrCallbackHolderTest, UlsrCallbackHolderTest004, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest004 start!");
    sptr<UlsrCallbackHolder> holder = new UlsrCallbackHolder();

    sptr<TestUlsrCallback> cb1 = new TestUlsrCallback();
    sptr<TestUlsrCallback> cb2 = new TestUlsrCallback();
    sptr<TestUlsrCallback> cb3 = new TestUlsrCallback();

    // Add callbacks to different priority queues
    holder->AddCallback(cb1, {1, 1}, UlsrPriority::HIGH);
    holder->AddCallback(cb2, {2, 2}, UlsrPriority::DEFAULT);
    holder->AddCallback(cb3, {3, 3}, UlsrPriority::LOW);
    EXPECT_EQ(holder->highPriorityCallbacks_.size(), 1);
    EXPECT_EQ(holder->defaultPriorityCallbacks_.size(), 1);
    EXPECT_EQ(holder->lowPriorityCallbacks_.size(), 1);

    // Remove all callbacks - all queues should be empty
    holder->RemoveCallback(cb1);
    holder->RemoveCallback(cb2);
    holder->RemoveCallback(cb3);
    EXPECT_EQ(holder->highPriorityCallbacks_.size(), 0);
    EXPECT_EQ(holder->defaultPriorityCallbacks_.size(), 0);
    EXPECT_EQ(holder->lowPriorityCallbacks_.size(), 0);

    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest004 end!");
}

// ============================================================================
// Test OnRemoteDied
// ============================================================================

/**
 * @tc.name: UlsrCallbackHolderTest005
 * @tc.desc: Test OnRemoteDied with nullptr and null object - should not crash
 * @tc.type: FUNC
 */
HWTEST_F(UlsrCallbackHolderTest, UlsrCallbackHolderTest005, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest005 start!");
    sptr<UlsrCallbackHolder> holder = new UlsrCallbackHolder();
    EXPECT_TRUE(holder != nullptr);

    // OnRemoteDied with nullptr and null object should return early without crash
    holder->OnRemoteDied(nullptr);
    sptr<IRemoteObject> obj = nullptr;
    holder->OnRemoteDied(obj);
    // No assertions needed - this test verifies no crash occurs

    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest005 end!");
}

/**
 * @tc.name: UlsrCallbackHolderTest006
 * @tc.desc: Test OnRemoteDied removes registered callback
 * @tc.type: FUNC
 */
HWTEST_F(UlsrCallbackHolderTest, UlsrCallbackHolderTest006, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest006 start!");
    sptr<UlsrCallbackHolder> holder = new UlsrCallbackHolder();
    EXPECT_TRUE(holder != nullptr);

    sptr<TestUlsrCallback> cb1 = new TestUlsrCallback();
    sptr<TestUlsrCallback> cb2 = new TestUlsrCallback();
    EXPECT_TRUE(cb1 != nullptr);
    EXPECT_TRUE(cb2 != nullptr);

    holder->AddCallback(cb1, {1, 1}, UlsrPriority::HIGH);
    holder->AddCallback(cb2, {2, 2}, UlsrPriority::DEFAULT);
    holder->OnRemoteDied(cb1->AsObject());

    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest006 end!");
}

// ============================================================================
// Test SyncUlsrNotify
// ============================================================================

/**
 * @tc.name: UlsrCallbackHolderTest007
 * @tc.desc: Test SyncUlsrNotify with empty callback list
 * @tc.type: FUNC
 */
HWTEST_F(UlsrCallbackHolderTest, UlsrCallbackHolderTest007, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest007 start!");
    sptr<UlsrCallbackHolder> holder = new UlsrCallbackHolder();
    EXPECT_TRUE(holder != nullptr);
    // No crash with empty callback list
    holder->SyncUlsrNotify();
    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest007 end!");
}

/**
 * @tc.name: UlsrCallbackHolderTest008
 * @tc.desc: Test SyncUlsrNotify with callbacks in different priority queues
 * @tc.type: FUNC
 */
HWTEST_F(UlsrCallbackHolderTest, UlsrCallbackHolderTest008, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest008 start!");
    sptr<UlsrCallbackHolder> holder = new UlsrCallbackHolder();
    EXPECT_TRUE(holder != nullptr);

    sptr<TestUlsrCallback> cbHigh = new TestUlsrCallback();
    sptr<TestUlsrCallback> cbDefault = new TestUlsrCallback();
    sptr<TestUlsrCallback> cbLow = new TestUlsrCallback();
    EXPECT_TRUE(cbHigh != nullptr);
    EXPECT_TRUE(cbDefault != nullptr);
    EXPECT_TRUE(cbLow != nullptr);

    holder->AddCallback(cbHigh, {1, 1}, UlsrPriority::HIGH);
    holder->AddCallback(cbDefault, {2, 2}, UlsrPriority::DEFAULT);
    holder->AddCallback(cbLow, {3, 3}, UlsrPriority::LOW);
    holder->SyncUlsrNotify();

    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest008 end!");
}

/**
 * @tc.name: UlsrCallbackHolderTest009
 * @tc.desc: Test SyncUlsrNotifyInner with multiple callbacks
 * @tc.type: FUNC
 */
HWTEST_F(UlsrCallbackHolderTest, UlsrCallbackHolderTest009, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest009 start!");
    sptr<UlsrCallbackHolder> holder = new UlsrCallbackHolder();
    EXPECT_TRUE(holder != nullptr);

    sptr<TestUlsrCallback> cb1 = new TestUlsrCallback();
    sptr<TestUlsrCallback> cb2 = new TestUlsrCallback();
    sptr<TestUlsrCallback> cb3 = new TestUlsrCallback();
    sptr<TestUlsrCallback> cb4 = new TestUlsrCallback();
    EXPECT_TRUE(cb1 != nullptr);
    EXPECT_TRUE(cb2 != nullptr);
    EXPECT_TRUE(cb3 != nullptr);
    EXPECT_TRUE(cb4 != nullptr);

    holder->AddCallback(cb1, {1, 1}, UlsrPriority::HIGH);
    holder->AddCallback(cb2, {2, 2}, UlsrPriority::HIGH);
    holder->AddCallback(cb3, {3, 3}, UlsrPriority::DEFAULT);
    holder->AddCallback(cb4, {4, 4}, UlsrPriority::LOW);
    holder->SyncUlsrNotify();

    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest009 end!");
}

// ============================================================================
// Test WakeupNotify
// ============================================================================

/**
 * @tc.name: UlsrCallbackHolderTest010
 * @tc.desc: Test WakeupNotify with empty callback list
 * @tc.type: FUNC
 */
HWTEST_F(UlsrCallbackHolderTest, UlsrCallbackHolderTest010, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest010 start!");
    sptr<UlsrCallbackHolder> holder = new UlsrCallbackHolder();
    EXPECT_TRUE(holder != nullptr);
    // No crash with empty callback list
    holder->WakeupNotify(true);
    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest010 end!");
}

/**
 * @tc.name: UlsrCallbackHolderTest011
 * @tc.desc: Test WakeupNotify with callbacks in different priority queues
 * @tc.type: FUNC
 */
HWTEST_F(UlsrCallbackHolderTest, UlsrCallbackHolderTest011, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest011 start!");
    sptr<UlsrCallbackHolder> holder = new UlsrCallbackHolder();
    EXPECT_TRUE(holder != nullptr);

    sptr<TestUlsrCallback> cbHigh = new TestUlsrCallback();
    sptr<TestUlsrCallback> cbDefault = new TestUlsrCallback();
    sptr<TestUlsrCallback> cbLow = new TestUlsrCallback();
    EXPECT_TRUE(cbHigh != nullptr);
    EXPECT_TRUE(cbDefault != nullptr);
    EXPECT_TRUE(cbLow != nullptr);

    holder->AddCallback(cbHigh, {1, 1}, UlsrPriority::HIGH);
    holder->AddCallback(cbDefault, {2, 2}, UlsrPriority::DEFAULT);
    holder->AddCallback(cbLow, {3, 3}, UlsrPriority::LOW);
    holder->WakeupNotify(true);

    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest011 end!");
}

// ============================================================================
// Test UlsrCallbackStub - OnRemoteRequest
// ============================================================================

/**
 * @tc.name: UlsrCallbackHolderTest012
 * @tc.desc: Test UlsrCallbackStub::OnRemoteRequest - descriptor mismatch
 * @tc.type: FUNC
 */
HWTEST_F(UlsrCallbackHolderTest, UlsrCallbackHolderTest012, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest012 start!");

    TestUlsrCallback stub;
    MessageParcel reply;
    MessageOption opt;

    MessageParcel dataMismatch;
    dataMismatch.WriteInterfaceToken(u"test.interface.token");
    int32_t ret = stub.OnRemoteRequest(0, dataMismatch, reply, opt);
    EXPECT_EQ(ret, E_GET_POWER_SERVICE_FAILED);

    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest012 end!");
}

/**
 * @tc.name: UlsrCallbackHolderTest013
 * @tc.desc: Test UlsrCallbackStub::OnRemoteRequest - CMD_ON_SYNC_ULSR
 * @tc.type: FUNC
 */
HWTEST_F(UlsrCallbackHolderTest, UlsrCallbackHolderTest013, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest013 start!");

    TestUlsrCallback stub;
    MessageParcel reply;
    MessageOption opt;

    MessageParcel data;
    data.WriteInterfaceToken(TestUlsrCallback::GetDescriptor());
    int32_t ret = stub.OnRemoteRequest(
        static_cast<uint32_t>(UlsrCallbackInterfaceCode::CMD_ON_SYNC_ULSR),
        data, reply, opt);
    EXPECT_EQ(ret, ERR_OK);

    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest013 end!");
}

/**
 * @tc.name: UlsrCallbackHolderTest014
 * @tc.desc: Test UlsrCallbackStub::OnRemoteRequest - CMD_ON_ASYNC_WAKEUP
 * @tc.type: FUNC
 */
HWTEST_F(UlsrCallbackHolderTest, UlsrCallbackHolderTest014, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest014 start!");

    TestUlsrCallback stub;
    MessageParcel reply;
    MessageOption opt;

    MessageParcel data;
    data.WriteInterfaceToken(TestUlsrCallback::GetDescriptor());
    data.WriteBool(true);  // ulsrResult
    int32_t ret = stub.OnRemoteRequest(
        static_cast<uint32_t>(UlsrCallbackInterfaceCode::CMD_ON_ASYNC_WAKEUP),
        data, reply, opt);
    EXPECT_EQ(ret, ERR_OK);

    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest014 end!");
}

/**
 * @tc.name: UlsrCallbackHolderTest015
 * @tc.desc: Test UlsrCallbackStub::OnRemoteRequest - unknown command
 * @tc.type: FUNC
 */
HWTEST_F(UlsrCallbackHolderTest, UlsrCallbackHolderTest015, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest015 start!");

    TestUlsrCallback stub;
    MessageParcel reply;
    MessageOption opt;

    MessageParcel data;
    data.WriteInterfaceToken(TestUlsrCallback::GetDescriptor());
    int32_t ret = stub.OnRemoteRequest(999, data, reply, opt);
    EXPECT_NE(ret, ERR_OK);

    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest015 end!");
}

/**
 * @tc.name: UlsrCallbackHolderTest016
 * @tc.desc: Test OnSyncUlsrStub and OnAsyncWakeupStub methods
 * @tc.type: FUNC
 */
HWTEST_F(UlsrCallbackHolderTest, UlsrCallbackHolderTest016, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest016 start!");

    TestUlsrCallback stub;

    MessageParcel data1;
    int32_t ret1 = stub.OnSyncUlsrStub(data1);
    EXPECT_EQ(ret1, ERR_OK);

    MessageParcel data2;
    data2.WriteBool(true);  // ulsrResult
    int32_t ret2 = stub.OnAsyncWakeupStub(data2);
    EXPECT_EQ(ret2, ERR_OK);

    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest016 end!");
}

// ============================================================================
// Test UlsrCallbackProxy
// ============================================================================

/**
 * @tc.name: UlsrCallbackHolderTest017
 * @tc.desc: Test UlsrCallbackProxy::OnSyncUlsr with nullptr remote
 * @tc.type: FUNC
 */
HWTEST_F(UlsrCallbackHolderTest, UlsrCallbackHolderTest017, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest017 start!");

    sptr<IRemoteObject> nullRemote = nullptr;
    UlsrCallbackProxy proxy(nullRemote);
    // No crash with nullptr remote, verify proxy is created
    EXPECT_TRUE(proxy.Remote() == nullptr);
    proxy.OnSyncUlsr();

    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest017 end!");
}

/**
 * @tc.name: UlsrCallbackHolderTest018
 * @tc.desc: Test UlsrCallbackProxy::OnAsyncWakeup with nullptr remote
 * @tc.type: FUNC
 */
HWTEST_F(UlsrCallbackHolderTest, UlsrCallbackHolderTest018, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest018 start!");

    sptr<IRemoteObject> nullRemote = nullptr;
    UlsrCallbackProxy proxy(nullRemote);
    // No crash with nullptr remote, verify proxy is created
    EXPECT_TRUE(proxy.Remote() == nullptr);
    proxy.OnAsyncWakeup();

    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest018 end!");
}

/**
 * @tc.name: UlsrCallbackHolderTest019
 * @tc.desc: Test UlsrCallbackProxy with nullptr remote
 * @tc.type: FUNC
 */
HWTEST_F(UlsrCallbackHolderTest, UlsrCallbackHolderTest019, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest019 start!");

    sptr<IRemoteObject> nullRemote = nullptr;
    UlsrCallbackProxy proxy1(nullRemote);
    UlsrCallbackProxy proxy2(nullRemote);

    EXPECT_TRUE(proxy1.Remote() == nullptr);
    EXPECT_TRUE(proxy2.Remote() == nullptr);

    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest019 end!");
}

/**
 * @tc.name: UlsrCallbackHolderTest020
 * @tc.desc: Test UlsrCallbackProxy::OnSyncUlsr with valid remote and various ret values
 * @tc.type: FUNC
 */
HWTEST_F(UlsrCallbackHolderTest, UlsrCallbackHolderTest020, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest020 start!");

    sptr<MockPowerRemoteObject> mockRemote = new MockPowerRemoteObject();
    EXPECT_TRUE(mockRemote != nullptr);

    UlsrCallbackProxy proxy(mockRemote);
    EXPECT_TRUE(proxy.Remote() != nullptr);

    // Case 1: SendRequest returns ERR_OK (ret == ERR_OK branch)
    MockPowerRemoteObject::SetRequestValue(ERR_OK);
    proxy.OnSyncUlsr();

    // Case 2: SendRequest returns error (ret != ERR_OK branch)
    MockPowerRemoteObject::SetRequestValue(ERR_INVALID_VALUE);
    proxy.OnSyncUlsr();

    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest020 end!");
}

/**
 * @tc.name: UlsrCallbackHolderTest021
 * @tc.desc: Test UlsrCallbackProxy::OnAsyncWakeup with valid remote and various ret values
 * @tc.type: FUNC
 */
HWTEST_F(UlsrCallbackHolderTest, UlsrCallbackHolderTest021, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest021 start!");

    sptr<MockPowerRemoteObject> mockRemote = new MockPowerRemoteObject();
    EXPECT_TRUE(mockRemote != nullptr);

    UlsrCallbackProxy proxy(mockRemote);
    EXPECT_TRUE(proxy.Remote() != nullptr);

    // Case 1: SendRequest returns ERR_OK (ret == ERR_OK branch)
    MockPowerRemoteObject::SetRequestValue(ERR_OK);
    proxy.OnAsyncWakeup();

    // Case 2: SendRequest returns error (ret != ERR_OK branch)
    MockPowerRemoteObject::SetRequestValue(ERR_INVALID_VALUE);
    proxy.OnAsyncWakeup();

    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest021 end!");
}

// ============================================================================
// Test Priority-Based Callback Management
// ============================================================================

/**
 * @tc.name: UlsrCallbackHolderTest022
 * @tc.desc: Test same callback cannot be added to multiple priority queues - callback should only be in first queue
 * @tc.type: FUNC
 */
HWTEST_F(UlsrCallbackHolderTest, UlsrCallbackHolderTest022, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest022 start!");
    // Use sptr holder for AddCallback, wrapper for verification (same object, different type)
    sptr<UlsrCallbackHolder> holder = new UlsrCallbackHolder();
    EXPECT_TRUE(holder != nullptr);
    UlsrCallbackHolderTestWrapper* wrapper = static_cast<UlsrCallbackHolderTestWrapper*>(holder.GetRefPtr());

    sptr<TestUlsrCallback> cb = new TestUlsrCallback();
    EXPECT_TRUE(cb != nullptr);

    // Add same callback to HIGH first (use sptr holder)
    holder->AddCallback(cb, {1, 1}, UlsrPriority::HIGH);
    // Try to add to DEFAULT (should fail - callback already exists in HIGH)
    holder->AddCallback(cb, {2, 2}, UlsrPriority::DEFAULT);
    // Try to add to LOW (should fail - callback already exists in HIGH)
    holder->AddCallback(cb, {3, 3}, UlsrPriority::LOW);

    // Verify: callback exists only in HIGH queue (count = 1)
    EXPECT_EQ(wrapper->CountInPriority(cb, UlsrPriority::HIGH), 1);
    EXPECT_EQ(wrapper->CountInPriority(cb, UlsrPriority::DEFAULT), 0);
    EXPECT_EQ(wrapper->CountInPriority(cb, UlsrPriority::LOW), 0);

    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest022 end!");
}

/**
 * @tc.name: UlsrCallbackHolderTest023
 * @tc.desc: Test same callback cannot be added twice to same priority - callback should only exist once
 * @tc.type: FUNC
 */
HWTEST_F(UlsrCallbackHolderTest, UlsrCallbackHolderTest023, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest023 start!");
    // Use sptr holder for AddCallback, wrapper for verification (same object, different type)
    sptr<UlsrCallbackHolder> holder = new UlsrCallbackHolder();
    EXPECT_TRUE(holder != nullptr);
    UlsrCallbackHolderTestWrapper* wrapper = static_cast<UlsrCallbackHolderTestWrapper*>(holder.GetRefPtr());

    sptr<TestUlsrCallback> cb1 = new TestUlsrCallback();
    sptr<TestUlsrCallback> cb2 = new TestUlsrCallback();
    EXPECT_TRUE(cb1 != nullptr);
    EXPECT_TRUE(cb2 != nullptr);

    // cb1 added to HIGH twice (second add should be rejected)
    holder->AddCallback(cb1, {1, 1}, UlsrPriority::HIGH);
    holder->AddCallback(cb1, {2, 2}, UlsrPriority::HIGH);
    // cb2 added to DEFAULT, then try to add to LOW (second add should be rejected)
    holder->AddCallback(cb2, {3, 3}, UlsrPriority::DEFAULT);
    holder->AddCallback(cb2, {4, 4}, UlsrPriority::LOW);

    // Verify cb1: only exists in HIGH queue (added twice but only first succeeded)
    EXPECT_EQ(wrapper->CountInPriority(cb1, UlsrPriority::HIGH), 1);
    // Verify cb2: exists only in DEFAULT queue (second add to LOW was rejected)
    EXPECT_EQ(wrapper->CountInPriority(cb2, UlsrPriority::DEFAULT), 1);
    EXPECT_EQ(wrapper->CountInPriority(cb2, UlsrPriority::LOW), 0);

    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest023 end!");
}

/**
 * @tc.name: UlsrCallbackHolderTest024
 * @tc.desc: Test AddCallback - default priority value
 * @tc.type: FUNC
 */
HWTEST_F(UlsrCallbackHolderTest, UlsrCallbackHolderTest024, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest024 start!");
    sptr<UlsrCallbackHolder> holder = new UlsrCallbackHolder();
    EXPECT_TRUE(holder != nullptr);

    sptr<TestUlsrCallback> cb1 = new TestUlsrCallback();
    sptr<TestUlsrCallback> cb2 = new TestUlsrCallback();
    EXPECT_TRUE(cb1 != nullptr);
    EXPECT_TRUE(cb2 != nullptr);

    holder->AddCallback(cb1, {1, 1}, UlsrPriority::DEFAULT);
    holder->AddCallback(cb2, {2, 2});
    // No crash when priority is default or omitted

    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest024 end!");
}

// ============================================================================
// Integration Test
// ============================================================================

/**
 * @tc.name: UlsrCallbackHolderTest025
 * @tc.desc: Integration test - register, sync, wakeup, unregister workflow
 * @tc.type: FUNC
 */
HWTEST_F(UlsrCallbackHolderTest, UlsrCallbackHolderTest025, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest025 start!");
    sptr<UlsrCallbackHolder> holder = new UlsrCallbackHolder();
    EXPECT_TRUE(holder != nullptr);

    sptr<TestUlsrCallback> cb1 = new TestUlsrCallback();
    sptr<TestUlsrCallback> cb2 = new TestUlsrCallback();
    sptr<TestUlsrCallback> cb3 = new TestUlsrCallback();
    EXPECT_TRUE(cb1 != nullptr);
    EXPECT_TRUE(cb2 != nullptr);
    EXPECT_TRUE(cb3 != nullptr);

    holder->AddCallback(cb1, {1, 1}, UlsrPriority::HIGH);
    holder->AddCallback(cb2, {2, 2}, UlsrPriority::DEFAULT);
    holder->AddCallback(cb3, {3, 3}, UlsrPriority::LOW);

    holder->SyncUlsrNotify();
    holder->WakeupNotify(false);
    holder->RemoveCallback(cb1);
    holder->RemoveCallback(cb2);
    holder->RemoveCallback(cb3);

    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest025 end!");
}

/**
 * @tc.name: UlsrCallbackHolderTest026
 * @tc.desc: Test AddCallback with invalid priority enum value - should not be added to any queue
 * @tc.type: FUNC
 */
HWTEST_F(UlsrCallbackHolderTest, UlsrCallbackHolderTest026, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest026 start!");
    // Use sptr holder for AddCallback, wrapper for verification (same object, different type)
    sptr<UlsrCallbackHolder> holder = new UlsrCallbackHolder();
    EXPECT_TRUE(holder != nullptr);
    UlsrCallbackHolderTestWrapper* wrapper = static_cast<UlsrCallbackHolderTestWrapper*>(holder.GetRefPtr());

    sptr<TestUlsrCallback> cb = new TestUlsrCallback();
    EXPECT_TRUE(cb != nullptr);

    UlsrPriority invalidPriority = static_cast<UlsrPriority>(999);
    // Invalid priority - callback should NOT be added (goes to default case)
    holder->AddCallback(cb, {1, 1}, invalidPriority);

    // Verify: callback was not added to any queue due to invalid priority
    EXPECT_EQ(wrapper->CountInPriority(cb, UlsrPriority::HIGH), 0);
    EXPECT_EQ(wrapper->CountInPriority(cb, UlsrPriority::DEFAULT), 0);
    EXPECT_EQ(wrapper->CountInPriority(cb, UlsrPriority::LOW), 0);

    // Now add with valid priority - should succeed
    holder->AddCallback(cb, {1, 1}, UlsrPriority::HIGH);
    EXPECT_EQ(wrapper->CountInPriority(cb, UlsrPriority::HIGH), 1);

    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest026 end!");
}

/**
 * @tc.name: UlsrCallbackHolderTest027
 * @tc.desc: Test AddCallback - duplicate callback detection in each priority queue
 * @tc.type: FUNC
 */
HWTEST_F(UlsrCallbackHolderTest, UlsrCallbackHolderTest027, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest027 start!");
    // Use sptr holder for AddCallback, wrapper for verification (same object, different type)
    sptr<UlsrCallbackHolder> holder = new UlsrCallbackHolder();
    EXPECT_TRUE(holder != nullptr);
    UlsrCallbackHolderTestWrapper* wrapper = static_cast<UlsrCallbackHolderTestWrapper*>(holder.GetRefPtr());

    sptr<TestUlsrCallback> cb1 = new TestUlsrCallback();
    sptr<TestUlsrCallback> cb2 = new TestUlsrCallback();
    sptr<TestUlsrCallback> cb3 = new TestUlsrCallback();
    sptr<TestUlsrCallback> cb4 = new TestUlsrCallback();
    EXPECT_TRUE(cb1 != nullptr);
    EXPECT_TRUE(cb2 != nullptr);
    EXPECT_TRUE(cb3 != nullptr);
    EXPECT_TRUE(cb4 != nullptr);

    // Case 1: Add same callback to HIGH first, then try to add again to DEFAULT
    // (high condition true -> second add should be rejected)
    holder->AddCallback(cb1, {1, 1}, UlsrPriority::HIGH);
    holder->AddCallback(cb1, {2, 2}, UlsrPriority::DEFAULT);
    EXPECT_EQ(wrapper->CountInPriority(cb1, UlsrPriority::HIGH), 1);
    EXPECT_EQ(wrapper->CountInPriority(cb1, UlsrPriority::DEFAULT), 0);
    EXPECT_EQ(wrapper->CountInPriority(cb1, UlsrPriority::LOW), 0);

    // Case 2: Add same callback to DEFAULT first, then try to add again to LOW
    // (default condition true -> second add should be rejected)
    holder->AddCallback(cb2, {3, 3}, UlsrPriority::DEFAULT);
    holder->AddCallback(cb2, {4, 4}, UlsrPriority::LOW);
    EXPECT_EQ(wrapper->CountInPriority(cb2, UlsrPriority::HIGH), 0);
    EXPECT_EQ(wrapper->CountInPriority(cb2, UlsrPriority::DEFAULT), 1);
    EXPECT_EQ(wrapper->CountInPriority(cb2, UlsrPriority::LOW), 0);

    // Case 3: Add same callback to LOW first, then try to add again to HIGH
    // (low condition true -> second add should be rejected)
    holder->AddCallback(cb3, {5, 5}, UlsrPriority::LOW);
    holder->AddCallback(cb3, {6, 6}, UlsrPriority::HIGH);
    EXPECT_EQ(wrapper->CountInPriority(cb3, UlsrPriority::HIGH), 0);
    EXPECT_EQ(wrapper->CountInPriority(cb3, UlsrPriority::DEFAULT), 0);
    EXPECT_EQ(wrapper->CountInPriority(cb3, UlsrPriority::LOW), 1);

    // Case 4: Normal add (no duplicates) - all three callbacks should exist
    holder->AddCallback(cb4, {7, 7}, UlsrPriority::HIGH);
    EXPECT_EQ(wrapper->CountInPriority(cb1, UlsrPriority::HIGH), 1);
    EXPECT_EQ(wrapper->CountInPriority(cb2, UlsrPriority::DEFAULT), 1);
    EXPECT_EQ(wrapper->CountInPriority(cb3, UlsrPriority::LOW), 1);
    EXPECT_EQ(wrapper->CountInPriority(cb4, UlsrPriority::HIGH), 1);

    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest027 end!");
}

/**
 * @tc.name: UlsrCallbackHolderTest028
 * @tc.desc: Test UlsrCallbackProxy::OnSyncUlsr with ERR_INVALID_VALUE to cover ret != ERR_OK branch
 * @tc.type: FUNC
 */
HWTEST_F(UlsrCallbackHolderTest, UlsrCallbackHolderTest028, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest028 start!");

    // Use RAII guard to ensure test isolation - save and restore mock state
    {
        MockRequestValueGuard guard(ERR_OK);
        sptr<MockPowerRemoteObject> mockRemote = new MockPowerRemoteObject();
        EXPECT_TRUE(mockRemote != nullptr);

        UlsrCallbackProxy proxy(mockRemote);
        EXPECT_TRUE(proxy.Remote() != nullptr);

        // Case 1: ERR_OK branch
        proxy.OnSyncUlsr();
    }

    // Case 2: ERR_INVALID_VALUE branch - ret != ERR_OK
    {
        MockRequestValueGuard guard(ERR_INVALID_VALUE);
        sptr<MockPowerRemoteObject> mockRemote = new MockPowerRemoteObject();
        EXPECT_TRUE(mockRemote != nullptr);

        UlsrCallbackProxy proxy(mockRemote);
        proxy.OnSyncUlsr();
    }

    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest028 end!");
}

/**
 * @tc.name: UlsrCallbackHolderTest029
 * @tc.desc: Test UlsrCallbackProxy::OnAsyncWakeup with ERR_INVALID_VALUE to cover ret != ERR_OK branch
 * @tc.type: FUNC
 */
HWTEST_F(UlsrCallbackHolderTest, UlsrCallbackHolderTest029, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest029 start!");

    // Use RAII guard to ensure test isolation - save and restore mock state
    {
        MockRequestValueGuard guard(ERR_OK);
        sptr<MockPowerRemoteObject> mockRemote = new MockPowerRemoteObject();
        EXPECT_TRUE(mockRemote != nullptr);

        UlsrCallbackProxy proxy(mockRemote);
        EXPECT_TRUE(proxy.Remote() != nullptr);

        // Case 1: ERR_OK branch
        proxy.OnAsyncWakeup();
    }

    // Case 2: ERR_INVALID_VALUE branch - ret != ERR_OK
    {
        MockRequestValueGuard guard(ERR_INVALID_VALUE);
        sptr<MockPowerRemoteObject> mockRemote = new MockPowerRemoteObject();
        EXPECT_TRUE(mockRemote != nullptr);

        UlsrCallbackProxy proxy(mockRemote);
        proxy.OnAsyncWakeup();
    }

    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest029 end!");
}

/**
 * @tc.name: UlsrCallbackHolderTest030
 * @tc.desc: Test SyncUlsrNotify anti-reentry - consecutive calls rejected (state already SYNC_ULSR)
 * @tc.type: FUNC
 */
HWTEST_F(UlsrCallbackHolderTest, UlsrCallbackHolderTest030, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest030 start!");
    sptr<UlsrCallbackHolder> holder = new UlsrCallbackHolder();
    EXPECT_TRUE(holder != nullptr);

    sptr<TestUlsrCallback> cb = new TestUlsrCallback();
    EXPECT_TRUE(cb != nullptr);
    holder->AddCallback(cb, {1, 1}, UlsrPriority::HIGH);
    cb->ResetCallFlags();
    EXPECT_FALSE(cb->IsSyncCalled());

    // First call: IDLE -> SYNC_ULSR (CAS success), callback should be triggered
    holder->SyncUlsrNotify();
    EXPECT_TRUE(cb->IsSyncCalled());  // First call triggers OnSyncUlsr
    // Second call: state is SYNC_ULSR, expected IDLE, CAS fails -> rejected
    holder->SyncUlsrNotify();
    EXPECT_FALSE(cb->IsAsyncCalled());
    // IsSyncCalled is still true (only set by first call), but no second callback was triggered

    // Restore state to IDLE for other tests
    cb->ResetCallFlags();
    holder->WakeupNotify(false);
    EXPECT_TRUE(cb->IsAsyncCalled());  // WakeupNotify succeeded, triggers OnAsyncWakeup

    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest030 end!");
}

/**
 * @tc.name: UlsrCallbackHolderTest031
 * @tc.desc: Test WakeupNotify anti-reentry - call without SyncUlsrNotify (state is IDLE, expected SYNC_ULSR)
 * @tc.type: FUNC
 */
HWTEST_F(UlsrCallbackHolderTest, UlsrCallbackHolderTest031, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest031 start!");
    sptr<UlsrCallbackHolder> holder = new UlsrCallbackHolder();
    EXPECT_TRUE(holder != nullptr);

    sptr<TestUlsrCallback> cb = new TestUlsrCallback();
    EXPECT_TRUE(cb != nullptr);
    holder->AddCallback(cb, {1, 1}, UlsrPriority::HIGH);
    cb->ResetCallFlags();
    EXPECT_FALSE(cb->IsAsyncCalled());

    // Call WakeupNotify without calling SyncUlsrNotify first
    // State is IDLE, expected SYNC_ULSR, CAS fails -> rejected
    // Async callback should NOT be triggered
    holder->WakeupNotify(true);
    EXPECT_FALSE(cb->IsAsyncCalled());  // Should still be false (rejected)

    // Also test consecutive WakeupNotify after a valid cycle
    cb->ResetCallFlags();
    holder->SyncUlsrNotify();  // IDLE -> SYNC_ULSR
    EXPECT_TRUE(cb->IsSyncCalled());
    holder->WakeupNotify(false);    // SYNC_ULSR -> IDLE (success), callback triggered
    EXPECT_TRUE(cb->IsAsyncCalled());  // First WakeupNotify succeeds

    // Second WakeupNotify when state is already IDLE should be rejected
    cb->ResetCallFlags();
    holder->WakeupNotify(true);    // IDLE -> IDLE (fails, already IDLE)
    EXPECT_FALSE(cb->IsAsyncCalled());  // Should still be false (rejected)

    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest031 end!");
}

/**
 * @tc.name: UlsrCallbackHolderTest032
 * @tc.desc: Test SyncUlsrNotifyInner and WakeupNotify with null callback - if (cb == nullptr) branch coverage
 * @tc.type: FUNC
 */
HWTEST_F(UlsrCallbackHolderTest, UlsrCallbackHolderTest032, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest032 start!");
    sptr<UlsrCallbackHolder> holder = new UlsrCallbackHolder();
    EXPECT_TRUE(holder != nullptr);
    UlsrCallbackHolderTestWrapper* wrapper = static_cast<UlsrCallbackHolderTestWrapper*>(holder.GetRefPtr());

    sptr<TestUlsrCallback> cb1 = new TestUlsrCallback();
    sptr<TestUlsrCallback> cb2 = new TestUlsrCallback();
    sptr<TestUlsrCallback> cb3 = new TestUlsrCallback();
    EXPECT_TRUE(cb1 != nullptr);
    EXPECT_TRUE(cb2 != nullptr);
    EXPECT_TRUE(cb3 != nullptr);

    // Add normal callbacks
    holder->AddCallback(cb1, {1, 1}, UlsrPriority::HIGH);
    holder->AddCallback(cb2, {2, 2}, UlsrPriority::DEFAULT);
    holder->AddCallback(cb3, {3, 3}, UlsrPriority::LOW);

    // Insert null callback record to HIGH queue
    wrapper->InsertNullCallbackRecord({4, 4}, UlsrPriority::HIGH);
    EXPECT_EQ(wrapper->CountInPriority(nullptr, UlsrPriority::HIGH), 1);  // Detected 1 nullptr callback

    // Test SyncUlsrNotifyInner - null callback should be skipped, normal callback triggered
    holder->SyncUlsrNotify();
    EXPECT_TRUE(cb1->IsSyncCalled());  // Normal callback triggered
    EXPECT_TRUE(cb2->IsSyncCalled());
    EXPECT_TRUE(cb3->IsSyncCalled());

    // Test WakeupNotify - null callback should be skipped, normal callback triggered
    holder->WakeupNotify(true);
    EXPECT_TRUE(cb1->IsAsyncCalled());  // Normal callback triggered
    EXPECT_TRUE(cb2->IsAsyncCalled());
    EXPECT_TRUE(cb3->IsAsyncCalled());

    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest032 end!");
}

/**
 * @tc.name: UlsrCallbackHolderTest033
 * @tc.desc: Test SyncUlsrNotifyInner timeout branches - remainingTimeMs < 0 at start and after processing
 * @tc.type: FUNC
 */
HWTEST_F(UlsrCallbackHolderTest, UlsrCallbackHolderTest033, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest033 start!");
    sptr<UlsrCallbackHolder> holder = new UlsrCallbackHolder();
    EXPECT_TRUE(holder != nullptr);
    UlsrCallbackHolderTestWrapper* wrapper = static_cast<UlsrCallbackHolderTestWrapper*>(holder.GetRefPtr());

    sptr<TestUlsrCallback> cb1 = new TestUlsrCallback();
    sptr<TestUlsrCallback> cb2 = new TestUlsrCallback();
    EXPECT_TRUE(cb1 != nullptr);
    EXPECT_TRUE(cb2 != nullptr);

    holder->AddCallback(cb1, {1, 1}, UlsrPriority::HIGH);
    holder->AddCallback(cb2, {2, 2}, UlsrPriority::DEFAULT);

    int64_t result = wrapper->SyncUlsrNotifyInner(0);
    EXPECT_EQ(result, 0);
    EXPECT_FALSE(cb1->IsSyncCalled());
    EXPECT_FALSE(cb2->IsSyncCalled());

    cb1->ResetCallFlags();
    cb2->ResetCallFlags();
    result = wrapper->SyncUlsrNotifyInner(-1000);
    EXPECT_EQ(result, -1000);
    EXPECT_FALSE(cb1->IsSyncCalled());
    EXPECT_FALSE(cb2->IsSyncCalled());

    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest033 end!");
}

} // namespace PowerMgr
} // namespace OHOS