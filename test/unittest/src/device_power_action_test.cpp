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

#include "device_power_action.h"

#include "init_reboot.h"
#include "list.h"
#include "power_hookmgr.h"
#include "power_log.h"
#include <gtest/gtest.h>
#include <string>

using namespace testing;
using namespace ext;
using namespace OHOS;
using namespace PowerMgr;
namespace {
int g_retval = 0;
bool g_invoked = false;
std::string g_cmdResult;
} // namespace

int DoRebootExt(const char* mode, const char* option)
{
    g_cmdResult = mode;
    return 0;
}
int HookMgrExecute(HOOK_MGR* hookMgr, int stage, void* context, const HOOK_EXEC_OPTIONS* options)
{
    return g_retval;
}

class DevicePowerActionTest : public DevicePowerAction, public Test {};

namespace {
HWTEST_F(DevicePowerActionTest, DevicePowerActionTest001, TestSize.Level0)
{
    g_cmdResult.clear();
    g_retval = 0;
    g_invoked = false;
    Reboot("");
    EXPECT_TRUE(g_invoked);
    EXPECT_EQ(g_cmdResult, "invalid_cmd");
}

HWTEST_F(DevicePowerActionTest, DevicePowerActionTest002, TestSize.Level0)
{
    g_cmdResult.clear();
    g_retval = -1;
    g_invoked = false;
    Reboot("");
    EXPECT_TRUE(g_invoked);
    EXPECT_EQ(g_cmdResult, "");
}

HWTEST_F(DevicePowerActionTest, DevicePowerActionTest003, TestSize.Level0)
{
    g_cmdResult.clear();
    g_retval = -1;
    g_invoked = false;
    Reboot("updater");
    EXPECT_TRUE(g_invoked);
    EXPECT_EQ(g_cmdResult, "updater");
}

HWTEST_F(DevicePowerActionTest, DevicePowerActionTest004, TestSize.Level0)
{
    g_cmdResult.clear();
    g_retval = -1;
    g_invoked = false;
    Shutdown("");
    EXPECT_TRUE(g_invoked);
    EXPECT_EQ(g_cmdResult, "shutdown");
}
} // namespace