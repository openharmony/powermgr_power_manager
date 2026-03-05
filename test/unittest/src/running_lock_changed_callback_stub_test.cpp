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

#include "running_lock_changed_callback_stub_test.h"

#include <gmock/gmock.h>
#include <iremote_object.h>
#include <iremote_stub.h>
#include <message_parcel.h>
#include <message_option.h>
#include <ipc_skeleton.h>

#include "running_lock_changed_callback_stub.h"
#include "irunning_lock_changed_callback.h"
#include "running_lock_changed_callback_ipc_interface_code.h"
#include "power_log.h"
#include "power_common.h"
#include "power_mgr_errors.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;

namespace {

class TestRunningLockChangedCallback : public RunningLockChangedCallbackStub {
public:
    TestRunningLockChangedCallback() : stateReceived_(false) {}
    virtual ~TestRunningLockChangedCallback() = default;
    void OnAsyncScreenRunningLockChanged(RunningLockChangeState state) override
    {
        stateReceived_ = true;
        lastState_ = state;
    }
    bool stateReceived_;
    RunningLockChangeState lastState_;
};

}

HWTEST_F(RunningLockChangedCallbackStubTest,
    OnRemoteRequest_DescriptorMismatch_ReturnError, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "OnRemoteRequest_DescriptorMismatch_ReturnError start!");

    sptr<TestRunningLockChangedCallback> stub = new TestRunningLockChangedCallback();
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(u"wrong.descriptor");

    int ret = stub->OnRemoteRequest(
        static_cast<uint32_t>(RunningLockChangedCallbackInterfaceCode::RUNNINGLOCK_STATE_CHANGED),
        data, reply, option);

    EXPECT_TRUE(ret == E_GET_POWER_SERVICE_FAILED);

    POWER_HILOGI(LABEL_TEST, "OnRemoteRequest_DescriptorMismatch_ReturnError end!");
}

HWTEST_F(RunningLockChangedCallbackStubTest,
    OnRemoteRequest_CodeNotMatch_CallBase, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "OnRemoteRequest_CodeNotMatch_CallBase start!");

    sptr<TestRunningLockChangedCallback> stub = new TestRunningLockChangedCallback();
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(RunningLockChangedCallbackStub::GetDescriptor());

    int ret = stub->OnRemoteRequest(999, data, reply, option);

    EXPECT_FALSE(ret == ERR_OK);

    POWER_HILOGI(LABEL_TEST, "OnRemoteRequest_CodeNotMatch_CallBase end!");
}

HWTEST_F(RunningLockChangedCallbackStubTest,
    OnRemoteRequest_ReadParcelFailed_ReturnError, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "OnRemoteRequest_ReadParcelFailed_ReturnError start!");

    sptr<TestRunningLockChangedCallback> stub = new TestRunningLockChangedCallback();
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(RunningLockChangedCallbackStub::GetDescriptor());

    int ret = stub->OnRemoteRequest(
        static_cast<uint32_t>(RunningLockChangedCallbackInterfaceCode::RUNNINGLOCK_STATE_CHANGED),
        data, reply, option);

    EXPECT_TRUE(ret == E_READ_PARCEL_ERROR);

    POWER_HILOGI(LABEL_TEST, "OnRemoteRequest_ReadParcelFailed_ReturnError end!");
}

HWTEST_F(RunningLockChangedCallbackStubTest,
    OnRemoteRequest_Success_InvokeCallback, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "OnRemoteRequest_Success_InvokeCallback start!");

    sptr<TestRunningLockChangedCallback> stub = new TestRunningLockChangedCallback();
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(RunningLockChangedCallbackStub::GetDescriptor());
    data.WriteUint32(static_cast<uint32_t>(RunningLockChangeState::RUNNINGLOCK_STATE_LOCKED));

    int ret = stub->OnRemoteRequest(
        static_cast<uint32_t>(RunningLockChangedCallbackInterfaceCode::RUNNINGLOCK_STATE_CHANGED),
        data, reply, option);

    EXPECT_TRUE(ret == ERR_OK);
    EXPECT_TRUE(stub->stateReceived_);
    EXPECT_EQ(stub->lastState_, RunningLockChangeState::RUNNINGLOCK_STATE_LOCKED);

    POWER_HILOGI(LABEL_TEST, "OnRemoteRequest_Success_InvokeCallback end!");
}

HWTEST_F(RunningLockChangedCallbackStubTest,
    OnRemoteRequest_UnlockedState_InvokeCallback, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "OnRemoteRequest_UnlockedState_InvokeCallback start!");

    sptr<TestRunningLockChangedCallback> stub = new TestRunningLockChangedCallback();
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(RunningLockChangedCallbackStub::GetDescriptor());
    data.WriteUint32(static_cast<uint32_t>(RunningLockChangeState::RUNNINGLOCK_STATE_UNLOCKED));

    int ret = stub->OnRemoteRequest(
        static_cast<uint32_t>(RunningLockChangedCallbackInterfaceCode::RUNNINGLOCK_STATE_CHANGED),
        data, reply, option);

    EXPECT_TRUE(ret == ERR_OK);
    EXPECT_TRUE(stub->stateReceived_);
    EXPECT_EQ(stub->lastState_, RunningLockChangeState::RUNNINGLOCK_STATE_UNLOCKED);

    POWER_HILOGI(LABEL_TEST, "OnRemoteRequest_UnlockedState_InvokeCallback end!");
}
