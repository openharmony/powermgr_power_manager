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

#include "zidl_callback_stub_test.h"

#include <gmock/gmock.h>
#include <iremote_object.h>
#include <iremote_stub.h>
#include <message_parcel.h>
#include <message_option.h>

#include "power_mode_callback_stub.h"
#include "power_runninglock_callback_stub.h"
#include "power_state_callback_stub.h"
#include "screen_off_pre_callback_stub.h"
#include "ulsr_callback_stub.h"
#include "sync_hibernate_callback_stub.h"
#include "sync_sleep_callback_stub.h"
#include "running_lock_changed_callback_stub.h"
#include "takeover_suspend_callback_stub.h"
#include "power_mgr_async_reply.h"
#include "power_mgr_async_reply_stub.h"
#include "power_log.h"
#include "power_common.h"
#include "power_mgr_errors.h"
#include "power_mode_callback_ipc_interface_code.h"
#include "power_state_callback_ipc_interface_code.h"
#include "power_runninglock_callback_ipc_interface_code.h"
#include "screen_off_pre_callback_ipc_interface_code.h"
#include "running_lock_changed_callback_ipc_interface_code.h"
#include "hibernate/sync_hibernate_callback_ipc_interface_code.h"
#include "suspend/sync_sleep_callback_ipc_interface_code.h"
#include "suspend/take_over_callback_ipc_interface_code.h"
#include "shutdown/takeover_shutdown_callback_stub.h"
#include "shutdown/takeover_shutdown_callback_ipc_interface_code.h"
#include "shutdown/takeover_info.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;

static constexpr uint32_t UNKNOWN_INTERFACE_CODE = 9999;

// Helper: test wrong descriptor for any stub
template <typename T>
int TestWrongDescriptor(uint32_t code)
{
    auto stub = new T();
    EXPECT_TRUE(stub != nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(u"wrong.descriptor");
    int ret = stub->OnRemoteRequest(code, data, reply, option);
    delete stub;
    return ret;
}

// Helper: test unknown code for any stub
template <typename T>
int TestUnknownCode()
{
    auto stub = new T();
    EXPECT_TRUE(stub != nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(stub->GetDescriptor());
    int ret = stub->OnRemoteRequest(UNKNOWN_INTERFACE_CODE, data, reply, option);
    delete stub;
    return ret;
}

// ==================== PowerModeCallbackStub ====================

HWTEST_F(ZidlCallbackStubTest, PowerMode_WrongDescriptor, TestSize.Level1)
{
    auto stub = new PowerModeCallbackStub();
    EXPECT_TRUE(stub != nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(u"wrong.descriptor");
    int ret = stub->OnRemoteRequest(
        static_cast<uint32_t>(PowerModeCallbackInterfaceCode::POWER_MODE_CHANGED), data, reply, option);
    EXPECT_EQ(ret, E_GET_POWER_SERVICE_FAILED);
    delete stub;
}

HWTEST_F(ZidlCallbackStubTest, PowerMode_UnknownCode, TestSize.Level1)
{
    auto stub = new PowerModeCallbackStub();
    EXPECT_TRUE(stub != nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(stub->GetDescriptor());
    int ret = stub->OnRemoteRequest(UNKNOWN_INTERFACE_CODE, data, reply, option);
    EXPECT_NE(ret, E_GET_POWER_SERVICE_FAILED);
    delete stub;
}

HWTEST_F(ZidlCallbackStubTest, PowerMode_Normal, TestSize.Level1)
{
    auto stub = new PowerModeCallbackStub();
    EXPECT_TRUE(stub != nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(stub->GetDescriptor());
    data.WriteUint32(static_cast<uint32_t>(PowerMode::NORMAL_MODE));
    int ret = stub->OnRemoteRequest(
        static_cast<uint32_t>(PowerModeCallbackInterfaceCode::POWER_MODE_CHANGED), data, reply, option);
    EXPECT_EQ(ret, ERR_OK);
    delete stub;
}

// ==================== PowerRunningLockCallbackStub ====================

HWTEST_F(ZidlCallbackStubTest, RunningLock_WrongDescriptor, TestSize.Level1)
{
    EXPECT_EQ(TestWrongDescriptor<PowerRunningLockCallbackStub>(
        static_cast<uint32_t>(PowerRunningLockCallbackInterfaceCode::POWER_RUNNINGLOCK_CHANGED)),
        E_GET_POWER_SERVICE_FAILED);
}

HWTEST_F(ZidlCallbackStubTest, RunningLock_UnknownCode, TestSize.Level1)
{
    auto ret = TestUnknownCode<PowerRunningLockCallbackStub>();
    EXPECT_NE(ret, E_GET_POWER_SERVICE_FAILED);
}

HWTEST_F(ZidlCallbackStubTest, RunningLock_Normal, TestSize.Level1)
{
    auto stub = new PowerRunningLockCallbackStub();
    EXPECT_TRUE(stub != nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(stub->GetDescriptor());
    data.WriteString("test_message");
    int ret = stub->OnRemoteRequest(
        static_cast<uint32_t>(PowerRunningLockCallbackInterfaceCode::POWER_RUNNINGLOCK_CHANGED), data, reply, option);
    EXPECT_EQ(ret, ERR_OK);
    ret = stub->OnRemoteRequest(static_cast<uint32_t>(999), data, reply, option);
    EXPECT_NE(ret, ERR_OK);
    delete stub;
}

// ==================== PowerStateCallbackStub ====================

HWTEST_F(ZidlCallbackStubTest, PowerState_WrongDescriptor, TestSize.Level1)
{
    EXPECT_EQ(TestWrongDescriptor<PowerStateCallbackStub>(
        static_cast<uint32_t>(PowerStateCallbackInterfaceCode::POWER_STATE_CHANGED)),
        E_GET_POWER_SERVICE_FAILED);
}

HWTEST_F(ZidlCallbackStubTest, PowerState_UnknownCode, TestSize.Level1)
{
    auto ret = TestUnknownCode<PowerStateCallbackStub>();
    EXPECT_NE(ret, E_GET_POWER_SERVICE_FAILED);
}

HWTEST_F(ZidlCallbackStubTest, PowerState_Normal, TestSize.Level1)
{
    auto stub = new PowerStateCallbackStub();
    EXPECT_TRUE(stub != nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(stub->GetDescriptor());
    data.WriteUint32(static_cast<uint32_t>(PowerState::AWAKE));
    int ret = stub->OnRemoteRequest(
        static_cast<uint32_t>(PowerStateCallbackInterfaceCode::POWER_STATE_CHANGED), data, reply, option);
    EXPECT_EQ(ret, ERR_OK);
    delete stub;
}

HWTEST_F(ZidlCallbackStubTest, PowerState_Async_Normal, TestSize.Level1)
{
    auto stub = new PowerStateCallbackStub();
    EXPECT_TRUE(stub != nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(stub->GetDescriptor());
    data.WriteUint32(static_cast<uint32_t>(PowerState::AWAKE));
    int ret = stub->OnRemoteRequest(
        static_cast<uint32_t>(PowerStateCallbackInterfaceCode::ASYNC_POWER_STATE_CHANGED), data, reply, option);
    EXPECT_EQ(ret, ERR_OK);
    delete stub;
}

// ==================== ScreenOffPreCallbackStub ====================

HWTEST_F(ZidlCallbackStubTest, ScreenOffPre_WrongDescriptor, TestSize.Level1)
{
    EXPECT_EQ(TestWrongDescriptor<ScreenOffPreCallbackStub>(
        static_cast<uint32_t>(ScreenOffPreCallbackInterfaceCode::SCREEN_OFF_PRE_CHANGED)),
        E_GET_POWER_SERVICE_FAILED);
}

HWTEST_F(ZidlCallbackStubTest, ScreenOffPre_UnknownCode, TestSize.Level1)
{
    auto ret = TestUnknownCode<ScreenOffPreCallbackStub>();
    EXPECT_NE(ret, E_GET_POWER_SERVICE_FAILED);
}

HWTEST_F(ZidlCallbackStubTest, ScreenOffPre_Normal, TestSize.Level1)
{
    auto stub = new ScreenOffPreCallbackStub();
    EXPECT_TRUE(stub != nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(stub->GetDescriptor());
    data.WriteUint32(1);
    int ret = stub->OnRemoteRequest(
        static_cast<uint32_t>(ScreenOffPreCallbackInterfaceCode::SCREEN_OFF_PRE_CHANGED), data, reply, option);
    EXPECT_EQ(ret, ERR_OK);
    delete stub;
}

// ==================== SyncHibernateCallbackStub ====================

HWTEST_F(ZidlCallbackStubTest, SyncHibernate_WrongDescriptor, TestSize.Level1)
{
    EXPECT_EQ(TestWrongDescriptor<SyncHibernateCallbackStub>(
        static_cast<uint32_t>(SyncHibernateCallbackInterfaceCode::CMD_ON_SYNC_HIBERNATE)),
        E_GET_POWER_SERVICE_FAILED);
}

HWTEST_F(ZidlCallbackStubTest, SyncHibernate_UnknownCode, TestSize.Level1)
{
    auto ret = TestUnknownCode<SyncHibernateCallbackStub>();
    EXPECT_NE(ret, E_GET_POWER_SERVICE_FAILED);
}

HWTEST_F(ZidlCallbackStubTest, SyncHibernate_Normal, TestSize.Level1)
{
    auto stub = new SyncHibernateCallbackStub();
    EXPECT_TRUE(stub != nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(stub->GetDescriptor());
    int ret = stub->OnRemoteRequest(
        static_cast<uint32_t>(SyncHibernateCallbackInterfaceCode::CMD_ON_SYNC_HIBERNATE), data, reply, option);
    EXPECT_EQ(ret, ERR_OK);
    delete stub;
}

HWTEST_F(ZidlCallbackStubTest, SyncHibernate_Wakeup_Normal, TestSize.Level1)
{
    auto stub = new SyncHibernateCallbackStub();
    EXPECT_TRUE(stub != nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(stub->GetDescriptor());
    data.WriteBool(true);
    int ret = stub->OnRemoteRequest(
        static_cast<uint32_t>(SyncHibernateCallbackInterfaceCode::CMD_ON_SYNC_WAKEUP), data, reply, option);
    EXPECT_EQ(ret, ERR_OK);
    delete stub;
}

// ==================== RunningLockChangedCallbackStub ====================

HWTEST_F(ZidlCallbackStubTest, RunningLockChanged_WrongDescriptor, TestSize.Level1)
{
    EXPECT_EQ(TestWrongDescriptor<RunningLockChangedCallbackStub>(
        static_cast<uint32_t>(RunningLockChangedCallbackInterfaceCode::RUNNINGLOCK_STATE_CHANGED)),
        E_GET_POWER_SERVICE_FAILED);
}

HWTEST_F(ZidlCallbackStubTest, RunningLockChanged_UnknownCode, TestSize.Level1)
{
    auto ret = TestUnknownCode<RunningLockChangedCallbackStub>();
    EXPECT_NE(ret, E_GET_POWER_SERVICE_FAILED);
}

HWTEST_F(ZidlCallbackStubTest, RunningLockChanged_Normal, TestSize.Level1)
{
    auto stub = new RunningLockChangedCallbackStub();
    EXPECT_TRUE(stub != nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(stub->GetDescriptor());
    data.WriteUint32(static_cast<uint32_t>(RunningLockChangeState::RUNNINGLOCK_STATE_LOCKED));
    data.WriteUint64(UINT64_MAX);
    int ret = stub->OnRemoteRequest(
        static_cast<uint32_t>(RunningLockChangedCallbackInterfaceCode::RUNNINGLOCK_STATE_CHANGED),
        data, reply, option);
    EXPECT_EQ(ret, ERR_OK);
    ret = stub->OnRemoteRequest(999, data, reply, option);
    EXPECT_NE(ret, ERR_OK);
    delete stub;
}

// ==================== TakeOverShutdownCallbackStub ====================

HWTEST_F(ZidlCallbackStubTest, TakeOverShutdown_WrongDescriptor, TestSize.Level1)
{
    EXPECT_EQ(TestWrongDescriptor<TakeOverShutdownCallbackStub>(
        static_cast<uint32_t>(TakeoverShutdownCallbackInterfaceCode::CMD_ON_TAKEOVER_SHUTDOWN)),
        E_GET_POWER_SERVICE_FAILED);
}

HWTEST_F(ZidlCallbackStubTest, TakeOverShutdown_ReadParcelableNull, TestSize.Level1)
{
    auto stub = new TakeOverShutdownCallbackStub();
    EXPECT_TRUE(stub != nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(stub->GetDescriptor());
    int ret = stub->OnRemoteRequest(
        static_cast<uint32_t>(TakeoverShutdownCallbackInterfaceCode::CMD_ON_TAKEOVER_SHUTDOWN),
        data, reply, option);
    EXPECT_EQ(ret, ERR_OK);
    delete stub;
}

HWTEST_F(ZidlCallbackStubTest, TakeOverHibernate_ReadParcelableNull, TestSize.Level1)
{
    auto stub = new TakeOverShutdownCallbackStub();
    EXPECT_TRUE(stub != nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(stub->GetDescriptor());
    int ret = stub->OnRemoteRequest(
        static_cast<uint32_t>(TakeoverShutdownCallbackInterfaceCode::CMD_ON_TAKEOVER_HIBERNATE),
        data, reply, option);
    EXPECT_EQ(ret, ERR_OK);
    delete stub;
}

HWTEST_F(ZidlCallbackStubTest, TakeOverShutdown_UnknownCode, TestSize.Level1)
{
    auto ret = TestUnknownCode<TakeOverShutdownCallbackStub>();
    EXPECT_NE(ret, E_GET_POWER_SERVICE_FAILED);
}

// ==================== PowerMgrStubAsync ====================

HWTEST_F(ZidlCallbackStubTest, AsyncReply_WrongDescriptor, TestSize.Level1)
{
    auto stub = new PowerMgrStubAsync();
    EXPECT_TRUE(stub != nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(u"wrong.descriptor");
    int ret = stub->OnRemoteRequest(PowerMgrStubAsync::SEND_ASYNC_REPLY, data, reply, option);
    EXPECT_EQ(ret, E_GET_POWER_SERVICE_FAILED);
    delete stub;
}

HWTEST_F(ZidlCallbackStubTest, AsyncReply_UnknownCode, TestSize.Level1)
{
    auto stub = new PowerMgrStubAsync();
    EXPECT_TRUE(stub != nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(stub->GetDescriptor());
    int ret = stub->OnRemoteRequest(UNKNOWN_INTERFACE_CODE, data, reply, option);
    EXPECT_NE(ret, ERR_OK);
    delete stub;
}

HWTEST_F(ZidlCallbackStubTest, AsyncReply_Normal, TestSize.Level1)
{
    auto stub = new PowerMgrStubAsync();
    EXPECT_TRUE(stub != nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(stub->GetDescriptor());
    data.WriteInt32(42);
    int ret = stub->OnRemoteRequest(PowerMgrStubAsync::SEND_ASYNC_REPLY, data, reply, option);
    EXPECT_EQ(ret, ERR_OK);
    delete stub;
}
