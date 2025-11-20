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

#define private   public
#define protected public

#include "power_mgr_service_test.h"

#include <csignal>
#include <iostream>
#include <thread>

#include <if_system_ability_manager.h>
#include <ipc_skeleton.h>
#include <iservice_registry.h>
#include <string_ex.h>
#include <system_ability_definition.h>

#include "permission.h"
#include "accesstoken_kit.h"
#include "display_manager_lite.h"
#include "mock_state_action.h"
#include "nativetoken_kit.h"
#include "power_common.h"
#include "power_mgr_client.h"
#include "power_mgr_service.h"
#include "power_utils.h"
#include "setting_helper.h"
#include "token_setproc.h"
#include "mock_power_mgr_client.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

namespace {
bool g_isSystem = true;
bool g_isPermissionGranted = true;
} // namespace

namespace OHOS::PowerMgr {
bool Permission::IsSystem()
{
    return g_isSystem;
}

bool Permission::IsPermissionGranted(const std::string& perm)
{
    return g_isPermissionGranted;
}
} // namespace OHOS::PowerMgr

void PowerMgrServiceTest::SetUpTestCase(void)
{
    stub_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    stub_->OnStart();
}

void PowerMgrServiceTest::TearDownTestCase(void)
{
}

void PowerMgrServiceTest::SetUp(void)
{
    if (!stateActionMock || !shutdownStateActionMock || !powerActionMock || !lockActionMock) {
        stateActionMock = new NiceMock<MockStateAction>;
        shutdownStateActionMock = new NiceMock<MockStateAction>;
        powerActionMock = new NiceMock<MockPowerAction>;
        lockActionMock = new NiceMock<MockLockAction>;
        stub_->EnableMock(stateActionMock, shutdownStateActionMock, powerActionMock, lockActionMock);
    }
}

void PowerMgrServiceTest::TearDown(void)
{
    g_isSystem = true;
    g_isPermissionGranted = true;
    stub_->EnableMock(nullptr, nullptr, nullptr, nullptr);
    // EnableMock for PowerStateMachine ignores nullptr, reset it manually
    auto& stateAction =
        const_cast<std::shared_ptr<IDeviceStateAction>&>(stub_->GetPowerStateMachine()->GetStateAction());
    stateAction.reset();
    stateActionMock = nullptr;
    shutdownStateActionMock = nullptr;
    powerActionMock = nullptr;
    lockActionMock = nullptr;
}

sptr<IPowerMgr> PowerMgrClient::GetPowerMgrProxy()
{
    return PowerMgrServiceTest::stub_;
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
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
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
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
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
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_OFF));
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
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
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
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
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
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
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
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
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
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
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
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
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
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
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
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrService014: Prepare Fail, Screen is OFF.";
    auto& powerMgrClient1 = PowerMgrClient::GetInstance();
    std::vector<std::string> dumpArgs {};
    std::string expectedDebugInfo = "Power manager dump options";
    std::string actualDebugInfo = powerMgrClient1.Dump(dumpArgs);

    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService014 function end!");
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
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_OFF));
    
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false);

    powerMgrClient.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_APPLICATION, "pre_bright");
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false);
    powerMgrClient.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_APPLICATION, "pre_bright_auth_success");
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
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
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_OFF));
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
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_OFF));
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

    sptr<IPCObjectProxy> testProxy = sptr<IPCObjectProxy>::MakeSptr(0, std::u16string{u"testProxy"});
    EXPECT_EQ(stateMaschine_->GetReasonBySuspendType(SuspendDeviceType::SUSPEND_DEVICE_REASON_TIMEOUT),
        StateChangeReason::STATE_CHANGE_REASON_TIMEOUT);
    pmsTest_->LockScreenAfterTimingOut(true, false, true, testProxy);
    EXPECT_EQ(stateMaschine_->GetReasonBySuspendType(SuspendDeviceType::SUSPEND_DEVICE_REASON_TIMEOUT),
        StateChangeReason::STATE_CHANGE_REASON_TIMEOUT);
    pmsTest_->LockScreenAfterTimingOut(false, false, true, testProxy);
    EXPECT_EQ(stateMaschine_->GetReasonBySuspendType(SuspendDeviceType::SUSPEND_DEVICE_REASON_TIMEOUT),
        StateChangeReason::STATE_CHANGE_REASON_TIMEOUT_NO_SCREEN_LOCK);
    EXPECT_EQ(stateMaschine_->GetReasonBySuspendType(SuspendDeviceType::SUSPEND_DEVICE_REASON_POWER_KEY),
        StateChangeReason::STATE_CHANGE_REASON_HARD_KEY);
    pmsTest_->LockScreenAfterTimingOut(true, true, true, testProxy);
    EXPECT_EQ(stateMaschine_->GetReasonBySuspendType(SuspendDeviceType::SUSPEND_DEVICE_REASON_TIMEOUT),
        StateChangeReason::STATE_CHANGE_REASON_TIMEOUT_NO_SCREEN_LOCK);
    runningLockMgr->UnLock(remoteObject);
    EXPECT_EQ(stateMaschine_->GetReasonBySuspendType(SuspendDeviceType::SUSPEND_DEVICE_REASON_TIMEOUT),
        StateChangeReason::STATE_CHANGE_REASON_TIMEOUT);

    // reset the value to default, otherwise it would interfere with LockScreenAfterTimingOutWithAppidTest
    // consider destroy and recreate pms instance instead of using singleton?
    pmsTest_->LockScreenAfterTimingOut(true, false, true, testProxy);
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
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_TRUE(powerMgrClient.IsScreenOn());
    EXPECT_TRUE(powerMgrClient.OverrideScreenOffTime(screenOffTime) == PowerErrors::ERR_OK);
    // wait till going to DIM
    usleep((screenOffTime - screenOffTime / PowerStateMachine::OFF_TIMEOUT_FACTOR + STATE_WAIT_TIME_MS) * US_PER_MS);

    EXPECT_TRUE(powerMgrClient.RefreshActivity());

    // wait till going to DIM
    usleep((screenOffTime - screenOffTime / PowerStateMachine::OFF_TIMEOUT_FACTOR + STATE_WAIT_TIME_MS) * US_PER_MS);

    // wait till going to SLEEP
    usleep((screenOffTime / PowerStateMachine::OFF_TIMEOUT_FACTOR + STATE_OFF_WAIT_TIME_MS) *
        US_PER_MS);
    usleep(DELAY_US);

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
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
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
    EXPECT_EQ(stateMaschine_->GetReasonByWakeType(WakeupDeviceType::WAKEUP_DEVICE_FROM_ULSR),
        StateChangeReason::STATE_CHANGE_REASON_WAKEUP_FROM_ULSR);
    EXPECT_EQ(stateMaschine_->GetReasonByWakeType(WakeupDeviceType::WAKEUP_DEVICE_MESSAGE_NOTIFICATION),
        StateChangeReason::STATE_CHANGE_REASON_MESSAGE_NOTIFICATION);
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
    EXPECT_EQ(PowerUtils::ParseWakeupDeviceType("wake up screen:wake screen switch on and receive notification"),
        WakeupDeviceType::WAKEUP_DEVICE_MESSAGE_NOTIFICATION);
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
    stub_->suspendController_ =
        std::make_shared<SuspendController>(stub_->shutdownController_, stub_->powerStateMachine_, stub_->ffrtTimer_);
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

    // try to wait ffrt tasks to end
    auto ffrtTimer = stub_->ffrtTimer_;
    if (ffrtTimer) {
        ffrtTimer->Clear();
    }
    ffrt::wait();
    stub_->suspendController_ = nullptr;
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest031 function end!");
}

/**
 * @tc.name: PowerMgrService032
 * @tc.desc: Test ForceSuspendDevice.
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceTest, PowerMgrService032, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService032 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    EXPECT_EQ(powerMgrClient.ForceSuspendDevice(""), PowerErrors::ERR_OK);
    powerMgrClient.WakeupDevice();
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService032 function end!");
}

/**
 * @tc.name: PowerMgrService033
 * @tc.desc: Test Dump.
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceTest, PowerMgrService033, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrService033 function start!");
    size_t size = 1000;
    std::vector<std::string> cmdsList;
    for (size_t i = 0; i < size; i++) {
        std::string cmd = "test_cmd" + std::to_string(i);
        cmdsList.push_back(cmd);
    }
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    std::string result = powerMgrClient.Dump(cmdsList);

    POWER_HILOGI(LABEL_TEST, "result : %{public}s", result.c_str());
    EXPECT_EQ(true, result == "remote error");
    POWER_HILOGI(LABEL_TEST, "PowerMgrService032 function end!");
}

/**
 * @tc.name: PowerMgrService034
 * @tc.desc: Test Dump.
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceTest, PowerMgrService034, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrService034 function start!");
    size_t size = 5;
    std::vector<std::string> cmdsList;
    for (size_t i = 0; i < size; i++) {
        std::string cmd = "test_cmd" + std::to_string(i);
        cmdsList.push_back(cmd);
    }
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    std::string result = powerMgrClient.Dump(cmdsList);

    POWER_HILOGI(LABEL_TEST, "result : %{public}s", result.c_str());
    EXPECT_EQ(true, result.empty());
    POWER_HILOGI(LABEL_TEST, "PowerMgrService034 function end!");
}

#ifdef POWER_MANAGER_TAKEOVER_SUSPEND
class TestTakeOverSuspendCallback : public ITakeOverSuspendCallback {
    public:
        TestTakeOverSuspendCallback() = default;
        virtual ~TestTakeOverSuspendCallback() = default;

        bool OnTakeOverSuspend(SuspendDeviceType type) override
        {
            return false;
        }
        sptr<IRemoteObject> AsObject() override
        {
            return nullptr;
        }
};

/**
 * @tc.name: PowerMgrService035
 * @tc.desc: Test PowerStateMachine::EmplaceInactive()
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceTest, PowerMgrService035, TestSize.Level0) {
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService035 function start!");
    auto powerMgrService = DelayedSpSingleton<PowerMgrService>::GetInstance();
    std::shared_ptr<PowerStateMachine> stateMachine = powerMgrService->GetPowerStateMachine();
    stateMachine->EmplaceInactive();
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService035 function end!");
}

/**
 * @tc.name: PowerMgrService036
 * @tc.desc: Test StateChangeReason Get
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceTest, PowerMgrService036, TestSize.Level0) {
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService036 function start!");
    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    ASSERT_TRUE(pmsTest_ != nullptr) << "PowerMgrService036 failed to get PowerMgrService";
    auto stateMachine_ = pmsTest_->GetPowerStateMachine();
    ASSERT_TRUE(stateMachine_ != nullptr) << "PowerMgrService036 failed to get PowerStateMachine";

    stateMachine_->GetSuspendTypeByReason(StateChangeReason::STATE_CHANGE_REASON_TIMEOUT_NO_SCREEN_LOCK);
    stateMachine_->GetSuspendTypeByReason(StateChangeReason::STATE_CHANGE_REASON_SYSTEM);
    EXPECT_EQ(stateMachine_->GetSuspendTypeByReason(StateChangeReason::STATE_CHANGE_REASON_TIMEOUT),
        SuspendDeviceType::SUSPEND_DEVICE_REASON_TIMEOUT);
    EXPECT_EQ(stateMachine_->GetSuspendTypeByReason(StateChangeReason::STATE_CHANGE_REASON_TIMEOUT_NO_SCREEN_LOCK),
        SuspendDeviceType::SUSPEND_DEVICE_REASON_TIMEOUT);
    EXPECT_EQ(stateMachine_->GetSuspendTypeByReason(StateChangeReason::STATE_CHANGE_REASON_HARD_KEY),
        SuspendDeviceType::SUSPEND_DEVICE_REASON_POWER_KEY);
    EXPECT_EQ(stateMachine_->GetSuspendTypeByReason(StateChangeReason::STATE_CHANGE_REASON_SYSTEM),
        SuspendDeviceType::SUSPEND_DEVICE_REASON_FORCE_SUSPEND);
    EXPECT_EQ(stateMachine_->GetSuspendTypeByReason(StateChangeReason::STATE_CHANGE_REASON_UNKNOWN),
        SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService036 function end!");
}

/**
 * @tc.name: PowerMgrService037
 * @tc.desc: Test RegisterSuspendTakeoverCallback
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceTest, PowerMgrService037, TestSize.Level0) {
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService037 function start!");
    sptr<TestTakeOverSuspendCallback> callback = new TestTakeOverSuspendCallback();
    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    TakeOverSuspendPriority priority = TakeOverSuspendPriority::HIGH;
    pmsTest_->RegisterSuspendTakeoverCallback(callback, priority);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService037 function end!");
}

/**
 * @tc.name: PowerMgrService038
 * @tc.desc: Test UnRegisterSuspendTakeoverCallback
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceTest, PowerMgrService038, TestSize.Level0) {
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService038 function start!");
    sptr<TestTakeOverSuspendCallback> callback = new TestTakeOverSuspendCallback();
    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    pmsTest_->UnRegisterSuspendTakeoverCallback(callback);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService038 function end!");
}
#endif

/**
 * @tc.name: PowerMgrService039
 * @tc.desc: Test SetPowerKeyFilteringStrategy with all branches
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceTest, PowerMgrService039, TestSize.Level2) {
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService039 start!");
    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    PowerKeyFilteringStrategy strategy;
    PowerErrors ret;

    // Test case 1: No system permission
    g_isSystem = false;
    g_isPermissionGranted = true;
    ret = pmsTest_->SetPowerKeyFilteringStrategy(PowerKeyFilteringStrategy::DISABLE_LONG_PRESS_FILTERING);
    EXPECT_EQ(ret, PowerErrors::ERR_SYSTEM_API_DENIED) << "Test case 1 failed";

    // Test case 2: No POWER_MANAGER permission
    g_isSystem = true;
    g_isPermissionGranted = false;
    ret = pmsTest_->SetPowerKeyFilteringStrategy(PowerKeyFilteringStrategy::DISABLE_LONG_PRESS_FILTERING);
    EXPECT_EQ(ret, PowerErrors::ERR_PERMISSION_DENIED) << "Test case 2 failed";

    // Test case 3: Valid strategy DISABLE_LONG_PRESS_FILTERING
    g_isPermissionGranted = true;
    strategy = PowerKeyFilteringStrategy::DISABLE_LONG_PRESS_FILTERING;
    ret = pmsTest_->SetPowerKeyFilteringStrategy(strategy);
    EXPECT_EQ(ret, PowerErrors::ERR_OK) << "Test case 3 failed";

    // Test case 4: Valid strategy LONG_PRESS_FILTERING_ONCE
    strategy = PowerKeyFilteringStrategy::LONG_PRESS_FILTERING_ONCE;
    ret = pmsTest_->SetPowerKeyFilteringStrategy(strategy);
    EXPECT_EQ(ret, PowerErrors::ERR_OK) << "Test case 4 failed";

    // Test case 5: Invalid strategy
    strategy = static_cast<PowerKeyFilteringStrategy>(PowerKeyFilteringStrategy::STRATEGY_MAX);
    ret = pmsTest_->SetPowerKeyFilteringStrategy(strategy);
    EXPECT_EQ(ret, PowerErrors::ERR_OK) << "Test case 5 failed";

    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService039 end!");
}

/**
 * @tc.name: PowerMgrService040
 * @tc.desc: Test   StateChangeReasonToStringConversion
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceTest, PowerMgrService040, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService040 start!");
    EXPECT_EQ(PowerUtils::GetReasonTypeString(StateChangeReason::STATE_CHANGE_REASON_MESSAGE_NOTIFICATION),
        "MESSAGE_NOTIFICATION");
    EXPECT_EQ(PowerUtils::GetReasonTypeString(StateChangeReason::STATE_CHANGE_REASON_BLUETOOTH_INCOMING_CALL),
        "BLUETOOTH_INCOMING_CALL");
    EXPECT_EQ(PowerUtils::GetReasonTypeString(StateChangeReason::STATE_CHANGE_REASON_PICKUP),
        "PICKUP");
    EXPECT_EQ(PowerUtils::GetReasonTypeString(StateChangeReason::STATE_CHANGE_REASON_INCOMING_CALL),
        "INCOMING_CALL");
    EXPECT_EQ(PowerUtils::GetReasonTypeString(StateChangeReason::STATE_CHANGE_REASON_POWER_KEY),
        "POWER_KEY");
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService040 end!");
}

/**
 * @tc.name: LockScreenAfterTimingOutWithAppidTest001
 * @tc.desc: test LockScreenAfterTimingOutWithAppid scenario:
 * step1: Wireless proj calling LockScreenAfterTimingOut(true, true false); Result: true, true, false.
 * step2: WelinkPC calling LockScreenAfterTimingOutWithAppid(appid, false); Result: false, true, false.
 * step3: WelinkPC ends its task(or has died) and restore changes made with it. Result: true, true, false.
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceTest, LockScreenAfterTimingOutWithAppidTest001, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::LockScreenAfterTimingOutWithAppidTest001 start!");
    sptr<IPCObjectProxy> wirelessProxy = sptr<IPCObjectProxy>::MakeSptr(0, std::u16string {u"wireless"});
    sptr<IPCObjectProxy> AppProxy = sptr<IPCObjectProxy>::MakeSptr(1, std::u16string {u"App"});
    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    auto stateMachine = pmsTest_->GetPowerStateMachine();
    auto result = std::tie(stateMachine->enabledTimingOutLockScreen_,
        stateMachine->enabledTimingOutLockScreenCheckLock_, stateMachine->enabledScreenOffEvent_);
    // step1
    pmsTest_->LockScreenAfterTimingOut(true, true, false, wirelessProxy);
    EXPECT_EQ(result, (std::tuple {true, true, false}));

    // step2
    pmsTest_->LockScreenAfterTimingOutWithAppid(0, false, AppProxy);
    EXPECT_EQ(result, (std::tuple {false, true, false}));

    // step3
    pmsTest_->LockScreenAfterTimingOutWithAppid(0, true, AppProxy);
    EXPECT_EQ(result, (std::tuple {true, true, false}));

    // make all clients die and reset to default
    AppProxy->SendObituary();
    wirelessProxy->SendObituary();
    EXPECT_EQ(result, (std::tuple {true, false, true}));
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::LockScreenAfterTimingOutWithAppidTest001 end!");
}

/**
 * @tc.name: LockScreenAfterTimingOutWithAppidTest002
 * @tc.desc: test LockScreenAfterTimingOutWithAppid scenario:
 * step1: Wireless proj calling LockScreenAfterTimingOut(true, true false); Result: true, true, false.
 * step2: WelinkPC calling LockScreenAfterTimingOutWithAppid(appid, false); Result: false, true, false.
 * step3: Wireless proj ends its task(or has died) and restore changes made with it. Result: false, false, false.
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceTest, LockScreenAfterTimingOutWithAppidTest002, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::LockScreenAfterTimingOutWithAppidTest002 start!");
    sptr<IPCObjectProxy> wirelessProxy = sptr<IPCObjectProxy>::MakeSptr(0, std::u16string {u"wireless"});
    sptr<IPCObjectProxy> AppProxy = sptr<IPCObjectProxy>::MakeSptr(1, std::u16string {u"App"});
    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    auto stateMachine = pmsTest_->GetPowerStateMachine();
    auto result = std::tie(stateMachine->enabledTimingOutLockScreen_,
        stateMachine->enabledTimingOutLockScreenCheckLock_, stateMachine->enabledScreenOffEvent_);
    // step1
    pmsTest_->LockScreenAfterTimingOut(true, true, false, wirelessProxy);
    EXPECT_EQ(result, (std::tuple {true, true, false}));

    // step2
    pmsTest_->LockScreenAfterTimingOutWithAppid(0, false, AppProxy);
    EXPECT_EQ(result, (std::tuple {false, true, false}));

    // step3
    pmsTest_->LockScreenAfterTimingOut(true, false, true, wirelessProxy);
    EXPECT_EQ(result, (std::tuple {false, false, false}));
    
    // make all clients die and reset to default
    AppProxy->SendObituary();
    wirelessProxy->SendObituary();
    EXPECT_EQ(result, (std::tuple {true, false, true}));
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::LockScreenAfterTimingOutWithAppidTest002 end!");
}

/**
 * @tc.name: LockScreenAfterTimingOutWithAppidTest003
 * @tc.desc: test LockScreenAfterTimingOutWithAppid scenario:
 * step1: WelinkPC calling LockScreenAfterTimingOutWithAppid(appid, false); Result: false, false, false.
 * step2: Wireless proj calling LockScreenAfterTimingOut(true, true false); Result: false, true, false.
 * step3: WelinkPC ends its task(or has died) and restore changes made with it. Result: true, true, false.
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceTest, LockScreenAfterTimingOutWithAppidTest003, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::LockScreenAfterTimingOutWithAppidTest003 start!");
    sptr<IPCObjectProxy> wirelessProxy = sptr<IPCObjectProxy>::MakeSptr(0, std::u16string {u"wireless"});
    sptr<IPCObjectProxy> AppProxy = sptr<IPCObjectProxy>::MakeSptr(1, std::u16string {u"App"});
    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    auto stateMachine = pmsTest_->GetPowerStateMachine();
    auto result = std::tie(stateMachine->enabledTimingOutLockScreen_,
        stateMachine->enabledTimingOutLockScreenCheckLock_, stateMachine->enabledScreenOffEvent_);

    // step1
    pmsTest_->LockScreenAfterTimingOutWithAppid(0, false, AppProxy);
    EXPECT_EQ(result, (std::tuple {false, false, false}));

    // step2
    pmsTest_->LockScreenAfterTimingOut(true, true, false, wirelessProxy);
    EXPECT_EQ(result, (std::tuple {false, true, false}));

    // step3
    pmsTest_->LockScreenAfterTimingOutWithAppid(0, true, AppProxy);
    EXPECT_EQ(result, (std::tuple {true, true, false}));

    // make all clients die and reset to default
    AppProxy->SendObituary();
    wirelessProxy->SendObituary();
    EXPECT_EQ(result, (std::tuple {true, false, true}));
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::LockScreenAfterTimingOutWithAppidTest003 end!");
}

/**
 * @tc.name: LockScreenAfterTimingOutWithAppidTest004
 * @tc.desc: test LockScreenAfterTimingOutWithAppid scenario:
 * step1: WelinkPC calling LockScreenAfterTimingOutWithAppid(appid, false); Result: false, false, false.
 * step2: Wireless proj calling LockScreenAfterTimingOut(true, true false); Result: false, true, false.
 * step3: Wireless proj ends its task(or has died) and restore changes made with it. Result: false, false, false.
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceTest, LockScreenAfterTimingOutWithAppidTest004, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::LockScreenAfterTimingOutWithAppidTest004 start!");
    sptr<IPCObjectProxy> wirelessProxy = sptr<IPCObjectProxy>::MakeSptr(0, std::u16string {u"wireless"});
    sptr<IPCObjectProxy> AppProxy = sptr<IPCObjectProxy>::MakeSptr(1, std::u16string {u"App"});
    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    auto stateMachine = pmsTest_->GetPowerStateMachine();
    auto result = std::tie(stateMachine->enabledTimingOutLockScreen_,
        stateMachine->enabledTimingOutLockScreenCheckLock_, stateMachine->enabledScreenOffEvent_);

    // step1
    pmsTest_->LockScreenAfterTimingOutWithAppid(0, false, AppProxy);
    EXPECT_EQ(result, (std::tuple {false, false, false}));

    // step2
    pmsTest_->LockScreenAfterTimingOut(true, true, false, wirelessProxy);
    EXPECT_EQ(result, (std::tuple {false, true, false}));

    // step3
    pmsTest_->LockScreenAfterTimingOut(true, false, true, wirelessProxy);
    EXPECT_EQ(result, (std::tuple {false, false, false}));

    // make all clients die and reset to default
    AppProxy->SendObituary();
    wirelessProxy->SendObituary();
    EXPECT_EQ(result, (std::tuple {true, false, true}));
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::LockScreenAfterTimingOutWithAppidTest004 end!");
}

/**
 * @tc.name: LockScreenAfterTimingOutWithAppidTest005(case 006 is actually the same)
 * @tc.desc: test LockScreenAfterTimingOutWithAppid scenario:
 * step1: WelinkPC calling LockScreenAfterTimingOutWithAppid(appid, false); Result: false, false, false.
 * step2: App A calling LockScreenAfterTimingOutWithAppid(appid_A, false); Result: false, false, false.
 * step3: WelinkPC ends its task(or has died) and restore changes made with it. Result: false, false, false.
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceTest, LockScreenAfterTimingOutWithAppidTest005, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::LockScreenAfterTimingOutWithAppidTest005 start!");
    sptr<IPCObjectProxy> AppProxy = sptr<IPCObjectProxy>::MakeSptr(1, std::u16string {u"App"});
    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    auto stateMachine = pmsTest_->GetPowerStateMachine();
    auto result = std::tie(stateMachine->enabledTimingOutLockScreen_,
        stateMachine->enabledTimingOutLockScreenCheckLock_, stateMachine->enabledScreenOffEvent_);
    
    // only one process(client), no need to set fake pid.
    // step1
    pmsTest_->LockScreenAfterTimingOutWithAppid(0, false, AppProxy);
    EXPECT_EQ(result, (std::tuple {false, false, false}));

    // step2
    pmsTest_->LockScreenAfterTimingOutWithAppid(1, false, AppProxy);
    EXPECT_EQ(result, (std::tuple {false, false, false}));

    // step3
    pmsTest_->LockScreenAfterTimingOutWithAppid(0, true, AppProxy);
    EXPECT_EQ(result, (std::tuple {false, false, false}));

    // make all clients die and reset to default
    AppProxy->SendObituary();
    EXPECT_EQ(result, (std::tuple {true, false, true}));
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::LockScreenAfterTimingOutWithAppidTest005 end!");
}

/**
 * @tc.name: LockScreenAfterTimingOutWithAppidTest007
 * @tc.desc: test LockScreenAfterTimingOutWithAppid scenario:
 * step1: WelinkPC calling LockScreenAfterTimingOutWithAppid(appid, false); Result: false, false, false.
 * step2: Wireless proj calling LockScreenAfterTimingOut(true, true false); Result: false, true, false
 * step3: App A calling LockScreenAfterTimingOutWithAppid(appid_A, false); Result: false, true, false.
 * step4: Wireless proj ends its task(or has died) and restore changes made with it. Result: false, false, false
 * step5: WelinkPC ends its task(or has died) and restore changes made with it. Result: false, false, false.
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceTest, LockScreenAfterTimingOutWithAppidTest007, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::LockScreenAfterTimingOutWithAppidTest007 start!");
    sptr<IPCObjectProxy> wirelessProxy = sptr<IPCObjectProxy>::MakeSptr(0, std::u16string {u"wireless"});
    sptr<IPCObjectProxy> AppProxy = sptr<IPCObjectProxy>::MakeSptr(1, std::u16string {u"App"});
    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    auto stateMachine = pmsTest_->GetPowerStateMachine();
    auto result = std::tie(stateMachine->enabledTimingOutLockScreen_,
        stateMachine->enabledTimingOutLockScreenCheckLock_, stateMachine->enabledScreenOffEvent_);
    
    // step1
    pmsTest_->LockScreenAfterTimingOutWithAppid(0, false, AppProxy);
    EXPECT_EQ(result, (std::tuple {false, false, false}));

    // step2
    pmsTest_->LockScreenAfterTimingOut(true, true, false, wirelessProxy);
    EXPECT_EQ(result, (std::tuple {false, true, false}));

    // step3
    pmsTest_->LockScreenAfterTimingOutWithAppid(1, false, AppProxy);
    EXPECT_EQ(result, (std::tuple {false, true, false}));

    // step4
    pmsTest_->LockScreenAfterTimingOut(true, false, true, wirelessProxy);
    EXPECT_EQ(result, (std::tuple {false, false, false}));

    // step5
    pmsTest_->LockScreenAfterTimingOutWithAppid(0, false, AppProxy);
    EXPECT_EQ(result, (std::tuple {false, false, false}));

    // make all clients die and reset to default
    AppProxy->SendObituary();
    wirelessProxy->SendObituary();
    EXPECT_EQ(result, (std::tuple {true, false, true}));
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::LockScreenAfterTimingOutWithAppidTest007 end!");
}

/**
 * @tc.name: LockScreenAfterTimingOutWithAppidTestAbnormal001
 * @tc.desc: cover abnormal branches
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceTest, LockScreenAfterTimingOutAbnormal001, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::LockScreenAfterTimingOutAbnormal001 start!");
    sptr<IPCObjectProxy> testProxy = sptr<IPCObjectProxy>::MakeSptr(0, std::u16string {u"test"});
    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    std::shared_ptr<PowerStateMachine> backup = pmsTest_->GetPowerStateMachine();
    auto result = std::tie(backup->enabledTimingOutLockScreen_, backup->enabledTimingOutLockScreenCheckLock_,
        backup->enabledScreenOffEvent_);
    EXPECT_EQ(result, (std::tuple {true, false, true}));
    pmsTest_->powerStateMachine_ = nullptr;
    pmsTest_->LockScreenAfterTimingOut(false, true, false, testProxy);
    pmsTest_->LockScreenAfterTimingOut(false, true, false, nullptr);
    pmsTest_->powerStateMachine_ = backup;
    EXPECT_EQ(result, (std::tuple {true, false, true}));

    pmsTest_->LockScreenAfterTimingOut(true, false, true, testProxy); // reset to default
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceTest::LockScreenAfterTimingOutWithAppidTestAbnormal001 end!");
}
}