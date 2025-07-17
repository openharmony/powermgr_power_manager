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
#include <cJSON.h>
#include <gtest/gtest.h>
#include "power_mgr_service.h"
#include "power_mode_policy.h"
#include "power_log.h"
#include "setting_helper.h"
#include "shutdown_dialog.h"
#include "vibrator_source_parser.h"
#include "wakeup_controller.h"
#include "wakeup_source_parser.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

class PowerConfigParseTest : public Test {
public:
    void SetUp() override
    {
        root_ = cJSON_CreateObject();
    }

    void TearDown() override
    {
        cJSON_Delete(root_);
    }

    cJSON* root_;
};
void SettingHelper::SetSettingWakeupSources(const std::string& jsonConfig)
{
}

void SettingHelper::SavePowerModeRecoverMap(const std::string& jsonConfig)
{
}

namespace {
static sptr<PowerMgrService> g_service;
constexpr size_t NUMBER_ONE = 1;
constexpr size_t NUMBER_TWO = 2;
constexpr int KEY1 = 10;
constexpr int KEY2 = 20;
constexpr int KEY3 = 30;
constexpr int VALUE1 = 100;
constexpr int VALUE2 = 200;
constexpr int VALUE3 = 300;

HWTEST_F(PowerConfigParseTest, PowerConfigParseTest001, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest001 function start!");
    EXPECT_TRUE(root_);
    cJSON_AddItemToObject(root_, "action", cJSON_CreateNumber(NUMBER_ONE));
    cJSON_AddItemToObject(root_, "delayMs", cJSON_CreateNumber(NUMBER_TWO));

    std::string key = "lid";
    std::shared_ptr<SuspendSources> parseSources = std::make_shared<SuspendSources>();
    bool result = SuspendSourceParser::ParseSourcesProc(parseSources, root_, key);

    EXPECT_TRUE(result);
    std::vector<SuspendSource> sources = parseSources->GetSourceList();
    EXPECT_EQ(sources.size(), NUMBER_ONE);
    EXPECT_EQ(sources[0].reason_, SuspendDeviceType::SUSPEND_DEVICE_REASON_LID);
    EXPECT_EQ(sources[0].action_, NUMBER_ONE);
    EXPECT_EQ(sources[0].delayMs_, NUMBER_TWO);
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest001 function end!");
}

HWTEST_F(PowerConfigParseTest, PowerConfigParseTest002, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest002 function start!");
    EXPECT_TRUE(root_);
    cJSON* valueObj = cJSON_CreateNumber(NUMBER_TWO);
    cJSON_AddItemToObject(root_, "action", valueObj);
    std::string key = "lid";
    std::shared_ptr<SuspendSources> parseSources = std::make_shared<SuspendSources>();
    bool result = SuspendSourceParser::ParseSourcesProc(parseSources, root_, key);
    EXPECT_TRUE(result);
    std::vector<SuspendSource> sources = parseSources->GetSourceList();
    EXPECT_EQ(sources.size(), NUMBER_ONE);
    EXPECT_EQ(sources[0].reason_, SuspendDeviceType::SUSPEND_DEVICE_REASON_LID);
    EXPECT_EQ(sources[0].action_, 0);
    EXPECT_EQ(sources[0].delayMs_, 0);
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest002 function end!");
}

HWTEST_F(PowerConfigParseTest, PowerConfigParseTest003, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest003 function start!");
    EXPECT_TRUE(root_);
    std::string key = "lid";
    std::shared_ptr<SuspendSources> parseSources = std::make_shared<SuspendSources>();
    bool result = SuspendSourceParser::ParseSourcesProc(parseSources, nullptr, key);
    EXPECT_TRUE(result);
    std::vector<SuspendSource> sources = parseSources->GetSourceList();
    EXPECT_EQ(sources.size(), NUMBER_ONE);
    EXPECT_EQ(sources[0].reason_, SuspendDeviceType::SUSPEND_DEVICE_REASON_LID);
    EXPECT_EQ(sources[0].action_, 0);
    EXPECT_EQ(sources[0].delayMs_, 0);
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest003 function end!");
}

HWTEST_F(PowerConfigParseTest, PowerConfigParseTest004, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest004 function start!");
    EXPECT_TRUE(root_);
    cJSON_AddItemToObject(root_, "action", cJSON_CreateNumber(NUMBER_ONE));

    std::string key = "lid";
    std::shared_ptr<SuspendSources> parseSources = std::make_shared<SuspendSources>();
    bool result = SuspendSourceParser::ParseSourcesProc(parseSources, root_, key);

    EXPECT_TRUE(result);
    std::vector<SuspendSource> sources = parseSources->GetSourceList();
    EXPECT_EQ(sources.size(), NUMBER_ONE);
    EXPECT_EQ(sources[0].reason_, SuspendDeviceType::SUSPEND_DEVICE_REASON_LID);
    EXPECT_EQ(sources[0].action_, 0);
    EXPECT_EQ(sources[0].delayMs_, 0);
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest004 function end!");
}

HWTEST_F(PowerConfigParseTest, PowerConfigParseTest005, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest005 function start!");
    EXPECT_TRUE(root_);
    cJSON_AddItemToObject(root_, "delayMs", cJSON_CreateNumber(NUMBER_TWO));

    std::string key = "lid";
    std::shared_ptr<SuspendSources> parseSources = std::make_shared<SuspendSources>();
    bool result = SuspendSourceParser::ParseSourcesProc(parseSources, root_, key);

    EXPECT_TRUE(result);
    std::vector<SuspendSource> sources = parseSources->GetSourceList();
    EXPECT_EQ(sources.size(), NUMBER_ONE);
    EXPECT_EQ(sources[0].reason_, SuspendDeviceType::SUSPEND_DEVICE_REASON_LID);
    EXPECT_EQ(sources[0].action_, 0);
    EXPECT_EQ(sources[0].delayMs_, 0);
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest005 function end!");
}

HWTEST_F(PowerConfigParseTest, PowerConfigParseTest006, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest006 function start!");
    EXPECT_TRUE(root_);
    const std::string json = R"({"lid": {"action": 1, "delayMs": 2}})";
    auto parseSources = SuspendSourceParser::ParseSources(json);
    std::vector<SuspendSource> sources = parseSources->GetSourceList();
    EXPECT_EQ(sources.size(), NUMBER_ONE);
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest006 function end!");
}

HWTEST_F(PowerConfigParseTest, PowerConfigParseTest007, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest007 function start!");
    EXPECT_TRUE(root_);
    auto parseSources = SuspendSourceParser::ParseSources("");
    std::vector<SuspendSource> sources = parseSources->GetSourceList();
    EXPECT_EQ(sources.size(), 0);
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest007 function end!");
}

HWTEST_F(PowerConfigParseTest, PowerConfigParseTest008, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest008 function start!");
    EXPECT_TRUE(root_);
    auto parseSources = SuspendSourceParser::ParseSources("[]");
    std::vector<SuspendSource> sources = parseSources->GetSourceList();
    EXPECT_EQ(sources.size(), 0);
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest008 function end!");
}

HWTEST_F(PowerConfigParseTest, PowerConfigParseTest010, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest010 function start!");
    EXPECT_TRUE(root_);
    PowerModePolicy powerModePolicy;
    powerModePolicy.InitRecoverMap();
    std::string json = "";
    bool ret = powerModePolicy.ParseRecoverJson(json);
    EXPECT_FALSE(ret);
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest010 function end!");
}

HWTEST_F(PowerConfigParseTest, PowerConfigParseTest011, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest011 function start!");
    EXPECT_TRUE(root_);
    PowerModePolicy powerModePolicy;
    std::string json = R"("this is a string")";
    bool ret = powerModePolicy.ParseRecoverJson(json);
    EXPECT_FALSE(ret);
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest011 function end!");
}

HWTEST_F(PowerConfigParseTest, PowerConfigParseTest012, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest012 function start!");
    EXPECT_TRUE(root_);
    PowerModePolicy powerModePolicy;
    std::string json = R"({invalid:json)";
    bool ret = powerModePolicy.ParseRecoverJson(json);
    EXPECT_FALSE(ret);
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest012 function end!");
}

HWTEST_F(PowerConfigParseTest, PowerConfigParseTest013, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest013 function start!");
    EXPECT_TRUE(root_);
    PowerModePolicy powerModePolicy;
    std::string json = R"({"10": 100, "20": 200, "30": 300})";
    bool ret = powerModePolicy.ParseRecoverJson(json);
    EXPECT_TRUE(ret);
    EXPECT_EQ(powerModePolicy.recoverMap_.size(), 3);
    EXPECT_EQ(powerModePolicy.recoverMap_[KEY1], VALUE1);
    EXPECT_EQ(powerModePolicy.recoverMap_[KEY2], VALUE2);
    EXPECT_EQ(powerModePolicy.recoverMap_[KEY3], VALUE3);
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest013 function end!");
}

HWTEST_F(PowerConfigParseTest, PowerConfigParseTest014, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest014 function start!");
    EXPECT_TRUE(root_);
    PowerModePolicy powerModePolicy;
    std::string json = R"({"abc": 100, "10x": 200})";
    bool ret = powerModePolicy.ParseRecoverJson(json);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(powerModePolicy.recoverMap_.empty());
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest014 function end!");
}

HWTEST_F(PowerConfigParseTest, PowerConfigParseTest015, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest015 function start!");
    EXPECT_TRUE(root_);
    PowerModePolicy powerModePolicy;
    powerModePolicy.recoverMap_[10] = 20;
    powerModePolicy.SavePowerModeRecoverMap();
    std::string json = R"({"22222222222222222222222222": 100, "10x": 200})";
    bool ret = powerModePolicy.ParseRecoverJson(json);
    EXPECT_TRUE(ret);
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest015 function end!");
}

HWTEST_F(PowerConfigParseTest, PowerConfigParseTest016, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest016 function start!");
    EXPECT_TRUE(root_);
    PowerModePolicy powerModePolicy;
    std::string json = R"({"10": "100", "20": "200", "30": "300"})";
    bool ret = powerModePolicy.ParseRecoverJson(json);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(powerModePolicy.recoverMap_.empty());
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest016 function end!");
}

HWTEST_F(PowerConfigParseTest, PowerConfigParseTest018, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest018 function start!");
    EXPECT_TRUE(root_);
    ShutdownDialog shutdownDialog;
    shutdownDialog.LoadDialogConfig();
    std::string json("");
    shutdownDialog.ParseJsonConfig(json);
    EXPECT_FALSE(shutdownDialog.bundleName_.empty());
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest018 function end!");
}

HWTEST_F(PowerConfigParseTest, PowerConfigParseTest019, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest019 function start!");
    EXPECT_TRUE(root_);
    ShutdownDialog shutdownDialog;
    std::string json("[]");
    shutdownDialog.ParseJsonConfig(json);
    EXPECT_FALSE(shutdownDialog.bundleName_.empty());
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest019 function end!");
}

HWTEST_F(PowerConfigParseTest, PowerConfigParseTest020, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest020 function start!");
    EXPECT_TRUE(root_);
    ShutdownDialog shutdownDialog;
    std::string json = R"({"uiExtensionType": "sysDialog/power"})";
    shutdownDialog.ParseJsonConfig(json);
    EXPECT_FALSE(shutdownDialog.bundleName_.empty());
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest020 function end!");
}

HWTEST_F(PowerConfigParseTest, PowerConfigParseTest021, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest021 function start!");
    EXPECT_TRUE(root_);
    ShutdownDialog shutdownDialog;
    std::string json = R"({"bundleName22": "test"})";
    shutdownDialog.ParseJsonConfig(json);
    EXPECT_FALSE(shutdownDialog.bundleName_.empty());
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest021 function end!");
}

HWTEST_F(PowerConfigParseTest, PowerConfigParseTest022, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest022 function start!");
    EXPECT_TRUE(root_);
    ShutdownDialog shutdownDialog;
    std::string json = R"({"bundleName": "com.ohos.powerdialog"})";
    shutdownDialog.ParseJsonConfig(json);
    EXPECT_FALSE(shutdownDialog.bundleName_.empty());
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest022 function end!");
}

HWTEST_F(PowerConfigParseTest, PowerConfigParseTest023, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest023 function start!");
    EXPECT_TRUE(root_);
    ShutdownDialog shutdownDialog;
    std::string json = R"({"abilityName": "PowerUiExtensionAbility"})";
    shutdownDialog.ParseJsonConfig(json);
    EXPECT_FALSE(shutdownDialog.bundleName_.empty());
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest023 function end!");
}

HWTEST_F(PowerConfigParseTest, PowerConfigParseTest024, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest024 function start!");
    EXPECT_TRUE(root_);
    cJSON* array = cJSON_CreateArray();
    cJSON_AddItemToObject(root_, "points", array);
    std::string key = "lid";
    std::shared_ptr<SuspendSources> parseSources = std::make_shared<SuspendSources>();
    bool result = SuspendSourceParser::ParseSourcesProc(parseSources, array, key);

    EXPECT_TRUE(result);
    std::vector<SuspendSource> sources = parseSources->GetSourceList();
    EXPECT_EQ(sources.size(), NUMBER_ONE);
    EXPECT_EQ(sources[0].reason_, SuspendDeviceType::SUSPEND_DEVICE_REASON_LID);
    EXPECT_EQ(sources[0].action_, 0);
    EXPECT_EQ(sources[0].delayMs_, 0);
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest024 function end!");
}
#ifdef POWER_DOUBLECLICK_ENABLE
HWTEST_F(PowerConfigParseTest, PowerConfigParseTest025, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest025 function start!");
    EXPECT_TRUE(root_);
    g_service = DelayedSpSingleton<PowerMgrService>::GetInstance();
    g_service->OnStart();
    std::shared_ptr<PowerStateMachine> stateMachine = g_service->GetPowerStateMachine();
    std::shared_ptr<WakeupController> wakeupController = std::make_shared<WakeupController>(stateMachine);
    std::string json("");
    EXPECT_TRUE(wakeupController != nullptr);
    wakeupController->WakeupParseJsonConfig(true, json);
    g_service->OnStop();
    DelayedSpSingleton<PowerMgrService>::DestroyInstance();
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest025 function end!");
}

HWTEST_F(PowerConfigParseTest, PowerConfigParseTest026, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest026 function start!");
    EXPECT_TRUE(root_);
    g_service = DelayedSpSingleton<PowerMgrService>::GetInstance();
    g_service->OnStart();
    std::shared_ptr<PowerStateMachine> stateMachine = g_service->GetPowerStateMachine();
    std::shared_ptr<WakeupController> wakeupController = std::make_shared<WakeupController>(stateMachine);
    std::string json("[]");
    EXPECT_TRUE(wakeupController != nullptr);
    wakeupController->WakeupParseJsonConfig(true, json);
    g_service->OnStop();
    DelayedSpSingleton<PowerMgrService>::DestroyInstance();
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest026 function end!");
}

HWTEST_F(PowerConfigParseTest, PowerConfigParseTest027, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest027 function start!");
    EXPECT_TRUE(root_);
    g_service = DelayedSpSingleton<PowerMgrService>::GetInstance();
    g_service->OnStart();
    std::shared_ptr<PowerStateMachine> stateMachine = g_service->GetPowerStateMachine();
    std::shared_ptr<WakeupController> wakeupController = std::make_shared<WakeupController>(stateMachine);
    std::string json = R"({"touchscreen": []})";
    EXPECT_TRUE(wakeupController != nullptr);
    wakeupController->WakeupParseJsonConfig(true, json);
    g_service->OnStop();
    DelayedSpSingleton<PowerMgrService>::DestroyInstance();
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest027 function end!");
}

HWTEST_F(PowerConfigParseTest, PowerConfigParseTest028, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest028 function start!");
    EXPECT_TRUE(root_);
    g_service = DelayedSpSingleton<PowerMgrService>::GetInstance();
    g_service->OnStart();
    std::shared_ptr<PowerStateMachine> stateMachine = g_service->GetPowerStateMachine();
    std::shared_ptr<WakeupController> wakeupController = std::make_shared<WakeupController>(stateMachine);
    std::string json = R"({"touchscreen": {"enable": "123"}})";
    EXPECT_TRUE(wakeupController != nullptr);
    wakeupController->WakeupParseJsonConfig(true, json);
    g_service->OnStop();
    DelayedSpSingleton<PowerMgrService>::DestroyInstance();
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest028 function end!");
}

HWTEST_F(PowerConfigParseTest, PowerConfigParseTest029, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest029 function start!");
    EXPECT_TRUE(root_);
    g_service = DelayedSpSingleton<PowerMgrService>::GetInstance();
    g_service->OnStart();
    std::shared_ptr<PowerStateMachine> stateMachine = g_service->GetPowerStateMachine();
    std::shared_ptr<WakeupController> wakeupController = std::make_shared<WakeupController>(stateMachine);
    std::string json = R"({"touchscreen": {"enable": false}})";
    EXPECT_TRUE(wakeupController != nullptr);
    wakeupController->WakeupParseJsonConfig(true, json);
    g_service->OnStop();
    DelayedSpSingleton<PowerMgrService>::DestroyInstance();
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest029 function end!");
}
#endif

#ifdef POWER_PICKUP_ENABLE
HWTEST_F(PowerConfigParseTest, PowerConfigParseTest030, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest030 function start!");
    EXPECT_TRUE(root_);
    g_service = DelayedSpSingleton<PowerMgrService>::GetInstance();
    g_service->OnStart();
    std::shared_ptr<PowerStateMachine> stateMachine = g_service->GetPowerStateMachine();
    std::shared_ptr<WakeupController> wakeupController = std::make_shared<WakeupController>(stateMachine);
    std::string json = R"({"pickup": {"enable": "123"}})";
    EXPECT_TRUE(wakeupController != nullptr);
    wakeupController->PickupWakeupParseJsonConfig(true, json);
    g_service->OnStop();
    DelayedSpSingleton<PowerMgrService>::DestroyInstance();
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest030 function end!");
}

HWTEST_F(PowerConfigParseTest, PowerConfigParseTest031, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest031 function start!");
    EXPECT_TRUE(root_);
    g_service = DelayedSpSingleton<PowerMgrService>::GetInstance();
    g_service->OnStart();
    std::shared_ptr<PowerStateMachine> stateMachine = g_service->GetPowerStateMachine();
    std::shared_ptr<WakeupController> wakeupController = std::make_shared<WakeupController>(stateMachine);
    std::string json("");
    EXPECT_TRUE(wakeupController != nullptr);
    wakeupController->PickupWakeupParseJsonConfig(true, json);
    g_service->OnStop();
    DelayedSpSingleton<PowerMgrService>::DestroyInstance();
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest031 function end!");
}

HWTEST_F(PowerConfigParseTest, PowerConfigParseTest032, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest032 function start!");
    EXPECT_TRUE(root_);
    g_service = DelayedSpSingleton<PowerMgrService>::GetInstance();
    g_service->OnStart();
    std::shared_ptr<PowerStateMachine> stateMachine = g_service->GetPowerStateMachine();
    std::shared_ptr<WakeupController> wakeupController = std::make_shared<WakeupController>(stateMachine);
    std::string json("[]");
    EXPECT_TRUE(wakeupController != nullptr);
    wakeupController->PickupWakeupParseJsonConfig(true, json);
    g_service->OnStop();
    DelayedSpSingleton<PowerMgrService>::DestroyInstance();
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest032 function end!");
}

HWTEST_F(PowerConfigParseTest, PowerConfigParseTest033, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest033 function start!");
    EXPECT_TRUE(root_);
    g_service = DelayedSpSingleton<PowerMgrService>::GetInstance();
    g_service->OnStart();
    std::shared_ptr<PowerStateMachine> stateMachine = g_service->GetPowerStateMachine();
    std::shared_ptr<WakeupController> wakeupController = std::make_shared<WakeupController>(stateMachine);
    std::string json = R"({"pickup": []})";
    EXPECT_TRUE(wakeupController != nullptr);
    wakeupController->PickupWakeupParseJsonConfig(true, json);
    g_service->OnStop();
    DelayedSpSingleton<PowerMgrService>::DestroyInstance();
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest033 function end!");
}

HWTEST_F(PowerConfigParseTest, PowerConfigParseTest034, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest034 function start!");
    EXPECT_TRUE(root_);
    g_service = DelayedSpSingleton<PowerMgrService>::GetInstance();
    g_service->OnStart();
    std::shared_ptr<PowerStateMachine> stateMachine = g_service->GetPowerStateMachine();
    std::shared_ptr<WakeupController> wakeupController = std::make_shared<WakeupController>(stateMachine);
    std::string json = R"({"pickup": {"enable": false}})";
    EXPECT_TRUE(wakeupController != nullptr);
    wakeupController->PickupWakeupParseJsonConfig(true, json);
    g_service->OnStop();
    DelayedSpSingleton<PowerMgrService>::DestroyInstance();
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest034 function end!");
}
#endif

HWTEST_F(PowerConfigParseTest, PowerConfigParseTest035, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest035 function start!");
    EXPECT_TRUE(root_);
    g_service = DelayedSpSingleton<PowerMgrService>::GetInstance();
    g_service->OnStart();
    std::shared_ptr<PowerStateMachine> stateMachine = g_service->GetPowerStateMachine();
    std::shared_ptr<WakeupController> wakeupController = std::make_shared<WakeupController>(stateMachine);
    std::string json = R"({"lid": {"enable": "123"}})";
    EXPECT_TRUE(wakeupController != nullptr);
    wakeupController->LidWakeupParseJsonConfig(false, json);
    g_service->OnStop();
    DelayedSpSingleton<PowerMgrService>::DestroyInstance();
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest035 function end!");
}

HWTEST_F(PowerConfigParseTest, PowerConfigParseTest036, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest036 function start!");
    EXPECT_TRUE(root_);
    g_service = DelayedSpSingleton<PowerMgrService>::GetInstance();
    g_service->OnStart();
    std::shared_ptr<PowerStateMachine> stateMachine = g_service->GetPowerStateMachine();
    std::shared_ptr<WakeupController> wakeupController = std::make_shared<WakeupController>(stateMachine);
    std::string json("");
    EXPECT_TRUE(wakeupController != nullptr);
    wakeupController->LidWakeupParseJsonConfig(true, json);
    g_service->OnStop();
    DelayedSpSingleton<PowerMgrService>::DestroyInstance();
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest036 function end!");
}

HWTEST_F(PowerConfigParseTest, PowerConfigParseTest037, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest037 function start!");
    EXPECT_TRUE(root_);
    g_service = DelayedSpSingleton<PowerMgrService>::GetInstance();
    g_service->OnStart();
    std::shared_ptr<PowerStateMachine> stateMachine = g_service->GetPowerStateMachine();
    std::shared_ptr<WakeupController> wakeupController = std::make_shared<WakeupController>(stateMachine);
    std::string json("[]");
    EXPECT_TRUE(wakeupController != nullptr);
    wakeupController->LidWakeupParseJsonConfig(true, json);
    g_service->OnStop();
    DelayedSpSingleton<PowerMgrService>::DestroyInstance();
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest037 function end!");
}

HWTEST_F(PowerConfigParseTest, PowerConfigParseTest038, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest038 function start!");
    EXPECT_TRUE(root_);
    g_service = DelayedSpSingleton<PowerMgrService>::GetInstance();
    g_service->OnStart();
    std::shared_ptr<PowerStateMachine> stateMachine = g_service->GetPowerStateMachine();
    std::shared_ptr<WakeupController> wakeupController = std::make_shared<WakeupController>(stateMachine);
    std::string json = R"({"lid": []})";
    EXPECT_TRUE(wakeupController != nullptr);
    wakeupController->LidWakeupParseJsonConfig(true, json);
    g_service->OnStop();
    DelayedSpSingleton<PowerMgrService>::DestroyInstance();
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest038 function end!");
}

HWTEST_F(PowerConfigParseTest, PowerConfigParseTest039, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest039 function start!");
    EXPECT_TRUE(root_);
    g_service = DelayedSpSingleton<PowerMgrService>::GetInstance();
    g_service->OnStart();
    std::shared_ptr<PowerStateMachine> stateMachine = g_service->GetPowerStateMachine();
    std::shared_ptr<WakeupController> wakeupController = std::make_shared<WakeupController>(stateMachine);
    EXPECT_TRUE(wakeupController != nullptr);
    std::string json = R"({"lid": {"enable": false}})";
    wakeupController->LidWakeupParseJsonConfig(true, json);
    g_service->OnStop();
    DelayedSpSingleton<PowerMgrService>::DestroyInstance();
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest039 function end!");
}

HWTEST_F(PowerConfigParseTest, PowerConfigParseTest040, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest040 function start!");
    EXPECT_TRUE(root_);
    const std::string json = "[]";
    auto parseSources = WakeupSourceParser::ParseSources(json);
    std::vector<WakeupSource> sources = parseSources->GetSourceList();
    EXPECT_EQ(sources.size(), 0);
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest040 function end!");
}

HWTEST_F(PowerConfigParseTest, PowerConfigParseTest041, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest041 function start!");
    EXPECT_TRUE(root_);
    std::string key = "test";
    std::shared_ptr<WakeupSources> parseSources = std::make_shared<WakeupSources>();
    bool result = WakeupSourceParser::ParseSourcesProc(parseSources, nullptr, key);
    EXPECT_FALSE(result);
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest041 function end!");
}

HWTEST_F(PowerConfigParseTest, PowerConfigParseTest042, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest042 function start!");
    EXPECT_TRUE(root_);
    cJSON_AddItemToObject(root_, WakeupSource::ENABLE_KEY, cJSON_CreateBool(false));
    cJSON_AddItemToObject(root_, WakeupSource::KEYS_KEY, cJSON_CreateNumber(NUMBER_TWO));

    std::string key = "test";
    std::shared_ptr<WakeupSources> parseSources = std::make_shared<WakeupSources>();
    bool result = WakeupSourceParser::ParseSourcesProc(parseSources, root_, key);

    EXPECT_FALSE(result);
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest042 function end!");
}

HWTEST_F(PowerConfigParseTest, PowerConfigParseTest043, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest043 function start!");
    EXPECT_TRUE(root_);
    cJSON_AddItemToObject(root_, WakeupSource::ENABLE_KEY, cJSON_CreateString("not bool"));
    cJSON_AddItemToObject(root_, WakeupSource::KEYS_KEY, cJSON_CreateString("not num"));

    std::string key = "test";
    std::shared_ptr<WakeupSources> parseSources = std::make_shared<WakeupSources>();
    bool result = WakeupSourceParser::ParseSourcesProc(parseSources, root_, key);

    EXPECT_FALSE(result);
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest043 function end!");
}

HWTEST_F(PowerConfigParseTest, PowerConfigParseTest044, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest044 function start!");
    EXPECT_TRUE(root_);
    const std::string json = "";
    VibratorSourceParser vibratorSourceParser;
    auto parseSources = vibratorSourceParser.ParseSources(json);
    EXPECT_EQ(parseSources.size(), 0);
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest044 function end!");
}

HWTEST_F(PowerConfigParseTest, PowerConfigParseTest045, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest045 function start!");
    EXPECT_TRUE(root_);
    const std::string json = "[]";
    VibratorSourceParser vibratorSourceParser;
    auto parseSources = vibratorSourceParser.ParseSources(json);
    EXPECT_EQ(parseSources.size(), 0);
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest045 function end!");
}

HWTEST_F(PowerConfigParseTest, PowerConfigParseTest046, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest046 function start!");
    EXPECT_TRUE(root_);
    const std::string json = "123";
    VibratorSourceParser vibratorSourceParser;
    auto parseSources = vibratorSourceParser.ParseSources(json);
    EXPECT_EQ(parseSources.size(), 0);
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest046 function end!");
}

HWTEST_F(PowerConfigParseTest, PowerConfigParseTest047, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest047 function start!");
    EXPECT_TRUE(root_);
    std::string key = "test";
    VibratorSourceParser vibratorSourceParser;
    std::vector<VibratorSource> parseSources;
    vibratorSourceParser.ParseSourcesProc(parseSources, nullptr, key);
    EXPECT_TRUE(parseSources.empty());
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest047 function end!");
}

HWTEST_F(PowerConfigParseTest, PowerConfigParseTest048, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest048 function start!");
    EXPECT_TRUE(root_);
    std::string key = "test";
    VibratorSourceParser vibratorSourceParser;
    std::vector<VibratorSource> parseSources;
    vibratorSourceParser.ParseSourcesProc(parseSources, root_, key);
    EXPECT_TRUE(parseSources.empty());
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest048 function end!");
}

HWTEST_F(PowerConfigParseTest, PowerConfigParseTest049, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest049 function start!");
    EXPECT_TRUE(root_);
    std::string key = "test";
    cJSON_AddItemToObject(root_, VibratorSource::ENABLE_KEY, cJSON_CreateBool(false));
    cJSON_AddItemToObject(root_, VibratorSource::TYPE_KEY, cJSON_CreateNumber(NUMBER_TWO));
    VibratorSourceParser vibratorSourceParser;
    std::vector<VibratorSource> parseSources;
    vibratorSourceParser.ParseSourcesProc(parseSources, root_, key);
    EXPECT_TRUE(parseSources.empty());
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest049 function end!");
}

HWTEST_F(PowerConfigParseTest, PowerConfigParseTest050, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest050 function start!");
    EXPECT_TRUE(root_);
    ShutdownDialog shutdownDialog;
    std::string json = R"({"abilityName": "PowerUiExtensionAbility", "bundleName": "com.ohos.powerdialog"})";
    shutdownDialog.ParseJsonConfig(json);
    EXPECT_FALSE(shutdownDialog.bundleName_.empty());
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest050 function end!");
}

HWTEST_F(PowerConfigParseTest, PowerConfigParseTest051, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest051 function start!");
    EXPECT_TRUE(root_);
    const std::string json = R"("this is a string")";
    auto parseSources = SuspendSourceParser::ParseSources(json);
    std::vector<SuspendSource> sources = parseSources->GetSourceList();
    EXPECT_EQ(sources.size(), 0);
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest051 function end!");
}

HWTEST_F(PowerConfigParseTest, PowerConfigParseTest052, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest052 function start!");
    EXPECT_TRUE(root_);
    const std::string json = R"("this is a string")";
    VibratorSourceParser vibratorSourceParser;
    auto parseSources = vibratorSourceParser.ParseSources(json);
    EXPECT_EQ(parseSources.size(), 0);
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest052 function end!");
}

HWTEST_F(PowerConfigParseTest, PowerConfigParseTest053, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest053 function start!");
    EXPECT_TRUE(root_);
    const std::string json = R"("this is a string")";
    auto parseSources = WakeupSourceParser::ParseSources(json);
    std::vector<WakeupSource> sources = parseSources->GetSourceList();
    EXPECT_EQ(sources.size(), 0);
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest053 function end!");
}

HWTEST_F(PowerConfigParseTest, PowerConfigParseTest054, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest054 function start!");
    EXPECT_TRUE(root_);
    ShutdownDialog shutdownDialog;
    std::string json = R"({"abilityName": "PowerUiExtensionAbility", "bundleName": "com.ohos.powerdialog",
        "uiExtensionType": "sysDialog/power"})";
    shutdownDialog.ParseJsonConfig(json);
    EXPECT_EQ(shutdownDialog.bundleName_, "com.ohos.powerdialog");
    EXPECT_EQ(shutdownDialog.abilityName_, "PowerUiExtensionAbility");
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest054 function end!");
}

HWTEST_F(PowerConfigParseTest, PowerConfigParseTest055, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest055 function start!");
    EXPECT_TRUE(root_);
    const std::string json = R"({"lid": {"enable": false, "type": "123"}})";;
    VibratorSourceParser vibratorSourceParser;
    auto parseSources = vibratorSourceParser.ParseSources(json);
    EXPECT_EQ(parseSources.size(), 0);
    POWER_HILOGI(LABEL_TEST, "PowerConfigParseTest055 function end!");
}
} // namespace
