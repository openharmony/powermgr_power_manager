/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "power_ext_intf_wrapper_test.h"
#include "power_ext_intf_wrapper.h"
#include "power_log.h"

namespace OHOS {
namespace PowerMgr {
using namespace testing;
using namespace testing::ext;

/**
 * @tc.name: GetRebootCommandTest
 * @tc.desc: test GetRebootCommand
 * @tc.type: FUNC
 */
HWTEST_F(PowerExtIntfWrapperTest, GetRebootCommandTest, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "GetRebootCommandTest start!");
    std::string rebootReason = "reboot_by_test";
    std::string rebootCmd;
    PowerExtIntfWrapper::Instance().GetRebootCommand(rebootReason, rebootCmd);
    POWER_HILOGI(LABEL_TEST, "GetRebootCommandTest end!");
}

/**
 * @tc.name: SubscribeScreenLockCommonEventTest
 * @tc.desc: test SubscribeScreenLockCommonEvent/UnSubscribeScreenLockCommonEvent
 * @tc.type: FUNC
 */
HWTEST_F(PowerExtIntfWrapperTest, SubscribeScreenLockCommonEventTest, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "SubscribeScreenLockCommonEventTest start!");
    PowerExtIntfWrapper::Instance().SubscribeScreenLockCommonEvent();
    PowerExtIntfWrapper::Instance().UnSubscribeScreenLockCommonEvent();
    POWER_HILOGI(LABEL_TEST, "SubscribeScreenLockCommonEventTest end!");
}

/**
 * @tc.name: BlockHibernateUntilScrLckReadyTest
 * @tc.desc: test BlockHibernateUntilScrLckReady
 * @tc.type: FUNC
 */
HWTEST_F(PowerExtIntfWrapperTest, BlockHibernateUntilScrLckReadyTest, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "BlockHibernateUntilScrLckReadyTest start!");
    PowerExtIntfWrapper::Instance().SubscribeScreenLockCommonEvent();
    PowerExtIntfWrapper::Instance().BlockHibernateUntilScrLckReady();
    PowerExtIntfWrapper::Instance().UnSubscribeScreenLockCommonEvent();
    POWER_HILOGI(LABEL_TEST, "BlockHibernateUntilScrLckReadyTest end!");
}
} // PowerMgr
} // OHOS