/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "napi_utils.h"
#include "power_log.h"

namespace OHOS {
namespace PowerMgr {
namespace {
constexpr uint32_t MAX_SIZE = 255;
}
napi_value NapiUtils::GetCallbackInfo(napi_env& env, napi_callback_info& info, size_t& argc, napi_value argv[])
{
    napi_value thisVar = nullptr;
    void* data = nullptr;
    if (napi_ok != napi_get_cb_info(env, info, &argc, argv, &thisVar, &data)) {
        POWER_HILOGW(COMP_FWK, "Failed to get the input parameter");
    }
    return thisVar;
}

std::string NapiUtils::GetStringFromNapi(napi_env& env, napi_value& napiStr)
{
    char str[MAX_SIZE] = {0};
    size_t strLen;
    if (napi_ok != napi_get_value_string_utf8(env, napiStr, str, MAX_SIZE - 1, &strLen)) {
        POWER_HILOGW(COMP_FWK, "napi_get_value_string_utf8 failed");
        return "";
    }
    return std::string(str);
}

napi_ref NapiUtils::CreateReference(napi_env& env, napi_value& value)
{
    napi_ref refVal = nullptr;
    if (napi_ok != napi_create_reference(env, value, 1, &refVal)) {
        POWER_HILOGW(COMP_FWK, "Failed to create a value reference");
        return refVal;
    }
    return refVal;
}

void NapiUtils::ReleaseReference(napi_env& env, napi_ref& ref)
{
    if (ref != nullptr) {
        napi_delete_reference(env, ref);
        ref = nullptr;
    }
}

bool NapiUtils::CheckValueType(napi_env& env, napi_value& value, napi_valuetype checkType)
{
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, value, &valueType);
    if (valueType != checkType) {
        POWER_HILOGW(COMP_FWK, "Parameter type error");
        return false;
    }
    return true;
}
} // namespace PowerMgr
} // namespace OHOS
