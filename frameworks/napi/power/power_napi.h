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

#ifndef POWERMGR_POWER_MANAGER_POWER_NAPI_H
#define POWERMGR_POWER_MANAGER_POWER_NAPI_H

#include "napi/native_api.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"

#include "async_callback_info.h"
namespace OHOS {
namespace PowerMgr {
class PowerNapi {
public:
    static napi_value Shutdown(napi_env env, napi_callback_info info);
    static napi_value Reboot(napi_env env, napi_callback_info info);
    static napi_value IsActive(napi_env env, napi_callback_info info);
    static napi_value Wakeup(napi_env env, napi_callback_info info);
    static napi_value Suspend(napi_env env, napi_callback_info info);
    static napi_value SetPowerMode(napi_env env, napi_callback_info info);
    static napi_value GetPowerMode(napi_env env, napi_callback_info info);

private:
    static napi_value RebootOrShutdown(napi_env env, napi_callback_info info, bool isReboot);
    static napi_value SetPowerModeCallback(
        napi_env& env, napi_value argv[], std::unique_ptr<AsyncCallbackInfo>& asyncInfo);
    static napi_value SetPowerModePromise(
        napi_env& env, napi_value argv[], std::unique_ptr<AsyncCallbackInfo>& asyncInfo);
    static void AsyncWork(napi_env& env, std::unique_ptr<AsyncCallbackInfo>& asyncInfo, const std::string resourceName,
        napi_async_execute_callback execute, napi_async_complete_callback complete);
};
} // namespace PowerMgr
} // namespace OHOS

#endif // POWERMGR_POWER_MANAGER_POWER_NAPI_H
