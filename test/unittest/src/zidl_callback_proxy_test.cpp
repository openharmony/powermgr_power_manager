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

#include "zidl_callback_proxy_test.h"

#include <gmock/gmock.h>
#include <iremote_object.h>
#include <iremote_proxy.h>
#include <message_parcel.h>
#include <message_option.h>

#include "power_mode_callback_proxy.h"
#include "power_runninglock_callback_proxy.h"
#include "power_state_callback_proxy.h"
#include "screen_off_pre_callback_proxy.h"
#include "ulsr_callback_proxy.h"
#include "sync_sleep_callback_proxy.h"
#include "sync_hibernate_callback_proxy.h"
#include "takeover_suspend_callback_proxy.h"
#include "running_lock_changed_callback_proxy.h"
#include "power_mgr_async_reply_proxy.h"
#include "shutdown/async_shutdown_callback_proxy.h"
#include "shutdown/sync_shutdown_callback_proxy.h"
#include "shutdown/takeover_shutdown_callback_proxy.h"
#include "power_log.h"
#include "power_common.h"
#include "power_mgr_errors.h"
#include "mock_parcel.h"
#include "mock_power_remote_object.h"
#include "shutdown/takeover_info.h"
#include "irunning_lock_changed_callback.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;

// Helper macros to reduce boilerplate
static inline void SetupMockWriteTokenFail()
{
    MockWriteInterfaceToken(false);
}

static inline void SetupMockParcelWriteFail()
{
    MockWriteInterfaceToken(true);
    MockParcelWrite(false);
}

static inline void SetupMockSendRequestFail()
{
    MockWriteInterfaceToken(true);
    MockParcelWrite(true);
    MockPowerRemoteObject::SetRequestValue(ERR_INVALID_OPERATION);
}

static inline void SetupMockNormal()
{
    MockWriteInterfaceToken(true);
    MockParcelWrite(true);
    MockPowerRemoteObject::SetRequestValue(ERR_OK);
}

static inline void TeardownMock()
{
    ResetMockMessageParcel();
    ResetMockParcelWrite();
}

template <typename T>
sptr<T> MakeProxy()
{
    auto proxy = new T(new MockPowerRemoteObject());
    EXPECT_TRUE(proxy != nullptr);
    return proxy;
}

// ==================== WriteInterfaceToken Failure ====================

HWTEST_F(ZidlCallbackProxyTest, PowerMode_WriteTokenFail, TestSize.Level1)
{
    SetupMockWriteTokenFail();
    auto proxy = MakeProxy<PowerModeCallbackProxy>();
    ASSERT_TRUE(proxy != nullptr);
    proxy->OnPowerModeChanged(PowerMode::NORMAL_MODE);
    TeardownMock();
}

HWTEST_F(ZidlCallbackProxyTest, PowerRunningLock_WriteTokenFail, TestSize.Level1)
{
    SetupMockWriteTokenFail();
    auto proxy = MakeProxy<PowerRunningLockCallbackProxy>();
    ASSERT_TRUE(proxy != nullptr);
    proxy->HandleRunningLockMessage("test");
    TeardownMock();
}

HWTEST_F(ZidlCallbackProxyTest, PowerState_WriteTokenFail, TestSize.Level1)
{
    SetupMockWriteTokenFail();
    auto proxy = MakeProxy<PowerStateCallbackProxy>();
    ASSERT_TRUE(proxy != nullptr);
    proxy->OnPowerStateChanged(PowerState::AWAKE);
    TeardownMock();
}

HWTEST_F(ZidlCallbackProxyTest, PowerState_Async_WriteTokenFail, TestSize.Level1)
{
    SetupMockWriteTokenFail();
    auto proxy = MakeProxy<PowerStateCallbackProxy>();
    ASSERT_TRUE(proxy != nullptr);
    proxy->OnAsyncPowerStateChanged(PowerState::AWAKE);
    TeardownMock();
}

HWTEST_F(ZidlCallbackProxyTest, ScreenOffPre_WriteTokenFail, TestSize.Level1)
{
    SetupMockWriteTokenFail();
    auto proxy = MakeProxy<ScreenOffPreCallbackProxy>();
    ASSERT_TRUE(proxy != nullptr);
    proxy->OnScreenStateChanged(1);
    TeardownMock();
}

HWTEST_F(ZidlCallbackProxyTest, AsyncUlsr_WriteTokenFail, TestSize.Level1)
{
    SetupMockWriteTokenFail();
    auto proxy = MakeProxy<UlsrCallbackProxy>();
    ASSERT_TRUE(proxy != nullptr);
    proxy->OnAsyncWakeup();
    TeardownMock();
}

HWTEST_F(ZidlCallbackProxyTest, SyncSleep_WriteTokenFail, TestSize.Level1)
{
    SetupMockWriteTokenFail();
    auto proxy = MakeProxy<SyncSleepCallbackProxy>();
    ASSERT_TRUE(proxy != nullptr);
    proxy->OnSyncSleep(true);
    TeardownMock();
}

HWTEST_F(ZidlCallbackProxyTest, SyncWakeup_WriteTokenFail, TestSize.Level1)
{
    SetupMockWriteTokenFail();
    auto proxy = MakeProxy<SyncSleepCallbackProxy>();
    ASSERT_TRUE(proxy != nullptr);
    proxy->OnSyncWakeup(true);
    TeardownMock();
}

HWTEST_F(ZidlCallbackProxyTest, SyncHibernate_WriteTokenFail, TestSize.Level1)
{
    SetupMockWriteTokenFail();
    auto proxy = MakeProxy<SyncHibernateCallbackProxy>();
    ASSERT_TRUE(proxy != nullptr);
    proxy->OnSyncHibernate();
    TeardownMock();
}

HWTEST_F(ZidlCallbackProxyTest, SyncHibernateWakeup_WriteTokenFail, TestSize.Level1)
{
    SetupMockWriteTokenFail();
    auto proxy = MakeProxy<SyncHibernateCallbackProxy>();
    ASSERT_TRUE(proxy != nullptr);
    proxy->OnSyncWakeup(true);
    TeardownMock();
}

HWTEST_F(ZidlCallbackProxyTest, TakeOverSuspend_WriteTokenFail, TestSize.Level1)
{
    SetupMockWriteTokenFail();
    auto proxy = MakeProxy<TakeOverSuspendCallbackProxy>();
    EXPECT_FALSE(proxy->OnTakeOverSuspend(SuspendDeviceType::SUSPEND_DEVICE_REASON_MIN));
    TeardownMock();
}

HWTEST_F(ZidlCallbackProxyTest, RunningLockChanged_WriteTokenFail, TestSize.Level1)
{
    SetupMockWriteTokenFail();
    auto proxy = MakeProxy<RunningLockChangedCallbackProxy>();
    ASSERT_TRUE(proxy != nullptr);
    proxy->OnAsyncScreenRunningLockChanged(RunningLockChangeState::RUNNINGLOCK_STATE_LOCKED, UINT64_MAX);
    TeardownMock();
}

HWTEST_F(ZidlCallbackProxyTest, AsyncReply_WriteTokenFail, TestSize.Level1)
{
    SetupMockWriteTokenFail();
    auto proxy = MakeProxy<PowerMgrProxyAsync>();
    ASSERT_TRUE(proxy != nullptr);
    proxy->SendAsyncReply(0);
    TeardownMock();
    proxy->SendAsyncReply(0);
}

HWTEST_F(ZidlCallbackProxyTest, AsyncShutdown_WriteTokenFail, TestSize.Level1)
{
    SetupMockWriteTokenFail();
    auto proxy = MakeProxy<AsyncShutdownCallbackProxy>();
    ASSERT_TRUE(proxy != nullptr);
    proxy->OnAsyncShutdown();
    TeardownMock();
}

HWTEST_F(ZidlCallbackProxyTest, AsyncShutdown_OrReboot_WriteTokenFail, TestSize.Level1)
{
    SetupMockWriteTokenFail();
    auto proxy = MakeProxy<AsyncShutdownCallbackProxy>();
    ASSERT_TRUE(proxy != nullptr);
    proxy->OnAsyncShutdownOrReboot(true);
    TeardownMock();
}

HWTEST_F(ZidlCallbackProxyTest, SyncShutdown_WriteTokenFail, TestSize.Level1)
{
    SetupMockWriteTokenFail();
    auto proxy = MakeProxy<SyncShutdownCallbackProxy>();
    ASSERT_TRUE(proxy != nullptr);
    proxy->OnSyncShutdown();
    TeardownMock();
}

HWTEST_F(ZidlCallbackProxyTest, SyncShutdown_OrReboot_WriteTokenFail, TestSize.Level1)
{
    SetupMockWriteTokenFail();
    auto proxy = MakeProxy<SyncShutdownCallbackProxy>();
    ASSERT_TRUE(proxy != nullptr);
    proxy->OnSyncShutdownOrReboot(true);
    TeardownMock();
}

HWTEST_F(ZidlCallbackProxyTest, TakeOverShutdown_WriteTokenFail, TestSize.Level1)
{
    SetupMockWriteTokenFail();
    auto proxy = MakeProxy<TakeOverShutdownCallbackProxy>();
    TakeOverInfo info;
    EXPECT_FALSE(proxy->OnTakeOverShutdown(info));
    TeardownMock();
}

HWTEST_F(ZidlCallbackProxyTest, TakeOverHibernate_WriteTokenFail, TestSize.Level1)
{
    SetupMockWriteTokenFail();
    auto proxy = MakeProxy<TakeOverShutdownCallbackProxy>();
    TakeOverInfo info;
    EXPECT_FALSE(proxy->OnTakeOverHibernate(info));
    TeardownMock();
}

// ==================== Parcel Write Failure ====================

HWTEST_F(ZidlCallbackProxyTest, RunningLockChanged_WriteUint32Fail, TestSize.Level1)
{
    SetupMockParcelWriteFail();
    auto proxy = MakeProxy<RunningLockChangedCallbackProxy>();
    ASSERT_TRUE(proxy != nullptr);
    proxy->OnAsyncScreenRunningLockChanged(RunningLockChangeState::RUNNINGLOCK_STATE_LOCKED, UINT64_MAX);
    TeardownMock();
}

HWTEST_F(ZidlCallbackProxyTest, SyncHibernateWakeup_WriteBoolFail, TestSize.Level1)
{
    SetupMockParcelWriteFail();
    auto proxy = MakeProxy<SyncHibernateCallbackProxy>();
    ASSERT_TRUE(proxy != nullptr);
    proxy->OnSyncWakeup(true);
    TeardownMock();
}

// ==================== SendRequest Failure ====================

HWTEST_F(ZidlCallbackProxyTest, PowerMode_SendRequestFail, TestSize.Level1)
{
    SetupMockSendRequestFail();
    auto proxy = MakeProxy<PowerModeCallbackProxy>();
    ASSERT_TRUE(proxy != nullptr);
    proxy->OnPowerModeChanged(PowerMode::NORMAL_MODE);
    TeardownMock();
}

HWTEST_F(ZidlCallbackProxyTest, PowerRunningLock_SendRequestFail, TestSize.Level1)
{
    SetupMockSendRequestFail();
    auto proxy = MakeProxy<PowerRunningLockCallbackProxy>();
    ASSERT_TRUE(proxy != nullptr);
    proxy->HandleRunningLockMessage("test");
    TeardownMock();
}

HWTEST_F(ZidlCallbackProxyTest, PowerState_SendRequestFail, TestSize.Level1)
{
    SetupMockSendRequestFail();
    auto proxy = MakeProxy<PowerStateCallbackProxy>();
    ASSERT_TRUE(proxy != nullptr);
    proxy->OnPowerStateChanged(PowerState::AWAKE);
    TeardownMock();
}

HWTEST_F(ZidlCallbackProxyTest, PowerState_Async_SendRequestFail, TestSize.Level1)
{
    SetupMockSendRequestFail();
    auto proxy = MakeProxy<PowerStateCallbackProxy>();
    ASSERT_TRUE(proxy != nullptr);
    proxy->OnAsyncPowerStateChanged(PowerState::AWAKE);
    TeardownMock();
}

HWTEST_F(ZidlCallbackProxyTest, ScreenOffPre_SendRequestFail, TestSize.Level1)
{
    SetupMockSendRequestFail();
    auto proxy = MakeProxy<ScreenOffPreCallbackProxy>();
    ASSERT_TRUE(proxy != nullptr);
    proxy->OnScreenStateChanged(1);
    TeardownMock();
}

HWTEST_F(ZidlCallbackProxyTest, AsyncUlsr_SendRequestFail, TestSize.Level1)
{
    SetupMockSendRequestFail();
    auto proxy = MakeProxy<UlsrCallbackProxy>();
    ASSERT_TRUE(proxy != nullptr);
    proxy->OnAsyncWakeup();
    TeardownMock();
}

HWTEST_F(ZidlCallbackProxyTest, SyncSleep_SendRequestFail, TestSize.Level1)
{
    SetupMockSendRequestFail();
    auto proxy = MakeProxy<SyncSleepCallbackProxy>();
    ASSERT_TRUE(proxy != nullptr);
    proxy->OnSyncSleep(true);
    TeardownMock();
}

HWTEST_F(ZidlCallbackProxyTest, SyncWakeup_SendRequestFail, TestSize.Level1)
{
    SetupMockSendRequestFail();
    auto proxy = MakeProxy<SyncSleepCallbackProxy>();
    ASSERT_TRUE(proxy != nullptr);
    proxy->OnSyncWakeup(true);
    TeardownMock();
}

HWTEST_F(ZidlCallbackProxyTest, SyncHibernate_SendRequestFail, TestSize.Level1)
{
    SetupMockSendRequestFail();
    auto proxy = MakeProxy<SyncHibernateCallbackProxy>();
    ASSERT_TRUE(proxy != nullptr);
    proxy->OnSyncHibernate();
    TeardownMock();
}

HWTEST_F(ZidlCallbackProxyTest, SyncHibernateWakeup_SendRequestFail, TestSize.Level1)
{
    SetupMockSendRequestFail();
    auto proxy = MakeProxy<SyncHibernateCallbackProxy>();
    ASSERT_TRUE(proxy != nullptr);
    proxy->OnSyncWakeup(true);
    TeardownMock();
}

HWTEST_F(ZidlCallbackProxyTest, TakeOverSuspend_SendRequestFail, TestSize.Level1)
{
    SetupMockSendRequestFail();
    auto proxy = MakeProxy<TakeOverSuspendCallbackProxy>();
    EXPECT_FALSE(proxy->OnTakeOverSuspend(SuspendDeviceType::SUSPEND_DEVICE_REASON_MIN));
    TeardownMock();
}

HWTEST_F(ZidlCallbackProxyTest, RunningLockChanged_SendRequestFail, TestSize.Level1)
{
    SetupMockSendRequestFail();
    auto proxy = MakeProxy<RunningLockChangedCallbackProxy>();
    ASSERT_TRUE(proxy != nullptr);
    proxy->OnAsyncScreenRunningLockChanged(RunningLockChangeState::RUNNINGLOCK_STATE_LOCKED, UINT64_MAX);
    TeardownMock();
}

HWTEST_F(ZidlCallbackProxyTest, AsyncShutdown_SendRequestFail, TestSize.Level1)
{
    SetupMockSendRequestFail();
    auto proxy = MakeProxy<AsyncShutdownCallbackProxy>();
    ASSERT_TRUE(proxy != nullptr);
    proxy->OnAsyncShutdown();
    TeardownMock();
}

HWTEST_F(ZidlCallbackProxyTest, AsyncShutdown_OrReboot_SendRequestFail, TestSize.Level1)
{
    SetupMockSendRequestFail();
    auto proxy = MakeProxy<AsyncShutdownCallbackProxy>();
    ASSERT_TRUE(proxy != nullptr);
    proxy->OnAsyncShutdownOrReboot(true);
    TeardownMock();
}

HWTEST_F(ZidlCallbackProxyTest, SyncShutdown_SendRequestFail, TestSize.Level1)
{
    SetupMockSendRequestFail();
    auto proxy = MakeProxy<SyncShutdownCallbackProxy>();
    ASSERT_TRUE(proxy != nullptr);
    proxy->OnSyncShutdown();
    TeardownMock();
}

HWTEST_F(ZidlCallbackProxyTest, SyncShutdown_OrReboot_SendRequestFail, TestSize.Level1)
{
    SetupMockSendRequestFail();
    auto proxy = MakeProxy<SyncShutdownCallbackProxy>();
    ASSERT_TRUE(proxy != nullptr);
    proxy->OnSyncShutdownOrReboot(true);
    TeardownMock();
}

HWTEST_F(ZidlCallbackProxyTest, TakeOverShutdown_SendRequestFail, TestSize.Level1)
{
    SetupMockSendRequestFail();
    auto proxy = MakeProxy<TakeOverShutdownCallbackProxy>();
    TakeOverInfo info;
    EXPECT_FALSE(proxy->OnTakeOverShutdown(info));
    TeardownMock();
}

HWTEST_F(ZidlCallbackProxyTest, TakeOverHibernate_SendRequestFail, TestSize.Level1)
{
    SetupMockSendRequestFail();
    auto proxy = MakeProxy<TakeOverShutdownCallbackProxy>();
    TakeOverInfo info;
    EXPECT_FALSE(proxy->OnTakeOverHibernate(info));
    TeardownMock();
}

// ==================== Null Remote ====================

HWTEST_F(ZidlCallbackProxyTest, TakeOverSuspend_NullRemote, TestSize.Level1)
{
    sptr<TakeOverSuspendCallbackProxy> proxy = new TakeOverSuspendCallbackProxy(nullptr);
    EXPECT_TRUE(proxy != nullptr);
    EXPECT_FALSE(proxy->OnTakeOverSuspend(SuspendDeviceType::SUSPEND_DEVICE_REASON_MIN));
}

HWTEST_F(ZidlCallbackProxyTest, AsyncUlsr_NullRemote, TestSize.Level1)
{
    sptr<UlsrCallbackProxy> proxy = new UlsrCallbackProxy(nullptr);
    EXPECT_TRUE(proxy != nullptr);
    proxy->OnAsyncWakeup();
}

// ==================== Normal Success Path (ret == ERR_OK) ====================

HWTEST_F(ZidlCallbackProxyTest, PowerMode_Normal, TestSize.Level1)
{
    SetupMockNormal();
    auto proxy = MakeProxy<PowerModeCallbackProxy>();
    ASSERT_TRUE(proxy != nullptr);
    proxy->OnPowerModeChanged(PowerMode::NORMAL_MODE);
    TeardownMock();
}

HWTEST_F(ZidlCallbackProxyTest, PowerRunningLock_Normal, TestSize.Level1)
{
    SetupMockNormal();
    auto proxy = MakeProxy<PowerRunningLockCallbackProxy>();
    ASSERT_TRUE(proxy != nullptr);
    proxy->HandleRunningLockMessage("test");
    TeardownMock();
}

HWTEST_F(ZidlCallbackProxyTest, PowerState_Normal, TestSize.Level1)
{
    SetupMockNormal();
    auto proxy = MakeProxy<PowerStateCallbackProxy>();
    ASSERT_TRUE(proxy != nullptr);
    proxy->OnPowerStateChanged(PowerState::AWAKE);
    TeardownMock();
}

HWTEST_F(ZidlCallbackProxyTest, PowerState_Async_Normal, TestSize.Level1)
{
    SetupMockNormal();
    auto proxy = MakeProxy<PowerStateCallbackProxy>();
    ASSERT_TRUE(proxy != nullptr);
    proxy->OnAsyncPowerStateChanged(PowerState::AWAKE);
    TeardownMock();
}

HWTEST_F(ZidlCallbackProxyTest, ScreenOffPre_Normal, TestSize.Level1)
{
    SetupMockNormal();
    auto proxy = MakeProxy<ScreenOffPreCallbackProxy>();
    ASSERT_TRUE(proxy != nullptr);
    proxy->OnScreenStateChanged(1);
    TeardownMock();
}

HWTEST_F(ZidlCallbackProxyTest, AsyncUlsr_Normal, TestSize.Level1)
{
    SetupMockNormal();
    auto proxy = MakeProxy<UlsrCallbackProxy>();
    ASSERT_TRUE(proxy != nullptr);
    proxy->OnAsyncWakeup();
    TeardownMock();
}

HWTEST_F(ZidlCallbackProxyTest, SyncSleep_Normal, TestSize.Level1)
{
    SetupMockNormal();
    auto proxy = MakeProxy<SyncSleepCallbackProxy>();
    ASSERT_TRUE(proxy != nullptr);
    proxy->OnSyncSleep(true);
    TeardownMock();
}

HWTEST_F(ZidlCallbackProxyTest, SyncWakeup_Normal, TestSize.Level1)
{
    SetupMockNormal();
    auto proxy = MakeProxy<SyncSleepCallbackProxy>();
    ASSERT_TRUE(proxy != nullptr);
    proxy->OnSyncWakeup(true);
    TeardownMock();
}

HWTEST_F(ZidlCallbackProxyTest, SyncHibernate_Normal, TestSize.Level1)
{
    SetupMockNormal();
    auto proxy = MakeProxy<SyncHibernateCallbackProxy>();
    ASSERT_TRUE(proxy != nullptr);
    proxy->OnSyncHibernate();
    TeardownMock();
}

HWTEST_F(ZidlCallbackProxyTest, SyncHibernateWakeup_Normal, TestSize.Level1)
{
    SetupMockNormal();
    auto proxy = MakeProxy<SyncHibernateCallbackProxy>();
    ASSERT_TRUE(proxy != nullptr);
    proxy->OnSyncWakeup(true);
    TeardownMock();
}

HWTEST_F(ZidlCallbackProxyTest, TakeOverSuspend_Normal, TestSize.Level1)
{
    SetupMockNormal();
    auto proxy = MakeProxy<TakeOverSuspendCallbackProxy>();
    EXPECT_FALSE(proxy->OnTakeOverSuspend(SuspendDeviceType::SUSPEND_DEVICE_REASON_MIN));
    TeardownMock();
}

HWTEST_F(ZidlCallbackProxyTest, RunningLockChanged_Normal, TestSize.Level1)
{
    SetupMockNormal();
    auto proxy = MakeProxy<RunningLockChangedCallbackProxy>();
    ASSERT_TRUE(proxy != nullptr);
    proxy->OnAsyncScreenRunningLockChanged(RunningLockChangeState::RUNNINGLOCK_STATE_LOCKED, UINT64_MAX);
    TeardownMock();
}

HWTEST_F(ZidlCallbackProxyTest, AsyncShutdown_Normal, TestSize.Level1)
{
    SetupMockNormal();
    auto proxy = MakeProxy<AsyncShutdownCallbackProxy>();
    ASSERT_TRUE(proxy != nullptr);
    proxy->OnAsyncShutdown();
    TeardownMock();
}

HWTEST_F(ZidlCallbackProxyTest, AsyncShutdown_OrReboot_Normal, TestSize.Level1)
{
    SetupMockNormal();
    auto proxy = MakeProxy<AsyncShutdownCallbackProxy>();
    ASSERT_TRUE(proxy != nullptr);
    proxy->OnAsyncShutdownOrReboot(true);
    TeardownMock();
}

HWTEST_F(ZidlCallbackProxyTest, SyncShutdown_Normal, TestSize.Level1)
{
    SetupMockNormal();
    auto proxy = MakeProxy<SyncShutdownCallbackProxy>();
    ASSERT_TRUE(proxy != nullptr);
    proxy->OnSyncShutdown();
    TeardownMock();
}

HWTEST_F(ZidlCallbackProxyTest, SyncShutdown_OrReboot_Normal, TestSize.Level1)
{
    SetupMockNormal();
    auto proxy = MakeProxy<SyncShutdownCallbackProxy>();
    ASSERT_TRUE(proxy != nullptr);
    proxy->OnSyncShutdownOrReboot(true);
    TeardownMock();
}

HWTEST_F(ZidlCallbackProxyTest, TakeOverShutdown_Normal, TestSize.Level1)
{
    SetupMockNormal();
    auto proxy = MakeProxy<TakeOverShutdownCallbackProxy>();
    TakeOverInfo info;
    EXPECT_FALSE(proxy->OnTakeOverShutdown(info));
    TeardownMock();
}

HWTEST_F(ZidlCallbackProxyTest, TakeOverHibernate_Normal, TestSize.Level1)
{
    SetupMockNormal();
    auto proxy = MakeProxy<TakeOverShutdownCallbackProxy>();
    TakeOverInfo info;
    EXPECT_FALSE(proxy->OnTakeOverHibernate(info));
    TeardownMock();
}

// ==================== Null Remote (covers RETURN_IF branch) ====================

HWTEST_F(ZidlCallbackProxyTest, PowerMode_NullRemote, TestSize.Level1)
{
    sptr<PowerModeCallbackProxy> proxy = new PowerModeCallbackProxy(nullptr);
    EXPECT_TRUE(proxy != nullptr);
    proxy->OnPowerModeChanged(PowerMode::NORMAL_MODE);
}

HWTEST_F(ZidlCallbackProxyTest, PowerRunningLock_NullRemote, TestSize.Level1)
{
    sptr<PowerRunningLockCallbackProxy> proxy = new PowerRunningLockCallbackProxy(nullptr);
    EXPECT_TRUE(proxy != nullptr);
    proxy->HandleRunningLockMessage("test");
}

HWTEST_F(ZidlCallbackProxyTest, PowerState_NullRemote, TestSize.Level1)
{
    sptr<PowerStateCallbackProxy> proxy = new PowerStateCallbackProxy(nullptr);
    EXPECT_TRUE(proxy != nullptr);
    proxy->OnPowerStateChanged(PowerState::AWAKE);
}

HWTEST_F(ZidlCallbackProxyTest, ScreenOffPre_NullRemote, TestSize.Level1)
{
    sptr<ScreenOffPreCallbackProxy> proxy = new ScreenOffPreCallbackProxy(nullptr);
    EXPECT_TRUE(proxy != nullptr);
    proxy->OnScreenStateChanged(1);
}

HWTEST_F(ZidlCallbackProxyTest, SyncSleep_NullRemote, TestSize.Level1)
{
    sptr<SyncSleepCallbackProxy> proxy = new SyncSleepCallbackProxy(nullptr);
    EXPECT_TRUE(proxy != nullptr);
    proxy->OnSyncSleep(true);
}

HWTEST_F(ZidlCallbackProxyTest, SyncHibernate_NullRemote, TestSize.Level1)
{
    sptr<SyncHibernateCallbackProxy> proxy = new SyncHibernateCallbackProxy(nullptr);
    EXPECT_TRUE(proxy != nullptr);
    proxy->OnSyncHibernate();
}

HWTEST_F(ZidlCallbackProxyTest, RunningLockChanged_NullRemote, TestSize.Level1)
{
    sptr<RunningLockChangedCallbackProxy> proxy = new RunningLockChangedCallbackProxy(nullptr);
    EXPECT_TRUE(proxy != nullptr);
    proxy->OnAsyncScreenRunningLockChanged(RunningLockChangeState::RUNNINGLOCK_STATE_LOCKED, UINT64_MAX);
}

HWTEST_F(ZidlCallbackProxyTest, AsyncShutdown_NullRemote, TestSize.Level1)
{
    sptr<AsyncShutdownCallbackProxy> proxy = new AsyncShutdownCallbackProxy(nullptr);
    EXPECT_TRUE(proxy != nullptr);
    proxy->OnAsyncShutdown();
}

HWTEST_F(ZidlCallbackProxyTest, SyncShutdown_NullRemote, TestSize.Level1)
{
    sptr<SyncShutdownCallbackProxy> proxy = new SyncShutdownCallbackProxy(nullptr);
    EXPECT_TRUE(proxy != nullptr);
    proxy->OnSyncShutdown();
}
