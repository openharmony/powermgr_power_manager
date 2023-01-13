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

namespace OHOS {
namespace PowerMgr {
enum class PowerErrors : int32_t {
    ERR_OK = 0,
    ERR_PERMISSION_DENIED = 201,
    ERR_PARAM_INVALID = 401,
    ERR_CONNECTION_FAIL = 4900101
};
} // namespace PowerMgr
} // namespace OHOS

#endif // POWERMGR_POWER_MGR_ERRORS_H
