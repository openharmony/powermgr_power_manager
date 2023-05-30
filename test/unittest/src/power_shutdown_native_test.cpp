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

#include "power_shutdown_native_test.h"

#include <iostream>

#include <datetime_ex.h>
#include <if_system_ability_manager.h>
#include <ipc_skeleton.h>
#include <string_ex.h>

#include "power_common.h"
#include "power_mgr_client.h"
#include "power_mgr_service.h"
#include "power_state_machine.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

void PowerShutdownNativeTest::SetUpTestCase()
{
}

void PowerShutdownNativeTest::TearDownTestCase()
{
}

void PowerShutdownNativeTest::SetUp()
{
}

void PowerShutdownNativeTest::TearDown()
{
}

void PowerShutdownTest1Callback::ShutdownCallback()
{
    POWER_HILOGI(LABEL_TEST, "PowerShutdownTest1Callback::ShutdownCallback.");
}

void PowerShutdownTest2Callback::ShutdownCallback()
{
    POWER_HILOGI(LABEL_TEST, "PowerShutdownTest2Callback::ShutdownCallback.");
}

namespace {
/**
 * @tc.name: PowerShutdownNative
 * @tc.desc: test callback in ShutdownService
 * @tc.type: FUNC
 */
HWTEST_F (PowerShutdownNativeTest, PowerShutdownNative001, TestSize.Level0)
{
    shared_ptr<ShutdownService> powerShutdownTest = make_shared<ShutdownService>();
    EXPECT_FALSE(powerShutdownTest->IsShuttingDown());
    auto priority = IShutdownCallback::ShutdownPriority::POWER_SHUTDOWN_PRIORITY_LOW;
    sptr<IShutdownCallback> powerShutdownCb = new PowerShutdownTest1Callback();
    powerShutdownTest->AddShutdownCallback(priority, powerShutdownCb);
    priority = IShutdownCallback::ShutdownPriority::POWER_SHUTDOWN_PRIORITY_DEFAULT;
    powerShutdownTest->AddShutdownCallback(priority, powerShutdownCb);
    priority = IShutdownCallback::ShutdownPriority::POWER_SHUTDOWN_PRIORITY_HIGH;
    powerShutdownTest->AddShutdownCallback(priority, powerShutdownCb);
    powerShutdownTest->AddShutdownCallback(priority, nullptr);
    priority = static_cast<IShutdownCallback::ShutdownPriority>(7U);
    powerShutdownTest->AddShutdownCallback(priority, powerShutdownCb);
    powerShutdownTest->Prepare();
    powerShutdownTest->DelShutdownCallback(powerShutdownCb);

    POWER_HILOGI(LABEL_TEST, "PowerShutdownNativeTest::PowerShutdownNative001 end.");
}

/**
 * @tc.name: PowerShutdownNative002
 * @tc.desc: test the registration of ShutdownCallback
 * @tc.type: FUNC
 */
HWTEST_F (PowerShutdownNativeTest, PowerShutdownNative002, TestSize.Level0)
{
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();

    auto priority = IShutdownCallback::ShutdownPriority::POWER_SHUTDOWN_PRIORITY_LOW;
    sptr<IShutdownCallback> powerShutdownCb = new PowerShutdownTest2Callback();
    ASSERT_TRUE(pmsTest->RegisterShutdownCallback(priority, powerShutdownCb) == true);
    ASSERT_TRUE(pmsTest->UnRegisterShutdownCallback(powerShutdownCb) == true);

    wptr<IRemoteObject> remoteObject = new PowerShutdownTest1Callback();
    auto stateMachineCallback = std::make_shared<PowerStateMachine::PowerStateCallbackDeathRecipient>();
    stateMachineCallback->OnRemoteDied(remoteObject);
    stateMachineCallback->OnRemoteDied(nullptr);

    ShutdownService *powerShutdownTest = new ShutdownService();
    powerShutdownTest->devicePowerAction_ = nullptr;
    powerShutdownTest->RebootOrShutdown("77", false);
    POWER_HILOGI(LABEL_TEST, "PowerShutdownNativeTest::PowerShutdownNative002 end.");
}
}
