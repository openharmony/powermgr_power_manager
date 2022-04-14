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

#include <string>

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "power_log.h"
#include "power_mgr_client.h"

using namespace OHOS::PowerMgr;

namespace {
constexpr int REASON_MAX = 512;
constexpr int RESULT_SIZE = 2;
}

struct PowerAsyncCallbackInfo {
    napi_env env;
    napi_async_work asyncWork = nullptr;
    napi_ref callbackRef = nullptr;
    napi_deferred deferred = nullptr;
    bool screenOn = false;
};

static PowerMgrClient &g_powerMgrClient = PowerMgrClient::GetInstance();

static napi_value RebootOrShutdown(napi_env env, napi_callback_info info, bool isReboot)
{
    size_t argc = 1;
    napi_value args[1] = { 0 };
    napi_value jsthis;
    void *data = nullptr;

    napi_status status = napi_get_cb_info(env, info, &argc, args, &jsthis, &data);
    NAPI_ASSERT(env, (status == napi_ok) && (argc >= 1), "Failed to get cb info");
    napi_valuetype type = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[0], &type));
    NAPI_ASSERT(env, type == napi_string, "Wrong argument type. string expected.");

    char reason[REASON_MAX] = { 0 };
    size_t reasonLen = 0;
    status = napi_get_value_string_utf8(env, args[0], reason, REASON_MAX - 1, &reasonLen);
    if (status != napi_ok) {
        POWER_HILOGE(FEATURE_SHUTDOWN, "Get shutdown reason failed");
        return nullptr;
    }
    POWER_HILOGD(FEATURE_SHUTDOWN, "reboot: %{public}d, reason: %{public}s", isReboot, reason);
    if (isReboot) {
        g_powerMgrClient.RebootDevice(std::string(reason));
    } else {
        g_powerMgrClient.ShutDownDevice(std::string(reason));
    }
    return nullptr;
}

static napi_value ShutdownDevice(napi_env env, napi_callback_info info)
{
    return RebootOrShutdown(env, info, false);
}

static napi_value RebootDevice(napi_env env, napi_callback_info info)
{
    return RebootOrShutdown(env, info, true);
}

static void IsScreenOnCallBack(napi_env env, PowerAsyncCallbackInfo *asyncCallbackInfo)
{
    napi_value resource = nullptr;
    napi_create_string_utf8(env, "IsScreenOn", NAPI_AUTO_LENGTH, &resource);
    napi_create_async_work(
        env,
        nullptr,
        resource,
        [](napi_env env, void *data) {
            PowerAsyncCallbackInfo *asyncCallbackInfo = (PowerAsyncCallbackInfo *)data;
            asyncCallbackInfo->screenOn = g_powerMgrClient.IsScreenOn();
            POWER_HILOGD(COMP_FWK, "Screen is %{public}s ", asyncCallbackInfo->screenOn ? "ON" : "OFF");
        },
        [](napi_env env, napi_status status, void *data) {
            PowerAsyncCallbackInfo *asyncCallbackInfo = (PowerAsyncCallbackInfo *)data;
            napi_value result[RESULT_SIZE] = { 0 };
            napi_get_boolean(env, asyncCallbackInfo->screenOn, &result[1]);
            if (asyncCallbackInfo->deferred) {
                napi_resolve_deferred(env, asyncCallbackInfo->deferred, result[1]);
            } else {
                napi_value tmp = nullptr;
                napi_value callback = nullptr;
                napi_get_reference_value(env, asyncCallbackInfo->callbackRef, &callback);
                napi_get_undefined(env, &result[0]);
                napi_call_function(env, nullptr, callback, RESULT_SIZE, result, &tmp);
                napi_delete_reference(env, asyncCallbackInfo->callbackRef);
            }
            napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
            delete asyncCallbackInfo;
        },
        (void *)asyncCallbackInfo,
        &asyncCallbackInfo->asyncWork);
    napi_queue_async_work(env, asyncCallbackInfo->asyncWork);
}

static napi_value IsScreenOn(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = { 0 };
    napi_value jsthis;
    void *data = nullptr;

    napi_status status = napi_get_cb_info(env, info, &argc, args, &jsthis, &data);
    NAPI_ASSERT(env, (status == napi_ok), "Failed to get cb info");

    auto asyncCallbackInfo = new PowerAsyncCallbackInfo();
    if (asyncCallbackInfo == nullptr) {
        POWER_HILOGE(COMP_FWK, "Failed to create asyncCallbackInfo");
        return nullptr;
    }
    asyncCallbackInfo->env = env;

    napi_valuetype type;
    if (argc == 1) {
        NAPI_CALL(env, napi_typeof(env, args[0], &type));
        if (type != napi_function) {
            POWER_HILOGE(COMP_FWK, "Wrong argument type. napi_function expected");
            delete asyncCallbackInfo;
            return nullptr;
        }
        napi_create_reference(env, args[0], 1, &asyncCallbackInfo->callbackRef);
    }
    napi_value result = nullptr;
    if (asyncCallbackInfo->callbackRef == nullptr) {
        POWER_HILOGD(COMP_FWK, "callbackRef is null");
        napi_create_promise(env, &asyncCallbackInfo->deferred, &result);
    } else {
        POWER_HILOGD(COMP_FWK, "callbackRef is not null");
        napi_get_undefined(env, &result);
    }
    IsScreenOnCallBack(env, asyncCallbackInfo);
    return result;
}

static napi_value WakeupDevice(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = { 0 };
    napi_value jsthis;
    void *data = nullptr;

    napi_status status = napi_get_cb_info(env, info, &argc, args, &jsthis, &data);
    NAPI_ASSERT(env, (status == napi_ok) && (argc >= 1), "Failed to get cb info");
    napi_valuetype type = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[0], &type));
    NAPI_ASSERT(env, type == napi_string, "Wrong argument type. string expected.");

    char reason[REASON_MAX] = { 0 };
    size_t reasonLen = 0;
    status = napi_get_value_string_utf8(env, args[0], reason, REASON_MAX - 1, &reasonLen);
    if (status != napi_ok) {
        POWER_HILOGE(FEATURE_WAKEUP, "Get wakeup reason failed");
        return nullptr;
    }
    POWER_HILOGD(FEATURE_WAKEUP, "Wakeup type: APPLICATION,  reason: %{public}s", reason);
    g_powerMgrClient.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_APPLICATION, std::string(reason));
    return nullptr;
}

static napi_value SuspendDevice(napi_env env, napi_callback_info info)
{
    napi_value thisArg = nullptr;
    void *data = nullptr;

    napi_status status = napi_get_cb_info(env, info, nullptr, nullptr, &thisArg, &data);
    NAPI_ASSERT(env, (status == napi_ok), "Failed to get cb info");

    POWER_HILOGD(FEATURE_SUSPEND, "Suspend type: APPLICATION");
    g_powerMgrClient.SuspendDevice(SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    return nullptr;
}

EXTERN_C_START
/*
 * function for module exports
 */
static napi_value PowerInit(napi_env env, napi_value exports)
{
    POWER_HILOGD(COMP_FWK, "Initialize the Power module");
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("shutdownDevice", ShutdownDevice),
        DECLARE_NAPI_FUNCTION("rebootDevice", RebootDevice),
        DECLARE_NAPI_FUNCTION("isScreenOn", IsScreenOn),
        DECLARE_NAPI_FUNCTION("wakeupDevice", WakeupDevice),
        DECLARE_NAPI_FUNCTION("suspendDevice", SuspendDevice),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    POWER_HILOGD(COMP_FWK, "The initialization of the Power module is complete");

    return exports;
}
EXTERN_C_END

/*
 * Module definition
 */
static napi_module g_module = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = "power",
    .nm_register_func = PowerInit,
    .nm_modname = "power",
    .nm_priv = ((void *)0),
    .reserved = {0}
};

/*
 * Module registration
 */
extern "C" __attribute__((constructor)) void RegisterPowerModule(void)
{
    napi_module_register(&g_module);
}