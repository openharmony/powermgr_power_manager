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

#include "power_mgr_mock_test.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;
using ::testing::_;

static sptr<PowerMgrService> service;
static MockStateAction* g_powerState;
static MockStateAction* g_stateAction;
static MockPowerAction* g_powerAction;
static MockLockAction* g_lockAction;

static void ResetMockAction()
{
    POWER_HILOGD(LABEL_TEST, "ResetMockAction:Start.");
    g_stateAction = new MockStateAction();
    g_powerAction = new MockPowerAction();
    g_lockAction = new MockLockAction();
    service->EnableMock(g_powerState, g_stateAction, g_powerAction, g_lockAction);
}

void PowerMgrMockTest::SetUpTestCase(void)
{
    // create singleton service object at the beginning
    service = DelayedSpSingleton<PowerMgrService>::GetInstance();
    service->OnStart();
    ResetMockAction();
}

void PowerMgrMockTest::TearDownTestCase(void)
{
    service->OnStop();
    DelayedSpSingleton<PowerMgrService>::DestroyInstance();
    delete g_stateAction;
    delete g_powerState;
    delete g_powerAction;
    delete g_lockAction;
}

void PowerMgrMockTest::SetUp(void)
{
}

void PowerMgrMockTest::TearDown(void)
{
}

namespace {
/**
 * @tc.name: PowerMgrUnittest001
 * @tc.desc: test RebootDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest001, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest001: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest001:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest001: Failed to get PowerMgrService";
    }
    if (false) {
        EXPECT_CALL(*g_powerAction, Reboot(std::string("test"))).Times(1);
        pms->RebootDevice(std::string("test"));
    }
    
    sleep(ASYNC_WAIT_TIME_S);

    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest001:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest001: end.";
}

/**
 * @tc.name: PowerMgrUnittest002
 * @tc.desc: test ShutDownDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest002, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest002: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest002:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest002: Failed to get PowerMgrService";
    }

    if (false) {
        EXPECT_CALL(*g_powerAction, Shutdown(std::string("test"))).Times(1);
        pms->ShutDownDevice(std::string("test"));
    }
    sleep(ASYNC_WAIT_TIME_S);

    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest002:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest002: end.";
}


/**
 * @tc.name: PowerMgrUnittest003
 * @tc.desc: test SuspendDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest003, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest003: start.";

    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest003:Start.");
    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest003: Failed to get PowerMgrService";
    }

    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest003:Start mock.");
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction,
        Suspend(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false));
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest003:Start suspend.");
    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);

    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest003:end.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest003: end.";
}

/**
 * @tc.name: PowerMgrUnittest004
 * @tc.desc: test SuspendDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest004, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest004: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest004:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest004: Failed to get PowerMgrService";
    }

    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction,
        Suspend(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_DEVICE_ADMIN, false));
    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_DEVICE_ADMIN, false);

    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest004:End .");
    GTEST_LOG_(INFO) << "PowerMgrUnittest004: end.";
}

/**
 * @tc.name: PowerMgrUnittest005
 * @tc.desc: test SuspendDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest005, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest005: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest005:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest005: Failed to get PowerMgrService";
    }

    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction,
        Suspend(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_TIMEOUT, false));
    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_TIMEOUT, false);

    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest005:End");
    GTEST_LOG_(INFO) << "PowerMgrUnittest005: end.";
}

/**
 * @tc.name: PowerMgrUnittest006
 * @tc.desc: test SuspendDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest006, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest006: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest006:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest006: Failed to get PowerMgrService";
    }

    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction,
        Suspend(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_LID_SWITCH, false));
    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_LID_SWITCH, false);

    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest006:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest006: end.";
}

/**
 * @tc.name: PowerMgrUnittest007
 * @tc.desc: test SuspendDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest007, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest007: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest007:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest007: Failed to get PowerMgrService";
    }

    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction,
        Suspend(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_POWER_BUTTON, false));
    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_POWER_BUTTON, false);

    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest007:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest007: end.";
}

/**
 * @tc.name: PowerMgrUnittest008
 * @tc.desc: test SuspendDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest008, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest008: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest008:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest008: Failed to get PowerMgrService";
    }

    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction,
        Suspend(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_HDMI, false));
    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_HDMI, false);

    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest008:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest008: end.";
}

/**
 * @tc.name: PowerMgrUnittest009
 * @tc.desc: test SuspendDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest009, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest009: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest009:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest009: Failed to get PowerMgrService";
    }

    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction,
        Suspend(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_SLEEP_BUTTON, false));
    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_SLEEP_BUTTON, false);

    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest009:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest009: end.";
}

/**
 * @tc.name: PowerMgrUnittest010
 * @tc.desc: test SuspendDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest010, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest010: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest010:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest010: Failed to get PowerMgrService";
    }

    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction,
        Suspend(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_ACCESSIBILITY, false));
    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_ACCESSIBILITY, false);

    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest010:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest010: end.";
}

/**
 * @tc.name: PowerMgrUnittest011
 * @tc.desc: test SuspendDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest011, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest011: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest011:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest011: Failed to get PowerMgrService";
    }

    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction,
        Suspend(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_FORCE_SUSPEND, false));
    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_FORCE_SUSPEND, false);

    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest011:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest011: end.";
}

/**
 * @tc.name: PowerMgrUnittest012
 * @tc.desc: test WakeupDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest012, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest012:  start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest012:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest012: Failed to get PowerMgrService";
    }

    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_ON, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction,
        Wakeup(0, WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN,
        std::string("test"), _)).Times(1);
    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN, std::string("test"));

    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest012:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest012: end.";
}

/**
 * @tc.name: PowerMgrUnittest013
 * @tc.desc: test WakeupDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest013, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest013:  start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest013:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest013: Failed to get PowerMgrService";
    }

    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_ON, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction,
        Wakeup(0, WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON,
        std::string("test"), _)).Times(1);
    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON, std::string("test"));

    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest013:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest013: end.";
}

/**
 * @tc.name: PowerMgrUnittest014
 * @tc.desc: test WakeupDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest014, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest014:  start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest014:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest014: Failed to get PowerMgrService";
    }

    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_ON, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction,
        Wakeup(0, WakeupDeviceType::WAKEUP_DEVICE_APPLICATION,
        std::string("test"), _)).Times(1);
    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_APPLICATION, std::string("test"));

    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest014:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest014: end.";
}

/**
 * @tc.name: PowerMgrUnittest015
 * @tc.desc: test WakeupDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest015, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest015:  start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest015:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest015: Failed to get PowerMgrService";
    }

    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_ON, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction,
        Wakeup(0, WakeupDeviceType::WAKEUP_DEVICE_PLUGGED_IN,
        std::string("test"), _)).Times(1);
    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_PLUGGED_IN, std::string("test"));

    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest015:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest015: end.";
}

/**
 * @tc.name: PowerMgrUnittest016
 * @tc.desc: test WakeupDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest016, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest016:  start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest016:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest016: Failed to get PowerMgrService";
    }

    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_ON, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction,
        Wakeup(0, WakeupDeviceType::WAKEUP_DEVICE_GESTURE,
        std::string("test"), _)).Times(1);
    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_GESTURE, std::string("test"));

    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest016:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest016: end.";
}

/**
 * @tc.name: PowerMgrUnittest017
 * @tc.desc: test WakeupDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest017, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest017:  start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest017:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest017: Failed to get PowerMgrService";
    }

    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_ON, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction,
        Wakeup(0, WakeupDeviceType::WAKEUP_DEVICE_CAMERA_LAUNCH,
        std::string("test"), _)).Times(1);
    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_CAMERA_LAUNCH, std::string("test"));

    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest017:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest017: end.";
}

/**
 * @tc.name: PowerMgrUnittest018
 * @tc.desc: test WakeupDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest018, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest018:  start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest018:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest018: Failed to get PowerMgrService";
    }

    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_ON, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction,
        Wakeup(0, WakeupDeviceType::WAKEUP_DEVICE_WAKE_KEY,
        std::string("test"), _)).Times(1);
    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_WAKE_KEY, std::string("test"));

    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest018:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest018: end.";
}

/**
 * @tc.name: PowerMgrUnittest019
 * @tc.desc: test WakeupDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest019, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest019:  start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest019:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest019: Failed to get PowerMgrService";
    }

    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_ON, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction,
        Wakeup(0, WakeupDeviceType::WAKEUP_DEVICE_WAKE_MOTION,
        std::string("test"), _)).Times(1);
    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_WAKE_MOTION, std::string("test"));

    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest019:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest019: end.";
}

/**
 * @tc.name: PowerMgrUnittest020
 * @tc.desc: test WakeupDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest020, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest020:  start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest020:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest020: Failed to get PowerMgrService";
    }

    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_ON, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction,
        Wakeup(0, WakeupDeviceType::WAKEUP_DEVICE_HDMI,
        std::string("test"), _)).Times(1);
    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_HDMI, std::string("test"));

    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest020:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest020: end.";
}

/**
 * @tc.name: PowerMgrUnittest021
 * @tc.desc: test WakeupDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest021, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest021:  start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest021:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest021: Failed to get PowerMgrService";
    }

    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_ON, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction,
        Wakeup(0, WakeupDeviceType::WAKEUP_DEVICE_LID,
        std::string("test"), _)).Times(1);
    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_LID, std::string("test"));

    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest021:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest021: end.";
}

/**
 * @tc.name: PowerMgrUnittest022
 * @tc.desc: test SuspendDevice and auto sleep by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest022, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest022: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest022:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest022: Failed to get PowerMgrService";
    }

    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN, std::string("test"));
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
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
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest022:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest022: end.";
}

/**
 * @tc.name: PowerMgrUnittest023
 * @tc.desc: test WakeupDevice and auto suspend and sleep by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest023, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest023: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest023:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest023: Failed to get PowerMgrService";
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
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest023:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest023: end.";
}

/**
 * @tc.name: PowerMgrUnittest024
 * @tc.desc: test RefreshActivity by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest024, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest024: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest024:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest024: Failed to get PowerMgrService";
    }

    EXPECT_CALL(*g_stateAction, GetDisplayState())
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(DisplayState::DISPLAY_DIM));
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_ON, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction,
        RefreshActivity(0, UserActivityType::USER_ACTIVITY_TYPE_ATTENTION, _)).Times(1);
    pms->RefreshActivity(0, UserActivityType::USER_ACTIVITY_TYPE_ATTENTION, true);

    sleep(SCREEN_OFF_WAIT_TIME_S + SLEEP_WAIT_TIME_S + 1);
    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest024:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest024: end.";
}

/**
 * @tc.name: PowerMgrUnittest025
 * @tc.desc: test IsScreenOn by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest025, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest025: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest025:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest025: Failed to get PowerMgrService";
    }

    EXPECT_CALL(*g_stateAction, GetDisplayState())
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_EQ(pms->IsScreenOn(), true);

    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest025:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest025: end.";
}

/**
 * @tc.name: PowerMgrUnittest026
 * @tc.desc: test IsScreenOn by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest026, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest026: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest026:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest026: Failed to get PowerMgrService";
    }

    EXPECT_CALL(*g_stateAction, GetDisplayState())
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(DisplayState::DISPLAY_DIM));
    EXPECT_EQ(pms->IsScreenOn(), true);

    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest026:End");
    GTEST_LOG_(INFO) << "PowerMgrUnittest026: end.";
}

/**
 * @tc.name: PowerMgrUnittest027
 * @tc.desc: test IsScreenOn by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest027, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest027: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest027:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest027: Failed to get PowerMgrService";
    }

    EXPECT_CALL(*g_stateAction, GetDisplayState())
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(DisplayState::DISPLAY_OFF));
    EXPECT_EQ(pms->IsScreenOn(), false);

    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest027:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest027: end.";
}

/**
 * @tc.name: PowerMgrUnittest028
 * @tc.desc: test IsScreenOn by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest028, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest028: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest028:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest028: Failed to get PowerMgrService";
    }

    EXPECT_CALL(*g_stateAction, GetDisplayState())
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(DisplayState::DISPLAY_SUSPEND));
    EXPECT_EQ(pms->IsScreenOn(), false);

    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest028:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest028: end.";
}

/**
 * @tc.name: PowerMgrUnittest029
 * @tc.desc: test ForceSuspendDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest029, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest029: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest029:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest029: Failed to get PowerMgrService";
    }

    EXPECT_CALL(*g_stateAction, GoToSleep(_, _, true))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_EQ(pms->ForceSuspendDevice(0), true);

    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest029:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest029: end.";
}

/**
 * @tc.name: PowerMgrUnittest030
 * @tc.desc: test Screen on RunningLock by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest030, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest030: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest030:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest030: Failed to get PowerMgrService";
    }

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_SCREEN);
    pms->CreateRunningLock(token, info);
    pms->Lock(token, info, 0);
    EXPECT_EQ(pms->IsUsed(token), true);

    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_DIM, ::testing::_)).Times(0);
    sleep(SCREEN_DIM_WAIT_TIME_S + 1);

    pms->UnLock(token);
    EXPECT_EQ(pms->IsUsed(token), false);

    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest030:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest030: end.";
}

/**
 * @tc.name: PowerMgrUnittest031
 * @tc.desc: test background RunningLock by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest031, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest031: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest031:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest031: Failed to get PowerMgrService";
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
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest031:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest031: end.";
}

/**
 * @tc.name: PowerMgrUnittest032
 * @tc.desc: test proximity RunningLock by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest032, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest032: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest032:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest032: Failed to get PowerMgrService";
    }

    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_DIM, ::testing::_)).Times(0);
    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL);
    pms->CreateRunningLock(token, info);

    EXPECT_CALL(*g_lockAction, Lock(_, _)).Times(1);
    pms->Lock(token, info, 0);
    EXPECT_EQ(pms->IsUsed(token), true);

    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_DIM, ::testing::_)).Times(0);
    sleep(SCREEN_DIM_WAIT_TIME_S + 1);

    EXPECT_CALL(*g_lockAction, Unlock(_, _)).Times(1);
    pms->UnLock(token);
    EXPECT_EQ(pms->IsUsed(token), false);

    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest032:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest032: end.";
}

/**
 * @tc.name: PowerMgrUnittest033
 * @tc.desc: test RunningLock release by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest033, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest033: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest033:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest033: Failed to get PowerMgrService";
    }

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_BACKGROUND);
    pms->CreateRunningLock(token, info);
    pms->ReleaseRunningLock(token);

    EXPECT_CALL(*g_lockAction, Lock(_, _)).Times(0);
    pms->Lock(token, info, 0);
    EXPECT_EQ(pms->IsUsed(token), false);

    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest033:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest033: end.";
}

/**
 * @tc.name: PowerMgrUnittest034
 * @tc.desc: test RefreshActivity by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest034, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest034: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest034:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest034: Failed to get PowerMgrService";
    }

    EXPECT_CALL(*g_stateAction, GetDisplayState())
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(DisplayState::DISPLAY_DIM));
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_ON, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction,
        RefreshActivity(0, UserActivityType::USER_ACTIVITY_TYPE_OTHER, _)).Times(1);
    pms->RefreshActivity(0, UserActivityType::USER_ACTIVITY_TYPE_OTHER, true);

    sleep(SCREEN_OFF_WAIT_TIME_S + SLEEP_WAIT_TIME_S + 1);
    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest034:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest034: end.";
}

/**
 * @tc.name: PowerMgrUnittest035
 * @tc.desc: test RefreshActivity by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest035, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest035: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest035:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest035: Failed to get PowerMgrService";
    }

    EXPECT_CALL(*g_stateAction, GetDisplayState())
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(DisplayState::DISPLAY_DIM));
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_ON, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction,
        RefreshActivity(0, UserActivityType::USER_ACTIVITY_TYPE_BUTTON, _)).Times(1);
    pms->RefreshActivity(0, UserActivityType::USER_ACTIVITY_TYPE_BUTTON, true);

    sleep(SCREEN_OFF_WAIT_TIME_S + SLEEP_WAIT_TIME_S + 1);
    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest035:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest035: end.";
}

/**
 * @tc.name: PowerMgrUnittest036
 * @tc.desc: test RefreshActivity by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest036, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest036: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest036:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest036: Failed to get PowerMgrService";
    }

    EXPECT_CALL(*g_stateAction, GetDisplayState())
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(DisplayState::DISPLAY_DIM));
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_ON, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction,
        RefreshActivity(0, UserActivityType::USER_ACTIVITY_TYPE_ACCESSIBILITY, _)).Times(1);
    pms->RefreshActivity(0, UserActivityType::USER_ACTIVITY_TYPE_ACCESSIBILITY, true);

    sleep(SCREEN_OFF_WAIT_TIME_S + SLEEP_WAIT_TIME_S + 1);
    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest036:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest036: end.";
}

/**
 * @tc.name: PowerMgrUnittest037
 * @tc.desc: test RefreshActivity by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest037, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest037: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest037:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest037: Failed to get PowerMgrService";
    }

    EXPECT_CALL(*g_stateAction, GetDisplayState())
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(DisplayState::DISPLAY_DIM));
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_ON, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction,
        RefreshActivity(0, UserActivityType::USER_ACTIVITY_TYPE_ATTENTION, _)).Times(1);
    pms->RefreshActivity(0, UserActivityType::USER_ACTIVITY_TYPE_ATTENTION, true);

    sleep(SCREEN_OFF_WAIT_TIME_S + SLEEP_WAIT_TIME_S + 1);
    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest037:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest037: end.";
}

/**
 * @tc.name: PowerMgrUnittest038
 * @tc.desc: test RefreshActivity by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest038, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest038: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest038:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest038: Failed to get PowerMgrService";
    }

    EXPECT_CALL(*g_stateAction, GetDisplayState())
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(DisplayState::DISPLAY_DIM));
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_ON, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction,
        RefreshActivity(0, UserActivityType::USER_ACTIVITY_TYPE_SOFTWARE, _)).Times(1);
    pms->RefreshActivity(0, UserActivityType::USER_ACTIVITY_TYPE_SOFTWARE, true);

    sleep(SCREEN_OFF_WAIT_TIME_S + SLEEP_WAIT_TIME_S + 1);
    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest038:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest038: end.";
}

/**
 * @tc.name: PowerMgrUnittest039
 * @tc.desc: test RefreshActivity by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest039, TestSize.Level2)
{
    UserActivityType abnormaltype = {};
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest039: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest039:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest039: Failed to get PowerMgrService";
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
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest039:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest039: end.";
}

/**
 * @tc.name: PowerMgrUnittest040
 * @tc.desc: test RefreshActivity by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest040, TestSize.Level2)
{
    UserActivityType abnormaltype=UserActivityType(6);
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest040: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest040:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest040: Failed to get PowerMgrService";
    }

    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN, std::string("test"));
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_ON));
    sleep((SCREEN_OFF_WAIT_TIME_S*2/3) + 1);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_DIM));
    EXPECT_CALL(*g_stateAction,
        RefreshActivity(0, _, _)).Times(0);
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    pms->RefreshActivity(0, abnormaltype, true);
    sleep((SCREEN_OFF_WAIT_TIME_S*1/3) + 1);

    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest040:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest040: end.";
}

/**
 * @tc.name: PowerMgrUnittest041
 * @tc.desc: test WakeupDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest041, TestSize.Level2)
{
    int wakeupReason = (static_cast<int>(WakeupDeviceType::WAKEUP_DEVICE_MAX)) + 1;
    WakeupDeviceType abnormaltype = WakeupDeviceType(wakeupReason);
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest041:  start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest041:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest041: Failed to get PowerMgrService";
    }

    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_ON, ::testing::_))
        .Times(0);
    EXPECT_CALL(*g_stateAction,
        Wakeup(0, _,
        std::string("test"), _)).Times(0);
    pms->WakeupDevice(0, abnormaltype, std::string("test"));
    EXPECT_EQ(pms->IsScreenOn(), false);

    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest041:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest041: end.";
}

/**
 * @tc.name: PowerMgrUnittest042
 * @tc.desc: test WakeupDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest042, TestSize.Level2)
{
    WakeupDeviceType abnormaltype=WakeupDeviceType(999);
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest042:  start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest042:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest042: Failed to get PowerMgrService";
    }

    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_ON, ::testing::_))
        .Times(0);
    EXPECT_CALL(*g_stateAction,
        Wakeup(0, _,
        std::string("test"), _)).Times(0);
    pms->WakeupDevice(0, abnormaltype, std::string("test"));
    EXPECT_EQ(PowerState::INACTIVE, pms->GetState());

    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest042:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest042: end.";
}

/**
 * @tc.name: PowerMgrUnittest043
 * @tc.desc: test WakeupDevice and auto suspend  by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest043, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest043: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest043:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest043: Failed to get PowerMgrService";
    }

    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN, std::string("test"));
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_ON));
    sleep((SCREEN_OFF_WAIT_TIME_S*2/3) + 1);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_DIM));
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    sleep((SCREEN_OFF_WAIT_TIME_S*1/3) + 1);

    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest043:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest043: end.";
}

/**
 * @tc.name: PowerMgrUnittest044
 * @tc.desc: test WakeupDevice and auto suspend  by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest044, TestSize.Level2)
{
    int wakeupReason = (static_cast<int>(WakeupDeviceType::WAKEUP_DEVICE_MAX)) + 1;
    WakeupDeviceType abnormaltype = WakeupDeviceType(wakeupReason);
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest044: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest044:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest044: Failed to get PowerMgrService";
    }

    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN, std::string("test"));
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_DIM, ::testing::_))
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_ON));
    sleep((SCREEN_OFF_WAIT_TIME_S*2/3)/2);
    pms->WakeupDevice(0, abnormaltype, std::string("test"));
    sleep((SCREEN_OFF_WAIT_TIME_S*2/3)/2 + 1);

    ResetMockAction();
    GTEST_LOG_(INFO) << "PowerMgrUnittest044: end.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest044:End.");
}

/**
 * @tc.name: PowerMgrUnittest045
 * @tc.desc: test WakeupDevice and auto suspend  by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest045, TestSize.Level2)
{
    WakeupDeviceType abnormaltype = WakeupDeviceType(999);
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest045: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest045:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest045: Failed to get PowerMgrService";
    }

    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN, std::string("test"));
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_ON));
    sleep((SCREEN_OFF_WAIT_TIME_S*2/3) + 1);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_DIM));
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    sleep((SCREEN_OFF_WAIT_TIME_S*2/3) /2);
    pms->WakeupDevice(0, abnormaltype, std::string("test"));
    sleep((SCREEN_OFF_WAIT_TIME_S*1/3)/2 + 1);

    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest045:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest045: end.";
}

/**
 * @tc.name: PowerMgrUnittest046
 * @tc.desc: test RefreshActivity by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest046, TestSize.Level2)
{
    UserActivityType abnormaltype = UserActivityType(6);
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest046: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest046:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest046: Failed to get PowerMgrService";
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
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest046:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest046: end.";
}

/**
 * @tc.name: PowerMgrUnittest047
 * @tc.desc: test RefreshActivity by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest047, TestSize.Level2)
{
    UserActivityType abnormaltype = {};
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest047: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest047:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest047: Failed to get PowerMgrService";
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
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest047:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest047: end.";
}

/**
 * @tc.name: PowerMgrUnittest048
 * @tc.desc: test SuspendDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest048, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest048: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest048:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest048: Failed to get PowerMgrService";
    }

    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON, std::string("test"));
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
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
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest048:End");
    GTEST_LOG_(INFO) << "PowerMgrUnittest048: end.";
}

/**
 * @tc.name: PowerMgrUnittest049
 * @tc.desc: test SuspendDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest049, TestSize.Level2)
{
    SuspendDeviceType abnormaltype = SuspendDeviceType(10);
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest049: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest049:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest049: Failed to get PowerMgrService";
    }

    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON, std::string("test"));
    EXPECT_CALL(*g_stateAction,
        Suspend(0, _, false)).Times(0);
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_)).Times(0);
    pms->SuspendDevice(0, abnormaltype, false);
    EXPECT_EQ(PowerState::AWAKE, pms->GetState());

    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest049:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest049: end.";
}

/**
 * @tc.name: PowerMgrUnittest050
 * @tc.desc: test SuspendDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest050, TestSize.Level2)
{
    SuspendDeviceType abnormaltype = SuspendDeviceType(999);
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest050: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest050:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest050: Failed to get PowerMgrService";
    }

    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON, std::string("test"));
    EXPECT_CALL(*g_stateAction,
        Suspend(0, _, false)).Times(0);
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_)).Times(0);
    pms->SuspendDevice(0, abnormaltype, false);
    EXPECT_EQ(PowerState::AWAKE, pms->GetState());

    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest050:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest050: end.";
}

/**
 * @tc.name: PowerMgrUnittest051
 * @tc.desc: test SuspendDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest051, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest051: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest051:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest051: Failed to get PowerMgrService";
    }

    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
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
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest051:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest051: end.";
}

/**
 * @tc.name: PowerMgrUnittest052
 * @tc.desc: test SuspendDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest052, TestSize.Level2)
{
    SuspendDeviceType abnormaltype = SuspendDeviceType(10);
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest052: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest052:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest052: Failed to get PowerMgrService";
    }

    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    EXPECT_CALL(*g_stateAction,
        Suspend(0, _, false)).Times(0);
    pms->SuspendDevice(0, abnormaltype, false);
    EXPECT_EQ(PowerState::INACTIVE, pms->GetState());

    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest052:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest052: end.";
}

/**
 * @tc.name: PowerMgrUnittest053
 * @tc.desc: test SuspendDevice by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest053, TestSize.Level2)
{
    SuspendDeviceType abnormaltype = {};
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest053: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest053:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest053: Failed to get PowerMgrService";
    }

    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    EXPECT_CALL(*g_stateAction,
        Suspend(0, _, false));
    pms->SuspendDevice(0, abnormaltype, false);
    EXPECT_EQ(PowerState::INACTIVE, pms->GetState());

    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest053:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest053: end.";
}

/**
 * @tc.name: PowerMgrUnittest054
 * @tc.desc: test ForceSuspendDevice by mock during Inactive
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest054, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest054: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest054:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest054: Failed to get PowerMgrService";
    }

    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    EXPECT_CALL(*g_stateAction, GoToSleep(_, _, _))
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    pms->ForceSuspendDevice(0);
    sleep(SLEEP_WAIT_TIME_S+1);
    EXPECT_EQ(PowerState::SLEEP, pms->GetState());

    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest054:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest054: end.";
}

/**
 * @tc.name: PowerMgrUnittest055
 * @tc.desc: test  auto suspend  by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest055, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest055: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest055:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest055: Failed to get PowerMgrService";
    }

    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_LID, std::string("test"));
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_DIM, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    sleep((SCREEN_OFF_WAIT_TIME_S*2/3) + 1);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_DIM));
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    sleep((SCREEN_OFF_WAIT_TIME_S*1/3) + 1);

    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest055:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest055: end.";
}

/**
 * @tc.name: PowerMgrUnittest056
 * @tc.desc: test SCREEN_ON RunningLock by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest056, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest056: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest056:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest056: Failed to get PowerMgrService";
    }

    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_DIM, ::testing::_)).Times(0);
    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_SCREEN);
    pms->CreateRunningLock(token, info);

    pms->Lock(token, info, 0);
    EXPECT_EQ(pms->IsUsed(token), true);

    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_DIM, ::testing::_)).Times(0);
    sleep((SCREEN_OFF_WAIT_TIME_S*2/3) + 1);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_EQ(PowerState::AWAKE, pms->GetState());

    pms->UnLock(token);
    EXPECT_EQ(pms->IsUsed(token), false);
    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest056:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest056: end.";
}

/**
 * @tc.name: PowerMgrUnittest057
 * @tc.desc: test SCREEN_ON RunningLock by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest057, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest057: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest057:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest057: Failed to get PowerMgrService";
    }

    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_SCREEN);
    pms->CreateRunningLock(token, info);
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_ON, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    pms->Lock(token, info, 0);
    EXPECT_EQ(pms->IsUsed(token), true);
    EXPECT_EQ(PowerState::AWAKE, pms->GetState());

    pms->UnLock(token);
    EXPECT_EQ(pms->IsUsed(token), false);
    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest057:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest057: end.";
}

/**
 * @tc.name: PowerMgrUnittest058
 * @tc.desc: test Auto DIM by mock after 10min
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest058, TestSize.Level2)
{
    int64_t time =600000;
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest058: start.";

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest058: Failed to get PowerMgrService";
    }
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_DIM, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    pms->SetDisplayOffTime(time);
    sleep(time/1000-2);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_DIM));
    sleep(3);
    ResetMockAction();
pms->SetDisplayOffTime(DEFAULT_DISPLAY_OFF_TIME);
    GTEST_LOG_(INFO) << "PowerMgrUnittest058: end.";
}

/**
 * @tc.name: PowerMgrUnittest059
 * @tc.desc: test Screen RunningLock by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest059, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest059: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest059:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest059: Failed to get PowerMgrService";
    }

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_SCREEN);
    pms->CreateRunningLock(token, info);
    pms->Lock(token, info, 0);
    EXPECT_EQ(pms->IsUsed(token), true);

    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_DIM, ::testing::_)).Times(0);
    sleep(SCREEN_DIM_WAIT_TIME_S + 1);

    pms->UnLock(token);
    EXPECT_EQ(pms->IsUsed(token), false);
    
    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest059:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest059: end.";
}

/**
 * @tc.name: PowerMgrUnittest060
 * @tc.desc: test Screen RunningLock by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest060, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest060: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest060:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest060: Failed to get PowerMgrService";
    }

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_SCREEN);
    pms->CreateRunningLock(token, info);
    pms->Lock(token, info, 0);
    EXPECT_EQ(pms->IsUsed(token), true);

    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_)).Times(0);
    sleep(SLEEP_WAIT_TIME_S*10);
    
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_)).Times(0);
    EXPECT_EQ(PowerState::AWAKE, pms->GetState());
    

    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest060:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest060: end.";
}

/**
 * @tc.name: PowerMgrUnittest061
 * @tc.desc: test Screen on RunningLock by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest061, TestSize.Level2)
{
    int i;
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest061: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest061:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest061: Failed to get PowerMgrService";
    }

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_SCREEN);
    pms->CreateRunningLock(token, info);
    for (i=0; i<10; i++) {
        pms->Lock(token, info, 0);
        EXPECT_EQ(pms->IsUsed(token), true);

        EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_DIM, ::testing::_)).Times(0);
        sleep(SCREEN_OFF_WAIT_TIME_S + 1);

        pms->UnLock(token);
        EXPECT_EQ(pms->IsUsed(token), false);
        EXPECT_EQ(PowerState::AWAKE, pms->GetState());
    }
    EXPECT_EQ(PowerState::AWAKE, pms->GetState());

    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest061:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest061: end.";
}

/**
 * @tc.name: PowerMgrUnittest062
 * @tc.desc: test Screen RunningLock by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest062, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest062: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest062:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest062: Failed to get PowerMgrService";
    }

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_SCREEN);
    pms->CreateRunningLock(token, info);

    pms->Lock(token, info, 0);
    EXPECT_EQ(pms->IsUsed(token), true);

    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
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
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest062:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest062: end.";
}

/**
 * @tc.name: PowerMgrUnittest063
 * @tc.desc: test Screen RunningLock by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest063, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest063: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest063:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest063: Failed to get PowerMgrService";
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
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest063:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest063: end.";
}

/**
 * @tc.name: PowerMgrUnittest064
 * @tc.desc: test Screen RunningLock by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest064, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest064: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest063:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest064: Failed to get PowerMgrService";
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
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest064:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest064: end.";
}

/**
 * @tc.name: PowerMgrUnittest065
 * @tc.desc: test background RunningLock by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest065, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest065: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest065:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest065: Failed to get PowerMgrService";
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
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest065:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest065: end.";
}

/**
 * @tc.name: PowerMgrUnittest066
 * @tc.desc: test background RunningLock by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest066, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest066: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest066:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest066: Failed to get PowerMgrService";
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
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest066:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest066: end.";
}

/**
 * @tc.name: PowerMgrUnittest067
 * @tc.desc: test background RunningLock by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest067, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest067: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest067:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest067: Failed to get PowerMgrService";
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
    
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    sleep((SCREEN_OFF_WAIT_TIME_S*1/3) + 1);
    EXPECT_CALL(*g_stateAction, GoToSleep(_, _, _)).Times(0);
    sleep(SLEEP_WAIT_TIME_S*10);
    
    EXPECT_CALL(*g_stateAction, GoToSleep(_, _, _)).Times(0);
    EXPECT_EQ(PowerState::INACTIVE, pms->GetState());
    

    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest067:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest067: end.";
}

/**
 * @tc.name: PowerMgrUnittest068
 * @tc.desc: test background RunningLock by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest068, TestSize.Level2)
{
    int i;
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest068: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest068:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest068: Failed to get PowerMgrService";
    }

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_BACKGROUND);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_ON));
    sleep((SCREEN_OFF_WAIT_TIME_S*2/3) + 1);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_DIM));
    pms->CreateRunningLock(token, info);
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
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
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest068:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest068: end.";
}

/**
 * @tc.name: PowerMgrUnittest069
 * @tc.desc: test background RunningLock by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest069, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest069: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest069:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest069: Failed to get PowerMgrService";
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
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_ON, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN, std::string("test"));
    sleep(SLEEP_WAIT_TIME_S + 1);
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_)).Times(0);
    EXPECT_CALL(*g_lockAction, Unlock(_, _)).Times(1);
    pms->UnLock(token);
    EXPECT_EQ(pms->IsUsed(token), false);
    EXPECT_EQ(PowerState::AWAKE, pms->GetState());

    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest069:End");
    GTEST_LOG_(INFO) << "PowerMgrUnittest069: end.";
}

/**
 * @tc.name: PowerMgrUnittest070
 * @tc.desc: test background RunningLock by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest070, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest070: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest070:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest070: Failed to get PowerMgrService";
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
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest070:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest070: end.";
}

/**
 * @tc.name: PowerMgrUnittest071
 * @tc.desc: test proximity screen control RunningLock by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest071, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest071: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest071:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest071: Failed to get PowerMgrService";
    }

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL);
    pms->CreateRunningLock(token, info);
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    EXPECT_EQ(PowerState::INACTIVE, pms->GetState());
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_ON, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    sleep(SCREEN_OFF_WAIT_TIME_S + 1);
    pms->Lock(token, info, 0);
    EXPECT_EQ(pms->IsUsed(token), true);
    EXPECT_EQ(PowerState::AWAKE, pms->GetState());
    pms->UnLock(token);
    EXPECT_EQ(pms->IsUsed(token), false);

    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest071:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest071: end.";
}


/**
 * @tc.name: PowerMgrUnittest072
 * @tc.desc: test proximity screen control RunningLock by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest072, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest072: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest072:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest072: Failed to get PowerMgrService";
    }

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL);
    pms->CreateRunningLock(token, info);
    EXPECT_CALL(*g_stateAction, GoToSleep(_, _, true))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_EQ(pms->ForceSuspendDevice(0), true);
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_ON, ::testing::_))
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
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest072:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest072: end.";
}

/**
 * @tc.name: PowerMgrUnittest073
 * @tc.desc: test proximity screen control RunningLock by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest073, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest073: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest073:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest073: Failed to get PowerMgrService";
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
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
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
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest073:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest073: end.";
}

/**
 * @tc.name: PowerMgrUnittest074
 * @tc.desc: test proximity RunningLock by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest074, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest074: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest074:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest074: Failed to get PowerMgrService";
    }

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL);
    pms->CreateRunningLock(token, info);
    EXPECT_CALL(*g_lockAction, Lock(_, _)).Times(1);
    pms->Lock(token, info, 0);
    EXPECT_EQ(pms->IsUsed(token), true);
    sleep(SCREEN_OFF_WAIT_TIME_S+1);
    EXPECT_EQ(PowerState::AWAKE, pms->GetState());
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction, GoToSleep(_, _, _)).Times(0);
    pms->MockProximity(RunningLockMgr::PROXIMITY_CLOSE);

    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest074:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest074: end.";
}

/**
 * @tc.name: PowerMgrUnittest075
 * @tc.desc: test proximity RunningLock by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest075, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest075: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest075:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest075: Failed to get PowerMgrService";
    }

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL);
    pms->CreateRunningLock(token, info);
    pms->Lock(token, info, 0);
    EXPECT_EQ(pms->IsUsed(token), true);
    sleep(SLEEP_WAIT_TIME_S*10);
    EXPECT_EQ(PowerState::AWAKE, pms->GetState());

    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction, GoToSleep(_, _, _)).Times(0);
    pms->MockProximity(RunningLockMgr::PROXIMITY_CLOSE);
    EXPECT_EQ(PowerState::INACTIVE, pms->GetState());

    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest075:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest075: end.";
}

/**
 * @tc.name: PowerMgrUnittest076
 * @tc.desc: test proximity RunningLock by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest076, TestSize.Level2)
{
    int i;
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest076: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest076:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest076: Failed to get PowerMgrService";
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
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction, GoToSleep(_, _, _)).Times(0);
    pms->MockProximity(RunningLockMgr::PROXIMITY_CLOSE);
    EXPECT_EQ(PowerState::INACTIVE, pms->GetState());

    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest076:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest076: end.";
}

/**
 * @tc.name: PowerMgrUnittest077
 * @tc.desc: test proximity RunningLock by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest077, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest077: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest077:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest077: Failed to get PowerMgrService";
    }

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL);
    pms->CreateRunningLock(token, info);
    EXPECT_CALL(*g_lockAction, Lock(_, _)).Times(1);
    pms->Lock(token, info, 0);
    EXPECT_EQ(pms->IsUsed(token), true);
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
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
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest077:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest077: end.";
}

/**
 * @tc.name: PowerMgrUnittest078
 * @tc.desc: test proximity RunningLock by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest078, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest078: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest078:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest078: Failed to get PowerMgrService";
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
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest078:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest078: end.";
}

/**
 * @tc.name: PowerMgrUnittest079
 * @tc.desc: test proximity RunningLock by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest079, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest079: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest079:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest079: Failed to get PowerMgrService";
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
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest079:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest079: end.";
}

/**
 * @tc.name: PowerMgrUnittest080
 * @tc.desc: test Auto SuspendDevice by mock after 15s
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest080, TestSize.Level2)
{
    int64_t time =15000;
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest080: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest080:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest080: Failed to get PowerMgrService";
    }

    pms->SetDisplayOffTime(time);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_ON));
    sleep(((time/1000)*2/3)+1);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_DIM));
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
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
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest080:End.");
    pms->SetDisplayOffTime(DEFAULT_DISPLAY_OFF_TIME);
    GTEST_LOG_(INFO) << "PowerMgrUnittest080: end.";
}

/**
 * @tc.name: PowerMgrUnittest081
 * @tc.desc: test Auto SuspendDevice by mock after 30s
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest081, TestSize.Level2)
{
    int64_t time =30000;
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest081: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest081:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest081: Failed to get PowerMgrService";
    }

    pms->SetDisplayOffTime(time);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_ON));
    sleep(((time/1000)*2/3)+1);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_DIM));
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
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
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest081:End");
    pms->SetDisplayOffTime(DEFAULT_DISPLAY_OFF_TIME);
    GTEST_LOG_(INFO) << "PowerMgrUnittest081: end.";
}

/**
 * @tc.name: PowerMgrUnittest082
 * @tc.desc: test Auto SuspendDevice by mock after 1min
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest082, TestSize.Level2)
{
    int64_t time =60000;
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest082: start.";

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest082: Failed to get PowerMgrService";
    }

    pms->SetDisplayOffTime(time);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_ON));
    sleep(((time/1000)*2/3)+1);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_DIM));
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
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
    GTEST_LOG_(INFO) << "PowerMgrUnittest082: end.";
}

/**
 * @tc.name: PowerMgrUnittest083
 * @tc.desc: test Auto SuspendDevice by mock after 2mins
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest083, TestSize.Level2)
{
    int64_t time =120000;
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest083: start.";

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest083: Failed to get PowerMgrService";
    }

    pms->SetDisplayOffTime(time);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_ON));
    sleep(((time/1000)*2/3)+1);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_DIM));
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
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
    GTEST_LOG_(INFO) << "PowerMgrUnittest083: end.";
}

/**
 * @tc.name: PowerMgrUnittest084
 * @tc.desc: test Auto SuspendDevice by mock after 5mins
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest084, TestSize.Level2)
{
    int64_t time =300000;
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest084: start.";

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest084: Failed to get PowerMgrService";
    }

    pms->SetDisplayOffTime(time);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_ON));
    sleep(((time/1000)*2/3)+1);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_DIM));
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
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
    GTEST_LOG_(INFO) << "PowerMgrUnittest084: end.";
}

/**
 * @tc.name: PowerMgrUnittest085
 * @tc.desc: test Auto SuspendDevice by mock after 10mins
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest085, TestSize.Level2)
{
    int64_t time =600000;
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest085: start.";

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest085: Failed to get PowerMgrService";
    }

    pms->SetDisplayOffTime(time);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_ON));
    sleep(((time/1000)*2/3)+1);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_DIM));
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
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
    GTEST_LOG_(INFO) << "PowerMgrUnittest085: end.";
}

/**
 * @tc.name: PowerMgrUnittest086
 * @tc.desc: test Auto DIM by mock after 15s
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest086, TestSize.Level2)
{
    int64_t time =15000;
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest086: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest086:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest086: Failed to get PowerMgrService";
    }
    pms->SetDisplayOffTime(time);
    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN, std::string("test"));
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest086:DeviceStateAction::SetDisplayState.");
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_DIM, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest086:Start sleep.");
    sleep(time/1000-2);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_DIM));
    sleep(3);
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest086: sleep end.");

    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest086:End.");
    pms->SetDisplayOffTime(DEFAULT_DISPLAY_OFF_TIME);
    GTEST_LOG_(INFO) << "PowerMgrUnittest086: end.";
}

/**
 * @tc.name: PowerMgrUnittest087
 * @tc.desc: test Auto DIM by mock after 30s
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest087, TestSize.Level2)
{
    int64_t time =30000;
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest087: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest0087:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest087: Failed to get PowerMgrService";
    }
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_DIM, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    pms->SetDisplayOffTime(time);
    sleep(time/1000-2);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_DIM));
    sleep(3);

    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest087:End.");
    pms->SetDisplayOffTime(DEFAULT_DISPLAY_OFF_TIME);
    GTEST_LOG_(INFO) << "PowerMgrUnittest087: end.";
}

/**
 * @tc.name: PowerMgrUnittest088
 * @tc.desc: test Auto DIM by mock after 60s
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest088, TestSize.Level2)
{
    int64_t time =60000;
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest088: start.";

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest088: Failed to get PowerMgrService";
    }
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_DIM, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    pms->SetDisplayOffTime(time);
    sleep(time/1000-2);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_DIM));
    sleep(3);

    ResetMockAction();
    pms->SetDisplayOffTime(DEFAULT_DISPLAY_OFF_TIME);
    GTEST_LOG_(INFO) << "PowerMgrUnittest088: end.";
}

/**
 * @tc.name: PowerMgrUnittest089
 * @tc.desc: test Auto DIM by mock after 2min
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest089, TestSize.Level2)
{
    int64_t time =120000;
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest089: start.";

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest089: Failed to get PowerMgrService";
    }
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_DIM, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    pms->SetDisplayOffTime(time);
    sleep(time/1000-2);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_DIM));
    sleep(3);

    ResetMockAction();
    pms->SetDisplayOffTime(DEFAULT_DISPLAY_OFF_TIME);
    GTEST_LOG_(INFO) << "PowerMgrUnittest089: end.";
}

/**
 * @tc.name: PowerMgrUnittest090
 * @tc.desc: test Auto DIM by mock after 5min
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest090, TestSize.Level2)
{
    int64_t time =300000;
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest090: start.";

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest090: Failed to get PowerMgrService";
    }
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_DIM, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    pms->SetDisplayOffTime(time);
    sleep(time/1000-2);
    ON_CALL(*g_stateAction, GetDisplayState())
        .WillByDefault(::testing::Return(DisplayState::DISPLAY_DIM));
    sleep(3);

    ResetMockAction();
    pms->SetDisplayOffTime(DEFAULT_DISPLAY_OFF_TIME);
    GTEST_LOG_(INFO) << "PowerMgrUnittest090: end.";
}

/**
 * @tc.name: PowerMgrUnittest091
 * @tc.desc: test proximity screen control RunningLock by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest091, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest091: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest091:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrUnittest091: Failed to get PowerMgrService";
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
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(::testing::AtLeast(1))
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    sleep((SCREEN_OFF_WAIT_TIME_S*1/3) + 1);
    EXPECT_EQ(PowerState::INACTIVE, pms->GetState());

    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest091:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest091: end.";
}

/**
 * @tc.name: PowerMgrUnittest092
 * @tc.desc: test RebootDeviceForDeprecated by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrUnittest001, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrUnittest092: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest092:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    EXPECT_CALL(*g_powerAction, Reboot(std::string("test"))).Times(1);
    pms->RebootDeviceForDeprecated(std::string("test"));

    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrUnittest092:End.");
    GTEST_LOG_(INFO) << "PowerMgrUnittest092: end.";
}
}
