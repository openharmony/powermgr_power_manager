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

#include "power_manager_napi.h"

#include <cstdio>
#include <cstdlib>
#include <string>
#include <cstring>
#include "power_common.h"
#include "running_lock_napi.h"

using namespace OHOS::PowerMgr;

static const uint8_t ARG_0 = 0;
static const uint8_t ARG_1 = 1;
static const uint8_t ARG_2 = 2;
static const uint8_t ARG_3 = 3;
static const uint8_t REASON_STRING_MAX = 128;
static const uint8_t LOCK_NAME_STRING_MAX = 128;

napi_ref PowerManagerNapi::runningLockTypeConstructor_ = nullptr;
napi_ref PowerManagerNapi::powerStateConstructor_ = nullptr;
napi_ref PowerManagerNapi::powerModeConstructor_ = nullptr;

void ShutdownContext::Init(napi_value* args, uint32_t argc)
{
    if (argc < 1) {
        return;
    }
    size_t out;
    char reason[REASON_STRING_MAX];
    napi_get_value_string_latin1(env_, args[ARG_0], reason, REASON_STRING_MAX, &out);
    reason_.copy(reason, strlen(reason), 0);
}

napi_value PowerManagerNapi::Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_FUNCTION("isRunningLockTypeSupported", IsRunningLockTypeSupported),
        DECLARE_NAPI_STATIC_FUNCTION("createRunningLock", CreateRunningLock),
        DECLARE_NAPI_STATIC_FUNCTION("shutownDevice", ShutdownDevice),
        DECLARE_NAPI_STATIC_FUNCTION("rebootDevice", RebootDevice),
        DECLARE_NAPI_STATIC_FUNCTION("isScreenOn", IsScreenOn),
        DECLARE_NAPI_STATIC_FUNCTION("getState", GetState),
        DECLARE_NAPI_STATIC_FUNCTION("getMode", GetMode),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));

    RunningLockNapi::InitRunningLockClass(env, exports);
    PowerManagerNapi::InitRunningLockType(env, exports);
    PowerManagerNapi::InitPowerState(env, exports);
    PowerManagerNapi::InitPowerMode(env, exports);

    return exports;
}

napi_value PowerManagerNapi::IsRunningLockTypeSupported(napi_env env, napi_callback_info info)
{
    POWER_HILOGD(MODULE_JS_NAPI, "enter");
    size_t argc = ARG_2;
    napi_value args[ARG_2] = { 0 };
    napi_value jsthis;
    void *data = nullptr;
    uint32_t result;

    napi_status status = napi_get_cb_info(env, info, &argc, args, &jsthis, &data);
    NAPI_ASSERT(env, (status == napi_ok) && (argc >= ARG_2 - 1), "Not enough parameters");

    napi_valuetype type;
    NAPI_CALL(env, napi_typeof(env, args[ARG_0], &type));
    NAPI_ASSERT(env, type == napi_number, "Wrong type of 1st parameter. number expected.");

    napi_get_value_uint32(env, args[ARG_0], &result);

    PowerNapiContext* context = new PowerNapiContext(env, args, argc, ARG_2 - 1, ARG_1);

    napi_value promise = context->GetPromise();
    context->StartAsyncWork("Power::IsRunningLockTypeSupported",
        [context, result] {
            bool ret = PowerMgrClient::GetInstance().IsRunningLockTypeSupported(result);
            napi_get_boolean(context->env_, ret, &context->outValue_);
            return true;
        },
        [](PowerNapiContext* object) { delete object; }
    );

    POWER_HILOGD(MODULE_JS_NAPI, "return");
    return promise;
}

napi_value PowerManagerNapi::CreateRunningLock(napi_env env, napi_callback_info info)
{
    POWER_HILOGD(MODULE_JS_NAPI, "enter");
    size_t argc = ARG_3;
    int argcnumber = 2;
    napi_value args[ARG_3] = { 0 };
    napi_value jsthis;
    void *data = nullptr;

    napi_status status = napi_get_cb_info(env, info, &argc, args, &jsthis, &data);
    NAPI_ASSERT(env, (status == napi_ok) && (argc >= argcnumber), "Not enough parameters");

    napi_valuetype type;
    NAPI_CALL(env, napi_typeof(env, args[ARG_0], &type));
    NAPI_ASSERT(env, type == napi_string, "Wrong type of 1st parameter. string expected.");
    NAPI_CALL(env, napi_typeof(env, args[ARG_1], &type));
    NAPI_ASSERT(env, type == napi_number, "Wrong type of 2st parameter. number expected.");

    char name[LOCK_NAME_STRING_MAX];
    size_t size;
    napi_get_value_string_latin1(env, args[ARG_0], name, LOCK_NAME_STRING_MAX, &size);
    uint32_t lockType;
    napi_get_value_uint32(env, args[1], &lockType);
    NAPI_ASSERT(env, lockType < static_cast<uint32_t>(RunningLockType::RUNNINGLOCK_BUTT),
        "RunningLock Type is not supported");

    PowerNapiContext* context = new PowerNapiContext(env, args, argc, ARG_3 - 1, ARG_1);

    napi_value promise = context->GetPromise();
    context->StartAsyncWork("Power::CreateRunningLock",
        [env, info, context] {
            return RunningLockNapi::CreateRunningLock(env, info, &context->outValue_);
        },
        [](PowerNapiContext* object) { delete object; }
    );

    POWER_HILOGD(MODULE_JS_NAPI, "return");
    return promise;
}

napi_value PowerManagerNapi::ShutdownDevice(napi_env env, napi_callback_info info)
{
    POWER_HILOGD(MODULE_JS_NAPI, "enter");
    uint32_t argc = ARG_2;
    napi_value args[ARG_2] = { 0 };
    napi_value jsthis;
    void *data = nullptr;

    napi_status status = napi_get_cb_info(env, info, &argc, args, &jsthis, &data);
    NAPI_ASSERT(env, (status == napi_ok) && (argc >= ARG_1), "Not enough parameters");

    napi_valuetype type;
    NAPI_CALL(env, napi_typeof(env, args[ARG_0], &type));
    NAPI_ASSERT(env, type == napi_string, "Wrong argument type. string expected.");

    ShutdownContext* context = new ShutdownContext(env, args, argc, ARG_2 - 1, ARG_0);

    napi_value promise = context->GetPromise();
    context->StartAsyncWork("Power::ShutdownDevice",
        [context] {
            PowerMgrClient::GetInstance().ShutDownDevice(context->reason_);
            return true;
        },
        [](PowerNapiContext* object) { delete object; }
    );

    POWER_HILOGD(MODULE_JS_NAPI, "return");
    return promise;
}

napi_value PowerManagerNapi::RebootDevice(napi_env env, napi_callback_info info)
{
    POWER_HILOGD(MODULE_JS_NAPI, "enter");
    uint32_t argc = ARG_2;
    napi_value args[ARG_2] = { 0 };
    napi_value jsthis;
    void *data = nullptr;

    napi_status status = napi_get_cb_info(env, info, &argc, args, &jsthis, &data);
    NAPI_ASSERT(env, (status == napi_ok) && (argc >= 1), "Not enough parameters");

    napi_valuetype type;
    NAPI_CALL(env, napi_typeof(env, args[0], &type));
    NAPI_ASSERT(env, type == napi_string, "Wrong argument type. string expected.");

    ShutdownContext* context = new ShutdownContext(env, args, argc, ARG_2 - 1, ARG_0);

    napi_value promise = context->GetPromise();
    context->StartAsyncWork("Power::RebootDevice",
        [context] {
            PowerMgrClient::GetInstance().RebootDevice(context->reason_);
            return true;
        },
        [](PowerNapiContext* object) { delete object; }
    );

    POWER_HILOGD(MODULE_JS_NAPI, "return");
    return promise;
}

napi_value PowerManagerNapi::IsScreenOn(napi_env env, napi_callback_info info)
{
    POWER_HILOGD(MODULE_JS_NAPI, "enter");
    size_t argc = ARG_1;
    napi_value args[ARG_1] = { 0 };
    napi_value jsthis;
    void *data = nullptr;

    napi_status status = napi_get_cb_info(env, info, &argc, args, &jsthis, &data);
    NAPI_ASSERT(env, (status == napi_ok) && (argc >= 0), "Bad parameters");

    PowerNapiContext* context = new PowerNapiContext(env, args, argc, ARG_1 - 1, ARG_1);

    napi_value promise = context->GetPromise();
    context->StartAsyncWork("Power::IsScreenOn",
        [context] {
            bool ret = PowerMgrClient::GetInstance().IsScreenOn();
            napi_get_boolean(context->env_, ret, &context->outValue_);
            return true;
        },
        [](PowerNapiContext* object) { delete object; }
    );

    POWER_HILOGD(MODULE_JS_NAPI, "return");
    return promise;
}

napi_value PowerManagerNapi::GetState(napi_env env, napi_callback_info info)
{
    POWER_HILOGD(MODULE_JS_NAPI, "enter");
    size_t argc = ARG_1;
    napi_value args[ARG_1] = { 0 };
    napi_value jsthis;
    void *data = nullptr;

    napi_status status = napi_get_cb_info(env, info, &argc, args, &jsthis, &data);
    NAPI_ASSERT(env, (status == napi_ok) && (argc >= 0), "Bad parameters");

    PowerNapiContext* context = new PowerNapiContext(env, args, argc, ARG_1 - 1, ARG_1);

    napi_value promise = context->GetPromise();
    context->StartAsyncWork("Power::GetState",
        [context] {
            PowerState state = PowerMgrClient::GetInstance().GetState();
            napi_create_uint32(context->env_, static_cast<uint32_t>(state), &context->outValue_);
            return true;
        },
        [](PowerNapiContext* object) { delete object; }
    );

    POWER_HILOGD(MODULE_JS_NAPI, "return");
    return promise;
}

napi_value PowerManagerNapi::GetMode(napi_env env, napi_callback_info info)
{
    POWER_HILOGD(MODULE_JS_NAPI, "enter");
    size_t argc = ARG_1;
    napi_value args[ARG_1] = { 0 };
    napi_value jsthis;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, args, &jsthis, &data);
    NAPI_ASSERT(env, (status == napi_ok) && (argc >= 0), "Bad parameters");
    PowerNapiContext* context = new PowerNapiContext(env, args, argc, 0, 1);
    napi_value promise = context->GetPromise();
    context->StartAsyncWork("Power::GetState",
        [context] {
            uint32_t mode = PowerMgrClient::GetInstance().GetDeviceMode();
            napi_create_uint32(context->env_, mode, &context->outValue_);
            return true;
        },
        [context](PowerNapiContext* object) { delete context; }
    );
    POWER_HILOGD(MODULE_JS_NAPI, "return");
    return promise;
}

napi_value PowerManagerNapi::InitRunningLockType(napi_env env, napi_value exports)
{
    napi_value background;
    napi_value proximity;
    int32_t refCount = 1;

    napi_create_uint32(env, static_cast<uint32_t>(RunningLockType::RUNNINGLOCK_BACKGROUND), &background);
    napi_create_uint32(env,  static_cast<uint32_t>(RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL), &proximity);

    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("BACKGROUND", background),
        DECLARE_NAPI_STATIC_PROPERTY("PROXIMITY_SCREEN_CONTROL", proximity),
    };

    napi_value result = nullptr;
    napi_define_class(env, "RunningLockType", NAPI_AUTO_LENGTH, EnumRunningLockTypeConstructor, nullptr,
        sizeof(desc) / sizeof(*desc), desc, &result);
    napi_create_reference(env, result, refCount, &runningLockTypeConstructor_);
    napi_set_named_property(env, exports, "RunningLockType", result);

    return exports;
}

napi_value PowerManagerNapi::InitPowerState(napi_env env, napi_value exports)
{
    napi_value awake;
    napi_value inactive;
    napi_value sleep;
    int32_t refCount = 1;

    napi_create_uint32(env, static_cast<uint32_t>(PowerState::AWAKE), &awake);
    napi_create_uint32(env, static_cast<uint32_t>(PowerState::INACTIVE), &inactive);
    napi_create_uint32(env, static_cast<uint32_t>(PowerState::SLEEP), &sleep);

    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("AWAKE", awake),
        DECLARE_NAPI_STATIC_PROPERTY("INACTIVE", inactive),
        DECLARE_NAPI_STATIC_PROPERTY("SLEEP", sleep),
    };

    napi_value result = nullptr;
    napi_define_class(env, "PowerState", NAPI_AUTO_LENGTH, EnumPowerStateConstructor, nullptr,
        sizeof(desc) / sizeof(*desc), desc, &result);
    napi_create_reference(env, result, refCount, &powerStateConstructor_);
    napi_set_named_property(env, exports, "PowerState", result);

    return exports;
}

napi_value PowerManagerNapi::InitPowerMode(napi_env env, napi_value exports)
{
    napi_value normal;
    napi_value power_save;
    int32_t refCount = 1;

    napi_create_uint32(env, 0, &normal);
    napi_create_uint32(env, 1, &power_save);

    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("NORMAL", normal),
        DECLARE_NAPI_STATIC_PROPERTY("POWER_SAVE", power_save),
    };

    napi_value result = nullptr;
    napi_define_class(env, "PowerMode", NAPI_AUTO_LENGTH, EnumPowerModeConstructor, nullptr,
        sizeof(desc) / sizeof(*desc), desc, &result);
    napi_create_reference(env, result, refCount, &powerModeConstructor_);
    napi_set_named_property(env, exports, "PowerMode", result);

    return exports;
}

napi_value PowerManagerNapi::EnumRunningLockTypeConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisArg = nullptr;
    void *data = nullptr;

    napi_get_cb_info(env, info, nullptr, nullptr, &thisArg, &data);

    return thisArg;
}

napi_value PowerManagerNapi::EnumPowerStateConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisArg = nullptr;
    void *data = nullptr;

    napi_get_cb_info(env, info, nullptr, nullptr, &thisArg, &data);

    return thisArg;
}

napi_value PowerManagerNapi::EnumPowerModeConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisArg = nullptr;
    void *data = nullptr;

    napi_get_cb_info(env, info, nullptr, nullptr, &thisArg, &data);

    return thisArg;
}

EXTERN_C_START
/*
 * function for module exports
 */
static napi_value PowerInit(napi_env env, napi_value exports)
{
    POWER_HILOGD(MODULE_JS_NAPI, "enter");

    napi_value ret = PowerManagerNapi::Init(env, exports);

    POWER_HILOGD(MODULE_JS_NAPI, "return");

    return ret;
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
extern "C" __attribute__((constructor)) void RegisterModule(void)
{
    napi_module_register(&g_module);
}
