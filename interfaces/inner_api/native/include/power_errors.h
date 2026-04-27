/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef POWERMGR_POWER_ERRORS_H
#define POWERMGR_POWER_ERRORS_H

#include <map>
#include <string>

namespace OHOS {
namespace PowerMgr {
enum class PowerErrors : int32_t {
    ERR_OK = 0,
    ERR_FAILURE = 1,
    ERR_PERMISSION_DENIED = 201,
    ERR_SYSTEM_API_DENIED = 202,
    /** Input is null, undefined, or mismatched type (basic type validation) */
    ERR_PARAM_INVALID = 401,
    ERR_CONNECTION_FAIL = 4900101,
    ERR_FREQUENT_FUNCTION_CALL = 4900201,
    ERR_POWER_MODE_TRANSIT_FAILED = 4900301,
    ERR_SKIP_FUNCTION_CALL = 4900401,
    ERR_READ_OPERATION_FAILED = 4900501,
    /** Input parameter value is not within expected range or valid values (value validation) */
    ERR_USER_PARAM_INVALID = 4900400,
    ERR_WRITE_OPERATION_FAILED = 4900601,
};

inline std::string GetErrorMessage(PowerErrors code)
{
    static const std::map<PowerErrors, std::string> errorTable = {
        {PowerErrors::ERR_CONNECTION_FAIL,           "Failed to connect to the service."},
        {PowerErrors::ERR_PERMISSION_DENIED,         "Permission is denied"},
        {PowerErrors::ERR_SYSTEM_API_DENIED,         "System permission is denied"},
        {PowerErrors::ERR_PARAM_INVALID,             "Invalid input parameter."},
        {PowerErrors::ERR_FREQUENT_FUNCTION_CALL,    "Frequent function calls."},
        {PowerErrors::ERR_POWER_MODE_TRANSIT_FAILED, "Setting the power mode failed."},
        {PowerErrors::ERR_READ_OPERATION_FAILED,     "Read operation failed."},
        {PowerErrors::ERR_USER_PARAM_INVALID,        "Invalid parameter"},
        {PowerErrors::ERR_WRITE_OPERATION_FAILED,    "Write operation failed."},
    };
    auto it = errorTable.find(code);
    if (it != errorTable.end()) {
        return it->second;
    }
    return "";
}
} // namespace PowerMgr
} // namespace OHOS

#endif // POWERMGR_POWER_MGR_ERRORS_H
