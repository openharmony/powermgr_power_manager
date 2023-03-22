/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "power_mgr_mock_system_test.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;
using ::testing::_;

static sptr<PowerMgrService> g_service;
static MockStateAction* g_shutdownState;
static MockStateAction* g_powerState;
static MockPowerAction* g_powerAction;
static MockLockAction* g_lockAction;

static void ResetMockAction()
{
    POWER_HILOGD(LABEL_TEST, "ResetMockAction:Start.");
    g_powerState = new MockStateAction();
    g_shutdownState = new MockStateAction();
    g_powerAction = new MockPowerAction();
    g_lockAction = new MockLockAction();
    g_service->EnableMock(g_powerState, g_shutdownState, g_powerAction, g_lockAction);
}

void PowerMgrMockSystemTest::SetUpTestCase(void)
{
    // create singleton g_service object at the beginning
    g_service = DelayedSpSingleton<PowerMgrService>::GetInstance();
    g_service->OnStart();
}

void PowerMgrMockSystemTest::TearDownTestCase(void)
{
    g_service->OnStop();
    DelayedSpSingleton<PowerMgrService>::DestroyInstance();
}

void PowerMgrMockSystemTest::SetUp(void)
{
    ResetMockAction();
}

void PowerMgrMockSystemTest::TearDown(void)
{
}

namespace {
/**
 * @tc.name: PowerMgrMock106
 * @tc.desc: test proximity RunningLock by mock
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrMockSystemTest, PowerMgrMock106, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerMgrMock106: start.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrMock106:Start.");

    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock106: Failed to get PowerMgrService";
    }

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL);
    pms->CreateRunningLock(token, info);
    pms->SetDisplaySuspend(true);
    pms->Lock(token, info, 0);
    EXPECT_EQ(pms->IsUsed(token), true);
    pms->MockProximity(RunningLockMgr::PROXIMITY_CLOSE);
    EXPECT_EQ(PowerState::AWAKE, pms->GetState());

    POWER_HILOGD(LABEL_TEST, "PowerMgrMock106:End.");
    GTEST_LOG_(INFO) << "PowerMgrMock106: end.";
}
}