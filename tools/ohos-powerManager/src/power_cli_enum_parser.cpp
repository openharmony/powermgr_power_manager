/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "power_cli_enum_parser.h"

namespace OHOS {
namespace PowerMgr {

static const std::unordered_map<std::string, PowerMode> POWER_MODE_MAP = {
    {"normal",       PowerMode::NORMAL_MODE},
    {"powerSave",    PowerMode::POWER_SAVE_MODE},
};

EnumParseResult PowerCliEnumParser::ParsePowerMode(const std::string& str)
{
    auto it = POWER_MODE_MAP.find(str);
    if (it != POWER_MODE_MAP.end()) {
        return {true, static_cast<int32_t>(it->second), ""};
    }
    return {false, 0, "Invalid power mode: '" + str + "'. Valid values: " + GetValidPowerModes()};
}

std::string PowerCliEnumParser::GetValidPowerModes()
{
    return "normal, powerSave";
}

} // namespace PowerMgr
} // namespace OHOS
