/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "power_mgr_dump_test.h"

#include <csignal>
#include <iostream>

#define private public
#define protected public
#include "power_mgr_service.h"
#undef private
#undef protected
#include "power_mgr_dumper.h"
#include "key_event.h"
#include "pointer_event.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

namespace {
auto g_pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
}

void PowerMgrDumpTest::SetUpTestCase()
{
    g_pmsTest->isBootCompleted_ = true;
}

void PowerMgrDumpTest::TearDownTestCase()
{
}

void PowerMgrDumpTest::SetUp(void)
{
}

void PowerMgrDumpTest::TearDown(void)
{
}

namespace {
/**
 * @tc.name: PowerMgrDumpNative001
 * @tc.desc: Test that args in PowerMgrDump is -i.
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrDumpTest, PowerMgrDumpNative001, TestSize.Level0)
{
    EXPECT_TRUE(g_pmsTest != nullptr) << "PowerMgrDumpNative001 fail to get PowerMgrService";
    int32_t fd = 1;
    std::vector<std::u16string> args;
    std::u16string arg = u"-i";
    args.push_back(arg);
    EXPECT_TRUE(g_pmsTest->Dump(fd, args) == ERR_OK);
}

/**
 * @tc.name: PowerMgrDumpNative002
 * @tc.desc: Test that args in PowerMgrDump is -k.
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrDumpTest, PowerMgrDumpNative002, TestSize.Level0)
{
    EXPECT_TRUE(g_pmsTest != nullptr) << "PowerMgrDumpNative002 fail to get PowerMgrService";
    int32_t fd = 1;
    std::vector<std::u16string> args;
    std::u16string arg = u"-k";
    args.push_back(arg);
    EXPECT_TRUE(g_pmsTest->Dump(fd, args) == ERR_OK);
}

/**
 * @tc.name: PowerMgrDumpNative003
 * @tc.desc: Test that args in PowerMgrDump is -r, -s, -a.
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrDumpTest, PowerMgrDumpNative003, TestSize.Level0)
{
    EXPECT_TRUE(g_pmsTest != nullptr) << "PowerMgrDumpNative003 fail to get PowerMgrService";
    int32_t fd = 1;
    std::vector<std::u16string> args;
    std::u16string arg = u"-r";
    std::u16string arg1 = u"-s";
    std::u16string arg2 = u"-a";
    args.push_back(arg);
    args.push_back(arg1);
    args.push_back(arg2);
    EXPECT_TRUE(g_pmsTest->Dump(fd, args) == ERR_OK);
}

/**
 * @tc.name: PowerMgrDumpNative004
 * @tc.desc: Test that args in PowerMgrDump is -h.
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrDumpTest, PowerMgrDumpNative004, TestSize.Level0)
{
    EXPECT_TRUE(g_pmsTest != nullptr) << "PowerMgrDumpNative004 fail to get PowerMgrService";
    int32_t fd = 1;
    std::vector<std::u16string> args;
    std::u16string arg = u"-h";
    args.push_back(arg);
    EXPECT_TRUE(g_pmsTest->Dump(fd, args) == ERR_OK);
}
} // namespace
