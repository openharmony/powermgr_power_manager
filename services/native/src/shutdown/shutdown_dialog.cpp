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
#include <dlfcn.h>

#ifdef HAS_MULTIMODALINPUT_INPUT_PART
#include <input_manager.h>
#include <key_event.h>
#include <key_option.h>
#endif
#include <cJSON.h>
#include <message_parcel.h>

#include "config_policy_utils.h"
#include "power_cjson_utils.h"
#include "power_log.h"
#include "power_mgr_service.h"
#include "power_vibrator.h"
#include "sysparam.h"

#ifdef POWER_MANAGER_ENABLE_BLOCK_LONG_PRESS
#include "setting_helper.h"
#endif

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
const std::string KEY_DOWN_DURATION = "const.powerkey.down_duration";
#ifdef POWER_MANAGER_ENABLE_BLOCK_LONG_PRESS
const std::string BLOCK_LONG_PRESS = "1";
#endif
} // namespace

std::string ShutdownDialog::bundleName_ = "com.ohos.powerdialog";
std::string ShutdownDialog::abilityName_ = "PowerUiExtensionAbility";
std::string ShutdownDialog::uiExtensionType_ = "sysDialog/power";
std::string ShutdownDialog::dialogBundleName_ = "com.ohos.systemui";
std::string ShutdownDialog::dialogAbilityName_ = "com.ohos.systemui.dialog";

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
    int32_t downDuration = SysParam::GetIntValue(KEY_DOWN_DURATION, LONG_PRESS_DELAY_MS);
    POWER_HILOGI(FEATURE_SHUTDOWN, "Initialize powerkey down duration %{public}d.", downDuration);
    keyOption->SetFinalKeyDownDuration(downDuration);
    auto inputManager = InputManager::GetInstance();
    if (!inputManager) {
        POWER_HILOGE(FEATURE_SHUTDOWN, "KeyMonitorInit inputManager is null");
        return;
    }
    longPressId_ =
        inputManager->SubscribeKeyEvent(keyOption, [this](std::shared_ptr<KeyEvent> keyEvent) {
            POWER_KHILOGI(FEATURE_SHUTDOWN, "Receive long press powerkey");
#ifdef POWER_MANAGER_ENABLE_BLOCK_LONG_PRESS
            //If the power button is shielded, the shutdown screen is not displayed.
            auto longPress = SettingHelper::GetBlockLongPress();
            if (longPress == BLOCK_LONG_PRESS) {
                POWER_HILOGI(FEATURE_SHUTDOWN, "need block power.");
                return;
            }
#endif
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

    Want want;
    want.SetElementName(dialogBundleName_, dialogAbilityName_);

    void *handler = dlopen("libpower_ability.z.so", RTLD_NOW | RTLD_NODELETE);
    if (handler == nullptr) {
        POWER_HILOGE(FEATURE_SHUTDOWN, "dlopen libpower_ability.z.so failed, reason : %{public}s", dlerror());
        return false;
    }

    auto powerConnectAbility = reinterpret_cast<void (*)(const Want&, const sptr<IAbilityConnection>&,
        int32_t)>(dlsym(handler, "PowerConnectAbility"));
    if (powerConnectAbility == nullptr) {
        POWER_HILOGE(FEATURE_SHUTDOWN, "find PowerConnectAbility function failed, reason : %{public}s", dlerror());
#ifndef FUZZ_TEST
        dlclose(handler);
#endif
        handler = nullptr;
        return false;
    }
    powerConnectAbility(want, dialogConnectionCallback_, INVALID_USERID);
#ifndef FUZZ_TEST
    dlclose(handler);
#endif
    handler = nullptr;
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
    if (contentStr.empty()) {
        POWER_HILOGE(COMP_UTILS, "json file is empty");
        return;
    }
    ParseJsonConfig(contentStr);
}

void ShutdownDialog::ParseJsonConfig(std::string& contentStr)
{
    cJSON* root = cJSON_Parse(contentStr.c_str());
    if (!root) {
        POWER_HILOGE(COMP_UTILS, "json parse error[%{public}s]", contentStr.c_str());
        return;
    }

    if (!cJSON_IsObject(root)) {
        POWER_HILOGE(COMP_UTILS, "json root invalid");
        cJSON_Delete(root);
        return;
    }

    cJSON* bundleNameItem = cJSON_GetObjectItemCaseSensitive(root, "bundleName");
    cJSON* abilityNameItem = cJSON_GetObjectItemCaseSensitive(root, "abilityName");
    cJSON* uiExtensionTypeItem = cJSON_GetObjectItemCaseSensitive(root, "uiExtensionType");
    if (!PowerMgrJsonUtils::IsValidJsonString(bundleNameItem) ||
        !PowerMgrJsonUtils::IsValidJsonString(abilityNameItem) ||
        !PowerMgrJsonUtils::IsValidJsonString(uiExtensionTypeItem)) {
        POWER_HILOGE(COMP_UTILS, "json variable not supported");
        cJSON_Delete(root);
        return;
    }

    bundleName_ = bundleNameItem->valuestring;
    abilityName_ = abilityNameItem->valuestring;
    uiExtensionType_ = uiExtensionTypeItem->valuestring;
    dialogBundleName_ = bundleNameItem->valuestring;
    dialogAbilityName_ = "com.ohos.sceneboard.systemdialog";
    POWER_HILOGI(COMP_UTILS, "PowerOff variables have changed");
    cJSON_Delete(root);
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
        std::string paramStr = "{\"ability.want.params.uiExtensionType\":" + midStr +
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
        pms->RefreshActivityInner(
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
