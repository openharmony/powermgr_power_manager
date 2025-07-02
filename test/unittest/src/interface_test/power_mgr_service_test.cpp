/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "accesstoken_kit.h"
#include "display_manager_lite.h"
#include "mock_state_action.h"
#include "nativetoken_kit.h"
#include "power_common.h"
#include "power_mgr_client.h"
#define private   public
#define protected public
#include "power_mgr_service.h"
#undef private
#undef protected
#include "power_utils.h"
#include "setting_helper.h"
#include "token_setproc.h"
#include "mock_power_mgr_client.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

void PowerMgrServiceTest::SetUpTestCase(void)
{
    DelayedSpSingleton<PowerMgrService>::GetInstance()->OnStart();
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

#ifdef POWER_MANAGER_TV_DREAMING
class MockDisplayManagerLite : public Rosen::DisplayManagerLite {
public:
    bool SuspendBegin(Rosen::PowerStateChangeReason reason)
    {
        if (reason == Rosen::PowerStateChangeReason::STATE_CHANGE_REASON_START_DREAM) {
            return true;
        }
        return DisplayManagerLite::SuspendBegin(reason);
    }
    bool WakeUpBegin(Rosen::PowerStateChangeReason reason)
    {
        if (reason == Rosen::PowerStateChangeReason::STATE_CHANGE_REASON_END_DREAM) {
            return true;
        }
        return DisplayManagerLite::WakeUpBegin(reason);
    }
};
Rosen::DisplayManagerLite& Rosen::DisplayManagerLite::GetInstance()
{
    static DisplayManagerLite instance;
    return static_cast<MockDisplayManagerLite&>(instance);
}
#endif
namespace {
constexpr const int64_t STATE_WAIT_TIME_MS = 300;
constexpr const int64_t STATE_OFF_WAIT_TIME_MS = 2000;
constexpr const int64_t NEXT_WAIT_TIME_S = 1;

/**
 * @tc.name: PowerMgrService001
 * @tc.desc: Test PowerMgrService service ready.
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceTest, PowerMgrService001, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrService001 function start!");
    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    ASSERT_TRUE(sam != nullptr) << "PowerMgrService01 fail to get GetSystemAbilityManager";
    sptr<IRemoteObject> remoteObject_ = sam->CheckSystemAbility(POWER_MANAGER_SERVICE_ID);
    ASSERT_TRUE(remoteObject_ != nullptr) << "GetSystemAbility failed.";
    POWER_HILOGI(LABEL_TEST, "PowerMgrService001 function end!");
}

/**
 * @tc.name: PowerMgrService002
 * @tc.desc: Test PowerMgrService Start and stop.
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceTest, PowerMgrService002, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrService002 function start!");
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
    POWER_HILOGI(LABEL_TEST, "PowerMgrService002 function end!");
}

/**
 * @tc.name: PowerMgrService003
 * @tc.desc: Test overrideScreenOffTime in screenon
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceTest, PowerMgrService003, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService003 function start!");
    uint32_t PARM_THREE = 3;
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.WakeupDevice();
    usleep(50000);
    EXPECT_EQ(powerMgrClient.OverrideScreenOffTime(1000), PowerErrors::ERR_OK);
    sleep(PARM_THREE);
    EXPECT_EQ(powerMgrClient.IsFoldScreenOn(), false) << "PowerMgrService003: Prepare Fail, Screen is ON.";
    EXPECT_EQ(powerMgrClient.RestoreScreenOffTime(), PowerErrors::ERR_OK);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService003 function end!");
}

/**
 * @tc.name: PowerMgrService004
 * @tc.desc: Test overrideScreenOffTime in screenon
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceTest, PowerMgrService004, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService004 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.WakeupDevice();
    EXPECT_FALSE(powerMgrClient.OverrideScreenOffTime(0) == PowerErrors::ERR_OK);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrService004: Prepare Fail, Screen is ON.";

    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService004 function end!");
}

/**
 * @tc.name: PowerMgrService005
 * @tc.desc: Test overrideScreenOffTime in screenon
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceTest, PowerMgrService005, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService005 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.WakeupDevice();
    EXPECT_FALSE(powerMgrClient.OverrideScreenOffTime(-1) == PowerErrors::ERR_OK);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrService005: Prepare Fail, Screen is OFF.";

    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService005 function end!");
}

/**
 * @tc.name: PowerMgrService006
 * @tc.desc: Test overrideScreenOffTime in screenoff
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceTest, PowerMgrService006, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService006 function start!");
    uint32_t PARM_THREE = 3;
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.SuspendDevice();
    EXPECT_EQ(powerMgrClient.OverrideScreenOffTime(1000), PowerErrors::ERR_OK);
    powerMgrClient.WakeupDevice();
    sleep(PARM_THREE);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerMgrService006: Prepare Fail, Screen is ON.";
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService006 function end!");
}

/**
 * @tc.name: PowerMgrService007
 * @tc.desc: Test overrideScreenOffTime in screenoff
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceTest, PowerMgrService007, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService007 function start!");
    auto& powerMgrClient = MockPowerMgrClient::GetInstance();
    powerMgrClient.SuspendDevice();
    EXPECT_FALSE(powerMgrClient.OverrideScreenOffTime(0) == PowerErrors::ERR_OK);
    powerMgrClient.WakeupDevice();
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrService007: Prepare Fail, Screen is ON.";

    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService007 function end!");
}

/**
 * @tc.name: PowerMgrService008
 * @tc.desc: Test overrideScreenOffTime in screenoff
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceTest, PowerMgrService008, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService008 function start!");
    auto& powerMgrClient = MockPowerMgrClient::GetInstance();
    powerMgrClient.SuspendDevice();
    EXPECT_FALSE(powerMgrClient.OverrideScreenOffTime(-1) == PowerErrors::ERR_OK);
    powerMgrClient.WakeupDevice();
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrService008: Prepare Fail, Screen is OFF.";

    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService008 function end!");
}

/**
 * @tc.name: PowerMgrService009
 * @tc.desc: Test restoreScreenOffTime in screenon
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceTest, PowerMgrService009, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService009 function start!");
    uint32_t PARM_TWO = 2;
    auto& powerMgrClient = MockPowerMgrClient::GetInstance();
    powerMgrClient.WakeupDevice();
    EXPECT_TRUE(powerMgrClient.OverrideScreenOffTime(1000) == PowerErrors::ERR_OK);
    EXPECT_TRUE(powerMgrClient.RestoreScreenOffTime() == PowerErrors::ERR_OK);
    sleep(PARM_TWO);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrService009: Prepare Fail, Screen is OFF.";

    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService009 function end!");
}

/**
 * @tc.name: PowerMgrService010
 * @tc.desc: Test restoreScreenOffTime in screenon
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceTest, PowerMgrService010, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService010 function start!");
    auto& powerMgrClient = MockPowerMgrClient::GetInstance();
    powerMgrClient.WakeupDevice();
    EXPECT_FALSE(powerMgrClient.OverrideScreenOffTime(0) == PowerErrors::ERR_OK);
    EXPECT_FALSE(powerMgrClient.RestoreScreenOffTime() == PowerErrors::ERR_OK);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrService010: Prepare Fail, Screen is OFF.";

    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService010 function end!");
}

/**
 * @tc.name: PowerMgrService011
 * @tc.desc: Test restoreScreenOffTime in screenon
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceTest, PowerMgrService011, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService011 function start!");
    auto& powerMgrClient = MockPowerMgrClient::GetInstance();
    powerMgrClient.WakeupDevice();
    EXPECT_FALSE(powerMgrClient.OverrideScreenOffTime(-1) == PowerErrors::ERR_OK);;
    EXPECT_FALSE(powerMgrClient.RestoreScreenOffTime() == PowerErrors::ERR_OK);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrService011: Prepare Fail, Screen is OFF.";

    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService011 function end!");
}

/**
 * @tc.name: PowerMgrService012
 * @tc.desc: Test restoreScreenOffTime in screenoff
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceTest, PowerMgrService012, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService012 function start!");
    uint32_t PARM_TWO = 2;
    auto& powerMgrClient = MockPowerMgrClient::GetInstance();
    powerMgrClient.SuspendDevice();
    EXPECT_TRUE(powerMgrClient.OverrideScreenOffTime(1000) == PowerErrors::ERR_OK);
    EXPECT_TRUE(powerMgrClient.RestoreScreenOffTime() == PowerErrors::ERR_OK);
    powerMgrClient.WakeupDevice();
    sleep(PARM_TWO);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrService012: Prepare Fail, Screen is OFF.";

    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService012 function end!");
}

/**
 * @tc.name: PowerMgrService013
 * @tc.desc: Test restoreScreenOffTime in screenoff
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceTest, PowerMgrService013, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService013 function start!");
    auto& powerMgrClient = MockPowerMgrClient::GetInstance();
    powerMgrClient.SuspendDevice();
    EXPECT_FALSE(powerMgrClient.OverrideScreenOffTime(0) == PowerErrors::ERR_OK);
    EXPECT_FALSE(powerMgrClient.RestoreScreenOffTime() == PowerErrors::ERR_OK);
    powerMgrClient.WakeupDevice();
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrService013: Prepare Fail, Screen is OFF.";

    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService013 function end!");
}

/**
 * @tc.name: PowerMgrService014
 * @tc.desc: Test restoreScreenOffTime in screenoff
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceTest, PowerMgrService014, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService014 function start!");
    auto& powerMgrClient = MockPowerMgrClient::GetInstance();
    powerMgrClient.SuspendDevice();
    EXPECT_FALSE(powerMgrClient.OverrideScreenOffTime(-1) == PowerErrors::ERR_OK);
    EXPECT_FALSE(powerMgrClient.RestoreScreenOffTime() == PowerErrors::ERR_OK);
    powerMgrClient.WakeupDevice();
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrService014: Prepare Fail, Screen is OFF.";

    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService014 function end!");
}

/**
 * @tc.name: PowerMgrService015
 * @tc.desc: Test Dump
 * @tc.type: FUNC
 * @tc.require: issueI650CX
 */
HWTEST_F(PowerMgrServiceTest, PowerMgrService015, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService015 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    std::vector<std::string> dumpArgs {};
    std::string expectedDebugInfo = "Power manager dump options";
    std::string actualDebugInfo = powerMgrClient.Dump(dumpArgs);
    auto index = actualDebugInfo.find(expectedDebugInfo);
    EXPECT_TRUE(index != string::npos);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService015 function end!");
}

/**
 * @tc.name: PowerMgrService016
 * @tc.desc: Test IsStandby
 * @tc.type: FUNC
 * @tc.require: issueI7QHBE
 */
HWTEST_F(PowerMgrServiceTest, PowerMgrService016, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService016 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    bool standby = false;
    PowerErrors ret = powerMgrClient.IsStandby(standby);
    EXPECT_NE(ret, PowerErrors::ERR_CONNECTION_FAIL);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService016 function end!");
}

/**
 * @tc.name: PowerMgrService017
 * @tc.desc: Test QueryRunningLockLists
 * @tc.type: FUNC
 * @tc.require: issueI8FCZA
 */
HWTEST_F(PowerMgrServiceTest, PowerMgrService017, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService017 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    std::map<std::string, RunningLockInfo> runningLockLists;
    bool ret = powerMgrClient.QueryRunningLockLists(runningLockLists);
    EXPECT_EQ(ret, true);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService017 function end!");
}

/**
 * @tc.name: PowerMgrService018
 * @tc.desc: Test RunningLock Deconstructor unlock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceTest, PowerMgrService018, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService018 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    auto runningLock = powerMgrClient.CreateRunningLock("runninglock", RunningLockType::RUNNINGLOCK_SCREEN);
    ASSERT_TRUE(runningLock != nullptr);
    runningLock->Lock();
    ASSERT_TRUE(runningLock->IsUsed()) << "runningLock->IsUsed() != true";
    runningLock->UnLock();
    EXPECT_EQ(powerMgrClient.ResetRunningLocks(), true);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService018 function end!");
}

/**
 * @tc.name: PowerMgrService019
 * @tc.desc: Test Pre-light the screen.
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceTest, PowerMgrService019, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService019 function start!");
    auto& powerMgrClient = MockPowerMgrClient::GetInstance();
    powerMgrClient.SuspendDevice();
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false);

    powerMgrClient.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_APPLICATION, "pre_bright");
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false);
    powerMgrClient.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_APPLICATION, "pre_bright_auth_success");
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true);

    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService019 function end!");
}

/**
 * @tc.name: PowerMgrService020
 * @tc.desc: Test if the authentication fails, the screen is on.
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceTest, PowerMgrService020, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService020 function start!");
    auto& powerMgrClient = MockPowerMgrClient::GetInstance();
    powerMgrClient.SuspendDevice();
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false);

    powerMgrClient.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_APPLICATION, "pre_bright");
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false);
    PowerErrors ret =
        powerMgrClient.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_APPLICATION, "pre_bright_auth_fail_screen_on");
    EXPECT_EQ(ret, PowerErrors::ERR_OK);

    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService020 function end!");
}

/**
 * @tc.name: PowerMgrService021
 * @tc.desc: Test if the authentication fails, the screen is off.
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceTest, PowerMgrService021, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService021 function start!");
    auto& powerMgrClient = MockPowerMgrClient::GetInstance();
    powerMgrClient.SuspendDevice();
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false);

    powerMgrClient.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_APPLICATION, "pre_bright");
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false);
    powerMgrClient.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_APPLICATION, "pre_bright_auth_fail_screen_off");
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false);

    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService021 function end!");
}

/**
 * @tc.name: PowerMgrService022
 * @tc.desc: Test PowerMgrService LockScreenAfterTimingOut.
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceTest, PowerMgrService022, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService022 function start!");
    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    ASSERT_TRUE(pmsTest_ != nullptr) << "PowerMgrService022 fail to get PowerMgrService";
    auto stateMaschine_ = pmsTest_->GetPowerStateMachine();
    auto runningLockMgr = pmsTest_->GetRunningLockMgr();

    RunningLockParam runningLockParam;
    runningLockParam.name = "runninglock_screen_on";
    runningLockParam.type = RunningLockType::RUNNINGLOCK_SCREEN;
    sptr<IRemoteObject> remoteObject = new RunningLockTokenStub();
    EXPECT_TRUE(runningLockMgr->CreateRunningLock(remoteObject, runningLockParam) != nullptr);
    runningLockMgr->Lock(remoteObject);

    EXPECT_EQ(stateMaschine_->GetReasonBySuspendType(SuspendDeviceType::SUSPEND_DEVICE_REASON_TIMEOUT),
        StateChangeReason::STATE_CHANGE_REASON_TIMEOUT);
    pmsTest_->LockScreenAfterTimingOut(true, false, true, nullptr);
    EXPECT_EQ(stateMaschine_->GetReasonBySuspendType(SuspendDeviceType::SUSPEND_DEVICE_REASON_TIMEOUT),
        StateChangeReason::STATE_CHANGE_REASON_TIMEOUT);
    pmsTest_->LockScreenAfterTimingOut(false, false, true, nullptr);
    EXPECT_EQ(stateMaschine_->GetReasonBySuspendType(SuspendDeviceType::SUSPEND_DEVICE_REASON_TIMEOUT),
        StateChangeReason::STATE_CHANGE_REASON_TIMEOUT_NO_SCREEN_LOCK);
    EXPECT_EQ(stateMaschine_->GetReasonBySuspendType(SuspendDeviceType::SUSPEND_DEVICE_REASON_POWER_KEY),
        StateChangeReason::STATE_CHANGE_REASON_HARD_KEY);
    pmsTest_->LockScreenAfterTimingOut(true, true, true, nullptr);
    EXPECT_EQ(stateMaschine_->GetReasonBySuspendType(SuspendDeviceType::SUSPEND_DEVICE_REASON_TIMEOUT),
        StateChangeReason::STATE_CHANGE_REASON_TIMEOUT_NO_SCREEN_LOCK);
    runningLockMgr->UnLock(remoteObject);
    EXPECT_EQ(stateMaschine_->GetReasonBySuspendType(SuspendDeviceType::SUSPEND_DEVICE_REASON_TIMEOUT),
        StateChangeReason::STATE_CHANGE_REASON_TIMEOUT);
    // wait runninglock async task to end, otherwise it will interfere with the next test case
    pmsTest_->OnStop();
    ffrt::wait();
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService022 function end!");
}

/**
 * @tc.name: PowerMgrService023
 * @tc.desc: Test transition to DIM state for Timeout.
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceTest, PowerMgrService023, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService023 function start!");
    constexpr const int64_t screenOffTime = 4000;
    constexpr const int64_t US_PER_MS = 1000;
    constexpr const uint32_t DELAY_US = 500 * 1000;
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.WakeupDevice();
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);
    EXPECT_TRUE(powerMgrClient.OverrideScreenOffTime(screenOffTime) == PowerErrors::ERR_OK);
    // wait till going to DIM
    usleep((screenOffTime - screenOffTime / PowerStateMachine::OFF_TIMEOUT_FACTOR + STATE_WAIT_TIME_MS) * US_PER_MS);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::DIM);
    EXPECT_TRUE(powerMgrClient.RefreshActivity());
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);
    // wait till going to DIM
    usleep((screenOffTime - screenOffTime / PowerStateMachine::OFF_TIMEOUT_FACTOR + STATE_WAIT_TIME_MS) * US_PER_MS);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::DIM);
    // wait till going to SLEEP
    usleep((screenOffTime / PowerStateMachine::OFF_TIMEOUT_FACTOR + STATE_OFF_WAIT_TIME_MS) *
        US_PER_MS);
    usleep(DELAY_US);
    EXPECT_NE(powerMgrClient.GetState(), PowerState::AWAKE);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService023 function end!");
}

/**
 * @tc.name: PowerMgrService024
 * @tc.desc: Test multithread refreshing.
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceTest, PowerMgrService024, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService024 function start!");
    uint32_t PARM_TEN = 10;
    constexpr const uint32_t TESTING_DURATION_S = 10;
    constexpr const uint32_t OPERATION_DELAY_US = 500 * 1000;
    constexpr const uint32_t EXTREMELY_SHORT_SCREEN_OFF_TIME_MS = 200;
    constexpr const uint32_t SHORT_SCREEN_OFF_TIME_MS = 800;
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.OverrideScreenOffTime(SHORT_SCREEN_OFF_TIME_MS);
    powerMgrClient.WakeupDevice();
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);
    std::vector<std::thread> refreshThreads;
    bool endTask = false;
    bool endRefresh = false;
    auto refreshTask = [&powerMgrClient, &endRefresh]() {
        while (!endRefresh) {
            powerMgrClient.RefreshActivity();
        }
    };

    for (int i = 0; i < 100; i++) {
        refreshThreads.emplace_back(std::thread(refreshTask));
    }

    auto checkingTask = [&powerMgrClient, &endTask]() {
        while (!endTask) {
            powerMgrClient.SuspendDevice();
            usleep(OPERATION_DELAY_US);
            EXPECT_NE(powerMgrClient.GetState(), PowerState::AWAKE);
            usleep(OPERATION_DELAY_US);
            powerMgrClient.WakeupDevice();
            EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);
            usleep(OPERATION_DELAY_US);
        }
    };
    // checks whether refresh tasks may unexpectedly turn screen on
    std::thread checkingThread(checkingTask);
    sleep(PARM_TEN);
    endTask = true;
    checkingThread.join();
    // part2 start
    powerMgrClient.WakeupDevice();
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);
    // checks whether timeout events are all blocked
    powerMgrClient.OverrideScreenOffTime(EXTREMELY_SHORT_SCREEN_OFF_TIME_MS);
    sleep(PARM_TEN);
    EXPECT_TRUE(powerMgrClient.IsScreenOn());
    endRefresh = true;
    for (auto& thread : refreshThreads) {
        thread.join();
    }
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService024 function end!");
}

/**
 * @tc.name: PowerMgrService025
 * @tc.desc: Test StateChangeReason Get
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceTest, PowerMgrService025, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService025 function start!");
    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    ASSERT_TRUE(pmsTest_ != nullptr) << "PowerMgrService025 failed to get PowerMgrService";
    auto stateMaschine_ = pmsTest_->GetPowerStateMachine();
    ASSERT_TRUE(stateMaschine_ != nullptr) << "PowerMgrService025 failed to get PowerStateMachine";

    EXPECT_EQ(stateMaschine_->GetReasonBySuspendType(SuspendDeviceType::SUSPEND_DEVICE_REASON_LID),
        StateChangeReason::STATE_CHANGE_REASON_LID);
    EXPECT_EQ(stateMaschine_->GetReasonBySuspendType(SuspendDeviceType::SUSPEND_DEVICE_REASON_SWITCH),
        StateChangeReason::STATE_CHANGE_REASON_SWITCH);
    EXPECT_EQ(stateMaschine_->GetReasonBySuspendType(SuspendDeviceType::SUSPEND_DEVICE_REASON_POWER_KEY),
        StateChangeReason::STATE_CHANGE_REASON_HARD_KEY);
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
    EXPECT_EQ(stateMaschine_->GetReasonByWakeType(WakeupDeviceType::WAKEUP_DEVICE_BLUETOOTH_INCOMING_CALL),
        StateChangeReason::STATE_CHANGE_REASON_BLUETOOTH_INCOMING_CALL);
    EXPECT_EQ(stateMaschine_->GetReasonByWakeType(WakeupDeviceType::WAKEUP_DEVICE_PICKUP),
        StateChangeReason::STATE_CHANGE_REASON_PICKUP);
    pmsTest_->OnStop();
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService025 function end!");
}

/**
 * @tc.name: PowerMgrService026
 * @tc.desc: Test ParseWakeupDeviceType
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceTest, PowerMgrService026, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService026 function start!");
    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    ASSERT_TRUE(pmsTest_ != nullptr) << "PowerMgrService026 failed to get PowerMgrService";
    auto stateMaschine_ = pmsTest_->GetPowerStateMachine();
    ASSERT_TRUE(stateMaschine_ != nullptr) << "PowerMgrService026 failed to get PowerStateMachine";

    EXPECT_EQ(PowerUtils::ParseWakeupDeviceType("incoming call"), WakeupDeviceType::WAKEUP_DEVICE_INCOMING_CALL);
    EXPECT_EQ(PowerUtils::ParseWakeupDeviceType("shell"), WakeupDeviceType::WAKEUP_DEVICE_SHELL);
    EXPECT_EQ(PowerUtils::ParseWakeupDeviceType("pre_bright"), WakeupDeviceType::WAKEUP_DEVICE_PRE_BRIGHT);
    EXPECT_EQ(PowerUtils::ParseWakeupDeviceType("pre_bright_auth_success"),
        WakeupDeviceType::WAKEUP_DEVICE_PRE_BRIGHT_AUTH_SUCCESS);
    EXPECT_EQ(PowerUtils::ParseWakeupDeviceType("pre_bright_auth_fail_screen_on"),
        WakeupDeviceType::WAKEUP_DEVICE_PRE_BRIGHT_AUTH_FAIL_SCREEN_ON);
    EXPECT_EQ(PowerUtils::ParseWakeupDeviceType("pre_bright_auth_fail_screen_off"),
        WakeupDeviceType::WAKEUP_DEVICE_PRE_BRIGHT_AUTH_FAIL_SCREEN_OFF);
    EXPECT_EQ(PowerUtils::ParseWakeupDeviceType("wake up screen:receive bluetooth call"),
        WakeupDeviceType::WAKEUP_DEVICE_BLUETOOTH_INCOMING_CALL);
    pmsTest_->OnStop();
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService026 function end!");
}

/**
 * @tc.name: PowerMgrService027
 * @tc.desc: Test entering doze state
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceTest, PowerMgrService027, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService027 function start!");
    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    ASSERT_TRUE(pmsTest_ != nullptr) << "PowerMgrService027 failed to get PowerMgrService";
    auto stateMaschine_ = pmsTest_->GetPowerStateMachine();
    ASSERT_TRUE(stateMaschine_ != nullptr) << "PowerMgrService027 failed to get PowerStateMachine";
    ::testing::NiceMock<MockStateAction>* stateActionMock = new ::testing::NiceMock<MockStateAction>;
    // will be managed by a unique_ptr in EnableMock
    stateMaschine_->EnableMock(stateActionMock);

    pmsTest_->SetEnableDoze(false);
    EXPECT_CALL(*stateActionMock, SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_));
    // prevent powerStateMachine from correcting its state (default action is to return 0 i.e DISPLAY_OFF)
    EXPECT_CALL(*stateActionMock, GetDisplayState()).WillOnce(::testing::Return(DisplayState::DISPLAY_ON));
    pmsTest_->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_DEVICE_ADMIN, true);
    EXPECT_EQ(stateMaschine_->GetState(), PowerState::INACTIVE);

    pmsTest_->SetEnableDoze(true);
    EXPECT_CALL(*stateActionMock, SetDisplayState(DisplayState::DISPLAY_DOZE, ::testing::_));
    EXPECT_CALL(*stateActionMock, GetDisplayState()).WillOnce(::testing::Return(DisplayState::DISPLAY_ON));
    pmsTest_->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_DEVICE_ADMIN, true);
    EXPECT_EQ(stateMaschine_->GetState(), PowerState::INACTIVE);
    // release mock object
    auto& stateAction = const_cast<std::shared_ptr<IDeviceStateAction>&>(stateMaschine_->GetStateAction());
    stateAction.reset();
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService027 function end!");
}

/**
 * @tc.name: PowerMgrService028
 * @tc.desc: Test switching doze state
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceTest, PowerMgrService028, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService028 function start!");
    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    ASSERT_TRUE(pmsTest_ != nullptr) << "PowerMgrService028 failed to get PowerMgrService";
    auto stateMaschine_ = pmsTest_->GetPowerStateMachine();
    ASSERT_TRUE(stateMaschine_ != nullptr) << "PowerMgrService028 failed to get PowerStateMachine";
    ::testing::NiceMock<MockStateAction>* stateActionMock = new ::testing::NiceMock<MockStateAction>;
    stateMaschine_->EnableMock(stateActionMock);

    EXPECT_CALL(*stateActionMock, SetDisplayState(DisplayState::DISPLAY_DOZE, ::testing::_))
        .Times(0);
    EXPECT_CALL(*stateActionMock, GetDisplayState()).WillOnce(::testing::Return(DisplayState::DISPLAY_ON));
    pmsTest_->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_APPLICATION, "display_doze");

    EXPECT_CALL(*stateActionMock,
        SetDisplayState(
            DisplayState::DISPLAY_DOZE_SUSPEND, StateChangeReason::STATE_CHANGE_REASON_SWITCHING_DOZE_MODE));
    EXPECT_CALL(*stateActionMock, GetDisplayState()).WillOnce(::testing::Return(DisplayState::DISPLAY_DOZE));
    pmsTest_->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_APPLICATION, "display_doze_suspend");

    EXPECT_CALL(*stateActionMock,
        SetDisplayState(DisplayState::DISPLAY_DOZE, StateChangeReason::STATE_CHANGE_REASON_SWITCHING_DOZE_MODE));
    EXPECT_CALL(*stateActionMock, GetDisplayState()).WillOnce(::testing::Return(DisplayState::DISPLAY_DOZE_SUSPEND));
    pmsTest_->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_APPLICATION, "display_doze");

    EXPECT_CALL(*stateActionMock,
        SetDisplayState(DisplayState::DISPLAY_OFF, StateChangeReason::STATE_CHANGE_REASON_SWITCHING_DOZE_MODE));
    EXPECT_CALL(*stateActionMock, GetDisplayState()).WillOnce(::testing::Return(DisplayState::DISPLAY_DOZE));
    pmsTest_->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_APPLICATION, "display_off");
    auto& stateAction = const_cast<std::shared_ptr<IDeviceStateAction>&>(stateMaschine_->GetStateAction());
    stateAction.reset();
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService028 function end!");
}

/**
 * @tc.name: PowerMgrService029
 * @tc.desc: Test observing doze switch
 * @tc.type: FUNC
 */
void AddPermission()
{
    const char* perms[2];
    perms[0] = "ohos.permission.MANAGE_SECURE_SETTINGS";
    perms[1] = "ohos.permission.MANAGE_SETTINGS";
    NativeTokenInfoParams info = {
        .dcapsNum = 0,
        .permsNum = 2,
        .dcaps = 0,
        .perms = perms,
        .acls = NULL,
        .processName = "powermgr",
        .aplStr = "system_core",
    };
    SetSelfTokenID(GetAccessTokenId(&info));
    OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

HWTEST_F(PowerMgrServiceTest, PowerMgrService029, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService029 function start!");
    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    ASSERT_TRUE(pmsTest_ != nullptr) << "PowerMgrService029 failed to get PowerMgrService";
    auto stateMaschine_ = pmsTest_->GetPowerStateMachine();
    ASSERT_TRUE(stateMaschine_ != nullptr) << "PowerMgrService029 failed to get PowerStateMachine";
    ::testing::NiceMock<MockStateAction>* stateActionMock = new ::testing::NiceMock<MockStateAction>;
    stateMaschine_->EnableMock(stateActionMock);
    AddPermission();
    SettingHelper::RegisterAodSwitchObserver();
    std::string originalValue {"0"};
    SettingProvider::GetInstance(-1).GetStringValue("hw_aod_watch_switch", originalValue);
    ErrCode ret = SettingProvider::GetInstance(-1).PutStringValue("hw_aod_watch_switch", "0", true);
    EXPECT_EQ(ret, ERR_OK);
    sleep(1);
    EXPECT_CALL(*stateActionMock, SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_));
    EXPECT_CALL(*stateActionMock, GetDisplayState()).WillOnce(::testing::Return(DisplayState::DISPLAY_ON));
    pmsTest_->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_DEVICE_ADMIN, true);
    EXPECT_EQ(stateMaschine_->GetState(), PowerState::INACTIVE);

    ret = SettingProvider::GetInstance(-1).PutStringValue("hw_aod_watch_switch", "1", true);
    EXPECT_EQ(ret, ERR_OK);
    sleep(1);
    EXPECT_CALL(*stateActionMock, SetDisplayState(DisplayState::DISPLAY_DOZE, ::testing::_));
    EXPECT_CALL(*stateActionMock, GetDisplayState()).WillOnce(::testing::Return(DisplayState::DISPLAY_ON));
    pmsTest_->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_DEVICE_ADMIN, true);
    EXPECT_EQ(stateMaschine_->GetState(), PowerState::INACTIVE);
    auto& stateAction = const_cast<std::shared_ptr<IDeviceStateAction>&>(stateMaschine_->GetStateAction());
    stateAction.reset();
    SettingProvider::GetInstance(-1).PutStringValue("hw_aod_watch_switch", originalValue, true);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService029 function end!");
}


#ifdef POWER_MANAGER_TV_DREAMING
/**
 * @tc.name: PowerMgrService030
 * @tc.desc: Test dream state
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceTest, PowerMgrServiceTest030, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService030 function start!");
    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    ASSERT_TRUE(pmsTest_ != nullptr) << "PowerMgrService030 failed to get PowerMgrService";
    auto stateMaschine_ = pmsTest_->GetPowerStateMachine();
    ASSERT_TRUE(stateMaschine_ != nullptr) << "PowerMgrService030 failed to get PowerStateMachine";
    stateMaschine_->Init();
    pmsTest_->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_DEVICE_ADMIN, true); // reset dream state
    pmsTest_->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_START_DREAM, true); // set dream from interface
    EXPECT_TRUE(stateMaschine_->SetDreamingState(StateChangeReason::STATE_CHANGE_REASON_START_DREAM));
    EXPECT_TRUE(stateMaschine_->SetDreamingState(StateChangeReason::STATE_CHANGE_REASON_END_DREAM));
    pmsTest_->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_APPLICATION, "start_dream");
    EXPECT_TRUE(stateMaschine_->SetDreamingState(StateChangeReason::STATE_CHANGE_REASON_START_DREAM));
    pmsTest_->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_APPLICATION, "end_dream");
    EXPECT_TRUE(stateMaschine_->SetDreamingState(StateChangeReason::STATE_CHANGE_REASON_END_DREAM));
    EXPECT_TRUE(stateMaschine_->SetDreamingState(StateChangeReason::STATE_CHANGE_REASON_START_DREAM));
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService030 function end!");
}
#endif

/**
 * @tc.name: PowerMgrServiceTest031
 * @tc.desc: test IsForceSleeping
 * @tc.type: FUNC
 * @tc.require: issueICE3O4
 */
HWTEST_F(PowerMgrServiceTest, PowerMgrServiceTest031, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest031 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    int32_t wakeupReason = (static_cast<int32_t>(WakeupDeviceType::WAKEUP_DEVICE_MAX)) + 1;
    WakeupDeviceType abnormaltype = WakeupDeviceType(wakeupReason);
    powerMgrClient.WakeupDevice();
    sleep(NEXT_WAIT_TIME_S);
    EXPECT_EQ(powerMgrClient.IsForceSleeping(), false);
    powerMgrClient.SuspendDevice();
    sleep(NEXT_WAIT_TIME_S);
    EXPECT_EQ(powerMgrClient.IsForceSleeping(), false);

    powerMgrClient.WakeupDevice();
    sleep(NEXT_WAIT_TIME_S);
    EXPECT_EQ(powerMgrClient.IsForceSleeping(), false);
    powerMgrClient.ForceSuspendDevice();
    sleep(NEXT_WAIT_TIME_S);
#ifdef POWER_MANAGER_ENABLE_FORCE_SLEEP_BROADCAST
    EXPECT_EQ(powerMgrClient.IsForceSleeping(), true);
#else
    EXPECT_EQ(powerMgrClient.IsForceSleeping(), false);
#endif
    powerMgrClient.WakeupDevice(abnormaltype);
    sleep(NEXT_WAIT_TIME_S);
#ifdef POWER_MANAGER_ENABLE_FORCE_SLEEP_BROADCAST
    EXPECT_EQ(powerMgrClient.IsForceSleeping(), true);
#else
    EXPECT_EQ(powerMgrClient.IsForceSleeping(), false);
#endif
    powerMgrClient.WakeupDevice();
    sleep(NEXT_WAIT_TIME_S);
    EXPECT_EQ(powerMgrClient.IsForceSleeping(), false);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest031 function end!");
}

/**
 * @tc.name: PowerMgrService032
 * @tc.desc: Test ForceSuspendDevice.
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceTest, PowerMgrService031, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService032 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    EXPECT_EQ(powerMgrClient.ForceSuspendDevice(""), PowerErrors::ERR_OK);
    powerMgrClient.WakeupDevice();
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService032 function end!");
}
}
