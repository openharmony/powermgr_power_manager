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

/**
 * @tc.name: ProximityControllerBaseTest015
 * @tc.desc: Test proximity controller base InitProximitySensorUser normal branch
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
HWTEST_F(ProximityControllerBaseTest, ProximityControllerBaseTest015, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest015 function start!");
    SensorInfo infos[] {{.sensorTypeId = SENSOR_TYPE_ID_PROXIMITY}};
    g_sensorInfo = infos;
    g_count = 1;
    std::shared_ptr<ProximityControllerBase> proximityControllerBase =
        std::make_shared<ProximityControllerBase>("it is for test a long string", nullptr);
    proximityControllerBase->Enable();
    proximityControllerBase->InitProximitySensorUser();
    EXPECT_TRUE(proximityControllerBase->IsSupported());
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest015 function end!");
}

/**
 * @tc.name: ProximityControllerBaseTest016
 * @tc.desc: Test proximity controller base InitProximitySensorUser abnormal branch
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
HWTEST_F(ProximityControllerBaseTest, ProximityControllerBaseTest016, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest016 function start!");
    SensorInfo infos[] {{.sensorTypeId = SENSOR_TYPE_ID_GESTURE}};
    g_sensorInfo = infos;
    g_count = 1;
    std::shared_ptr<ProximityControllerBase> proximityControllerBase =
        std::make_shared<ProximityControllerBase>("it is for test a long string", nullptr);
    proximityControllerBase->Enable();
    proximityControllerBase->InitProximitySensorUser();
    EXPECT_FALSE(proximityControllerBase->IsSupported());
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest016 function end!");
}

/**
 * @tc.name: ProximityControllerBaseTest017
 * @tc.desc: Test proximity controller base InitProximitySensorUser abnormal branch
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
HWTEST_F(ProximityControllerBaseTest, ProximityControllerBaseTest017, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest017 function start!");
    SensorInfo infos[] {{.sensorTypeId = SENSOR_TYPE_ID_GESTURE}};
    g_sensorInfo = nullptr;
    g_count = 1;
    std::shared_ptr<ProximityControllerBase> proximityControllerBase =
        std::make_shared<ProximityControllerBase>("it is for test a long string", nullptr);
    proximityControllerBase->Enable();
    proximityControllerBase->InitProximitySensorUser();
    EXPECT_FALSE(proximityControllerBase->IsSupported());
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest017 function end!");
}

/**
 * @tc.name: ProximityControllerBaseTest018
 * @tc.desc: Test proximity controller base InitProximitySensorUser abnormal branch
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
HWTEST_F(ProximityControllerBaseTest, ProximityControllerBaseTest018, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest018 function start!");
    SensorInfo infos[] {{.sensorTypeId = SENSOR_TYPE_ID_PROXIMITY}};
    g_sensorInfo = infos;
    g_count = 1;
    std::shared_ptr<ProximityControllerBase> proximityControllerBase =
        std::make_shared<ProximityControllerBase>("ProximityControllerBaseTest018", nullptr);
    proximityControllerBase->Enable();
    proximityControllerBase->InitProximitySensorUser();
    EXPECT_TRUE(proximityControllerBase->IsSupported());
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest018 function end!");
}

/**
 * @tc.name: ProximityControllerBaseTest019
 * @tc.desc: Test proximity controller base InitProximitySensorUser abnormal branch
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
HWTEST_F(ProximityControllerBaseTest, ProximityControllerBaseTest019, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest019 function start!");
    SensorInfo infos[] {{.sensorTypeId = SENSOR_TYPE_ID_PROXIMITY}};
    g_sensorInfo = infos;
    g_count = 1;
    std::shared_ptr<ProximityControllerBase> proximityControllerBase =
        std::make_shared<ProximityControllerBase>("it is for test a long string", nullptr);
    proximityControllerBase->Enable();
    proximityControllerBase->InitProximitySensorUser();
    EXPECT_TRUE(proximityControllerBase->IsSupported());
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest019 function end!");
}

/**
 * @tc.name: ProximityControllerBaseTest020
 * @tc.desc: Test proximity controller base InitProximitySensorUser abnormal branch
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
HWTEST_F(ProximityControllerBaseTest, ProximityControllerBaseTest020, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest020 function start!");
    SensorInfo infos[] {{.sensorTypeId = SENSOR_TYPE_ID_PROXIMITY}};
    g_sensorInfo = nullptr;
    g_count = 1;
    g_intReturnValue = 1;
    std::shared_ptr<ProximityControllerBase> proximityControllerBase =
        std::make_shared<ProximityControllerBase>("it is for test a long string", nullptr);
    proximityControllerBase->Enable();
    proximityControllerBase->InitProximitySensorUser();
    EXPECT_FALSE(proximityControllerBase->IsSupported());
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest020 function end!");
}

/**
 * @tc.name: ProximityControllerBaseTest021
 * @tc.desc: Test proximity controller base InitProximitySensorUser abnormal branch
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
HWTEST_F(ProximityControllerBaseTest, ProximityControllerBaseTest021, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest021 function start!");
    SensorInfo infos[] {{.sensorTypeId = SENSOR_TYPE_ID_PROXIMITY}};
    g_sensorInfo = infos;
    g_count = 1;
    g_intReturnValue = 1;
    std::shared_ptr<ProximityControllerBase> proximityControllerBase =
        std::make_shared<ProximityControllerBase>("it is for test a long string", nullptr);
    proximityControllerBase->Enable();
    proximityControllerBase->InitProximitySensorUser();
    EXPECT_FALSE(proximityControllerBase->IsSupported());
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest021 function end!");
}

/**
 * @tc.name: ProximityControllerBaseTest022
 * @tc.desc: Test proximity controller base InitProximitySensorUser abnormal branch
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
HWTEST_F(ProximityControllerBaseTest, ProximityControllerBaseTest022, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest022 function start!");
    SensorInfo infos[] {{.sensorTypeId = SENSOR_TYPE_ID_PROXIMITY}};
    g_sensorInfo = infos;
    g_count = 1;
    g_intReturnValue = 1;
    std::shared_ptr<ProximityControllerBase> proximityControllerBase =
        std::make_shared<ProximityControllerBase>("it is for test a long string", nullptr);
    g_intReturnValue = 0;
    proximityControllerBase->Enable();
    proximityControllerBase->InitProximitySensorUser();
    EXPECT_TRUE(proximityControllerBase->IsSupported());
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest022 function end!");
}

/**
 * @tc.name: ProximityControllerBaseTest023
 * @tc.desc: Test proximity controller base InitProximitySensorUser abnormal branch
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
HWTEST_F(ProximityControllerBaseTest, ProximityControllerBaseTest023, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest023 function start!");
    SensorInfo infos[] {{.sensorTypeId = SENSOR_TYPE_ID_NONE}};
    g_sensorInfo = infos;
    g_count = 1;
    std::shared_ptr<ProximityControllerBase> proximityControllerBase =
        std::make_shared<ProximityControllerBase>("it is for test a long string", nullptr);
    proximityControllerBase->Enable();
    proximityControllerBase->InitProximitySensorUser();
    EXPECT_FALSE(proximityControllerBase->IsSupported());
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest023 function end!");
}

/**
 * @tc.name: ProximityControllerBaseTest024
 * @tc.desc: Test proximity controller base InitProximitySensorUser abnormal branch
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
HWTEST_F(ProximityControllerBaseTest, ProximityControllerBaseTest024, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest024 function start!");
    SensorInfo infos[] {{.sensorTypeId = SENSOR_TYPE_ID_ACCELEROMETER}};
    g_sensorInfo = infos;
    g_count = 1;
    std::shared_ptr<ProximityControllerBase> proximityControllerBase =
        std::make_shared<ProximityControllerBase>("it is for test a long string", nullptr);
    proximityControllerBase->Enable();
    proximityControllerBase->InitProximitySensorUser();
    EXPECT_FALSE(proximityControllerBase->IsSupported());
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest024 function end!");
}

/**
 * @tc.name: ProximityControllerBaseTest025
 * @tc.desc: Test proximity controller base InitProximitySensorUser abnormal branch
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
HWTEST_F(ProximityControllerBaseTest, ProximityControllerBaseTest025, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest025 function start!");
    SensorInfo infos[] {{.sensorTypeId = SENSOR_TYPE_ID_GYROSCOPE}};
    g_sensorInfo = infos;
    g_count = 1;
    std::shared_ptr<ProximityControllerBase> proximityControllerBase =
        std::make_shared<ProximityControllerBase>("it is for test a long string", nullptr);
    proximityControllerBase->Enable();
    proximityControllerBase->InitProximitySensorUser();
    EXPECT_FALSE(proximityControllerBase->IsSupported());
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest025 function end!");
}

/**
 * @tc.name: ProximityControllerBaseTest026
 * @tc.desc: Test proximity controller base InitProximitySensorUser abnormal branch
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
HWTEST_F(ProximityControllerBaseTest, ProximityControllerBaseTest026, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest026 function start!");
    SensorInfo infos[] {{.sensorTypeId = SENSOR_TYPE_ID_GYROSCOPE}};
    g_sensorInfo = infos;
    g_count = 1;
    std::shared_ptr<ProximityControllerBase> proximityControllerBase =
        std::make_shared<ProximityControllerBase>("ProximityControllerBaseTest026", nullptr);
    proximityControllerBase->Enable();
    proximityControllerBase->InitProximitySensorUser();
    EXPECT_FALSE(proximityControllerBase->IsSupported());
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest026 function end!");
}

/**
 * @tc.name: ProximityControllerBaseTest027
 * @tc.desc: Test proximity controller base InitProximitySensorUser abnormal branch
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
HWTEST_F(ProximityControllerBaseTest, ProximityControllerBaseTest027, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest027 function start!");
    SensorInfo infos[] {{.sensorTypeId = SENSOR_TYPE_ID_AMBIENT_LIGHT}};
    g_sensorInfo = infos;
    g_count = 1;
    std::shared_ptr<ProximityControllerBase> proximityControllerBase =
        std::make_shared<ProximityControllerBase>("ProximityControllerBaseTest027", nullptr);
    proximityControllerBase->Enable();
    proximityControllerBase->InitProximitySensorUser();
    EXPECT_FALSE(proximityControllerBase->IsSupported());
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest027 function end!");
}

/**
 * @tc.name: ProximityControllerBaseTest028
 * @tc.desc: Test proximity controller base InitProximitySensorUser abnormal branch
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
HWTEST_F(ProximityControllerBaseTest, ProximityControllerBaseTest028, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest028 function start!");
    SensorInfo infos[] {{.sensorTypeId = SENSOR_TYPE_ID_AMBIENT_LIGHT}};
    g_sensorInfo = infos;
    g_count = 1;
    std::shared_ptr<ProximityControllerBase> proximityControllerBase =
        std::make_shared<ProximityControllerBase>("it is for test a long string", nullptr);
    proximityControllerBase->Enable();
    proximityControllerBase->InitProximitySensorUser();
    EXPECT_FALSE(proximityControllerBase->IsSupported());
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest028 function end!");
}

/**
 * @tc.name: ProximityControllerBaseTest029
 * @tc.desc: Test proximity controller base InitProximitySensorUser abnormal branch
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
HWTEST_F(ProximityControllerBaseTest, ProximityControllerBaseTest029, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest029 function start!");
    SensorInfo infos[] {{.sensorTypeId = SENSOR_TYPE_ID_MAGNETIC_FIELD}};
    g_sensorInfo = infos;
    g_count = 1;
    std::shared_ptr<ProximityControllerBase> proximityControllerBase =
        std::make_shared<ProximityControllerBase>("it is for test a long string", nullptr);
    proximityControllerBase->Enable();
    proximityControllerBase->InitProximitySensorUser();
    EXPECT_FALSE(proximityControllerBase->IsSupported());
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest029 function end!");
}

/**
 * @tc.name: ProximityControllerBaseTest030
 * @tc.desc: Test proximity controller base InitProximitySensorUser abnormal branch
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
HWTEST_F(ProximityControllerBaseTest, ProximityControllerBaseTest030, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest030 function start!");
    SensorInfo infos[] {{.sensorTypeId = SENSOR_TYPE_ID_CAPACITIVE}};
    g_sensorInfo = infos;
    g_count = 1;
    std::shared_ptr<ProximityControllerBase> proximityControllerBase =
        std::make_shared<ProximityControllerBase>("it is for test a long string", nullptr);
    proximityControllerBase->Enable();
    proximityControllerBase->InitProximitySensorUser();
    EXPECT_FALSE(proximityControllerBase->IsSupported());
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest030 function end!");
}

/**
 * @tc.name: ProximityControllerBaseTest031
 * @tc.desc: Test proximity controller base InitProximitySensorUser abnormal branch
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
HWTEST_F(ProximityControllerBaseTest, ProximityControllerBaseTest031, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest031 function start!");
    SensorInfo infos[] {{.sensorTypeId = SENSOR_TYPE_ID_BAROMETER}};
    g_sensorInfo = infos;
    g_count = 1;
    std::shared_ptr<ProximityControllerBase> proximityControllerBase =
        std::make_shared<ProximityControllerBase>("it is for test a long string", nullptr);
    proximityControllerBase->Enable();
    proximityControllerBase->InitProximitySensorUser();
    EXPECT_FALSE(proximityControllerBase->IsSupported());
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest031 function end!");
}

/**
 * @tc.name: ProximityControllerBaseTest032
 * @tc.desc: Test proximity controller base InitProximitySensorUser abnormal branch
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
HWTEST_F(ProximityControllerBaseTest, ProximityControllerBaseTest032, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest032 function start!");
    SensorInfo infos[] {{.sensorTypeId = SENSOR_TYPE_ID_TEMPERATURE}};
    g_sensorInfo = infos;
    g_count = 1;
    std::shared_ptr<ProximityControllerBase> proximityControllerBase =
        std::make_shared<ProximityControllerBase>("it is for test a long string", nullptr);
    proximityControllerBase->Enable();
    proximityControllerBase->InitProximitySensorUser();
    EXPECT_FALSE(proximityControllerBase->IsSupported());
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest032 function end!");
}

/**
 * @tc.name: ProximityControllerBaseTest033
 * @tc.desc: Test proximity controller base InitProximitySensorUser abnormal branch
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
HWTEST_F(ProximityControllerBaseTest, ProximityControllerBaseTest033, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest033 function start!");
    SensorInfo infos[] {{.sensorTypeId = SENSOR_TYPE_ID_HALL}};
    g_sensorInfo = infos;
    g_count = 1;
    std::shared_ptr<ProximityControllerBase> proximityControllerBase =
        std::make_shared<ProximityControllerBase>("it is for test a long string", nullptr);
    proximityControllerBase->Enable();
    proximityControllerBase->InitProximitySensorUser();
    EXPECT_FALSE(proximityControllerBase->IsSupported());
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest033 function end!");
}

/**
 * @tc.name: ProximityControllerBaseTest034
 * @tc.desc: Test proximity controller base InitProximitySensorUser abnormal branch
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
HWTEST_F(ProximityControllerBaseTest, ProximityControllerBaseTest034, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest034 function start!");
    SensorInfo infos[] {{.sensorTypeId = SENSOR_TYPE_ID_HUMIDITY}};
    g_sensorInfo = infos;
    g_count = 1;
    std::shared_ptr<ProximityControllerBase> proximityControllerBase =
        std::make_shared<ProximityControllerBase>("it is for test a long string", nullptr);
    proximityControllerBase->Enable();
    proximityControllerBase->InitProximitySensorUser();
    EXPECT_FALSE(proximityControllerBase->IsSupported());
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest034 function end!");
}

/**
 * @tc.name: ProximityControllerBaseTest035
 * @tc.desc: Test proximity controller base InitProximitySensorUser abnormal branch
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
HWTEST_F(ProximityControllerBaseTest, ProximityControllerBaseTest035, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest035 function start!");
    SensorInfo infos[] {{.sensorTypeId = SENSOR_TYPE_ID_PHYSICAL_MAX}};
    g_sensorInfo = infos;
    g_count = 1;
    std::shared_ptr<ProximityControllerBase> proximityControllerBase =
        std::make_shared<ProximityControllerBase>("it is for test a long string", nullptr);
    proximityControllerBase->Enable();
    proximityControllerBase->InitProximitySensorUser();
    EXPECT_FALSE(proximityControllerBase->IsSupported());
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest035 function end!");
}

/**
 * @tc.name: ProximityControllerBaseTest036
 * @tc.desc: Test proximity controller base InitProximitySensorUser abnormal branch
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
HWTEST_F(ProximityControllerBaseTest, ProximityControllerBaseTest036, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest036 function start!");
    SensorInfo infos[] {{.sensorTypeId = SENSOR_TYPE_ID_ORIENTATION}};
    g_sensorInfo = infos;
    g_count = 1;
    std::shared_ptr<ProximityControllerBase> proximityControllerBase =
        std::make_shared<ProximityControllerBase>("it is for test a long string", nullptr);
    proximityControllerBase->Enable();
    proximityControllerBase->InitProximitySensorUser();
    EXPECT_FALSE(proximityControllerBase->IsSupported());
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest036 function end!");
}

/**
 * @tc.name: ProximityControllerBaseTest037
 * @tc.desc: Test proximity controller base InitProximitySensorUser abnormal branch
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
HWTEST_F(ProximityControllerBaseTest, ProximityControllerBaseTest037, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest037 function start!");
    SensorInfo infos[] {{.sensorTypeId = SENSOR_TYPE_ID_GRAVITY}};
    g_sensorInfo = infos;
    g_count = 1;
    std::shared_ptr<ProximityControllerBase> proximityControllerBase =
        std::make_shared<ProximityControllerBase>("it is for test a long string", nullptr);
    proximityControllerBase->Enable();
    proximityControllerBase->InitProximitySensorUser();
    EXPECT_FALSE(proximityControllerBase->IsSupported());
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest037 function end!");
}

/**
 * @tc.name: ProximityControllerBaseTest038
 * @tc.desc: Test proximity controller base InitProximitySensorUser abnormal branch
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
HWTEST_F(ProximityControllerBaseTest, ProximityControllerBaseTest038, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest038 function start!");
    SensorInfo infos[] {{.sensorTypeId = SENSOR_TYPE_ID_LINEAR_ACCELERATION}};
    g_sensorInfo = infos;
    g_count = 1;
    std::shared_ptr<ProximityControllerBase> proximityControllerBase =
        std::make_shared<ProximityControllerBase>("it is for test a long string", nullptr);
    proximityControllerBase->Enable();
    proximityControllerBase->InitProximitySensorUser();
    EXPECT_FALSE(proximityControllerBase->IsSupported());
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest038 function end!");
}

/**
 * @tc.name: ProximityControllerBaseTest039
 * @tc.desc: Test proximity controller base InitProximitySensorUser abnormal branch
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
HWTEST_F(ProximityControllerBaseTest, ProximityControllerBaseTest039, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest039 function start!");
    SensorInfo infos[] {{.sensorTypeId = SENSOR_TYPE_ID_ROTATION_VECTOR}};
    g_sensorInfo = infos;
    g_count = 1;
    std::shared_ptr<ProximityControllerBase> proximityControllerBase =
        std::make_shared<ProximityControllerBase>("it is for test a long string", nullptr);
    proximityControllerBase->Enable();
    proximityControllerBase->InitProximitySensorUser();
    EXPECT_FALSE(proximityControllerBase->IsSupported());
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest039 function end!");
}

/**
 * @tc.name: ProximityControllerBaseTest040
 * @tc.desc: Test proximity controller base InitProximitySensorUser abnormal branch
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
HWTEST_F(ProximityControllerBaseTest, ProximityControllerBaseTest040, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest040 function start!");
    SensorInfo infos[] {{.sensorTypeId = SENSOR_TYPE_ID_AMBIENT_TEMPERATURE}};
    g_sensorInfo = infos;
    g_count = 1;
    std::shared_ptr<ProximityControllerBase> proximityControllerBase =
        std::make_shared<ProximityControllerBase>("it is for test a long string", nullptr);
    proximityControllerBase->Enable();
    proximityControllerBase->InitProximitySensorUser();
    EXPECT_FALSE(proximityControllerBase->IsSupported());
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest040 function end!");
}

/**
 * @tc.name: ProximityControllerBaseTest041
 * @tc.desc: Test proximity controller base InitProximitySensorUser abnormal branch
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
HWTEST_F(ProximityControllerBaseTest, ProximityControllerBaseTest041, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest041 function start!");
    SensorInfo infos[] {{.sensorTypeId = SENSOR_TYPE_ID_MAGNETIC_FIELD_UNCALIBRATED}};
    g_sensorInfo = infos;
    g_count = 1;
    std::shared_ptr<ProximityControllerBase> proximityControllerBase =
        std::make_shared<ProximityControllerBase>("it is for test a long string", nullptr);
    proximityControllerBase->Enable();
    proximityControllerBase->InitProximitySensorUser();
    EXPECT_FALSE(proximityControllerBase->IsSupported());
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest041 function end!");
}

/**
 * @tc.name: ProximityControllerBaseTest042
 * @tc.desc: Test proximity controller base InitProximitySensorUser abnormal branch
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
HWTEST_F(ProximityControllerBaseTest, ProximityControllerBaseTest042, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest042 function start!");
    SensorInfo infos[] {{.sensorTypeId = SENSOR_TYPE_ID_GAME_ROTATION_VECTOR}};
    g_sensorInfo = infos;
    g_count = 1;
    std::shared_ptr<ProximityControllerBase> proximityControllerBase =
        std::make_shared<ProximityControllerBase>("it is for test a long string", nullptr);
    proximityControllerBase->Enable();
    proximityControllerBase->InitProximitySensorUser();
    EXPECT_FALSE(proximityControllerBase->IsSupported());
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest042 function end!");
}

/**
 * @tc.name: ProximityControllerBaseTest043
 * @tc.desc: Test proximity controller base InitProximitySensorUser abnormal branch
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
HWTEST_F(ProximityControllerBaseTest, ProximityControllerBaseTest043, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest043 function start!");
    SensorInfo infos[] {{.sensorTypeId = SENSOR_TYPE_ID_GYROSCOPE_UNCALIBRATED}};
    g_sensorInfo = infos;
    g_count = 1;
    std::shared_ptr<ProximityControllerBase> proximityControllerBase =
        std::make_shared<ProximityControllerBase>("it is for test a long string", nullptr);
    proximityControllerBase->Enable();
    proximityControllerBase->InitProximitySensorUser();
    EXPECT_FALSE(proximityControllerBase->IsSupported());
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest043 function end!");
}

/**
 * @tc.name: ProximityControllerBaseTest044
 * @tc.desc: Test proximity controller base InitProximitySensorUser abnormal branch
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
HWTEST_F(ProximityControllerBaseTest, ProximityControllerBaseTest044, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest044 function start!");
    SensorInfo infos[] {{.sensorTypeId = SENSOR_TYPE_ID_SIGNIFICANT_MOTION}};
    g_sensorInfo = infos;
    g_count = 1;
    std::shared_ptr<ProximityControllerBase> proximityControllerBase =
        std::make_shared<ProximityControllerBase>("it is for test a long string", nullptr);
    proximityControllerBase->Enable();
    proximityControllerBase->InitProximitySensorUser();
    EXPECT_FALSE(proximityControllerBase->IsSupported());
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest044 function end!");
}

/**
 * @tc.name: ProximityControllerBaseTest045
 * @tc.desc: Test proximity controller base InitProximitySensorUser abnormal branch
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
HWTEST_F(ProximityControllerBaseTest, ProximityControllerBaseTest045, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest045 function start!");
    SensorInfo infos[] {{.sensorTypeId = SENSOR_TYPE_ID_PEDOMETER_DETECTION}};
    g_sensorInfo = infos;
    g_count = 1;
    std::shared_ptr<ProximityControllerBase> proximityControllerBase =
        std::make_shared<ProximityControllerBase>("it is for test a long string", nullptr);
    proximityControllerBase->Enable();
    proximityControllerBase->InitProximitySensorUser();
    EXPECT_FALSE(proximityControllerBase->IsSupported());
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest045 function end!");
}

/**
 * @tc.name: ProximityControllerBaseTest046
 * @tc.desc: Test proximity controller base InitProximitySensorUser abnormal branch
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
HWTEST_F(ProximityControllerBaseTest, ProximityControllerBaseTest046, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest046 function start!");
    SensorInfo infos[] {{.sensorTypeId = SENSOR_TYPE_ID_PEDOMETER}};
    g_sensorInfo = infos;
    g_count = 1;
    std::shared_ptr<ProximityControllerBase> proximityControllerBase =
        std::make_shared<ProximityControllerBase>("it is for test a long string", nullptr);
    proximityControllerBase->Enable();
    proximityControllerBase->InitProximitySensorUser();
    EXPECT_FALSE(proximityControllerBase->IsSupported());
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest046 function end!");
}

/**
 * @tc.name: ProximityControllerBaseTest047
 * @tc.desc: Test proximity controller base InitProximitySensorUser abnormal branch
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
HWTEST_F(ProximityControllerBaseTest, ProximityControllerBaseTest047, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest047 function start!");
    SensorInfo infos[] {{.sensorTypeId = SENSOR_TYPE_ID_GEOMAGNETIC_ROTATION_VECTOR}};
    g_sensorInfo = infos;
    g_count = 1;
    std::shared_ptr<ProximityControllerBase> proximityControllerBase =
        std::make_shared<ProximityControllerBase>("it is for test a long string", nullptr);
    proximityControllerBase->Enable();
    proximityControllerBase->InitProximitySensorUser();
    EXPECT_FALSE(proximityControllerBase->IsSupported());
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest047 function end!");
}

/**
 * @tc.name: ProximityControllerBaseTest048
 * @tc.desc: Test proximity controller base InitProximitySensorUser abnormal branch
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
HWTEST_F(ProximityControllerBaseTest, ProximityControllerBaseTest048, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest048 function start!");
    SensorInfo infos[] {{.sensorTypeId = SENSOR_TYPE_ID_HEART_RATE}};
    g_sensorInfo = infos;
    g_count = 1;
    std::shared_ptr<ProximityControllerBase> proximityControllerBase =
        std::make_shared<ProximityControllerBase>("it is for test a long string", nullptr);
    proximityControllerBase->Enable();
    proximityControllerBase->InitProximitySensorUser();
    EXPECT_FALSE(proximityControllerBase->IsSupported());
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest048 function end!");
}

/**
 * @tc.name: ProximityControllerBaseTest049
 * @tc.desc: Test proximity controller base InitProximitySensorUser abnormal branch
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
HWTEST_F(ProximityControllerBaseTest, ProximityControllerBaseTest049, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest049 function start!");
    SensorInfo infos[] {{.sensorTypeId = SENSOR_TYPE_ID_DEVICE_ORIENTATION}};
    g_sensorInfo = infos;
    g_count = 1;
    std::shared_ptr<ProximityControllerBase> proximityControllerBase =
        std::make_shared<ProximityControllerBase>("it is for test a long string", nullptr);
    proximityControllerBase->Enable();
    proximityControllerBase->InitProximitySensorUser();
    EXPECT_FALSE(proximityControllerBase->IsSupported());
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest049 function end!");
}

/**
 * @tc.name: ProximityControllerBaseTest050
 * @tc.desc: Test proximity controller base InitProximitySensorUser abnormal branch
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
HWTEST_F(ProximityControllerBaseTest, ProximityControllerBaseTest050, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest050 function start!");
    SensorInfo infos[] {{.sensorTypeId = SENSOR_TYPE_ID_WEAR_DETECTION}};
    g_sensorInfo = infos;
    g_count = 1;
    std::shared_ptr<ProximityControllerBase> proximityControllerBase =
        std::make_shared<ProximityControllerBase>("it is for test a long string", nullptr);
    proximityControllerBase->Enable();
    proximityControllerBase->InitProximitySensorUser();
    EXPECT_FALSE(proximityControllerBase->IsSupported());
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest050 function end!");
}

/**
 * @tc.name: ProximityControllerBaseTest051
 * @tc.desc: Test proximity controller base InitProximitySensorUser abnormal branch
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
HWTEST_F(ProximityControllerBaseTest, ProximityControllerBaseTest051, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest051 function start!");
    SensorInfo infos[] {{.sensorTypeId = SENSOR_TYPE_ID_ACCELEROMETER_UNCALIBRATED}};
    g_sensorInfo = infos;
    g_count = 1;
    std::shared_ptr<ProximityControllerBase> proximityControllerBase =
        std::make_shared<ProximityControllerBase>("it is for test a long string", nullptr);
    proximityControllerBase->Enable();
    proximityControllerBase->InitProximitySensorUser();
    EXPECT_FALSE(proximityControllerBase->IsSupported());
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest051 function end!");
}

/**
 * @tc.name: ProximityControllerBaseTest052
 * @tc.desc: Test proximity controller base InitProximitySensorUser abnormal branch
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
HWTEST_F(ProximityControllerBaseTest, ProximityControllerBaseTest052, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest052 function start!");
    SensorInfo infos[] {{.sensorTypeId = SENSOR_TYPE_ID_MAX}};
    g_sensorInfo = infos;
    g_count = 1;
    std::shared_ptr<ProximityControllerBase> proximityControllerBase =
        std::make_shared<ProximityControllerBase>("it is for test a long string", nullptr);
    proximityControllerBase->Enable();
    proximityControllerBase->InitProximitySensorUser();
    EXPECT_FALSE(proximityControllerBase->IsSupported());
    POWER_HILOGI(LABEL_TEST, "ProximityControllerBaseTest052 function end!");
}
} // namespace