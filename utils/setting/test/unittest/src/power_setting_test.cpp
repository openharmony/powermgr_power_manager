/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include <mutex>
#include <condition_variable>
#include "power_setting_helper.h"
#include "power_setting_observer.h"
#include "power_log.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::PowerMgr;

namespace {
static const std::string TEST_KEY = "settings.power.test";
static const std::string INVALID_TEST_KEY = "settings.power.invalid";
static const std::string STRING_VALUE = "test";
static const std::string INVALID_STRING_VALUE = "-1";
constexpr int32_t INT_VALUE = 9527;
constexpr int32_t INVALID_INT_VALUE = -1;
constexpr int64_t LONG_VALUE = 952795279527L;
constexpr int64_t INVALID_LONG_VALUE = -1L;
constexpr bool BOOL_VALUE = true;
constexpr bool INVALID_BOOL_VALUE = false;

constexpr int32_t POWER_SA_ID = 3301;
PowerSettingHelper& g_settingHelper = PowerSettingHelper::GetInstance(POWER_SA_ID);
}

class PowerSettingTest : public Test {
public:
    void SetUp() override {}

    void TearDown() override {}
};

namespace {
/**
 * @tc.name: PowerSettingTest001
 * @tc.desc: Test get power setting invalid key
 * @tc.type: FUNC
 */
HWTEST_F(PowerSettingTest, PowerSettingTest001, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerSettingTest001 fun is start");
    std::string valueStr = INVALID_STRING_VALUE;
    ErrCode ret = g_settingHelper.GetStringValue(INVALID_TEST_KEY, valueStr);
    EXPECT_EQ(ret, ERR_NAME_NOT_FOUND);

    int32_t valueInt = INVALID_INT_VALUE;
    ret = g_settingHelper.GetIntValue(INVALID_TEST_KEY, valueInt);
    EXPECT_EQ(ret, ERR_NAME_NOT_FOUND);

    int64_t valueLong = INVALID_LONG_VALUE;
    ret = g_settingHelper.GetLongValue(INVALID_TEST_KEY, valueLong);
    EXPECT_EQ(ret, ERR_NAME_NOT_FOUND);

    bool valueBool = INVALID_BOOL_VALUE;
    ret = g_settingHelper.GetBoolValue(INVALID_TEST_KEY, valueBool);
    EXPECT_EQ(ret, ERR_NAME_NOT_FOUND);
    POWER_HILOGI(LABEL_TEST, "PowerSettingTest001 fun is end");
}

/**
 * @tc.name: PowerSettingTest002
 * @tc.desc: Test put and get power setting string value
 * @tc.type: FUNC
 */
HWTEST_F(PowerSettingTest, PowerSettingTest002, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerSettingTest002 fun is start");
    std::string value = STRING_VALUE;
    ErrCode ret = g_settingHelper.PutStringValue(TEST_KEY, value);
    EXPECT_EQ(ret, ERR_OK);

    std::string valueGet;
    ret = g_settingHelper.GetStringValue(TEST_KEY, valueGet);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(valueGet, STRING_VALUE);
    POWER_HILOGI(LABEL_TEST, "PowerSettingTest002 fun is end");
}

/**
 * @tc.name: PowerSettingTest003
 * @tc.desc: Test put and get power setting int value
 * @tc.type: FUNC
 */
HWTEST_F(PowerSettingTest, PowerSettingTest003, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerSettingTest003 fun is start");
    int32_t value = INT_VALUE;
    ErrCode ret = g_settingHelper.PutIntValue(TEST_KEY, value);
    EXPECT_EQ(ret, ERR_OK);

    int32_t valueGet;
    ret = g_settingHelper.GetIntValue(TEST_KEY, valueGet);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(valueGet, INT_VALUE);
    POWER_HILOGI(LABEL_TEST, "PowerSettingTest003 fun is end");
}

/**
 * @tc.name: PowerSettingTest004
 * @tc.desc: Test put and get power setting long value
 * @tc.type: FUNC
 */
HWTEST_F(PowerSettingTest, PowerSettingTest004, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerSettingTest004 fun is start");
    int64_t value = LONG_VALUE;
    ErrCode ret = g_settingHelper.PutLongValue(TEST_KEY, value);
    EXPECT_EQ(ret, ERR_OK);

    int64_t valueGet;
    ret = g_settingHelper.GetLongValue(TEST_KEY, valueGet);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(valueGet, LONG_VALUE);
    POWER_HILOGI(LABEL_TEST, "PowerSettingTest004 fun is end");
}

/**
 * @tc.name: PowerSettingTest005
 * @tc.desc: Test put and get power setting bool value
 * @tc.type: FUNC
 */
HWTEST_F(PowerSettingTest, PowerSettingTest005, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerSettingTest005 fun is start");
    bool value = BOOL_VALUE;
    ErrCode ret = g_settingHelper.PutBoolValue(TEST_KEY, value);
    EXPECT_EQ(ret, ERR_OK);

    bool valueGet;
    ret = g_settingHelper.GetBoolValue(TEST_KEY, valueGet);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(valueGet, BOOL_VALUE);
    POWER_HILOGI(LABEL_TEST, "PowerSettingTest005 fun is end");
}

/**
 * @tc.name: PowerSettingTest006
 * @tc.desc: Test create power setting observer
 * @tc.type: FUNC
 */
HWTEST_F(PowerSettingTest, PowerSettingTest006, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerSettingTest006 fun is start");
    PowerSettingObserver::UpdateFunc func = [](const std::string& key) {};
    auto observer = g_settingHelper.CreateObserver(TEST_KEY, func);
    EXPECT_EQ(observer->GetKey(), TEST_KEY);
    POWER_HILOGI(LABEL_TEST, "PowerSettingTest006 fun is end");
}

/**
 * @tc.name: PowerSettingTest007
 * @tc.desc: Test register power setting observer
 * @tc.type: FUNC
 */
HWTEST_F(PowerSettingTest, PowerSettingTest007, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerSettingTest007 fun is start");
    static std::condition_variable cv;
    PowerSettingObserver::UpdateFunc observerFunc = [&](const std::string& key) {
        POWER_HILOGI(LABEL_TEST, "observer callback enter, key=%{public}s", key.c_str());
        std::string valueGet;
        ErrCode ret = g_settingHelper.GetStringValue(key, valueGet);
        EXPECT_EQ(ret, ERR_OK);
        EXPECT_EQ(valueGet, STRING_VALUE);
        cv.notify_all();
    };

    auto observer = g_settingHelper.CreateObserver(TEST_KEY, observerFunc);
    ErrCode ret = g_settingHelper.RegisterObserver(observer);
    EXPECT_EQ(ret, ERR_OK);

    std::string value = STRING_VALUE;
    ret = g_settingHelper.PutStringValue(TEST_KEY, value);
    EXPECT_EQ(ret, ERR_OK);
    std::mutex mutex;
    std::unique_lock<std::mutex> lock(mutex);
    cv.wait(lock); // wait for observer callback

    ret = g_settingHelper.UnregisterObserver(observer);
    EXPECT_EQ(ret, ERR_OK);
    POWER_HILOGI(LABEL_TEST, "PowerSettingTest007 fun is end");
}

#ifdef SHIELD
/**
 * @tc.name: PowerSettingTest008
 * @tc.desc: Test unregister power setting observer
 * @tc.type: FUNC
 */
HWTEST_F(PowerSettingTest, PowerSettingTest008, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerSettingTest008 fun is start");
    static std::condition_variable cv;
    PowerSettingObserver::UpdateFunc observerFunc = [&](const std::string& key) {
        FAIL();
    };
    auto observer = g_settingHelper.CreateObserver(TEST_KEY, observerFunc);
    ErrCode ret = g_settingHelper.RegisterObserver(observer);
    EXPECT_EQ(ret, ERR_OK);

    ret = g_settingHelper.UnregisterObserver(observer);
    EXPECT_EQ(ret, ERR_OK);

    std::string value = STRING_VALUE;
    ret = g_settingHelper.PutStringValue(TEST_KEY, value);
    EXPECT_EQ(ret, ERR_OK);

    std::mutex mutex;
    std::unique_lock<std::mutex> lock(mutex);
    cv.wait_for(lock, std::chrono::seconds(1)); // wait for observer callback with timeout
    POWER_HILOGI(LABEL_TEST, "PowerSettingTest008 fun is end");
}
#endif
}