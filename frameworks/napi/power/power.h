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

#ifndef POWERMGR_POWER_MANAGER_POWER_H
#define POWERMGR_POWER_MANAGER_POWER_H

#include "napi/native_api.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"

#include "power_mode_info.h"

namespace OHOS {
namespace PowerMgr {
struct PowerAsyncCallbackInfo {
    napi_env env;
    napi_async_work asyncWork = nullptr;
    napi_ref callbackRef = nullptr;
    napi_deferred deferred = nullptr;
    bool screenOn = false;
    uint32_t powerMode = static_cast<uint32_t>(PowerMode::NORMAL_MODE);
};

class Power {
public:
    static napi_value ShutdownDevice(napi_env env, napi_callback_info info);
    static napi_value RebootDevice(napi_env env, napi_callback_info info);
    static napi_value IsScreenOn(napi_env env, napi_callback_info info);
    static napi_value WakeupDevice(napi_env env, napi_callback_info info);
    static napi_value SuspendDevice(napi_env env, napi_callback_info info);

private:
    static napi_value RebootOrShutdown(napi_env env, napi_callback_info info, bool isReboot);
    static void IsScreenOnCallBack(napi_env env, std::unique_ptr<PowerAsyncCallbackInfo>& asCallbackInfoPtr);
};
} // namespace PowerMgr
} // namespace OHOS

#endif // POWERMGR_POWER_MANAGER_POWER_H
