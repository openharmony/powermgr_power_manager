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

#ifndef POWERMGR_POWER_MANAGER_POWER_SETTING_OBSERVER_H
#define POWERMGR_POWER_MANAGER_POWER_SETTING_OBSERVER_H

#include "data_ability_observer_stub.h"

namespace OHOS {
namespace PowerMgr {
class SettingObserver : public AAFwk::DataAbilityObserverStub {
public:
    SettingObserver();
    ~SettingObserver() override;
    void OnChange() override;

    void SetKey(const std::string& key);
    const std::string& GetKey();

    using UpdateFunc = std::function<void(const std::string&)>;
    void SetUpdateFunc(UpdateFunc& func);
private:
    std::string key_ {};
    UpdateFunc update_ = nullptr;
};
} // OHOS
} // PowerMgr

#endif // POWERMGR_POWER_MANAGER_POWER_SETTING_OBSERVER_H
