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

#include "power_mock_object_test.h"

#include <datetime_ex.h>

#include "errors.h"
#include "mock_power_remote_object.h"
#include "power_log.h"
#include "power_mgr_client.h"
#include "power_mgr_proxy.h"
#include "power_mode_callback_proxy.h"
#include "power_state_callback_proxy.h"
#include "running_lock.h"
#include "running_lock_info.h"
#include "running_lock_token_stub.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

void PowerMockObjectTest::PowerModeTestCallback::OnPowerModeChanged(PowerMode mode)
{
    POWER_HILOGD(LABEL_TEST, "PowerModeTestCallback::OnPowerModeChanged.");
}

void PowerMockObjectTest::PowerShutdownTestCallback::ShutdownCallback()
{
    POWER_HILOGD(LABEL_TEST, "PowerShutdownTestCallback::ShutdownCallback.");
}

void PowerMockObjectTest::PowerStateTestCallback::OnPowerStateChanged(PowerState state)
{
    POWER_HILOGD(LABEL_TEST, "PowerStateTestCallback::OnPowerStateChanged.");
}

namespace {
/**
 * @tc.name: PowerMockObjectTest001
 * @tc.desc: Test Power proxy when the PowerRemoteObject is mock
 * @tc.type: FUNC
 * @tc.require: issueI5IUHE
 */
HWTEST_F(PowerMockObjectTest, PowerMockObjectTest001, TestSize.Level2)
{
    WorkTriggerList worklist;
    worklist.push_back(nullptr);
    worklist.push_back(nullptr);
    worklist.push_back(nullptr);
    PowerMode mode = PowerMode::NORMAL_MODE;
    sptr<MockPowerRemoteObject> remote = new MockPowerRemoteObject();
    std::shared_ptr<PowerMgrProxy> sptrProxy = std::make_shared<PowerMgrProxy>(remote);
    std::shared_ptr<PowerModeCallbackProxy> callbackProxy = std::make_shared<PowerModeCallbackProxy>(remote);
    callbackProxy->OnPowerModeChanged(mode);
    std::shared_ptr<PowerStateCallbackProxy> stateCallbackProxy = std::make_shared<PowerStateCallbackProxy>(remote);
    PowerState state = PowerState::AWAKE;
    stateCallbackProxy->OnPowerStateChanged(state);
    sptr<IRemoteObject> token = new RunningLockTokenStub();
    EXPECT_FALSE(sptrProxy->SetWorkTriggerList(token, worklist));
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_SCREEN);
    EXPECT_FALSE(sptrProxy->CreateRunningLock(token, info) == PowerErrors::ERR_OK);
    EXPECT_FALSE(sptrProxy->ReleaseRunningLock(token));
    state = sptrProxy->GetState();
    EXPECT_EQ(state, PowerState::UNKNOWN);
    EXPECT_FALSE(sptrProxy->Lock(token, info, 0));
    EXPECT_FALSE(sptrProxy->UnLock(token));
    EXPECT_FALSE(sptrProxy->IsUsed(token));
}

/**
 * @tc.name: PowerMockObjectTest002
 * @tc.desc: Test Power proxy when PowerRemoteObject is mock
 * @tc.type: FUNC
 * @tc.require: issueI5IUHE
 */
HWTEST_F(PowerMockObjectTest, PowerMockObjectTest002, TestSize.Level2)
{
    pid_t uid = 0;
    pid_t pid = 0;
    sptr<MockPowerRemoteObject> remote = new MockPowerRemoteObject();
    std::shared_ptr<PowerMgrProxy> sptrProxy = std::make_shared<PowerMgrProxy>(remote);
    PowerMode mode = PowerMode::NORMAL_MODE;
    int32_t suspendReason = (static_cast<int32_t>(SuspendDeviceType::SUSPEND_DEVICE_REASON_MAX)) + 1;
    SuspendDeviceType abnormaltype = SuspendDeviceType(suspendReason);
    auto error1 = sptrProxy->SuspendDevice(0, abnormaltype, false);
    EXPECT_EQ(error1, PowerErrors::ERR_CONNECTION_FAIL);
    EXPECT_FALSE(sptrProxy->SetDisplaySuspend(true));
    auto error2 =
        sptrProxy->WakeupDevice(GetTickCount(), WakeupDeviceType::WAKEUP_DEVICE_APPLICATION, std::string("app call"));
    EXPECT_EQ(error2, PowerErrors::ERR_CONNECTION_FAIL);
    EXPECT_FALSE(sptrProxy->RefreshActivity(GetTickCount(), UserActivityType::USER_ACTIVITY_TYPE_ATTENTION, true));
    EXPECT_EQ(sptrProxy->SetDeviceMode(mode), PowerErrors::ERR_CONNECTION_FAIL);
    auto ret = sptrProxy->GetDeviceMode();
    EXPECT_TRUE(ret == mode);
    EXPECT_FALSE(sptrProxy->ProxyRunningLock(true, uid, pid));
    EXPECT_FALSE(sptrProxy->IsRunningLockTypeSupported(1));
    EXPECT_FALSE(sptrProxy->OverrideScreenOffTime(200));
    EXPECT_FALSE(sptrProxy->RestoreScreenOffTime());
}

/**
 * @tc.name: PowerMockObjectTest003
 * @tc.desc: Test Power proxy when PowerRemoteObject is mock
 * @tc.type: FUNC
 * @tc.require: issueI5IUHE
 */
HWTEST_F(PowerMockObjectTest, PowerMockObjectTest003, TestSize.Level2)
{
    sptr<MockPowerRemoteObject> remote = new MockPowerRemoteObject();
    std::shared_ptr<PowerMgrProxy> sptrProxy = std::make_shared<PowerMgrProxy>(remote);
    sptr<IPowerStateCallback> cb1 = new PowerStateTestCallback();
    sptr<IShutdownCallback> cb2 = new PowerShutdownTestCallback();
    sptr<IPowerModeCallback> cb3 = new PowerModeTestCallback();
    auto priority = IShutdownCallback::ShutdownPriority::POWER_SHUTDOWN_PRIORITY_LOW;
    EXPECT_FALSE(sptrProxy->RegisterPowerStateCallback(cb1));
    EXPECT_FALSE(sptrProxy->UnRegisterPowerStateCallback(cb1));
    EXPECT_FALSE(sptrProxy->RegisterPowerStateCallback(nullptr));
    EXPECT_FALSE(sptrProxy->UnRegisterPowerStateCallback(nullptr));
    EXPECT_FALSE(sptrProxy->RegisterShutdownCallback(priority, cb2));
    EXPECT_FALSE(sptrProxy->UnRegisterShutdownCallback(cb2));
    EXPECT_FALSE(sptrProxy->RegisterShutdownCallback(priority, nullptr));
    EXPECT_FALSE(sptrProxy->UnRegisterShutdownCallback(nullptr));
    EXPECT_FALSE(sptrProxy->RegisterPowerModeCallback(cb3));
    EXPECT_FALSE(sptrProxy->UnRegisterPowerModeCallback(cb3));
    EXPECT_FALSE(sptrProxy->RegisterPowerModeCallback(nullptr));
    EXPECT_FALSE(sptrProxy->UnRegisterPowerModeCallback(nullptr));
    sptrProxy->RebootDevice(" ");
    sptrProxy->RebootDeviceForDeprecated(" ");
    sptrProxy->ShutDownDevice(" ");
    EXPECT_FALSE(sptrProxy->ForceSuspendDevice(0));
}

/**
 * @tc.name: PowerMockObjectTest004
 * @tc.desc: Test running lock when the PowerRemoteObject is mock
 * @tc.type: FUNC
 * @tc.require: issueI5IUHE
 */
HWTEST_F(PowerMockObjectTest, PowerMockObjectTest004, TestSize.Level2)
{
    std::shared_ptr<RunningLock> runningLock =
        std::make_shared<RunningLock>(nullptr, "runninglock1", RunningLockType::RUNNINGLOCK_SCREEN);
    EXPECT_FALSE(ERR_OK == runningLock->Lock(10));
    EXPECT_FALSE(runningLock->IsUsed());
    runningLock->UnLock();
    WorkTriggerList workList;
    workList.push_back(nullptr);
    workList.push_back(nullptr);
    workList.push_back(nullptr);
    EXPECT_EQ(ERR_OK, runningLock->SetWorkTriggerList(workList));
    auto resList = runningLock->GetWorkTriggerList();
    EXPECT_EQ(*(resList.begin()), nullptr);
}
} // namespace