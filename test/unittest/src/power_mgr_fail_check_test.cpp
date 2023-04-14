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
    delete g_powerState;
    delete g_stateAction;
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
 * @tc.name: PowerMgrFailCheck001
 * @tc.desc: test SuspendDevice failed by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrFailCheck001, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrFailCheck001: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrFailCheck001:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrFailCheck001: Failed to get PowerMgrService";
    }

    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::FAILED));
    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_POWER_BUTTON, false);

    std::vector<std::string> args;
    std::string str("-s");
    args.push_back(str);
    std::string dumpInfo = pms->ShellDump(args, args.size());
    GTEST_LOG_(INFO) << dumpInfo;

    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrFailCheck001:End.");
    GTEST_LOG_(INFO) << "PowerMgrFailCheck001: end.";
}

/**
 * @tc.name: PowerMgrFailCheck002
 * @tc.desc: test WakeupDevice failed by mock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrMockTest, PowerMgrFailCheck002, TestSize.Level2)
{
    sleep(NEXT_WAIT_TIME_S);
    GTEST_LOG_(INFO) << "PowerMgrFailCheck002:  start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrFailCheck002:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrFailCheck002: Failed to get PowerMgrService";
    }

    pms->SuspendDevice(0, SuspendDeviceType::SUSPEND_DEVICE_REASON_POWER_BUTTON, false);
    EXPECT_CALL(*g_stateAction, SetDisplayState(DisplayState::DISPLAY_ON, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Return(ActionResult::FAILED));
    pms->WakeupDevice(0, WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON, std::string("test"));

    std::vector<std::string> args;
    std::string str("-s");
    args.push_back(str);
    std::string dumpInfo = pms->ShellDump(args, args.size());
    GTEST_LOG_(INFO) << dumpInfo;

    ResetMockAction();
    POWER_HILOGD(LABEL_TEST, "PowerMgrFailCheck002:End.");
    GTEST_LOG_(INFO) << "PowerMgrFailCheck002: end.";
}
}