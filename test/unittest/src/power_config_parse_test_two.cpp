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
#include "power_mode_policy.h"
#include "power_log.h"
#include "setting_helper.h"
#include "shutdown_dialog.h"
#include "vibrator_source_parser.h"
#include "wakeup_controller.h"
#include "wakeup_source_parser.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

class PowerConfigParseTestTwo : public Test {
public:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

CJSON_PUBLIC(cJSON *) cJSON_Parse(const char *value)
{
    return nullptr;
}

namespace {
HWTEST_F(PowerConfigParseTestTwo, PowerConfigParseTestTwo001, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTestTwo001 function start!");
    const std::string json = R"("this is a string")";
    VibratorSourceParser vibratorSourceParser;
    auto parseSources = vibratorSourceParser.ParseSources(json);
    EXPECT_EQ(parseSources.size(), 0);
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTestTwo001 function end!");
}
} // namespace
