/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "shutdown_dialog.h"

#include <atomic>
#include <fstream>
#include <memory>
#include <set>
#include <string_ex.h>

#include <ability_manager_client.h>
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
#include <input_manager.h>
#include <key_event.h>
#include <key_option.h>
#endif
#include <message_parcel.h>

#include "config_policy_utils.h"
#include "json/reader.h"
#include "json/value.h"
#include "power_log.h"
#include "power_mgr_service.h"
#include "power_vibrator.h"

using namespace OHOS::MMI;
using namespace OHOS::AAFwk;

namespace OHOS {
namespace PowerMgr {
namespace {
static constexpr int32_t LONG_PRESS_DELAY_MS = 3000;
static constexpr int32_t INVALID_USERID = -1;
static constexpr int32_t MESSAGE_PARCEL_KEY_SIZE = 3;
static constexpr int32_t INIT_LONG_PRESS_RETRY = 3;
static constexpr uint32_t RETRY_TIME = 1000;
std::atomic_bool g_isDialogShow = false;
std::atomic_bool g_longPressShow = false;
int32_t g_retryCount = 1;
sptr<IRemoteObject> g_remoteObject = nullptr;
const std::string DIALOG_CONFIG_PATH = "etc/systemui/poweroff_config.json";
} // namespace

std::string ShutdownDialog::bundleName_ = "com.ohos.powerdialog";
std::string ShutdownDialog::abilityName_ = "PowerUiExtensionAbility";
std::string ShutdownDialog::uiExtensionType_ = "sysDialog/power";

ShutdownDialog::ShutdownDialog() : dialogConnectionCallback_(new DialogAbilityConnection()) {}

ShutdownDialog::~ShutdownDialog()
{
    dialogConnectionCallback_ = nullptr;
}

void ShutdownDialog::KeyMonitorInit()
{
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
    POWER_HILOGD(FEATURE_SHUTDOWN, "Initialize the long press powerkey");
    std::shared_ptr<KeyOption> keyOption = std::make_shared<KeyOption>();
    std::set<int32_t> preKeys;

    keyOption->SetPreKeys(preKeys);
    keyOption->SetFinalKey(KeyEvent::KEYCODE_POWER);
    keyOption->SetFinalKeyDown(true);
    keyOption->SetFinalKeyDownDuration(LONG_PRESS_DELAY_MS);
    longPressId_ =
        InputManager::GetInstance()->SubscribeKeyEvent(keyOption, [this](std::shared_ptr<KeyEvent> keyEvent) {
            POWER_HILOGI(FEATURE_SHUTDOWN, "Receive long press powerkey");
            ConnectSystemUi();
            StartVibrator();
        });
    if (longPressId_ < ERR_OK) {
        if (g_retryCount <= INIT_LONG_PRESS_RETRY) {
            POWER_HILOGI(FEATURE_SHUTDOWN, "SubscribeKey long press failed errcode = %{public}d, Try again in 1 second",
                longPressId_);
            FFRTTask task = [this] {
                KeyMonitorInit();
            };
            FFRTUtils::SubmitDelayTask(task, RETRY_TIME, queue_);
            g_retryCount++;
        }
        return;
    }
    POWER_HILOGI(FEATURE_SHUTDOWN, "SubscribeKey long press success");
#endif
}

void ShutdownDialog::KeyMonitorCancel()
{
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
    InputManager* inputManager = InputManager::GetInstance();
    if (inputManager == nullptr) {
        POWER_HILOGW(FEATURE_SHUTDOWN, "InputManager is null");
        return;
    }
    if (longPressId_ >= ERR_OK) {
        inputManager->UnsubscribeKeyEvent(longPressId_);
    }
    longPressId_ = 0;
#endif
}

bool ShutdownDialog::ConnectSystemUi()
{
    if (g_isDialogShow) {
        AppExecFwk::ElementName element;
        dialogConnectionCallback_->OnAbilityConnectDone(element, g_remoteObject, INVALID_USERID);
        POWER_HILOGW(FEATURE_SHUTDOWN, "power dialog has been show");
        return true;
    }
    auto ams = AbilityManagerClient::GetInstance();
    if (ams == nullptr) {
        POWER_HILOGW(FEATURE_SHUTDOWN, "AbilityManagerClient is nullptr");
        return false;
    }

    Want want;
    want.SetElementName("com.ohos.systemui", "com.ohos.systemui.dialog");
    ErrCode result = ams->ConnectAbility(want, dialogConnectionCallback_, INVALID_USERID);
    if (result != ERR_OK) {
        POWER_HILOGW(FEATURE_SHUTDOWN, "ConnectAbility systemui dialog failed, result = %{public}d", result);
        return false;
    }
    POWER_HILOGI(FEATURE_SHUTDOWN, "ConnectAbility systemui dialog success.");
    return true;
}

bool ShutdownDialog::IsLongPress() const
{
    return g_longPressShow;
}

void ShutdownDialog::ResetLongPressFlag()
{
    g_longPressShow = false;
}

void ShutdownDialog::LoadDialogConfig()
{
    char buf[MAX_PATH_LEN];
    char* configPath = GetOneCfgFile(DIALOG_CONFIG_PATH.c_str(), buf, MAX_PATH_LEN);
    if (configPath == nullptr || *configPath == '\0') {
        POWER_HILOGI(COMP_UTILS, "do not find shutdown off json");
        return;
    }
    std::ifstream inputStream(configPath, std::ios::in | std::ios::binary);
    std::string contentStr(std::istreambuf_iterator<char> {inputStream}, std::istreambuf_iterator<char> {});
    Json::Reader reader;
    Json::Value root;
    if (!reader.parse(contentStr.data(), contentStr.data() + contentStr.size(), root)) {
        POWER_HILOGE(COMP_UTILS, "json parse error");
        return;
    }

    if (!root["bundleName"].isString() ||
        !root["abilityName"].isString() || !root["uiExtensionType"].isString()) {
            return;
    }

    bundleName_ = root["bundleName"].asString();
    abilityName_ = root["abilityName"].asString();
    uiExtensionType_ = root["uiExtensionType"].asString();
}
void ShutdownDialog::DialogAbilityConnection::OnAbilityConnectDone(
    const AppExecFwk::ElementName& element, const sptr<IRemoteObject>& remoteObject, int resultCode)
{
    POWER_HILOGI(FEATURE_SHUTDOWN, "OnAbilityConnectDone");
    std::lock_guard lock(mutex_);
    if (remoteObject == nullptr) {
        POWER_HILOGW(FEATURE_SHUTDOWN, "remoteObject is nullptr");
        return;
    }
    if (g_remoteObject == nullptr) {
        g_remoteObject = remoteObject;
    }
    FFRTUtils::SubmitTask([remoteObject] {
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        data.WriteInt32(MESSAGE_PARCEL_KEY_SIZE);
        data.WriteString16(u"bundleName");
        data.WriteString16(Str8ToStr16(ShutdownDialog::GetBundleName()));
        data.WriteString16(u"abilityName");
        data.WriteString16(Str8ToStr16(ShutdownDialog::GetAbilityName()));
        data.WriteString16(u"parameters");
        std::string midStr = "\"";
        // sysDialogZOrder = 2 displayed on the lock screen
        std::string paramStr = "{\"ability.want.params.uiExtensionType\":"+ midStr +
            ShutdownDialog::GetUiExtensionType() + midStr + ",\"sysDialogZOrder\":2}";
        data.WriteString16(Str8ToStr16(paramStr));
        POWER_HILOGI(FEATURE_SHUTDOWN, "show power dialog is begin");
        const uint32_t cmdCode = 1;
        int32_t ret = remoteObject->SendRequest(cmdCode, data, reply, option);
        if (ret != ERR_OK) {
            POWER_HILOGW(FEATURE_SHUTDOWN, "show power dialog is failed: %{public}d", ret);
            return;
        }
        g_isDialogShow = true;
        g_longPressShow = true;
        POWER_HILOGI(FEATURE_SHUTDOWN, "show power dialog is success");
        auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
        if (pms == nullptr) {
            return;
        }
        pms->RefreshActivity(
            static_cast<int64_t>(time(nullptr)), UserActivityType::USER_ACTIVITY_TYPE_ATTENTION, false);
    });
}

void ShutdownDialog::StartVibrator()
{
    std::shared_ptr<PowerVibrator> vibrator = PowerVibrator::GetInstance();
    std::string scene = "shutdown_diag";
    vibrator->StartVibrator(scene);
}
void ShutdownDialog::DialogAbilityConnection::OnAbilityDisconnectDone(
    const AppExecFwk::ElementName& element, int resultCode)
{
    POWER_HILOGD(FEATURE_SHUTDOWN, "OnAbilityDisconnectDone");
    std::lock_guard lock(mutex_);
    g_isDialogShow = false;
    g_longPressShow = false;
    g_remoteObject = nullptr;
}
} // namespace PowerMgr
} // namespace OHOS
