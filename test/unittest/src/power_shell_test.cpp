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
#include <parameters.h>
#include <power_log.h>
#include <power_mgr_client.h>
#include <power_shell_command.h>

namespace OHOS {
namespace system {
bool GetBoolParameter(const std::string&, bool)
{
    return true;
}
} // namespace system

namespace PowerMgr {
using namespace testing::ext;
namespace {
std::string g_lastCalled {};
}

PowerErrors PowerMgrClient::LockScreenAfterTimingOut(bool, bool, bool)
{
    g_lastCalled = "LockScreenAfterTimingOut";
    return PowerErrors::ERR_OK;
}

PowerErrors PowerMgrClient::LockScreenAfterTimingOutWithAppid(pid_t, bool)
{
    g_lastCalled = "LockScreenAfterTimingOutWithAppid";
    return PowerErrors::ERR_OK;
}

class PowerShellTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown()
    {
        g_lastCalled.clear();
    }
};

namespace {
HWTEST_F(PowerShellTest, PowerShellTest001, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerShellTest001 start");
    char name[] = "power_shell_test";
    char command[] = "timeoutscreenlock";
    char paramAppid[] = "11";
    char paramBool[] = "1";
    char* argvTwoParamsWithId[] = {name, command, paramAppid, paramBool};
    char* argvTwoParamsWithoutId[] = {name, command, paramBool, paramBool};
    char* argvThreeParamsWithId[] = {name, command, paramAppid, paramBool, paramBool};
    char* argvThreeParamWithoutId[] = {name, command, paramBool, paramBool, paramBool};

    PowerShellCommand cmd1{sizeof(argvTwoParamsWithId) / sizeof(char*), argvTwoParamsWithId};
    cmd1.ExecCommand();
    EXPECT_EQ(g_lastCalled, "LockScreenAfterTimingOutWithAppid");
    g_lastCalled.clear();

    PowerShellCommand cmd2{sizeof(argvTwoParamsWithoutId) / sizeof(char*), argvTwoParamsWithoutId};
    cmd2.ExecCommand();
    EXPECT_EQ(g_lastCalled, "LockScreenAfterTimingOut");
    g_lastCalled.clear();

    PowerShellCommand cmd3{sizeof(argvThreeParamsWithId) / sizeof(char*), argvThreeParamsWithId};
    cmd3.ExecCommand();
    EXPECT_EQ(g_lastCalled, "LockScreenAfterTimingOut");
    g_lastCalled.clear();

    PowerShellCommand cmd4{sizeof(argvThreeParamWithoutId) / sizeof(char*), argvThreeParamWithoutId};
    cmd4.ExecCommand();
    EXPECT_EQ(g_lastCalled, "LockScreenAfterTimingOut");
    g_lastCalled.clear();

    POWER_HILOGI(LABEL_TEST, "PowerShellTest001 end");
}
} // namespace
} // namespace PowerMgr
} // namespace OHOS