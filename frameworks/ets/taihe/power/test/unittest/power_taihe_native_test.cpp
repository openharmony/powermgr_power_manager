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

#include "ohos.power.proj.hpp"
#include "ohos.power.impl.hpp"
#include "ohos.power.user.hpp"
#include "taihe/runtime.hpp"
#include "power_mgr_client.h"
#include "power_shutdown_callback.h"
#include "power_errors.h"
#include "power_log.h"

using namespace taihe;
using namespace ohos::power;
using namespace OHOS::PowerMgr;

using namespace testing;
using namespace testing::ext;

namespace {
PowerErrors g_error = PowerErrors::ERR_OK;
bool g_pass = false;
bool g_cbFlag = false;
PowerMode g_mode = PowerMode::NORMAL_MODE;
int32_t g_errCode = 0;
}

namespace taihe {
void set_business_error(int32_t err_code, taihe::string_view msg)
{
    g_errCode = err_code;
    (void)msg;
}
}

namespace OHOS::PowerMgr {
bool PowerMgrClient::IsScreenOn(bool needPrintLog)
{
    g_pass = true;
    return false;
}

PowerErrors PowerMgrClient::WakeupDevice(
    WakeupDeviceType reason, const std::string& detail, const std::string& apiVersion)
{
    g_pass = true;
    return g_error;
}

PowerErrors PowerMgrClient::ForceSuspendDevice(const std::string& apiVersion)
{
    g_pass = true;
    return g_error;
}

PowerErrors PowerMgrClient::SuspendDevice(
    SuspendDeviceType reason, bool suspendImmed, const std::string& apiVersion)
{
    g_pass = true;
    return g_error;
}

PowerMode PowerMgrClient::GetDeviceMode()
{
    g_pass = true;
    return g_mode;
}

PowerErrors PowerMgrClient::SetDeviceMode(const PowerMode mode)
{
    g_pass = true;
    return g_error;
}

PowerErrors PowerMgrClient::IsStandby(bool& isStandby)
{
    g_pass = true;
    return g_error;
}

PowerErrors PowerMgrClient::Hibernate(bool clearMemory, const std::string& reason, const std::string& apiVersion)
{
    g_pass = true;
    return g_error;
}

PowerErrors PowerMgrClient::RestoreScreenOffTime(const std::string& apiVersion)
{
    g_pass = true;
    return g_error;
}

PowerErrors PowerMgrClient::OverrideScreenOffTime(int64_t timeout, const std::string& apiVersion)
{
    g_pass = true;
    return g_error;
}

PowerErrors PowerMgrClient::RefreshActivity(UserActivityType type, const std::string& refreshReason)
{
    g_pass = true;
    return g_error;
}

PowerErrors PowerMgrClient::RegisterAsyncShutdownCallback(
    const sptr<IAsyncShutdownCallback>& callback, ShutdownPriority priority)
{
    g_pass = true;
    return g_error;
}

PowerErrors PowerMgrClient::UnRegisterAsyncShutdownCallback(const sptr<IAsyncShutdownCallback>& callback)
{
    g_pass = true;
    return g_error;
}
} // namespace OHOS::PowerMgr

namespace {
class PowerTaiheNativeTest : public ::testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp()
    {
        EXPECT_FALSE(g_pass);
    }
    void TearDown()
    {
        g_pass = false;
        g_mode = PowerMode::NORMAL_MODE;
        g_cbFlag = false;
        g_error = PowerErrors::ERR_OK;
        g_errCode = 0;
    }
};

/**
 * @tc.name: PowerTaiheNativeTest_001
 * @tc.desc: test power taihe native
 * @tc.type: FUNC
 * @tc.require: issue#ICAK9Z
 */
HWTEST_F(PowerTaiheNativeTest, PowerTaiheNativeTest_001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerTaiheNativeTest_001 start");
    IsActive();
    EXPECT_TRUE(g_pass);
    POWER_HILOGI(LABEL_TEST, "PowerTaiheNativeTest_001 end");
}

/**
 * @tc.name: PowerTaiheNativeTest_002
 * @tc.desc: test power taihe native
 * @tc.type: FUNC
 * @tc.require: issue#ICAK9Z
 */
HWTEST_F(PowerTaiheNativeTest, PowerTaiheNativeTest_002, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerTaiheNativeTest_002 start");
    string_view detail = "PowerTaiheNativeTest_002";
    Wakeup(detail);

    g_error = PowerErrors::ERR_FAILURE;
    Wakeup(detail);
    EXPECT_TRUE(g_errCode != static_cast<int32_t>(PowerErrors::ERR_FAILURE));

    g_error = PowerErrors::ERR_SYSTEM_API_DENIED;
    Wakeup(detail);
    EXPECT_TRUE(g_errCode == static_cast<int32_t>(PowerErrors::ERR_SYSTEM_API_DENIED));
    EXPECT_TRUE(g_pass);
    POWER_HILOGI(LABEL_TEST, "PowerTaiheNativeTest_002 end");
}

/**
 * @tc.name: PowerTaiheNativeTest_003
 * @tc.desc: test power taihe native
 * @tc.type: FUNC
 * @tc.require: issue#ICAK9Z
 */
HWTEST_F(PowerTaiheNativeTest, PowerTaiheNativeTest_003, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerTaiheNativeTest_003 start");
    bool isImmediate = false;
    Suspend(&isImmediate);

    isImmediate = true;
    Suspend(&isImmediate);

    g_error = PowerErrors::ERR_FAILURE;
    Suspend(&isImmediate);
    EXPECT_TRUE(g_errCode != static_cast<int32_t>(PowerErrors::ERR_FAILURE));

    g_error = PowerErrors::ERR_SYSTEM_API_DENIED;
    Suspend(&isImmediate);
    EXPECT_TRUE(g_errCode == static_cast<int32_t>(PowerErrors::ERR_SYSTEM_API_DENIED));
    EXPECT_TRUE(g_pass);
    POWER_HILOGI(LABEL_TEST, "PowerTaiheNativeTest_003 end");
}

/**
 * @tc.name: PowerTaiheNativeTest_004
 * @tc.desc: test power taihe native
 * @tc.type: FUNC
 * @tc.require: issue#ICAK9Z
 */
HWTEST_F(PowerTaiheNativeTest, PowerTaiheNativeTest_004, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerTaiheNativeTest_004 start");
    GetPowerMode();
    g_mode = PowerMode::POWER_SAVE_MODE;
    GetPowerMode();
    g_mode = PowerMode::PERFORMANCE_MODE;
    GetPowerMode();
    g_mode = PowerMode::EXTREME_POWER_SAVE_MODE;
    GetPowerMode();
    g_mode = PowerMode::CUSTOM_POWER_SAVE_MODE;
    GetPowerMode();
    g_mode = static_cast<PowerMode>(0);
    GetPowerMode();
    EXPECT_TRUE(g_pass);
    POWER_HILOGI(LABEL_TEST, "PowerTaiheNativeTest_004 end");
}

/**
 * @tc.name: PowerTaiheNativeTest_005
 * @tc.desc: test power taihe native
 * @tc.type: FUNC
 * @tc.require: issue#ICAK9Z
 */
HWTEST_F(PowerTaiheNativeTest, PowerTaiheNativeTest_005, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerTaiheNativeTest_005 start");
    DevicePowerMode mode(DevicePowerMode::key_t::MODE_NORMAL);
    SetPowerModeSync(mode);

    g_error = PowerErrors::ERR_FAILURE;
    SetPowerModeSync(mode);
    EXPECT_TRUE(g_errCode != static_cast<int32_t>(PowerErrors::ERR_FAILURE));

    g_error = PowerErrors::ERR_CONNECTION_FAIL;
    SetPowerModeSync(mode);
    EXPECT_TRUE(g_errCode == static_cast<int32_t>(PowerErrors::ERR_CONNECTION_FAIL));
    EXPECT_TRUE(g_pass);
    POWER_HILOGI(LABEL_TEST, "PowerTaiheNativeTest_005 end");
}

/**
 * @tc.name: PowerTaiheNativeTest_006
 * @tc.desc: test power taihe native
 * @tc.type: FUNC
 * @tc.require: issue#ICAK9Z
 */
HWTEST_F(PowerTaiheNativeTest, PowerTaiheNativeTest_006, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerTaiheNativeTest_006 start");
    IsStandby();

    g_error = PowerErrors::ERR_FAILURE;
    IsStandby();
    EXPECT_TRUE(g_errCode != static_cast<int32_t>(PowerErrors::ERR_FAILURE));

    g_error = PowerErrors::ERR_CONNECTION_FAIL;
    IsStandby();
    EXPECT_TRUE(g_errCode == static_cast<int32_t>(PowerErrors::ERR_CONNECTION_FAIL));
    EXPECT_TRUE(g_pass);
    POWER_HILOGI(LABEL_TEST, "PowerTaiheNativeTest_006 end");
}

/**
 * @tc.name: PowerTaiheNativeTest_007
 * @tc.desc: test power taihe native
 * @tc.type: FUNC
 * @tc.require: issue#ICAK9Z
 */
HWTEST_F(PowerTaiheNativeTest, PowerTaiheNativeTest_007, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerTaiheNativeTest_007 start");
    bool clearMemory = false;
    Hibernate(clearMemory);

    g_error = PowerErrors::ERR_FAILURE;
    Hibernate(clearMemory);
    EXPECT_TRUE(g_errCode != static_cast<int32_t>(PowerErrors::ERR_FAILURE));

    clearMemory = true;
    g_error = PowerErrors::ERR_SYSTEM_API_DENIED;
    Hibernate(clearMemory);
    EXPECT_TRUE(g_errCode == static_cast<int32_t>(PowerErrors::ERR_SYSTEM_API_DENIED));
    EXPECT_TRUE(g_pass);
    POWER_HILOGI(LABEL_TEST, "PowerTaiheNativeTest_007 end");
}

/**
 * @tc.name: PowerTaiheNativeTest_008
 * @tc.desc: test power taihe native
 * @tc.type: FUNC
 * @tc.require: issue#ICAK9Z
 */
HWTEST_F(PowerTaiheNativeTest, PowerTaiheNativeTest_008, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerTaiheNativeTest_008 start");
    int64_t timeout = 0;
    SetScreenOffTime(timeout);
    EXPECT_TRUE(g_errCode == static_cast<int32_t>(PowerErrors::ERR_PARAM_INVALID));

    constexpr int32_t INVALID_TIMEOUT = -2;
    SetScreenOffTime(INVALID_TIMEOUT);
    EXPECT_TRUE(g_errCode == static_cast<int32_t>(PowerErrors::ERR_PARAM_INVALID));

    constexpr int32_t RESTORE_DEFAULT_SCREENOFF_TIME = -1;
    SetScreenOffTime(RESTORE_DEFAULT_SCREENOFF_TIME);
    EXPECT_TRUE(g_errCode == static_cast<int32_t>(PowerErrors::ERR_PARAM_INVALID));

    timeout = 30000; // ms
    SetScreenOffTime(timeout);

    g_error = PowerErrors::ERR_FAILURE;
    SetScreenOffTime(timeout);
    EXPECT_TRUE(g_errCode != static_cast<int32_t>(PowerErrors::ERR_FAILURE));

    g_error = PowerErrors::ERR_SYSTEM_API_DENIED;
    SetScreenOffTime(timeout);
    EXPECT_TRUE(g_errCode == static_cast<int32_t>(PowerErrors::ERR_SYSTEM_API_DENIED));
    EXPECT_TRUE(g_pass);
    POWER_HILOGI(LABEL_TEST, "PowerTaiheNativeTest_008 end");
}

/**
 * @tc.name: PowerTaiheNativeTest_009
 * @tc.desc: test power taihe native
 * @tc.type: FUNC
 * @tc.require: issue#ICTIYR
 */
HWTEST_F(PowerTaiheNativeTest, PowerTaiheNativeTest_009, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerTaiheNativeTest_009 start");
    RefreshActivity("PowerTaiheNativeTest_009");
    g_error = PowerErrors::ERR_FREQUENT_FUNCTION_CALL;
    RefreshActivity("PowerTaiheNativeTest_009");
    EXPECT_TRUE(g_errCode == static_cast<int32_t>(PowerErrors::ERR_FREQUENT_FUNCTION_CALL));
    EXPECT_TRUE(g_pass);
    POWER_HILOGI(LABEL_TEST, "PowerTaiheNativeTest_009 end");
}

/**
 * @tc.name: PowerTaiheNativeTest_010
 * @tc.desc: test power taihe native
 * @tc.type: FUNC
 * @tc.require: issue#ICTIYR
 */
HWTEST_F(PowerTaiheNativeTest, PowerTaiheNativeTest_010, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerTaiheNativeTest_010 start");
    struct MyCallbackImpl {
        void operator()(bool isReboot)
        {
            g_cbFlag = true;
            EXPECT_TRUE(isReboot == false);
        }
    };
    callback<void(bool)> shutdownCb =
        ::taihe::make_holder<MyCallbackImpl, ::taihe::callback<void(bool)>>();
    RegisterShutdownCallback(shutdownCb);

    std::shared_ptr<PowerShutdownCallback> cb = std::make_shared<PowerShutdownCallback>();
    cb->CreateCallback(shutdownCb);
    bool isReboot = false;
    cb->OnAsyncShutdownOrReboot(isReboot);
    EXPECT_TRUE(g_cbFlag);
    EXPECT_TRUE(g_pass);
    POWER_HILOGI(LABEL_TEST, "PowerTaiheNativeTest_010 end");
}

/**
 * @tc.name: PowerTaiheNativeTest_011
 * @tc.desc: test power taihe native
 * @tc.type: FUNC
 * @tc.require: issue#ICTIYR
 */
HWTEST_F(PowerTaiheNativeTest, PowerTaiheNativeTest_011, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerTaiheNativeTest_011 start");
    struct MyCallbackImpl {
        void operator()()
        {
            g_cbFlag = true;
        }
    };
    callback<void()> shutdownCb =
        ::taihe::make_holder<MyCallbackImpl, ::taihe::callback<void()>>();
    optional<callback<void()>> cb = optional<callback<void()>>{std::in_place, shutdownCb};
    UnRegisterShutdownCallback(cb);
    EXPECT_TRUE(g_cbFlag);
    EXPECT_TRUE(g_pass);
    POWER_HILOGI(LABEL_TEST, "PowerTaiheNativeTest_011 end");
}
} // namespace

