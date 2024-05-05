/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "power_mgr_service_test.h"

#include <csignal>
#include <iostream>
#include <thread>

#include <if_system_ability_manager.h>
#include <ipc_skeleton.h>
#include <iservice_registry.h>
#include <string_ex.h>
#include <system_ability_definition.h>

#include "power_common.h"
#include "power_mgr_client.h"
#include "power_mgr_service.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

void PowerMgrServiceTest::SetUpTestCase(void)
{
}

void PowerMgrServiceTest::TearDownTestCase(void)
{
}

void PowerMgrServiceTest::SetUp(void)
{
}

void PowerMgrServiceTest::TearDown(void)
{
}

namespace {
constexpr const int64_t STATE_WAIT_TIME_MS = 300;
constexpr const int64_t STATE_OFF_WAIT_TIME_MS = 2000;
/**
 * @tc.name: PowerMgrService01
 * @tc.desc: Test PowerMgrService service ready.
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceTest, PowerMgrService001, TestSize.Level0)
{
    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    ASSERT_TRUE(sam != nullptr) << "PowerMgrService01 fail to get GetSystemAbilityManager";
    sptr<IRemoteObject> remoteObject_ = sam->CheckSystemAbility(POWER_MANAGER_SERVICE_ID);
    ASSERT_TRUE(remoteObject_ != nullptr) << "GetSystemAbility failed.";
}

/**
 * @tc.name: PowerMgrService002
 * @tc.desc: Test PowerMgrService Start and stop.
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceTest, PowerMgrService002, TestSize.Level0)
{
    if (false) {
        auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
        ASSERT_TRUE(pmsTest_ != nullptr) << "PowerMgrService02 fail to get PowerMgrService";
        ASSERT_TRUE(!pmsTest_->IsServiceReady()) << "SetUpTestCase pmsTest_ is ready";
        pmsTest_->OnStart();
        ASSERT_TRUE(pmsTest_->IsServiceReady()) << "SetUpTestCase pmsTest_ is not ready";
        pmsTest_->OnStop();
        ASSERT_TRUE(!pmsTest_->IsServiceReady()) << "SetUpTestCase pmsTest_ stop fail";
        pmsTest_->OnStart();
        ASSERT_TRUE(pmsTest_->IsServiceReady()) << "SetUpTestCase pmsTest_ is not ready";
        pmsTest_->OnStop();
        ASSERT_TRUE(!pmsTest_->IsServiceReady()) << "SetUpTestCase pmsTest_ stop fail";
    }
}

/**
 * @tc.name: PowerMgrService003
 * @tc.desc: Test overrideScreenOffTime in screenon
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceTest, PowerMgrService003, TestSize.Level0)
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.WakeupDevice();
    EXPECT_TRUE(powerMgrClient.OverrideScreenOffTime(1000));
    sleep(3);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerMgrService003: Prepare Fail, Screen is ON.";
    EXPECT_TRUE(powerMgrClient.RestoreScreenOffTime());
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService003 end.");
}

/**
 * @tc.name: PowerMgrService004
 * @tc.desc: Test overrideScreenOffTime in screenon
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceTest, PowerMgrService004, TestSize.Level0)
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.WakeupDevice();
    EXPECT_FALSE(powerMgrClient.OverrideScreenOffTime(0));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrService004: Prepare Fail, Screen is ON.";

    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService004 end.");
}

/**
 * @tc.name: PowerMgrService005
 * @tc.desc: Test overrideScreenOffTime in screenon
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceTest, PowerMgrService005, TestSize.Level0)
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.WakeupDevice();
    EXPECT_FALSE(powerMgrClient.OverrideScreenOffTime(-1));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrService005: Prepare Fail, Screen is OFF.";
    
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService005 end.");
}

/**
 * @tc.name: PowerMgrService006
 * @tc.desc: Test overrideScreenOffTime in screenoff
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceTest, PowerMgrService006, TestSize.Level0)
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.SuspendDevice();
    EXPECT_TRUE(powerMgrClient.OverrideScreenOffTime(1000));
    powerMgrClient.WakeupDevice();
    sleep(3);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerMgrService006: Prepare Fail, Screen is ON.";
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService006 end.");
}

/**
 * @tc.name: PowerMgrService007
 * @tc.desc: Test overrideScreenOffTime in screenoff
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceTest, PowerMgrService007, TestSize.Level0)
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.SuspendDevice();
    EXPECT_FALSE(powerMgrClient.OverrideScreenOffTime(0));
    powerMgrClient.WakeupDevice();
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrService007: Prepare Fail, Screen is ON.";

    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService007 end.");
}

/**
 * @tc.name: PowerMgrService008
 * @tc.desc: Test overrideScreenOffTime in screenoff
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceTest, PowerMgrService008, TestSize.Level0)
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.SuspendDevice();
    EXPECT_FALSE(powerMgrClient.OverrideScreenOffTime(-1));
    powerMgrClient.WakeupDevice();
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrService008: Prepare Fail, Screen is OFF.";
    
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService008 end.");
}

/**
 * @tc.name: PowerMgrService009
 * @tc.desc: Test restoreScreenOffTime in screenon
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceTest, PowerMgrService009, TestSize.Level0)
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.WakeupDevice();
    EXPECT_TRUE(powerMgrClient.OverrideScreenOffTime(1000));
    EXPECT_TRUE(powerMgrClient.RestoreScreenOffTime());
    sleep(2);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrService009: Prepare Fail, Screen is OFF.";
    
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService009 end.");
}

/**
 * @tc.name: PowerMgrService010
 * @tc.desc: Test restoreScreenOffTime in screenon
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceTest, PowerMgrService010, TestSize.Level0)
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.WakeupDevice();
    EXPECT_FALSE(powerMgrClient.OverrideScreenOffTime(0));
    EXPECT_FALSE(powerMgrClient.RestoreScreenOffTime());
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrService010: Prepare Fail, Screen is OFF.";
    
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService010 end.");
}

/**
 * @tc.name: PowerMgrService011
 * @tc.desc: Test restoreScreenOffTime in screenon
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceTest, PowerMgrService011, TestSize.Level0)
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.WakeupDevice();
    EXPECT_FALSE(powerMgrClient.OverrideScreenOffTime(-1));;
    EXPECT_FALSE(powerMgrClient.RestoreScreenOffTime());
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrService011: Prepare Fail, Screen is OFF.";
    
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService011 end.");
}

/**
 * @tc.name: PowerMgrService012
 * @tc.desc: Test restoreScreenOffTime in screenoff
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceTest, PowerMgrService012, TestSize.Level0)
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.SuspendDevice();
    EXPECT_TRUE(powerMgrClient.OverrideScreenOffTime(1000));
    EXPECT_TRUE(powerMgrClient.RestoreScreenOffTime());
    powerMgrClient.WakeupDevice();
    sleep(2);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrService012: Prepare Fail, Screen is OFF.";
    
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService012 end.");
}

/**
 * @tc.name: PowerMgrService013
 * @tc.desc: Test restoreScreenOffTime in screenoff
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceTest, PowerMgrService013, TestSize.Level0)
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.SuspendDevice();
    EXPECT_FALSE(powerMgrClient.OverrideScreenOffTime(0));
    EXPECT_FALSE(powerMgrClient.RestoreScreenOffTime());
    powerMgrClient.WakeupDevice();
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrService013: Prepare Fail, Screen is OFF.";
    
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService013 end.");
}

/**
 * @tc.name: PowerMgrService014
 * @tc.desc: Test restoreScreenOffTime in screenoff
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceTest, PowerMgrService014, TestSize.Level0)
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.SuspendDevice();
    EXPECT_FALSE(powerMgrClient.OverrideScreenOffTime(-1));
    EXPECT_FALSE(powerMgrClient.RestoreScreenOffTime());
    powerMgrClient.WakeupDevice();
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrService014: Prepare Fail, Screen is OFF.";
    
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService014 end.");
}

/**
 * @tc.name: PowerMgrService015
 * @tc.desc: Test Dump
 * @tc.type: FUNC
 * @tc.require: issueI650CX
 */
HWTEST_F(PowerMgrServiceTest, PowerMgrService015, TestSize.Level2)
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    std::vector<std::string> dumpArgs {};
    std::string expectedDebugInfo = "Power manager dump options";
    std::string actualDebugInfo = powerMgrClient.Dump(dumpArgs);
    auto index = actualDebugInfo.find(expectedDebugInfo);
    EXPECT_TRUE(index != string::npos);
}

/**
 * @tc.name: PowerMgrService016
 * @tc.desc: Test IsStandby
 * @tc.type: FUNC
 * @tc.require: issueI7QHBE
 */
HWTEST_F(PowerMgrServiceTest, PowerMgrService016, TestSize.Level2)
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    bool standby = false;
    PowerErrors ret = powerMgrClient.IsStandby(standby);
    bool testPassed = (ret == PowerErrors::ERR_OK || ret == PowerErrors::ERR_CONNECTION_FAIL);
    EXPECT_TRUE(testPassed);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService016 end.");
}

/**
 * @tc.name: PowerMgrService017
 * @tc.desc: Test QueryRunningLockLists
 * @tc.type: FUNC
 * @tc.require: issueI8FCZA
 */
HWTEST_F(PowerMgrServiceTest, PowerMgrService017, TestSize.Level2)
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    std::map<std::string, RunningLockInfo> runningLockLists;
    bool ret = powerMgrClient.QueryRunningLockLists(runningLockLists);
    EXPECT_EQ(ret, true);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService017 end.");
}

/**
 * @tc.name: PowerMgrService018
 * @tc.desc: Test RunningLock Deconstructor unlock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceTest, PowerMgrService018, TestSize.Level0)
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    auto runningLock1 = powerMgrClient.CreateRunningLock("runninglock1", RunningLockType::RUNNINGLOCK_SCREEN);
    ASSERT_TRUE(runningLock1 != nullptr);
    runningLock1->Lock();
    ASSERT_TRUE(runningLock1->IsUsed()) << "runningLock1->IsUsed() != true";
    runningLock1->UnLock();
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService018 end.");
}

/**
 * @tc.name: PowerMgrService019
 * @tc.desc: Test Pre-light the screen.
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceTest, PowerMgrService019, TestSize.Level0)
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.SuspendDevice();
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false);

    powerMgrClient.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_APPLICATION, "pre_bright");
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false);
    powerMgrClient.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_APPLICATION, "pre_bright_auth_success");
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true);

    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService019 end.");
}

/**
 * @tc.name: PowerMgrService020
 * @tc.desc: Test if the authentication fails, the screen is on.
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceTest, PowerMgrService020, TestSize.Level0)
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.SuspendDevice();
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false);

    powerMgrClient.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_APPLICATION, "pre_bright");
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false);
    PowerErrors ret =
        powerMgrClient.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_APPLICATION, "pre_bright_auth_fail_screen_on");
    EXPECT_EQ(ret, PowerErrors::ERR_OK);

    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService020 end.");
}

/**
 * @tc.name: PowerMgrService021
 * @tc.desc: Test if the authentication fails, the screen is off.
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceTest, PowerMgrService021, TestSize.Level0)
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.SuspendDevice();
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false);

    powerMgrClient.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_APPLICATION, "pre_bright");
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false);
    powerMgrClient.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_APPLICATION, "pre_bright_auth_fail_screen_off");
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false);

    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService021 end.");
}

/**
 * @tc.name: PowerMgrService022
 * @tc.desc: Test PowerMgrService LockScreenAfterTimingOut.
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceTest, PowerMgrService022, TestSize.Level0)
{
    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    ASSERT_TRUE(pmsTest_ != nullptr) << "PowerMgrService022 fail to get PowerMgrService";
    pmsTest_->OnStart();
    auto stateMaschine_ = pmsTest_->GetPowerStateMachine();
    auto runningLockMgr = pmsTest_->GetRunningLockMgr();

    pmsTest_.GetRefPtr()->IncStrongRef(pmsTest_.GetRefPtr());
    RunningLockParam runningLockParam;
    runningLockParam.name = "runninglock_screen_on";
    runningLockParam.type = RunningLockType::RUNNINGLOCK_SCREEN;
    sptr<IRemoteObject> remoteObject = new RunningLockTokenStub();
    EXPECT_TRUE(runningLockMgr->CreateRunningLock(remoteObject, runningLockParam) != nullptr);
    runningLockMgr->Lock(remoteObject);

    EXPECT_EQ(stateMaschine_->GetReasionBySuspendType(SuspendDeviceType::SUSPEND_DEVICE_REASON_TIMEOUT),
        StateChangeReason::STATE_CHANGE_REASON_TIMEOUT);
    pmsTest_->LockScreenAfterTimingOut(true, false);
    EXPECT_EQ(stateMaschine_->GetReasionBySuspendType(SuspendDeviceType::SUSPEND_DEVICE_REASON_TIMEOUT),
        StateChangeReason::STATE_CHANGE_REASON_TIMEOUT);
    pmsTest_->LockScreenAfterTimingOut(false, false);
    EXPECT_EQ(stateMaschine_->GetReasionBySuspendType(SuspendDeviceType::SUSPEND_DEVICE_REASON_TIMEOUT),
        StateChangeReason::STATE_CHANGE_REASON_TIMEOUT_NO_SCREEN_LOCK);
    EXPECT_EQ(stateMaschine_->GetReasionBySuspendType(SuspendDeviceType::SUSPEND_DEVICE_REASON_POWER_KEY),
        StateChangeReason::STATE_CHANGE_REASON_POWER_KEY);
    pmsTest_->LockScreenAfterTimingOut(true, true);
    EXPECT_EQ(stateMaschine_->GetReasionBySuspendType(SuspendDeviceType::SUSPEND_DEVICE_REASON_TIMEOUT),
        StateChangeReason::STATE_CHANGE_REASON_TIMEOUT_NO_SCREEN_LOCK);
    runningLockMgr->UnLock(remoteObject);
    EXPECT_EQ(stateMaschine_->GetReasionBySuspendType(SuspendDeviceType::SUSPEND_DEVICE_REASON_TIMEOUT),
        StateChangeReason::STATE_CHANGE_REASON_TIMEOUT);
    // wait runninglock async task to end, otherwise it will interfere with the next test case
    pmsTest_->OnStop();
    ffrt::wait();
}

/**
 * @tc.name: PowerMgrService023
 * @tc.desc: Test transition to DIM state for Timeout.
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceTest, PowerMgrService023, TestSize.Level0)
{
    constexpr const int64_t screenOffTime = 4000;
    constexpr const int64_t US_PER_MS = 1000;
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.WakeupDevice();
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);
    EXPECT_TRUE(powerMgrClient.OverrideScreenOffTime(screenOffTime));
    // wait till going to DIM
    usleep((screenOffTime - screenOffTime / PowerStateMachine::OFF_TIMEOUT_FACTOR + STATE_WAIT_TIME_MS) * US_PER_MS);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::DIM);
    EXPECT_TRUE(powerMgrClient.RefreshActivity());
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);
    // wait till going to DIM
    usleep((screenOffTime - screenOffTime / PowerStateMachine::OFF_TIMEOUT_FACTOR + STATE_WAIT_TIME_MS) * US_PER_MS);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::DIM);
    // wait till going to INACTIVE
    usleep((screenOffTime / PowerStateMachine::OFF_TIMEOUT_FACTOR + STATE_OFF_WAIT_TIME_MS) *
        US_PER_MS);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::INACTIVE);
}

/**
 * @tc.name: PowerMgrService024
 * @tc.desc: Test multithread refreshing.
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceTest, PowerMgrService024, TestSize.Level0)
{
    constexpr const uint32_t TESTING_DURATION_S = 10;
    constexpr const uint32_t OPERATION_DELAY_US = 500 * 1000;
    constexpr const uint32_t EXTREMELY_SHORT_SCREEN_OFF_TIME_MS = 200;
    constexpr const uint32_t SHORT_SCREEN_OFF_TIME_MS = 800;
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.OverrideScreenOffTime(SHORT_SCREEN_OFF_TIME_MS);
    powerMgrClient.WakeupDevice();
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);
    std::vector<std::thread> refreshThreads;
    bool notified = false;
    auto refreshTask = [&powerMgrClient, &notified]() {
        while (!notified) {
            powerMgrClient.RefreshActivity();
        }
    };

    for (int i = 0; i < 100; i++) {
        refreshThreads.emplace_back(std::thread(refreshTask));
    }

    auto checkingTask = [&powerMgrClient, &notified]() {
        while (!notified) {
            powerMgrClient.SuspendDevice();
            EXPECT_EQ(powerMgrClient.GetState(), PowerState::INACTIVE);
            usleep(OPERATION_DELAY_US);
            powerMgrClient.WakeupDevice();
            EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);
            usleep(OPERATION_DELAY_US);
        }
    };
    // checks whether refresh tasks may unexpectly turn screen on
    std::thread checkingThread(checkingTask);
    sleep(10);
    notified = true;
    for (auto& thread : refreshThreads) {
        thread.join();
    }
    checkingThread.join();
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService024 end.");
}

/**
 * @tc.name: PowerMgrService025
 * @tc.desc: Test StateChangeReason Get
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceTest, PowerMgrService025, TestSize.Level2)
{
    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    ASSERT_TRUE(pmsTest_ != nullptr) << "PowerMgrService025 failed to get PowerMgrService";
    pmsTest_->OnStart();
    auto stateMaschine_ = pmsTest_->GetPowerStateMachine();
    ASSERT_TRUE(stateMaschine_ != nullptr) << "PowerMgrService025 failed to get PowerStateMachine";

    EXPECT_EQ(stateMaschine_->GetReasionBySuspendType(SuspendDeviceType::SUSPEND_DEVICE_REASON_LID),
        StateChangeReason::STATE_CHANGE_REASON_LID);
    EXPECT_EQ(stateMaschine_->GetReasionBySuspendType(SuspendDeviceType::SUSPEND_DEVICE_REASON_SWITCH),
        StateChangeReason::STATE_CHANGE_REASON_SWITCH);
    EXPECT_EQ(stateMaschine_->GetReasionBySuspendType(SuspendDeviceType::SUSPEND_DEVICE_REASON_POWER_KEY),
        StateChangeReason::STATE_CHANGE_REASON_POWER_KEY);
    EXPECT_EQ(stateMaschine_->GetReasonByWakeType(WakeupDeviceType::WAKEUP_DEVICE_LID),
        StateChangeReason::STATE_CHANGE_REASON_LID);
    EXPECT_EQ(stateMaschine_->GetReasonByWakeType(WakeupDeviceType::WAKEUP_DEVICE_SWITCH),
        StateChangeReason::STATE_CHANGE_REASON_SWITCH);
    EXPECT_EQ(stateMaschine_->GetReasonByWakeType(WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON),
        StateChangeReason::STATE_CHANGE_REASON_POWER_KEY);
    EXPECT_EQ(stateMaschine_->GetReasonByWakeType(WakeupDeviceType::WAKEUP_DEVICE_INCOMING_CALL),
        StateChangeReason::STATE_CHANGE_REASON_INCOMING_CALL);
    EXPECT_EQ(stateMaschine_->GetReasonByWakeType(WakeupDeviceType::WAKEUP_DEVICE_SHELL),
        StateChangeReason::STATE_CHANGE_REASON_SHELL);
    pmsTest_->OnStop();
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService025 end.");
}

/**
 * @tc.name: PowerMgrService026
 * @tc.desc: Test ParseWakeupDeviceType
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceTest, PowerMgrService026, TestSize.Level2)
{
    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    ASSERT_TRUE(pmsTest_ != nullptr) << "PowerMgrService026 failed to get PowerMgrService";
    pmsTest_->OnStart();
    auto stateMaschine_ = pmsTest_->GetPowerStateMachine();
    ASSERT_TRUE(stateMaschine_ != nullptr) << "PowerMgrService026 failed to get PowerStateMachine";

    EXPECT_EQ(stateMaschine_->ParseWakeupDeviceType("incoming call"), WakeupDeviceType::WAKEUP_DEVICE_INCOMING_CALL);
    EXPECT_EQ(stateMaschine_->ParseWakeupDeviceType("shell"), WakeupDeviceType::WAKEUP_DEVICE_SHELL);
    EXPECT_EQ(stateMaschine_->ParseWakeupDeviceType("pre_bright"), WakeupDeviceType::WAKEUP_DEVICE_PRE_BRIGHT);
    EXPECT_EQ(stateMaschine_->ParseWakeupDeviceType("pre_bright_auth_success"),
        WakeupDeviceType::WAKEUP_DEVICE_PRE_BRIGHT_AUTH_SUCCESS);
    EXPECT_EQ(stateMaschine_->ParseWakeupDeviceType("pre_bright_auth_fail_screen_on"),
        WakeupDeviceType::WAKEUP_DEVICE_PRE_BRIGHT_AUTH_FAIL_SCREEN_ON);
    EXPECT_EQ(stateMaschine_->ParseWakeupDeviceType("pre_bright_auth_fail_screen_off"),
        WakeupDeviceType::WAKEUP_DEVICE_PRE_BRIGHT_AUTH_FAIL_SCREEN_OFF);

    pmsTest_->OnStop();
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService026 end.");
}
}
