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

#include "power_screen_saver.h"
#include "power_log.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;

namespace {
class PowerScreenSaverTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    std::shared_ptr<PowerScreenSaver> powerScreenSaver_;
};

void PowerScreenSaverTest::SetUpTestCase()
{
    POWER_HILOGI(LABEL_TEST, "PowerScreenSaverTest::SetUpTestCase");
}

void PowerScreenSaverTest::TearDownTestCase()
{
    POWER_HILOGI(LABEL_TEST, "PowerScreenSaverTest::TearDownTestCase");
}

void PowerScreenSaverTest::SetUp()
{
    POWER_HILOGI(LABEL_TEST, "PowerScreenSaverTest::SetUp");
    powerScreenSaver_ = std::make_shared<PowerScreenSaver>();
}

void PowerScreenSaverTest::TearDown()
{
    POWER_HILOGI(LABEL_TEST, "PowerScreenSaverTest::TearDown");
    powerScreenSaver_.reset();
}

/**
 * @tc.name: PowerScreenSaverInit001
 * @tc.desc: Test PowerScreenSaver construction and null timer manager handling
 * @tc.type: FUNC
 */
HWTEST_F(PowerScreenSaverTest, PowerScreenSaverInit001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerScreenSaverInit001 start");

    ASSERT_NE(powerScreenSaver_, nullptr);

    // Methods should handle null timer manager gracefully without crash
    powerScreenSaver_->UpdateScreenSaverTimer();
    powerScreenSaver_->CancelScreenSaverTimer();
    powerScreenSaver_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_AC);
    powerScreenSaver_->RegisterSettingScreenSaverObservers();

    POWER_HILOGI(LABEL_TEST, "PowerScreenSaverInit001 end");
}

/**
 * @tc.name: PowerScreenSaverSetPowerStatus001
 * @tc.desc: Test SetPowerPlugStatus with all enum values
 * @tc.type: FUNC
 */
HWTEST_F(PowerScreenSaverTest, PowerScreenSaverSetPowerStatus001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerScreenSaverSetPowerStatus001 start");

    // Test all PowerPlugStatus enum values - should not crash with null timer manager
    EXPECT_TRUE(powerScreenSaver_ != nullptr);
    powerScreenSaver_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_INVALID);
    powerScreenSaver_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_AC);
    powerScreenSaver_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_DC);

    POWER_HILOGI(LABEL_TEST, "PowerScreenSaverSetPowerStatus001 end");
}

/**
 * @tc.name: PowerScreenSaverMultipleOperations001
 * @tc.desc: Test multiple operations in sequence
 * @tc.type: FUNC
 */
HWTEST_F(PowerScreenSaverTest, PowerScreenSaverMultipleOperations001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerScreenSaverMultipleOperations001 start");

    // Multiple update calls
    powerScreenSaver_->UpdateScreenSaverTimer();
    powerScreenSaver_->UpdateScreenSaverTimer();

    // Multiple cancel calls
    powerScreenSaver_->CancelScreenSaverTimer();
    powerScreenSaver_->CancelScreenSaverTimer();

    // Alternating operations
    powerScreenSaver_->UpdateScreenSaverTimer();
    powerScreenSaver_->CancelScreenSaverTimer();
    powerScreenSaver_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_AC);
    EXPECT_TRUE(powerScreenSaver_ != nullptr);

    POWER_HILOGI(LABEL_TEST, "PowerScreenSaverMultipleOperations001 end");
}

/**
 * @tc.name: PowerScreenSaverRegisterObservers001
 * @tc.desc: Test RegisterSettingScreenSaverObservers
 * @tc.type: FUNC
 */
HWTEST_F(PowerScreenSaverTest, PowerScreenSaverRegisterObservers001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerScreenSaverRegisterObservers001 start");

    // Should handle gracefully without SettingProvider
    powerScreenSaver_->RegisterSettingScreenSaverObservers();
    EXPECT_TRUE(powerScreenSaver_ != nullptr);

    POWER_HILOGI(LABEL_TEST, "PowerScreenSaverRegisterObservers001 end");
}

/**
 * @tc.name: PowerScreenSaverMultipleInstances001
 * @tc.desc: Test multiple PowerScreenSaver instances
 * @tc.type: FUNC
 */
HWTEST_F(PowerScreenSaverTest, PowerScreenSaverMultipleInstances001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerScreenSaverMultipleInstances001 start");

    auto screenSaver1 = std::make_shared<PowerScreenSaver>();
    auto screenSaver2 = std::make_shared<PowerScreenSaver>();

    ASSERT_NE(screenSaver1, nullptr);
    ASSERT_NE(screenSaver2, nullptr);

    // Both should handle null timer manager
    screenSaver1->UpdateScreenSaverTimer();
    screenSaver2->UpdateScreenSaverTimer();

    POWER_HILOGI(LABEL_TEST, "PowerScreenSaverMultipleInstances001 end");
}

} // namespace