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

#include "running_lock_changed_callback_proxy_test.h"

#include <gmock/gmock.h>
#include <iremote_object.h>
#include <iremote_proxy.h>
#include <message_parcel.h>
#include <message_option.h>
#include <ipc_skeleton.h>

#include "running_lock_changed_callback_proxy.h"
#include "irunning_lock_changed_callback.h"
#include "running_lock_changed_callback_ipc_interface_code.h"
#include "power_log.h"
#include "power_common.h"
#include "power_mgr_errors.h"
#include "mock_power_remote_object.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;

HWTEST_F(RunningLockChangedCallbackProxyTest,
    OnAsyncScreenRunningLockChanged_SendRequestFailed_LogError, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "OnAsyncScreenRunningLockChanged_SendRequestFailed_LogError start!");

    sptr<MockPowerRemoteObject> mockRemote = new MockPowerRemoteObject();
    MockPowerRemoteObject::SetRequestValue(ERR_INVALID_OPERATION);

    sptr<RunningLockChangedCallbackProxy> proxy = new RunningLockChangedCallbackProxy(mockRemote);

    proxy->OnAsyncScreenRunningLockChanged(RunningLockChangeState::RUNNINGLOCK_STATE_LOCKED);

    EXPECT_TRUE(mockRemote != nullptr);
    EXPECT_TRUE(proxy != nullptr);

    POWER_HILOGI(LABEL_TEST, "OnAsyncScreenRunningLockChanged_SendRequestFailed_LogError end!");
}

HWTEST_F(RunningLockChangedCallbackProxyTest,
    OnAsyncScreenRunningLockChanged_Success_InvokeRemote, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "OnAsyncScreenRunningLockChanged_Success_InvokeRemote start!");

    sptr<MockPowerRemoteObject> mockRemote = new MockPowerRemoteObject();
    MockPowerRemoteObject::SetRequestValue(ERR_OK);

    sptr<RunningLockChangedCallbackProxy> proxy = new RunningLockChangedCallbackProxy(mockRemote);

    proxy->OnAsyncScreenRunningLockChanged(RunningLockChangeState::RUNNINGLOCK_STATE_LOCKED);

    EXPECT_TRUE(mockRemote != nullptr);
    EXPECT_TRUE(proxy != nullptr);

    POWER_HILOGI(LABEL_TEST, "OnAsyncScreenRunningLockChanged_Success_InvokeRemote end!");
}

HWTEST_F(RunningLockChangedCallbackProxyTest,
    OnAsyncScreenRunningLockChanged_UnlockedState_InvokeRemote, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "OnAsyncScreenRunningLockChanged_UnlockedState_InvokeRemote start!");

    sptr<MockPowerRemoteObject> mockRemote = new MockPowerRemoteObject();
    MockPowerRemoteObject::SetRequestValue(ERR_OK);

    sptr<RunningLockChangedCallbackProxy> proxy = new RunningLockChangedCallbackProxy(mockRemote);

    proxy->OnAsyncScreenRunningLockChanged(RunningLockChangeState::RUNNINGLOCK_STATE_UNLOCKED);

    EXPECT_TRUE(mockRemote != nullptr);
    EXPECT_TRUE(proxy != nullptr);

    POWER_HILOGI(LABEL_TEST, "OnAsyncScreenRunningLockChanged_UnlockedState_InvokeRemote end!");
}
