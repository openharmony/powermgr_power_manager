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

#include "ability_scheduler_interface.h"
#include "errors.h"
#include "mutex"
#include "setting_observer.h"
#include "uri.h"

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
    sptr<SettingObserver> CreateObserver(const std::string& key, SettingObserver::UpdateFunc& func);
    ErrCode RegisterObserver(const sptr<SettingObserver>& observer);
    ErrCode UnregisterObserver(const sptr<SettingObserver>& observer);

protected:
    ~SettingProvider() override;

private:
    static SettingProvider* instance_;
    static std::mutex mutex_;
    static sptr<IRemoteObject> remoteObj_;
    static Uri settingUri_;

    static void Initialize(int32_t systemAbilityId);
    static sptr<AAFwk::IAbilityScheduler> AcquireDataAbility();
    static bool ReleaseDataAbility(sptr<AAFwk::IAbilityScheduler>& dataAbility);
    static Uri AssembleUri(const std::string& key);
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_POWER_MANAGER_POWER_SETTING_HELPER_H
