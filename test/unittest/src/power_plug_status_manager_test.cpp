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
#include <memory>

#include "power_plug_status_manager.h"
#include "power_log.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;

namespace {
class PowerPlugStatusManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    std::shared_ptr<PowerPlugStatusManager> powerPlugStatusManager_;
};

void PowerPlugStatusManagerTest::SetUpTestCase()
{
    POWER_HILOGI(LABEL_TEST, "PowerPlugStatusManagerTest::SetUpTestCase");
}

void PowerPlugStatusManagerTest::TearDownTestCase()
{
    POWER_HILOGI(LABEL_TEST, "PowerPlugStatusManagerTest::TearDownTestCase");
}

void PowerPlugStatusManagerTest::SetUp()
{
    POWER_HILOGI(LABEL_TEST, "PowerPlugStatusManagerTest::SetUp");
    powerPlugStatusManager_ = std::make_shared<PowerPlugStatusManager>();
}

void PowerPlugStatusManagerTest::TearDown()
{
    POWER_HILOGI(LABEL_TEST, "PowerPlugStatusManagerTest::TearDown");
    powerPlugStatusManager_.reset();
}

/**
 * @tc.name: PowerPlugStatusManagerInit001
 * @tc.desc: Test PowerPlugStatusManager::Init without battery service
 * @tc.type: FUNC
 */
HWTEST_F(PowerPlugStatusManagerTest, PowerPlugStatusManagerInit001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerPlugStatusManagerInit001 start");

    // Init without battery service - should initialize to INVALID
    powerPlugStatusManager_->Init();

    // After Init, status should be set (without battery service, remains INVALID)
    powerPlugStatusManager_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_DC);
    EXPECT_FALSE(powerPlugStatusManager_->IsPowerPluged());

    POWER_HILOGI(LABEL_TEST, "PowerPlugStatusManagerInit001 end");
}

/**
 * @tc.name: PowerPlugStatusManagerSetStatus001
 * @tc.desc: Test SetPowerPlugStatus with INVALID status
 * @tc.type: FUNC
 */
HWTEST_F(PowerPlugStatusManagerTest, PowerPlugStatusManagerSetStatus001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerPlugStatusManagerSetStatus001 start");

    powerPlugStatusManager_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_INVALID);
    // After setting to INVALID, IsPowerPluged should return false
    EXPECT_FALSE(powerPlugStatusManager_->IsPowerPluged());

    POWER_HILOGI(LABEL_TEST, "PowerPlugStatusManagerSetStatus001 end");
}

/**
 * @tc.name: PowerPlugStatusManagerSetStatus002
 * @tc.desc: Test SetPowerPlugStatus with AC status
 * @tc.type: FUNC
 */
HWTEST_F(PowerPlugStatusManagerTest, PowerPlugStatusManagerSetStatus002, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerPlugStatusManagerSetStatus002 start");

    powerPlugStatusManager_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_AC);
    // After setting to AC, IsPowerPluged should return true
    EXPECT_TRUE(powerPlugStatusManager_->IsPowerPluged());

    POWER_HILOGI(LABEL_TEST, "PowerPlugStatusManagerSetStatus002 end");
}

/**
 * @tc.name: PowerPlugStatusManagerSetStatus003
 * @tc.desc: Test SetPowerPlugStatus with DC status
 * @tc.type: FUNC
 */
HWTEST_F(PowerPlugStatusManagerTest, PowerPlugStatusManagerSetStatus003, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerPlugStatusManagerSetStatus003 start");

    powerPlugStatusManager_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_DC);
    // After setting to DC, IsPowerPluged should return false
    EXPECT_FALSE(powerPlugStatusManager_->IsPowerPluged());

    POWER_HILOGI(LABEL_TEST, "PowerPlugStatusManagerSetStatus003 end");
}

/**
 * @tc.name: PowerPlugStatusManagerIsPowerPluged001
 * @tc.desc: Test IsPowerPluged returns correct value for AC status
 * @tc.type: FUNC
 */
HWTEST_F(PowerPlugStatusManagerTest, PowerPlugStatusManagerIsPowerPluged001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerPlugStatusManagerIsPowerPluged001 start");

    powerPlugStatusManager_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_AC);
    bool isPlugged = powerPlugStatusManager_->IsPowerPluged();
    EXPECT_TRUE(isPlugged);

    POWER_HILOGI(LABEL_TEST, "PowerPlugStatusManagerIsPowerPluged001 end");
}

/**
 * @tc.name: PowerPlugStatusManagerIsPowerPluged002
 * @tc.desc: Test IsPowerPluged returns correct value for DC status
 * @tc.type: FUNC
 */
HWTEST_F(PowerPlugStatusManagerTest, PowerPlugStatusManagerIsPowerPluged002, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerPlugStatusManagerIsPowerPluged002 start");

    powerPlugStatusManager_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_DC);
    bool isPlugged = powerPlugStatusManager_->IsPowerPluged();
    EXPECT_FALSE(isPlugged);

    POWER_HILOGI(LABEL_TEST, "PowerPlugStatusManagerIsPowerPluged002 end");
}

/**
 * @tc.name: PowerPlugStatusManagerIsPowerPluged003
 * @tc.desc: Test IsPowerPluged returns correct value for INVALID status
 * @tc.type: FUNC
 */
HWTEST_F(PowerPlugStatusManagerTest, PowerPlugStatusManagerIsPowerPluged003, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerPlugStatusManagerIsPowerPluged003 start");

    powerPlugStatusManager_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_INVALID);
    bool isPlugged = powerPlugStatusManager_->IsPowerPluged();
    EXPECT_FALSE(isPlugged);

    POWER_HILOGI(LABEL_TEST, "PowerPlugStatusManagerIsPowerPluged003 end");
}

/**
 * @tc.name: PowerPlugStatusManagerStatusTransition001
 * @tc.desc: Test status transition from INVALID to AC
 * @tc.type: FUNC
 */
HWTEST_F(PowerPlugStatusManagerTest, PowerPlugStatusManagerStatusTransition001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerPlugStatusManagerStatusTransition001 start");

    powerPlugStatusManager_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_INVALID);
    EXPECT_FALSE(powerPlugStatusManager_->IsPowerPluged());

    powerPlugStatusManager_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_AC);
    EXPECT_TRUE(powerPlugStatusManager_->IsPowerPluged());

    POWER_HILOGI(LABEL_TEST, "PowerPlugStatusManagerStatusTransition001 end");
}

/**
 * @tc.name: PowerPlugStatusManagerStatusTransition002
 * @tc.desc: Test status transition from AC to DC
 * @tc.type: FUNC
 */
HWTEST_F(PowerPlugStatusManagerTest, PowerPlugStatusManagerStatusTransition002, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerPlugStatusManagerStatusTransition002 start");

    powerPlugStatusManager_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_AC);
    EXPECT_TRUE(powerPlugStatusManager_->IsPowerPluged());

    powerPlugStatusManager_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_DC);
    EXPECT_FALSE(powerPlugStatusManager_->IsPowerPluged());

    POWER_HILOGI(LABEL_TEST, "PowerPlugStatusManagerStatusTransition002 end");
}

/**
 * @tc.name: PowerPlugStatusManagerStatusTransition003
 * @tc.desc: Test status transition from DC to AC
 * @tc.type: FUNC
 */
HWTEST_F(PowerPlugStatusManagerTest, PowerPlugStatusManagerStatusTransition003, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerPlugStatusManagerStatusTransition003 start");

    powerPlugStatusManager_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_DC);
    EXPECT_FALSE(powerPlugStatusManager_->IsPowerPluged());

    powerPlugStatusManager_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_AC);
    EXPECT_TRUE(powerPlugStatusManager_->IsPowerPluged());

    POWER_HILOGI(LABEL_TEST, "PowerPlugStatusManagerStatusTransition003 end");
}

/**
 * @tc.name: PowerPlugStatusManagerMultipleStatusChanges001
 * @tc.desc: Test multiple status changes in sequence
 * @tc.type: FUNC
 */
HWTEST_F(PowerPlugStatusManagerTest, PowerPlugStatusManagerMultipleStatusChanges001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerPlugStatusManagerMultipleStatusChanges001 start");

    // Sequence: INVALID -> AC -> DC -> AC -> INVALID
    powerPlugStatusManager_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_INVALID);
    EXPECT_FALSE(powerPlugStatusManager_->IsPowerPluged());

    powerPlugStatusManager_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_AC);
    EXPECT_TRUE(powerPlugStatusManager_->IsPowerPluged());

    powerPlugStatusManager_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_DC);
    EXPECT_FALSE(powerPlugStatusManager_->IsPowerPluged());

    powerPlugStatusManager_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_AC);
    EXPECT_TRUE(powerPlugStatusManager_->IsPowerPluged());

    powerPlugStatusManager_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_INVALID);
    EXPECT_FALSE(powerPlugStatusManager_->IsPowerPluged());

    POWER_HILOGI(LABEL_TEST, "PowerPlugStatusManagerMultipleStatusChanges001 end");
}

/**
 * @tc.name: PowerPlugStatusManagerSameStatusSet001
 * @tc.desc: Test setting the same status multiple times
 * @tc.type: FUNC
 */
HWTEST_F(PowerPlugStatusManagerTest, PowerPlugStatusManagerSameStatusSet001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerPlugStatusManagerSameStatusSet001 start");

    // Set AC status multiple times
    powerPlugStatusManager_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_AC);
    EXPECT_TRUE(powerPlugStatusManager_->IsPowerPluged());

    powerPlugStatusManager_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_AC);
    EXPECT_TRUE(powerPlugStatusManager_->IsPowerPluged());

    powerPlugStatusManager_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_AC);
    EXPECT_TRUE(powerPlugStatusManager_->IsPowerPluged());

    POWER_HILOGI(LABEL_TEST, "PowerPlugStatusManagerSameStatusSet001 end");
}

/**
 * @tc.name: PowerPlugStatusManagerSameStatusSet002
 * @tc.desc: Test setting DC status multiple times
 * @tc.type: FUNC
 */
HWTEST_F(PowerPlugStatusManagerTest, PowerPlugStatusManagerSameStatusSet002, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerPlugStatusManagerSameStatusSet002 start");

    // Set DC status multiple times
    powerPlugStatusManager_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_DC);
    EXPECT_FALSE(powerPlugStatusManager_->IsPowerPluged());

    powerPlugStatusManager_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_DC);
    EXPECT_FALSE(powerPlugStatusManager_->IsPowerPluged());

    powerPlugStatusManager_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_DC);
    EXPECT_FALSE(powerPlugStatusManager_->IsPowerPluged());

    POWER_HILOGI(LABEL_TEST, "PowerPlugStatusManagerSameStatusSet002 end");
}

/**
 * @tc.name: PowerPlugStatusManagerEnumValues001
 * @tc.desc: Test all enum values are correctly defined
 * @tc.type: FUNC
 */
HWTEST_F(PowerPlugStatusManagerTest, PowerPlugStatusManagerEnumValues001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerPlugStatusManagerEnumValues001 start");

    // Verify enum values
    EXPECT_EQ(static_cast<int32_t>(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_INVALID), -1);
    EXPECT_EQ(static_cast<int32_t>(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_AC), 0);
    EXPECT_EQ(static_cast<int32_t>(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_DC), 1);

    POWER_HILOGI(LABEL_TEST, "PowerPlugStatusManagerEnumValues001 end");
}

/**
 * @tc.name: PowerPlugStatusManagerDefaultStatus001
 * @tc.desc: Test default status is INVALID
 * @tc.type: FUNC
 */
HWTEST_F(PowerPlugStatusManagerTest, PowerPlugStatusManagerDefaultStatus001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerPlugStatusManagerDefaultStatus001 start");

    // Create new instance without calling Init or SetPowerPlugStatus
    auto manager = std::make_shared<PowerPlugStatusManager>();
    // Default status should be INVALID
    EXPECT_FALSE(manager->IsPowerPluged());

    POWER_HILOGI(LABEL_TEST, "PowerPlugStatusManagerDefaultStatus001 end");
}

/**
 * @tc.name: PowerPlugStatusManagerAlternatingStatus001
 * @tc.desc: Test alternating between AC and DC status
 * @tc.type: FUNC
 */
HWTEST_F(PowerPlugStatusManagerTest, PowerPlugStatusManagerAlternatingStatus001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerPlugStatusManagerAlternatingStatus001 start");

    // Alternate between AC and DC multiple times
    for (int i = 0; i < 10; i++) {
        if (i % 2 == 0) {
            powerPlugStatusManager_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_AC);
            EXPECT_TRUE(powerPlugStatusManager_->IsPowerPluged());
        } else {
            powerPlugStatusManager_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_DC);
            EXPECT_FALSE(powerPlugStatusManager_->IsPowerPluged());
        }
    }

    POWER_HILOGI(LABEL_TEST, "PowerPlugStatusManagerAlternatingStatus001 end");
}

} // namespace