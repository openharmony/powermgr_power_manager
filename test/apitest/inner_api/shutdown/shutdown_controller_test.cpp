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
#define private   public
#define protected public

#include <condition_variable>
#include <mutex>

#ifdef POWERMGR_GTEST
#define private   public
#define protected public
#endif

#include <gtest/gtest.h>
#include <mock_power_action.h>
#include <mock_state_action.h>
#include <power_log.h>
#include <shutdown/takeover_shutdown_callback_stub.h>
#include <shutdown_controller.h>
#include "power_errors.h"
#include "power_mgr_service.h"
#include <screen_manager_lite.h>

namespace OHOS::Rosen {
bool ScreenManagerLite::SetScreenPowerForAll(Rosen::ScreenPowerState state, Rosen::PowerStateChangeReason reason)
{
    return false;
}
}

namespace OHOS::PowerMgr {
sptr<PowerMgrService> g_pmsTest = nullptr;
class ShutDownControllerTest : public testing::Test {
public:
    bool called = false;
    class TakeOverShutdownCallback : public TakeOverShutdownCallbackStub {
    public:
        ~TakeOverShutdownCallback() override = default;
        bool OnTakeOverShutdown(const TakeOverInfo& info) override
        {
            if (info.reason_ == "takeoverFalse") {
                return false;
            }
            return true;
        }
        bool OnTakeOverHibernate(const TakeOverInfo& info) override
        {
            if (info.reason_ == "takeoverFalse") {
                return false;
            }
            return true;
        }
    };
};

namespace {
using namespace OHOS;
using namespace PowerMgr;
using namespace testing;
using namespace ext;
HWTEST_F(ShutDownControllerTest, ShutDownControllerTest001, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "ShutdownControllerTest001 function start!");
    sptr<ITakeOverShutdownCallback> callback = sptr<TakeOverShutdownCallback>::MakeSptr();
    NiceMock<MockStateAction>* stateActionMock = new NiceMock<MockStateAction>;
    NiceMock<MockPowerAction>* powerActionMock = new NiceMock<MockPowerAction>;
    ShutdownController controller;
    controller.EnableMock(powerActionMock, stateActionMock);
    controller.AddCallback(callback, ShutdownPriority::DEFAULT);
    EXPECT_CALL(*powerActionMock, Reboot("::testing::_")).Times(0);
    controller.Reboot("test_case");

    std::mutex localMutex;
    std::condition_variable cv;
    bool notified = false;
    EXPECT_CALL(*powerActionMock, Reboot("test_case")).WillOnce(Invoke([&localMutex, &cv, &notified]() {
        std::unique_lock lock(localMutex);
        notified = true;
        cv.notify_all();
    }));
    controller.Reboot("test_case", true);
    std::unique_lock lock(localMutex);
    cv.wait(lock, [&notified]() { return notified; });
    POWER_HILOGI(LABEL_TEST, "ShutdownControllerTest001 function end!");
}

/**
 * @tc.name: ShutDownControllerTest002
 * @tc.desc: Test TriggerTakeoverHibernateCallback
 * @tc.type: FUNC
 */
HWTEST_F(ShutDownControllerTest, ShutDownControllerTest002, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "ShutDownControllerTest002 function start!");
    sptr<ITakeOverShutdownCallback> callback = sptr<TakeOverShutdownCallback>::MakeSptr();
    ShutdownController controller;
    controller.AddCallback(callback, ShutdownPriority::DEFAULT);
    bool ret = controller.TriggerTakeOverHibernateCallback(TakeOverInfo("takeoverTrue", true));
    EXPECT_TRUE(ret);
    ret = controller.TriggerTakeOverHibernateCallback(TakeOverInfo("takeoverFalse", false));
    EXPECT_FALSE(ret);
    POWER_HILOGI(LABEL_TEST, "ShutDownControllerTest002 function end!");
}

/**
 * @tc.name: ShutDownControllerTest003
 * @tc.desc: Test TriggerTakeoverShutdownCallback
 * @tc.type: FUNC
 */
HWTEST_F(ShutDownControllerTest, ShutDownControllerTest003, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "ShutDownControllerTest003 function start!");
    sptr<ITakeOverShutdownCallback> callback = sptr<TakeOverShutdownCallback>::MakeSptr();
    ShutdownController controller;
    controller.AddCallback(callback, ShutdownPriority::DEFAULT);
    bool ret = controller.TriggerTakeOverShutdownCallback(TakeOverInfo("takeoverTrue", true));
    EXPECT_TRUE(ret);
    ret = controller.TriggerTakeOverShutdownCallback(TakeOverInfo("takeoverFalse", false));
    EXPECT_FALSE(ret);
    POWER_HILOGI(LABEL_TEST, "ShutDownControllerTest003 function end!");
}

/**
 * @tc.name: ShutDownControllerTest004
 * @tc.desc: Test SetShutdownReason
 * @tc.type: FUNC
 */
HWTEST_F(ShutDownControllerTest, ShutDownControllerTest004, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "ShutDownControllerTest004 function start!");
    ShutdownController controller;
    g_pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    EXPECT_TRUE(g_pmsTest != nullptr);
    g_pmsTest->OnStart();
    std::string setReasonFirst = "reasonfirst";
    std::string setReasonSecond =
        "reasonfirstreasonfirstreasonfirstreasonfirstreasonfirstreasonfirstreasonfirstreasonfirstreasonfirst";
    std::string getReasonFirst = "";
    std::string getReasonSecond = "";
    controller.SetShutdownReason(setReasonFirst);
    PowerErrors ret = g_pmsTest->GetShutdownReason(getReasonFirst);
    EXPECT_TRUE(ret == PowerErrors::ERR_OK);
    EXPECT_TRUE(getReasonFirst == "reasonfirst");
    controller.SetShutdownReason(setReasonSecond);
    ret = g_pmsTest->GetShutdownReason(getReasonSecond);
    EXPECT_TRUE(ret == PowerErrors::ERR_OK);
    EXPECT_TRUE(getReasonSecond ==
        "reasonfirstreasonfirstreasonfirstreasonfirstreasonfirstreasonfirstreasonfirstreasonfirstreasonf");
    POWER_HILOGI(LABEL_TEST, "ShutDownControllerTest004 function end!");
}
} // namespace
} // namespace OHOS::PowerMgr