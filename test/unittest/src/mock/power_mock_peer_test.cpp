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

#include "power_mock_peer_test.h"

#include <datetime_ex.h>

#include "mock_power_remote_object.h"
#include "parcel.h"
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
#include "power_mgr_async_reply_stub.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

void MockPeerTest::PowerModeTestCallback::OnPowerModeChanged(PowerMode mode)
{
    POWER_HILOGI(LABEL_TEST, "PowerModeTestCallback::OnPowerModeChanged.");
}

void MockPeerTest::PowerStateTestCallback::OnPowerStateChanged(PowerState state)
{
    POWER_HILOGI(LABEL_TEST, "PowerStateTestCallback::OnPowerStateChanged.");
}

void MockPeerTest::PowerRunningLockTestCallback::HandleRunningLockMessage(std::string message)
{
    POWER_HILOGI(LABEL_TEST, "PowerRunningLockTestCallback::HandleRunningLockMessage.");
}
namespace {
/**
 * @tc.name: PowerClientMockPeerTest001
 * @tc.desc: Test Power client by mock peer, callback is nullptr
 * @tc.type: FUNC
 * @tc.require: issueI650CX
 */
HWTEST_F(MockPeerTest, PowerClientMockPeerTest001, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerClientMockPeerTest001 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    sptr<IPowerStateCallback> stateCallback = nullptr;
    sptr<IPowerModeCallback> modeCallback= nullptr;
    sptr<IPowerRunninglockCallback> runninglockCallback = nullptr;

    EXPECT_FALSE(powerMgrClient.RegisterPowerStateCallback(stateCallback));
    EXPECT_FALSE(powerMgrClient.UnRegisterPowerStateCallback(stateCallback));
    EXPECT_FALSE(powerMgrClient.RegisterPowerModeCallback(modeCallback));
    EXPECT_FALSE(powerMgrClient.UnRegisterPowerModeCallback(modeCallback));
    EXPECT_FALSE(powerMgrClient.RegisterRunningLockCallback(runninglockCallback));
    EXPECT_FALSE(powerMgrClient.UnRegisterRunningLockCallback(runninglockCallback));
    EXPECT_EQ(powerMgrClient.ForceSuspendDevice("-1"), PowerErrors::ERR_CONNECTION_FAIL);
    EXPECT_EQ(powerMgrClient.Hibernate(true, "", "-1"), PowerErrors::ERR_CONNECTION_FAIL);
    POWER_HILOGI(LABEL_TEST, "PowerClientMockPeerTest001 function end!");
}

/**
 * @tc.name: PowerClientMockPeerTest002
 * @tc.desc: Test Power client by mock peer, callback is not nullptr
 * @tc.type: FUNC
 * @tc.require: issueI5IUHE
 */
HWTEST_F(MockPeerTest, PowerClientMockPeerTest002, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerClientMockPeerTest002 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    sptr<IPowerStateCallback> stateCallback = new PowerStateTestCallback();
    sptr<IPowerModeCallback> modeCallback = new PowerModeTestCallback();
    sptr<IPowerRunninglockCallback> runninglockCallback = new PowerRunningLockTestCallback();

    EXPECT_FALSE(powerMgrClient.RegisterPowerStateCallback(stateCallback));
    EXPECT_FALSE(powerMgrClient.UnRegisterPowerStateCallback(stateCallback));
    EXPECT_FALSE(powerMgrClient.RegisterPowerModeCallback(modeCallback));
    EXPECT_FALSE(powerMgrClient.UnRegisterPowerModeCallback(modeCallback));
    EXPECT_FALSE(powerMgrClient.RegisterRunningLockCallback(runninglockCallback));
    EXPECT_FALSE(powerMgrClient.UnRegisterRunningLockCallback(runninglockCallback));
    EXPECT_EQ(powerMgrClient.ForceSuspendDevice("-1"), PowerErrors::ERR_CONNECTION_FAIL);
    EXPECT_EQ(powerMgrClient.Hibernate(true, "", "-1"), PowerErrors::ERR_CONNECTION_FAIL);
    POWER_HILOGI(LABEL_TEST, "PowerClientMockPeerTest002 function end!");
}

/**
 * @tc.name: MockPeerTest001
 * @tc.desc: Test proxy when the PeerHolder is nullptr
 * @tc.type: FUNC
 * @tc.require: issueI5IUHE
 */
HWTEST_F(MockPeerTest, MockPeerTest001, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "MockPeerTest001 function start!");
    pid_t uid = 0;
    pid_t pid = 0;
    int32_t powerError = 1;
    int32_t timeOutMs = -1;
    std::string name;
    bool lockTypeSupported = false;
    bool isUsed = false;
    sptr<IPCObjectStub> remote = new IPCObjectStub();
    std::shared_ptr<PowerMgrProxy> sptrProxy = std::make_shared<PowerMgrProxy>(remote);
    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_SCREEN);
    sptrProxy->CreateRunningLockIpc(token, info, powerError);
    EXPECT_FALSE(powerError == static_cast<int32_t>(PowerErrors::ERR_OK));
    EXPECT_NE(sptrProxy->ReleaseRunningLockIpc(token, name), ERR_OK);
    sptrProxy->IsRunningLockTypeSupportedIpc(
        static_cast<int32_t>(RunningLockType::RUNNINGLOCK_BUTT), lockTypeSupported);
    EXPECT_FALSE(lockTypeSupported);
    sptrProxy->LockIpc(token, timeOutMs, powerError);
    EXPECT_FALSE(powerError == static_cast<int32_t>(PowerErrors::ERR_OK));
    sptrProxy->UnLockIpc(token, name, powerError);
    EXPECT_FALSE(powerError == static_cast<int32_t>(PowerErrors::ERR_OK));
    sptrProxy->IsUsedIpc(token, isUsed);
    EXPECT_FALSE(isUsed);
    EXPECT_NE(sptrProxy->ProxyRunningLockIpc(true, pid, uid), ERR_OK);
    std::unique_ptr<VectorPair> vectorPairInfos = std::make_unique<VectorPair>();
    std::vector<std::pair<pid_t, pid_t>> processInfos;
    processInfos.emplace_back(pid, uid);
    vectorPairInfos->SetProcessInfos(processInfos);
    EXPECT_NE(sptrProxy->ProxyRunningLocksIpc(true, *vectorPairInfos), ERR_OK);
    EXPECT_NE(sptrProxy->ResetRunningLocksIpc(), ERR_OK);
    POWER_HILOGI(LABEL_TEST, "MockPeerTest001 function end!");
}

/**
 * @tc.name: MockPeerTest002
 * @tc.desc: Test proxy when the PeerHolder is nullptr
 * @tc.type: FUNC
 * @tc.require: issueI5IUHE
 */
HWTEST_F(MockPeerTest, MockPeerTest002, TestSize.Level2)
{
    int32_t powerError = 1;
    int32_t powerMode = 0;
    int32_t powerState = -1;
    std::string apiVersion = "-1";
    int32_t ret = 0;
    bool isScreenOn = false;
    POWER_HILOGI(LABEL_TEST, "MockPeerTest002 function start!");
    sptr<IPCObjectStub> remote = new IPCObjectStub();
    std::shared_ptr<PowerMgrProxy> sptrProxy = std::make_shared<PowerMgrProxy>(remote);
    int32_t suspendReason = (static_cast<int32_t>(SuspendDeviceType::SUSPEND_DEVICE_REASON_MAX)) + 1;
    SuspendDeviceType abnormaltype = SuspendDeviceType(suspendReason);
    ret = sptrProxy->SuspendDeviceIpc(0, static_cast<int32_t>(abnormaltype), false, apiVersion, powerError);
    EXPECT_EQ(ret, ERR_INVALID_DATA);
    ret = sptrProxy->WakeupDeviceIpc(GetTickCount(), static_cast<int32_t>(WakeupDeviceType::WAKEUP_DEVICE_APPLICATION),
        std::string("app call"), apiVersion, powerError);
    EXPECT_EQ(ret, ERR_INVALID_DATA);
    int32_t userAttention = static_cast<int32_t>(UserActivityType::USER_ACTIVITY_TYPE_ATTENTION);
    EXPECT_NE(sptrProxy->RefreshActivityIpc(GetTickCount(), userAttention, true), ERR_OK);
    sptrProxy->OverrideScreenOffTimeIpc(200, apiVersion, powerError);
    EXPECT_FALSE(powerError == static_cast<int32_t>(PowerErrors::ERR_OK));
    sptrProxy->RestoreScreenOffTimeIpc(apiVersion, powerError);
    EXPECT_FALSE(powerError == static_cast<int32_t>(PowerErrors::ERR_OK));
    ret = sptrProxy->GetStateIpc(powerState);
    EXPECT_EQ(ret, ERR_INVALID_DATA);
    sptrProxy->IsScreenOnIpc(true, isScreenOn);
    EXPECT_FALSE(isScreenOn);
    sptrProxy->SetDisplaySuspendIpc(true);
    PowerMode mode1 = PowerMode::NORMAL_MODE;
    ret = sptrProxy->SetDeviceModeIpc(static_cast<int32_t>(mode1), powerError);
    EXPECT_EQ(ret, ERR_INVALID_DATA);
    sptrProxy->GetDeviceModeIpc(powerMode);
    EXPECT_FALSE(powerMode == static_cast<int32_t>(mode1));
    POWER_HILOGI(LABEL_TEST, "MockPeerTest002 function end!");
}

/**
 * @tc.name: MockPeerTest003
 * @tc.desc: Test proxy when the PeerHolder is nullptr
 * @tc.type: FUNC
 * @tc.require: issueI5IUHE
 */
HWTEST_F(MockPeerTest, MockPeerTest003, TestSize.Level2)
{
    int32_t powerError = 1;
    std::string apiVersion = "-1";
    int32_t ret = 0;
    POWER_HILOGI(LABEL_TEST, "MockPeerTest003 function start!");
    sptr<IPCObjectStub> remote = new IPCObjectStub();
    std::shared_ptr<PowerMgrProxy> sptrProxy = std::make_shared<PowerMgrProxy>(remote);
    sptr<IPowerStateCallback> cb1 = new PowerStateTestCallback();
    sptr<IPowerModeCallback> cb3 = new PowerModeTestCallback();
    sptr<IPowerRunninglockCallback> cb5 =new PowerRunningLockTestCallback();
    sptr<PowerMgrStubAsync> asyncCallback = new PowerMgrStubAsync();
    sptr<IPowerMgrAsync> powerProxy = iface_cast<IPowerMgrAsync>(asyncCallback);
    EXPECT_NE(sptrProxy->RegisterPowerStateCallbackIpc(cb1, true), ERR_OK);
    EXPECT_NE(sptrProxy->UnRegisterPowerStateCallbackIpc(cb1), ERR_OK);
    EXPECT_NE(sptrProxy->RegisterPowerStateCallbackIpc(nullptr, true), ERR_OK);
    EXPECT_NE(sptrProxy->UnRegisterPowerStateCallbackIpc(nullptr), ERR_OK);
    EXPECT_NE(sptrProxy->RegisterPowerModeCallbackIpc(cb3), ERR_OK);
    EXPECT_NE(sptrProxy->UnRegisterPowerModeCallbackIpc(cb3), ERR_OK);
    EXPECT_NE(sptrProxy->RegisterPowerModeCallbackIpc(nullptr), ERR_OK);
    EXPECT_NE(sptrProxy->UnRegisterPowerModeCallbackIpc(nullptr), ERR_OK);
    EXPECT_NE(sptrProxy->RegisterRunningLockCallbackIpc(cb5), ERR_OK);
    EXPECT_NE(sptrProxy->UnRegisterRunningLockCallbackIpc(cb5), ERR_OK);
    EXPECT_NE(sptrProxy->RegisterRunningLockCallbackIpc(nullptr), ERR_OK);
    EXPECT_NE(sptrProxy->UnRegisterRunningLockCallbackIpc(nullptr), ERR_OK);
    ret = sptrProxy->RebootDeviceIpc(" ", powerError);
    EXPECT_EQ(ret, ERR_INVALID_DATA);
    ret = sptrProxy->ShutDownDeviceIpc(" ", powerError);
    EXPECT_EQ(ret, ERR_INVALID_DATA);
    ret = sptrProxy->ForceSuspendDeviceIpc(0, apiVersion, powerProxy);
    EXPECT_EQ(ret, ERR_INVALID_DATA);
    static std::vector<std::string> dumpArgs;
    dumpArgs.push_back("-a");
    std::string errorCode;
    std::string actualDebugInfo;
    sptrProxy->ShellDumpIpc(dumpArgs, dumpArgs.size(), actualDebugInfo);
    EXPECT_EQ(errorCode, actualDebugInfo);
    POWER_HILOGI(LABEL_TEST, "MockPeerTest003 function end!");
}
} // namespace