/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef POWERMGR_POWER_CJSON_UTILS_H
#define POWERMGR_POWER_CJSON_UTILS_H

#include <cJSON.h>
#include <cstring>

namespace OHOS {
namespace PowerMgr {
namespace PowerMgrJsonUtils {
inline bool IsValidJsonString(const cJSON* jsonValue)
{
    return jsonValue && cJSON_IsString(jsonValue) && jsonValue->valuestring != nullptr;
}

inline bool IsValidJsonStringAndNoEmpty(const cJSON* jsonValue)
{
    return jsonValue && cJSON_IsString(jsonValue) && jsonValue->valuestring != nullptr &&
        strlen(jsonValue->valuestring) > 0;
}

inline bool IsValidJsonNumber(const cJSON* jsonValue)
{
    return jsonValue && cJSON_IsNumber(jsonValue);
}

inline bool IsValidJsonObject(const cJSON* jsonValue)
{
    return jsonValue && cJSON_IsObject(jsonValue);
}

inline bool IsValidJsonArray(const cJSON* jsonValue)
{
    return jsonValue && cJSON_IsArray(jsonValue);
}

inline bool IsValidJsonBool(const cJSON* jsonValue)
{
    return jsonValue && cJSON_IsBool(jsonValue);
}

inline bool IsValidJsonObjectOrJsonArray(const cJSON* jsonValue)
{
    return IsValidJsonObject(jsonValue) || IsValidJsonArray(jsonValue);
}
} // namespace PowerMgrJsonUtils
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_POWER_CJSON_UTILS_H
