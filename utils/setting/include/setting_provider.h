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

#ifndef POWERMGR_POWER_MANAGER_POWER_SETTING_HELPER_H
#define POWERMGR_POWER_MANAGER_POWER_SETTING_HELPER_H

#include "datashare_helper.h"
#include "errors.h"
#include "mutex"
#include "setting_observer.h"

namespace OHOS {
namespace PowerMgr {
class SettingProvider : public NoCopyable {
public:
    static SettingProvider& GetInstance(int32_t systemAbilityId);
    ErrCode GetStringValue(const std::string& key, std::string& value);
    ErrCode GetIntValue(const std::string& key, int32_t& value);
    ErrCode GetLongValue(const std::string& key, int64_t& value);
    ErrCode GetBoolValue(const std::string& key, bool& value);
    ErrCode PutStringValue(const std::string& key, const std::string& value, bool needNotify = true);
    ErrCode PutIntValue(const std::string& key, int32_t value, bool needNotify = true);
    ErrCode PutLongValue(const std::string& key, int64_t value, bool needNotify = true);
    ErrCode PutBoolValue(const std::string& key, bool value, bool needNotify = true);
    bool IsValidKey(const std::string& key);
    sptr<SettingObserver> CreateObserver(const std::string& key, SettingObserver::UpdateFunc& func);
    static void ExecRegisterCb(const sptr<SettingObserver>& observer);
    ErrCode RegisterObserver(const sptr<SettingObserver>& observer);
    ErrCode UnregisterObserver(const sptr<SettingObserver>& observer);
    void UpdateCurrentUserId();

protected:
    ~SettingProvider() override;

private:
    static constexpr const char* SETTING_DISPLAY_OFF_TIME_KEY {"settings.display.screen_off_timeout"};
    static constexpr const char* SETTING_AUTO_ADJUST_BRIGHTNESS_KEY {"settings.display.auto_screen_brightness"};
    static constexpr const char* SETTING_BRIGHTNESS_KEY {"settings.display.screen_brightness_status"};
    static constexpr const char* SETTING_VIBRATION_KEY {"physic_navi_haptic_feedback_enabled"};
    static constexpr const char* SETTING_WINDOW_ROTATION_KEY {"settings.general.accelerometer_rotation_status"};
    static constexpr const char* SETTING_POWER_SUSPEND_SOURCES_KEY {"settings.power.suspend_sources"};
    static constexpr const char* SETTING_POWER_WAKEUP_SOURCES_KEY {"settings.power.wakeup_sources"};
    static constexpr const char* SETTING_INTELL_VOICE_KEY {"intell_voice_trigger_enabled"};
    static constexpr const char* SETTING_POWER_WAKEUP_DOUBLE_KEY {"settings.power.wakeup_double_click"};
    static constexpr const char* SETTING_POWER_WAKEUP_PICKUP_KEY {"settings.power.wakeup_pick_up"};
    static constexpr const char* SETTING_POWER_MODE_KEY  {"settings.power.smart_mode_status"};
    static constexpr const char* SETTING_POWER_MODE_BACKUP_KEY  {"settings.power.smart_mode_status.backup"};
    static constexpr const char* SETTING_POWER_WAKEUP_LID_KEY {"settings.power.wakeup_lid"};
    static SettingProvider* instance_;
    static std::mutex settingMutex_;
    static sptr<IRemoteObject> remoteObj_;
    static int32_t currentUserId_;

    static void Initialize(int32_t systemAbilityId);
    static std::shared_ptr<DataShare::DataShareHelper> CreateDataShareHelper(const std::string& key);
    static bool ReleaseDataShareHelper(std::shared_ptr<DataShare::DataShareHelper>& helper);
    static Uri AssembleUri(const std::string& key);
    static bool IsNeedMultiUser(const std::string& key);
    static std::string ReplaceUserIdForUri(int32_t userId);
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_POWER_MANAGER_POWER_SETTING_HELPER_H
