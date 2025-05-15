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
#include <cJSON.h>
#include <gtest/gtest.h>
#include "power_mgr_service.h"
#include "power_log.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

class PowerConfigParseTest : public Test {
public:
    void SetUp() override
    {
        root_ = cJSON_CreateObject();
    }

    void TearDown() override
    {
        cJSON_Delete(root_);
    }

    cJSON* root_;
};

namespace {
constexpr size_t NUMBER_ONE = 1;
constexpr size_t NUMBER_TWO = 2;

HWTEST_F(PowerConfigParseTest, PowerConfigParseTest001, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest001 function start!");
    cJSON_AddItemToObject(root_, "action", cJSON_CreateNumber(NUMBER_ONE));
    cJSON_AddItemToObject(root_, "delayMs", cJSON_CreateNumber(NUMBER_TWO));

    std::string key = "lid";
    std::shared_ptr<SuspendSources> parseSources = std::make_shared<SuspendSources>();
    bool result = SuspendSourceParser::ParseSourcesProc(parseSources, root_, key);

    EXPECT_TRUE(result);
    std::vector<SuspendSource> sources = parseSources->GetSourceList();
    EXPECT_EQ(sources.size(), NUMBER_ONE);
    EXPECT_EQ(sources[0].reason_, SuspendDeviceType::SUSPEND_DEVICE_REASON_LID);
    EXPECT_EQ(sources[0].action_, NUMBER_ONE);
    EXPECT_EQ(sources[0].delayMs_, NUMBER_TWO);
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest001 function end!");
}

HWTEST_F(PowerConfigParseTest, PowerConfigParseTest002, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest002 function start!");
    cJSON* valueObj = cJSON_CreateNumber(NUMBER_TWO);
    cJSON_AddItemToObject(root_, "action", valueObj);
    std::string key = "lid";
    std::shared_ptr<SuspendSources> parseSources = std::make_shared<SuspendSources>();
    bool result = SuspendSourceParser::ParseSourcesProc(parseSources, root_, key);
    EXPECT_TRUE(result);
    std::vector<SuspendSource> sources = parseSources->GetSourceList();
    EXPECT_EQ(sources.size(), NUMBER_ONE);
    EXPECT_EQ(sources[0].reason_, SuspendDeviceType::SUSPEND_DEVICE_REASON_LID);
    EXPECT_EQ(sources[0].action_, 0);
    EXPECT_EQ(sources[0].delayMs_, 0);
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest002 function end!");
}

HWTEST_F(PowerConfigParseTest, PowerConfigParseTest003, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest003 function start!");
    std::string key = "lid";
    std::shared_ptr<SuspendSources> parseSources = std::make_shared<SuspendSources>();
    bool result = SuspendSourceParser::ParseSourcesProc(parseSources, nullptr, key);
    EXPECT_TRUE(result);
    std::vector<SuspendSource> sources = parseSources->GetSourceList();
    EXPECT_EQ(sources.size(), NUMBER_ONE);
    EXPECT_EQ(sources[0].reason_, SuspendDeviceType::SUSPEND_DEVICE_REASON_LID);
    EXPECT_EQ(sources[0].action_, 0);
    EXPECT_EQ(sources[0].delayMs_, 0);
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest003 function end!");
}

HWTEST_F(PowerConfigParseTest, PowerConfigParseTest004, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest004 function start!");
    cJSON_AddItemToObject(root_, "action", cJSON_CreateNumber(NUMBER_ONE));

    std::string key = "lid";
    std::shared_ptr<SuspendSources> parseSources = std::make_shared<SuspendSources>();
    bool result = SuspendSourceParser::ParseSourcesProc(parseSources, root_, key);

    EXPECT_TRUE(result);
    std::vector<SuspendSource> sources = parseSources->GetSourceList();
    EXPECT_EQ(sources.size(), NUMBER_ONE);
    EXPECT_EQ(sources[0].reason_, SuspendDeviceType::SUSPEND_DEVICE_REASON_LID);
    EXPECT_EQ(sources[0].action_, 0);
    EXPECT_EQ(sources[0].delayMs_, 0);
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest004 function end!");
}

HWTEST_F(PowerConfigParseTest, PowerConfigParseTest005, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest005 function start!");
    cJSON_AddItemToObject(root_, "delayMs", cJSON_CreateNumber(NUMBER_TWO));
    
    std::string key = "lid";
    std::shared_ptr<SuspendSources> parseSources = std::make_shared<SuspendSources>();
    bool result = SuspendSourceParser::ParseSourcesProc(parseSources, root_, key);

    EXPECT_TRUE(result);
    std::vector<SuspendSource> sources = parseSources->GetSourceList();
    EXPECT_EQ(sources.size(), NUMBER_ONE);
    EXPECT_EQ(sources[0].reason_, SuspendDeviceType::SUSPEND_DEVICE_REASON_LID);
    EXPECT_EQ(sources[0].action_, 0);
    EXPECT_EQ(sources[0].delayMs_, 0);
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest005 function end!");
}

HWTEST_F(PowerConfigParseTest, PowerConfigParseTest006, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest006 function start!");
    const std::string json = R"({"lid": {"action": 1, "delayMs": 2}})";
    auto parseSources = SuspendSourceParser::ParseSources(json);
    std::vector<SuspendSource> sources = parseSources->GetSourceList();
    EXPECT_EQ(sources.size(), NUMBER_ONE);
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest006 function end!");
}


HWTEST_F(PowerConfigParseTest, PowerConfigParseTest007, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest007 function start!");
    auto parseSources = SuspendSourceParser::ParseSources("");
    std::vector<SuspendSource> sources = parseSources->GetSourceList();
    EXPECT_EQ(sources.size(), 0);
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest007 function end!");
}

HWTEST_F(PowerConfigParseTest, PowerConfigParseTest008, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest008 function start!");
    auto parseSources = SuspendSourceParser::ParseSources("[]");
    std::vector<SuspendSource> sources = parseSources->GetSourceList();
    EXPECT_EQ(sources.size(), 0);
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest008 function end!");
}
} // namespace
