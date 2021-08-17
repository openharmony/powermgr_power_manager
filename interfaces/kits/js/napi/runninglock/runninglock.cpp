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
#include "running_lock_info.h"

using namespace OHOS::PowerMgr;

namespace {
constexpr int RESULT_SIZE = 2;
constexpr int RUNNINGLOCK_NAME_MAX = 512;
constexpr int CREATRUNNINGLOCK_ARGC = 3;
constexpr int ISRUNNINGLOCKTYPESUPPORTED_ARGC = 2;
}

struct RunningLockAsyncCallbackInfo {
    napi_env env;
    napi_async_work asyncWork = nullptr;
    napi_ref callbackRef = nullptr;
    napi_deferred deferred = nullptr;
    char name[RUNNINGLOCK_NAME_MAX] = { 0 };
    size_t nameLen = 0;
    RunningLockType type = RunningLockType::RUNNINGLOCK_BUTT;
    bool isSupported = false;
    std::shared_ptr<RunningLock> runningLock = nullptr;
};

struct RunningLockEntity {
    std::shared_ptr<RunningLock> runningLock;
};

static napi_ref g_runningLockConstructor;
static PowerMgrClient &g_powerMgrClient = PowerMgrClient::GetInstance();

static napi_value Unlock(napi_env env, napi_callback_info info)
{
    POWER_HILOGD(MODULE_JS_NAPI, "%{public}s: enter", __func__);
    napi_value thisArg = nullptr;
    void *data = nullptr;

    napi_status status = napi_get_cb_info(env, info, nullptr, nullptr, &thisArg, &data);
    NAPI_ASSERT(env, (status == napi_ok), "Unlock: failed to get cb info");

    RunningLockEntity *entity = nullptr;
    status = napi_unwrap(env, thisArg, (void **)&entity);
    if (status != napi_ok) {
        POWER_HILOGE(MODULE_JS_NAPI, "%{public}s: cannot unwrap for pointer", __func__);
        return nullptr;
    }
    if (entity == nullptr || entity->runningLock == nullptr) {
        POWER_HILOGE(MODULE_JS_NAPI, "%{public}s: entity->runningLock is nullptr", __func__);
        return nullptr;
    }
    entity->runningLock->UnLock();
    POWER_HILOGD(MODULE_JS_NAPI, "%{public}s: exit", __func__);
    return nullptr;
}

static napi_value IsUsed(napi_env env, napi_callback_info info)
{
    POWER_HILOGD(MODULE_JS_NAPI, "%{public}s: enter", __func__);
    napi_value thisArg = nullptr;
    napi_value result = nullptr;
    void *data = nullptr;

    napi_status status = napi_get_cb_info(env, info, nullptr, nullptr, &thisArg, &data);
    NAPI_ASSERT(env, (status == napi_ok), "IsUsed: failed to get cb info");
    napi_get_boolean(env, false, &result);
    RunningLockEntity *entity = nullptr;
    status = napi_unwrap(env, thisArg, (void **)&entity);
    if (status != napi_ok) {
        POWER_HILOGE(MODULE_JS_NAPI, "%{public}s: cannot unwrap for pointer", __func__);
        return result;
    }
    if (entity == nullptr || entity->runningLock == nullptr) {
        POWER_HILOGE(MODULE_JS_NAPI, "%{public}s: entity->runningLock is nullptr", __func__);
        return result;
    }
    bool isUsed = entity->runningLock->IsUsed();
    napi_get_boolean(env, isUsed, &result);
    POWER_HILOGD(MODULE_JS_NAPI, "%{public}s: exit", __func__);
    return result;
}

static napi_value Lock(napi_env env, napi_callback_info info)
{
    POWER_HILOGD(MODULE_JS_NAPI, "%{public}s: enter", __func__);
    size_t argc = 1;
    napi_value args[1] = { 0 };
    napi_value thisArg = nullptr;
    void *data = nullptr;

    napi_status status = napi_get_cb_info(env, info, &argc, args, &thisArg, &data);
    NAPI_ASSERT(env, (status == napi_ok) && (argc >= 1), "Lock: failed to get cb info");
    napi_valuetype type = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[0], &type));
    NAPI_ASSERT(env, type == napi_number, "Lock: wrong argument type. number expected.");

    uint32_t timeOut;
    status = napi_get_value_uint32(env, args[0], &timeOut);
    if (status != napi_ok) {
        POWER_HILOGE(MODULE_JS_NAPI, "%{public}s: napi_get_value_uint32 failed", __func__);
        return nullptr;
    }
    RunningLockEntity *entity = nullptr;
    status = napi_unwrap(env, thisArg, (void **)&entity);
    if (status != napi_ok) {
        POWER_HILOGE(MODULE_JS_NAPI, "%{public}s: cannot unwrap for pointer", __func__);
        return nullptr;
    }
    if (entity == nullptr || entity->runningLock == nullptr) {
        POWER_HILOGE(MODULE_JS_NAPI, "%{public}s: entity->runningLock is nullptr", __func__);
        return nullptr;
    }
    entity->runningLock->Lock(timeOut);
    POWER_HILOGD(MODULE_JS_NAPI, "%{public}s: exit", __func__);
    return nullptr;
}

static napi_value CreateInstanceForRunningLock(napi_env env, RunningLockAsyncCallbackInfo *asyncCallbackInfo)
{
    napi_value cons = nullptr;
    napi_value instance = nullptr;
    napi_status callBackStatus;
    RunningLockEntity *entity = nullptr;

    if (asyncCallbackInfo->runningLock == nullptr) {
        POWER_HILOGE(MODULE_JS_NAPI, "%{public}s: runningLock is nullptr", __func__);
        return nullptr;
    }
    callBackStatus = napi_get_reference_value(env, g_runningLockConstructor, &cons);
    if (callBackStatus != napi_ok) {
        POWER_HILOGE(MODULE_JS_NAPI, "%{public}s: napi get reference value failed", __func__);
        return nullptr;
    }
    callBackStatus = napi_new_instance(env, cons, 0, nullptr, &instance);
    if (callBackStatus != napi_ok || instance == nullptr) {
        POWER_HILOGE(MODULE_JS_NAPI, "%{public}s: napi new reference failed", __func__);
        return nullptr;
    }
    callBackStatus = napi_unwrap(env, instance, (void **)&entity);
    if (callBackStatus != napi_ok || entity == nullptr) {
        POWER_HILOGE(MODULE_JS_NAPI, "%{public}s: cannot unwrap entity from instance", __func__);
        return nullptr;
    }
    entity->runningLock = asyncCallbackInfo->runningLock;
    return instance;
}

static void CreateRunningLockCallBack(napi_env env, RunningLockAsyncCallbackInfo *asyncCallbackInfo)
{
    napi_value resource = nullptr;
    napi_create_string_utf8(env, "createRunningLock", NAPI_AUTO_LENGTH, &resource);
    napi_create_async_work(
        env,
        nullptr,
        resource,
        [](napi_env env, void *data) {
            RunningLockAsyncCallbackInfo *asyncCallbackInfo = (RunningLockAsyncCallbackInfo *)data;
            asyncCallbackInfo->runningLock = g_powerMgrClient.CreateRunningLock(std::string(asyncCallbackInfo->name),
                asyncCallbackInfo->type);
        },
        [](napi_env env, napi_status status, void *data) {
            RunningLockAsyncCallbackInfo *asyncCallbackInfo = (RunningLockAsyncCallbackInfo *)data;
            napi_value result[RESULT_SIZE] = { 0 };
            result[1] = CreateInstanceForRunningLock(env, asyncCallbackInfo);
            if (result[1] == nullptr) {
                napi_value message = nullptr;
                napi_create_string_utf8(env, "runningLock create failed", NAPI_AUTO_LENGTH, &message);
                napi_create_error(env, nullptr, message, &result[0]);
            }
            if (asyncCallbackInfo->deferred) {
                if (result[1] != nullptr) {
                    napi_resolve_deferred(env, asyncCallbackInfo->deferred, result[1]);
                } else {
                    napi_reject_deferred(env, asyncCallbackInfo->deferred, result[0]);
                }
            } else {
                napi_value tmp = nullptr;
                napi_value callback = nullptr;
                napi_get_undefined(env, &result[0]);
                napi_get_reference_value(env, asyncCallbackInfo->callbackRef, &callback);
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

static napi_value CreateRunningLock(napi_env env, napi_callback_info info)
{
    POWER_HILOGD(MODULE_JS_NAPI, "%{public}s: enter", __func__);
    size_t argc = CREATRUNNINGLOCK_ARGC;
    napi_value argv[CREATRUNNINGLOCK_ARGC] = { 0 };
    napi_value thisArg = nullptr;
    void *data = nullptr;

    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisArg, &data);
    NAPI_ASSERT(env, (status == napi_ok) && (argc >= CREATRUNNINGLOCK_ARGC - 1),
        "CreateRunningLock: failed to get cb info");

    auto asyncCallbackInfo = new RunningLockAsyncCallbackInfo();
    if (asyncCallbackInfo == nullptr) {
        POWER_HILOGE(MODULE_JS_NAPI, "%{public}s: asyncCallbackInfo is nullptr", __func__);
        return nullptr;
    }
    asyncCallbackInfo->env = env;

    for (size_t i = 0; i < argc; i++) {
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[i], &valueType);
        if ((i == 0) && (valueType == napi_string)) {
            napi_get_value_string_utf8(env, argv[i], asyncCallbackInfo->name, RUNNINGLOCK_NAME_MAX - 1,
                &asyncCallbackInfo->nameLen);
        }
        if (i == 1) {
            int32_t type = static_cast<int32_t>(RunningLockType::RUNNINGLOCK_BUTT);
            napi_get_value_int32(env, argv[i], &type);
            asyncCallbackInfo->type = static_cast<RunningLockType>(type);
        }
        if (valueType == napi_function) {
            napi_create_reference(env, argv[i], 1, &asyncCallbackInfo->callbackRef);
        }
    }
    napi_value result = nullptr;
    if (asyncCallbackInfo->callbackRef == nullptr) {
        POWER_HILOGD(MODULE_JS_NAPI, "%{public}s: callbackRef is null", __func__);
        napi_create_promise(env, &asyncCallbackInfo->deferred, &result);
    } else {
        POWER_HILOGD(MODULE_JS_NAPI, "%{public}s: callbackRef is not null", __func__);
        napi_get_undefined(env, &result);
    }
    CreateRunningLockCallBack(env, asyncCallbackInfo);
    POWER_HILOGD(MODULE_JS_NAPI, "%{public}s: exit", __func__);
    return result;
}

static void IsRunningLockTypeSupportedCallBack(napi_env env,
    RunningLockAsyncCallbackInfo *asyncCallbackInfo)
{
    napi_value resource = nullptr;
    napi_create_string_utf8(env, "isRunningLockTypeSupported", NAPI_AUTO_LENGTH, &resource);
    napi_create_async_work(
        env,
        nullptr,
        resource,
        [](napi_env env, void *data) {
            RunningLockAsyncCallbackInfo *asyncCallbackInfo = (RunningLockAsyncCallbackInfo *)data;
            if (asyncCallbackInfo->type == RunningLockType::RUNNINGLOCK_BACKGROUND) {
                asyncCallbackInfo->isSupported = true;
            }
            POWER_HILOGD(MODULE_JS_NAPI, "%{public}s: runningLock isSupported %{public}s", __func__,
                asyncCallbackInfo->isSupported ? "true" : "false");
        },
        [](napi_env env, napi_status status, void *data) {
            RunningLockAsyncCallbackInfo *asyncCallbackInfo = (RunningLockAsyncCallbackInfo *)data;
            napi_value result[RESULT_SIZE] = { 0 };
            napi_get_boolean(env, asyncCallbackInfo->isSupported, &result[1]);
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

static napi_value IsRunningLockTypeSupported(napi_env env, napi_callback_info info)
{
    POWER_HILOGD(MODULE_JS_NAPI, "%{public}s: enter", __func__);
    size_t argc = ISRUNNINGLOCKTYPESUPPORTED_ARGC;
    napi_value argv[ISRUNNINGLOCKTYPESUPPORTED_ARGC] = { 0 };
    napi_value thisArg = nullptr;
    void *data = nullptr;

    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisArg, &data);
    NAPI_ASSERT(env, (status == napi_ok) && (argc >= 1),
        "IsRunningLockTypeSupported: failed to get cb info");

    auto asyncCallbackInfo = new RunningLockAsyncCallbackInfo();
    if (asyncCallbackInfo == nullptr) {
        POWER_HILOGE(MODULE_JS_NAPI, "%{public}s: asyncCallbackInfo is nullptr", __func__);
        return nullptr;
    }
    asyncCallbackInfo->env = env;

    for (size_t i = 0; i < argc; i++) {
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[i], &valueType);
        if (i == 0) {
            int32_t type = static_cast<int32_t>(RunningLockType::RUNNINGLOCK_BUTT);
            napi_get_value_int32(env, argv[i], &type);
            asyncCallbackInfo->type = static_cast<RunningLockType>(type);
        }
        if (valueType == napi_function) {
            napi_create_reference(env, argv[i], 1, &asyncCallbackInfo->callbackRef);
        }
    }

    napi_value result = nullptr;
    if (asyncCallbackInfo->callbackRef == nullptr) {
        POWER_HILOGD(MODULE_JS_NAPI, "%{public}s: callbackRef is null", __func__);
        napi_create_promise(env, &asyncCallbackInfo->deferred, &result);
    } else {
        POWER_HILOGD(MODULE_JS_NAPI, "%{public}s: callbackRef is not null", __func__);
        napi_get_undefined(env, &result);
    }
    IsRunningLockTypeSupportedCallBack(env, asyncCallbackInfo);
    POWER_HILOGD(MODULE_JS_NAPI, "%{public}s: exit", __func__);
    return result;
}

static napi_value EnumRunningLockTypeConstructor(napi_env env, napi_callback_info info)
{
    POWER_HILOGD(MODULE_JS_NAPI, "%{public}s: enter", __func__);
    napi_value thisArg = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &thisArg, &data);
    napi_value global = nullptr;
    napi_get_global(env, &global);
    POWER_HILOGD(MODULE_JS_NAPI, "%{public}s: exit", __func__);
    return thisArg;
}

static napi_value CreateEnumRunningLockType(napi_env env, napi_value exports)
{
    POWER_HILOGD(MODULE_JS_NAPI, "%{public}s: enter", __func__);
    napi_value background = nullptr;
    napi_value proximityscreencontrol = nullptr;

    napi_create_int32(env, (int32_t)RunningLockType::RUNNINGLOCK_BACKGROUND, &background);
    napi_create_int32(env, (int32_t)RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL, &proximityscreencontrol);

    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("BACKGROUND", background),
        DECLARE_NAPI_STATIC_PROPERTY("PROXIMITY_SCREEN_CONTROL", proximityscreencontrol),
    };
    napi_value result = nullptr;
    napi_define_class(env, "RunningLockType", NAPI_AUTO_LENGTH, EnumRunningLockTypeConstructor, nullptr,
        sizeof(desc) / sizeof(*desc), desc, &result);

    napi_set_named_property(env, exports, "RunningLockType", result);
    POWER_HILOGD(MODULE_JS_NAPI, "%{public}s: exit", __func__);
    return exports;
}

static napi_value RunningLockConstructor(napi_env env, napi_callback_info info)
{
    POWER_HILOGD(MODULE_JS_NAPI, "%{public}s: enter", __func__);
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, &data);

    auto entity = new RunningLockEntity();
    napi_wrap(
        env, thisVar, entity,
        [](napi_env env, void *data, void *hint) {
            POWER_HILOGD(MODULE_JS_NAPI, "%{public}s: Destructor", __func__);
            auto entity = (RunningLockEntity*)data;
            delete entity;
        },
        nullptr, nullptr);
    POWER_HILOGD(MODULE_JS_NAPI, "%{public}s: exit", __func__);

    return thisVar;
}

static napi_value CreateRunningLockClass(napi_env env, napi_value exports)
{
    POWER_HILOGD(MODULE_JS_NAPI, "%{public}s: enter", __func__);

    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("unlock", Unlock),
        DECLARE_NAPI_FUNCTION("isUsed", IsUsed),
        DECLARE_NAPI_FUNCTION("lock", Lock),
    };

    napi_value result = nullptr;
    napi_define_class(env, "RunningLock", NAPI_AUTO_LENGTH, RunningLockConstructor, nullptr,
        sizeof(desc) / sizeof(*desc), desc, &result);

    napi_create_reference(env, result, 1, &g_runningLockConstructor);
    napi_set_named_property(env, exports, "RunningLock", result);

    POWER_HILOGD(MODULE_JS_NAPI, "%{public}s: exit", __func__);
    return exports;
}

EXTERN_C_START
/*
 * function for module exports
 */
static napi_value RunningLockInit(napi_env env, napi_value exports)
{
    POWER_HILOGD(MODULE_JS_NAPI, "%{public}s: enter", __func__);

    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("createRunningLock", CreateRunningLock),
        DECLARE_NAPI_FUNCTION("isRunningLockTypeSupported", IsRunningLockTypeSupported),
    };

    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));

    CreateRunningLockClass(env, exports);
    CreateEnumRunningLockType(env, exports);
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
    .nm_filename = "runningLock",
    .nm_register_func = RunningLockInit,
    .nm_modname = "runningLock",
    .nm_priv = ((void *)0),
    .reserved = {0}
};

/*
 * Module registration
 */
extern "C" __attribute__((constructor)) void RegisterRunninglockModule(void)
{
    napi_module_register(&g_module);
}