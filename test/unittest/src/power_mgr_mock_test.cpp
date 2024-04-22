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
#include "power_mgr_client.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;
using ::testing::_;

static sptr<PowerMgrService> g_service;
static MockStateAction* g_shutdownState;
static MockStateAction* g_stateAction;
static MockPowerAction* g_powerAction;
static MockLockAction* g_lockAction;

static void ResetMockAction()
{
    POWER_HILOGI(LABEL_TEST, "ResetMockAction:Start.");
    g_stateAction = new MockStateAction();
    g_shutdownState = new MockStateAction();
    g_powerAction = new MockPowerAction();
    g_lockAction = new MockLockAction();
    g_service->EnableMock(g_stateAction, g_shutdownState, g_powerAction, g_lockAction);
}

void PowerMgrMockTest::SetUpTestCase(void)
{
    // create singleton service object at the beginning
    g_service = DelayedSpSingleton<PowerMgrService>::GetInstance();
    g_service->OnStart();
}

void PowerMgrMockTest::TearDownTestCase(void)
{
    g_service->OnStop();
    DelayedSpSingleton<PowerMgrService>::DestroyInstance();
}

void PowerMgrMockTest::SetUp(void)
{
    ResetMockAction();
}

void PowerMgrMockTest::TearDown(void)
{
}

namespace {
/**
 * @tc.name: PowerMgrMock001
 * @tc.desc: test RebootDeviceForDeprecated by mock
 * @tc.type: FUNC
 * @tc.require: issueI6MWC0
 */
HWTEST_F(PowerMgrMockTest, PowerMgrMock003, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerMgrMock003: start.";
    POWER_HILOGI(LABEL_TEST, "PowerMgrMock003:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock003: Failed to get PowerMgrService";
    }

    EXPECT_CALL(*g_powerAction, Reboot(std::string("test"))).Times(1);
    pms->RebootDeviceForDeprecated(std::string("test"));

    POWER_HILOGI(LABEL_TEST, "PowerMgrMock003:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock003: end.";
    usleep(SLEEP_WAIT_TIME_MS * TRANSFER_NS_TO_MS);
}
}
