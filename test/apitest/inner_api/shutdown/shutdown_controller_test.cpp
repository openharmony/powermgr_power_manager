/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#include <mock_power_action.h>
#include <mock_state_action.h>
#include <power_log.h>
#include <shutdown/takeover_shutdown_callback_stub.h>
#include <shutdown_controller.h>

namespace OHOS::PowerMgr {
class ShutDownControllerTest : public testing::Test {
public:
    bool called = false;
    class TakeOverShutdownCallback : public TakeOverShutdownCallbackStub {
    public:
        ~TakeOverShutdownCallback() override = default;
        bool OnTakeOverShutdown(const TakeOverInfo& info) override
        {
            return true;
        }
    };
};

namespace {
using namespace OHOS;
using namespace PowerMgr;
using namespace testing;
using namespace ext;
HWTEST_F(ShutDownControllerTest, ShutDownControllerTest001, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "ShutdownControllerTest001 function start!");
    sptr<ITakeOverShutdownCallback> callback = sptr<TakeOverShutdownCallback>::MakeSptr();
    NiceMock<MockStateAction>* stateActionMock = new NiceMock<MockStateAction>;
    NiceMock<MockPowerAction>* powerActionMock = new NiceMock<MockPowerAction>;
    ShutdownController controller;
    controller.EnableMock(powerActionMock, stateActionMock);
    controller.AddCallback(callback, ShutdownPriority::DEFAULT);
    EXPECT_CALL(*powerActionMock, Reboot("::testing::_")).Times(0);
    controller.Reboot("test_case");
    EXPECT_CALL(*powerActionMock, Reboot("test_case"));
    controller.Reboot("test_case", true);
    sleep(1);
    POWER_HILOGI(LABEL_TEST, "ShutdownControllerTest001 function end!");
}
}
}