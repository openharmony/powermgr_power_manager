/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "sysparam.h"

#include "ffrt_utils.h"
#include "power_log.h"
#include "string_ex.h"
#include "syspara/parameter.h"
#include "syspara/parameters.h"

namespace OHOS {
namespace PowerMgr {

void SysParam::LoopReadBootCompletedParameter(BootCompletedCallback& callback)
{
    ffrt::submit([callback](void) -> void {
        constexpr int32_t delayTimeMs = 50;
        constexpr int32_t logInterval = 100;
        int32_t count = 0;
        while (!system::GetBoolParameter(KEY_BOOT_COMPLETED, false)) {
            count++;
            if (count >= logInterval) {
                POWER_HILOGW(COMP_UTILS, "bootevent not fired!");
                count = 0;
            }
            ffrt::this_task::sleep_for(std::chrono::milliseconds(delayTimeMs));
        }
        POWER_HILOGI(COMP_UTILS, "Get booteventCompleted true success!");
        callback();
    });
}

void SysParam::RegisterBootCompletedCallback(BootCompletedCallback& callback)
{
    POWER_HILOGI(COMP_UTILS, "start to RegisterBootCompletedCallback");
    int32_t ret = WatchParameter(
        KEY_BOOT_COMPLETED,
        [](const char* key, const char* value, void* context) {
            if (strcmp(value, "true") == 0) {
                ((BootCompletedCallback)context)();
            }
        },
        reinterpret_cast<void*>(callback));
    if (ret != 0) {
        POWER_HILOGW(COMP_UTILS, "RegisterBootCompletedCallback failed, ret=%{public}d", ret);
    }
}

void SysParam::RegisterBootCompletedCallbackForPowerSa(BootCompletedCallback& callback)
{
    POWER_HILOGI(COMP_UTILS, "start to RegisterBootCompletedCallback for power SA");
    int32_t ret = WatchParameter(
        KEY_BOOT_COMPLETED,
        [](const char* key, const char* value, void* context) {
            if (strcmp(value, "true") == 0) {
                ((BootCompletedCallback)context)();
            }
        },
        reinterpret_cast<void*>(callback));
    if (ret != 0) {
        POWER_HILOGW(COMP_UTILS, "RegisterBootCompletedCallback for power SA failed, ret=%{public}d", ret);
    }
    LoopReadBootCompletedParameter(callback);
}

int32_t SysParam::GetIntValue(const std::string& key, int32_t def)
{
    char value[VALUE_MAX_LEN] = {0};
    int32_t ret = GetParameter(key.c_str(), std::to_string(def).c_str(), value, VALUE_MAX_LEN);
    if (ret < 0) {
        POWER_HILOGW(COMP_UTILS, "GetParameter failed, return default value, ret=%{public}d, def=%{public}d", ret, def);
        return def;
    }
    int32_t intValue = def;
    if (!StrToInt(TrimStr(value), intValue)) {
        POWER_HILOGW(COMP_UTILS, "StrToInt failed, return default def, value=%{public}s, def=%{public}d", value, def);
        return def;
    }
    StrToInt(TrimStr(value), intValue);
    return intValue;
}
} // namespace PowerMgr
} // namespace OHOS