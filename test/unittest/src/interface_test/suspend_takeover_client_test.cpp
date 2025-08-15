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

#include "suspend_takeover_client_test.h"

#include "power_log.h"
#include "power_mgr_client.h"
#include "power_mgr_service.h"
#include "take_over_callback_ipc_interface_code.h"
#include <condition_variable>
#include <future>
#include <mutex>

namespace OHOS {
namespace PowerMgr {
namespace UnitTest {
namespace {
MessageParcel g_reply;
MessageOption g_option;
bool g_isTakeOverSuspend = false;
} // namespace
using namespace testing::ext;

void SuspendTakeoverTest::SetUpTestCase() {}

void SuspendTakeoverTest::TearDownTestCase() {}

void SuspendTakeoverTest::SetUp()
{
    g_isTakeOverSuspend = false;
}

void SuspendTakeoverTest::TearDown() {}

bool SuspendTakeoverTest::TakeOverSuspendCallback::OnTakeOverSuspend(SuspendDeviceType type)
{
    g_isTakeOverSuspend = true;
    return true;
}

/**
 * @tc.name: RegisterSuspendCallback001
 * @tc.desc: Test RegisterSuspendCallback
 * @tc.type: FUNC
 */
HWTEST_F(SuspendTakeoverTest, RegisterSuspendTakeoverCallback001, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "RegisterSuspendTakeoverCallback001 function start!");
    auto& client = PowerMgrClient::GetInstance();
    sptr<ITakeOverSuspendCallback> takeOverSuspendCallback = new TakeOverSuspendCallback();
    client.RegisterSuspendTakeoverCallback(takeOverSuspendCallback, TakeOverSuspendPriority::DEFAULT);
    client.SuspendDevice();
    EXPECT_TRUE(g_isTakeOverSuspend);
    POWER_HILOGI(LABEL_TEST, "RegisterSuspendTakeoverCallback001 function end!");
}

/**
 * @tc.name: UnRegisterSuspendCallback001
 * @tc.desc: Test UnRegisterSuspendCallback
 * @tc.type: FUNC
 */
HWTEST_F(SuspendTakeoverTest, UnRegisterSuspendTakeoverCallback001, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "UnRegisterSuspendTakeoverCallback001 function start!");
    auto& client = PowerMgrClient::GetInstance();
    sptr<ITakeOverSuspendCallback> takeOverSuspendCallback = new TakeOverSuspendCallback();
    client.RegisterSuspendTakeoverCallback(takeOverSuspendCallback, TakeOverSuspendPriority::DEFAULT);
    client.UnRegisterSuspendTakeoverCallback(takeOverSuspendCallback);
    client.SuspendDevice();
    EXPECT_TRUE(g_isTakeOverSuspend);
    POWER_HILOGI(LABEL_TEST, "UnRegisterSuspendTakeoverCallback001 function end!");
}

/**
 * @tc.name: TakeOverSuspendCallbackStub001
 * @tc.desc: Test TakeOverSuspendCallbackStub
 * @tc.type: FUNC
 */
HWTEST_F(SuspendTakeoverTest, TakeOverSuspendCallbackStub001, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "TakeOverSuspendCallbackStub001 function start!");
    int32_t code = 0; // CMD_ON_TAKEOVER_SUSPEND
    MessageParcel data;
    MessageParcel reply;

    TakeOverSuspendCallback takeOverSuspendCallback;
    SuspendDeviceType type = SuspendDeviceType::SUSPEND_DEVICE_REASON_POWER_KEY;
    data.WriteUint32(static_cast<uint32_t>(type));
    data.WriteInterfaceToken(TakeOverSuspendCallback::GetDescriptor());
    int32_t ret = takeOverSuspendCallback.OnRemoteRequest(code, data, reply, g_option);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(reply.ReadBool(), true);
    EXPECT_EQ(g_isTakeOverSuspend, true);
    bool retVal = takeOverSuspendCallback.TakeOverSuspendCallbackStub::OnTakeOverSuspend(
        SuspendDeviceType::SUSPEND_DEVICE_REASON_POWER_KEY);
    EXPECT_EQ(retVal, false);
    POWER_HILOGI(LABEL_TEST, "TakeOverSuspendCallbackStub001 function end!");
}

/**
 * @tc.name: TakeOverSuspendCallbackStub002
 * @tc.desc: Test TakeOverSuspendCallbackStub
 * @tc.type: FUNC
 */
HWTEST_F(SuspendTakeoverTest, TakeOverSuspendCallbackStub002, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "TakeOverSuspendCallbackStub002 function start!");
    int32_t code = 2; // error code test
    MessageParcel data;
    MessageParcel reply;

    TakeOverSuspendCallback takeOverSuspendCallback;
    SuspendDeviceType type = SuspendDeviceType::SUSPEND_DEVICE_REASON_FORCE_SUSPEND;
    data.WriteInterfaceToken(TakeOverSuspendCallback::GetDescriptor());
    data.WriteUint32(static_cast<uint32_t>(type));
    int32_t ret = takeOverSuspendCallback.OnRemoteRequest(code, data, reply, g_option);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(reply.ReadBool(), true);
    EXPECT_EQ(g_isTakeOverSuspend, true);
    bool retVal = takeOverSuspendCallback.TakeOverSuspendCallbackStub::OnTakeOverSuspend(
        SuspendDeviceType::SUSPEND_DEVICE_REASON_FORCE_SUSPEND);
    EXPECT_EQ(retVal, false);
    POWER_HILOGI(LABEL_TEST, "TakeOverSuspendCallbackStub002 function end!");
}

/**
 * @tc.name: TakeOverSuspendCallbackStub003
 * @tc.desc: Test TakeOverSuspendCallbackStub::OnTakeOverSuspendCallbackStub
 * {descripter != remoteDescripter} case
 * @tc.type: FUNC
 */
HWTEST_F(SuspendTakeoverTest, TakeOverSuspendCallbackStub003, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "TakeOverSuspendCallbackStub003 function start!");
    int32_t code = static_cast<uint32_t>(TakeOverSuspendCallbackInterfaceCode::CMD_ON_TAKEOVER_SUSPEND);
    MessageParcel data;
    MessageParcel reply;
    data.WriteUint32(0);
    data.WriteInterfaceToken(u"test.interface.token");
    TakeOverSuspendCallback takeOverSuspendCallback;
    int32_t ret = takeOverSuspendCallback.OnRemoteRequest(code, data, reply, g_option);
    EXPECT_EQ(ret, E_GET_POWER_SERVICE_FAILED);
    POWER_HILOGI(LABEL_TEST, "TakeOverSuspendCallbackStub003 function end!");
}

/**
 * @tc.name: TakeOverSuspendCallbackStub004
 * @tc.desc: Test TakeOverSuspendCallbackStub::OnTakeOverSuspendCallbackStub
 * {data.ReadUint32(rawType) == false} case
 * @tc.type: FUNC
 */
HWTEST_F(SuspendTakeoverTest, TakeOverSuspendCallbackStub004, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "TakeOverSuspendCallbackStub004 function start!");
    int32_t code = static_cast<uint32_t>(TakeOverSuspendCallbackInterfaceCode::CMD_ON_TAKEOVER_SUSPEND);
    MessageParcel data;
    MessageParcel reply;
    TakeOverSuspendCallback takeOverSuspendCallback;
    data.WriteInterfaceToken(TakeOverSuspendCallback::GetDescriptor());
    int32_t ret = takeOverSuspendCallback.OnRemoteRequest(code, data, reply, g_option);
    EXPECT_EQ(ret, E_READ_PARCEL_ERROR);
    POWER_HILOGI(LABEL_TEST, "TakeOverSuspendCallbackStub004 function end!");
}

/**
 * @tc.name: TakeOverSuspendCallbackStub005
 * @tc.desc: Test TakeOverSuspendCallbackStub::OnTakeOverSuspendCallbackStub
 * {data.ReadUint32(rawType), rawType > max} case
 * @tc.type: FUNC
 */
HWTEST_F(SuspendTakeoverTest, TakeOverSuspendCallbackStub005, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "TakeOverSuspendCallbackStub005 function start!");
    int32_t code = static_cast<uint32_t>(TakeOverSuspendCallbackInterfaceCode::CMD_ON_TAKEOVER_SUSPEND);
    MessageParcel data;
    MessageParcel reply;
    TakeOverSuspendCallback takeOverSuspendCallback;
    data.WriteInterfaceToken(TakeOverSuspendCallback::GetDescriptor());
    data.WriteUint32(100);
    int32_t ret = takeOverSuspendCallback.OnRemoteRequest(code, data, reply, g_option);
    EXPECT_EQ(ret, E_READ_PARCEL_ERROR);
    POWER_HILOGI(LABEL_TEST, "TakeOverSuspendCallbackStub005 function end!");
}
} // namespace UnitTest
} // namespace PowerMgr
} // namespace OHOS
