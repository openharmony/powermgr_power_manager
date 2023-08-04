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
#include "power_parsesources_mock_test.h"
#include <fstream>
#include <thread>
#include <unistd.h>

#include <datetime_ex.h>
#include <input_manager.h>
#include <securec.h>

#include "power_mgr_service.h"
#include "power_state_callback_stub.h"
#include "power_state_machine.h"
#include "setting_helper.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;
static sptr<PowerMgrService> g_service;

bool SettingHelper::IsSuspendSourcesSettingValid()
{
    return true;
}

bool SuspendSourceParser::ParseSourcesProc(
    std::shared_ptr<SuspendSources>& parseSources, Json::Value& valueObj, std::string& key)
{
    return false;
}

bool WakeupSourceParser::ParseSourcesProc(
    std::shared_ptr<WakeupSources>& parseSources, Json::Value& valueObj, std::string& key)
{
    return false;
}

namespace {
/**
 * @tc.name: PowerParseSourceMockTest
 * @tc.desc: test ParseSources(exception)
 * @tc.type: FUNC
 * @tc.require: issueI7G6OY
 */
HWTEST_F(PowerParseSourceMockTest, PowerParseSourceMockTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerParseSourceMockTest001: start";

    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pmsTest_ == nullptr) {
        GTEST_LOG_(INFO) << "PowerParseSourceMockTest001: Failed to get PowerMgrService";
    }

    std::shared_ptr<SuspendSources> sources1 = SuspendSourceParser::ParseSources();
    EXPECT_TRUE(SettingHelper::IsSuspendSourcesSettingValid() == true);

    std::shared_ptr<WakeupSources> sources2 = WakeupSourceParser::ParseSources();
    EXPECT_TRUE(SettingHelper::IsSuspendSourcesSettingValid() == true);

    std::vector<std::string> tmp;
    static const std::string jsonStr = "{\"powerkey\":";
    std::shared_ptr<SuspendSources> sources3 = SuspendSourceParser::ParseSources(jsonStr);
    tmp = sources3->getSourceKeys();
    EXPECT_TRUE(tmp.size() != 0);
    std::shared_ptr<WakeupSources> sources4 = WakeupSourceParser::ParseSources(jsonStr);
    tmp = sources4->getSourceKeys();
    EXPECT_TRUE(tmp.size() != 0);

    static const std::string jsonStr2 =
        "{\"powerkey\": {\"enable\": false},\"keyborad\": {\"enable\": false},\"mouse\": {\"enable\": "
        "false},\"touchscreen\": {\"enable\": false,\"click\": 2},\"touchpad\": {\"enable\": false},\"pen\": "
        "{\"enable\": false},\"lid\": {\"enable\": false},\"switch\": {\"enable\": false}}";
    std::shared_ptr<WakeupSources> sources5 = WakeupSourceParser::ParseSources(jsonStr2);
    tmp = sources5->getSourceKeys();
    EXPECT_TRUE(tmp.size() != 0);

    static const std::string jsonStr3 =
        "{ \"powerkey\": { \"action\": 0, \"delayMs\": 1002 }, \"timeout\": { \"action\": 1, \"delayMs\": 2002 }, "
        "\"lid\": { \"action\": 1, \"delayMs\": 3002 }, \"switch\": { \"action\": 1, \"delayMs\": 4002 } }";
    std::shared_ptr<SuspendSources> sources6 = SuspendSourceParser::ParseSources(jsonStr3);
    tmp = sources6->getSourceKeys();
    EXPECT_TRUE(tmp.size() != 0);

    GTEST_LOG_(INFO) << "PowerParseSourceMockTest001:  end";
}
} // namespace