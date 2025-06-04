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
#ifdef POWER_MANAGER_ENABLE_WATCH_CUSTOMIZED_SCREEN_COMMON_EVENT_RULES
#include "power_state_machine_info.h"
#endif

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
    POWER_HILOGI(LABEL_TEST, "GetRebootCommandTest function start!");
    std::string rebootReason = "reboot_by_test";
    std::string rebootCmd;
    PowerExtIntfWrapper::ErrCode code = PowerExtIntfWrapper::Instance().GetRebootCommand(rebootReason, rebootCmd);
    EXPECT_TRUE(code == PowerExtIntfWrapper::ErrCode::ERR_NOT_FOUND || code == PowerExtIntfWrapper::ErrCode::ERR_OK);
    POWER_HILOGI(LABEL_TEST, "GetRebootCommandTest function end!");
}

/**
 * @tc.name: SubscribeScreenLockCommonEventTest
 * @tc.desc: test SubscribeScreenLockCommonEvent/UnSubscribeScreenLockCommonEvent
 * @tc.type: FUNC
 */
HWTEST_F(PowerExtIntfWrapperTest, SubscribeScreenLockCommonEventTest, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "SubscribeScreenLockCommonEventTest function start!");
    PowerExtIntfWrapper::ErrCode code = PowerExtIntfWrapper::Instance().SubscribeScreenLockCommonEvent();
    EXPECT_TRUE(code == PowerExtIntfWrapper::ErrCode::ERR_NOT_FOUND || code == PowerExtIntfWrapper::ErrCode::ERR_OK);
    code = PowerExtIntfWrapper::Instance().UnSubscribeScreenLockCommonEvent();
    EXPECT_TRUE(code == PowerExtIntfWrapper::ErrCode::ERR_NOT_FOUND || code == PowerExtIntfWrapper::ErrCode::ERR_OK);
    POWER_HILOGI(LABEL_TEST, "SubscribeScreenLockCommonEventTest function end!");
}

/**
 * @tc.name: BlockHibernateUntilScrLckReadyTest
 * @tc.desc: test BlockHibernateUntilScrLckReady
 * @tc.type: FUNC
 */
HWTEST_F(PowerExtIntfWrapperTest, BlockHibernateUntilScrLckReadyTest, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "BlockHibernateUntilScrLckReadyTest function start!");
    PowerExtIntfWrapper::ErrCode code = PowerExtIntfWrapper::Instance().SubscribeScreenLockCommonEvent();
    EXPECT_TRUE(code == PowerExtIntfWrapper::ErrCode::ERR_NOT_FOUND || code == PowerExtIntfWrapper::ErrCode::ERR_OK);
    code = PowerExtIntfWrapper::Instance().BlockHibernateUntilScrLckReady();
    EXPECT_TRUE(code == PowerExtIntfWrapper::ErrCode::ERR_NOT_FOUND || code == PowerExtIntfWrapper::ErrCode::ERR_OK);
    code = PowerExtIntfWrapper::Instance().UnSubscribeScreenLockCommonEvent();
    EXPECT_TRUE(code == PowerExtIntfWrapper::ErrCode::ERR_NOT_FOUND || code == PowerExtIntfWrapper::ErrCode::ERR_OK);
    POWER_HILOGI(LABEL_TEST, "BlockHibernateUntilScrLckReadyTest function end!");
}

/**
 * @tc.name: SetScreenOnEventRulesTest
 * @tc.desc: test SetScreenOnEventRules
 * @tc.type: FUNC
 */
#ifdef POWER_MANAGER_ENABLE_WATCH_CUSTOMIZED_SCREEN_COMMON_EVENT_RULES
HWTEST_F(PowerExtIntfWrapperTest, SetScreenOnEventRulesTest, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "SetScreenOnEventRulesTest function start!");
    StateChangeReason reason = StateChangeReason::STATE_CHANGE_REASON_PICKUP;
    std::vector<StateChangeReason> WATCH_CUSTOMIZED_STATE_CHANGE_REASONS {StateChangeReason::STATE_CHANGE_REASON_PICKUP,
        StateChangeReason::STATE_CHANGE_REASON_INCOMING_CALL,
        StateChangeReason::STATE_CHANGE_REASON_BLUETOOTH_INCOMING_CALL};
    std::vector<WakeupDeviceType> WATCH_CUSTOMIZED_WAKEUP_DEVICE_TYPES {WakeupDeviceType::WAKEUP_DEVICE_PICKUP,
        WakeupDeviceType::WAKEUP_DEVICE_INCOMING_CALL,
        WakeupDeviceType::WAKEUP_DEVICE_BLUETOOTH_INCOMING_CALL};
    PowerExtIntfWrapper::ErrCode code = PowerExtIntfWrapper::Instance().SetScreenOnEventRules(
        reason, WATCH_CUSTOMIZED_STATE_CHANGE_REASONS, WATCH_CUSTOMIZED_WAKEUP_DEVICE_TYPES);
    EXPECT_TRUE(code == PowerExtIntfWrapper::ErrCode::ERR_NOT_FOUND ||
        code == PowerExtIntfWrapper::ErrCode::ERR_OK);
    POWER_HILOGI(LABEL_TEST, "SetScreenOnEventRulesTest function end!");
}
#endif
} // PowerMgr
} // OHOS