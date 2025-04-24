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

#include "interface_loader_test.h"
#include "interface_loader.h"
#include "power_log.h"

namespace OHOS {
namespace PowerMgr {
using namespace testing;
using namespace testing::ext;

namespace {
    const std::string TEST_LIB_PATH = "libpower_ability.z.so";
    const std::vector<std::string> TEST_LIB_SYMBOLS = {
        "PowerConnectAbility",
        "PowerStartAbility",
        "UnExistedSymbolPowerAbc",
        "PowerGetForegroundApplications",
        "PowerIsForegroundApplication",
        "UnExistedSymbolPowerXyz",
    };
    InterfaceLoader g_loader(TEST_LIB_PATH, TEST_LIB_SYMBOLS);
}

void InterfaceLoaderTest::SetUpTestCase()
{
    bool res = g_loader.Init();
    EXPECT_TRUE(res);
}

void InterfaceLoaderTest::TearDownTestCase()
{
    g_loader.DeInit();
}

/**
 * @tc.name: InterfaceLoaderTest001
 * @tc.desc: test QueryInterface, symbol exists
 * @tc.type: FUNC
 */
HWTEST_F(InterfaceLoaderTest, InterfaceLoaderTest001, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "InterfaceLoaderTest001 start!");
    void* connectAbilityFunc = g_loader.QueryInterface("PowerConnectAbility");
    EXPECT_NE(connectAbilityFunc, nullptr);
    void* isForegroundAppFunc = g_loader.QueryInterface("PowerIsForegroundApplication");
    EXPECT_NE(isForegroundAppFunc, nullptr);
    POWER_HILOGI(LABEL_TEST, "InterfaceLoaderTest001 end!");
}

/**
 * @tc.name: InterfaceLoaderTest002
 * @tc.desc: test QueryInterface, symbol don't exist
 * @tc.type: FUNC
 */
HWTEST_F(InterfaceLoaderTest, InterfaceLoaderTest002, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "InterfaceLoaderTest002 start!");
    void* powerAbcFunc = g_loader.QueryInterface("UnExistedSymbolPowerAbc");
    EXPECT_EQ(powerAbcFunc, nullptr);
    void* powerXyzFunc = g_loader.QueryInterface("UnExistedSymbolPowerXyz");
    EXPECT_EQ(powerXyzFunc, nullptr);
    POWER_HILOGI(LABEL_TEST, "InterfaceLoaderTest002 end!");
}
} // PowerMgr
} // OHOS