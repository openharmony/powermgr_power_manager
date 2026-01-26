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
#include "power_setting_helper_test.h"

#include "power_test_utils.h"
#include "power_log.h"
#define private public
#include "setting_helper.h"
#undef private

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
    return true;
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

static void PresetData(const std::string& testName)
{
    static std::map<std::string, std::function<void()>> caseFuncMap {
        {"PowerSettingHelperTest002", []() {
            DataFactory<tuple<std::string, int>>::Reset();
            DataFactory<tuple<std::string, int>>::Produce({"pass", ERR_OK}); // case 0
            DataFactory<tuple<std::string, int>>::Produce({"pass", ERR_INVALID_OPERATION}, 5); // case 1
            DataFactory<tuple<std::string, int>>::Produce({"pass", ERR_OK}); // case 1
            DataFactory<tuple<std::string, int>>::Produce({"fail", ERR_INVALID_OPERATION}, 21); // case 2
            DataFactory<tuple<std::string, int>>::Produce({"not_found_key", ERR_NAME_NOT_FOUND}); // case 3
        }},
        {"PowerSettingHelperTest003", []() {
            DataFactory<tuple<std::string, int>>::Reset();
            DataFactory<tuple<std::string, int>>::Produce({"601", ERR_OK}); // case 0
            DataFactory<tuple<std::string, int>>::Produce({"600", ERR_INVALID_OPERATION}, 5); // case 1
            DataFactory<tuple<std::string, int>>::Produce({"600", ERR_OK}); // case 1
            DataFactory<tuple<std::string, int>>::Produce({"0", ERR_INVALID_OPERATION}, 21); // case 2
            DataFactory<tuple<std::string, int>>::Produce({"0", ERR_NAME_NOT_FOUND}); // case 3
        }},
    };
    auto it = caseFuncMap.find(testName);
    if (it != caseFuncMap.end() && it->second) {
        it->second();
    }
}

ErrCode SettingProvider::GetStringValue(const std::string& key, std::string& value)
{
    const auto& data = DataFactory<tuple<std::string, int>>::Consume();
    value = std::get<0>(data);
    return std::get<1>(data);
}

void PowerSettingHelperTest::SetUp()
{
    const ::testing::UnitTest* unitTest = ::testing::UnitTest::GetInstance();
    const ::testing::TestInfo* testInfo = unitTest->current_test_info();
    const std::string testCaseName = testInfo->test_case_name();
    const std::string testName = testInfo->name();
    PresetData(testName);
}

void PowerSettingHelperTest::TearDown()
{
}
} // namespace OHOS::PowerMgr

namespace {
/**
 * @tc.name: PowerSettingHelperTest001
 * @tc.desc: test SettingHelper GetSettingDisplayOffTime
 * @tc.type: FUNC
 * @tc.require: issue1579
 */
#ifndef POWER_MANAGER_ENABLE_CHARGING_TYPE_SETTING
HWTEST_F(PowerSettingHelperTest, PowerSettingHelperTest001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerSettingHelperTest001 function start!");
    GTEST_LOG_(INFO) << "PowerSettingHelperTest001: start";
    int64_t defaultVal = 30000;
    g_getSettingLongValue = SLEEP_WAIT_TIME_S;
    int64_t value = SettingHelper::GetSettingDisplayOffTime(defaultVal);
#ifdef POWER_MANAGER_DISABLE_AUTO_DISPLAYOFF
    EXPECT_EQ(SLEEP_WAIT_TIME_S, value);
#else
    EXPECT_EQ(defaultVal, value);
#endif
    g_getSettingLongValue = 2;
    value = SettingHelper::GetSettingDisplayOffTime(defaultVal);
    EXPECT_EQ(defaultVal, value);
    g_getSettingLongValue = 2000;
    value = SettingHelper::GetSettingDisplayOffTime(defaultVal);
    EXPECT_EQ(g_getSettingLongValue, value);
    POWER_HILOGI(LABEL_TEST, "PowerSettingHelperTest001 function end!");
    GTEST_LOG_(INFO) << "PowerSettingHelperTest001: end";
}
#endif

/**
 * @tc.name: PowerSettingHelperTest002
 * @tc.desc: test SettingHelper GetSettingStringValueWithRetry
 * @tc.type: FUNC
 */
HWTEST_F(PowerSettingHelperTest, PowerSettingHelperTest002, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerSettingHelperTest002 function start!");
    GTEST_LOG_(INFO) << "PowerSettingHelperTest002: start";
    std::string getKey = "SETTING_POWER_MODE_BACKUP_KEY";
    std::string getValue = "";
    // case 0: return ERR_OK without retry
    std::string result = SettingHelper::GetSettingStringValueWithRetry(getKey, getValue);
    EXPECT_TRUE(result == "pass");
    // case 1: return ERR_OK after 5 retries
    result = SettingHelper::GetSettingStringValueWithRetry(getKey, getValue);
    EXPECT_TRUE(result == "pass");
    // case 2: return ERR_INVALID_OPERATION after 20 retries, get value fail
    result = SettingHelper::GetSettingStringValueWithRetry(getKey, getValue);
    EXPECT_TRUE(result == "fail");
    // case 3: return ERR_NAME_NOT_FOUND because not found key
    result = SettingHelper::GetSettingStringValueWithRetry(getKey, getValue);
    EXPECT_TRUE(result == "not_found_key");
    POWER_HILOGI(LABEL_TEST, "PowerSettingHelperTest002 function end!");
    GTEST_LOG_(INFO) << "PowerSettingHelperTest002: end";
}

/**
 * @tc.name: PowerSettingHelperTest003
 * @tc.desc: test SettingHelper GetSettingIntValueWithRetry
 * @tc.type: FUNC
 */
HWTEST_F(PowerSettingHelperTest, PowerSettingHelperTest003, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerSettingHelperTest003 function start!");
    GTEST_LOG_(INFO) << "PowerSettingHelperTest003: start";
    std::string getKey = "SETTING_POWER_MODE_KEY";
    int32_t getValue = 0;
    // case 0: return ERR_OK without retry
    int32_t result = SettingHelper::GetSettingIntValueWithRetry(getKey, getValue);
    EXPECT_TRUE(result == 601);
    // case 1: return ERR_OK after 5 retries
    result = SettingHelper::GetSettingIntValueWithRetry(getKey, getValue);
    EXPECT_TRUE(result == 600);
    // case 2: return ERR_INVALID_OPERATION after 20 retries, get value fail
    result = SettingHelper::GetSettingIntValueWithRetry(getKey, getValue);
    EXPECT_TRUE(result == 0);
    // case 3: return ERR_NAME_NOT_FOUND because not found key
    result = SettingHelper::GetSettingIntValueWithRetry(getKey, getValue);
    EXPECT_TRUE(result == 0);
    POWER_HILOGI(LABEL_TEST, "PowerSettingHelperTest003 function end!");
    GTEST_LOG_(INFO) << "PowerSettingHelperTest003: end";
}
} // namespace