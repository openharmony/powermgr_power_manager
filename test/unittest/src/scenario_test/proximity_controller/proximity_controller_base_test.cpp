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
#include <memory>

#include "proximity_controller_base.h"
#include "power_log.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

namespace {
SensorInfo* g_sensorInfo = nullptr;
int32_t g_count = 0;
int32_t g_intReturnValue = 0;
int32_t g_intReturnValue1 = 0;
} // namespace

int32_t GetAllSensors(SensorInfo **sensorInfo, int32_t *count)
{
    *sensorInfo = g_sensorInfo;
    *count = g_count;
    return g_intReturnValue;
}

int32_t SubscribeSensor(int32_t /* sensorId */, const SensorUser* /* user */)
{
    return g_intReturnValue;
}

int32_t ActivateSensor(int32_t /* sensorId */, const SensorUser* /* user */)
{
    return g_intReturnValue1;
}

namespace OHOS {
namespace PowerMgr {
class ProximityControllerBaseTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp();
    void TearDown();
};
void ProximityControllerBaseTest::SetUp()
{
    g_sensorInfo = nullptr;
    g_count = 0;
    g_intReturnValue = 0;
    g_intReturnValue1 = 0;
}

void ProximityControllerBaseTest::TearDown()
{
}
} // namespace PowerMgr
} // namespace OHOS

namespace {
/**
 * @tc.name: ProximityControllerBaseTest001
 * @tc.desc: Test proximity controller base abnormal branch
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
HWTEST_F(ProximityControllerBaseTest, ProximityControllerBaseTest001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest001 function start!");
    std::shared_ptr<ProximityControllerBase> proximityControllerBase =
        std::make_shared<ProximityControllerBase>("ProximityControllerBaseTest", nullptr);
    EXPECT_FALSE(proximityControllerBase->IsSupported());
    g_intReturnValue = 1;
    proximityControllerBase = std::make_shared<ProximityControllerBase>("ProximityControllerBaseTest", nullptr);
    EXPECT_FALSE(proximityControllerBase->IsSupported());
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest001 function end!");
}

/**
 * @tc.name: ProximityControllerBaseTest002
 * @tc.desc: Test proximity controller base abnormal branch
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
HWTEST_F(ProximityControllerBaseTest, ProximityControllerBaseTest002, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest002 function start!");
    SensorInfo info;
    g_sensorInfo = &info;
    std::shared_ptr<ProximityControllerBase> proximityControllerBase =
        std::make_shared<ProximityControllerBase>("ProximityControllerBaseTest", nullptr);
    EXPECT_FALSE(proximityControllerBase->IsSupported());
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest002 function end!");
}

/**
 * @tc.name: ProximityControllerBaseTest003
 * @tc.desc: Test proximity controller base Disable abnormal branch
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
HWTEST_F(ProximityControllerBaseTest, ProximityControllerBaseTest003, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest003 function start!");
    SensorInfo info;
    g_sensorInfo = &info;
    std::shared_ptr<ProximityControllerBase> proximityControllerBase =
        std::make_shared<ProximityControllerBase>("ProximityControllerBaseTest", nullptr);
    proximityControllerBase->Disable();
    EXPECT_FALSE(proximityControllerBase->IsEnabled());
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest003 function end!");
}

/**
 * @tc.name: ProximityControllerBaseTest004
 * @tc.desc: Test proximity controller base Enable abnormal branch
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
HWTEST_F(ProximityControllerBaseTest, ProximityControllerBaseTest004, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest004 function start!");
    SensorInfo info;
    g_sensorInfo = &info;
    std::shared_ptr<ProximityControllerBase> proximityControllerBase =
        std::make_shared<ProximityControllerBase>("ProximityControllerBaseTest", nullptr);
    proximityControllerBase->Enable();
    EXPECT_TRUE(proximityControllerBase->IsEnabled());
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest004 function end!");
}

/**
 * @tc.name: ProximityControllerBaseTest005
 * @tc.desc: Test proximity controller base Enable abnormal branch
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
HWTEST_F(ProximityControllerBaseTest, ProximityControllerBaseTest005, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest005 function start!");
    SensorInfo infos[] {{.sensorTypeId = SENSOR_TYPE_ID_PROXIMITY}};
    g_sensorInfo = infos;
    g_count = 1;
    std::shared_ptr<ProximityControllerBase> proximityControllerBase =
        std::make_shared<ProximityControllerBase>("ProximityControllerBaseTest", nullptr);
    EXPECT_TRUE(proximityControllerBase->IsSupported());

    g_intReturnValue = 1;
    proximityControllerBase->Enable();
    EXPECT_TRUE(proximityControllerBase->IsEnabled());

    g_intReturnValue = 0;
    g_intReturnValue1 = 1;
    proximityControllerBase->Enable();
    EXPECT_TRUE(proximityControllerBase->IsEnabled());
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest005 function end!");
}

/**
 * @tc.name: ProximityControllerBaseTest006
 * @tc.desc: Test IProximityController SetStatus
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
HWTEST_F(ProximityControllerBaseTest, ProximityControllerBaseTest006, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest006 function start!");
    std::shared_ptr<ProximityControllerBase> proximityControllerBase =
        std::make_shared<ProximityControllerBase>("ProximityControllerBaseTest", nullptr);
    EXPECT_EQ(proximityControllerBase->GetStatus(), 0);
    proximityControllerBase->SetStatus(1);
    EXPECT_EQ(proximityControllerBase->GetStatus(), 1);
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest006 function end!");
}

/**
 * @tc.name: ProximityControllerBaseTest007
 * @tc.desc: Test proximity controller base InitProximitySensorUser normal branch
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
HWTEST_F(ProximityControllerBaseTest, ProximityControllerBaseTest007, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest007 function start!");
    SensorInfo infos[] {{.sensorTypeId = SENSOR_TYPE_ID_PROXIMITY}};
    g_sensorInfo = infos;
    g_count = 1;
    std::shared_ptr<ProximityControllerBase> proximityControllerBase =
        std::make_shared<ProximityControllerBase>("ProximityControllerBaseTest", nullptr);
    proximityControllerBase->Enable();
    proximityControllerBase->InitProximitySensorUser();
    EXPECT_TRUE(proximityControllerBase->IsSupported());
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest007 function end!");
}

/**
 * @tc.name: ProximityControllerBaseTest008
 * @tc.desc: Test proximity controller base InitProximitySensorUser abnormal branch
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
HWTEST_F(ProximityControllerBaseTest, ProximityControllerBaseTest008, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest008 function start!");
    SensorInfo infos[] {{.sensorTypeId = SENSOR_TYPE_ID_GESTURE}};
    g_sensorInfo = infos;
    g_count = 1;
    std::shared_ptr<ProximityControllerBase> proximityControllerBase =
        std::make_shared<ProximityControllerBase>("ProximityControllerBaseTest", nullptr);
    proximityControllerBase->Enable();
    proximityControllerBase->InitProximitySensorUser();
    EXPECT_FALSE(proximityControllerBase->IsSupported());
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest008 function end!");
}

/**
 * @tc.name: ProximityControllerBaseTest009
 * @tc.desc: Test proximity controller base InitProximitySensorUser abnormal branch
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
HWTEST_F(ProximityControllerBaseTest, ProximityControllerBaseTest009, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest009 function start!");
    SensorInfo infos[] {{.sensorTypeId = SENSOR_TYPE_ID_GESTURE}};
    g_sensorInfo = nullptr;
    g_count = 1;
    std::shared_ptr<ProximityControllerBase> proximityControllerBase =
        std::make_shared<ProximityControllerBase>("ProximityControllerBaseTest", nullptr);
    proximityControllerBase->Enable();
    proximityControllerBase->InitProximitySensorUser();
    EXPECT_FALSE(proximityControllerBase->IsSupported());
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest009 function end!");
}

/**
 * @tc.name: ProximityControllerBaseTest010
 * @tc.desc: Test proximity controller base InitProximitySensorUser abnormal branch
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
HWTEST_F(ProximityControllerBaseTest, ProximityControllerBaseTest010, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest010 function start!");
    SensorInfo infos[] {{.sensorTypeId = SENSOR_TYPE_ID_PROXIMITY}};
    g_sensorInfo = infos;
    g_count = 1;
    std::shared_ptr<ProximityControllerBase> proximityControllerBase = std::make_shared<ProximityControllerBase>(
        "This is a very long string, its length is much longer than sixteen", nullptr);
    proximityControllerBase->Enable();
    proximityControllerBase->InitProximitySensorUser();
    EXPECT_TRUE(proximityControllerBase->IsSupported());
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest010 function end!");
}

/**
 * @tc.name: ProximityControllerBaseTest011
 * @tc.desc: Test proximity controller base InitProximitySensorUser abnormal branch
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
HWTEST_F(ProximityControllerBaseTest, ProximityControllerBaseTest011, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest011 function start!");
    SensorInfo infos[] {{.sensorTypeId = SENSOR_TYPE_ID_PROXIMITY}};
    g_sensorInfo = infos;
    g_count = 1;
    std::shared_ptr<ProximityControllerBase> proximityControllerBase =
        std::make_shared<ProximityControllerBase>("Test", nullptr);
    proximityControllerBase->Enable();
    proximityControllerBase->InitProximitySensorUser();
    EXPECT_TRUE(proximityControllerBase->IsSupported());
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest011 function end!");
}

/**
 * @tc.name: ProximityControllerBaseTest012
 * @tc.desc: Test proximity controller base InitProximitySensorUser abnormal branch
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
HWTEST_F(ProximityControllerBaseTest, ProximityControllerBaseTest012, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest012 function start!");
    SensorInfo infos[] {{.sensorTypeId = SENSOR_TYPE_ID_PROXIMITY}};
    g_sensorInfo = nullptr;
    g_count = 1;
    g_intReturnValue = 1;
    std::shared_ptr<ProximityControllerBase> proximityControllerBase =
        std::make_shared<ProximityControllerBase>("ProximityControllerBaseTest", nullptr);
    proximityControllerBase->Enable();
    proximityControllerBase->InitProximitySensorUser();
    EXPECT_FALSE(proximityControllerBase->IsSupported());
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest012 function end!");
}

/**
 * @tc.name: ProximityControllerBaseTest013
 * @tc.desc: Test proximity controller base InitProximitySensorUser abnormal branch
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
HWTEST_F(ProximityControllerBaseTest, ProximityControllerBaseTest013, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest013 function start!");
    SensorInfo infos[] {{.sensorTypeId = SENSOR_TYPE_ID_PROXIMITY}};
    g_sensorInfo = infos;
    g_count = 1;
    g_intReturnValue = 1;
    std::shared_ptr<ProximityControllerBase> proximityControllerBase =
        std::make_shared<ProximityControllerBase>("ProximityControllerBaseTest", nullptr);
    proximityControllerBase->Enable();
    proximityControllerBase->InitProximitySensorUser();
    EXPECT_FALSE(proximityControllerBase->IsSupported());
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest013 function end!");
}

/**
 * @tc.name: ProximityControllerBaseTest014
 * @tc.desc: Test proximity controller base InitProximitySensorUser abnormal branch
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
HWTEST_F(ProximityControllerBaseTest, ProximityControllerBaseTest014, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest014 function start!");
    SensorInfo infos[] {{.sensorTypeId = SENSOR_TYPE_ID_PROXIMITY}};
    g_sensorInfo = infos;
    g_count = 1;
    g_intReturnValue = 1;
    std::shared_ptr<ProximityControllerBase> proximityControllerBase =
        std::make_shared<ProximityControllerBase>("ProximityControllerBaseTest", nullptr);
    g_intReturnValue = 0;
    proximityControllerBase->Enable();
    proximityControllerBase->InitProximitySensorUser();
    EXPECT_TRUE(proximityControllerBase->IsSupported());
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest014 function end!");
}
} // namespace