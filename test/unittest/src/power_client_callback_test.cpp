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

#include "power_client_callback_test.h"

#include <iostream>

#include <datetime_ex.h>
#include <gtest/gtest.h>
#include <if_system_ability_manager.h>
#include <ipc_skeleton.h>
#include <string_ex.h>

#include "power_common.h"
#include "power_mgr_client.h"
#include "power_mgr_service.h"
#include "power_state_machine.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

void PowerMgrClientCallbackTest::PowerRunningLockTest1Callback::HandleRunningLockMessage(std::string message)
{
    POWER_HILOGI(
        LABEL_TEST, "PowerRunningLockTest1Callback::HandleRunningLockMessage, message = %{public}s.", message.c_str());
}

void PowerMgrClientCallbackTest::ScreenOffPreTest1Callback::OnScreenStateChanged(uint32_t state)
{
    POWER_HILOGI(LABEL_TEST, "ScreenOffPreTest1Callback::OnScreenStateChanged, state = %{public}u.", state);
}

void PowerMgrClientCallbackTest::PowerSyncHibernateTest1Callback::OnSyncHibernate()
{
    POWER_HILOGI(LABEL_TEST, "PowerSyncHibernateTest1Callback::OnSyncHibernate.");
}

void PowerMgrClientCallbackTest::PowerSyncHibernateTest1Callback::OnSyncWakeup(bool hibernateResult)
{
    POWER_HILOGI(
        LABEL_TEST, "PowerSyncHibernateTest1Callback::OnSyncWakeup, hibernateResult = %{public}d.", hibernateResult);
}

void PowerMgrClientCallbackTest::PowerSyncSleepTest1Callback::OnSyncSleep(bool onForceSleep)
{
    POWER_HILOGI(LABEL_TEST, "PowerSyncSleepTest1Callback::OnSyncSleep, onForceSleep = %{public}d.", onForceSleep);
}

void PowerMgrClientCallbackTest::PowerSyncSleepTest1Callback::OnSyncWakeup(bool onForceSleep)
{
    POWER_HILOGI(LABEL_TEST, "PowerSyncSleepTest1Callback::OnSyncWakeup, onForceSleep = %{public}d.", onForceSleep);
}

void PowerMgrClientCallbackTest::AsyncUlsrTestCallback::OnAsyncWakeup()
{
    POWER_HILOGI(LABEL_TEST, "AsyncUlsrTestCallback::OnAsyncWakeup.");
}

void PowerMgrClientCallbackTest::RunningLockChangedTestCallback::OnAsyncScreenRunningLockChanged(
    RunningLockChangeState state, uint64_t displayId)
{
    POWER_HILOGI(LABEL_TEST,
        "RunningLockChangedTestCallback::OnAsyncScreenRunningLockChanged, state = %{public}d, displayId = "
        "%{public}" PRIu64,
        static_cast<int32_t>(state), displayId);
}

namespace {
/**
 * @tc.name: PowerMgrRunningLockCallback001
 * @tc.desc: test PowerRunninglockCallback
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrClientCallbackTest, PowerMgrRunningLockCallback001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrRunningLockCallback001 function start!");
    bool ret = false;
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    sptr<IPowerRunninglockCallback> cb = new PowerRunningLockTest1Callback();
    ret = powerMgrClient.RegisterRunningLockCallback(cb);
    EXPECT_TRUE(ret);
    ret = false;
    ret = powerMgrClient.UnRegisterRunningLockCallback(cb);
    EXPECT_TRUE(ret);
    POWER_HILOGI(LABEL_TEST, "PowerMgrRunningLockCallback001 function end!");
}

/**
 * @tc.name: PowerMgrScreenOffCallback001
 * @tc.desc: test ScreenOffPreCallback
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrClientCallbackTest, PowerMgrScreenOffCallback001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrScreenOffCallback001 function start!");
    bool ret = 0;
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    sptr<IScreenOffPreCallback> cb = new ScreenOffPreTest1Callback();
    ret = powerMgrClient.RegisterScreenStateCallback(4000, cb);
    EXPECT_TRUE(ret);
    ret = 0;
    ret = powerMgrClient.UnRegisterScreenStateCallback(cb);
    EXPECT_TRUE(ret);
    POWER_HILOGI(LABEL_TEST, "PowerMgrScreenOffCallback001 function end!");
}

/**
 * @tc.name: PowerMgrSyncHibernateCallback001
 * @tc.desc: test SyncHibernateCallback with priority DEFAULT
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrClientCallbackTest, PowerMgrSyncHibernateCallback001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrSyncHibernateCallback001 function start!");
    bool ret = 0;
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    sptr<ISyncHibernateCallback> cb = new PowerSyncHibernateTest1Callback();
    ret = powerMgrClient.RegisterSyncHibernateCallback(cb,
        HibernateCallbackPriority::DEFAULT);
    EXPECT_TRUE(ret);
    ret = 0;
    ret = powerMgrClient.UnRegisterSyncHibernateCallback(cb);
    EXPECT_TRUE(ret);
    POWER_HILOGI(LABEL_TEST, "PowerMgrSyncHibernateCallback001 function end!");
}

/**
 * @tc.name: PowerMgrSyncHibernateCallback002
 * @tc.desc: test RegisterSyncHibernateCallback without priority (overload, default priority)
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrClientCallbackTest, PowerMgrSyncHibernateCallback002, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrSyncHibernateCallback002 function start!");
    bool ret = false;
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    sptr<ISyncHibernateCallback> cb = new PowerSyncHibernateTest1Callback();
    ret = powerMgrClient.RegisterSyncHibernateCallback(cb);
    EXPECT_TRUE(ret);
    ret = false;
    ret = powerMgrClient.UnRegisterSyncHibernateCallback(cb);
    EXPECT_TRUE(ret);
    POWER_HILOGI(LABEL_TEST, "PowerMgrSyncHibernateCallback002 function end!");
}

/**
 * @tc.name: PowerMgrSyncHibernateCallback003
 * @tc.desc: test RegisterSyncHibernateCallback with HIGH priority
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrClientCallbackTest, PowerMgrSyncHibernateCallback003, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrSyncHibernateCallback003 function start!");
    bool ret = false;
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    sptr<ISyncHibernateCallback> cb = new PowerSyncHibernateTest1Callback();
    ret = powerMgrClient.RegisterSyncHibernateCallback(cb,
        HibernateCallbackPriority::HIGH);
    EXPECT_TRUE(ret);
    ret = false;
    ret = powerMgrClient.UnRegisterSyncHibernateCallback(cb);
    EXPECT_TRUE(ret);
    POWER_HILOGI(LABEL_TEST, "PowerMgrSyncHibernateCallback003 function end!");
}

/**
 * @tc.name: PowerMgrSyncHibernateCallback004
 * @tc.desc: test RegisterSyncHibernateCallback with LOW priority
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrClientCallbackTest, PowerMgrSyncHibernateCallback004, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrSyncHibernateCallback004 function start!");
    bool ret = false;
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    sptr<ISyncHibernateCallback> cb = new PowerSyncHibernateTest1Callback();
    ret = powerMgrClient.RegisterSyncHibernateCallback(cb,
        HibernateCallbackPriority::LOW);
    EXPECT_TRUE(ret);
    ret = false;
    ret = powerMgrClient.UnRegisterSyncHibernateCallback(cb);
    EXPECT_TRUE(ret);
    POWER_HILOGI(LABEL_TEST, "PowerMgrSyncHibernateCallback004 function end!");
}

/**
 * @tc.name: PowerMgrSyncHibernateCallback005
 * @tc.desc: test RegisterSyncHibernateCallback nullptr with overload (no priority)
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrClientCallbackTest, PowerMgrSyncHibernateCallback005, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrSyncHibernateCallback005 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    bool ret = powerMgrClient.RegisterSyncHibernateCallback(nullptr);
    EXPECT_FALSE(ret);
    POWER_HILOGI(LABEL_TEST, "PowerMgrSyncHibernateCallback005 function end!");
}

/**
 * @tc.name: PowerMgrSyncHibernateCallback006
 * @tc.desc: test RegisterSyncHibernateCallback nullptr with HIGH priority
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrClientCallbackTest, PowerMgrSyncHibernateCallback006, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrSyncHibernateCallback006 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    bool ret = powerMgrClient.RegisterSyncHibernateCallback(nullptr,
        HibernateCallbackPriority::HIGH);
    EXPECT_FALSE(ret);
    POWER_HILOGI(LABEL_TEST, "PowerMgrSyncHibernateCallback006 function end!");
}

/**
 * @tc.name: PowerMgrSyncHibernateCallback007
 * @tc.desc: test RegisterSyncHibernateCallback nullptr with LOW priority
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrClientCallbackTest, PowerMgrSyncHibernateCallback007, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrSyncHibernateCallback007 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    bool ret = powerMgrClient.RegisterSyncHibernateCallback(nullptr,
        HibernateCallbackPriority::LOW);
    EXPECT_FALSE(ret);
    POWER_HILOGI(LABEL_TEST, "PowerMgrSyncHibernateCallback007 function end!");
}

/**
 * @tc.name: PowerMgrSyncHibernateCallback008
 * @tc.desc: test UnRegisterSyncHibernateCallback nullptr
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrClientCallbackTest, PowerMgrSyncHibernateCallback008, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrSyncHibernateCallback008 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    bool ret = powerMgrClient.UnRegisterSyncHibernateCallback(nullptr);
    EXPECT_FALSE(ret);
    POWER_HILOGI(LABEL_TEST, "PowerMgrSyncHibernateCallback008 function end!");
}

/**
 * @tc.name: PowerMgrSyncHibernateCallback009
 * @tc.desc: test register same callback twice with different priorities
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrClientCallbackTest, PowerMgrSyncHibernateCallback009, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrSyncHibernateCallback009 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    sptr<ISyncHibernateCallback> cb = new PowerSyncHibernateTest1Callback();
    bool ret = powerMgrClient.RegisterSyncHibernateCallback(cb,
        HibernateCallbackPriority::LOW);
    EXPECT_TRUE(ret);

    ret = powerMgrClient.RegisterSyncHibernateCallback(cb,
        HibernateCallbackPriority::HIGH);
    EXPECT_TRUE(ret);
    ret = powerMgrClient.UnRegisterSyncHibernateCallback(cb);
    EXPECT_TRUE(ret);
    POWER_HILOGI(LABEL_TEST, "PowerMgrSyncHibernateCallback009 function end!");
}

/**
 * @tc.name: PowerMgrSyncHibernateCallback010
 * @tc.desc: test register multiple callbacks with different priorities then unregister all
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrClientCallbackTest, PowerMgrSyncHibernateCallback010, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrSyncHibernateCallback010 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    sptr<ISyncHibernateCallback> cbLow = new PowerSyncHibernateTest1Callback();
    sptr<ISyncHibernateCallback> cbDefault = new PowerSyncHibernateTest1Callback();
    sptr<ISyncHibernateCallback> cbHigh = new PowerSyncHibernateTest1Callback();
    bool ret = powerMgrClient.RegisterSyncHibernateCallback(cbLow,
        HibernateCallbackPriority::LOW);
    EXPECT_TRUE(ret);
    ret = powerMgrClient.RegisterSyncHibernateCallback(cbDefault);
    EXPECT_TRUE(ret);
    ret = powerMgrClient.RegisterSyncHibernateCallback(cbHigh,
        HibernateCallbackPriority::HIGH);
    EXPECT_TRUE(ret);
    ret = powerMgrClient.UnRegisterSyncHibernateCallback(cbLow);
    EXPECT_TRUE(ret);
    ret = powerMgrClient.UnRegisterSyncHibernateCallback(cbDefault);
    EXPECT_TRUE(ret);
    ret = powerMgrClient.UnRegisterSyncHibernateCallback(cbHigh);
    EXPECT_TRUE(ret);
    POWER_HILOGI(LABEL_TEST, "PowerMgrSyncHibernateCallback010 function end!");
}

/**
 * @tc.name: PowerMgrSyncHibernateCallback011
 * @tc.desc: test unregister without register
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrClientCallbackTest, PowerMgrSyncHibernateCallback011, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrSyncHibernateCallback011 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    sptr<ISyncHibernateCallback> cb = new PowerSyncHibernateTest1Callback();
    bool ret = powerMgrClient.UnRegisterSyncHibernateCallback(cb);
    EXPECT_TRUE(ret);
    POWER_HILOGI(LABEL_TEST, "PowerMgrSyncHibernateCallback011 function end!");
}

/**
 * @tc.name: PowerMgrSyncHibernateCallback012
 * @tc.desc: test register and unregister multiple times
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrClientCallbackTest, PowerMgrSyncHibernateCallback012, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrSyncHibernateCallback012 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    sptr<ISyncHibernateCallback> cb = new PowerSyncHibernateTest1Callback();
    bool ret = powerMgrClient.RegisterSyncHibernateCallback(cb);
    EXPECT_TRUE(ret);
    ret = powerMgrClient.UnRegisterSyncHibernateCallback(cb);
    EXPECT_TRUE(ret);
    ret = powerMgrClient.RegisterSyncHibernateCallback(cb,
        HibernateCallbackPriority::HIGH);
    EXPECT_TRUE(ret);
    ret = powerMgrClient.UnRegisterSyncHibernateCallback(cb);
    EXPECT_TRUE(ret);
    POWER_HILOGI(LABEL_TEST, "PowerMgrSyncHibernateCallback012 function end!");
}

/**
 * @tc.name: PowerMgrSyncSleepCallback001
 * @tc.desc: test SyncSleepCallback
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrClientCallbackTest, PowerMgrSyncSleepCallback001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrSyncSleepCallback001 function start!");
    bool ret = 0;
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    sptr<ISyncSleepCallback> cb = new PowerSyncSleepTest1Callback();
    ret = powerMgrClient.RegisterSyncSleepCallback(cb, SleepPriority::HIGH);
    EXPECT_TRUE(ret);
    ret = 0;
    ret = powerMgrClient.UnRegisterSyncSleepCallback(cb);
    EXPECT_TRUE(ret);
    POWER_HILOGI(LABEL_TEST, "PowerMgrSyncSleepCallback001 function end!");
}

/**
 * @tc.name: PowerMgrAsyncUlsrCallback001
 * @tc.desc: test AsyncUlsrCallback
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrClientCallbackTest, PowerMgrUlsrCallback001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrAsyncUlsrCallback001 function start!");
    PowerErrors ret = PowerErrors::ERR_OK;
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    sptr<IAsyncUlsrCallback> cb = new AsyncUlsrTestCallback();
    ret = powerMgrClient.RegisterUlsrCallback(cb);
#ifdef POWER_MANAGER_ENABLE_SUSPEND_WITH_TAG
    EXPECT_TRUE(ret == PowerErrors::ERR_OK);
#else
    EXPECT_TRUE(ret != PowerErrors::ERR_OK);
#endif
    ret = powerMgrClient.UnRegisterUlsrCallback(cb);
#ifdef POWER_MANAGER_ENABLE_SUSPEND_WITH_TAG
    EXPECT_TRUE(ret == PowerErrors::ERR_OK);
#else
    EXPECT_TRUE(ret != PowerErrors::ERR_OK);
#endif
    POWER_HILOGI(LABEL_TEST, "PowerMgrSyncSleepCallback001 function end!");
}

#ifdef POWER_MANAGER_ENABLE_MONITOR_RUNNING_LOCK_CHANGE
/**
 * @tc.name: PowerMgrRunningLockChangedCallback001
 * @tc.desc: test RegisterRunningLockChangedCallback and UnRegisterRunningLockChangedCallback
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrClientCallbackTest, PowerMgrRunningLockChangedCallback001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrRunningLockChangedCallback001 function start!");
    PowerErrors ret = PowerErrors::ERR_OK;
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    sptr<IRunningLockChangedCallback> cb = new RunningLockChangedTestCallback();
    ret = powerMgrClient.RegisterRunningLockChangedCallback(cb);
    EXPECT_TRUE(ret == PowerErrors::ERR_OK);
    ret = powerMgrClient.UnRegisterRunningLockChangedCallback(cb);
    EXPECT_TRUE(ret == PowerErrors::ERR_OK);
    POWER_HILOGI(LABEL_TEST, "PowerMgrRunningLockChangedCallback001 function end!");
}

/**
 * @tc.name: PowerMgrRunningLockChangedCallback002
 * @tc.desc: test RegisterRunningLockChangedCallback with null callback
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrClientCallbackTest, PowerMgrRunningLockChangedCallback002, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrRunningLockChangedCallback002 function start!");
    PowerErrors ret = PowerErrors::ERR_OK;
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    ret = powerMgrClient.RegisterRunningLockChangedCallback(nullptr);
    EXPECT_TRUE(ret == PowerErrors::ERR_PARAM_INVALID);
    POWER_HILOGI(LABEL_TEST, "PowerMgrRunningLockChangedCallback002 function end!");
}

/**
 * @tc.name: PowerMgrRunningLockChangedCallback003
 * @tc.desc: test UnRegisterRunningLockChangedCallback with null callback
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrClientCallbackTest, PowerMgrRunningLockChangedCallback003, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrRunningLockChangedCallback003 function start!");
    PowerErrors ret = PowerErrors::ERR_OK;
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    ret = powerMgrClient.UnRegisterRunningLockChangedCallback(nullptr);
    EXPECT_TRUE(ret == PowerErrors::ERR_PARAM_INVALID);
    POWER_HILOGI(LABEL_TEST, "PowerMgrRunningLockChangedCallback003 function end!");
}
#endif
}