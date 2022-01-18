/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "power_mgr_st_mock_test.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;
using ::testing::_;

static sptr<PowerMgrService> g_service;
static MockStateAction* g_stateAction;
static MockPowerAction* g_powerAction;
static MockLockAction* g_lockAction;

static void ResetMockAction()
{
    POWER_HILOGD(MODULE_SERVICE, "ResetMockAction:Start.");
    g_stateAction = new MockStateAction();
    g_powerAction = new MockPowerAction();
    g_lockAction = new MockLockAction();
    g_service->EnableMock(g_stateAction, g_powerAction, g_lockAction);
}

void PowerMgrSTMockTest::SetUpTestCase(void)
{
    // create singleton service object at the beginning
    g_service = DelayedSpSingleton<PowerMgrService>::GetInstance();
    g_service->OnStart();
    ResetMockAction();
}

void PowerMgrSTMockTest::TearDownTestCase(void)
{
    g_service->OnStop();
    DelayedSpSingleton<PowerMgrService>::DestroyInstance();
    delete g_stateAction;
    delete g_powerAction;
    delete g_lockAction;
}

void PowerMgrSTMockTest::SetUp(void)
{
}

void PowerMgrSTMockTest::TearDown(void)
{
}

namespace {
/**
 * @tc.name: PowerMgrMock001
 * @tc.desc: test RebootDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock001, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock001: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock001:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock001: Failed to get PowerMgrService";
    }

    EXPECT_CALL(*g_powerAction, Reboot(std::string("test"))).Times(1);
    pms->RebootDevice(std::string("test"));
    sleep(ASYNC_WAIT_TIME_S);

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock001:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock001: end.";
}

/**
 * @tc.name: PowerMgrMock002
 * @tc.desc: test ShutDownDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock002, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock002: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock002:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock002: Failed to get PowerMgrService";
    }

    EXPECT_CALL(*g_powerAction, Shutdown(std::string("test"))).Times(1);
    pms->ShutDownDevice(std::string("test"));
    sleep(ASYNC_WAIT_TIME_S);

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock002:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock002: end.";
}


/**
 * @tc.name: PowerMgrMock003
 * @tc.desc: test SuspendDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock003, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock003: start.";

    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock003:Start.");
    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock003: Failed to get PowerMgrService";
    }

    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock003:Start mock.");
    EXPECT_CALL(*g_stateAction,   SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction,
        Suspend(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false));
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock003:Start suspend.");
    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock003:end.");
    GTEST_LOG_(INFO) << "PowerMgrMock003: end.";
}

/**
 * @tc.name: PowerMgrMock004
 * @tc.desc: test SuspendDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock004, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock004: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock004:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock004: Failed to get PowerMgrService";
    }

    EXPECT_CALL(*g_stateAction,   SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction,
        Suspend(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_DEVICE_ADMIN, false));
    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_DEVICE_ADMIN, false);

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock004:End .");
    GTEST_LOG_(INFO) << "PowerMgrMock004: end.";
}

/**
 * @tc.name: PowerMgrMock005
 * @tc.desc: test SuspendDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock005, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock005: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock005:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock005: Failed to get PowerMgrService";
    }

    EXPECT_CALL(*g_stateAction,   SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction,
        Suspend(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_TIMEOUT, false));
    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_TIMEOUT, false);

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock005:End");
    GTEST_LOG_(INFO) << "PowerMgrMock005: end.";
}

/**
 * @tc.name: PowerMgrMock006
 * @tc.desc: test SuspendDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock006, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock006: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock006:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock006: Failed to get PowerMgrService";
    }

    EXPECT_CALL(*g_stateAction,   SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction,
        Suspend(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_LID_SWITCH, false));
    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_LID_SWITCH, false);

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock006:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock006: end.";
}

/**
 * @tc.name: PowerMgrMock007
 * @tc.desc: test SuspendDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock007, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock007: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock007:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock007: Failed to get PowerMgrService";
    }

    EXPECT_CALL(*g_stateAction,   SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction,
        Suspend(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_POWER_BUTTON, false));
    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_POWER_BUTTON, false);

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock007:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock007: end.";
}

/**
 * @tc.name: PowerMgrMock008
 * @tc.desc: test SuspendDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock008, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock008: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock008:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock008: Failed to get PowerMgrService";
    }

    EXPECT_CALL(*g_stateAction,   SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction,
        Suspend(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_HDMI, false));
    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_HDMI, false);

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock008:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock008: end.";
}

/**
 * @tc.name: PowerMgrMock009
 * @tc.desc: test SuspendDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock009, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock009: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock009:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock009: Failed to get PowerMgrService";
    }

    EXPECT_CALL(*g_stateAction,   SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction,
        Suspend(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_SLEEP_BUTTON, false));
    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_SLEEP_BUTTON, false);

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock009:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock009: end.";
}

/**
 * @tc.name: PowerMgrMock010
 * @tc.desc: test SuspendDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock010, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock010: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock010:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock010: Failed to get PowerMgrService";
    }

    EXPECT_CALL(*g_stateAction,   SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction,
        Suspend(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_ACCESSIBILITY, false));
    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_ACCESSIBILITY, false);

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock010:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock010: end.";
}

/**
 * @tc.name: PowerMgrMock011
 * @tc.desc: test SuspendDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock011, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock011: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock011:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock011: Failed to get PowerMgrService";
    }

    EXPECT_CALL(*g_stateAction,   SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction,
        Suspend(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_FORCE_SUSPEND, false));
    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_FORCE_SUSPEND, false);

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock011:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock011: end.";
}

/**
 * @tc.name: PowerMgrMock012
 * @tc.desc: test WakeupDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock012, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock012:  start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock012:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock012: Failed to get PowerMgrService";
    }

    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    EXPECT_CALL(*g_stateAction,  SetDisplayState(DisplayState::DISPLAY_ON, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction,
        Wakeup(0, WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN,
        std::string("test"), _)).Times(1);
    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN, std::string("test"));

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock012:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock012: end.";
}

/**
 * @tc.name: PowerMgrMock013
 * @tc.desc: test WakeupDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock013, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock013:  start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock013:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock013: Failed to get PowerMgrService";
    }

    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    EXPECT_CALL(*g_stateAction,  SetDisplayState(DisplayState::DISPLAY_ON, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction,
        Wakeup(0, WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON,
        std::string("test"), _)).Times(1);
    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON, std::string("test"));

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock013:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock013: end.";
}

/**
 * @tc.name: PowerMgrMock014
 * @tc.desc: test WakeupDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock014, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock014:  start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock014:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock014: Failed to get PowerMgrService";
    }

    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    EXPECT_CALL(*g_stateAction,  SetDisplayState(DisplayState::DISPLAY_ON, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction,
        Wakeup(0, WakeupDeviceType::WAKEUP_DEVICE_APPLICATION,
        std::string("test"), _)).Times(1);
    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_APPLICATION, std::string("test"));

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock014:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock014: end.";
}

/**
 * @tc.name: PowerMgrMock015
 * @tc.desc: test WakeupDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock015, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock015:  start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock015:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock015: Failed to get PowerMgrService";
    }

    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    EXPECT_CALL(*g_stateAction,  SetDisplayState(DisplayState::DISPLAY_ON, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction,
        Wakeup(0, WakeupDeviceType::WAKEUP_DEVICE_PLUGGED_IN,
        std::string("test"), _)).Times(1);
    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_PLUGGED_IN, std::string("test"));

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock015:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock015: end.";
}

/**
 * @tc.name: PowerMgrMock016
 * @tc.desc: test WakeupDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock016, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock016:  start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock016:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock016: Failed to get PowerMgrService";
    }

    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    EXPECT_CALL(*g_stateAction,  SetDisplayState(DisplayState::DISPLAY_ON, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction,
        Wakeup(0, WakeupDeviceType::WAKEUP_DEVICE_GESTURE,
        std::string("test"), _)).Times(1);
    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_GESTURE, std::string("test"));

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock016:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock016: end.";
}

/**
 * @tc.name: PowerMgrMock017
 * @tc.desc: test WakeupDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock017, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock017:  start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock017:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock017: Failed to get PowerMgrService";
    }

    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    EXPECT_CALL(*g_stateAction,  SetDisplayState(DisplayState::DISPLAY_ON, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction,
        Wakeup(0, WakeupDeviceType::WAKEUP_DEVICE_CAMERA_LAUNCH,
        std::string("test"), _)).Times(1);
    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_CAMERA_LAUNCH, std::string("test"));

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock017:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock017: end.";
}

/**
 * @tc.name: PowerMgrMock018
 * @tc.desc: test WakeupDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock018, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock018:  start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock018:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock018: Failed to get PowerMgrService";
    }

    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    EXPECT_CALL(*g_stateAction,  SetDisplayState(DisplayState::DISPLAY_ON, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction,
        Wakeup(0, WakeupDeviceType::WAKEUP_DEVICE_WAKE_KEY,
        std::string("test"), _)).Times(1);
    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_WAKE_KEY, std::string("test"));

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock018:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock018: end.";
}

/**
 * @tc.name: PowerMgrMock019
 * @tc.desc: test WakeupDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock019, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock019:  start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock019:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock019: Failed to get PowerMgrService";
    }

    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    EXPECT_CALL(*g_stateAction,  SetDisplayState(DisplayState::DISPLAY_ON, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction,
        Wakeup(0, WakeupDeviceType::WAKEUP_DEVICE_WAKE_MOTION,
        std::string("test"), _)).Times(1);
    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_WAKE_MOTION, std::string("test"));

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock019:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock019: end.";
}

/**
 * @tc.name: PowerMgrMock020
 * @tc.desc: test WakeupDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock020, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock020:  start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock020:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock020: Failed to get PowerMgrService";
    }

    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    EXPECT_CALL(*g_stateAction,  SetDisplayState(DisplayState::DISPLAY_ON, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction,
        Wakeup(0, WakeupDeviceType::WAKEUP_DEVICE_HDMI,
        std::string("test"), _)).Times(1);
    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_HDMI, std::string("test"));

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock020:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock020: end.";
}

/**
 * @tc.name: PowerMgrMock021
 * @tc.desc: test WakeupDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock021, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock021:  start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock021:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock021: Failed to get PowerMgrService";
    }

    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    EXPECT_CALL(*g_stateAction,  SetDisplayState(DisplayState::DISPLAY_ON, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction,
        Wakeup(0, WakeupDeviceType::WAKEUP_DEVICE_LID,
        std::string("test"), _)).Times(1);
    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_LID, std::string("test"));

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock021:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock021: end.";
}

/**
 * @tc.name: PowerMgrMock022
 * @tc.desc: test SuspendDevice and auto sleep by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock022, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock022: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock022:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock022: Failed to get PowerMgrService";
    }

    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN, std::string("test"));
    EXPECT_CALL(*g_stateAction,   SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction,
        Suspend(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false));
    EXPECT_CALL(*g_stateAction, GoToSleep(_, _, false))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_OFF));
    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    sleep(SLEEP_WAIT_TIME_S + 1);

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock022:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock022: end.";
}

/**
 * @tc.name: PowerMgrMock023
 * @tc.desc: test WakeupDevice and auto suspend and sleep by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock023, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock023: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock023:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock023: Failed to get PowerMgrService";
    }

    EXPECT_CALL(*g_stateAction, GoToSleep(_, _, false))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_ON));
    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_LID, std::string("test"));
    sleep((SCREEN_OFF_WAIT_TIME_S*2/3) + 1);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_DIM));
    sleep((SCREEN_OFF_WAIT_TIME_S*1/3) + 1);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_OFF));
    sleep(SLEEP_WAIT_TIME_S + 1);

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock023:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock023: end.";
}

/**
 * @tc.name: PowerMgrMock024
 * @tc.desc: test RefreshActivity by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock024, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock024: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock024:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock024: Failed to get PowerMgrService";
    }

    EXPECT_CALL(*g_stateAction, GetDisplayState())
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(DisplayState::DISPLAY_DIM));
    EXPECT_CALL(*g_stateAction,  SetDisplayState(DisplayState::DISPLAY_ON, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction,
        RefreshActivity(0, UserActivityType::USER_ACTIVITY_TYPE_TOUCH, _)).Times(1);
    pms->RefreshActivity(0, UserActivityType::USER_ACTIVITY_TYPE_TOUCH, true);

    sleep(SCREEN_OFF_WAIT_TIME_S + SLEEP_WAIT_TIME_S + 1);
    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock024:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock024: end.";
}

/**
 * @tc.name: PowerMgrMock025
 * @tc.desc: test IsScreenOn by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock025, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock025: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock025:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock025: Failed to get PowerMgrService";
    }

    EXPECT_CALL(*g_stateAction, GetDisplayState())
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_EQ(pms->IsScreenOn(), true);

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock025:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock025: end.";
}

/**
 * @tc.name: PowerMgrMock026
 * @tc.desc: test IsScreenOn by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock026, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock026: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock026:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock026: Failed to get PowerMgrService";
    }

    EXPECT_CALL(*g_stateAction, GetDisplayState())
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(DisplayState::DISPLAY_DIM));
    EXPECT_EQ(pms->IsScreenOn(), true);

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock026:End");
    GTEST_LOG_(INFO) << "PowerMgrMock026: end.";
}

/**
 * @tc.name: PowerMgrMock027
 * @tc.desc: test IsScreenOn by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock027, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock027: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock027:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock027: Failed to get PowerMgrService";
    }

    EXPECT_CALL(*g_stateAction, GetDisplayState())
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(DisplayState::DISPLAY_OFF));
    EXPECT_EQ(pms->IsScreenOn(), false);

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock027:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock027: end.";
}

/**
 * @tc.name: PowerMgrMock028
 * @tc.desc: test IsScreenOn by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock028, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock028: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock028:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock028: Failed to get PowerMgrService";
    }

    EXPECT_CALL(*g_stateAction, GetDisplayState())
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(DisplayState::DISPLAY_SUSPEND));
    EXPECT_EQ(pms->IsScreenOn(), false);

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock028:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock028: end.";
}

/**
 * @tc.name: PowerMgrMock029
 * @tc.desc: test ForceSuspendDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock029, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock029: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock029:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock029: Failed to get PowerMgrService";
    }

    EXPECT_CALL(*g_stateAction, GoToSleep(_, _, true))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_EQ(pms->ForceSuspendDevice(0), true);

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock029:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock029: end.";
}

/**
 * @tc.name: PowerMgrMock030
 * @tc.desc: test Screen on RunningLock by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock030, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock030: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock030:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock030: Failed to get PowerMgrService";
    }

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_SCREEN);
    pms->CreateRunningLock(token, info);
    pms->Lock(token, info, 0);
    EXPECT_EQ(pms->IsUsed(token), true);

    EXPECT_CALL(*g_stateAction,  SetDisplayState(DisplayState::DISPLAY_DIM, ::testing::_)).Times(0);
    sleep(SCREEN_DIM_WAIT_TIME_S + 1);

    pms->UnLock(token);
    EXPECT_EQ(pms->IsUsed(token), false);

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock030:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock030: end.";
}

/**
 * @tc.name: PowerMgrMock031
 * @tc.desc: test background RunningLock by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock031, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock031: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock031:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock031: Failed to get PowerMgrService";
    }

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_BACKGROUND);
    pms->CreateRunningLock(token, info);

    EXPECT_CALL(*g_lockAction, Lock(_, _)).Times(1);
    pms->Lock(token, info, 0);
    EXPECT_EQ(pms->IsUsed(token), true);

    EXPECT_CALL(*g_stateAction, GoToSleep(_, _, _)).Times(0);
    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    sleep(SLEEP_WAIT_TIME_S + 1);

    EXPECT_CALL(*g_lockAction, Unlock(_, _)).Times(1);
    pms->UnLock(token);
    EXPECT_EQ(pms->IsUsed(token), false);

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock031:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock031: end.";
}

/**
 * @tc.name: PowerMgrMock032
 * @tc.desc: test proximity RunningLock by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock032, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock032: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock032:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock032: Failed to get PowerMgrService";
    }

    EXPECT_CALL(*g_stateAction,  SetDisplayState(DisplayState::DISPLAY_DIM, ::testing::_)).Times(0);
    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL);
    pms->CreateRunningLock(token, info);

    EXPECT_CALL(*g_lockAction, Lock(_, _)).Times(1);
    pms->Lock(token, info, 0);
    EXPECT_EQ(pms->IsUsed(token), true);

    EXPECT_CALL(*g_stateAction,  SetDisplayState(DisplayState::DISPLAY_DIM, ::testing::_)).Times(0);
    sleep(SCREEN_DIM_WAIT_TIME_S + 1);

    EXPECT_CALL(*g_lockAction, Unlock(_, _)).Times(1);
    pms->UnLock(token);
    EXPECT_EQ(pms->IsUsed(token), false);

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock032:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock032: end.";
}

/**
 * @tc.name: PowerMgrMock033
 * @tc.desc: test RunningLock release by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock033, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock033: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock033:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock033: Failed to get PowerMgrService";
    }

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_BACKGROUND);
    pms->CreateRunningLock(token, info);
    pms->ReleaseRunningLock(token);

    EXPECT_CALL(*g_lockAction, Lock(_, _)).Times(0);
    pms->Lock(token, info, 0);
    EXPECT_EQ(pms->IsUsed(token), false);

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock033:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock033: end.";
}

/**
 * @tc.name: PowerMgrMock034
 * @tc.desc: test RefreshActivity by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock034, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock034: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock034:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock034: Failed to get PowerMgrService";
    }

    EXPECT_CALL(*g_stateAction, GetDisplayState())
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(DisplayState::DISPLAY_DIM));
    EXPECT_CALL(*g_stateAction,  SetDisplayState(DisplayState::DISPLAY_ON, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction,
        RefreshActivity(0, UserActivityType::USER_ACTIVITY_TYPE_OTHER, _)).Times(1);
    pms->RefreshActivity(0, UserActivityType::USER_ACTIVITY_TYPE_OTHER, true);

    sleep(SCREEN_OFF_WAIT_TIME_S + SLEEP_WAIT_TIME_S + 1);
    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock034:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock034: end.";
}

/**
 * @tc.name: PowerMgrMock035
 * @tc.desc: test RefreshActivity by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock035, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock035: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock035:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock035: Failed to get PowerMgrService";
    }

    EXPECT_CALL(*g_stateAction, GetDisplayState())
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(DisplayState::DISPLAY_DIM));
    EXPECT_CALL(*g_stateAction,  SetDisplayState(DisplayState::DISPLAY_ON, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction,
        RefreshActivity(0, UserActivityType::USER_ACTIVITY_TYPE_BUTTON, _)).Times(1);
    pms->RefreshActivity(0, UserActivityType::USER_ACTIVITY_TYPE_BUTTON, true);

    sleep(SCREEN_OFF_WAIT_TIME_S + SLEEP_WAIT_TIME_S + 1);
    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock035:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock035: end.";
}

/**
 * @tc.name: PowerMgrMock036
 * @tc.desc: test RefreshActivity by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock036, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock036: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock036:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock036: Failed to get PowerMgrService";
    }

    EXPECT_CALL(*g_stateAction, GetDisplayState())
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(DisplayState::DISPLAY_DIM));
    EXPECT_CALL(*g_stateAction,  SetDisplayState(DisplayState::DISPLAY_ON, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction,
        RefreshActivity(0, UserActivityType::USER_ACTIVITY_TYPE_ACCESSIBILITY, _)).Times(1);
    pms->RefreshActivity(0, UserActivityType::USER_ACTIVITY_TYPE_ACCESSIBILITY, true);

    sleep(SCREEN_OFF_WAIT_TIME_S + SLEEP_WAIT_TIME_S + 1);
    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock036:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock036: end.";
}

/**
 * @tc.name: PowerMgrMock037
 * @tc.desc: test RefreshActivity by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock037, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock037: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock037:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock037: Failed to get PowerMgrService";
    }

    EXPECT_CALL(*g_stateAction, GetDisplayState())
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(DisplayState::DISPLAY_DIM));
    EXPECT_CALL(*g_stateAction,  SetDisplayState(DisplayState::DISPLAY_ON, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction,
        RefreshActivity(0, UserActivityType::USER_ACTIVITY_TYPE_ATTENTION, _)).Times(1);
    pms->RefreshActivity(0, UserActivityType::USER_ACTIVITY_TYPE_ATTENTION, true);

    sleep(SCREEN_OFF_WAIT_TIME_S + SLEEP_WAIT_TIME_S + 1);
    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock037:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock037: end.";
}

/**
 * @tc.name: PowerMgrMock038
 * @tc.desc: test RefreshActivity by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock038, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock038: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock038:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock038: Failed to get PowerMgrService";
    }

    EXPECT_CALL(*g_stateAction, GetDisplayState())
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(DisplayState::DISPLAY_DIM));
    EXPECT_CALL(*g_stateAction,  SetDisplayState(DisplayState::DISPLAY_ON, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction,
        RefreshActivity(0, UserActivityType::USER_ACTIVITY_TYPE_SOFTWARE, _)).Times(1);
    pms->RefreshActivity(0, UserActivityType::USER_ACTIVITY_TYPE_SOFTWARE, true);

    sleep(SCREEN_OFF_WAIT_TIME_S + SLEEP_WAIT_TIME_S + 1);
    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock038:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock038: end.";
}

/**
 * @tc.name: PowerMgrMock039
 * @tc.desc: test RefreshActivity by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock039, TestSize.Level2)
{
    UserActivityType abnormaltype= {};
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock039: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock039:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock039: Failed to get PowerMgrService";
    }
    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN, std::string("test"));
    EXPECT_CALL(*g_stateAction, GetDisplayState())
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(DisplayState::DISPLAY_DIM));
    EXPECT_CALL(*g_stateAction,
        RefreshActivity(0, _, _)).Times(1);
    pms->RefreshActivity(0, abnormaltype, true);

    sleep(SLEEP_WAIT_TIME_S + 1);
    EXPECT_EQ(pms->IsScreenOn(), false);

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock039:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock039: end.";
}

/**
 * @tc.name: PowerMgrMock040
 * @tc.desc: test RefreshActivity by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock040, TestSize.Level2)
{
    UserActivityType abnormaltype=UserActivityType(6);
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock040: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock040:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock040: Failed to get PowerMgrService";
    }
    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN, std::string("test"));
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_ON));
    sleep((SCREEN_OFF_WAIT_TIME_S*2/3) + 1);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_DIM));
    EXPECT_CALL(*g_stateAction,
        RefreshActivity(0, _, _)).Times(0);
    EXPECT_CALL(*g_stateAction,   SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    pms->RefreshActivity(0, abnormaltype, true);
    sleep((SCREEN_OFF_WAIT_TIME_S*1/3) + 1);

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock040:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock040: end.";
}

/**
 * @tc.name: PowerMgrMock041
 * @tc.desc: test WakeupDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock041, TestSize.Level2)
{
    WakeupDeviceType abnormaltype=WakeupDeviceType(10);
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock041:  start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock041:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock041: Failed to get PowerMgrService";
    }

    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    EXPECT_CALL(*g_stateAction,  SetDisplayState(DisplayState::DISPLAY_ON, ::testing::_))
        .Times(0);
    EXPECT_CALL(*g_stateAction,
        Wakeup(0, _,
        std::string("test"), _)).Times(0);
    pms->WakeupDevice(0, abnormaltype, std::string("test"));
    EXPECT_EQ(pms->IsScreenOn(), false);

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock041:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock041: end.";
}

/**
 * @tc.name: PowerMgrMock042
 * @tc.desc: test WakeupDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock042, TestSize.Level2)
{
    WakeupDeviceType abnormaltype=WakeupDeviceType(999);
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock042:  start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock042:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock042: Failed to get PowerMgrService";
    }

    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    EXPECT_CALL(*g_stateAction,  SetDisplayState(DisplayState::DISPLAY_ON, ::testing::_))
        .Times(0);
    EXPECT_CALL(*g_stateAction,
        Wakeup(0, _,
        std::string("test"), _)).Times(0);
    pms->WakeupDevice(0, abnormaltype, std::string("test"));
    EXPECT_EQ(PowerState::INACTIVE, pms->GetState());

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock042:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock042: end.";
}

/**
 * @tc.name: PowerMgrMock043
 * @tc.desc: test WakeupDevice and auto suspend  by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock043, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock043: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock043:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock043: Failed to get PowerMgrService";
    }

    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN, std::string("test"));
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_ON));
    sleep((SCREEN_OFF_WAIT_TIME_S*2/3) + 1);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_DIM));
    EXPECT_CALL(*g_stateAction,   SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    sleep((SCREEN_OFF_WAIT_TIME_S*1/3) + 1);

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock043:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock043: end.";
}

/**
 * @tc.name: PowerMgrMock044
 * @tc.desc: test WakeupDevice and auto suspend  by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock044, TestSize.Level2)
{
    WakeupDeviceType abnormaltype = WakeupDeviceType(10);
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock044: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock044:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock044: Failed to get PowerMgrService";
    }

    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN, std::string("test"));
    EXPECT_CALL(*g_stateAction,  SetDisplayState(DisplayState::DISPLAY_DIM, ::testing::_))
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_ON));
    sleep((SCREEN_OFF_WAIT_TIME_S*2/3)/2);
    pms->WakeupDevice(0, abnormaltype, std::string("test"));
    sleep((SCREEN_OFF_WAIT_TIME_S*2/3)/2 + 1);

    ResetMockAction();
    GTEST_LOG_(INFO) << "PowerMgrMock044: end.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock044:End.");
}

/**
 * @tc.name: PowerMgrMock045
 * @tc.desc: test WakeupDevice and auto suspend  by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock045, TestSize.Level2)
{
    WakeupDeviceType abnormaltype = WakeupDeviceType(999);
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock045: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock045:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock045: Failed to get PowerMgrService";
    }

    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN, std::string("test"));
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_ON));
    sleep((SCREEN_OFF_WAIT_TIME_S*2/3) + 1);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_DIM));
    EXPECT_CALL(*g_stateAction,   SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    sleep((SCREEN_OFF_WAIT_TIME_S*2/3) /2);
    pms->WakeupDevice(0, abnormaltype, std::string("test"));
    sleep((SCREEN_OFF_WAIT_TIME_S*1/3)/2 + 1);

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock045:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock045: end.";
}

/**
 * @tc.name: PowerMgrMock046
 * @tc.desc: test RefreshActivity by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock046, TestSize.Level2)
{
    UserActivityType abnormaltype = UserActivityType(6);
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock046: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock046:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock046: Failed to get PowerMgrService";
    }

    EXPECT_CALL(*g_stateAction, GetDisplayState())
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(DisplayState::DISPLAY_OFF));
    EXPECT_CALL(*g_stateAction,
        RefreshActivity(0, _, true)).Times(0);
    sleep(SCREEN_OFF_WAIT_TIME_S + SLEEP_WAIT_TIME_S/2);
    pms->RefreshActivity(0, abnormaltype, true);

    sleep(SCREEN_OFF_WAIT_TIME_S + SLEEP_WAIT_TIME_S/2 + 1);
    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock046:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock046: end.";
}

/**
 * @tc.name: PowerMgrMock047
 * @tc.desc: test RefreshActivity by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock047, TestSize.Level2)
{
    UserActivityType abnormaltype = {};
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock047: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock047:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock047: Failed to get PowerMgrService";
    }

    EXPECT_CALL(*g_stateAction, GetDisplayState())
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(DisplayState::DISPLAY_OFF));
    EXPECT_CALL(*g_stateAction,
        RefreshActivity(0, _, true)).Times(0);
    sleep(SCREEN_OFF_WAIT_TIME_S + SLEEP_WAIT_TIME_S/2);
    pms->RefreshActivity(0, abnormaltype, true);

    sleep(SCREEN_OFF_WAIT_TIME_S + SLEEP_WAIT_TIME_S/2 + 1);
    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock047:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock047: end.";
}

/**
 * @tc.name: PowerMgrMock048
 * @tc.desc: test SuspendDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock048, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock048: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock048:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock048: Failed to get PowerMgrService";
    }

    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON, std::string("test"));
    EXPECT_CALL(*g_stateAction,   SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction,
        Suspend(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false));
    EXPECT_CALL(*g_stateAction, GoToSleep(_, _, _))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    EXPECT_EQ(PowerState::INACTIVE, pms->GetState());
    sleep(SLEEP_WAIT_TIME_S+1);
    EXPECT_EQ(PowerState::SLEEP, pms->GetState());

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock048:End");
    GTEST_LOG_(INFO) << "PowerMgrMock048: end.";
}

/**
 * @tc.name: PowerMgrMock049
 * @tc.desc: test SuspendDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock049, TestSize.Level2)
{
    SuspendDeviceType abnormaltype = SuspendDeviceType(10);
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock049: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock049:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock049: Failed to get PowerMgrService";
    }

    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON, std::string("test"));
    EXPECT_CALL(*g_stateAction,
        Suspend(0, _, false)).Times(0);
    EXPECT_CALL(*g_stateAction,   SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_)).Times(0);
    pms->SuspendDevice(0, abnormaltype, false);
    EXPECT_EQ(PowerState::AWAKE, pms->GetState());

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock049:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock049: end.";
}

/**
 * @tc.name: PowerMgrMock050
 * @tc.desc: test SuspendDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock050, TestSize.Level2)
{
    SuspendDeviceType abnormaltype = SuspendDeviceType(999);
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock050: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock050:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock050: Failed to get PowerMgrService";
    }

    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON, std::string("test"));
    EXPECT_CALL(*g_stateAction,
        Suspend(0, _, false)).Times(0);
    EXPECT_CALL(*g_stateAction,   SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_)).Times(0);
    pms->SuspendDevice(0, abnormaltype, false);
    EXPECT_EQ(PowerState::AWAKE, pms->GetState());

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock050:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock050: end.";
}

/**
 * @tc.name: PowerMgrMock051
 * @tc.desc: test SuspendDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock051, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock051: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock051:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock051: Failed to get PowerMgrService";
    }

    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    EXPECT_CALL(*g_stateAction,   SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction,
        Suspend(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false));
    EXPECT_CALL(*g_stateAction, GoToSleep(_, _, false))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    EXPECT_EQ(PowerState::INACTIVE, pms->GetState());
    sleep(SLEEP_WAIT_TIME_S+1);
    EXPECT_EQ(PowerState::SLEEP, pms->GetState());

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock051:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock051: end.";
}

/**
 * @tc.name: PowerMgrMock052
 * @tc.desc: test SuspendDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock052, TestSize.Level2)
{
    SuspendDeviceType abnormaltype = SuspendDeviceType(10);
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock052: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock052:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock052: Failed to get PowerMgrService";
    }

    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    EXPECT_CALL(*g_stateAction,
        Suspend(0, _, false)).Times(0);
    pms->SuspendDevice(0, abnormaltype, false);
    EXPECT_EQ(PowerState::INACTIVE, pms->GetState());

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock052:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock052: end.";
}

/**
 * @tc.name: PowerMgrMock053
 * @tc.desc: test SuspendDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock053, TestSize.Level2)
{
    SuspendDeviceType abnormaltype = {};
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock053: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock053:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock053: Failed to get PowerMgrService";
    }

    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    EXPECT_CALL(*g_stateAction,
        Suspend(0, _, false));
    pms->SuspendDevice(0, abnormaltype, false);
    EXPECT_EQ(PowerState::INACTIVE, pms->GetState());

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock053:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock053: end.";
}

/**
 * @tc.name: PowerMgrMock054
 * @tc.desc: test ForceSuspendDevice by mock during Inactive
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock054, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock054: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock054:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock054: Failed to get PowerMgrService";
    }

    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    EXPECT_CALL(*g_stateAction, GoToSleep(_, _, _))
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    pms->ForceSuspendDevice(0);
    sleep(SLEEP_WAIT_TIME_S+1);
    EXPECT_EQ(PowerState::SLEEP, pms->GetState());

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock054:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock054: end.";
}

/**
 * @tc.name: PowerMgrMock055
 * @tc.desc: test  auto suspend  by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock055, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock055: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock055:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock055: Failed to get PowerMgrService";
    }

    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_LID, std::string("test"));
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_CALL(*g_stateAction,  SetDisplayState(DisplayState::DISPLAY_DIM, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    sleep((SCREEN_OFF_WAIT_TIME_S*2/3) + 1);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_DIM));
    EXPECT_CALL(*g_stateAction,   SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    sleep((SCREEN_OFF_WAIT_TIME_S*1/3) + 1);

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock055:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock055: end.";
}

/**
 * @tc.name: PowerMgrMock056
 * @tc.desc: test SCREEN_ON RunningLock by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock056, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock056: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock056:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock056: Failed to get PowerMgrService";
    }

    EXPECT_CALL(*g_stateAction,  SetDisplayState(DisplayState::DISPLAY_DIM, ::testing::_)).Times(0);
    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_SCREEN);
    pms->CreateRunningLock(token, info);

    pms->Lock(token, info, 0);
    EXPECT_EQ(pms->IsUsed(token), true);

    EXPECT_CALL(*g_stateAction,  SetDisplayState(DisplayState::DISPLAY_DIM, ::testing::_)).Times(0);
    sleep((SCREEN_OFF_WAIT_TIME_S*2/3) + 1);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_EQ(PowerState::AWAKE, pms->GetState());

    pms->UnLock(token);
    EXPECT_EQ(pms->IsUsed(token), false);
    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock056:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock056: end.";
}

/**
 * @tc.name: PowerMgrMock057
 * @tc.desc: test SCREEN_ON RunningLock by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock057, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock057: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock057:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock057: Failed to get PowerMgrService";
    }

    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_SCREEN);
    pms->CreateRunningLock(token, info);
    EXPECT_CALL(*g_stateAction,  SetDisplayState(DisplayState::DISPLAY_ON, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    pms->Lock(token, info, 0);
    EXPECT_EQ(pms->IsUsed(token), true);
    EXPECT_EQ(PowerState::AWAKE, pms->GetState());

    pms->UnLock(token);
    EXPECT_EQ(pms->IsUsed(token), false);
    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock057:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock057: end.";
}

/**
 * @tc.name: PowerMgrMock058
 * @tc.desc: test Auto DIM by mock after 10min
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock058, TestSize.Level2)
{
    int64_t time =600000;
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock058: start.";

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock058: Failed to get PowerMgrService";
    }
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_CALL(*g_stateAction,  SetDisplayState(DisplayState::DISPLAY_DIM, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction,   SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    pms->SetDisplayOffTime(time);
    sleep(time/1000-2);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_DIM));
    sleep(3);
    ResetMockAction();
pms->SetDisplayOffTime(DEFAULT_DISPLAY_OFF_TIME);
    GTEST_LOG_(INFO) << "PowerMgrMock058: end.";
}

/**
 * @tc.name: PowerMgrMock059
 * @tc.desc: test Screen RunningLock by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock059, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock059: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock059:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock059: Failed to get PowerMgrService";
    }

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_SCREEN);
    pms->CreateRunningLock(token, info);
    pms->Lock(token, info, 0);
    EXPECT_EQ(pms->IsUsed(token), true);

    EXPECT_CALL(*g_stateAction,  SetDisplayState(DisplayState::DISPLAY_DIM, ::testing::_)).Times(0);
    sleep(SCREEN_DIM_WAIT_TIME_S + 1);

    pms->UnLock(token);
    EXPECT_EQ(pms->IsUsed(token), false);
    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock059:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock059: end.";
}

/**
 * @tc.name: PowerMgrMock060
 * @tc.desc: test Screen RunningLock by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock060, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock060: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock060:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock060: Failed to get PowerMgrService";
    }

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_SCREEN);
    pms->CreateRunningLock(token, info);
    pms->Lock(token, info, 0);
    EXPECT_EQ(pms->IsUsed(token), true);

    EXPECT_CALL(*g_stateAction,   SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_)).Times(0);
    sleep(SLEEP_WAIT_TIME_S*10);
    EXPECT_CALL(*g_stateAction,   SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_)).Times(0);
    EXPECT_EQ(PowerState::AWAKE, pms->GetState());

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock060:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock060: end.";
}

/**
 * @tc.name: PowerMgrMock061
 * @tc.desc: test Screen on RunningLock by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock061, TestSize.Level2)
{
    int i;
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock061: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock061:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock061: Failed to get PowerMgrService";
    }

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_SCREEN);
    pms->CreateRunningLock(token, info);
    for (i=0; i<10; i++) {
        pms->Lock(token, info, 0);
        EXPECT_EQ(pms->IsUsed(token), true);
        EXPECT_CALL(*g_stateAction,  SetDisplayState(DisplayState::DISPLAY_DIM, ::testing::_)).Times(0);
        sleep(SCREEN_OFF_WAIT_TIME_S + 1);
        pms->UnLock(token);
        EXPECT_EQ(pms->IsUsed(token), false);
        EXPECT_EQ(PowerState::AWAKE, pms->GetState());
    }
    EXPECT_EQ(PowerState::AWAKE, pms->GetState());

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock061:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock061: end.";
}

/**
 * @tc.name: PowerMgrMock062
 * @tc.desc: test Screen RunningLock by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock062, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock062: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock062:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock062: Failed to get PowerMgrService";
    }

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_SCREEN);
    pms->CreateRunningLock(token, info);

    pms->Lock(token, info, 0);
    EXPECT_EQ(pms->IsUsed(token), true);

    EXPECT_CALL(*g_stateAction,   SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(ActionResult::SUCCESS));

    EXPECT_CALL(*g_stateAction, GoToSleep(_, _, _)).Times(1);
    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    sleep(SLEEP_WAIT_TIME_S + 1);

    pms->UnLock(token);
    EXPECT_EQ(pms->IsUsed(token), false);
    sleep(SLEEP_WAIT_TIME_S + 1);
    EXPECT_EQ(PowerState::SLEEP, pms->GetState());

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock062:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock062: end.";
}

/**
 * @tc.name: PowerMgrMock063
 * @tc.desc: test Screen RunningLock by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock063, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock063: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock063:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock063: Failed to get PowerMgrService";
    }

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_SCREEN);
    pms->CreateRunningLock(token, info);

    pms->Lock(token, info, 0);
    EXPECT_EQ(pms->IsUsed(token), true);


    EXPECT_CALL(*g_stateAction, GoToSleep(_, _, true))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_EQ(pms->ForceSuspendDevice(0), true);
    sleep(SLEEP_WAIT_TIME_S + 1);

    pms->UnLock(token);
    EXPECT_EQ(pms->IsUsed(token), false);
    EXPECT_EQ(PowerState::SLEEP, pms->GetState());

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock063:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock063: end.";
}

/**
 * @tc.name: PowerMgrMock064
 * @tc.desc: test Screen RunningLock by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock064, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock064: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock063:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock064: Failed to get PowerMgrService";
    }

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_SCREEN);
    pms->CreateRunningLock(token, info);

    pms->Lock(token, info, 0);
    EXPECT_EQ(pms->IsUsed(token), true);

    EXPECT_CALL(*g_stateAction, Suspend(_, _, _)).Times(0);

    EXPECT_CALL(*g_stateAction,
        Wakeup(0, WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN,
        std::string("test"), _)).Times(1);
    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN, std::string("test"));

    pms->UnLock(token);
    EXPECT_EQ(PowerState::AWAKE, pms->GetState());

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock064:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock064: end.";
}

/**
 * @tc.name: PowerMgrMock065
 * @tc.desc: test background RunningLock by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock065, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock065: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock065:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock065: Failed to get PowerMgrService";
    }

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_BACKGROUND);
    pms->CreateRunningLock(token, info);

    EXPECT_CALL(*g_stateAction, GoToSleep(_, _, true))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_EQ(pms->ForceSuspendDevice(0), true);
    EXPECT_EQ(PowerState::SLEEP, pms->GetState());
    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN, std::string("test"));
    pms->Lock(token, info, 0);
    EXPECT_EQ(pms->IsUsed(token), true);
    EXPECT_CALL(*g_lockAction, Unlock(_, _)).Times(1);
    pms->UnLock(token);

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock065:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock065: end.";
}

/**
 * @tc.name: PowerMgrMock066
 * @tc.desc: test background RunningLock by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock066, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock066: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock066:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock066: Failed to get PowerMgrService";
    }

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_BACKGROUND);
    pms->CreateRunningLock(token, info);

    EXPECT_CALL(*g_lockAction, Lock(_, _)).Times(1);
    pms->Lock(token, info, 0);
    EXPECT_EQ(pms->IsUsed(token), true);

    EXPECT_CALL(*g_lockAction, Unlock(_, _)).Times(1);
    pms->UnLock(token);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_ON));
    sleep((SCREEN_OFF_WAIT_TIME_S*2/3) + 1);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_DIM));
    sleep((SCREEN_OFF_WAIT_TIME_S*1/3) + 1);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_OFF));
    EXPECT_EQ(pms->IsUsed(token), false);
    EXPECT_CALL(*g_stateAction, GoToSleep(_, _, _)).Times(1);
    sleep(SLEEP_WAIT_TIME_S + 1);
    EXPECT_EQ(PowerState::SLEEP, pms->GetState());

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock066:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock066: end.";
}

/**
 * @tc.name: PowerMgrMock067
 * @tc.desc: test background RunningLock by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock067, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock067: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock067:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock067: Failed to get PowerMgrService";
    }

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_BACKGROUND);
    pms->CreateRunningLock(token, info);
    pms->Lock(token, info, 0);
    EXPECT_EQ(pms->IsUsed(token), true);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_ON));
    sleep((SCREEN_OFF_WAIT_TIME_S*2/3) + 1);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_DIM));
    EXPECT_CALL(*g_stateAction,   SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    sleep((SCREEN_OFF_WAIT_TIME_S*1/3) + 1);
    EXPECT_CALL(*g_stateAction, GoToSleep(_, _, _)).Times(0);
    sleep(SLEEP_WAIT_TIME_S*10);
    EXPECT_CALL(*g_stateAction, GoToSleep(_, _, _)).Times(0);
    EXPECT_EQ(PowerState::INACTIVE, pms->GetState());

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock067:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock067: end.";
}

/**
 * @tc.name: PowerMgrMock068
 * @tc.desc: test background RunningLock by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock068, TestSize.Level2)
{
    int i;
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock068: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock068:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock068: Failed to get PowerMgrService";
    }

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_BACKGROUND);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_ON));
    sleep((SCREEN_OFF_WAIT_TIME_S*2/3) + 1);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_DIM));
    pms->CreateRunningLock(token, info);
    EXPECT_CALL(*g_stateAction,   SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction, GoToSleep(_, _, _)).Times(0);
    pms->Lock(token, info, 0);
    EXPECT_EQ(pms->IsUsed(token), true);
    sleep((SCREEN_OFF_WAIT_TIME_S*1/3) + 1);
    pms->UnLock(token);
    EXPECT_EQ(pms->IsUsed(token), false);
    EXPECT_EQ(PowerState::INACTIVE, pms->GetState());
    for (i=0; i<10; i++) {
        pms->Lock(token, info, 0);
        EXPECT_EQ(pms->IsUsed(token), true);
        sleep(SCREEN_OFF_WAIT_TIME_S + 1);

        pms->UnLock(token);
        EXPECT_EQ(pms->IsUsed(token), false);
        EXPECT_EQ(PowerState::INACTIVE, pms->GetState());
    }
    EXPECT_EQ(PowerState::INACTIVE, pms->GetState());

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock068:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock068: end.";
}

/**
 * @tc.name: PowerMgrMock069
 * @tc.desc: test background RunningLock by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock069, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock069: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock069:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock069: Failed to get PowerMgrService";
    }

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_BACKGROUND);
    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    sleep(SLEEP_WAIT_TIME_S);
    pms->CreateRunningLock(token, info);
    EXPECT_CALL(*g_lockAction, Lock(_, _)).Times(1);
        pms->Lock(token, info, 0);
    EXPECT_EQ(pms->IsUsed(token), true);
    sleep(SLEEP_WAIT_TIME_S + 1);
    EXPECT_CALL(*g_stateAction,  SetDisplayState(DisplayState::DISPLAY_ON, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN, std::string("test"));
    sleep(SLEEP_WAIT_TIME_S + 1);
    EXPECT_CALL(*g_stateAction,   SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_)).Times(0);

    EXPECT_CALL(*g_lockAction, Unlock(_, _)).Times(1);
    pms->UnLock(token);
    EXPECT_EQ(pms->IsUsed(token), false);
    EXPECT_EQ(PowerState::AWAKE, pms->GetState());

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock069:End");
    GTEST_LOG_(INFO) << "PowerMgrMock069: end.";
}

/**
 * @tc.name: PowerMgrMock070
 * @tc.desc: test background RunningLock by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock070, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock070: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock070:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock070: Failed to get PowerMgrService";
    }

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_BACKGROUND);
    pms->CreateRunningLock(token, info);

    EXPECT_CALL(*g_lockAction, Lock(_, _)).Times(1);
    pms->Lock(token, info, 0);
    EXPECT_EQ(pms->IsUsed(token), true);

    sleep(SLEEP_WAIT_TIME_S + 1);
    EXPECT_CALL(*g_stateAction, GoToSleep(_, _, true))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_EQ(pms->ForceSuspendDevice(0), true);
    sleep(SLEEP_WAIT_TIME_S + 1);

    EXPECT_CALL(*g_lockAction, Unlock(_, _)).Times(1);
    pms->UnLock(token);
    EXPECT_EQ(pms->IsUsed(token), false);
    EXPECT_EQ(PowerState::SLEEP, pms->GetState());

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock070:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock070: end.";
}

/**
 * @tc.name: PowerMgrMock071
 * @tc.desc: test proximity screen control RunningLock by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock071, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock071: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock071:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock071: Failed to get PowerMgrService";
    }

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL);
    pms->CreateRunningLock(token, info);
    EXPECT_CALL(*g_stateAction,   SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    EXPECT_EQ(PowerState::INACTIVE, pms->GetState());
    EXPECT_CALL(*g_stateAction,  SetDisplayState(DisplayState::DISPLAY_ON, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    sleep(SCREEN_OFF_WAIT_TIME_S + 1);
    pms->Lock(token, info, 0);
    EXPECT_EQ(pms->IsUsed(token), true);
    EXPECT_EQ(PowerState::AWAKE, pms->GetState());

    pms->UnLock(token);
    EXPECT_EQ(pms->IsUsed(token), false);

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock071:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock071: end.";
}


/**
 * @tc.name: PowerMgrMock072
 * @tc.desc: test proximity screen control RunningLock by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock072, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock072: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock072:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock072: Failed to get PowerMgrService";
    }

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL);
    pms->CreateRunningLock(token, info);
    EXPECT_CALL(*g_stateAction, GoToSleep(_, _, true))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_EQ(pms->ForceSuspendDevice(0), true);
    EXPECT_CALL(*g_stateAction,  SetDisplayState(DisplayState::DISPLAY_ON, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    sleep(SCREEN_OFF_WAIT_TIME_S + 1);
    EXPECT_EQ(PowerState::SLEEP, pms->GetState());
    pms->Lock(token, info, 0);
    EXPECT_EQ(pms->IsUsed(token), true);
    EXPECT_EQ(PowerState::AWAKE, pms->GetState());
    pms->UnLock(token);
    EXPECT_EQ(pms->IsUsed(token), false);

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock072:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock072: end.";
}

/**
 * @tc.name: PowerMgrMock073
 * @tc.desc: test proximity screen control RunningLock by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock073, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock073: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock073:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock073: Failed to get PowerMgrService";
    }

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL);
    pms->CreateRunningLock(token, info);
    pms->Lock(token, info, 0);
    EXPECT_EQ(pms->IsUsed(token), true);
    sleep(SLEEP_WAIT_TIME_S);

    pms->UnLock(token);
    EXPECT_EQ(pms->IsUsed(token), false);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_ON));
    sleep((SCREEN_OFF_WAIT_TIME_S*2/3) + 1);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_DIM));
    EXPECT_CALL(*g_stateAction,   SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    sleep((SCREEN_OFF_WAIT_TIME_S*1/3) + 1);
    EXPECT_EQ(PowerState::INACTIVE, pms->GetState());
    EXPECT_CALL(*g_stateAction, GoToSleep(_, _, false))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
        ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_OFF));
    sleep(SLEEP_WAIT_TIME_S + 1);
    EXPECT_EQ(PowerState::SLEEP, pms->GetState());

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock073:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock073: end.";
}

/**
 * @tc.name: PowerMgrMock074
 * @tc.desc: test proximity RunningLock by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock074, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock074: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock074:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock074: Failed to get PowerMgrService";
    }

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL);
    pms->CreateRunningLock(token, info);

    EXPECT_CALL(*g_lockAction, Lock(_, _)).Times(1);
    pms->Lock(token, info, 0);
    EXPECT_EQ(pms->IsUsed(token), true);
    sleep(SCREEN_OFF_WAIT_TIME_S+1);
    EXPECT_EQ(PowerState::AWAKE, pms->GetState());
    EXPECT_CALL(*g_stateAction,   SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction, GoToSleep(_, _, _)).Times(0);
    pms->MockProximity(RunningLockMgr::PROXIMITY_CLOSE);

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock074:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock074: end.";
}

/**
 * @tc.name: PowerMgrMock075
 * @tc.desc: test proximity RunningLock by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock075, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock075: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock075:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock075: Failed to get PowerMgrService";
    }

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL);
    pms->CreateRunningLock(token, info);
    pms->Lock(token, info, 0);
    EXPECT_EQ(pms->IsUsed(token), true);
    sleep(SLEEP_WAIT_TIME_S*10);
    EXPECT_EQ(PowerState::AWAKE, pms->GetState());
    EXPECT_CALL(*g_stateAction,   SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction, GoToSleep(_, _, _)).Times(0);
    pms->MockProximity(RunningLockMgr::PROXIMITY_CLOSE);
    EXPECT_EQ(PowerState::INACTIVE, pms->GetState());

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock075:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock075: end.";
}

/**
 * @tc.name: PowerMgrMock076
 * @tc.desc: test proximity RunningLock by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock076, TestSize.Level2)
{
    int i;
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock076: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock076:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock076: Failed to get PowerMgrService";
    }

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL);
    pms->CreateRunningLock(token, info);
    for (i=0; i<10; i++) {
        pms->Lock(token, info, 0);
        EXPECT_EQ(pms->IsUsed(token), true);
        sleep(SCREEN_OFF_WAIT_TIME_S + 1);
        pms->UnLock(token);
        EXPECT_EQ(pms->IsUsed(token), false);
        EXPECT_EQ(PowerState::AWAKE, pms->GetState());
    }
    pms->Lock(token, info, 0);
    EXPECT_EQ(pms->IsUsed(token), true);
    EXPECT_CALL(*g_stateAction,   SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction, GoToSleep(_, _, _)).Times(0);
    pms->MockProximity(RunningLockMgr::PROXIMITY_CLOSE);
    EXPECT_EQ(PowerState::INACTIVE, pms->GetState());

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock076:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock076: end.";
}

/**
 * @tc.name: PowerMgrMock077
 * @tc.desc: test proximity RunningLock by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock077, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock077: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock077:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock077: Failed to get PowerMgrService";
    }

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL);
    pms->CreateRunningLock(token, info);
    EXPECT_CALL(*g_lockAction, Lock(_, _)).Times(1);
    pms->Lock(token, info, 0);
    EXPECT_EQ(pms->IsUsed(token), true);
    EXPECT_CALL(*g_stateAction,   SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction, GoToSleep(_, _, false))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    sleep(SLEEP_WAIT_TIME_S + 1);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_ON));
    sleep((SCREEN_OFF_WAIT_TIME_S*2/3) + 1);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_DIM));
    sleep((SCREEN_OFF_WAIT_TIME_S*1/3) + 1);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_OFF));
    EXPECT_EQ(PowerState::SLEEP, pms->GetState());

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock077:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock077: end.";
}

/**
 * @tc.name: PowerMgrMock078
 * @tc.desc: test proximity RunningLock by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock078, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock078: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock078:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock078: Failed to get PowerMgrService";
    }

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL);
    pms->CreateRunningLock(token, info);
    EXPECT_CALL(*g_lockAction, Lock(_, _)).Times(1);
    pms->Lock(token, info, 0);
    EXPECT_EQ(pms->IsUsed(token), true);

    EXPECT_CALL(*g_stateAction, GoToSleep(_, _, true))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    pms->ForceSuspendDevice(0);
    EXPECT_EQ(PowerState::SLEEP, pms->GetState());

    EXPECT_CALL(*g_lockAction, Unlock(_, _)).Times(1);
    pms->UnLock(token);
    EXPECT_EQ(pms->IsUsed(token), false);

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock078:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock078: end.";
}

/**
 * @tc.name: PowerMgrMock079
 * @tc.desc: test proximity RunningLock by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock079, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock079: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock079:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock079: Failed to get PowerMgrService";
    }

    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL);
    pms->CreateRunningLock(token, info);

    EXPECT_CALL(*g_lockAction, Lock(_, _)).Times(1);
    pms->Lock(token, info, 0);
    EXPECT_EQ(pms->IsUsed(token), true);

    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN, std::string("test"));
    EXPECT_EQ(PowerState::AWAKE, pms->GetState());

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock079:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock079: end.";
}

/**
 * @tc.name: PowerMgrMock080
 * @tc.desc: test Auto SuspendDevice by mock after 15s
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock080, TestSize.Level2)
{
    int64_t time =15000;
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock080: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock080:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock080: Failed to get PowerMgrService";
    }

    pms->SetDisplayOffTime(time);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_ON));
    sleep(((time/1000)*2/3)+1);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_DIM));
    EXPECT_CALL(*g_stateAction,   SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    sleep(time/1000*1/3+1);
    EXPECT_CALL(*g_stateAction, GoToSleep(_, _, false))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_OFF));
    sleep(SLEEP_WAIT_TIME_S+1);

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock080:End.");
    pms->SetDisplayOffTime(DEFAULT_DISPLAY_OFF_TIME);
    GTEST_LOG_(INFO) << "PowerMgrMock080: end.";
}

/**
 * @tc.name: PowerMgrMock081
 * @tc.desc: test Auto SuspendDevice by mock after 30s
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock081, TestSize.Level2)
{
    int64_t time =30000;
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock081: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock081:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock081: Failed to get PowerMgrService";
    }

    pms->SetDisplayOffTime(time);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_ON));
    sleep(((time/1000)*2/3)+1);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_DIM));
    EXPECT_CALL(*g_stateAction,   SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    sleep(((time/1000)*1/3)+1);
    EXPECT_CALL(*g_stateAction, GoToSleep(_, _, false))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_OFF));
    sleep(SLEEP_WAIT_TIME_S+1);

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock081:End");
    pms->SetDisplayOffTime(DEFAULT_DISPLAY_OFF_TIME);
    GTEST_LOG_(INFO) << "PowerMgrMock081: end.";
}

/**
 * @tc.name: PowerMgrMock082
 * @tc.desc: test Auto SuspendDevice by mock after 1min
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock082, TestSize.Level2)
{
    int64_t time =60000;
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock082: start.";

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock082: Failed to get PowerMgrService";
    }

    pms->SetDisplayOffTime(time);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_ON));
    sleep(((time/1000)*2/3)+1);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_DIM));
    EXPECT_CALL(*g_stateAction,   SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    sleep(time/1000*1/3+1);
    EXPECT_CALL(*g_stateAction, GoToSleep(_, _, false))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_OFF));
    sleep(SLEEP_WAIT_TIME_S+1);

    ResetMockAction();
    pms->SetDisplayOffTime(DEFAULT_DISPLAY_OFF_TIME);
    GTEST_LOG_(INFO) << "PowerMgrMock082: end.";
}

/**
 * @tc.name: PowerMgrMock083
 * @tc.desc: test Auto SuspendDevice by mock after 2mins
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock083, TestSize.Level2)
{
    int64_t time =120000;
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock083: start.";

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock083: Failed to get PowerMgrService";
    }

    pms->SetDisplayOffTime(time);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_ON));
    sleep(((time/1000)*2/3)+1);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_DIM));
    EXPECT_CALL(*g_stateAction,   SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    sleep(time/1000*1/3+1);
    EXPECT_CALL(*g_stateAction, GoToSleep(_, _, false))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_OFF));
    sleep(SLEEP_WAIT_TIME_S+1);

    ResetMockAction();
    pms->SetDisplayOffTime(DEFAULT_DISPLAY_OFF_TIME);
    GTEST_LOG_(INFO) << "PowerMgrMock083: end.";
}

/**
 * @tc.name: PowerMgrMock084
 * @tc.desc: test Auto SuspendDevice by mock after 5mins
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock084, TestSize.Level2)
{
    int64_t time =300000;
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock084: start.";

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock084: Failed to get PowerMgrService";
    }

    pms->SetDisplayOffTime(time);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_ON));
    sleep(((time/1000)*2/3)+1);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_DIM));
    EXPECT_CALL(*g_stateAction,   SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    sleep(time/1000*1/3+1);
    EXPECT_CALL(*g_stateAction, GoToSleep(_, _, false))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_OFF));
    sleep(SLEEP_WAIT_TIME_S+1);

    ResetMockAction();
    pms->SetDisplayOffTime(DEFAULT_DISPLAY_OFF_TIME);
    GTEST_LOG_(INFO) << "PowerMgrMock084: end.";
}

/**
 * @tc.name: PowerMgrMock085
 * @tc.desc: test Auto SuspendDevice by mock after 10mins
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock085, TestSize.Level2)
{
    int64_t time =600000;
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock085: start.";

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock085: Failed to get PowerMgrService";
    }

    pms->SetDisplayOffTime(time);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_ON));
    sleep(((time/1000)*2/3)+1);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_DIM));
    EXPECT_CALL(*g_stateAction,   SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    sleep(time/1000*1/3+1);
    EXPECT_CALL(*g_stateAction, GoToSleep(_, _, false))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_OFF));
    sleep(SLEEP_WAIT_TIME_S+1);

    ResetMockAction();
pms->SetDisplayOffTime(DEFAULT_DISPLAY_OFF_TIME);
    GTEST_LOG_(INFO) << "PowerMgrMock085: end.";
}

/**
 * @tc.name: PowerMgrMock086
 * @tc.desc: test Auto DIM by mock after 15s
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock086, TestSize.Level2)
{
    int64_t time =15000;
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock086: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock086:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock086: Failed to get PowerMgrService";
    }
    pms->SetDisplayOffTime(time);
    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN, std::string("test"));
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock086:DeviceStateAction::SetDisplayState.");
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_CALL(*g_stateAction,  SetDisplayState(DisplayState::DISPLAY_DIM, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction,   SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock086:Start sleep.");
    sleep(time/1000-2);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_DIM));
    sleep(3);
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock086: sleep end.");

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock086:End.");
    pms->SetDisplayOffTime(DEFAULT_DISPLAY_OFF_TIME);
    GTEST_LOG_(INFO) << "PowerMgrMock086: end.";
}

/**
 * @tc.name: PowerMgrMock087
 * @tc.desc: test Auto DIM by mock after 30s
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock087, TestSize.Level2)
{
    int64_t time =30000;
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock087: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock0087:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock087: Failed to get PowerMgrService";
    }
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_CALL(*g_stateAction,  SetDisplayState(DisplayState::DISPLAY_DIM, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction,   SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    pms->SetDisplayOffTime(time);
    sleep(time/1000-2);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_DIM));
    sleep(3);

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock087:End.");
    pms->SetDisplayOffTime(DEFAULT_DISPLAY_OFF_TIME);
    GTEST_LOG_(INFO) << "PowerMgrMock087: end.";
}

/**
 * @tc.name: PowerMgrMock088
 * @tc.desc: test Auto DIM by mock after 60s
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock088, TestSize.Level2)
{
    int64_t time =60000;
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock088: start.";

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock088: Failed to get PowerMgrService";
    }
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_CALL(*g_stateAction,  SetDisplayState(DisplayState::DISPLAY_DIM, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction,   SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    pms->SetDisplayOffTime(time);
    sleep(time/1000-2);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_DIM));
    sleep(3);

    ResetMockAction();
    pms->SetDisplayOffTime(DEFAULT_DISPLAY_OFF_TIME);
    GTEST_LOG_(INFO) << "PowerMgrMock088: end.";
}

/**
 * @tc.name: PowerMgrMock089
 * @tc.desc: test Auto DIM by mock after 2min
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock089, TestSize.Level2)
{
    int64_t time =120000;
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock089: start.";

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock089: Failed to get PowerMgrService";
    }
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_CALL(*g_stateAction,  SetDisplayState(DisplayState::DISPLAY_DIM, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction,   SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    pms->SetDisplayOffTime(time);
    sleep(time/1000-2);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_DIM));
    sleep(3);

    ResetMockAction();
    pms->SetDisplayOffTime(DEFAULT_DISPLAY_OFF_TIME);
    GTEST_LOG_(INFO) << "PowerMgrMock089: end.";
}

/**
 * @tc.name: PowerMgrMock090
 * @tc.desc: test Auto DIM by mock after 5min
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock090, TestSize.Level2)
{
    int64_t time =300000;
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock090: start.";

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock090: Failed to get PowerMgrService";
    }
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_CALL(*g_stateAction,  SetDisplayState(DisplayState::DISPLAY_DIM, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction,   SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    pms->SetDisplayOffTime(time);
    sleep(time/1000-2);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_DIM));
    sleep(3);

    ResetMockAction();
    pms->SetDisplayOffTime(DEFAULT_DISPLAY_OFF_TIME);
    GTEST_LOG_(INFO) << "PowerMgrMock090: end.";
}

/**
 * @tc.name: PowerMgrMock091
 * @tc.desc: test proximity screen control RunningLock by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock091, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock091: start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock091:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock091: Failed to get PowerMgrService";
    }

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL);
    pms->CreateRunningLock(token, info);
    pms->Lock(token, info, 0);
    EXPECT_EQ(pms->IsUsed(token), true);
    sleep(SLEEP_WAIT_TIME_S);
    pms->UnLock(token);
    EXPECT_EQ(pms->IsUsed(token), false);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_ON));
    sleep((SCREEN_OFF_WAIT_TIME_S*2/3)+1);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_DIM));
    EXPECT_CALL(*g_stateAction,   SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    sleep((SCREEN_OFF_WAIT_TIME_S*1/3) + 1);
    EXPECT_EQ(PowerState::INACTIVE, pms->GetState());

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock091:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock091: end.";
}

/**
 * @tc.name: PowerMgrMock092
 * @tc.desc: test WakeupDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock092, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock092:  start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock092:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock092: Failed to get PowerMgrService";
    }

    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_CALL(*g_stateAction,  SetDisplayState(DisplayState::DISPLAY_DIM, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    sleep((SCREEN_OFF_WAIT_TIME_S*2/3) + 1);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_DIM));
    EXPECT_CALL(*g_stateAction,
        Wakeup(0, WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN,
        std::string("test"), _)).Times(1);
    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN, std::string("test"));

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock092:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock092: end.";
}

/**
 * @tc.name: PowerMgrMock093
 * @tc.desc: test WakeupDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock093, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock093:  start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock093:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock093: Failed to get PowerMgrService";
    }

    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_CALL(*g_stateAction,  SetDisplayState(DisplayState::DISPLAY_DIM, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    sleep((SCREEN_OFF_WAIT_TIME_S*2/3) + 1);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_DIM));
    EXPECT_CALL(*g_stateAction,
        Wakeup(0, WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON,
        std::string("test"), _)).Times(1);
    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON, std::string("test"));

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock093:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock093: end.";
}

/**
 * @tc.name: PowerMgrMock094
 * @tc.desc: test WakeupDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock094, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock094:  start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock094:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock094: Failed to get PowerMgrService";
    }

    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_CALL(*g_stateAction,  SetDisplayState(DisplayState::DISPLAY_DIM, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    sleep((SCREEN_OFF_WAIT_TIME_S*2/3) + 1);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_DIM));
    EXPECT_CALL(*g_stateAction,
        Wakeup(0, WakeupDeviceType::WAKEUP_DEVICE_APPLICATION,
        std::string("test"), _)).Times(1);
    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_APPLICATION, std::string("test"));

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock094:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock094: end.";
}

/**
 * @tc.name: PowerMgrMock095
 * @tc.desc: test WakeupDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock095, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock095:  start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock095:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock095: Failed to get PowerMgrService";
    }

    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_CALL(*g_stateAction,  SetDisplayState(DisplayState::DISPLAY_DIM, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    sleep((SCREEN_OFF_WAIT_TIME_S*2/3) + 1);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_DIM));
    EXPECT_CALL(*g_stateAction,
        Wakeup(0, WakeupDeviceType::WAKEUP_DEVICE_PLUGGED_IN,
        std::string("test"), _)).Times(1);
    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_PLUGGED_IN, std::string("test"));

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock095:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock095: end.";
}

/**
 * @tc.name: PowerMgrMock096
 * @tc.desc: test WakeupDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock096, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock096:  start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock096:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock096: Failed to get PowerMgrService";
    }

    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_CALL(*g_stateAction,  SetDisplayState(DisplayState::DISPLAY_DIM, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    sleep((SCREEN_OFF_WAIT_TIME_S*2/3) + 1);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_DIM));
    EXPECT_CALL(*g_stateAction,
        Wakeup(0, WakeupDeviceType::WAKEUP_DEVICE_GESTURE,
        std::string("test"), _)).Times(1);
    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_GESTURE, std::string("test"));

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock096:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock096: end.";
}

/**
 * @tc.name: PowerMgrMock097
 * @tc.desc: test WakeupDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock097, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock097:  start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock097:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock097: Failed to get PowerMgrService";
    }

    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_CALL(*g_stateAction,  SetDisplayState(DisplayState::DISPLAY_DIM, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    sleep((SCREEN_OFF_WAIT_TIME_S*2/3) + 1);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_DIM));
    EXPECT_CALL(*g_stateAction,
        Wakeup(0, WakeupDeviceType::WAKEUP_DEVICE_CAMERA_LAUNCH,
        std::string("test"), _)).Times(1);
    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_CAMERA_LAUNCH, std::string("test"));

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock097:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock097: end.";
}

/**
 * @tc.name: PowerMgrMock098
 * @tc.desc: test WakeupDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock098, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock098:  start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock098:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock098: Failed to get PowerMgrService";
    }

    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_CALL(*g_stateAction,  SetDisplayState(DisplayState::DISPLAY_DIM, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    sleep((SCREEN_OFF_WAIT_TIME_S*2/3) + 1);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_DIM));
    EXPECT_CALL(*g_stateAction,
        Wakeup(0, WakeupDeviceType::WAKEUP_DEVICE_WAKE_KEY,
        std::string("test"), _)).Times(1);
    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_WAKE_KEY, std::string("test"));

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock098:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock098: end.";
}

/**
 * @tc.name: PowerMgrMock099
 * @tc.desc: test WakeupDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock099, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock099:  start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock099:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock099: Failed to get PowerMgrService";
    }

    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_CALL(*g_stateAction,  SetDisplayState(DisplayState::DISPLAY_DIM, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    sleep((SCREEN_OFF_WAIT_TIME_S*2/3) + 1);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_DIM));
    EXPECT_CALL(*g_stateAction,
        Wakeup(0, WakeupDeviceType::WAKEUP_DEVICE_WAKE_MOTION,
        std::string("test"), _)).Times(1);
    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_WAKE_MOTION, std::string("test"));

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock099:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock099: end.";
}

/**
 * @tc.name: PowerMgrMock100
 * @tc.desc: test WakeupDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock100, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock100:  start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock100:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock100: Failed to get PowerMgrService";
    }

    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_CALL(*g_stateAction,  SetDisplayState(DisplayState::DISPLAY_DIM, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    sleep((SCREEN_OFF_WAIT_TIME_S*2/3) + 1);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_DIM));
    EXPECT_CALL(*g_stateAction,
        Wakeup(0, WakeupDeviceType::WAKEUP_DEVICE_HDMI,
        std::string("test"), _)).Times(1);
    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_HDMI, std::string("test"));

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock100:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock100: end.";
}

/**
 * @tc.name: PowerMgrMock101
 * @tc.desc: test WakeupDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrSTMockTest, PowerMgrMock101, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrMock101:  start.";
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock101:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock101: Failed to get PowerMgrService";
    }

    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_CALL(*g_stateAction,  SetDisplayState(DisplayState::DISPLAY_DIM, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    sleep((SCREEN_OFF_WAIT_TIME_S*2/3) + 1);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_DIM));
    EXPECT_CALL(*g_stateAction,
        Wakeup(0, WakeupDeviceType::WAKEUP_DEVICE_LID,
        std::string("test"), _)).Times(1);
    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_LID, std::string("test"));

    ResetMockAction();
    POWER_HILOGD(MODULE_SERVICE, "PowerMgrMock101:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock101: end.";
}
}
