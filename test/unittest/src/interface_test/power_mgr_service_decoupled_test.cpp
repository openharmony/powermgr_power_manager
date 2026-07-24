/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "power_mgr_service.h"
#include "power_mgr_client.h"
#include "power_errors.h"
#include "permission.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

namespace {
// Permission switches: link-time symbol override for Permission::IsSystem/IsPermissionGranted.
bool g_isSystem = true;
bool g_isPermissionGranted = true;
} // namespace

namespace OHOS::PowerMgr {
bool Permission::IsSystem()
{
    return g_isSystem;
}
bool Permission::IsPermissionGranted(const std::string& perm)
{
    return g_isPermissionGranted;
}
} // namespace OHOS::PowerMgr

// Standalone fixture (does not reuse PowerMgrServiceTest, to avoid duplicate SetUp/SetUpTestCase).
// The decoupling 801 tests only need a PowerMgrService instance and the permission switches.
class PowerMgrServiceDecoupledTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
        pms->OnStart();
    }
    static void TearDownTestCase() {}
    void SetUp()
    {
        // Reset permission switches before each case to avoid cross-contamination.
        g_isSystem = true;
        g_isPermissionGranted = true;
    }
    void TearDown()
    {
        g_isSystem = true;
        g_isPermissionGranted = true;
    }
};

/**
 * @tc.name: PowerMgrServiceDecoupled001
 * @tc.desc: test OverrideScreenOffTime permission check and 801 on decoupling
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceDecoupledTest, PowerMgrServiceDecoupled001, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceDecoupled001 function start!");
    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    PowerErrors ret;

    // Test case 1: not a system app -> ERR_SYSTEM_API_DENIED (202)
    g_isSystem = false;
    g_isPermissionGranted = true;
    ret = pmsTest_->OverrideScreenOffTime(10000, "26");
    EXPECT_EQ(ret, PowerErrors::ERR_SYSTEM_API_DENIED) << "Test case 1 failed";

    // Test case 2: system app without POWER_MANAGER permission -> ERR_PERMISSION_DENIED (201)
    g_isSystem = true;
    g_isPermissionGranted = false;
    ret = pmsTest_->OverrideScreenOffTime(10000, "26");
    EXPECT_EQ(ret, PowerErrors::ERR_PERMISSION_DENIED) << "Test case 2 failed";

    // Test case 3: with permission. Returns 801 on decoupling; runs normally on non-decoupled.
    // The case always compiles (OverrideScreenOffTime always exists); the assertion flips with
    // the feature so it passes whether or not the image enables decoupling.
    g_isPermissionGranted = true;
    ret = pmsTest_->OverrideScreenOffTime(10000, "26");
#ifdef POWER_MANAGER_ENABLE_SCREEN_DECOUPLING
    EXPECT_EQ(ret, PowerErrors::ERR_CAPABILITY_NOT_SUPPORTED) << "Test case 3 failed";
#else
    EXPECT_NE(ret, PowerErrors::ERR_CAPABILITY_NOT_SUPPORTED) << "Test case 3 failed (non-decoupled)";
#endif

    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceDecoupled001 function end!");
}

/**
 * @tc.name: PowerMgrServiceDecoupled002
 * @tc.desc: test RestoreScreenOffTime permission check and 801 on decoupling
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceDecoupledTest, PowerMgrServiceDecoupled002, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceDecoupled002 function start!");
    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    PowerErrors ret;

    g_isSystem = false;
    g_isPermissionGranted = true;
    ret = pmsTest_->RestoreScreenOffTime("26");
    EXPECT_EQ(ret, PowerErrors::ERR_SYSTEM_API_DENIED) << "Test case 1 failed";

    g_isSystem = true;
    g_isPermissionGranted = false;
    ret = pmsTest_->RestoreScreenOffTime("26");
    EXPECT_EQ(ret, PowerErrors::ERR_PERMISSION_DENIED) << "Test case 2 failed";

    g_isPermissionGranted = true;
    ret = pmsTest_->RestoreScreenOffTime("26");
#ifdef POWER_MANAGER_ENABLE_SCREEN_DECOUPLING
    EXPECT_EQ(ret, PowerErrors::ERR_CAPABILITY_NOT_SUPPORTED) << "Test case 3 failed";
#else
    EXPECT_NE(ret, PowerErrors::ERR_CAPABILITY_NOT_SUPPORTED) << "Test case 3 failed (non-decoupled)";
#endif

    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceDecoupled002 function end!");
}

/**
 * @tc.name: PowerMgrServiceDecoupled003
 * @tc.desc: test RefreshActivity (PowerErrors overload) returns 801 on decoupling
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceDecoupledTest, PowerMgrServiceDecoupled003, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceDecoupled003 function start!");
    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    // NOLINTNEXTLINE(WordsTool.6) The underlying interface has a fixed naming convention UserActivity
    PowerErrors ret = pmsTest_->RefreshActivity(0, UserActivityType::USER_ACTIVITY_TYPE_OTHER, std::string("reason"));
#ifdef POWER_MANAGER_ENABLE_SCREEN_DECOUPLING
    EXPECT_EQ(ret, PowerErrors::ERR_CAPABILITY_NOT_SUPPORTED);
#else
    EXPECT_NE(ret, PowerErrors::ERR_CAPABILITY_NOT_SUPPORTED);
#endif
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceDecoupled003 function end!");
}

/**
 * @tc.name: PowerMgrServiceDecoupled004
 * @tc.desc: test RefreshActivity (bool overload) returns false on decoupling
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceDecoupledTest, PowerMgrServiceDecoupled004, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceDecoupled004 function start!");
    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    // NOLINTNEXTLINE(WordsTool.6) The underlying interface has a fixed naming convention UserActivity
    bool ret = pmsTest_->RefreshActivity(0, UserActivityType::USER_ACTIVITY_TYPE_OTHER, true);
#ifdef POWER_MANAGER_ENABLE_SCREEN_DECOUPLING
    EXPECT_FALSE(ret);
#else
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceDecoupled004 non-decoupled, ret=%{public}d", ret);
#endif
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceDecoupled004 function end!");
}
