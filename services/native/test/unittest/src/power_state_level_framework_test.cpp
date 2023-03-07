/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "power_state_level_framework_test.h"

#include <iostream>

#include <gtest/gtest.h>
#include <string_ex.h>

#include "power_state_machine_info.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

namespace {
/**
 * @tc.name: PowerStateLevelFramework001
 * @tc.desc: test system-level sleep level enumeration
 * @tc.type: FUNC
 * @tc.require: issueI6JSKU
 */
HWTEST_F (PowerStateLevelFrameworkTest, PowerStateLevelFramework001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerStateLevelFramework001: test system-level sleep level enumeration is start";
    EXPECT_TRUE(static_cast<uint32_t>(PowerState::AWAKE) == AWAKE) << "PowerState AWAKE correct";
    EXPECT_TRUE(static_cast<uint32_t>(PowerState::FREEZE) == FREEZE) << "PowerState FREEZE correct";
    EXPECT_TRUE(static_cast<uint32_t>(PowerState::INACTIVE) == INACTIVE) << "PowerState INACTIVE correct";
    EXPECT_TRUE(static_cast<uint32_t>(PowerState::STAND_BY) == STAND_BY) << "PowerState STAND_BY correct";
    EXPECT_TRUE(static_cast<uint32_t>(PowerState::DOZE) == DOZE) << "PowerState DOZE correct";
    EXPECT_TRUE(static_cast<uint32_t>(PowerState::SLEEP) == SLEEP) << "PowerState SLEEP correct";
    EXPECT_TRUE(static_cast<uint32_t>(PowerState::HIBERNATE) == HIBERNATE) << "PowerState HIBERNATE correct";
    EXPECT_TRUE(static_cast<uint32_t>(PowerState::SHUTDOWN) == SHUTDOWN) << "PowerState HIBERNATE correct";
    GTEST_LOG_(INFO) << "PowerStateLevelFramework001: test system-level sleep level enumeration is end";
}
}