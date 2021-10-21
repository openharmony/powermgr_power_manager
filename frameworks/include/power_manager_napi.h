/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef POWERMGR_POWER_MANAGER_NAPI_H
#define POWERMGR_POWER_MANAGER_NAPI_H

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "power_mgr_client.h"
#include "power_napi_context.h"

namespace OHOS {
namespace PowerMgr {
class ShutdownContext : public PowerNapiContext {
public:
    using PowerNapiContext::PowerNapiContext;
    const std::string reason_;
protected:
    virtual void Init(napi_value* args, uint32_t argc) override;
};

class PowerManagerNapi {
public:
    static napi_value Init(napi_env env, napi_value exports);
private:
    PowerManagerNapi() = default;
    ~PowerManagerNapi() = default;
    static napi_value IsRunningLockTypeSupported(napi_env env, napi_callback_info info);
    static napi_value CreateRunningLock(napi_env env, napi_callback_info info);
    static napi_value ShutdownDevice(napi_env env, napi_callback_info info);
    static napi_value RebootDevice(napi_env env, napi_callback_info info);
    static napi_value IsScreenOn(napi_env env, napi_callback_info info);
    static napi_value GetState(napi_env env, napi_callback_info info);
    static napi_value GetMode(napi_env env, napi_callback_info info);
    static napi_value InitRunningLockType(napi_env env, napi_value exports);
    static napi_value InitPowerState(napi_env env, napi_value exports);
    static napi_value InitPowerMode(napi_env env, napi_value exports);
    static napi_value EnumRunningLockTypeConstructor(napi_env env, napi_callback_info info);
    static napi_value EnumPowerStateConstructor(napi_env env, napi_callback_info info);
    static napi_value EnumPowerModeConstructor(napi_env env, napi_callback_info info);

    static napi_ref runningLockTypeConstructor_;
    static napi_ref powerStateConstructor_;
    static napi_ref powerModeConstructor_;
};
} // namespace PowerMgr
} // namespace OHOS

#endif // POWERMGR_POWER_MANAGER_NAPI_H
