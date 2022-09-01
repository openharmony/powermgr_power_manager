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

#include "setting_observer.h"

namespace OHOS {
namespace PowerMgr {
SettingObserver::SettingObserver() = default;
SettingObserver::~SettingObserver() = default;

void SettingObserver::OnChange()
{
    if (update_) {
        update_(key_);
    }
}

void SettingObserver::SetKey(const std::string& key)
{
    key_ = key;
}

const std::string& SettingObserver::GetKey()
{
    return key_;
}

void SettingObserver::SetUpdateFunc(UpdateFunc& func)
{
    update_ = func;
}
} // OHOS
} // PowerMgr