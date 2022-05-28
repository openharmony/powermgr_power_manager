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

#include "mutex"
#include "uri.h"
#include "errors.h"
#include "ability_manager_client.h"
#include "power_setting_observer.h"

namespace OHOS {
namespace PowerMgr {
class PowerSettingHelper : public NoCopyable {
public:
    static PowerSettingHelper& GetInstance(int32_t systemAbilityId);
    ErrCode GetStringValue(const std::string& key, std::string& value);
    ErrCode GetIntValue(const std::string& key, int32_t& value);
    ErrCode GetLongValue(const std::string& key, int64_t& value);
    ErrCode GetBoolValue(const std::string& key, bool& value);
    ErrCode PutStringValue(const std::string& key, const std::string& value);
    ErrCode PutIntValue(const std::string& key, int32_t value);
    ErrCode PutLongValue(const std::string& key, int64_t value);
    ErrCode PutBoolValue(const std::string& key, bool value);
    ErrCode DeleteKey(const std::string& key);
    sptr<PowerSettingObserver> CreateObserver(const std::string& key, PowerSettingObserver::UpdateFunc& func);
    ErrCode RegisterObserver(const sptr<PowerSettingObserver>& observer);
    ErrCode UnregisterObserver(const sptr<PowerSettingObserver>& observer);

private:
    const std::string SETTING_URI = "dataability:///com.ohos.settingsdata.DataAbility";
    const std::string SETTING_COLUMN_KEYWORD = "KEYWORD";
    const std::string SETTING_COLUMN_VALUE = "VALUE";
    static PowerSettingHelper* instance_;
    static std::mutex mutex_;
    static sptr<IRemoteObject> remoteObj_;

    static void Initialize(int32_t systemAbilityId);
    Uri AssembleUri(const std::string& key);
};
} // OHOS
} // PowerMgr
#endif // POWERMGR_POWER_MANAGER_POWER_SETTING_HELPER_H
