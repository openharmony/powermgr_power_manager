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

#include "power_common.h"
#include "power_log.h"
#include "async_ulsr_callback_stub.h"
#include "ulsr/ulsr_callback_holder.h"

namespace OHOS {
namespace PowerMgr {

using namespace testing::ext;
void UlsrCallbackHolderTest::SetUpTestCase()
{
}

void UlsrCallbackHolderTest::TearDownTestCase()
{
}

void UlsrCallbackHolderTest::SetUp()
{
}

void UlsrCallbackHolderTest::TearDown()
{
}

class Test1AsyncUlsrCallback : public IAsyncUlsrCallback {
public:
    Test1AsyncUlsrCallback() = default;
    virtual ~Test1AsyncUlsrCallback() = default;

    void OnAsyncWakeup() override
    {
        POWER_HILOGI(LABEL_TEST, "Test1AsyncUlsrCallback OnAsyncWakeup!");
    }
    sptr<IRemoteObject> AsObject() override
    {
        return nullptr;
    }
};

class Test2AsyncUlsrCallback : public AsyncUlsrCallbackStub {
public:
    Test2AsyncUlsrCallback() = default;
    virtual ~Test2AsyncUlsrCallback() = default;

    void OnAsyncWakeup() override
    {
        POWER_HILOGI(LABEL_TEST, "Test2AsyncUlsrCallback OnAsyncWakeup!");
    }
};

/**
 * @tc.name: UlsrCallbackHolderTest001
 * @tc.desc: Test AddCallback
 * @tc.type: FUNC
 */
HWTEST_F(UlsrCallbackHolderTest, UlsrCallbackHolderTest001, TestSize.Level2) {
    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest001 start!");
    sptr<UlsrCallbackHolder> ulsrCallbackHolder = new UlsrCallbackHolder();
    ulsrCallbackHolder->AddCallback(nullptr, { 6, 6 });
    EXPECT_EQ(ulsrCallbackHolder->cachedCallbacks_.size(), 0);
    sptr<Test1AsyncUlsrCallback> callback1 = new Test1AsyncUlsrCallback();
    ulsrCallbackHolder->AddCallback(callback1, { 6, 6 });
    EXPECT_EQ(ulsrCallbackHolder->cachedCallbacks_.size(), 0);

    sptr<Test2AsyncUlsrCallback> callback2 = new Test2AsyncUlsrCallback();
    ulsrCallbackHolder->AddCallback(callback2, { 6, 6 });
    EXPECT_EQ(ulsrCallbackHolder->cachedCallbacks_.size(), 1);
    ulsrCallbackHolder->AddCallback(callback2, { 6, 6 });
    EXPECT_EQ(ulsrCallbackHolder->cachedCallbacks_.size(), 1);

    ulsrCallbackHolder->WakeupNotify();

    ulsrCallbackHolder->RemoveCallback(callback2);
    EXPECT_EQ(ulsrCallbackHolder->cachedCallbacks_.size(), 0);
    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest001 end!");
}

/**
 * @tc.name: UlsrCallbackHolderTest002
 * @tc.desc: Test RemoveCallback
 * @tc.type: FUNC
 */
HWTEST_F(UlsrCallbackHolderTest, UlsrCallbackHolderTest002, TestSize.Level2) {
    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest002 start!");
    sptr<UlsrCallbackHolder> ulsrCallbackHolder = new UlsrCallbackHolder();
    ulsrCallbackHolder->RemoveCallback(nullptr);
    EXPECT_EQ(ulsrCallbackHolder->cachedCallbacks_.size(), 0);
    sptr<Test1AsyncUlsrCallback> callback1 = new Test1AsyncUlsrCallback();
    ulsrCallbackHolder->RemoveCallback(callback1);
    EXPECT_EQ(ulsrCallbackHolder->cachedCallbacks_.size(), 0);
    sptr<Test2AsyncUlsrCallback> callback2 = new Test2AsyncUlsrCallback();
    ulsrCallbackHolder->RemoveCallback(callback2);
    EXPECT_EQ(ulsrCallbackHolder->cachedCallbacks_.size(), 0);
    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest002 end!");
}

/**
 * @tc.name: UlsrCallbackHolderTest003
 * @tc.desc: Test OnRemoteDied
 * @tc.type: FUNC
 */
HWTEST_F(UlsrCallbackHolderTest, UlsrCallbackHolderTest003, TestSize.Level2) {
    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest003 start!");
    sptr<UlsrCallbackHolder> ulsrCallbackHolder = new UlsrCallbackHolder();
    ulsrCallbackHolder->OnRemoteDied(nullptr);
    sptr<IRemoteObject> obj = nullptr;
    ulsrCallbackHolder->OnRemoteDied(obj);
    sptr<Test2AsyncUlsrCallback> callback1 = new Test2AsyncUlsrCallback();
    ulsrCallbackHolder->AddCallback(callback1, { 6, 6 });
    ulsrCallbackHolder->OnRemoteDied(callback1->AsObject());
    EXPECT_EQ(ulsrCallbackHolder->cachedCallbacks_.size(), 0);
    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest003 end!");
}

/**
 * @tc.name: UlsrCallbackHolderTest004
 * @tc.desc: Test WakeupNotify
 * @tc.type: FUNC
 */
HWTEST_F(UlsrCallbackHolderTest, UlsrCallbackHolderTest004, TestSize.Level2) {
    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest004 start!");
    sptr<UlsrCallbackHolder> ulsrCallbackHolder = new UlsrCallbackHolder();
    sptr<IAsyncUlsrCallback> cb = nullptr;
    ulsrCallbackHolder->cachedCallbacks_.emplace(cb, std::make_pair(6, 6));
    ulsrCallbackHolder->WakeupNotify();
    EXPECT_EQ(ulsrCallbackHolder->cachedCallbacks_.size(), 1);
    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::UlsrCallbackHolderTest004 end!");
}

/**
 * @tc.name: UlsrCallbackHolderTest005
 * @tc.desc: Test WakeupNotify
 * @tc.type: FUNC
 */
HWTEST_F(UlsrCallbackHolderTest, AsyncUlsrCallbackStub001, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::AsyncUlsrCallbackStub001 end!");
    Test2AsyncUlsrCallback cb;
    MessageParcel reply;
    MessageOption opt;

    MessageParcel data0;
    data0.WriteInterfaceToken(u"test.interface.token");
    int32_t ret0 = cb.OnRemoteRequest(0, data0, reply, opt);
    EXPECT_EQ(ret0, E_GET_POWER_SERVICE_FAILED);

    MessageParcel data1;
    data1.WriteInterfaceToken(Test2AsyncUlsrCallback::GetDescriptor());
    int32_t ret1 = cb.OnRemoteRequest(0, data1, reply, opt);
    EXPECT_EQ(ret1, ERR_OK);

    MessageParcel data2;
    data2.WriteInterfaceToken(Test2AsyncUlsrCallback::GetDescriptor());
    int32_t ret2 = cb.OnRemoteRequest(-1, data2, reply, opt);
    EXPECT_NE(ret2, ERR_OK);
    POWER_HILOGI(LABEL_TEST, "UlsrCallbackHolderTest::AsyncUlsrCallbackStub001 end!");
}
} // namespace PowerMgr
} // namespace OHOS