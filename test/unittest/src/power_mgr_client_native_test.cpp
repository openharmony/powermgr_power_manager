/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "power_mgr_client_native_test.h"

#include <iostream>

#include <datetime_ex.h>
#include <gtest/gtest.h>
#include <if_system_ability_manager.h>
#include <iservice_registry.h>
#include <system_ability_definition.h>
#include <ipc_skeleton.h>
#include <string_ex.h>
#include "running_lock_token_stub.h"
#include "power_common.h"
#include "power_mgr_client.h"
#include "power_mgr_service.h"
#include "power_state_machine.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

void PowerStateTestCallback::OnPowerStateChanged(PowerState state)
{
    POWER_HILOGI(LABEL_TEST, "PowerStateTestCallback::OnPowerStateChanged state = %u.",
        static_cast<uint32_t>(state));
}

namespace {
/**
 * @tc.name: PowerMgrClientNative001
 * @tc.desc: test callback in PowerMgrClient
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientNativeTest, PowerMgrClientNative001, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrClient001::fun is start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    wptr<IRemoteObject> remoteObject_ = sam->CheckSystemAbility(POWER_MANAGER_SERVICE_ID);
    powerMgrClient.ResetProxy(remoteObject_);
    powerMgrClient.~PowerMgrClient();
    EXPECT_TRUE(powerMgrClient.Connect() == ERR_OK);
    powerMgrClient.ResetProxy(remoteObject_);
    powerMgrClient.ResetProxy(nullptr);
    if (powerMgrClient.deathRecipient_ != nullptr) {
        powerMgrClient.deathRecipient_->OnRemoteDied(remoteObject_);
        powerMgrClient.deathRecipient_->OnRemoteDied(nullptr);
    }
    EXPECT_FALSE(powerMgrClient.RegisterSyncSleepCallback(nullptr, SleepPriority::DEFAULT));
    EXPECT_FALSE(powerMgrClient.UnRegisterSyncSleepCallback(nullptr));
    EXPECT_FALSE(powerMgrClient.RegisterPowerStateCallback(nullptr));
    EXPECT_FALSE(powerMgrClient.UnRegisterPowerStateCallback(nullptr));
    EXPECT_FALSE(powerMgrClient.RegisterPowerModeCallback(nullptr));
    EXPECT_FALSE(powerMgrClient.UnRegisterPowerModeCallback(nullptr));

    EXPECT_FALSE(powerMgrClient.RegisterRunningLockCallback(nullptr));
    EXPECT_FALSE(powerMgrClient.UnRegisterRunningLockCallback(nullptr));
    EXPECT_FALSE(powerMgrClient.RegisterScreenStateCallback(0, nullptr));
    EXPECT_FALSE(powerMgrClient.UnRegisterScreenStateCallback(nullptr));
    powerMgrClient.~PowerMgrClient();

    POWER_HILOGI(LABEL_TEST, "PowerMgrClient001::fun is end!");
}

/**
 * @tc.name: RunningLockNative001
 * @tc.desc: test init in RunningLock
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrClientNativeTest, RunningLockNative002, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockNative002::fun is start!");
    std::shared_ptr<RunningLock> runningLock =
        std::make_shared<RunningLock>(nullptr, "runninglock1", RunningLockType::RUNNINGLOCK_SCREEN);
    runningLock->Create();
    EXPECT_TRUE(runningLock->UnLock() != ERR_OK);
    runningLock->Release();
    POWER_HILOGI(LABEL_TEST, "RunningLockNative002::fun is end!");
}
}