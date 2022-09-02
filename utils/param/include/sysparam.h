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

#ifndef POWERMGR_POWER_MANAGER_SYSPARAM_H
#define POWERMGR_POWER_MANAGER_SYSPARAM_H

#include <cstdint>
#include <cstring>
#include <functional>
#include <singleton.h>

namespace OHOS {
namespace PowerMgr {
class SysParam {
public:
    typedef void (* BootCompletedCallback)();
    static void RegisterBootCompletedCallback(BootCompletedCallback&);
    static int32_t GetIntValue(const std::string& key, int32_t def);

private:
    static constexpr const char* KEY_BOOT_COMPLETED {"bootevent.boot.completed"};
    static constexpr int32_t VALUE_MAX_LEN = 32;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_POWER_MANAGER_SYSPARAM_H
