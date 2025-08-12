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

#include "ohos.runningLock.proj.hpp"
#include "ohos.runningLock.impl.hpp"
#include "ohos.runningLock.user.hpp"
#include "taihe/runtime.hpp"
#include "power_mgr_errors.h"
#include "power_mgr_client.h"
#include "power_errors.h"
#include "power_log.h"

using namespace taihe;
using namespace ohos::runningLock;
using namespace OHOS::PowerMgr;

using namespace testing;
using namespace testing::ext;

namespace {
PowerErrors g_error = PowerErrors::ERR_OK;
std::shared_ptr<OHOS::PowerMgr::RunningLock> g_runningLock {nullptr};
bool g_pass = false;
int g_lockReturn = OHOS::ERR_OK;
const int32_t TIMEOUT = -1;
}

namespace taihe {
void set_business_error(int32_t err_code, taihe::string_view msg)
{
    (void)err_code;
    (void)msg;
}
}

namespace OHOS::PowerMgr {
std::shared_ptr<RunningLock> PowerMgrClient::CreateRunningLock(const std::string& name, RunningLockType type)
{
    g_pass = true;
    return g_runningLock;
}

PowerErrors PowerMgrClient::GetError()
{
    return g_error;
}

ErrCode RunningLock::Lock(int32_t timeOutMs)
{
    return g_lockReturn;
}

ErrCode RunningLock::UnLock()
{
    return g_lockReturn;
}

bool RunningLock::IsUsed()
{
    if (g_lockReturn == OHOS::PowerMgr::E_PERMISSION_DENIED) {
        return false;
    }
    return true;
}
} // namespace OHOS::PowerMgr

namespace {
class RunningLockTaiheNativeTest : public ::testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown()
    {
        g_error = PowerErrors::ERR_OK;
        g_runningLock = nullptr;
        g_pass = false;
    }
};

/**
 * @tc.name: RunningLockTaiheNativeTest_001
 * @tc.desc: test running lock taihe native
 * @tc.type: FUNC
 * @tc.require: issue#ICAK9Z
 */
HWTEST_F(RunningLockTaiheNativeTest, RunningLockTaiheNativeTest_001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockTaiheNativeTest_001 start");
    const string_view name = "RunningLockTaiheNativeTest_001";
    const ohos::runningLock::RunningLockType type(
        ohos::runningLock::RunningLockType::key_t::PROXIMITY_SCREEN_CONTROL);
    ohos::runningLock::RunningLock lock = CreateSync(name, type);
    EXPECT_FALSE(lock.is_error());
    lock->Hold(TIMEOUT);
    bool isHolding = lock->IsHolding();
    EXPECT_FALSE(isHolding);
    lock->Unhold();

    g_error = PowerErrors::ERR_FAILURE;
    CreateSync(name, type);

    g_error = PowerErrors::ERR_PERMISSION_DENIED;
    CreateSync(name, type);
    EXPECT_TRUE(g_pass);
    POWER_HILOGI(LABEL_TEST, "RunningLockTaiheNativeTest_001 end");
}

/**
 * @tc.name: RunningLockTaiheNativeTest_002
 * @tc.desc: test running lock taihe native
 * @tc.type: FUNC
 * @tc.require: issue#ICAK9Z
 */
HWTEST_F(RunningLockTaiheNativeTest, RunningLockTaiheNativeTest_002, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockTaiheNativeTest_002 start");
    const string_view name = "RunningLockTaiheNativeTest_002";
    const ohos::runningLock::RunningLockType type(
        ohos::runningLock::RunningLockType::key_t::PROXIMITY_SCREEN_CONTROL);
    g_runningLock = std::make_shared<OHOS::PowerMgr::RunningLock>(
        nullptr, std::string(name), OHOS::PowerMgr::RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL);
    ohos::runningLock::RunningLock lock = CreateSync(name, type);
    EXPECT_FALSE(lock.is_error());
    lock->Hold(TIMEOUT);
    bool isHolding = lock->IsHolding();
    EXPECT_TRUE(isHolding);
    lock->Unhold();

    g_lockReturn = E_PERMISSION_DENIED;
    lock->Hold(TIMEOUT);
    isHolding = lock->IsHolding();
    EXPECT_FALSE(isHolding);
    lock->Unhold();
    EXPECT_TRUE(g_pass);
    POWER_HILOGI(LABEL_TEST, "RunningLockTaiheNativeTest_002 end");
}

/**
 * @tc.name: RunningLockTaiheNativeTest_003
 * @tc.desc: test running lock taihe native
 * @tc.type: FUNC
 * @tc.require: issue#ICAK9Z
 */
HWTEST_F(RunningLockTaiheNativeTest, RunningLockTaiheNativeTest_003, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockTaiheNativeTest_003 start");
    ohos::runningLock::RunningLockType type(ohos::runningLock::RunningLockType::key_t::PROXIMITY_SCREEN_CONTROL);
    EXPECT_TRUE(IsSupported(type));
    POWER_HILOGI(LABEL_TEST, "RunningLockTaiheNativeTest_003 end");
}
} // namespace