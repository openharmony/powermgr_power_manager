/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "powermgr_service_mock_parcel_test.h"

#include <datetime_ex.h>

#include "power_log.h"
#include "power_mode_callback_proxy.h"
#include "power_mgr_service.h"
#include "power_runninglock_callback_proxy.h"
#include "power_state_callback_proxy.h"
#include "power_state_machine_info.h"
#include "powermgr_service_test_proxy.h"
#include "running_lock_token_stub.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

namespace {
static sptr<PowerMgrService> g_powerMgrService = nullptr;
static std::shared_ptr<PowerMgrServiceTestProxy> g_powerMgrServiceProxy = nullptr;
} // namespace

void PowerMgrServiceMockParcelTest::SetUpTestCase()
{
    g_powerMgrService = DelayedSpSingleton<PowerMgrService>::GetInstance();
    g_powerMgrService->OnStart();

    if (g_powerMgrServiceProxy == nullptr) {
        g_powerMgrServiceProxy = std::make_shared<PowerMgrServiceTestProxy>(g_powerMgrService);
    }
}

void PowerMgrServiceMockParcelTest::TearDownTestCase()
{
    g_powerMgrService->OnStop();
    DelayedSpSingleton<PowerMgrService>::DestroyInstance();
}

void PowerMgrServiceMockParcelTest::PowerModeTestCallback::OnPowerModeChanged(PowerMode mode)
{
    POWER_HILOGI(LABEL_TEST, "PowerModeTestCallback::OnPowerModeChanged.");
}

void PowerMgrServiceMockParcelTest::PowerStateTestCallback::OnPowerStateChanged(PowerState state)
{
    POWER_HILOGI(LABEL_TEST, "PowerStateTestCallback::OnPowerStateChanged.");
}

void PowerMgrServiceMockParcelTest::PowerRunningLockTestCallback::HandleRunningLockMessage(std::string message)
{
    POWER_HILOGI(LABEL_TEST, "PowerRunningLockTestCallback::HandleRunningLockMessage.");
}
namespace {
/**
 * @tc.name: PowerMgrServiceMockParcelTest001
 * @tc.desc: PowerMgr service test when the Parcel is mock
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceMockParcelTest, PowerMgrServiceMockParcelTest001, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceMockParcelTest001 start.");
    ASSERT_NE(g_powerMgrServiceProxy, nullptr);
    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo runningLockInfo;
    runningLockInfo.name = "runninglock";
    runningLockInfo.type = RunningLockType::RUNNINGLOCK_SCREEN;
    pid_t uid = 0;
    pid_t pid = 0;
    int32_t timeOutMs = 0;
    int32_t powerError = 1;
    std::string name;
    bool lockTypeSupported = false;
    bool isUsed = false;
    g_powerMgrServiceProxy->CreateRunningLockIpc(token, runningLockInfo, powerError);
    EXPECT_FALSE(powerError == static_cast<int32_t>(PowerErrors::ERR_OK));
    EXPECT_NE(g_powerMgrServiceProxy->ReleaseRunningLockIpc(token), ERR_OK);
    g_powerMgrServiceProxy->IsRunningLockTypeSupportedIpc(
        static_cast<int32_t>(RunningLockType::RUNNINGLOCK_BUTT), lockTypeSupported);
    EXPECT_FALSE(lockTypeSupported);
    g_powerMgrServiceProxy->LockIpc(token, timeOutMs, powerError);
    EXPECT_NE(powerError, ERR_OK);
    g_powerMgrServiceProxy->UnLockIpc(token, name, powerError);
    EXPECT_NE(powerError, ERR_OK);
    g_powerMgrServiceProxy->IsUsedIpc(token, isUsed);
    EXPECT_FALSE(isUsed);
    EXPECT_NE(g_powerMgrServiceProxy->ProxyRunningLockIpc(true, pid, uid), ERR_OK);
    std::unique_ptr<VectorPair> vectorPairInfos = std::make_unique<VectorPair>();
    std::vector<std::pair<pid_t, pid_t>> processInfos;
    processInfos.emplace_back(pid, uid);
    vectorPairInfos->SetProcessInfos(processInfos);
    EXPECT_NE(g_powerMgrServiceProxy->ProxyRunningLocksIpc(true, *vectorPairInfos), ERR_OK);
    EXPECT_NE(g_powerMgrServiceProxy->ResetRunningLocksIpc(), ERR_OK);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceMockParcelTest001 end.");
}

/**
 * @tc.name: PowerMgrServiceMockParcelTest002
 * @tc.desc: PowerMgr service test when the Parcel is mock
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceMockParcelTest, PowerMgrServiceMockParcelTest002, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceMockParcelTest002 start.");
    int32_t PARM_ONE = 1;
    int32_t powerError = 1;
    int32_t powerState = -1;
    std::string apiVersion = "-1";
    int32_t ret = 0;
    bool isScreenOn = false;
    ASSERT_NE(g_powerMgrServiceProxy, nullptr);
    int32_t suspendReason = (static_cast<int32_t>(SuspendDeviceType::SUSPEND_DEVICE_REASON_MAX)) + PARM_ONE;
    ret = g_powerMgrServiceProxy->SuspendDeviceIpc(0, suspendReason, false, apiVersion, powerError);
    EXPECT_EQ(ret, ERR_INVALID_DATA);
    ret = g_powerMgrServiceProxy->WakeupDeviceIpc(GetTickCount(),
        static_cast<int32_t>(WakeupDeviceType::WAKEUP_DEVICE_APPLICATION),
        std::string("app call"), apiVersion, powerError);
    EXPECT_EQ(ret, ERR_INVALID_DATA);
    int32_t attention = static_cast<int32_t>(UserActivityType::USER_ACTIVITY_TYPE_ATTENTION);
    EXPECT_NE(g_powerMgrServiceProxy->RefreshActivityIpc(GetTickCount(), attention, true), ERR_OK);
    g_powerMgrServiceProxy->OverrideScreenOffTimeIpc(200, powerError);
    EXPECT_FALSE(powerError == static_cast<int32_t>(PowerErrors::ERR_OK));
    g_powerMgrServiceProxy->RestoreScreenOffTimeIpc(apiVersion, powerError);
    EXPECT_FALSE(powerError == static_cast<int32_t>(PowerErrors::ERR_OK));
    ret = g_powerMgrServiceProxy->GetStateIpc(powerState);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    g_powerMgrServiceProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_FALSE(isScreenOn);
    g_powerMgrServiceProxy->SetDisplaySuspendIpc(true);
    PowerMode setMode = PowerMode::NORMAL_MODE;
    ret = g_powerMgrServiceProxy->SetDeviceModeIpc(static_cast<int32_t>(setMode), powerError);
    EXPECT_EQ(ret, ERR_INVALID_DATA);
    int32_t getMode = 0;
    ret = g_powerMgrServiceProxy->GetDeviceModeIpc(getMode);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceMockParcelTest002 end.");
}

/**
 * @tc.name: PowerMgrServiceMockParcelTest003
 * @tc.desc: PowerMgr service test when the Parcel is mock
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceMockParcelTest, PowerMgrServiceMockParcelTest003, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceMockParcelTest003 start.");
    int32_t powerError = 1;
    std::string apiVersion = "-1";
    ASSERT_NE(g_powerMgrServiceProxy, nullptr);
    sptr<IPowerStateCallback> stateCb = new PowerStateTestCallback();
    sptr<IPowerModeCallback> modeCb = new PowerModeTestCallback();
    sptr<IPowerRunninglockCallback> RunninglockCb =new PowerRunningLockTestCallback();
    EXPECT_NE(g_powerMgrServiceProxy->RegisterPowerStateCallbackIpc(stateCb), ERR_OK);
    EXPECT_NE(g_powerMgrServiceProxy->UnRegisterPowerStateCallbackIpc(stateCb), ERR_OK);
    EXPECT_NE(g_powerMgrServiceProxy->RegisterPowerStateCallbackIpc(nullptr), ERR_OK);
    EXPECT_NE(g_powerMgrServiceProxy->UnRegisterPowerStateCallbackIpc(nullptr), ERR_OK);
    EXPECT_NE(g_powerMgrServiceProxy->RegisterPowerModeCallbackIpc(modeCb), ERR_OK);
    EXPECT_NE(g_powerMgrServiceProxy->UnRegisterPowerModeCallbackIpc(modeCb), ERR_OK);
    EXPECT_NE(g_powerMgrServiceProxy->RegisterPowerModeCallbackIpc(nullptr), ERR_OK);
    EXPECT_NE(g_powerMgrServiceProxy->UnRegisterPowerModeCallbackIpc(nullptr), ERR_OK);
    EXPECT_NE(g_powerMgrServiceProxy->RegisterRunningLockCallbackIpc(RunninglockCb), ERR_OK);
    EXPECT_NE(g_powerMgrServiceProxy->UnRegisterRunningLockCallbackIpc(RunninglockCb), ERR_OK);
    EXPECT_NE(g_powerMgrServiceProxy->RegisterRunningLockCallbackIpc(nullptr), ERR_OK);
    EXPECT_NE(g_powerMgrServiceProxy->UnRegisterRunningLockCallbackIpc(nullptr), ERR_OK);
    EXPECT_FALSE(g_powerMgrServiceProxy->ForceSuspendDeviceIpc(0) == ERR_OK);
    static std::vector<std::string> dumpArgs;
    dumpArgs.push_back("-a");
    std::string errorCode;
    std::string actualDebugInfo;
    g_powerMgrServiceProxy->ShellDumpIpc(dumpArgs, dumpArgs.size(), actualDebugInfo);
    EXPECT_EQ(errorCode, actualDebugInfo);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceMockParcelTest003 end.");
}
} // namespace