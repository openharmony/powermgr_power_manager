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
    static void SetUpTestCase() {};
    static void TearDownTestCase() {};
    void SetUp();
    void TearDown();
};

void ProximityRunningLockTest::SetUp()
{
    g_pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    EXPECT_TRUE(g_pmsTest != nullptr) << "ProximityRunningLockTest fail to get PowerMgrService";
    g_pmsTest->OnStart();
    g_pmsTest->SuspendControllerInit();
    g_pmsTest->WakeupControllerInit();
}

void ProximityRunningLockTest::TearDown()
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
    sptr<IRemoteObject> runningLockToken = sptr<RunningLockTokenStub>::MakeSptr();
    RunningLockInfo info = {"ProximityRunningLockTest", RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL};
    g_pmsTest->CreateRunningLock(runningLockToken, info);

    ProximityData awayData = {.distance = ProximityControllerBase::PROXIMITY_AWAY_SCALAR};
    SensorEvent awayEvent = {
        .sensorTypeId = SENSOR_TYPE_ID_PROXIMITY,
        .data = reinterpret_cast<uint8_t *>(&awayData) };
    // Try three times
    for (int i = 0; i < TRY_TIMES; ++i) {
        g_pmsTest->Lock(runningLockToken);
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
        g_pmsTest->UnLock(runningLockToken);
        usleep(SLEEP_WAIT_TIME_MS * TRANSFER_NS_TO_MS / 10); // 100ms for unlock async screen on
    }
    POWER_HILOGI(LABEL_TEST, "ProximityRunningLockTest001 function end!");
}
} // namespace