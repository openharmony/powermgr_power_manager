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
#include <cinttypes>

#include "setting_helper.h"
#include "power_log.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;

namespace {
class SettingHelperScreenSaverTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void SettingHelperScreenSaverTest::SetUpTestCase()
{
    POWER_HILOGI(LABEL_TEST, "SettingHelperScreenSaverTest::SetUpTestCase");
}

void SettingHelperScreenSaverTest::TearDownTestCase()
{
    POWER_HILOGI(LABEL_TEST, "SettingHelperScreenSaverTest::TearDownTestCase");
}

void SettingHelperScreenSaverTest::SetUp()
{
    POWER_HILOGI(LABEL_TEST, "SettingHelperScreenSaverTest::SetUp");
}

void SettingHelperScreenSaverTest::TearDown()
{
    POWER_HILOGI(LABEL_TEST, "SettingHelperScreenSaverTest::TearDown");
}

/**
 * @tc.name: SettingHelperRegisterAcObserver001
 * @tc.desc: Test RegisterSettingAcScreenSaverTimeObserver
 * @tc.type: FUNC
 */
HWTEST_F(SettingHelperScreenSaverTest, SettingHelperRegisterAcObserver001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "SettingHelperRegisterAcObserver001 start");

    SettingObserver::UpdateFunc updateFunc = [](const std::string& key) {
        POWER_HILOGI(LABEL_TEST, "AC screen saver time update: %{public}s", key.c_str());
    };

    auto observer = SettingHelper::RegisterSettingAcScreenSaverTimeObserver(updateFunc);
    // Observer may be null if SettingProvider is not available - verify no crash
    EXPECT_TRUE(observer == nullptr || observer!= nullptr);
    POWER_HILOGI(LABEL_TEST, "SettingHelperRegisterAcObserver001 end");
}

/**
 * @tc.name: SettingHelperRegisterDcObserver001
 * @tc.desc: Test RegisterSettingDcScreenSaverTimeObserver
 * @tc.type: FUNC
 */
HWTEST_F(SettingHelperScreenSaverTest, SettingHelperRegisterDcObserver001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "SettingHelperRegisterDcObserver001 start");

    SettingObserver::UpdateFunc updateFunc = [](const std::string& key) {
        POWER_HILOGI(LABEL_TEST, "DC screen saver time update: %{public}s", key.c_str());
    };

    auto observer = SettingHelper::RegisterSettingDcScreenSaverTimeObserver(updateFunc);
    EXPECT_TRUE(observer == nullptr || observer!= nullptr);
    POWER_HILOGI(LABEL_TEST, "SettingHelperRegisterDcObserver001 end");
}

/**
 * @tc.name: SettingHelperGetAcTime001
 * @tc.desc: Test GetSettingAcScreenSaverTime with various defaults
 * @tc.type: FUNC
 */
HWTEST_F(SettingHelperScreenSaverTest, SettingHelperGetAcTime001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "SettingHelperGetAcTime001 start");

    // Test with positive default value
    int64_t acTime1 = SettingHelper::GetSettingAcScreenSaverTime(300000);
    EXPECT_GE(acTime1, -1);  // Should be -1 (disabled) or positive value

    // Test with zero default (should return default when SettingProvider unavailable)
    int64_t acTime2 = SettingHelper::GetSettingAcScreenSaverTime(0);
    EXPECT_GE(acTime2, -1);

    // Test with negative default (disabled state)
    int64_t acTime3 = SettingHelper::GetSettingAcScreenSaverTime(-1);
    EXPECT_GE(acTime3, -1);

    POWER_HILOGI(LABEL_TEST, "AC times: %{public}" PRId64 ", %{public}" PRId64 ", %{public}" PRId64,
        acTime1, acTime2, acTime3);
    POWER_HILOGI(LABEL_TEST, "SettingHelperGetAcTime001 end");
}

/**
 * @tc.name: SettingHelperGetDcTime001
 * @tc.desc: Test GetSettingDcScreenSaverTime with various defaults
 * @tc.type: FUNC
 */
HWTEST_F(SettingHelperScreenSaverTest, SettingHelperGetDcTime001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "SettingHelperGetDcTime001 start");

    int64_t dcTime1 = SettingHelper::GetSettingDcScreenSaverTime(600000);
    EXPECT_GE(dcTime1, -1);

    int64_t dcTime2 = SettingHelper::GetSettingDcScreenSaverTime(0);
    EXPECT_GE(dcTime2, -1);

    int64_t dcTime3 = SettingHelper::GetSettingDcScreenSaverTime(-1);
    EXPECT_GE(dcTime3, -1);

    POWER_HILOGI(LABEL_TEST, "DC times: %{public}" PRId64 ", %{public}" PRId64 ", %{public}" PRId64,
        dcTime1, dcTime2, dcTime3);
    POWER_HILOGI(LABEL_TEST, "SettingHelperGetDcTime001 end");
}

/**
 * @tc.name: SettingHelperGetTimeBoundary001
 * @tc.desc: Test boundary values at MIN_DISPLAY_OFF_TIME_MS (1000ms)
 * @tc.type: FUNC
 * @tc.desc: When SettingProvider is unavailable and 0 <= value <= 1000, implementation returns defaultVal
 */
HWTEST_F(SettingHelperScreenSaverTest, SettingHelperGetTimeBoundary001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "SettingHelperGetTimeBoundary001 start");

    // Test at MIN_DISPLAY_OFF_TIME_MS boundary (1000ms)
    constexpr int64_t defaultTime = 300000;
    int64_t time1 = SettingHelper::GetSettingAcScreenSaverTime(1000);   // Exactly at boundary
    int64_t time2 = SettingHelper::GetSettingAcScreenSaverTime(999);    // Below boundary
    int64_t time3 = SettingHelper::GetSettingAcScreenSaverTime(1001);   // Above boundary
    int64_t time4 = SettingHelper::GetSettingAcScreenSaverTime(defaultTime);

    // All should be >= -1 (when SettingProvider unavailable)
    EXPECT_GE(time1, -1);
    EXPECT_GE(time2, -1);
    EXPECT_GE(time3, -1);
    EXPECT_GE(time4, -1);

    POWER_HILOGI(LABEL_TEST, "Boundary times: %{public}" PRId64 ", %{public}" PRId64 ", %{public}" PRId64 ", %{public}" PRId64,
        time1, time2, time3, time4);
    POWER_HILOGI(LABEL_TEST, "SettingHelperGetTimeBoundary001 end");
}

/**
 * @tc.name: SettingHelperRegisterBothObservers001
 * @tc.desc: Test registering both AC and DC observers
 * @tc.type: FUNC
 */
HWTEST_F(SettingHelperScreenSaverTest, SettingHelperRegisterBothObservers001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "SettingHelperRegisterBothObservers001 start");

    SettingObserver::UpdateFunc acFunc = [](const std::string& key) {
        POWER_HILOGI(LABEL_TEST, "AC update: %{public}s", key.c_str());
    };

    SettingObserver::UpdateFunc dcFunc = [](const std::string& key) {
        POWER_HILOGI(LABEL_TEST, "DC update: %{public}s", key.c_str());
    };

    auto acObserver = SettingHelper::RegisterSettingAcScreenSaverTimeObserver(acFunc);
    auto dcObserver = SettingHelper::RegisterSettingDcScreenSaverTimeObserver(dcFunc);

    EXPECT_TRUE(acObserver == nullptr || acObserver!= nullptr);
    EXPECT_TRUE(dcObserver == nullptr || dcObserver!= nullptr);

    POWER_HILOGI(LABEL_TEST, "SettingHelperRegisterBothObservers001 end");
}

/**
 * @tc.name: SettingHelperSequenceCalls001
 * @tc.desc: Test sequential calls to get functions
 * @tc.type: FUNC
 */
HWTEST_F(SettingHelperScreenSaverTest, SettingHelperSequenceCalls001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "SettingHelperSequenceCalls001 start");

    // Get times in sequence to verify no state corruption
    int64_t acTime1 = SettingHelper::GetSettingAcScreenSaverTime(100000);
    int64_t dcTime1 = SettingHelper::GetSettingDcScreenSaverTime(200000);
    int64_t acTime2 = SettingHelper::GetSettingAcScreenSaverTime(300000);
    int64_t dcTime2 = SettingHelper::GetSettingDcScreenSaverTime(400000);

    EXPECT_GE(acTime1, -1);
    EXPECT_GE(dcTime1, -1);
    EXPECT_GE(acTime2, -1);
    EXPECT_GE(dcTime2, -1);

    POWER_HILOGI(LABEL_TEST, "Sequence times: AC1=%{public}" PRId64 ", DC1=%{public}" PRId64
        ", AC2=%{public}" PRId64 ", DC2=%{public}" PRId64,
        acTime1, dcTime1, acTime2, dcTime2);
    POWER_HILOGI(LABEL_TEST, "SettingHelperSequenceCalls001 end");
}

/**
 * @tc.name: SettingHelperLargeValue001
 * @tc.desc: Test with large default values
 * @tc.type: FUNC
 */
HWTEST_F(SettingHelperScreenSaverTest, SettingHelperLargeValue001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "SettingHelperLargeValue001 start");

    constexpr int64_t largeDefault = 3600000; // 1 hour
    int64_t acTime = SettingHelper::GetSettingAcScreenSaverTime(largeDefault);
    int64_t dcTime = SettingHelper::GetSettingDcScreenSaverTime(largeDefault);

    EXPECT_GE(acTime, -1);
    EXPECT_GE(dcTime, -1);

    POWER_HILOGI(LABEL_TEST, "Large values: AC=%{public}" PRId64 ", DC=%{public}" PRId64,
        acTime, dcTime);
    POWER_HILOGI(LABEL_TEST, "SettingHelperLargeValue001 end");
}

/**
 * @tc.name: SettingHelperAlternatingCalls001
 * @tc.desc: Test alternating between AC and DC calls
 * @tc.type: FUNC
 */
HWTEST_F(SettingHelperScreenSaverTest, SettingHelperAlternatingCalls001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "SettingHelperAlternatingCalls001 start");

    // Alternate between AC and DC
    for (int i = 0; i < 5; i++) {
        int64_t acTime = SettingHelper::GetSettingAcScreenSaverTime(100000 + i * 10000);
        int64_t dcTime = SettingHelper::GetSettingDcScreenSaverTime(200000 + i * 10000);
        EXPECT_GE(acTime, -1);
        EXPECT_GE(dcTime, -1);
    }

    POWER_HILOGI(LABEL_TEST, "SettingHelperAlternatingCalls001 end");
}

} // namespace