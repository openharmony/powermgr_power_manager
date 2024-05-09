/*
 * Copyright (c) 2016-2023 Huawei Device Co., Ltd.
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

#include "powermgr_service_native_test.h"

#include "power_common.h"
#include "power_mgr_service.h"
#include "power_state_machine.h"
#include "powermgr_service_test_proxy.h"
#include "running_lock_token_stub.h"
#include <datetime_ex.h>
#include <if_system_ability_manager.h>
#include <iostream>
#include <ipc_skeleton.h>
#include <string_ex.h>

using namespace OHOS;
using namespace OHOS::PowerMgr;
using namespace std;
using namespace testing::ext;

namespace {
static sptr<PowerMgrService> g_powerMgrService = nullptr;
static std::shared_ptr<PowerMgrServiceTestProxy> g_powerMgrServiceProxy = nullptr;
constexpr int32_t DISPLAY_POWER_MANAGER_ID = 3308;
const std::string TEST_DEVICE_ID = "test_device_id";
} // namespace

void PowerMgrServiceNativeTest::SetUpTestCase()
{
    constexpr const uint32_t WAIT_INIT_TIME_S = 5;
    g_powerMgrService = DelayedSpSingleton<PowerMgrService>::GetInstance();
    g_powerMgrService->OnStart();
    g_powerMgrService->OnAddSystemAbility(DISPLAY_POWER_MANAGER_ID, TEST_DEVICE_ID);

    if (g_powerMgrServiceProxy == nullptr) {
        g_powerMgrServiceProxy = std::make_shared<PowerMgrServiceTestProxy>(g_powerMgrService);
    }
    g_powerMgrServiceProxy->SuspendDevice(GetTickCount());
    // wait for "SetState for INIT" to be done
    sleep(WAIT_INIT_TIME_S);
}

void PowerMgrServiceNativeTest::TearDownTestCase()
{
    g_powerMgrService->OnStop();
    DelayedSpSingleton<PowerMgrService>::DestroyInstance();
}

void PowerMgrServiceNativeTest::PowerModeTestCallback::OnPowerModeChanged(PowerMode mode)
{
    POWER_HILOGI(LABEL_TEST, "PowerModeTestCallback::OnPowerModeChanged");
}

void PowerMgrServiceNativeTest::PowerStateTestCallback::OnPowerStateChanged(PowerState state)
{
    POWER_HILOGI(LABEL_TEST, "PowerStateTestCallback::OnPowerStateChanged");
}

void PowerMgrServiceNativeTest::PowerRunningLockTestCallback::HandleRunningLockMessage(std::string message)
{
    POWER_HILOGI(LABEL_TEST, "PowerRunningLockTestCallback::HandleRunningLockMessage.");
}

void PowerMgrServiceNativeTest::ScreenOffPreTestCallback::OnScreenStateChanged(uint32_t state)
{
    POWER_HILOGI(LABEL_TEST, "ScreenOffPreTestCallback::OnScreenStateChanged.");
}
namespace {
/**
 * @tc.name: PowerMgrServiceNativeTest001
 * @tc.desc: test RefreshActivity
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest001, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest001::fun is start");
    g_powerMgrServiceProxy->WakeupDevice(GetTickCount());
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), true);
    g_powerMgrServiceProxy->OverrideScreenOffTime(SCREEN_OFF_WAIT_TIME_MS);
    sleep(SCREEN_OFF_WAIT_TIME_S / 2);
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), true);
    g_powerMgrServiceProxy->RefreshActivity(GetTickCount());
    sleep(SCREEN_OFF_WAIT_TIME_S / 2);
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), true);
    g_powerMgrServiceProxy->OverrideScreenOffTime(DEFAULT_SLEEP_TIME);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest001::fun is end");
}

/**
 * @tc.name: PowerMgrServiceNativeTest002
 * @tc.desc: test RefreshActivity
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest002, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest002::fun is start");
    g_powerMgrServiceProxy->WakeupDevice(GetTickCount());
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), true);
    g_powerMgrServiceProxy->OverrideScreenOffTime(SCREEN_OFF_WAIT_TIME_MS);
    sleep(SCREEN_OFF_WAIT_TIME_S / 2);
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), true);
    g_powerMgrServiceProxy->RefreshActivity(GetTickCount(), UserActivityType::USER_ACTIVITY_TYPE_BUTTON);
    sleep(SCREEN_OFF_WAIT_TIME_S / 2);
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), true);
    g_powerMgrServiceProxy->OverrideScreenOffTime(DEFAULT_SLEEP_TIME);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest002::fun is end");
}

/**
 * @tc.name: PowerMgrServiceNativeTest003
 * @tc.desc: test RefreshActivity
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest003, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest003::fun is start");
    g_powerMgrServiceProxy->WakeupDevice(GetTickCount());
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), true);
    g_powerMgrServiceProxy->OverrideScreenOffTime(SCREEN_OFF_WAIT_TIME_MS);
    sleep(SCREEN_OFF_WAIT_TIME_S / 2);
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), true);
    g_powerMgrServiceProxy->RefreshActivity(GetTickCount(), UserActivityType::USER_ACTIVITY_TYPE_TOUCH);
    sleep(SCREEN_OFF_WAIT_TIME_S / 2);
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), true);
    g_powerMgrServiceProxy->OverrideScreenOffTime(DEFAULT_SLEEP_TIME);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest003::fun is end");
}

/**
 * @tc.name: PowerMgrServiceNativeTest004
 * @tc.desc: test RefreshActivity
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest004, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest004::fun is start");
    g_powerMgrServiceProxy->WakeupDevice(GetTickCount());
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), true);
    g_powerMgrServiceProxy->OverrideScreenOffTime(SCREEN_OFF_WAIT_TIME_MS);
    sleep(SCREEN_OFF_WAIT_TIME_S / 2);
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), true);
    g_powerMgrServiceProxy->RefreshActivity(GetTickCount(), UserActivityType::USER_ACTIVITY_TYPE_ACCESSIBILITY);
    sleep(SCREEN_OFF_WAIT_TIME_S / 2);
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), true);
    g_powerMgrServiceProxy->OverrideScreenOffTime(DEFAULT_SLEEP_TIME);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest004::fun is end");
}

/**
 * @tc.name: PowerMgrServiceNativeTest005
 * @tc.desc: test RefreshActivity
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest005, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest005::fun is start");
    g_powerMgrServiceProxy->WakeupDevice(GetTickCount());
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), true);
    g_powerMgrServiceProxy->OverrideScreenOffTime(SCREEN_OFF_WAIT_TIME_MS);
    sleep(SCREEN_OFF_WAIT_TIME_S / 2);
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), true);
    g_powerMgrServiceProxy->RefreshActivity(GetTickCount(), UserActivityType::USER_ACTIVITY_TYPE_ATTENTION);
    sleep(SCREEN_OFF_WAIT_TIME_S / 2);
    EXPECT_EQ(g_powerMgrServiceProxy->GetState(), PowerState::AWAKE);

    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), true);
    g_powerMgrServiceProxy->OverrideScreenOffTime(DEFAULT_SLEEP_TIME);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest005::fun is end");
}

/**
 * @tc.name: PowerMgrServiceNativeTest006
 * @tc.desc: test RefreshActivity
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest006, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest006::fun is start");
    g_powerMgrServiceProxy->WakeupDevice(GetTickCount());
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), true);
    g_powerMgrServiceProxy->OverrideScreenOffTime(SCREEN_OFF_WAIT_TIME_MS);
    sleep(SCREEN_OFF_WAIT_TIME_S / 2);
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), true);
    g_powerMgrServiceProxy->RefreshActivity(GetTickCount(), UserActivityType::USER_ACTIVITY_TYPE_SOFTWARE);
    sleep(SCREEN_OFF_WAIT_TIME_S / 2);
    EXPECT_EQ(g_powerMgrServiceProxy->GetState(), PowerState::AWAKE);

    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), true);
    g_powerMgrServiceProxy->OverrideScreenOffTime(DEFAULT_SLEEP_TIME);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest006::fun is end");
}

/**
 * @tc.name: PowerMgrServiceNativeTest007
 * @tc.desc: test RefreshActivity
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest007, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest007::fun is start");
    UserActivityType abnormaltype = UserActivityType(9);
    g_powerMgrServiceProxy->WakeupDevice(GetTickCount());
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), true);
    g_powerMgrServiceProxy->OverrideScreenOffTime(SCREEN_OFF_WAIT_TIME_MS);
    usleep(SCREEN_OFF_WAIT_TIME_S * TRANSFER_NS_TO_MS / 2);
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), true);
    g_powerMgrServiceProxy->RefreshActivity(GetTickCount(), abnormaltype);
    usleep(SCREEN_OFF_WAIT_TIME_S * TRANSFER_NS_TO_MS / 2);
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), true);
    g_powerMgrServiceProxy->OverrideScreenOffTime(DEFAULT_SLEEP_TIME);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest007::fun is end");
}

/**
 * @tc.name: PowerMgrServiceNativeTest008
 * @tc.desc: test WakeupDevice
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest008, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest008::fun is start");
    int32_t wakeupReason = (static_cast<int32_t>(WakeupDeviceType::WAKEUP_DEVICE_MAX)) + 1;
    WakeupDeviceType abnormaltype = WakeupDeviceType(wakeupReason);

    g_powerMgrServiceProxy->SuspendDevice(GetTickCount());
    sleep(SLEEP_WAIT_TIME_S);
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), false);
    g_powerMgrServiceProxy->WakeupDevice(GetTickCount(), abnormaltype);
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), false);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest008::fun is end");
}

/**
 * @tc.name: PowerMgrServiceNativeTest009
 * @tc.desc: test Suspend Device
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest009, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest009::fun is start");
    int32_t suspendReason = (static_cast<int32_t>(SuspendDeviceType::SUSPEND_DEVICE_REASON_MAX)) + 1;
    SuspendDeviceType abnormaltype = SuspendDeviceType(suspendReason);

    g_powerMgrServiceProxy->WakeupDevice(GetTickCount());
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), true);
    g_powerMgrServiceProxy->SuspendDevice(GetTickCount(), abnormaltype, false);
    EXPECT_EQ(g_powerMgrServiceProxy->GetState(), PowerState::AWAKE);

    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), true);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest009::fun is end");
}

/**
 * @tc.name: PowerMgrServiceNativeTest010
 * @tc.desc: test SCREEN_ON RunningLock
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest010, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest010:Start");
    g_powerMgrServiceProxy->WakeupDevice(GetTickCount());
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), true);
    int32_t time = SLEEP_WAIT_TIME_MS;
    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo runningLockInfo;
    runningLockInfo.name = "runninglock";
    runningLockInfo.type = RunningLockType::RUNNINGLOCK_SCREEN;
    int32_t timeOutMs = 0;
    pid_t uid = 0;
    pid_t pid = 0;
    auto error = g_powerMgrServiceProxy->CreateRunningLock(token, runningLockInfo);
    EXPECT_TRUE(error == PowerErrors::ERR_OK);
    EXPECT_FALSE(g_powerMgrServiceProxy->ProxyRunningLock(true, pid, uid));
    EXPECT_TRUE(g_powerMgrServiceProxy->ProxyRunningLocks(true, {std::make_pair(pid, uid)}));
    EXPECT_TRUE(g_powerMgrServiceProxy->ResetRunningLocks());
    g_powerMgrServiceProxy->OverrideScreenOffTime(time);
    g_powerMgrServiceProxy->Lock(token);
    EXPECT_EQ(g_powerMgrServiceProxy->IsUsed(token), true);
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), true);
    usleep(time * TRANSFER_MS_TO_S);
    EXPECT_EQ(g_powerMgrServiceProxy->GetState(), PowerState::AWAKE);
    g_powerMgrServiceProxy->UnLock(token);
    EXPECT_EQ(g_powerMgrServiceProxy->IsUsed(token), false);
    g_powerMgrServiceProxy->OverrideScreenOffTime(DEFAULT_SLEEP_TIME);
    EXPECT_TRUE(g_powerMgrServiceProxy->ReleaseRunningLock(token));
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest010:End");
}

/**
 * @tc.name: PowerMgrServiceNativeTest011
 * @tc.desc: test SCREEN_ON RunningLock
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest011, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest011:Start");
    g_powerMgrServiceProxy->WakeupDevice(GetTickCount());
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), true);
    int32_t time = SLEEP_WAIT_TIME_MS;
    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo runningLockInfo;
    runningLockInfo.name = "runninglock";
    runningLockInfo.type = RunningLockType::RUNNINGLOCK_SCREEN;
    int32_t timeOutMs = 0;
    auto error = g_powerMgrServiceProxy->CreateRunningLock(token, runningLockInfo);
    EXPECT_TRUE(error == PowerErrors::ERR_OK);
    g_powerMgrServiceProxy->OverrideScreenOffTime(time);

    g_powerMgrServiceProxy->Lock(token);
    EXPECT_EQ(g_powerMgrServiceProxy->IsUsed(token), true);
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), true);
    usleep(time * TRANSFER_MS_TO_S * DOUBLE_TIMES);
    EXPECT_EQ(g_powerMgrServiceProxy->GetState(), PowerState::AWAKE);

    g_powerMgrServiceProxy->UnLock(token);
    EXPECT_EQ(g_powerMgrServiceProxy->IsUsed(token), false);
    g_powerMgrServiceProxy->OverrideScreenOffTime(DEFAULT_SLEEP_TIME);
    EXPECT_TRUE(g_powerMgrServiceProxy->ReleaseRunningLock(token));
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest011:End");
}

/**
 * @tc.name: PowerMgrServiceNativeTest012
 * @tc.desc: test SCREEN_ON RunningLock
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest012, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest012:Start");
    g_powerMgrServiceProxy->WakeupDevice(GetTickCount());
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), true);
    int32_t time = SLEEP_WAIT_TIME_MS;
    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo runningLockInfo;
    runningLockInfo.name = "runninglock";
    runningLockInfo.type = RunningLockType::RUNNINGLOCK_SCREEN;
    int32_t timeOutMs = 0;
    auto error = g_powerMgrServiceProxy->CreateRunningLock(token, runningLockInfo);
    EXPECT_TRUE(error == PowerErrors::ERR_OK);
    g_powerMgrServiceProxy->OverrideScreenOffTime(time);

    g_powerMgrServiceProxy->Lock(token);
    EXPECT_EQ(g_powerMgrServiceProxy->IsUsed(token), true);
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), true);
    g_powerMgrServiceProxy->SuspendDevice(GetTickCount());
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), false);

    g_powerMgrServiceProxy->UnLock(token);
    EXPECT_EQ(g_powerMgrServiceProxy->IsUsed(token), false);
    g_powerMgrServiceProxy->OverrideScreenOffTime(DEFAULT_SLEEP_TIME);
    EXPECT_TRUE(g_powerMgrServiceProxy->ReleaseRunningLock(token));
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest012:End");
}

/**
 * @tc.name: PowerMgrServiceNativeTest015
 * @tc.desc: test SetDisplaySuspend
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest015, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest015::fun is start");
    g_powerMgrServiceProxy->SuspendDevice(GetTickCount());
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), false);
    g_powerMgrServiceProxy->SetDisplaySuspend(true);

    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), false);
    g_powerMgrServiceProxy->SetDisplaySuspend(false);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest015::fun is end");
}

/**
 * @tc.name: PowerMgrServiceNativeTest016
 * @tc.desc: test Suspend Device in proxy
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest016, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest016::fun is start");
    g_powerMgrServiceProxy->WakeupDevice(GetTickCount());
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), true);

    g_powerMgrServiceProxy->SuspendDevice(GetTickCount());
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), false);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest016::fun is end");
}

/**
 * @tc.name: PowerMgrServiceNativeTest017
 * @tc.desc: test Suspend Device
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest017, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest017::fun is start");
    g_powerMgrServiceProxy->WakeupDevice(GetTickCount());
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), true);

    g_powerMgrServiceProxy->SuspendDevice(GetTickCount(), SuspendDeviceType::SUSPEND_DEVICE_REASON_DEVICE_ADMIN, false);
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), false);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest017::fun is end");
}

/**
 * @tc.name: PowerMgrServiceNativeTest018
 * @tc.desc: test Suspend Device
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest018, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest018::fun is start");
    g_powerMgrServiceProxy->WakeupDevice(GetTickCount());
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), true);

    g_powerMgrServiceProxy->SuspendDevice(GetTickCount(), SuspendDeviceType::SUSPEND_DEVICE_REASON_TIMEOUT, false);
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), false);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest018::fun is end");
}

/**
 * @tc.name: PowerMgrServiceNativeTest019
 * @tc.desc: test Suspend Device
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest019, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest019::fun is start");
    g_powerMgrServiceProxy->WakeupDevice(GetTickCount());
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), true);

    g_powerMgrServiceProxy->SuspendDevice(GetTickCount(), SuspendDeviceType::SUSPEND_DEVICE_REASON_LID, false);
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), false);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest019::fun is end");
}

/**
 * @tc.name: PowerMgrServiceNativeTest020
 * @tc.desc: test Suspend Device
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest020, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest020::fun is start");
    g_powerMgrServiceProxy->WakeupDevice(GetTickCount());
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), true);

    g_powerMgrServiceProxy->SuspendDevice(GetTickCount(), SuspendDeviceType::SUSPEND_DEVICE_REASON_POWER_KEY, false);
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), false);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest020::fun is end");
}

/**
 * @tc.name: PowerMgrServiceNativeTest021
 * @tc.desc: test Suspend Device
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest021, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest021::fun is start");
    g_powerMgrServiceProxy->WakeupDevice(GetTickCount());
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), true);

    g_powerMgrServiceProxy->SuspendDevice(GetTickCount(), SuspendDeviceType::SUSPEND_DEVICE_REASON_HDMI, false);
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), false);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest021::fun is end");
}

/**
 * @tc.name: PowerMgrServiceNativeTest022
 * @tc.desc: test Suspend Device
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest022, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest022::fun is start");
    g_powerMgrServiceProxy->WakeupDevice(GetTickCount());
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), true);

    g_powerMgrServiceProxy->SuspendDevice(GetTickCount(), SuspendDeviceType::SUSPEND_DEVICE_REASON_SLEEP_KEY, false);
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), false);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest022::fun is end");
}

/**
 * @tc.name: PowerMgrServiceNativeTest023
 * @tc.desc: test Suspend Device
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest023, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest023::fun is start");
    g_powerMgrServiceProxy->WakeupDevice(GetTickCount());
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), true);

    g_powerMgrServiceProxy->SuspendDevice(
        GetTickCount(), SuspendDeviceType::SUSPEND_DEVICE_REASON_ACCESSIBILITY, false);
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), false);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest023::fun is end");
}

/**
 * @tc.name: PowerMgrServiceNativeTest024
 * @tc.desc: test Suspend Device
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest024, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest024::fun is start");
    g_powerMgrServiceProxy->WakeupDevice(GetTickCount());
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), true);

    g_powerMgrServiceProxy->SuspendDevice(
        GetTickCount(), SuspendDeviceType::SUSPEND_DEVICE_REASON_FORCE_SUSPEND, false);
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), false);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest024::fun is end");
}

/**
 * @tc.name: PowerMgrServiceNativeTest025
 * @tc.desc: test WakeupDevice(int64_t timeMs) in proxy
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest025, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest025::fun is start");
    g_powerMgrServiceProxy->SuspendDevice(GetTickCount());
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), false);

    g_powerMgrServiceProxy->WakeupDevice(GetTickCount());
    EXPECT_EQ(g_powerMgrServiceProxy->GetState(), PowerState::AWAKE);
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), true);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest025::fun is end");
}

/**
 * @tc.name: PowerMgrServiceNativeTest026
 * @tc.desc: test WakeupDevice
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest026, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest026::fun is start");
    g_powerMgrServiceProxy->SuspendDevice(GetTickCount());
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), false);

    g_powerMgrServiceProxy->WakeupDevice(GetTickCount(), WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN);
    EXPECT_EQ(g_powerMgrServiceProxy->GetState(), PowerState::AWAKE);
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), true);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest026::fun is end");
}

/**
 * @tc.name: PowerMgrServiceNativeTest027
 * @tc.desc: test WakeupDevice
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest027, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest027::fun is start");
    g_powerMgrServiceProxy->SuspendDevice(GetTickCount());
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), false);

    g_powerMgrServiceProxy->WakeupDevice(GetTickCount(), WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON);
    EXPECT_EQ(g_powerMgrServiceProxy->GetState(), PowerState::AWAKE);
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), true);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest027::fun is end");
}

/**
 * @tc.name: PowerMgrServiceNativeTest028
 * @tc.desc: test WakeupDevice
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest028, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest028::fun is start");
    g_powerMgrServiceProxy->SuspendDevice(GetTickCount());
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), false);

    g_powerMgrServiceProxy->WakeupDevice(GetTickCount(), WakeupDeviceType::WAKEUP_DEVICE_PLUGGED_IN);
    EXPECT_EQ(g_powerMgrServiceProxy->GetState(), PowerState::AWAKE);
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), true);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest028::fun is end");
}

/**
 * @tc.name: PowerMgrServiceNativeTest029
 * @tc.desc: test WakeupDevice
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest029, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest029::fun is start");
    g_powerMgrServiceProxy->SuspendDevice(GetTickCount());
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), false);

    g_powerMgrServiceProxy->WakeupDevice(GetTickCount(), WakeupDeviceType::WAKEUP_DEVICE_GESTURE);
    EXPECT_EQ(g_powerMgrServiceProxy->GetState(), PowerState::AWAKE);
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), true);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest029::fun is end");
}

/**
 * @tc.name: PowerMgrServiceNativeTest030
 * @tc.desc: test WakeupDevice
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest030, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest030::fun is start");
    g_powerMgrServiceProxy->SuspendDevice(GetTickCount());
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), false);

    g_powerMgrServiceProxy->WakeupDevice(GetTickCount(), WakeupDeviceType::WAKEUP_DEVICE_CAMERA_LAUNCH);
    EXPECT_EQ(g_powerMgrServiceProxy->GetState(), PowerState::AWAKE);
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), true);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest030::fun is end");
}

/**
 * @tc.name: PowerMgrServiceNativeTest031
 * @tc.desc: test WakeupDevice
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest031, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest031::fun is start");
    g_powerMgrServiceProxy->SuspendDevice(GetTickCount());
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), false);

    g_powerMgrServiceProxy->WakeupDevice(GetTickCount(), WakeupDeviceType::WAKEUP_DEVICE_WAKE_KEY);
    EXPECT_EQ(g_powerMgrServiceProxy->GetState(), PowerState::AWAKE);
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), true);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest031::fun is end");
}

/**
 * @tc.name: PowerMgrServiceNativeTest032
 * @tc.desc: test WakeupDevice
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest032, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest032::fun is start");
    g_powerMgrServiceProxy->SuspendDevice(GetTickCount());
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), false);

    g_powerMgrServiceProxy->WakeupDevice(GetTickCount(), WakeupDeviceType::WAKEUP_DEVICE_WAKE_MOTION);
    EXPECT_EQ(g_powerMgrServiceProxy->GetState(), PowerState::AWAKE);
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), true);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest032::fun is end");
}

/**
 * @tc.name: PowerMgrServiceNativeTest033
 * @tc.desc: test WakeupDevice
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest033, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest033::fun is start");
    g_powerMgrServiceProxy->SuspendDevice(GetTickCount());
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), false);

    g_powerMgrServiceProxy->WakeupDevice(GetTickCount(), WakeupDeviceType::WAKEUP_DEVICE_HDMI);
    EXPECT_EQ(g_powerMgrServiceProxy->GetState(), PowerState::AWAKE);
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), true);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest033::fun is end");
}

/**
 * @tc.name: PowerMgrServiceNativeTest034
 * @tc.desc: test WakeupDevice
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest034, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest034::fun is start");
    g_powerMgrServiceProxy->SuspendDevice(GetTickCount());
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), false);

    g_powerMgrServiceProxy->WakeupDevice(GetTickCount(), WakeupDeviceType::WAKEUP_DEVICE_LID);
    EXPECT_EQ(g_powerMgrServiceProxy->GetState(), PowerState::AWAKE);
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), true);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest034::fun is end");
}

/**
 * @tc.name: PowerMgrServiceNativeTest035
 * @tc.desc: test WakeupDevice
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest035, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest035::fun is start");
    g_powerMgrServiceProxy->SuspendDevice(GetTickCount());
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), false);

    g_powerMgrServiceProxy->WakeupDevice(GetTickCount(), WakeupDeviceType::WAKEUP_DEVICE_DOUBLE_CLICK);
    EXPECT_EQ(g_powerMgrServiceProxy->GetState(), PowerState::AWAKE);
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), true);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest035::fun is end");
}

/**
 * @tc.name: PowerMgrServiceNativeTest036
 * @tc.desc: test WakeupDevice
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest036, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest036::fun is start");
    g_powerMgrServiceProxy->SuspendDevice(GetTickCount());
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), false);

    g_powerMgrServiceProxy->WakeupDevice(GetTickCount(), WakeupDeviceType::WAKEUP_DEVICE_KEYBOARD);
    EXPECT_EQ(g_powerMgrServiceProxy->GetState(), PowerState::AWAKE);
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), true);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest036::fun is end");
}

/**
 * @tc.name: PowerMgrServiceNativeTest037
 * @tc.desc: test WakeupDevice
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest037, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest037::fun is start");
    g_powerMgrServiceProxy->SuspendDevice(GetTickCount());
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), false);

    g_powerMgrServiceProxy->WakeupDevice(GetTickCount(), WakeupDeviceType::WAKEUP_DEVICE_MOUSE);
    EXPECT_EQ(g_powerMgrServiceProxy->GetState(), PowerState::AWAKE);
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), true);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest037::fun is end");
}

/**
 * @tc.name: PowerMgrServiceNativeTest038
 * @tc.desc: test IsRunningLockTypeSupported
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest038, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest038:Start");
    auto ret = g_powerMgrServiceProxy->IsRunningLockTypeSupported(RunningLockType::RUNNINGLOCK_BUTT);
    EXPECT_EQ(ret, false);
    ret = g_powerMgrServiceProxy->IsRunningLockTypeSupported(RunningLockType::RUNNINGLOCK_SCREEN);
    EXPECT_EQ(ret, true);
    ret = g_powerMgrServiceProxy->IsRunningLockTypeSupported(RunningLockType::RUNNINGLOCK_BACKGROUND);
    EXPECT_EQ(ret, true);
    ret = g_powerMgrServiceProxy->IsRunningLockTypeSupported(RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL);
    EXPECT_EQ(ret, true);
    g_powerMgrServiceProxy->WakeupDevice(GetTickCount());
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), true);
    g_powerMgrServiceProxy->SuspendDevice(GetTickCount());
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), false);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest038:End");
}

/**
 * @tc.name: PowerMgrServiceNativeTest039
 * @tc.desc: test Power service function, callback is not nullptr
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest039, TestSize.Level2)
{
    sptr<IPowerStateCallback> stateCallback = new PowerStateTestCallback();
    sptr<IPowerModeCallback> modeCallback = new PowerModeTestCallback();
    sptr<IPowerRunninglockCallback> RunninglockCallback =new PowerRunningLockTestCallback();
    sptr<IScreenOffPreCallback> screenOffPreCallback = new ScreenOffPreTestCallback();

    EXPECT_TRUE(g_powerMgrServiceProxy->RegisterPowerStateCallback(stateCallback));
    EXPECT_TRUE(g_powerMgrServiceProxy->UnRegisterPowerStateCallback(stateCallback));
    EXPECT_TRUE(g_powerMgrServiceProxy->RegisterPowerModeCallback(modeCallback));
    EXPECT_TRUE(g_powerMgrServiceProxy->UnRegisterPowerModeCallback(modeCallback));
    EXPECT_TRUE(g_powerMgrServiceProxy->RegisterRunningLockCallback(RunninglockCallback));
    EXPECT_TRUE(g_powerMgrServiceProxy->UnRegisterRunningLockCallback(RunninglockCallback));
    EXPECT_TRUE(g_powerMgrServiceProxy->RegisterScreenStateCallback(4000, screenOffPreCallback));
    EXPECT_TRUE(g_powerMgrServiceProxy->UnRegisterScreenStateCallback(screenOffPreCallback));
}

/**
 * @tc.name: PowerMgrServiceNativeTest040
 * @tc.desc: test Power service dump
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest040, TestSize.Level0)
{
    ASSERT_NE(g_powerMgrServiceProxy, nullptr);
    std::vector<std::string> dumpArgsNone {};
    std::vector<std::string> dumpArgsHelp {};
    dumpArgsHelp.push_back("-h");

    std::string expectedDebugInfo;
    expectedDebugInfo.append("Power manager dump options:\n");

    std::string noneDebugInfo = g_powerMgrServiceProxy->ShellDump(dumpArgsNone, dumpArgsNone.size());
    auto noneIndex = noneDebugInfo.find(expectedDebugInfo);
    EXPECT_TRUE(noneIndex != string::npos);

    std::string helpDebugInfo = g_powerMgrServiceProxy->ShellDump(dumpArgsHelp, dumpArgsHelp.size());
    auto helpIndex = helpDebugInfo.find(expectedDebugInfo);
    EXPECT_TRUE(helpIndex != string::npos);
}

/**
 * @tc.name: PowerMgrServiceNativeTest041
 * @tc.desc: test IsStandby
 * @tc.type: FUNC
 * @tc.require: issueI7QHBE
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest041, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest041::fun is start");
    bool standby = false;
    auto error = g_powerMgrServiceProxy->IsStandby(standby);
    EXPECT_TRUE(error == PowerErrors::ERR_OK);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest041::fun is end");
}

/**
 * @tc.name: PowerMgrServiceNativeTest042
 * @tc.desc: test WakeupDevice
 * @tc.type: FUNC
 * @tc.require: #I9G5XH
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest042, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest042::fun is start");
    g_powerMgrServiceProxy->SuspendDevice(GetTickCount());
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), false);

    g_powerMgrServiceProxy->WakeupDevice(GetTickCount(), WakeupDeviceType::WAKEUP_DEVICE_AOD_SLIDING);
    EXPECT_EQ(g_powerMgrServiceProxy->GetState(), PowerState::AWAKE);
    EXPECT_EQ(g_powerMgrServiceProxy->IsScreenOn(), true);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest042::fun is end");
}
} // namespace
