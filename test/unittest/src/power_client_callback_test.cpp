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
 * @tc.desc: test SyncHibernateCallback
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrClientCallbackTest, PowerMgrSyncHibernateCallback001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrSyncHibernateCallback001 function start!");
    bool ret = 0;
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    sptr<ISyncHibernateCallback> cb = new PowerSyncHibernateTest1Callback();
    ret = powerMgrClient.RegisterSyncHibernateCallback(cb);
    EXPECT_TRUE(ret);
    ret = 0;
    ret = powerMgrClient.UnRegisterSyncHibernateCallback(cb);
    EXPECT_TRUE(ret);
    POWER_HILOGI(LABEL_TEST, "PowerMgrSyncHibernateCallback001 function end!");
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
}