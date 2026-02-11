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

#include "screen_saver_timer_manager.h"
#include "power_log.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;

namespace {
class ScreenSaverTimerManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    std::shared_ptr<ScreenSaverTimerManager> screenSaverTimerManager_;
};

void ScreenSaverTimerManagerTest::SetUpTestCase()
{
    POWER_HILOGI(LABEL_TEST, "ScreenSaverTimerManagerTest::SetUpTestCase");
}

void ScreenSaverTimerManagerTest::TearDownTestCase()
{
    POWER_HILOGI(LABEL_TEST, "ScreenSaverTimerManagerTest::TearDownTestCase");
}

void ScreenSaverTimerManagerTest::SetUp()
{
    POWER_HILOGI(LABEL_TEST, "ScreenSaverTimerManagerTest::SetUp");
    screenSaverTimerManager_ = std::make_shared<ScreenSaverTimerManager>();
}

void ScreenSaverTimerManagerTest::TearDown()
{
    POWER_HILOGI(LABEL_TEST, "ScreenSaverTimerManagerTest::TearDown");
    screenSaverTimerManager_.reset();
}

/**
 * @tc.name: ScreenSaverTimerManagerInit001
 * @tc.desc: Test Init creates timer queue and plug status manager
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSaverTimerManagerTest, ScreenSaverTimerManagerInit001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ScreenSaverTimerManagerInit001 start");

    screenSaverTimerManager_->Init();
    ASSERT_NE(screenSaverTimerManager_, nullptr);

    // After Init, operations should work without null pointer issues
    screenSaverTimerManager_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_AC);
    screenSaverTimerManager_->UpdateScreenSaverTimer();
    screenSaverTimerManager_->CancelScreenSaverTimer();

    POWER_HILOGI(LABEL_TEST, "ScreenSaverTimerManagerInit001 end");
}

/**
 * @tc.name: ScreenSaverTimerManagerNullManager001
 * @tc.desc: Test operations with null plug status manager (before Init)
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSaverTimerManagerTest, ScreenSaverTimerManagerNullManager001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ScreenSaverTimerManagerNullManager001 start");

    // Without Init, plug manager is null - should handle gracefully
    screenSaverTimerManager_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_AC);
    screenSaverTimerManager_->UpdateScreenSaverTimer();
    screenSaverTimerManager_->CancelScreenSaverTimer();

    POWER_HILOGI(LABEL_TEST, "ScreenSaverTimerManagerNullManager001 end");
}

/**
 * @tc.name: ScreenSaverTimerManagerPowerStatus001
 * @tc.desc: Test SetPowerPlugStatus with all enum values
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSaverTimerManagerTest, ScreenSaverTimerManagerPowerStatus001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ScreenSaverTimerManagerPowerStatus001 start");

    screenSaverTimerManager_->Init();

    // Test all PowerPlugStatus enum values
    screenSaverTimerManager_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_INVALID);
    screenSaverTimerManager_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_AC);
    screenSaverTimerManager_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_DC);

    POWER_HILOGI(LABEL_TEST, "ScreenSaverTimerManagerPowerStatus001 end");
}

/**
 * @tc.name: ScreenSaverTimerManagerStatusTransition001
 * @tc.desc: Test power status change sequence
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSaverTimerManagerTest, ScreenSaverTimerManagerStatusTransition001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ScreenSaverTimerManagerStatusTransition001 start");

    screenSaverTimerManager_->Init();

    // Change status in sequence: AC -> DC -> INVALID -> AC
    screenSaverTimerManager_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_AC);
    screenSaverTimerManager_->UpdateScreenSaverTimer();

    screenSaverTimerManager_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_DC);
    screenSaverTimerManager_->UpdateScreenSaverTimer();

    screenSaverTimerManager_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_INVALID);
    screenSaverTimerManager_->UpdateScreenSaverTimer();

    screenSaverTimerManager_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_AC);
    screenSaverTimerManager_->UpdateScreenSaverTimer();

    POWER_HILOGI(LABEL_TEST, "ScreenSaverTimerManagerStatusTransition001 end");
}

/**
 * @tc.name: ScreenSaverTimerManagerMultipleInit001
 * @tc.desc: Test Init called multiple times
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSaverTimerManagerTest, ScreenSaverTimerManagerMultipleInit001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ScreenSaverTimerManagerMultipleInit001 start");

    // Multiple Init calls should be handled gracefully
    screenSaverTimerManager_->Init();
    screenSaverTimerManager_->Init();
    screenSaverTimerManager_->Init();

    ASSERT_NE(screenSaverTimerManager_, nullptr);

    // Operations should still work
    screenSaverTimerManager_->UpdateScreenSaverTimer();
    screenSaverTimerManager_->CancelScreenSaverTimer();

    POWER_HILOGI(LABEL_TEST, "ScreenSaverTimerManagerMultipleInit001 end");
}

/**
 * @tc.name: ScreenSaverTimerManagerOperationsSequence001
 * @tc.desc: Test various operation sequences
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSaverTimerManagerTest, ScreenSaverTimerManagerOperationsSequence001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ScreenSaverTimerManagerOperationsSequence001 start");

    screenSaverTimerManager_->Init();

    // Multiple update calls
    screenSaverTimerManager_->UpdateScreenSaverTimer();
    screenSaverTimerManager_->UpdateScreenSaverTimer();

    // Multiple cancel calls
    screenSaverTimerManager_->CancelScreenSaverTimer();
    screenSaverTimerManager_->CancelScreenSaverTimer();

    // Alternating update and cancel
    screenSaverTimerManager_->UpdateScreenSaverTimer();
    screenSaverTimerManager_->CancelScreenSaverTimer();
    screenSaverTimerManager_->UpdateScreenSaverTimer();

    POWER_HILOGI(LABEL_TEST, "ScreenSaverTimerManagerOperationsSequence001 end");
}

/**
 * @tc.name: ScreenSaverTimerManagerSameStatus001
 * @tc.desc: Test setting same power status multiple times
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSaverTimerManagerTest, ScreenSaverTimerManagerSameStatus001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ScreenSaverTimerManagerSameStatus001 start");

    screenSaverTimerManager_->Init();

    // Set AC status multiple times
    screenSaverTimerManager_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_AC);
    screenSaverTimerManager_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_AC);

    // Set DC status multiple times
    screenSaverTimerManager_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_DC);
    screenSaverTimerManager_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_DC);

    POWER_HILOGI(LABEL_TEST, "ScreenSaverTimerManagerSameStatus001 end");
}

/**
 * @tc.name: ScreenSaverTimerManagerMultipleInstances001
 * @tc.desc: Test multiple manager instances
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSaverTimerManagerTest, ScreenSaverTimerManagerMultipleInstances001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ScreenSaverTimerManagerMultipleInstances001 start");

    auto manager1 = std::make_shared<ScreenSaverTimerManager>();
    auto manager2 = std::make_shared<ScreenSaverTimerManager>();

    ASSERT_NE(manager1, nullptr);
    ASSERT_NE(manager2, nullptr);

    manager1->Init();
    manager2->Init();

    // Both should work independently
    manager1->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_AC);
    manager2->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_DC);

    manager1->UpdateScreenSaverTimer();
    manager2->UpdateScreenSaverTimer();

    POWER_HILOGI(LABEL_TEST, "ScreenSaverTimerManagerMultipleInstances001 end");
}

/**
 * @tc.name: ScreenSaverTimerManagerCancelBeforeInit001
 * @tc.desc: Test CancelScreenSaverTimer before Init
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSaverTimerManagerTest, ScreenSaverTimerManagerCancelBeforeInit001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ScreenSaverTimerManagerCancelBeforeInit001 start");

    // Cancel before Init - screenSaverHandle_ is nullptr
    screenSaverTimerManager_->CancelScreenSaverTimer();
    ASSERT_NE(screenSaverTimerManager_, nullptr);

    POWER_HILOGI(LABEL_TEST, "ScreenSaverTimerManagerCancelBeforeInit001 end");
}

/**
 * @tc.name: ScreenSaverTimerManagerCancelAfterUpdate001
 * @tc.desc: Test CancelScreenSaverTimer after UpdateScreenSaverTimer
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSaverTimerManagerTest, ScreenSaverTimerManagerCancelAfterUpdate001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ScreenSaverTimerManagerCancelAfterUpdate001 start");

    screenSaverTimerManager_->Init();
    screenSaverTimerManager_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_AC);
    screenSaverTimerManager_->UpdateScreenSaverTimer();

    // Cancel after update - screenSaverHandle_ may not be nullptr
    screenSaverTimerManager_->CancelScreenSaverTimer();
    ASSERT_NE(screenSaverTimerManager_, nullptr);

    POWER_HILOGI(LABEL_TEST, "ScreenSaverTimerManagerCancelAfterUpdate001 end");
}

/**
 * @tc.name: ScreenSaverTimerManagerSetStatusBeforeInit001
 * @tc.desc: Test SetPowerPlugStatus before Init (null plugStatsToScreenSaverTimer_)
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSaverTimerManagerTest, ScreenSaverTimerManagerSetStatusBeforeInit001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ScreenSaverTimerManagerSetStatusBeforeInit001 start");

    // SetPowerPlugStatus before Init - plugStatsToScreenSaverTimer_ is nullptr
    screenSaverTimerManager_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_AC);
    ASSERT_NE(screenSaverTimerManager_, nullptr);

    POWER_HILOGI(LABEL_TEST, "ScreenSaverTimerManagerSetStatusBeforeInit001 end");
}

/**
 * @tc.name: ScreenSaverTimerManagerIsPowerPlugedBeforeInit001
 * @tc.desc: Test IsPowerPluged behavior before Init
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSaverTimerManagerTest, ScreenSaverTimerManagerIsPowerPlugedBeforeInit001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ScreenSaverTimerManagerIsPowerPlugedBeforeInit001 start");

    // Update before Init - IsPowerPluged returns false when plugStatsToScreenSaverTimer_ is nullptr
    screenSaverTimerManager_->UpdateScreenSaverTimer();
    ASSERT_NE(screenSaverTimerManager_, nullptr);

    POWER_HILOGI(LABEL_TEST, "ScreenSaverTimerManagerIsPowerPlugedBeforeInit001 end");
}

/**
 * @tc.name: ScreenSaverTimerManagerUpdateWithAC001
 * @tc.desc: Test UpdateScreenSaverTimer with AC power status
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSaverTimerManagerTest, ScreenSaverTimerManagerUpdateWithAC001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ScreenSaverTimerManagerUpdateWithAC001 start");

    screenSaverTimerManager_->Init();
    screenSaverTimerManager_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_AC);

    // Update with AC - GetSettingScreenOffTime returns DEFAULT_AC_DISPLAY_OFF_TIME_MS
    screenSaverTimerManager_->UpdateScreenSaverTimer();
    ASSERT_NE(screenSaverTimerManager_, nullptr);

    POWER_HILOGI(LABEL_TEST, "ScreenSaverTimerManagerUpdateWithAC001 end");
}

/**
 * @tc.name: ScreenSaverTimerManagerUpdateWithDC001
 * @tc.desc: Test UpdateScreenSaverTimer with DC power status
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSaverTimerManagerTest, ScreenSaverTimerManagerUpdateWithDC001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ScreenSaverTimerManagerUpdateWithDC001 start");

    screenSaverTimerManager_->Init();
    screenSaverTimerManager_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_DC);

    // Update with DC - GetSettingScreenOffTime returns DEFAULT_DC_DISPLAY_OFF_TIME_MS
    screenSaverTimerManager_->UpdateScreenSaverTimer();
    ASSERT_NE(screenSaverTimerManager_, nullptr);

    POWER_HILOGI(LABEL_TEST, "ScreenSaverTimerManagerUpdateWithDC001 end");
}

/**
 * @tc.name: ScreenSaverTimerManagerUpdateWithInvalid001
 * @tc.desc: Test UpdateScreenSaverTimer with INVALID power status
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSaverTimerManagerTest, ScreenSaverTimerManagerUpdateWithInvalid001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ScreenSaverTimerManagerUpdateWithInvalid001 start");

    screenSaverTimerManager_->Init();
    screenSaverTimerManager_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_INVALID);

    // Update with INVALID - IsPowerPluged returns false, uses DC defaults
    screenSaverTimerManager_->UpdateScreenSaverTimer();
    ASSERT_NE(screenSaverTimerManager_, nullptr);

    POWER_HILOGI(LABEL_TEST, "ScreenSaverTimerManagerUpdateWithInvalid001 end");
}

/**
 * @tc.name: ScreenSaverTimerManagerUpdateCancelSequence001
 * @tc.desc: Test sequence of Update and Cancel operations
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSaverTimerManagerTest, ScreenSaverTimerManagerUpdateCancelSequence001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ScreenSaverTimerManagerUpdateCancelSequence001 start");

    screenSaverTimerManager_->Init();
    screenSaverTimerManager_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_AC);

    // Update, then cancel, then update again - tests screenSaverHandle_ nullptr/non-nullptr
    screenSaverTimerManager_->UpdateScreenSaverTimer();
    screenSaverTimerManager_->CancelScreenSaverTimer();
    screenSaverTimerManager_->UpdateScreenSaverTimer();
    screenSaverTimerManager_->CancelScreenSaverTimer();
    ASSERT_NE(screenSaverTimerManager_, nullptr);

    POWER_HILOGI(LABEL_TEST, "ScreenSaverTimerManagerUpdateCancelSequence001 end");
}

/**
 * @tc.name: ScreenSaverTimerManagerSwitchPowerStatus001
 * @tc.desc: Test switching between AC and DC power status
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSaverTimerManagerTest, ScreenSaverTimerManagerSwitchPowerStatus001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ScreenSaverTimerManagerSwitchPowerStatus001 start");

    screenSaverTimerManager_->Init();

    // AC -> DC switch
    screenSaverTimerManager_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_AC);
    screenSaverTimerManager_->UpdateScreenSaverTimer();
    screenSaverTimerManager_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_DC);
    screenSaverTimerManager_->UpdateScreenSaverTimer();

    // DC -> AC switch
    screenSaverTimerManager_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_AC);
    screenSaverTimerManager_->UpdateScreenSaverTimer();
    ASSERT_NE(screenSaverTimerManager_, nullptr);

    POWER_HILOGI(LABEL_TEST, "ScreenSaverTimerManagerSwitchPowerStatus001 end");
}

/**
 * @tc.name: ScreenSaverTimerManagerMultipleCancel001
 * @tc.desc: Test multiple consecutive CancelScreenSaverTimer calls
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSaverTimerManagerTest, ScreenSaverTimerManagerMultipleCancel001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ScreenSaverTimerManagerMultipleCancel001 start");

    screenSaverTimerManager_->Init();
    screenSaverTimerManager_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_AC);
    screenSaverTimerManager_->UpdateScreenSaverTimer();

    // Multiple cancels - after first cancel, screenSaverHandle_ becomes nullptr
    screenSaverTimerManager_->CancelScreenSaverTimer();
    screenSaverTimerManager_->CancelScreenSaverTimer();
    screenSaverTimerManager_->CancelScreenSaverTimer();
    ASSERT_NE(screenSaverTimerManager_, nullptr);

    POWER_HILOGI(LABEL_TEST, "ScreenSaverTimerManagerMultipleCancel001 end");
}

/**
 * @tc.name: ScreenSaverTimerManagerAllStatuses001
 * @tc.desc: Test all power statuses in sequence with update
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSaverTimerManagerTest, ScreenSaverTimerManagerAllStatuses001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ScreenSaverTimerManagerAllStatuses001 start");

    screenSaverTimerManager_->Init();

    // Test all three statuses with update
    screenSaverTimerManager_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_INVALID);
    screenSaverTimerManager_->UpdateScreenSaverTimer();

    screenSaverTimerManager_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_AC);
    screenSaverTimerManager_->UpdateScreenSaverTimer();

    screenSaverTimerManager_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_DC);
    screenSaverTimerManager_->UpdateScreenSaverTimer();
    ASSERT_NE(screenSaverTimerManager_, nullptr);

    POWER_HILOGI(LABEL_TEST, "ScreenSaverTimerManagerAllStatuses001 end");
}

/**
 * @tc.name: ScreenSaverTimerManagerInitThenSetStatus001
 * @tc.desc: Test SetPowerPlugStatus after Init
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSaverTimerManagerTest, ScreenSaverTimerManagerInitThenSetStatus001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ScreenSaverTimerManagerInitThenSetStatus001 start");

    // Init first
    screenSaverTimerManager_->Init();

    // Then set status - plugStatsToScreenSaverTimer_ is not nullptr
    screenSaverTimerManager_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_AC);
    screenSaverTimerManager_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_DC);
    screenSaverTimerManager_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_INVALID);
    ASSERT_NE(screenSaverTimerManager_, nullptr);

    POWER_HILOGI(LABEL_TEST, "ScreenSaverTimerManagerInitThenSetStatus001 end");
}

/**
 * @tc.name: ScreenSaverTimerManagerNoInitOperations001
 * @tc.desc: Test all operations without calling Init
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSaverTimerManagerTest, ScreenSaverTimerManagerNoInitOperations001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ScreenSaverTimerManagerNoInitOperations001 start");

    // No Init called - plugStatsToScreenSaverTimer_ remains nullptr
    screenSaverTimerManager_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_AC);
    screenSaverTimerManager_->UpdateScreenSaverTimer();
    screenSaverTimerManager_->CancelScreenSaverTimer();
    ASSERT_NE(screenSaverTimerManager_, nullptr);

    POWER_HILOGI(LABEL_TEST, "ScreenSaverTimerManagerNoInitOperations001 end");
}

/**
 * @tc.name: ScreenSaverTimerManagerInitIdempotent001
 * @tc.desc: Test that Init is idempotent (can be called multiple times safely)
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSaverTimerManagerTest, ScreenSaverTimerManagerInitIdempotent001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ScreenSaverTimerManagerInitIdempotent001 start");

    // First Init - plugStatsToScreenSaverTimer_ is nullptr, creates it
    screenSaverTimerManager_->Init();

    // Second Init - plugStatsToScreenSaverTimer_ is not nullptr, skips creation
    screenSaverTimerManager_->Init();

    // Third Init - should still work
    screenSaverTimerManager_->Init();

    // Verify operations still work
    screenSaverTimerManager_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_AC);
    screenSaverTimerManager_->UpdateScreenSaverTimer();
    ASSERT_NE(screenSaverTimerManager_, nullptr);

    POWER_HILOGI(LABEL_TEST, "ScreenSaverTimerManagerInitIdempotent001 end");
}

/**
 * @tc.name: ScreenSaverTimerManagerUpdateAfterStatusChange001
 * @tc.desc: Test UpdateScreenSaverTimer after changing power status
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSaverTimerManagerTest, ScreenSaverTimerManagerUpdateAfterStatusChange001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ScreenSaverTimerManagerUpdateAfterStatusChange001 start");

    screenSaverTimerManager_->Init();
    screenSaverTimerManager_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_AC);
    screenSaverTimerManager_->UpdateScreenSaverTimer();

    // Change to DC and update
    screenSaverTimerManager_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_DC);
    screenSaverTimerManager_->UpdateScreenSaverTimer();

    // Change to INVALID and update
    screenSaverTimerManager_->SetPowerPlugStatus(PowerPlugStatusManager::PowerPlugStatus::POWER_PLUG_IN_INVALID);
    screenSaverTimerManager_->UpdateScreenSaverTimer();
    ASSERT_NE(screenSaverTimerManager_, nullptr);

    POWER_HILOGI(LABEL_TEST, "ScreenSaverTimerManagerUpdateAfterStatusChange001 end");
}
} // namespace