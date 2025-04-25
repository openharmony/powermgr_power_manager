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
#include "power_hookmgr.h"
#include "power_log.h"

namespace OHOS {
namespace PowerMgr {
namespace Test {
using namespace testing::ext;
class PowerHookMgrTest : public testing::Test {};

/**
 * @tc.name: PowerHookMgrTest001
 * @tc.desc: test GetPowerHookMgr
 * @tc.type: FUNC
 */
HWTEST_F(PowerHookMgrTest, PowerHookMgrTest001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerHookMgrTest001 function start!");
    HOOK_MGR* testHookMgr = GetPowerHookMgr();
    EXPECT_NE(testHookMgr, nullptr);
    POWER_HILOGI(LABEL_TEST, "PowerHookMgrTest001 function end!");
}
} // namespace Test
} // namespace PowerMgr
} // namespace OHOS