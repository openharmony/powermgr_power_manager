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
#include "power_setting_help_test.h"

#include "power_log.h"
#include "setting_helper.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

namespace {
#ifndef POWER_MANAGER_ENABLE_CHARGING_TYPE_SETTING
constexpr int SLEEP_WAIT_TIME_S = -1;
static int32_t g_getSettingLongValue = 0;
#endif
} // namespace

namespace OHOS {
namespace system {
#ifndef POWER_MANAGER_ENABLE_CHARGING_TYPE_SETTING
bool GetBoolParameter(const std::string& key, bool def)
{
    return false;
}
#endif
}
}

namespace OHOS::PowerMgr {
#ifndef POWER_MANAGER_ENABLE_CHARGING_TYPE_SETTING
int64_t SettingHelper::GetSettingLongValue(const std::string& key, int64_t defaultVal)
{
    return g_getSettingLongValue;
}
#endif
} // namespace OHOS::PowerMgr

namespace {
/**
 * @tc.name: PowerSettingHelpTest001
 * @tc.desc: test SettingHelper GetSettingDisplayOffTime
 * @tc.type: FUNC
 * @tc.require: issue1579
 */
#ifndef POWER_MANAGER_ENABLE_CHARGING_TYPE_SETTING
HWTEST_F(PowerSettingHelpTest, PowerSettingHelpTest001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerSettingHelpTest001 function start!");
    GTEST_LOG_(INFO) << "PowerSettingHelpTest001: start";
    int64_t defaultVal = 30000;
    g_getSettingLongValue = SLEEP_WAIT_TIME_S;
    int64_t value = SettingHelper::GetSettingDisplayOffTime(defaultVal);
    EXPECT_EQ(defaultVal, value);
    g_getSettingLongValue = 2;
    value = SettingHelper::GetSettingDisplayOffTime(defaultVal);
    EXPECT_EQ(defaultVal, value);
    g_getSettingLongValue = 2000;
    value = SettingHelper::GetSettingDisplayOffTime(defaultVal);
    EXPECT_EQ(g_getSettingLongValue, value);
    POWER_HILOGI(LABEL_TEST, "PowerSettingHelpTest001 function end!");
    GTEST_LOG_(INFO) << "PowerSettingHelpTest001: end";
}
#endif
} // namespace