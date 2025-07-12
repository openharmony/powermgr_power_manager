/*
 * Copyright (c) 2016-2024 Huawei Device Co., Ltd.
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
constexpr int32_t FAIL_VALUE = -1;
constexpr int32_t REASON_VALUE = static_cast<int32_t>(WakeupDeviceType::WAKEUP_DEVICE_APPLICATION);
const std::string API_VERSION = "-1";
const std::string TEST_DEVICE_ID = "test_device_id";
} // namespace

void PowerMgrServiceNativeTest::SetUpTestCase()
{
    constexpr const uint32_t WAIT_INIT_TIME_S = 5;
    g_powerMgrService = DelayedSpSingleton<PowerMgrService>::GetInstance();
    g_powerMgrService->OnStart();
    g_powerMgrService->OnAddSystemAbility(DISPLAY_POWER_MANAGER_ID, TEST_DEVICE_ID);
    int32_t powerError = 1;
    bool isScreenOn = false;

    if (g_powerMgrServiceProxy == nullptr) {
        g_powerMgrServiceProxy = std::make_shared<PowerMgrServiceTestProxy>(g_powerMgrService);
    }
    g_powerMgrServiceProxy->SuspendDeviceIpc(GetTickCount(), REASON_VALUE, false, API_VERSION, powerError);
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

void PowerMgrServiceNativeTest::ScreenOffPreTestCallback::OnScreenStateChanged(uint32_t state)
{
    POWER_HILOGI(LABEL_TEST, "ScreenOffPreTestCallback::OnScreenStateChanged.");
}

void PowerMgrServiceNativeTest::PowerRunningLockTestCallback::HandleRunningLockMessage(std::string message)
{
    POWER_HILOGI(LABEL_TEST, "PowerRunningLockTestCallback::HandleRunningLockMessage.");
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
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest001 function start!");
    int32_t PARM_TWO = 2;
    int32_t powerError = 1;
    bool isScreenOn = false;
    g_powerMgrServiceProxy->WakeupDeviceIpc(
        GetTickCount(), REASON_VALUE, std::string("app call"), API_VERSION, powerError);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, true);
    g_powerMgrServiceProxy->OverrideScreenOffTimeIpc(SCREEN_OFF_WAIT_TIME_MS, powerError);
    sleep(SCREEN_OFF_WAIT_TIME_S / PARM_TWO);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, true);
    g_powerMgrServiceProxy->RefreshActivityIpc(GetTickCount());
    sleep(SCREEN_OFF_WAIT_TIME_S / PARM_TWO);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, true);
    g_powerMgrServiceProxy->OverrideScreenOffTimeIpc(PowerStateMachine::DEFAULT_SLEEP_TIME_MS, powerError);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest001 function end!");
}

/**
 * @tc.name: PowerMgrServiceNativeTest002
 * @tc.desc: test RefreshActivity
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest002, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest002 function start!");
    int32_t PARM_TWO = 2;
    int32_t powerError = 1;
    bool isScreenOn = false;
    g_powerMgrServiceProxy->WakeupDeviceIpc(
        GetTickCount(), REASON_VALUE, std::string("app call"), API_VERSION, powerError);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, true);
    g_powerMgrServiceProxy->OverrideScreenOffTimeIpc(SCREEN_OFF_WAIT_TIME_MS, powerError);
    sleep(SCREEN_OFF_WAIT_TIME_S / PARM_TWO);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, true);
    int32_t userButton = static_cast<int32_t>(UserActivityType::USER_ACTIVITY_TYPE_BUTTON);
    g_powerMgrServiceProxy->RefreshActivityIpc(GetTickCount(), userButton);
    sleep(SCREEN_OFF_WAIT_TIME_S / PARM_TWO);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, true);
    g_powerMgrServiceProxy->OverrideScreenOffTimeIpc(PowerStateMachine::DEFAULT_SLEEP_TIME_MS, powerError);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest002 function end!");
}

/**
 * @tc.name: PowerMgrServiceNativeTest003
 * @tc.desc: test RefreshActivity
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest003, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest003 function start!");
    int32_t PARM_TWO = 2;
    int32_t powerError = 1;
    bool isScreenOn = false;
    g_powerMgrServiceProxy->WakeupDeviceIpc(
        GetTickCount(), REASON_VALUE, std::string("app call"), API_VERSION, powerError);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, true);
    g_powerMgrServiceProxy->OverrideScreenOffTimeIpc(SCREEN_OFF_WAIT_TIME_MS, powerError);
    sleep(SCREEN_OFF_WAIT_TIME_S / PARM_TWO);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, true);
    int32_t userTouch = static_cast<int32_t>(UserActivityType::USER_ACTIVITY_TYPE_TOUCH);
    g_powerMgrServiceProxy->RefreshActivityIpc(GetTickCount(), userTouch);
    sleep(SCREEN_OFF_WAIT_TIME_S / PARM_TWO);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, true);
    g_powerMgrServiceProxy->OverrideScreenOffTimeIpc(PowerStateMachine::DEFAULT_SLEEP_TIME_MS, powerError);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest003 function end!");
}

/**
 * @tc.name: PowerMgrServiceNativeTest004
 * @tc.desc: test RefreshActivity
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest004, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest004 function start!");
    int32_t PARM_TWO = 2;
    int32_t powerError = 1;
    bool isScreenOn = false;
    g_powerMgrServiceProxy->WakeupDeviceIpc(
        GetTickCount(), REASON_VALUE, std::string("app call"), API_VERSION, powerError);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, true);
    g_powerMgrServiceProxy->OverrideScreenOffTimeIpc(SCREEN_OFF_WAIT_TIME_MS, powerError);
    sleep(SCREEN_OFF_WAIT_TIME_S / PARM_TWO);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, true);
    int32_t userAcc = static_cast<int32_t>(UserActivityType::USER_ACTIVITY_TYPE_ACCESSIBILITY);
    g_powerMgrServiceProxy->RefreshActivityIpc(GetTickCount(), userAcc);
    sleep(SCREEN_OFF_WAIT_TIME_S / PARM_TWO);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, true);
    g_powerMgrServiceProxy->OverrideScreenOffTimeIpc(PowerStateMachine::DEFAULT_SLEEP_TIME_MS, powerError);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest004 function end!");
}

/**
 * @tc.name: PowerMgrServiceNativeTest005
 * @tc.desc: test RefreshActivity
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest005, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest005 function start!");
    int32_t PARM_TWO = 2;
    int32_t powerError = 1;
    int32_t powerState = -1;
    bool isScreenOn = false;
    g_powerMgrServiceProxy->WakeupDeviceIpc(
        GetTickCount(), REASON_VALUE, std::string("app call"), API_VERSION, powerError);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, true);
    g_powerMgrServiceProxy->OverrideScreenOffTimeIpc(SCREEN_OFF_WAIT_TIME_MS, powerError);
    sleep(SCREEN_OFF_WAIT_TIME_S / PARM_TWO);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, true);
    int32_t userAttention = static_cast<int32_t>(UserActivityType::USER_ACTIVITY_TYPE_ATTENTION);
    g_powerMgrServiceProxy->RefreshActivityIpc(GetTickCount(), userAttention);
    sleep(SCREEN_OFF_WAIT_TIME_S / PARM_TWO);
    g_powerMgrServiceProxy->GetStateIpc(powerState);
    EXPECT_EQ(powerState, static_cast<int32_t>(PowerState::AWAKE));

    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, true);
    g_powerMgrServiceProxy->OverrideScreenOffTimeIpc(PowerStateMachine::DEFAULT_SLEEP_TIME_MS, powerError);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest005 function end!");
}

/**
 * @tc.name: PowerMgrServiceNativeTest006
 * @tc.desc: test RefreshActivity
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest006, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest006 function start!");
    int32_t PARM_TWO = 2;
    int32_t powerError = 1;
    int32_t powerState = -1;
    bool isScreenOn = false;
    g_powerMgrServiceProxy->WakeupDeviceIpc(
        GetTickCount(), REASON_VALUE, std::string("app call"), API_VERSION, powerError);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, true);
    g_powerMgrServiceProxy->OverrideScreenOffTimeIpc(SCREEN_OFF_WAIT_TIME_MS, powerError);
    sleep(SCREEN_OFF_WAIT_TIME_S / PARM_TWO);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, true);
    int32_t software = static_cast<int32_t>(UserActivityType::USER_ACTIVITY_TYPE_SOFTWARE);
    g_powerMgrServiceProxy->RefreshActivityIpc(GetTickCount(), software);
    sleep(SCREEN_OFF_WAIT_TIME_S / PARM_TWO);
    g_powerMgrServiceProxy->GetStateIpc(powerState);
    EXPECT_EQ(powerState, static_cast<int32_t>(PowerState::AWAKE));

    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, true);
    g_powerMgrServiceProxy->OverrideScreenOffTimeIpc(PowerStateMachine::DEFAULT_SLEEP_TIME_MS, powerError);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest006 function end!");
}

/**
 * @tc.name: PowerMgrServiceNativeTest007
 * @tc.desc: test RefreshActivity
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest007, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest007 function start!");
    int32_t PARM_TWO = 2;
    UserActivityType abnormaltype = UserActivityType(9);
    int32_t powerError = 1;
    bool isScreenOn = false;
    g_powerMgrServiceProxy->WakeupDeviceIpc(
        GetTickCount(), REASON_VALUE, std::string("app call"), API_VERSION, powerError);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, true);
    g_powerMgrServiceProxy->OverrideScreenOffTimeIpc(SCREEN_OFF_WAIT_TIME_MS, powerError);
    usleep(SCREEN_OFF_WAIT_TIME_S * TRANSFER_NS_TO_MS / PARM_TWO);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, true);
    g_powerMgrServiceProxy->RefreshActivityIpc(GetTickCount(), static_cast<int32_t>(abnormaltype));
    usleep(SCREEN_OFF_WAIT_TIME_S * TRANSFER_NS_TO_MS / PARM_TWO);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, true);
    g_powerMgrServiceProxy->OverrideScreenOffTimeIpc(PowerStateMachine::DEFAULT_SLEEP_TIME_MS, powerError);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest007 function end!");
}

/**
 * @tc.name: PowerMgrServiceNativeTest008
 * @tc.desc: test WakeupDevice
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest008, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest008 function start!");
    int32_t wakeupReason = (static_cast<int32_t>(WakeupDeviceType::WAKEUP_DEVICE_MAX)) + 1;
    WakeupDeviceType abnormaltype = WakeupDeviceType(wakeupReason);
    int32_t powerError = 1;
    bool isScreenOn = false;

    g_powerMgrServiceProxy->SuspendDeviceIpc(GetTickCount(), REASON_VALUE, false, API_VERSION, powerError);
    sleep(SLEEP_WAIT_TIME_S);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, false);
    g_powerMgrServiceProxy->WakeupDeviceIpc(GetTickCount(), static_cast<int32_t>(abnormaltype),
        std::string("app call"), API_VERSION, powerError);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, false);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest008 function end!");
}

/**
 * @tc.name: PowerMgrServiceNativeTest009
 * @tc.desc: test Suspend Device
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest009, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest009 function start!");
    int32_t suspendReason = (static_cast<int32_t>(SuspendDeviceType::SUSPEND_DEVICE_REASON_MAX)) + 1;
    int32_t powerError = 1;
    int32_t powerState = -1;
    bool isScreenOn = false;

    g_powerMgrServiceProxy->WakeupDeviceIpc(
        GetTickCount(), REASON_VALUE, std::string("app call"), API_VERSION, powerError);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, true);
    g_powerMgrServiceProxy->SuspendDeviceIpc(GetTickCount(), suspendReason, false, API_VERSION, powerError);
    g_powerMgrServiceProxy->GetStateIpc(powerState);
    EXPECT_EQ(powerState, static_cast<int32_t>(PowerState::AWAKE));

    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, true);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest009 function end!");
}

/**
 * @tc.name: PowerMgrServiceNativeTest010
 * @tc.desc: test SCREEN_ON RunningLock
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest010, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest010 function start!");
    int32_t powerError = 1;
    int32_t powerState = -1;
    std::string name;
    bool isScreenOn = false;
    bool isUsed = false;
    g_powerMgrServiceProxy->WakeupDeviceIpc(
        GetTickCount(), REASON_VALUE, std::string("app call"), API_VERSION, powerError);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, true);
    int32_t time = SLEEP_WAIT_TIME_MS;
    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo runningLockInfo;
    runningLockInfo.name = "runninglock";
    runningLockInfo.type = RunningLockType::RUNNINGLOCK_SCREEN;
    int32_t timeOutMs = 0;
    pid_t uid = 0;
    pid_t pid = 0;
    g_powerMgrServiceProxy->CreateRunningLockIpc(token, runningLockInfo, powerError);
    EXPECT_TRUE(powerError == static_cast<int32_t>(PowerErrors::ERR_OK));
    EXPECT_EQ(g_powerMgrServiceProxy->ProxyRunningLockIpc(true, pid, uid), FAIL_VALUE);
    std::unique_ptr<VectorPair> vectorPairInfos = std::make_unique<VectorPair>();
    std::vector<std::pair<pid_t, pid_t>> processInfos;
    processInfos.emplace_back(pid, uid);
    vectorPairInfos->SetProcessInfos(processInfos);
    EXPECT_EQ(g_powerMgrServiceProxy->ProxyRunningLocksIpc(true, *vectorPairInfos), ERR_OK);
    EXPECT_EQ(g_powerMgrServiceProxy->ResetRunningLocksIpc(), ERR_OK);
    g_powerMgrServiceProxy->OverrideScreenOffTimeIpc(time, powerError);
    g_powerMgrServiceProxy->LockIpc(token, timeOutMs, powerError);
    g_powerMgrServiceProxy->IsUsedIpc(token, isUsed);
    EXPECT_EQ(isUsed, true);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, true);
    usleep(time * TRANSFER_MS_TO_S);
    g_powerMgrServiceProxy->GetStateIpc(powerState);
    EXPECT_EQ(powerState, static_cast<int32_t>(PowerState::AWAKE));
    g_powerMgrServiceProxy->UnLockIpc(token, name, powerError);
    g_powerMgrServiceProxy->IsUsedIpc(token, isUsed);
    EXPECT_EQ(isUsed, false);
    g_powerMgrServiceProxy->OverrideScreenOffTimeIpc(PowerStateMachine::DEFAULT_SLEEP_TIME_MS, powerError);
    EXPECT_EQ(g_powerMgrServiceProxy->ReleaseRunningLockIpc(token), ERR_OK);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest010 function end!");
}

/**
 * @tc.name: PowerMgrServiceNativeTest011
 * @tc.desc: test SCREEN_ON RunningLock
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest011, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest011 function start!");
    int32_t powerError = 1;
    int32_t powerState = -1;
    std::string name;
    bool isScreenOn = false;
    bool isUsed = false;
    g_powerMgrServiceProxy->WakeupDeviceIpc(
        GetTickCount(), REASON_VALUE, std::string("app call"), API_VERSION, powerError);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, true);
    int32_t time = SLEEP_WAIT_TIME_MS;
    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo runningLockInfo;
    runningLockInfo.name = "runninglock";
    runningLockInfo.type = RunningLockType::RUNNINGLOCK_SCREEN;
    int32_t timeOutMs = 0;
    g_powerMgrServiceProxy->CreateRunningLockIpc(token, runningLockInfo, powerError);
    EXPECT_TRUE(powerError == static_cast<int32_t>(PowerErrors::ERR_OK));
    g_powerMgrServiceProxy->OverrideScreenOffTimeIpc(time, powerError);

    g_powerMgrServiceProxy->LockIpc(token, timeOutMs, powerError);
    g_powerMgrServiceProxy->IsUsedIpc(token, isUsed);
    EXPECT_EQ(isUsed, true);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, true);
    usleep(time * TRANSFER_MS_TO_S * DOUBLE_TIMES);
    g_powerMgrServiceProxy->GetStateIpc(powerState);
    EXPECT_EQ(powerState, static_cast<int32_t>(PowerState::AWAKE));

    g_powerMgrServiceProxy->UnLockIpc(token, name, powerError);
    g_powerMgrServiceProxy->IsUsedIpc(token, isUsed);
    EXPECT_EQ(isUsed, false);
    g_powerMgrServiceProxy->OverrideScreenOffTimeIpc(PowerStateMachine::DEFAULT_SLEEP_TIME_MS, powerError);
    EXPECT_EQ(g_powerMgrServiceProxy->ReleaseRunningLockIpc(token), ERR_OK);
    sleep(2);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest011 function end!");
}

/**
 * @tc.name: PowerMgrServiceNativeTest012
 * @tc.desc: test SCREEN_ON RunningLock
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest012, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest012 function start!");
    int32_t powerError = 1;
    std::string name;
    bool isScreenOn = false;
    bool isUsed = false;
    g_powerMgrServiceProxy->WakeupDeviceIpc(
        GetTickCount(), REASON_VALUE, std::string("app call"), API_VERSION, powerError);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, true);
    int32_t time = SLEEP_WAIT_TIME_MS;
    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo runningLockInfo;
    runningLockInfo.name = "runninglock";
    runningLockInfo.type = RunningLockType::RUNNINGLOCK_SCREEN;
    int32_t timeOutMs = 0;
    g_powerMgrServiceProxy->CreateRunningLockIpc(token, runningLockInfo, powerError);
    EXPECT_TRUE(powerError == static_cast<int32_t>(PowerErrors::ERR_OK));
    g_powerMgrServiceProxy->OverrideScreenOffTimeIpc(time, powerError);

    g_powerMgrServiceProxy->LockIpc(token, timeOutMs, powerError);
    g_powerMgrServiceProxy->IsUsedIpc(token, isUsed);
    EXPECT_EQ(isUsed, true);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, true);
    g_powerMgrServiceProxy->SuspendDeviceIpc(GetTickCount(), REASON_VALUE, false, API_VERSION, powerError);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, false);

    g_powerMgrServiceProxy->UnLockIpc(token, name, powerError);
    g_powerMgrServiceProxy->IsUsedIpc(token, isUsed);
    EXPECT_EQ(isUsed, false);
    g_powerMgrServiceProxy->OverrideScreenOffTimeIpc(PowerStateMachine::DEFAULT_SLEEP_TIME_MS, powerError);
    EXPECT_EQ(g_powerMgrServiceProxy->ReleaseRunningLockIpc(token), ERR_OK);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest012 function end!");
}

/**
 * @tc.name: PowerMgrServiceNativeTest013
 * @tc.desc: test SetDisplaySuspend
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest013, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest013 function start!");
    int32_t powerError = 1;
    bool isScreenOn = false;
    g_powerMgrServiceProxy->SuspendDeviceIpc(GetTickCount(), REASON_VALUE, false, API_VERSION, powerError);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, false);
    g_powerMgrServiceProxy->SetDisplaySuspendIpc(true);

    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, false);
    g_powerMgrServiceProxy->SetDisplaySuspendIpc(false);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest013 function end!");
}

/**
 * @tc.name: PowerMgrServiceNativeTest014
 * @tc.desc: test Suspend Device in proxy
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest014, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest014 function start!");
    int32_t powerError = 1;
    bool isScreenOn = false;
    g_powerMgrServiceProxy->WakeupDeviceIpc(
        GetTickCount(), REASON_VALUE, std::string("app call"), API_VERSION, powerError);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, true);

    g_powerMgrServiceProxy->SuspendDeviceIpc(GetTickCount(), REASON_VALUE, false, API_VERSION, powerError);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, false);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest014 function end!");
}

/**
 * @tc.name: PowerMgrServiceNativeTest015
 * @tc.desc: test Suspend Device
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest015, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest015 function start!");
    int32_t powerError = 1;
    bool isScreenOn = false;
    g_powerMgrServiceProxy->WakeupDeviceIpc(
        GetTickCount(), REASON_VALUE, std::string("app call"), API_VERSION, powerError);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, true);

    g_powerMgrServiceProxy->SuspendDeviceIpc(GetTickCount(),
        static_cast<int32_t>(SuspendDeviceType::SUSPEND_DEVICE_REASON_DEVICE_ADMIN), false, API_VERSION, powerError);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, false);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest015 function end!");
}

/**
 * @tc.name: PowerMgrServiceNativeTest016
 * @tc.desc: test Suspend Device
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest016, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest016 function start!");
    int32_t powerError = 1;
    bool isScreenOn = false;
    g_powerMgrServiceProxy->WakeupDeviceIpc(
        GetTickCount(), REASON_VALUE, std::string("app call"), API_VERSION, powerError);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, true);

    g_powerMgrServiceProxy->SuspendDeviceIpc(GetTickCount(),
        static_cast<int32_t>(SuspendDeviceType::SUSPEND_DEVICE_REASON_TIMEOUT), false, API_VERSION, powerError);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, false);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest016 function end!");
}

/**
 * @tc.name: PowerMgrServiceNativeTest017
 * @tc.desc: test Suspend Device
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest017, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest017 function start!");
    int32_t powerError = 1;
    bool isScreenOn = false;
    g_powerMgrServiceProxy->WakeupDeviceIpc(
        GetTickCount(), REASON_VALUE, std::string("app call"), API_VERSION, powerError);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, true);

    g_powerMgrServiceProxy->SuspendDeviceIpc(GetTickCount(),
        static_cast<int32_t>(SuspendDeviceType::SUSPEND_DEVICE_REASON_LID), false, API_VERSION, powerError);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, false);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest017 function end!");
}

/**
 * @tc.name: PowerMgrServiceNativeTest018
 * @tc.desc: test Suspend Device
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest018, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest018 function start!");
    int32_t powerError = 1;
    bool isScreenOn = false;
    g_powerMgrServiceProxy->WakeupDeviceIpc(
        GetTickCount(), REASON_VALUE, std::string("app call"), API_VERSION, powerError);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, true);

    g_powerMgrServiceProxy->SuspendDeviceIpc(GetTickCount(),
        static_cast<int32_t>(SuspendDeviceType::SUSPEND_DEVICE_REASON_POWER_KEY), false, API_VERSION, powerError);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, false);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest018 function end!");
}

/**
 * @tc.name: PowerMgrServiceNativeTest019
 * @tc.desc: test Suspend Device
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest019, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest019 function start!");
    int32_t powerError = 1;
    bool isScreenOn = false;
    g_powerMgrServiceProxy->WakeupDeviceIpc(
        GetTickCount(), REASON_VALUE, std::string("app call"), API_VERSION, powerError);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, true);

    g_powerMgrServiceProxy->SuspendDeviceIpc(GetTickCount(),
        static_cast<int32_t>(SuspendDeviceType::SUSPEND_DEVICE_REASON_HDMI), false, API_VERSION, powerError);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, false);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest019 function end!");
}

/**
 * @tc.name: PowerMgrServiceNativeTest020
 * @tc.desc: test Suspend Device
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest020, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest020 function start!");
    int32_t powerError = 1;
    bool isScreenOn = false;
    g_powerMgrServiceProxy->WakeupDeviceIpc(
        GetTickCount(), REASON_VALUE, std::string("app call"), API_VERSION, powerError);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, true);

    g_powerMgrServiceProxy->SuspendDeviceIpc(GetTickCount(),
        static_cast<int32_t>(SuspendDeviceType::SUSPEND_DEVICE_REASON_SLEEP_KEY), false, API_VERSION, powerError);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, false);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest020 function end!");
}

/**
 * @tc.name: PowerMgrServiceNativeTest021
 * @tc.desc: test Suspend Device
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest021, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest021 function start!");
    int32_t powerError = 1;
    bool isScreenOn = false;
    g_powerMgrServiceProxy->WakeupDeviceIpc(
        GetTickCount(), REASON_VALUE, std::string("app call"), API_VERSION, powerError);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, true);

    int32_t suspendAcc = static_cast<int32_t>(SuspendDeviceType::SUSPEND_DEVICE_REASON_ACCESSIBILITY);
    g_powerMgrServiceProxy->SuspendDeviceIpc(GetTickCount(), suspendAcc, false, API_VERSION, powerError);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, false);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest021 function end!");
}

/**
 * @tc.name: PowerMgrServiceNativeTest022
 * @tc.desc: test Suspend Device
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest022, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest022 function start!");
    int32_t powerError = 1;
    bool isScreenOn = false;
    g_powerMgrServiceProxy->WakeupDeviceIpc(
        GetTickCount(), REASON_VALUE, std::string("app call"), API_VERSION, powerError);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, true);

    g_powerMgrServiceProxy->SuspendDeviceIpc(GetTickCount(),
        static_cast<int32_t>(SuspendDeviceType::SUSPEND_DEVICE_REASON_FORCE_SUSPEND), false, API_VERSION, powerError);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, false);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest022 function end!");
}

/**
 * @tc.name: PowerMgrServiceNativeTest023
 * @tc.desc: test WakeupDevice(int64_t timeMs) in proxy
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest023, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest023 function start!");
    int32_t powerError = 1;
    int32_t powerState = -1;
    bool isScreenOn = false;
    g_powerMgrServiceProxy->SuspendDeviceIpc(GetTickCount(), REASON_VALUE, false, API_VERSION, powerError);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, false);

    g_powerMgrServiceProxy->WakeupDeviceIpc(
        GetTickCount(), REASON_VALUE, std::string("app call"), API_VERSION, powerError);
    g_powerMgrServiceProxy->GetStateIpc(powerState);
    EXPECT_EQ(powerState, static_cast<int32_t>(PowerState::AWAKE));
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, true);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest023 function end!");
}

/**
 * @tc.name: PowerMgrServiceNativeTest024
 * @tc.desc: test WakeupDevice
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest024, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest024 function start!");
    int32_t powerError = 1;
    int32_t powerState = -1;
    bool isScreenOn = false;
    g_powerMgrServiceProxy->SuspendDeviceIpc(GetTickCount(), REASON_VALUE, false, API_VERSION, powerError);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, false);

    g_powerMgrServiceProxy->WakeupDeviceIpc(GetTickCount(),
        static_cast<int32_t>(WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN),
        std::string("app call"), API_VERSION, powerError);
    g_powerMgrServiceProxy->GetStateIpc(powerState);
    EXPECT_EQ(powerState, static_cast<int32_t>(PowerState::AWAKE));
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, true);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest024 function end!");
}

/**
 * @tc.name: PowerMgrServiceNativeTest025
 * @tc.desc: test WakeupDevice
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest025, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest025 function start!");
    int32_t powerError = 1;
    int32_t powerState = -1;
    bool isScreenOn = false;
    g_powerMgrServiceProxy->SuspendDeviceIpc(GetTickCount(), REASON_VALUE, false, API_VERSION, powerError);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, false);

    g_powerMgrServiceProxy->WakeupDeviceIpc(GetTickCount(),
        static_cast<int32_t>(WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON),
        std::string("app call"), API_VERSION, powerError);
    g_powerMgrServiceProxy->GetStateIpc(powerState);
    EXPECT_EQ(powerState, static_cast<int32_t>(PowerState::AWAKE));
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, true);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest025 function end!");
}

/**
 * @tc.name: PowerMgrServiceNativeTest026
 * @tc.desc: test WakeupDevice
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest026, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest026 function start!");
    int32_t powerError = 1;
    int32_t powerState = -1;
    bool isScreenOn = false;
    g_powerMgrServiceProxy->SuspendDeviceIpc(GetTickCount(), REASON_VALUE, false, API_VERSION, powerError);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, false);

    g_powerMgrServiceProxy->WakeupDeviceIpc(GetTickCount(),
        static_cast<int32_t>(WakeupDeviceType::WAKEUP_DEVICE_PLUGGED_IN),
        std::string("app call"), API_VERSION, powerError);
    g_powerMgrServiceProxy->GetStateIpc(powerState);
    EXPECT_EQ(powerState, static_cast<int32_t>(PowerState::AWAKE));
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, true);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest026 function end!");
}

/**
 * @tc.name: PowerMgrServiceNativeTest027
 * @tc.desc: test WakeupDevice
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest027, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest027 function start!");
    int32_t powerError = 1;
    int32_t powerState = -1;
    bool isScreenOn = false;
    g_powerMgrServiceProxy->SuspendDeviceIpc(GetTickCount(), REASON_VALUE, false, API_VERSION, powerError);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, false);

    g_powerMgrServiceProxy->WakeupDeviceIpc(GetTickCount(),
        static_cast<int32_t>(WakeupDeviceType::WAKEUP_DEVICE_GESTURE),
        std::string("app call"), API_VERSION, powerError);
    g_powerMgrServiceProxy->GetStateIpc(powerState);
    EXPECT_EQ(powerState, static_cast<int32_t>(PowerState::AWAKE));
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, true);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest027 function end!");
}

/**
 * @tc.name: PowerMgrServiceNativeTest028
 * @tc.desc: test WakeupDevice
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest028, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest028 function start!");
    int32_t powerError = 1;
    int32_t powerState = -1;
    bool isScreenOn = false;
    g_powerMgrServiceProxy->SuspendDeviceIpc(GetTickCount(), REASON_VALUE, false, API_VERSION, powerError);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, false);

    g_powerMgrServiceProxy->WakeupDeviceIpc(GetTickCount(),
        static_cast<int32_t>(WakeupDeviceType::WAKEUP_DEVICE_CAMERA_LAUNCH),
        std::string("app call"), API_VERSION, powerError);
    g_powerMgrServiceProxy->GetStateIpc(powerState);
    EXPECT_EQ(powerState, static_cast<int32_t>(PowerState::AWAKE));
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, true);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest028 function end!");
}

/**
 * @tc.name: PowerMgrServiceNativeTest029
 * @tc.desc: test WakeupDevice
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest029, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest029 function start!");
    int32_t powerError = 1;
    int32_t powerState = -1;
    bool isScreenOn = false;
    g_powerMgrServiceProxy->SuspendDeviceIpc(GetTickCount(), REASON_VALUE, false, API_VERSION, powerError);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, false);

    g_powerMgrServiceProxy->WakeupDeviceIpc(GetTickCount(),
        static_cast<int32_t>(WakeupDeviceType::WAKEUP_DEVICE_WAKE_KEY),
        std::string("app call"), API_VERSION, powerError);
    g_powerMgrServiceProxy->GetStateIpc(powerState);
    EXPECT_EQ(powerState, static_cast<int32_t>(PowerState::AWAKE));
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, true);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest029 function end!");
}

/**
 * @tc.name: PowerMgrServiceNativeTest030
 * @tc.desc: test WakeupDevice
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest030, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest030 function start!");
    int32_t powerError = 1;
    int32_t powerState = -1;
    bool isScreenOn = false;
    g_powerMgrServiceProxy->SuspendDeviceIpc(GetTickCount(), REASON_VALUE, false, API_VERSION, powerError);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, false);

    g_powerMgrServiceProxy->WakeupDeviceIpc(GetTickCount(),
        static_cast<int32_t>(WakeupDeviceType::WAKEUP_DEVICE_WAKE_MOTION),
        std::string("app call"), API_VERSION, powerError);
    g_powerMgrServiceProxy->GetStateIpc(powerState);
    EXPECT_EQ(powerState, static_cast<int32_t>(PowerState::AWAKE));
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, true);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest030 function end!");
}

/**
 * @tc.name: PowerMgrServiceNativeTest031
 * @tc.desc: test WakeupDevice
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest031, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest031 function start!");
    int32_t powerError = 1;
    int32_t powerState = -1;
    bool isScreenOn = false;
    g_powerMgrServiceProxy->SuspendDeviceIpc(GetTickCount(), REASON_VALUE, false, API_VERSION, powerError);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, false);

    g_powerMgrServiceProxy->WakeupDeviceIpc(GetTickCount(),
        static_cast<int32_t>(WakeupDeviceType::WAKEUP_DEVICE_HDMI),
        std::string("app call"), API_VERSION, powerError);
    g_powerMgrServiceProxy->GetStateIpc(powerState);
    EXPECT_EQ(powerState, static_cast<int32_t>(PowerState::AWAKE));
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, true);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest031 function end!");
}

/**
 * @tc.name: PowerMgrServiceNativeTest032
 * @tc.desc: test WakeupDevice
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest032, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest032 function start!");
    int32_t powerError = 1;
    int32_t powerState = -1;
    bool isScreenOn = false;
    g_powerMgrServiceProxy->SuspendDeviceIpc(GetTickCount(), REASON_VALUE, false, API_VERSION, powerError);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, false);

    g_powerMgrServiceProxy->WakeupDeviceIpc(GetTickCount(), static_cast<int32_t>(WakeupDeviceType::WAKEUP_DEVICE_LID),
        std::string("app call"), API_VERSION, powerError);
    g_powerMgrServiceProxy->GetStateIpc(powerState);
    EXPECT_EQ(powerState, static_cast<int32_t>(PowerState::AWAKE));
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, true);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest032 function end!");
}

/**
 * @tc.name: PowerMgrServiceNativeTest033
 * @tc.desc: test WakeupDevice
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest033, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest033 function start!");
    int32_t powerError = 1;
    int32_t powerState = -1;
    bool isScreenOn = false;
    g_powerMgrServiceProxy->SuspendDeviceIpc(GetTickCount(), REASON_VALUE, false, API_VERSION, powerError);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, false);

    g_powerMgrServiceProxy->WakeupDeviceIpc(GetTickCount(),
        static_cast<int32_t>(WakeupDeviceType::WAKEUP_DEVICE_DOUBLE_CLICK),
        std::string("app call"), API_VERSION, powerError);
    g_powerMgrServiceProxy->GetStateIpc(powerState);
    EXPECT_EQ(powerState, static_cast<int32_t>(PowerState::AWAKE));
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, true);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest033 function end!");
}

/**
 * @tc.name: PowerMgrServiceNativeTest034
 * @tc.desc: test WakeupDevice
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest034, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest034 function start!");
    int32_t powerError = 1;
    int32_t powerState = -1;
    bool isScreenOn = false;
    g_powerMgrServiceProxy->SuspendDeviceIpc(GetTickCount(), REASON_VALUE, false, API_VERSION, powerError);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, false);

    g_powerMgrServiceProxy->WakeupDeviceIpc(GetTickCount(),
        static_cast<int32_t>(WakeupDeviceType::WAKEUP_DEVICE_KEYBOARD),
        std::string("app call"), API_VERSION, powerError);
    g_powerMgrServiceProxy->GetStateIpc(powerState);
    EXPECT_EQ(powerState, static_cast<int32_t>(PowerState::AWAKE));
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, true);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest034 function end!");
}

/**
 * @tc.name: PowerMgrServiceNativeTest035
 * @tc.desc: test WakeupDevice
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest035, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest035 function start!");
    int32_t powerError = 1;
    int32_t powerState = -1;
    bool isScreenOn = false;
    g_powerMgrServiceProxy->SuspendDeviceIpc(GetTickCount(), REASON_VALUE, false, API_VERSION, powerError);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, false);

    g_powerMgrServiceProxy->WakeupDeviceIpc(GetTickCount(),
        static_cast<int32_t>(WakeupDeviceType::WAKEUP_DEVICE_MOUSE),
        std::string("app call"), API_VERSION, powerError);
    g_powerMgrServiceProxy->GetStateIpc(powerState);
    EXPECT_EQ(powerState, static_cast<int32_t>(PowerState::AWAKE));
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, true);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest035 function end!");
}

/**
 * @tc.name: PowerMgrServiceNativeTest036
 * @tc.desc: test IsRunningLockTypeSupported
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest036, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest036 function start!");
    int32_t powerError = 1;
    bool lockTypeSupported = false;
    bool isScreenOn = false;
    g_powerMgrServiceProxy->IsRunningLockTypeSupportedIpc(
        static_cast<int32_t>(RunningLockType::RUNNINGLOCK_BUTT), lockTypeSupported);
    EXPECT_EQ(lockTypeSupported, false);
    g_powerMgrServiceProxy->IsRunningLockTypeSupportedIpc(
        static_cast<int32_t>(RunningLockType::RUNNINGLOCK_SCREEN), lockTypeSupported);
    EXPECT_EQ(lockTypeSupported, true);
    g_powerMgrServiceProxy->IsRunningLockTypeSupportedIpc(
        static_cast<int32_t>(RunningLockType::RUNNINGLOCK_BACKGROUND), lockTypeSupported);
    EXPECT_EQ(lockTypeSupported, true);
#ifdef HAS_SENSORS_SENSOR_PART
    g_powerMgrServiceProxy->IsRunningLockTypeSupportedIpc(
        static_cast<int32_t>(RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL), lockTypeSupported);
    EXPECT_EQ(lockTypeSupported, true);
#endif
    g_powerMgrServiceProxy->WakeupDeviceIpc(
        GetTickCount(), REASON_VALUE, std::string("app call"), API_VERSION, powerError);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, true);
    g_powerMgrServiceProxy->SuspendDeviceIpc(GetTickCount(), REASON_VALUE, false, API_VERSION, powerError);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, false);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest036 function end!");
}

/**
 * @tc.name: PowerMgrServiceNativeTest037
 * @tc.desc: test Power service function, callback is not nullptr
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest037, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest037 function start!");
    sptr<IPowerStateCallback> stateCallback = new PowerStateTestCallback();
    sptr<IPowerModeCallback> modeCallback = new PowerModeTestCallback();
    sptr<IScreenOffPreCallback> screenOffPreCallback = new ScreenOffPreTestCallback();
    sptr<IPowerRunninglockCallback> RunninglockCallback =new PowerRunningLockTestCallback();

    EXPECT_EQ(g_powerMgrServiceProxy->RegisterPowerStateCallbackIpc(stateCallback), ERR_OK);
    EXPECT_EQ(g_powerMgrServiceProxy->UnRegisterPowerStateCallbackIpc(stateCallback), ERR_OK);
    EXPECT_EQ(g_powerMgrServiceProxy->RegisterPowerModeCallbackIpc(modeCallback), ERR_OK);
    EXPECT_EQ(g_powerMgrServiceProxy->UnRegisterPowerModeCallbackIpc(modeCallback), ERR_OK);
    EXPECT_EQ(g_powerMgrServiceProxy->RegisterScreenStateCallbackIpc(4000, screenOffPreCallback), ERR_OK);
    EXPECT_EQ(g_powerMgrServiceProxy->UnRegisterScreenStateCallbackIpc(screenOffPreCallback), ERR_OK);
    EXPECT_EQ(g_powerMgrServiceProxy->RegisterRunningLockCallbackIpc(RunninglockCallback), ERR_OK);
    EXPECT_EQ(g_powerMgrServiceProxy->UnRegisterRunningLockCallbackIpc(RunninglockCallback), ERR_OK);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest037 function end!");
}

/**
 * @tc.name: PowerMgrServiceNativeTest038
 * @tc.desc: test Power service dump
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest038, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest038 function start!");
    ASSERT_NE(g_powerMgrServiceProxy, nullptr);
    std::vector<std::string> dumpArgsNone {};
    std::vector<std::string> dumpArgsHelp {};
    dumpArgsHelp.push_back("-h");

    std::string expectedDebugInfo;
    expectedDebugInfo.append("Power manager dump options:\n");

    std::string noneDebugInfo;
    g_powerMgrServiceProxy->ShellDumpIpc(dumpArgsNone, dumpArgsNone.size(), noneDebugInfo);
    auto noneIndex = noneDebugInfo.find(expectedDebugInfo);
    EXPECT_TRUE(noneIndex != string::npos);

    std::string helpDebugInfo;
    g_powerMgrServiceProxy->ShellDumpIpc(dumpArgsHelp, dumpArgsHelp.size(), helpDebugInfo);
    auto helpIndex = helpDebugInfo.find(expectedDebugInfo);
    EXPECT_TRUE(helpIndex != string::npos);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest038 function end!");
}

/**
 * @tc.name: PowerMgrServiceNativeTest039
 * @tc.desc: test IsStandby
 * @tc.type: FUNC
 * @tc.require: issueI7QHBE
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest039, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest039 function start!");
    bool standby = false;
    int32_t powerError = 1;
    g_powerMgrServiceProxy->IsStandbyIpc(standby, powerError);
    EXPECT_NE(powerError, static_cast<int32_t>(PowerErrors::ERR_CONNECTION_FAIL));
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest039 function end!");
}

/**
 * @tc.name: PowerMgrServiceNativeTest040
 * @tc.desc: test WakeupDevice
 * @tc.type: FUNC
 * @tc.require: #I9G5XH
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest040, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest040 function start!");
    int32_t powerError = 1;
    int32_t powerState = -1;
    bool isScreenOn = false;
    g_powerMgrServiceProxy->SuspendDeviceIpc(GetTickCount(), REASON_VALUE, false, API_VERSION, powerError);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, false);

    g_powerMgrServiceProxy->WakeupDeviceIpc(GetTickCount(),
        static_cast<int32_t>(WakeupDeviceType::WAKEUP_DEVICE_AOD_SLIDING),
        std::string("app call"), API_VERSION, powerError);
    g_powerMgrServiceProxy->GetStateIpc(powerState);
    EXPECT_EQ(powerState, static_cast<int32_t>(PowerState::AWAKE));
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, true);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest040 function end!");
}

/**
 * @tc.name: PowerMgrServiceNativeTest041
 * @tc.desc: test WakeupDevice
 * @tc.type: FUNC
 * @tc.require: #I9O7I2
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest041, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest041 function start!");
    int32_t powerError = 1;
    int32_t powerState = -1;
    bool isScreenOn = false;
    g_powerMgrServiceProxy->SuspendDeviceIpc(GetTickCount(), REASON_VALUE, false, API_VERSION, powerError);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, false);

    g_powerMgrServiceProxy->WakeupDeviceIpc(GetTickCount(), static_cast<int32_t>(WakeupDeviceType::WAKEUP_DEVICE_PEN),
        std::string("app call"), API_VERSION, powerError);
    g_powerMgrServiceProxy->GetStateIpc(powerState);
    EXPECT_EQ(powerState, static_cast<int32_t>(PowerState::AWAKE));
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_EQ(isScreenOn, true);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest041 function end!");
}

/**
 * @tc.name: PowerMgrServiceNativeTest042
 * @tc.desc: test IsForceSleeping
 * @tc.type: FUNC
 * @tc.require: issueICE3O4
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest042, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest042 function start!");
    int32_t powerError = 1;
    bool ret = true;
    int32_t wakeupReason = (static_cast<int32_t>(WakeupDeviceType::WAKEUP_DEVICE_MAX)) + 1;
    g_powerMgrServiceProxy->WakeupDeviceIpc(
        GetTickCount(), REASON_VALUE, std::string("app call"), API_VERSION, powerError);
    sleep(NEXT_WAIT_TIME_S);
    g_powerMgrServiceProxy->IsForceSleepingIpc(ret);
    EXPECT_EQ(ret, false);

    g_powerMgrServiceProxy->SuspendDeviceIpc(GetTickCount(), REASON_VALUE, false, API_VERSION, powerError);
    sleep(NEXT_WAIT_TIME_S);
    g_powerMgrServiceProxy->IsForceSleepingIpc(ret);
    EXPECT_EQ(ret, false);

    g_powerMgrServiceProxy->WakeupDeviceIpc(
        GetTickCount(), REASON_VALUE, std::string("app call"), API_VERSION, powerError);
    sleep(NEXT_WAIT_TIME_S);
    g_powerMgrServiceProxy->IsForceSleepingIpc(ret);
    EXPECT_EQ(ret, false);
    g_powerMgrServiceProxy->ForceSuspendDeviceIpc(GetTickCount());
    sleep(NEXT_WAIT_TIME_S);
    g_powerMgrServiceProxy->IsForceSleepingIpc(ret);
#ifdef POWER_MANAGER_ENABLE_FORCE_SLEEP_BROADCAST
    EXPECT_EQ(ret, true);
#else
    EXPECT_EQ(ret, false);
#endif
    g_powerMgrServiceProxy->WakeupDeviceIpc(
        GetTickCount(), wakeupReason, std::string("app call"), API_VERSION, powerError);
    sleep(NEXT_WAIT_TIME_S);
    g_powerMgrServiceProxy->IsForceSleepingIpc(ret);
#ifdef POWER_MANAGER_ENABLE_FORCE_SLEEP_BROADCAST
    EXPECT_EQ(ret, true);
#else
    EXPECT_EQ(ret, false);
#endif
    g_powerMgrServiceProxy->WakeupDeviceIpc(
        GetTickCount(), REASON_VALUE, std::string("app call"), API_VERSION, powerError);
    sleep(NEXT_WAIT_TIME_S);
    g_powerMgrServiceProxy->IsForceSleepingIpc(ret);
    EXPECT_EQ(ret, false);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest042 function end!");
}

/**
 * @tc.name: PowerMgrServiceNativeTest043
 * @tc.desc: test ProxyRunningLock function.
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNativeTest043, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest043 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    std::shared_ptr<RunningLock> runningLock = powerMgrClient.CreateRunningLock(
        "PowerMgrServiceNativeTest043", RunningLockType::RUNNINGLOCK_BACKGROUND);
    ASSERT_NE(runningLock, nullptr);

    pid_t curUid = getuid();
    pid_t curPid = getpid();
    runningLock->Lock();
    EXPECT_EQ(g_powerMgrServiceProxy->ProxyRunningLockIpc(true, curPid, curUid), ERR_OK);
    runningLock->UnLock();
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest043 function end!");
}
} // namespace
