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
#include "power_state_machine_info.h"
#include "running_lock.h"
#include "running_lock_info.h"
#include "running_lock_token_stub.h"
#include "work_trigger.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

void MockParcelTest::PowerModeTestCallback::OnPowerModeChanged(PowerMode mode)
{
    POWER_HILOGD(LABEL_TEST, "PowerModeTestCallback::OnPowerModeChanged.");
}

void MockParcelTest::PowerShutdownTestCallback::ShutdownCallback()
{
    POWER_HILOGD(LABEL_TEST, "PowerShutdownTestCallback::ShutdownCallback.");
}

void MockParcelTest::PowerStateTestCallback::OnPowerStateChanged(PowerState state)
{
    POWER_HILOGD(LABEL_TEST, "PowerStateTestCallback::OnPowerStateChanged.");
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
    auto type1 = static_cast<uint32_t>(RunningLockType::RUNNINGLOCK_BUTT);
    auto type2 = static_cast<uint32_t>(RunningLockType::RUNNINGLOCK_SCREEN);
    EXPECT_FALSE(powerMgrClient.IsRunningLockTypeSupported(type1));
    EXPECT_FALSE(powerMgrClient.IsRunningLockTypeSupported(type2));
    EXPECT_FALSE(powerMgrClient.IsScreenOn());
    powerMgrClient.SetDisplaySuspend(true);
    powerMgrClient.WakeupDevice();
    powerMgrClient.RefreshActivity();
    powerMgrClient.RefreshActivity(UserActivityType::USER_ACTIVITY_TYPE_ATTENTION);
    powerMgrClient.GetState();
    powerMgrClient.RebootDevice(" ");
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
    sptr<IShutdownCallback> shutdownCallback = new PowerShutdownTestCallback();
    sptr<IPowerModeCallback> modeCallback = new PowerModeTestCallback();

    EXPECT_FALSE(powerMgrClient.RegisterPowerStateCallback(stateCallback));
    EXPECT_FALSE(powerMgrClient.UnRegisterPowerStateCallback(stateCallback));
    EXPECT_FALSE(powerMgrClient.RegisterShutdownCallback(shutdownCallback));
    EXPECT_FALSE(powerMgrClient.UnRegisterShutdownCallback(shutdownCallback));
    EXPECT_FALSE(powerMgrClient.RegisterPowerModeCallback(modeCallback));
    EXPECT_FALSE(powerMgrClient.UnRegisterPowerModeCallback(modeCallback));
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
    EXPECT_FALSE(info.ReadFromParcelWorkTriggerList(parcel, info.workTriggerlist));
    EXPECT_FALSE(info.MarshallingWorkTriggerList(parcel, info.workTriggerlist));
    EXPECT_FALSE(info.ReadFromParcel(parcel));
    EXPECT_FALSE(info.Marshalling(parcel));
    EXPECT_FALSE(info.Unmarshalling(parcel));
    WorkTrigger* workTrigger = new WorkTrigger();
    EXPECT_FALSE(workTrigger->ReadFromParcel(parcel));
    EXPECT_FALSE(workTrigger->Marshalling(parcel));
    EXPECT_FALSE(workTrigger->Unmarshalling(parcel));
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
    WorkTriggerList worklist;
    worklist.push_back(nullptr);
    worklist.push_back(nullptr);
    worklist.push_back(nullptr);
    sptrProxy->SetWorkTriggerList(token, worklist);
    sptrProxy->ProxyRunningLock(true, uid, pid);
    auto type = static_cast<uint32_t>(RunningLockType::RUNNINGLOCK_BUTT);
    EXPECT_FALSE(sptrProxy->IsRunningLockTypeSupported(type));
    sptrProxy->Lock(token, info, 0);
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
    sptr<IPCObjectStub> remote = new IPCObjectStub();
    std::shared_ptr<PowerMgrProxy> sptrProxy = std::make_shared<PowerMgrProxy>(remote);
    sptr<IPowerStateCallback> cb1 = new PowerStateTestCallback();
    sptr<IShutdownCallback> cb2 = new PowerShutdownTestCallback();
    sptr<IPowerModeCallback> cb3 = new PowerModeTestCallback();
    auto priority = IShutdownCallback::ShutdownPriority::POWER_SHUTDOWN_PRIORITY_LOW;
    sptrProxy->RegisterPowerStateCallback(cb1);
    sptrProxy->UnRegisterPowerStateCallback(cb1);
    sptrProxy->RegisterPowerStateCallback(nullptr);
    sptrProxy->UnRegisterPowerStateCallback(nullptr);
    sptrProxy->RegisterShutdownCallback(priority, cb2);
    sptrProxy->UnRegisterShutdownCallback(cb2);
    sptrProxy->RegisterShutdownCallback(priority, nullptr);
    sptrProxy->UnRegisterShutdownCallback(nullptr);
    sptrProxy->RegisterPowerModeCallback(cb3);
    sptrProxy->UnRegisterPowerModeCallback(cb3);
    sptrProxy->RegisterPowerModeCallback(nullptr);
    sptrProxy->UnRegisterPowerModeCallback(nullptr);
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
    EXPECT_FALSE(Permission::IsSystemCore());
    EXPECT_FALSE(Permission::IsSystemBasic());
    EXPECT_FALSE(Permission::IsSystemApl());
    EXPECT_TRUE(Permission::IsSystemHap());
    EXPECT_TRUE(Permission::IsSystem());
    EXPECT_TRUE(Permission::IsPermissionGranted(""));
    EXPECT_TRUE(Permission::IsSystemHapPermGranted(""));
}
} // namespace
