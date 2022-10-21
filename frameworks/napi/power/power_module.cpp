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

#include "napi/native_api.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"

#include "power.h"
#include "power_log.h"
#include "power_mode_info.h"
#include "power_napi.h"

using namespace OHOS::PowerMgr;

static napi_value EnumPowerModeClassConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisArg = nullptr;
    void* data = nullptr;

    napi_get_cb_info(env, info, nullptr, nullptr, &thisArg, &data);

    napi_value global = nullptr;
    napi_get_global(env, &global);

    return thisArg;
}

static napi_value CreateDevicePowerMode(napi_env env, napi_value exports)
{
    napi_value normal = nullptr;
    napi_value powerSave = nullptr;
    napi_value performance = nullptr;
    napi_value extremePowerSave = nullptr;

    napi_create_int32(env, (int32_t)PowerMode::NORMAL_MODE, &normal);
    napi_create_int32(env, (int32_t)PowerMode::POWER_SAVE_MODE, &powerSave);
    napi_create_int32(env, (int32_t)PowerMode::PERFORMANCE_MODE, &performance);
    napi_create_int32(env, (int32_t)PowerMode::EXTREME_POWER_SAVE_MODE, &extremePowerSave);

    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("MODE_NORMAL", normal),
        DECLARE_NAPI_STATIC_PROPERTY("MODE_POWER_SAVE", powerSave),
        DECLARE_NAPI_STATIC_PROPERTY("MODE_PERFORMANCE", performance),
        DECLARE_NAPI_STATIC_PROPERTY("MODE_EXTREME_POWER_SAVE", extremePowerSave),
    };
    napi_value result = nullptr;
    napi_define_class(env, "DevicePowerMode", NAPI_AUTO_LENGTH, EnumPowerModeClassConstructor, nullptr,
        sizeof(desc) / sizeof(*desc), desc, &result);

    napi_set_named_property(env, exports, "DevicePowerMode", result);

    return exports;
}

EXTERN_C_START
/*
 * function for module exports
 */
static napi_value PowerInit(napi_env env, napi_value exports)
{
    POWER_HILOGD(COMP_FWK, "Initialize the Power module");
    napi_property_descriptor desc[] = {
        // Old interfaces deprecated since 9
        DECLARE_NAPI_FUNCTION("shutdownDevice", Power::ShutdownDevice),
        DECLARE_NAPI_FUNCTION("rebootDevice", Power::RebootDevice),
        DECLARE_NAPI_FUNCTION("isScreenOn", Power::IsScreenOn),
        DECLARE_NAPI_FUNCTION("wakeupDevice", Power::WakeupDevice),
        DECLARE_NAPI_FUNCTION("suspendDevice", Power::SuspendDevice),

        // New interfaces
        DECLARE_NAPI_FUNCTION("shutdown", PowerNapi::Shutdown),
        DECLARE_NAPI_FUNCTION("reboot", PowerNapi::Reboot),
        DECLARE_NAPI_FUNCTION("isActive", PowerNapi::IsActive),
        DECLARE_NAPI_FUNCTION("wakeup", PowerNapi::Wakeup),
        DECLARE_NAPI_FUNCTION("suspend", PowerNapi::Suspend),
        DECLARE_NAPI_FUNCTION("setPowerMode", PowerNapi::SetPowerMode),
        DECLARE_NAPI_FUNCTION("getPowerMode", PowerNapi::GetPowerMode)};
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    CreateDevicePowerMode(env, exports);
    POWER_HILOGD(COMP_FWK, "The initialization of the Power module is complete");

    return exports;
}
EXTERN_C_END

/*
 * Module definition
 */
static napi_module g_module = {.nm_version = 1,
    .nm_flags = 0,
    .nm_filename = "power",
    .nm_register_func = PowerInit,
    .nm_modname = "power",
    .nm_priv = ((void*)0),
    .reserved = {0}};

/*
 * Module registration
 */
extern "C" __attribute__((constructor)) void RegisterPowerModule(void)
{
    napi_module_register(&g_module);
}
