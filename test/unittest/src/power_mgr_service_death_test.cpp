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

#include "power_mgr_service_death_test.h"

#include "mock_power_remote_object.h"
#include "power_mgr_client.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

namespace {
/**
 * @tc.name: PowerMgrServiceDeathTest_001
 * @tc.desc: test OnRemoteDied function(Input remoteObj is nullptr, function don't reset proxy)
 * @tc.type: FUNC
 * @tc.require: issueI650CX
 */
HWTEST_F (PowerMgrServiceDeathTest, PowerMgrServiceDeathTest_001, TestSize.Level0)
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    EXPECT_EQ(powerMgrClient.Connect(), ERR_OK);

    std::shared_ptr<IRemoteObject::DeathRecipient> deathRecipient =
        std::make_shared<PowerMgrClient::PowerMgrDeathRecipient>();
    wptr<IRemoteObject> remoteObj = nullptr;
    EXPECT_NE(deathRecipient, nullptr);
    deathRecipient->OnRemoteDied(remoteObj);
    EXPECT_NE(powerMgrClient.proxy_, nullptr);
}

/**
 * @tc.name: PowerMgrServiceDeathTest_002
 * @tc.desc: test OnRemoteDied function(Input remoteObj is not nullptr, function don't reset proxy)
 * @tc.type: FUNC
 * @tc.require: issueI650CX
 */
HWTEST_F (PowerMgrServiceDeathTest, PowerMgrServiceDeathTest_002, TestSize.Level0)
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    EXPECT_EQ(powerMgrClient.Connect(), ERR_OK);

    std::shared_ptr<IRemoteObject::DeathRecipient> deathRecipient =
        std::make_shared<PowerMgrClient::PowerMgrDeathRecipient>();
    EXPECT_NE(deathRecipient, nullptr);
    sptr<IRemoteObject> sptrRemoteObj = new MockPowerRemoteObject();
    EXPECT_FALSE(sptrRemoteObj == nullptr);
    deathRecipient->OnRemoteDied(sptrRemoteObj);
    EXPECT_NE(powerMgrClient.proxy_, nullptr);
}
}