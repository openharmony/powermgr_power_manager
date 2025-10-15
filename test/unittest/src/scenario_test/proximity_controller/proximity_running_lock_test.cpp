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

#include "power_log.h"
#define private public
#include "power_mgr_service.h"
#undef private
#include "mock_state_action.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

namespace {
sptr<PowerMgrService> g_pmsTest {nullptr};
constexpr int SLEEP_WAIT_TIME_MS = 1000;
constexpr int DELAY_WORK_WAIT_TIME_MS = 800;
constexpr int TRANSFER_NS_TO_MS = 1000;
constexpr int TRY_TIMES = 3;
} // namespace

namespace OHOS {
namespace PowerMgr {
class ProximityRunningLockTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() {};
    void TearDown() {};
    bool HoldProximityLock();
    bool UnholdProximityLock();
private:
    static inline sptr<IRemoteObject> lockToken_ {nullptr};
};

bool ProximityRunningLockTest::HoldProximityLock()
{
    if (lockToken_ == nullptr) {
        lockToken_ = sptr<RunningLockTokenStub>::MakeSptr();
        RunningLockInfo info = {"ProximityRunningLockTest", RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL};
        g_pmsTest->CreateRunningLock(lockToken_, info);
    }
    g_pmsTest->Lock(lockToken_);
    return true;
}

bool ProximityRunningLockTest::UnholdProximityLock()
{
    if (lockToken_ == nullptr) {
        POWER_HILOGE(LABEL_TEST, "lockToken_ is nullptr");
        return false;
    }
    g_pmsTest->UnLock(lockToken_);
    return true;
}

void ProximityRunningLockTest::SetUpTestCase()
{
    g_pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    EXPECT_TRUE(g_pmsTest != nullptr) << "ProximityRunningLockTest fail to get PowerMgrService";
    g_pmsTest->OnStart();
    g_pmsTest->SuspendControllerInit();
    g_pmsTest->WakeupControllerInit();
}

void ProximityRunningLockTest::TearDownTestCase()
{
    g_pmsTest->OnStop();
}
} // namespace PowerMgr
} // namespace OHOS

namespace {
/**
 * @tc.name: ProximityRunningLockTest001
 * @tc.desc: Test proximity running lock action RecordSensorCallback
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
HWTEST_F(ProximityRunningLockTest, ProximityRunningLockTest001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ProximityRunningLockTest001 function start!");
    ProximityData closeData = {.distance = ProximityControllerBase::PROXIMITY_CLOSE_SCALAR};
    SensorEvent closeEvent = {
        .sensorTypeId = SENSOR_TYPE_ID_PROXIMITY,
        .data = reinterpret_cast<uint8_t *>(&closeData) };

    ProximityData awayData = {.distance = ProximityControllerBase::PROXIMITY_AWAY_SCALAR};
    SensorEvent awayEvent = {
        .sensorTypeId = SENSOR_TYPE_ID_PROXIMITY,
        .data = reinterpret_cast<uint8_t *>(&awayData) };
    // Try three times
    for (int i = 0; i < TRY_TIMES; ++i) {
        HoldProximityLock();
        RunningLockMgr::ProximityController::RecordSensorCallback(&closeEvent);
        usleep((SLEEP_WAIT_TIME_MS + DELAY_WORK_WAIT_TIME_MS) * TRANSFER_NS_TO_MS);
        POWER_HILOGI(LABEL_TEST, "ProximityRunningLockTest close IsScreenOn:%{public}d", g_pmsTest->IsScreenOn());
        EXPECT_TRUE(g_pmsTest->GetRunningLockMgr()->proximityController_->IsClose());
        EXPECT_FALSE(g_pmsTest->IsScreenOn());

        RunningLockMgr::ProximityController::RecordSensorCallback(&awayEvent);
        usleep(SLEEP_WAIT_TIME_MS * TRANSFER_NS_TO_MS);
        EXPECT_FALSE(g_pmsTest->GetRunningLockMgr()->proximityController_->IsClose());
        POWER_HILOGI(LABEL_TEST, "ProximityRunningLockTest away IsScreenOn:%{public}d", g_pmsTest->IsScreenOn());
        EXPECT_TRUE(g_pmsTest->IsScreenOn());

        if (i) {
            g_pmsTest->isDuringCallStateEnable_ = true;
            EXPECT_TRUE(g_pmsTest->IsDuringCallStateEnable());
        }
        g_pmsTest->GetPowerStateMachine()->SetDuringCallState(true);
        g_pmsTest->GetRunningLockMgr()->HandleProximityAwayEvent();
        g_pmsTest->GetPowerStateMachine()->SetDuringCallState(false);
        EXPECT_TRUE(g_pmsTest->IsScreenOn());
        UnholdProximityLock();
        usleep(SLEEP_WAIT_TIME_MS * TRANSFER_NS_TO_MS / 10); // 100ms for unlock async screen on
        if (i) {
            g_pmsTest->isDuringCallStateEnable_ = false;
            EXPECT_FALSE(g_pmsTest->IsDuringCallStateEnable());
        }
    }
    POWER_HILOGI(LABEL_TEST, "ProximityRunningLockTest001 function end!");
}

/**
 * @tc.name: ProximityRunningLockTest002
 * @tc.desc: Test HandleProximityCloseEvent needDelay false
 * @tc.type: FUNC
 * @tc.require: issues#1567
 */
HWTEST_F(ProximityRunningLockTest, ProximityRunningLockTest002, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ProximityRunningLockTest002 function start!");
    auto stateMachine = g_pmsTest->GetPowerStateMachine();
    ::testing::NiceMock<MockStateAction>* stateActionMock = new ::testing::NiceMock<MockStateAction>;
    stateMachine->EnableMock(stateActionMock);
    EXPECT_CALL(*stateActionMock, SetDisplayState(DisplayState::DISPLAY_OFF, ::testing::_))
        .WillOnce(::testing::Return(ActionResult::SUCCESS));
    HoldProximityLock();
    g_pmsTest->GetRunningLockMgr()->HandleProximityCloseEvent(false);
    EXPECT_CALL(*stateActionMock, GetDisplayState())
        .Times(::testing::AtLeast(1))
        .WillRepeatedly(::testing::Return(DisplayState::DISPLAY_OFF));
    EXPECT_FALSE(g_pmsTest->IsScreenOn());
    EXPECT_CALL(*stateActionMock, SetDisplayState(DisplayState::DISPLAY_ON, ::testing::_))
        .WillOnce(::testing::Return(ActionResult::FAILED));
    UnholdProximityLock();
    ::testing::Mock::AllowLeak(stateActionMock);
    POWER_HILOGI(LABEL_TEST, "ProximityRunningLockTest002 function end!");
}
} // namespace