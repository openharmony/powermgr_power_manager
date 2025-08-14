/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#include "power_screenon_mock_test.h"
#include <fstream>
#include <thread>
#include <unistd.h>

#ifdef POWERMGR_GTEST
#define private   public
#define protected public
#endif

#include <datetime_ex.h>
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
#include <input_manager.h>
#endif
#include <securec.h>

#include "permission.h"
#include "power_mgr_service.h"
#include "power_state_callback_stub.h"
#include "power_state_machine.h"
#include "setting_helper.h"
#include "power_log.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;
static sptr<PowerMgrService> g_service;
static constexpr int32_t TEST_MODEL = 2;
static constexpr int SLEEP_WAIT_TIME_S = 2;

namespace {
bool g_isSystem = true;
bool g_isPermissionGranted = true;
} // namespace

namespace OHOS::PowerMgr {
void PowerScreenOnMockTest::SetUpTestCase(void)
{
    g_service = DelayedSpSingleton<PowerMgrService>::GetInstance();
    g_service->OnStart();
}

void PowerScreenOnMockTest::TearDownTestCase(void)
{
    g_service->OnStop();
    DelayedSpSingleton<PowerMgrService>::DestroyInstance();
}

void PowerScreenOnMockTest::SetUp(void)
{
    EXPECT_TRUE(g_isSystem);
    EXPECT_TRUE(g_isPermissionGranted);
}

void PowerScreenOnMockTest::TearDown(void)
{
    g_isSystem = true;
    g_isPermissionGranted = true;
}

bool PowerMgrService::IsScreenOn(bool needPrintLog)
{
    static uint32_t count = 0;
    bool ret = true;

    if ((count % TEST_MODEL)) {
        ret = !ret;
    }
    count++;
    return ret;
}

bool Permission::IsSystem()
{
    GTEST_LOG_(INFO) << "PowerScreenOnMockTest g_isSystem: " << g_isSystem;
    return g_isSystem;
}

bool Permission::IsPermissionGranted(const std::string& perm)
{
    GTEST_LOG_(INFO) << "PowerScreenOnMockTest IsPermissionGranted: " << g_isSystem;
    return g_isPermissionGranted;
}
} // namespace OHOS::PowerMgr


namespace {
/**
 * @tc.name: PowerScreenOnMockTest001
 * @tc.desc: test ControlListener(exception)
 * @tc.type: FUNC
 * @tc.require: issueI7G6OY
 */
HWTEST_F(PowerScreenOnMockTest, PowerScreenOnMockTest001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerScreenOnMockTest001 function start!");
    GTEST_LOG_(INFO) << "PowerScreenOnMockTest001: start";

    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pmsTest_ == nullptr) {
        GTEST_LOG_(INFO) << "PowerScreenOnMockTest001: Failed to get PowerMgrService";
    }
    pmsTest_->Init();
    pmsTest_->WakeupControllerInit();
    pmsTest_->wakeupController_->stateMachine_->stateAction_->SetDisplayState(DisplayState::DISPLAY_OFF);
    pmsTest_->wakeupController_->ControlListener(WakeupDeviceType ::WAKEUP_DEVICE_POWER_BUTTON);
    EXPECT_TRUE(pmsTest_ != nullptr);

    sleep(SLEEP_WAIT_TIME_S);
    POWER_HILOGI(LABEL_TEST, "PowerScreenOnMockTest001 function end!");
    GTEST_LOG_(INFO) << "PowerScreenOnMockTest001: end";
}

/**
 * @tc.name: PowerScreenOnMockTest002
 * @tc.desc: test RefreshActivity(exception)
 * @tc.type: FUNC
 * @tc.require: issueI7G6OY
 */
HWTEST_F(PowerScreenOnMockTest, PowerScreenOnMockTest002, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerScreenOnMockTest002 function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pmsTest == nullptr) {
        GTEST_LOG_(INFO) << "PowerScreenOnMockTest002: Failed to get PowerMgrService";
    }
    pmsTest->Init();
    std::string reason = "PowerScreenOnMockTest002";
    PowerErrors ret =
        pmsTest->RefreshActivity(GetTickCount(), UserActivityType::USER_ACTIVITY_TYPE_APPLICATION, reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    ret = pmsTest->RefreshActivity(GetTickCount(), UserActivityType::USER_ACTIVITY_TYPE_APPLICATION, reason);
    EXPECT_EQ(ret, PowerErrors::ERR_FREQUENT_FUNCTION_CALL);
    g_isPermissionGranted = false;
    ret = pmsTest->RefreshActivity(GetTickCount(), UserActivityType::USER_ACTIVITY_TYPE_APPLICATION, reason);
    EXPECT_EQ(ret, PowerErrors::ERR_PERMISSION_DENIED);
    g_isSystem = false;
    ret = pmsTest->RefreshActivity(GetTickCount(), UserActivityType::USER_ACTIVITY_TYPE_APPLICATION, reason);
    EXPECT_EQ(ret, PowerErrors::ERR_SYSTEM_API_DENIED);
    POWER_HILOGI(LABEL_TEST, "PowerScreenOnMockTest002 function end!");
}
} // namespace