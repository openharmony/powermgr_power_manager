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

#include <memory>

#include "napi/native_api.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"

#include "power_log.h"
#include "running_lock_info.h"
#include "runninglock_entity.h"
#include "runninglock_interface.h"
#include "runninglock_napi.h"

using namespace OHOS::PowerMgr;

namespace {
static thread_local napi_ref g_napiRunningLockIns;
}

static napi_value RunningLockConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, &data);

    std::unique_ptr<RunningLockEntity> runningLockEntity = std::make_unique<RunningLockEntity>();
    napi_status status = napi_wrap(
        env, thisVar, runningLockEntity.get(),
        [](napi_env env, void* data, void* hint) {
            POWER_HILOGD(FEATURE_RUNNING_LOCK, "Destructor");
            auto entity = (RunningLockEntity*)data;
            delete entity;
        },
        nullptr, nullptr);
    if (status == napi_ok) {
        runningLockEntity.release();
    }

    return thisVar;
}

static napi_value CreateRunningLockClass(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        // Old interfaces deprecated since 9
        DECLARE_NAPI_FUNCTION("unlock", RunningLockInterface::Unlock),
        DECLARE_NAPI_FUNCTION("isUsed", RunningLockInterface::IsUsed),
        DECLARE_NAPI_FUNCTION("lock", RunningLockInterface::Lock),
        // New interfaces
        DECLARE_NAPI_FUNCTION("hold", RunningLockNapi::Hold),
        DECLARE_NAPI_FUNCTION("isHolding", RunningLockNapi::IsHolding),
        DECLARE_NAPI_FUNCTION("unhold", RunningLockNapi::UnHold),
    };

    napi_value result = nullptr;
    napi_define_class(env, "RunningLock", NAPI_AUTO_LENGTH, RunningLockConstructor, nullptr,
        sizeof(desc) / sizeof(*desc), desc, &result);

    napi_create_reference(env, result, 1, &g_napiRunningLockIns);
    napi_set_named_property(env, exports, "RunningLock", result);
    return exports;
}

static napi_value RunningLockTypeConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisArg = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &thisArg, &data);
    napi_value global = nullptr;
    napi_get_global(env, &global);
    return thisArg;
}

static napi_value CreateRunningLockType(napi_env env, napi_value exports)
{
    napi_value background = nullptr;
    napi_value proximityscreencontrol = nullptr;

    napi_create_int32(env, (int32_t)RunningLockType::RUNNINGLOCK_BACKGROUND, &background);
    napi_create_int32(env, (int32_t)RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL, &proximityscreencontrol);

    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("BACKGROUND", background),
        DECLARE_NAPI_STATIC_PROPERTY("PROXIMITY_SCREEN_CONTROL", proximityscreencontrol),
    };
    napi_value result = nullptr;
    napi_define_class(env, "RunningLockType", NAPI_AUTO_LENGTH, RunningLockTypeConstructor, nullptr,
        sizeof(desc) / sizeof(*desc), desc, &result);

    napi_set_named_property(env, exports, "RunningLockType", result);
    return exports;
}

static napi_value CreateRunningLock(napi_env env, napi_callback_info info)
{
    return RunningLockInterface::CreateRunningLock(env, info, g_napiRunningLockIns);
}

static napi_value Create(napi_env env, napi_callback_info info)
{
    return RunningLockNapi::Create(env, info, g_napiRunningLockIns);
}

EXTERN_C_START
/*
 * function for module exports
 */
static napi_value RunningLockInit(napi_env env, napi_value exports)
{
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "Initialize the RunningLock module");

    CreateRunningLockType(env, exports);
    CreateRunningLockClass(env, exports);
    napi_property_descriptor desc[] = {
        // Old interfaces deprecated since 9
        DECLARE_NAPI_FUNCTION("createRunningLock", CreateRunningLock),
        DECLARE_NAPI_FUNCTION("isRunningLockTypeSupported", RunningLockInterface::IsRunningLockTypeSupported),
        // New interfaces
        DECLARE_NAPI_FUNCTION("create", Create), DECLARE_NAPI_FUNCTION("isSupported", RunningLockNapi::IsSupported)};

    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "The initialization of the RunningLock module is complete");

    return exports;
}
EXTERN_C_END

/*
 * Module definition
 */
static napi_module g_module = {.nm_version = 1,
    .nm_flags = 0,
    .nm_filename = "runningLock",
    .nm_register_func = RunningLockInit,
    .nm_modname = "runningLock",
    .nm_priv = ((void*)0),
    .reserved = {0}};

/*
 * Module registration
 */
extern "C" __attribute__((constructor)) void RegisterRunninglockModule(void)
{
    napi_module_register(&g_module);
}
