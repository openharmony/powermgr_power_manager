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

#include <cstdio>
#include <cstdlib>
#include <string>
#include "hilog_wrapper.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
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
    POWER_HILOGD(MODULE_JS_NAPI, "%{public}s: enter, %{public}s", __func__, isReboot ? "reboot" : "shutdown");
    size_t argc = 1;
    napi_value args[1] = { 0 };
    napi_value jsthis;
    void *data = nullptr;

    napi_status status = napi_get_cb_info(env, info, &argc, args, &jsthis, &data);
    NAPI_ASSERT(env, (status == napi_ok) && (argc >= 1), "failed to get cb info");
    napi_valuetype type = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[0], &type));
    NAPI_ASSERT(env, type == napi_string, "wrong argument type. string expected.");

    char reason[REASON_MAX] = { 0 };
    size_t reasonLen = 0;
    status = napi_get_value_string_utf8(env, args[0], reason, REASON_MAX - 1, &reasonLen);
    if (status != napi_ok) {
        POWER_HILOGE(MODULE_JS_NAPI, "%{public}s: get reason failed", __func__);
        return nullptr;
    }
    if (isReboot) {
        g_powerMgrClient.RebootDevice(std::string(reason));
    } else {
        g_powerMgrClient.ShutDownDevice(std::string(reason));
    }
    POWER_HILOGD(MODULE_JS_NAPI, "%{public}s: reason %{public}s, exit", __func__, reason);
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
            POWER_HILOGD(MODULE_JS_NAPI, "%{public}s: screen is %{public}s ", __func__,
                asyncCallbackInfo->screenOn ? "on" : "off");
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
    POWER_HILOGD(MODULE_JS_NAPI, "%{public}s: enter", __func__);
    size_t argc = 1;
    napi_value args[1] = { 0 };
    napi_value jsthis;
    void *data = nullptr;

    napi_status status = napi_get_cb_info(env, info, &argc, args, &jsthis, &data);
    NAPI_ASSERT(env, (status == napi_ok), "IsScreenOn: failed to get cb info");

    auto asyncCallbackInfo = new PowerAsyncCallbackInfo();
    if (asyncCallbackInfo == nullptr) {
        POWER_HILOGE(MODULE_JS_NAPI, "%{public}s: new asyncCallbackInfo failed", __func__);
        return nullptr;
    }
    asyncCallbackInfo->env = env;

    napi_valuetype type;
    if (argc == 1) {
        NAPI_CALL(env, napi_typeof(env, args[0], &type));
        if (type != napi_function) {
            POWER_HILOGE(MODULE_JS_NAPI, "%{public}s: wrong argument type. napi_function expected", __func__);
            delete asyncCallbackInfo;
            return nullptr;
        }
        napi_create_reference(env, args[0], 1, &asyncCallbackInfo->callbackRef);
    }
    napi_value result = nullptr;
    if (asyncCallbackInfo->callbackRef == nullptr) {
        POWER_HILOGD(MODULE_JS_NAPI, "%{public}s: callbackRef is null", __func__);
        napi_create_promise(env, &asyncCallbackInfo->deferred, &result);
    } else {
        POWER_HILOGD(MODULE_JS_NAPI, "%{public}s: callbackRef is not null", __func__);
        napi_get_undefined(env, &result);
    }
    IsScreenOnCallBack(env, asyncCallbackInfo);
    POWER_HILOGD(MODULE_JS_NAPI, "%{public}s: exit", __func__);
    return result;
}

EXTERN_C_START
/*
 * function for module exports
 */
static napi_value PowerInit(napi_env env, napi_value exports)
{
    POWER_HILOGD(MODULE_JS_NAPI, "%{public}s: enter", __func__);
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("shutdownDevice", ShutdownDevice),
        DECLARE_NAPI_FUNCTION("rebootDevice", RebootDevice),
        DECLARE_NAPI_FUNCTION("isScreenOn", IsScreenOn),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    POWER_HILOGD(MODULE_JS_NAPI, "%{public}s: exit", __func__);

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