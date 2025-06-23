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

#include "power_mock_object_test.h"

#include <datetime_ex.h>

#include "errors.h"
#include "mock_power_remote_object.h"
#include "power_log.h"
#include "power_mgr_client.h"
#include "power_mgr_proxy.h"
#include "power_mode_callback_proxy.h"
#include "power_state_callback_proxy.h"
#include "power_runninglock_callback_proxy.h"
#include "running_lock.h"
#include "running_lock_info.h"
#include "running_lock_token_stub.h"
#include "power_mgr_async_reply_stub.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

void PowerMockObjectTest::PowerModeTestCallback::OnPowerModeChanged(PowerMode mode)
{
    POWER_HILOGI(LABEL_TEST, "PowerModeTestCallback::OnPowerModeChanged.");
}

void PowerMockObjectTest::PowerStateTestCallback::OnPowerStateChanged(PowerState state)
{
    POWER_HILOGI(LABEL_TEST, "PowerStateTestCallback::OnPowerStateChanged.");
}

void PowerMockObjectTest::PowerRunningLockTestCallback::HandleRunningLockMessage(std::string message)
{
    POWER_HILOGI(LABEL_TEST, "PowerRunningLockTestCallback::HandleRunningLockMessage.");
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
    int32_t powerError = 1;
    int32_t timeOutMs = -1;
    int32_t powerState = -1;
    int32_t ret = 0;
    std::string name;
    bool isUsed = false;
    POWER_HILOGI(LABEL_TEST, "PowerMockObjectTest001 function start!");
    PowerMode mode = PowerMode::NORMAL_MODE;
    sptr<MockPowerRemoteObject> remote = new MockPowerRemoteObject();
    std::shared_ptr<PowerMgrProxy> sptrProxy = std::make_shared<PowerMgrProxy>(remote);
    std::shared_ptr<PowerModeCallbackProxy> callbackProxy = std::make_shared<PowerModeCallbackProxy>(remote);
    callbackProxy->OnPowerModeChanged(mode);
    std::shared_ptr<PowerStateCallbackProxy> stateCallbackProxy = std::make_shared<PowerStateCallbackProxy>(remote);
    PowerState state = PowerState::AWAKE;
    stateCallbackProxy->OnPowerStateChanged(state);
    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_SCREEN);
    sptrProxy->CreateRunningLockIpc(token, info, powerError);
    EXPECT_FALSE(powerError == static_cast<int32_t>(PowerErrors::ERR_OK));
    EXPECT_NE(sptrProxy->ReleaseRunningLockIpc(token, name), ERR_OK);
    ret = sptrProxy->GetStateIpc(powerState);
    EXPECT_EQ(ret, ERR_TRANSACTION_FAILED);
    sptrProxy->LockIpc(token, timeOutMs, powerError);
    EXPECT_FALSE(powerError == static_cast<int32_t>(PowerErrors::ERR_OK));
    sptrProxy->UnLockIpc(token, name, powerError);
    EXPECT_FALSE(powerError == static_cast<int32_t>(PowerErrors::ERR_OK));
    sptrProxy->IsUsedIpc(token, isUsed);
    EXPECT_FALSE(isUsed);
    POWER_HILOGI(LABEL_TEST, "PowerMockObjectTest001 function end!");
}

/**
 * @tc.name: PowerMockObjectTest002
 * @tc.desc: Test Power proxy when PowerRemoteObject is mock
 * @tc.type: FUNC
 * @tc.require: issueI5IUHE
 */
HWTEST_F(PowerMockObjectTest, PowerMockObjectTest002, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMockObjectTest002 function start!");
    pid_t uid = 0;
    pid_t pid = 0;
    int32_t powerError = 1;
    int32_t powerMode = 0;
    int32_t ret = 0;
    std::string apiVersion = "-1";
    bool lockTypeSupported = false;
    sptr<MockPowerRemoteObject> remote = new MockPowerRemoteObject();
    std::shared_ptr<PowerMgrProxy> sptrProxy = std::make_shared<PowerMgrProxy>(remote);
    PowerMode mode = PowerMode::NORMAL_MODE;
    int32_t suspendReason = (static_cast<int32_t>(SuspendDeviceType::SUSPEND_DEVICE_REASON_MAX)) + 1;
    SuspendDeviceType abnormaltype = SuspendDeviceType(suspendReason);
    ret = sptrProxy->SuspendDeviceIpc(0, static_cast<int32_t>(abnormaltype), false, apiVersion, powerError);
    EXPECT_EQ(ret, ERR_TRANSACTION_FAILED);
    EXPECT_NE(sptrProxy->SetDisplaySuspendIpc(true), ERR_OK);
    ret = sptrProxy->WakeupDeviceIpc(GetTickCount(), static_cast<int32_t>(WakeupDeviceType::WAKEUP_DEVICE_APPLICATION),
        std::string("app call"), apiVersion, powerError);
    EXPECT_EQ(ret, ERR_TRANSACTION_FAILED);
    int32_t attention = static_cast<int32_t>(UserActivityType::USER_ACTIVITY_TYPE_ATTENTION);
    EXPECT_NE(sptrProxy->RefreshActivityIpc(GetTickCount(), attention, true), ERR_OK);
    ret = sptrProxy->SetDeviceModeIpc(static_cast<int32_t>(mode), powerError);
    EXPECT_EQ(ret, ERR_TRANSACTION_FAILED);
    sptrProxy->GetDeviceModeIpc(powerMode);
    EXPECT_FALSE(powerMode == static_cast<int32_t>(mode));
    EXPECT_NE(sptrProxy->ProxyRunningLockIpc(true, pid, uid), ERR_OK);
    sptrProxy->IsRunningLockTypeSupportedIpc(
        static_cast<int32_t>(RunningLockType::RUNNINGLOCK_BACKGROUND), lockTypeSupported);
    EXPECT_FALSE(lockTypeSupported);
    sptrProxy->OverrideScreenOffTimeIpc(200, apiVersion, powerError);
    EXPECT_FALSE(powerError == static_cast<int32_t>(PowerErrors::ERR_OK));
    sptrProxy->RestoreScreenOffTimeIpc(apiVersion, powerError);
    EXPECT_FALSE(powerError == static_cast<int32_t>(PowerErrors::ERR_OK));
    POWER_HILOGI(LABEL_TEST, "PowerMockObjectTest002 function end!");
}

/**
 * @tc.name: PowerMockObjectTest003
 * @tc.desc: Test Power proxy when PowerRemoteObject is mock
 * @tc.type: FUNC
 * @tc.require: issueI9C4GG
 */
HWTEST_F(PowerMockObjectTest, PowerMockObjectTest003, TestSize.Level2)
{
    int32_t powerError = 1;
    int32_t timeOutMs = -1;
    std::string name;
    bool isUsed = false;
    POWER_HILOGI(LABEL_TEST, "PowerMockObjectTest003 function start!");
    sptr<MockPowerRemoteObject> remote = new MockPowerRemoteObject();
    std::shared_ptr<PowerMgrProxy> sptrProxy = std::make_shared<PowerMgrProxy>(remote);
    std::shared_ptr<PowerRunningLockCallbackProxy> callbackProxy =
        std::make_shared<PowerRunningLockCallbackProxy>(remote);
    std::string message = "runninglock message";
    callbackProxy->HandleRunningLockMessage(message);
    sptr<IRemoteObject> token = new RunningLockTokenStub();
#ifdef HAS_SENSORS_SENSOR_PART
    RunningLockInfo info("test2", RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL);
    sptrProxy->CreateRunningLockIpc(token, info, powerError);
    EXPECT_FALSE(powerError == static_cast<int32_t>(PowerErrors::ERR_OK));
    EXPECT_NE(sptrProxy->ReleaseRunningLockIpc(token, name), ERR_OK);
    sptrProxy->LockIpc(token, timeOutMs, powerError);
    EXPECT_FALSE(powerError == static_cast<int32_t>(PowerErrors::ERR_OK));
    sptrProxy->UnLockIpc(token, name, powerError);
    EXPECT_FALSE(powerError == static_cast<int32_t>(PowerErrors::ERR_OK));
    sptrProxy->IsUsedIpc(token, isUsed);
    EXPECT_FALSE(isUsed);
#endif
    POWER_HILOGI(LABEL_TEST, "PowerMockObjectTest003 function end!");
}

/**
 * @tc.name: PowerMockObjectTest004
 * @tc.desc: Test Power proxy when PowerRemoteObject is mock
 * @tc.type: FUNC
 * @tc.require: issueI5IUHE
 */
HWTEST_F(PowerMockObjectTest, PowerMockObjectTest004, TestSize.Level2)
{
    int32_t powerError = 1;
    std::string apiVersion = "-1";
    POWER_HILOGI(LABEL_TEST, "PowerMockObjectTest004 function start!");
    sptr<MockPowerRemoteObject> remote = new MockPowerRemoteObject();
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
    sptrProxy->RebootDeviceIpc(" ", powerError);
    sptrProxy->RebootDeviceForDeprecatedIpc(" ", powerError);
    sptrProxy->ShutDownDeviceIpc(" ", powerError);
    int32_t ret = sptrProxy->ForceSuspendDeviceIpc(0, apiVersion, powerProxy);
    EXPECT_EQ(ret, ERR_TRANSACTION_FAILED);
    POWER_HILOGI(LABEL_TEST, "PowerMockObjectTest004 function end!");
}
} // namespace