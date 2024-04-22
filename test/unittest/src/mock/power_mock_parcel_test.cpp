/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "power_mock_parcel_test.h"

#include <datetime_ex.h>

#include "mock_power_remote_object.h"
#include "parcel.h"
#include "permission.h"
#include "power_log.h"
#include "power_mgr_client.h"
#include "power_mgr_proxy.h"
#include "power_mode_callback_proxy.h"
#include "power_state_callback_proxy.h"
#include "power_runninglock_callback_proxy.h"
#include "power_state_machine_info.h"
#include "running_lock.h"
#include "running_lock_info.h"
#include "running_lock_token_stub.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

void MockParcelTest::PowerModeTestCallback::OnPowerModeChanged(PowerMode mode)
{
    POWER_HILOGI(LABEL_TEST, "PowerModeTestCallback::OnPowerModeChanged.");
}

void MockParcelTest::PowerStateTestCallback::OnPowerStateChanged(PowerState state)
{
    POWER_HILOGI(LABEL_TEST, "PowerStateTestCallback::OnPowerStateChanged.");
}

void MockParcelTest::PowerRunningLockTestCallback::HandleRunningLockMessage(std::string message)
{
    POWER_HILOGI(LABEL_TEST, "PowerRunningLockTestCallback::HandleRunningLockMessage.");
}
namespace {
/**
 * @tc.name: PowerMockParcelTest001
 * @tc.desc: Test Power Client when the Parcel is mock
 * @tc.type: FUNC
 * @tc.require: issueI5IUHE
 */
HWTEST_F(MockParcelTest, PowerMockParcelTest001, TestSize.Level2)
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    EXPECT_FALSE(powerMgrClient.IsRunningLockTypeSupported(RunningLockType::RUNNINGLOCK_BUTT));
    EXPECT_FALSE(powerMgrClient.IsRunningLockTypeSupported(RunningLockType::RUNNINGLOCK_SCREEN));
    EXPECT_FALSE(powerMgrClient.IsScreenOn());
    powerMgrClient.SetDisplaySuspend(true);
    powerMgrClient.WakeupDevice();
    powerMgrClient.RefreshActivity();
    powerMgrClient.RefreshActivity(UserActivityType::USER_ACTIVITY_TYPE_ATTENTION);
    powerMgrClient.GetState();
    powerMgrClient.RebootDevice(" ");
    powerMgrClient.RebootDeviceForDeprecated(" ");
    powerMgrClient.ShutDownDevice(" ");
    powerMgrClient.ForceSuspendDevice();
    int32_t suspendReason = (static_cast<int32_t>(SuspendDeviceType::SUSPEND_DEVICE_REASON_MAX)) + 1;
    SuspendDeviceType abnormaltype = SuspendDeviceType(suspendReason);
    powerMgrClient.SuspendDevice(abnormaltype, false);
    EXPECT_FALSE(powerMgrClient.OverrideScreenOffTime(100));
    EXPECT_FALSE(powerMgrClient.RestoreScreenOffTime());
}

/**
 * @tc.name: PowerMockParcelTest002
 * @tc.desc: Test Power Client when the Parcel is mock
 * @tc.type: FUNC
 * @tc.require: issueI5IUHE
 */
HWTEST_F(MockParcelTest, PowerMockParcelTest002, TestSize.Level2)
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    PowerMode mode1 = PowerMode::POWER_SAVE_MODE;
    powerMgrClient.SetDeviceMode(mode1);
    powerMgrClient.GetDeviceMode();
    sptr<IPowerStateCallback> stateCallback = new PowerStateTestCallback();
    sptr<IPowerModeCallback> modeCallback = new PowerModeTestCallback();
    sptr<IPowerRunninglockCallback> runninglockCallback = new PowerRunningLockTestCallback();

    EXPECT_FALSE(powerMgrClient.RegisterPowerStateCallback(stateCallback));
    EXPECT_FALSE(powerMgrClient.UnRegisterPowerStateCallback(stateCallback));
    EXPECT_FALSE(powerMgrClient.RegisterPowerModeCallback(modeCallback));
    EXPECT_FALSE(powerMgrClient.UnRegisterPowerModeCallback(modeCallback));
    EXPECT_FALSE(powerMgrClient.RegisterRunningLockCallback(runninglockCallback));
    EXPECT_FALSE(powerMgrClient.UnRegisterRunningLockCallback(runninglockCallback));
    static std::vector<std::string> dumpArgs;
    dumpArgs.push_back("-a");
    std::string errCode = "can't connect service";
    std::string actualDebugInfo = powerMgrClient.Dump(dumpArgs);
    EXPECT_EQ(actualDebugInfo, errCode);
}

/**
 * @tc.name: PowerMockParcelTest003
 * @tc.desc: Test lock info when the Parcel is mock
 * @tc.type: FUNC
 * @tc.require: issueI5IUHE
 */
HWTEST_F(MockParcelTest, PowerMockParcelTest003, TestSize.Level2)
{
    Parcel parcel;
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_BACKGROUND);
    EXPECT_FALSE(info.ReadFromParcel(parcel));
    EXPECT_FALSE(info.Marshalling(parcel));
    EXPECT_FALSE(info.Unmarshalling(parcel));
}

/**
 * @tc.name: PowerMockParcelTest004
 * @tc.desc: Test proxy when the parcel is mock
 * @tc.type: FUNC
 * @tc.require: issueI5IUHE
 */
HWTEST_F(MockParcelTest, PowerMockParcelTest004, TestSize.Level2)
{
    pid_t uid = 0;
    pid_t pid = 0;
    sptr<IPCObjectStub> remote = new IPCObjectStub();
    std::shared_ptr<PowerMgrProxy> sptrProxy = std::make_shared<PowerMgrProxy>(remote);
    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_SCREEN);
    sptrProxy->CreateRunningLock(token, info);
    sptrProxy->ReleaseRunningLock(token);
    sptrProxy->ProxyRunningLock(true, pid, uid);
    EXPECT_FALSE(sptrProxy->IsRunningLockTypeSupported(RunningLockType::RUNNINGLOCK_BUTT));
    sptrProxy->Lock(token);
    sptrProxy->UnLock(token);
    EXPECT_FALSE(sptrProxy->IsUsed(token));
}

/**
 * @tc.name: PowerMockParcelTest005
 * @tc.desc: Test proxy when the parcel is mock
 * @tc.type: FUNC
 * @tc.require: issueI5IUHE
 */
HWTEST_F(MockParcelTest, PowerMockParcelTest005, TestSize.Level2)
{
    bool ret = 0;
    sptr<IPCObjectStub> remote = new IPCObjectStub();
    std::shared_ptr<PowerMgrProxy> sptrProxy = std::make_shared<PowerMgrProxy>(remote);
    sptr<IPowerStateCallback> cb1 = new PowerStateTestCallback();
    sptr<IPowerModeCallback> cb3 = new PowerModeTestCallback();
    sptr<IPowerRunninglockCallback> cb5 =new PowerRunningLockTestCallback();
    ret = sptrProxy->RegisterPowerStateCallback(cb1);
    EXPECT_EQ(ret, false);
    sptrProxy->UnRegisterPowerStateCallback(cb1);
    sptrProxy->RegisterPowerStateCallback(nullptr);
    sptrProxy->UnRegisterPowerStateCallback(nullptr);
    EXPECT_FALSE(sptrProxy->RegisterPowerModeCallback(cb3));
    EXPECT_FALSE(sptrProxy->UnRegisterPowerModeCallback(cb3));
    EXPECT_FALSE(sptrProxy->RegisterPowerModeCallback(nullptr));
    EXPECT_FALSE(sptrProxy->UnRegisterPowerModeCallback(nullptr));
    EXPECT_FALSE(sptrProxy->RegisterRunningLockCallback(cb5));
    EXPECT_FALSE(sptrProxy->UnRegisterRunningLockCallback(cb5));
    EXPECT_FALSE(sptrProxy->RegisterRunningLockCallback(nullptr));
    EXPECT_FALSE(sptrProxy->UnRegisterRunningLockCallback(nullptr));
}

/**
 * @tc.name: PowerMockParcelTest006
 * @tc.desc: Test proxy when the parcel is mock
 * @tc.type: FUNC
 * @tc.require: issueI5IUHE
 */
HWTEST_F(MockParcelTest, PowerMockParcelTest006, TestSize.Level2)
{
    sptr<IPCObjectStub> remote = new IPCObjectStub();
    std::shared_ptr<PowerMgrProxy> sptrProxy = std::make_shared<PowerMgrProxy>(remote);
    int32_t suspendReason = (static_cast<int32_t>(SuspendDeviceType::SUSPEND_DEVICE_REASON_MAX)) + 1;
    SuspendDeviceType abnormaltype = SuspendDeviceType(suspendReason);
    sptrProxy->SuspendDevice(0, abnormaltype, false);
    sptrProxy->WakeupDevice(GetTickCount(), WakeupDeviceType::WAKEUP_DEVICE_APPLICATION, std::string("app call"));
    sptrProxy->RefreshActivity(GetTickCount(), UserActivityType::USER_ACTIVITY_TYPE_ATTENTION, true);
    EXPECT_FALSE(sptrProxy->OverrideScreenOffTime(200));
    EXPECT_FALSE(sptrProxy->RestoreScreenOffTime());
    auto state = sptrProxy->GetState();
    EXPECT_EQ(state, PowerState::UNKNOWN);
    EXPECT_FALSE(sptrProxy->IsScreenOn());
    sptrProxy->SetDisplaySuspend(true);
    PowerMode mode1 = PowerMode::NORMAL_MODE;
    sptrProxy->SetDeviceMode(mode1);
    sptrProxy->GetDeviceMode();
    sptrProxy->RebootDevice(" ");
    sptrProxy->ShutDownDevice(" ");
    sptrProxy->ForceSuspendDevice(0);
    static std::vector<std::string> dumpArgs;
    dumpArgs.push_back("-a");
    std::string errCode = "remote error";
    std::string actualDebugInfo = sptrProxy->ShellDump(dumpArgs, dumpArgs.size());
    EXPECT_EQ(actualDebugInfo, errCode);
}

/**
 * @tc.name: PowerUtilMockParcelTest001
 * @tc.desc: test Permission function
 * @tc.type: FUNC
 * @tc.require: issueI650CX
 */
HWTEST_F (MockParcelTest, PowerUtilMockParcelTest001, TestSize.Level2)
{
    EXPECT_TRUE(Permission::IsSystem());
    EXPECT_TRUE(Permission::IsPermissionGranted(""));
}
} // namespace
