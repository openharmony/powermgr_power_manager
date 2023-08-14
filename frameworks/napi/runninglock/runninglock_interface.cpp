/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "runninglock_interface.h"

#include <cstdint>
#include <memory>
#include <string>

#include "power_log.h"
#include "power_mgr_client.h"
#include "runninglock_entity.h"
#include "xpower_event_js.h"

namespace OHOS {
namespace PowerMgr {
namespace {
constexpr int RESULT_SIZE = 2;
constexpr int RUNNINGLOCK_NAME_MAX = 512;
constexpr int CREATRUNNINGLOCK_ARGC = 3;
constexpr int ISRUNNINGLOCKTYPESUPPORTED_ARGC = 2;
constexpr int32_t INDEX_0 = 0;
constexpr int32_t INDEX_1 = 1;
constexpr int32_t INDEX_2 = 2;
static PowerMgrClient& g_powerMgrClient = PowerMgrClient::GetInstance();
}

napi_value RunningLockInterface::CreateRunningLock(napi_env env, napi_callback_info info, napi_ref& napiRunningLock)
{
    size_t argc = CREATRUNNINGLOCK_ARGC;
    napi_value argv[CREATRUNNINGLOCK_ARGC] = {0};
    napi_value thisArg = nullptr;
    void* data = nullptr;

    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisArg, &data);
    NAPI_ASSERT(env, (status == napi_ok) && (argc >= CREATRUNNINGLOCK_ARGC - 1), "Failed to get cb info");

    std::unique_ptr<RunningLockAsyncInfo> asyncInfo = std::make_unique<RunningLockAsyncInfo>();
    if (asyncInfo == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "asyncInfo is nullptr");
        return nullptr;
    }
    asyncInfo->napiRunningLockIns = napiRunningLock;

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv[INDEX_0], &valueType);
    NAPI_ASSERT(env, valueType == napi_string, "The input parameter type is not string");
    char name[RUNNINGLOCK_NAME_MAX] = {0};
    napi_get_value_string_utf8(env, argv[INDEX_0], name, RUNNINGLOCK_NAME_MAX + 1, &asyncInfo->nameLen);
    asyncInfo->name = name;

    napi_typeof(env, argv[INDEX_1], &valueType);
    NAPI_ASSERT(env, valueType == napi_number, "The input parameter type is not number");
    int32_t type = static_cast<int32_t>(RunningLockType::RUNNINGLOCK_BUTT);
    napi_get_value_int32(env, argv[INDEX_1], &type);
    asyncInfo->type = static_cast<RunningLockType>(type);

    if (argc == CREATRUNNINGLOCK_ARGC) {
        napi_typeof(env, argv[INDEX_2], &valueType);
        NAPI_ASSERT(env, valueType == napi_function, "The input parameter type is not function");
        napi_create_reference(env, argv[INDEX_2], 1, &asyncInfo->callbackRef);
    }
    napi_value result = nullptr;
    if (asyncInfo->callbackRef == nullptr) {
        POWER_HILOGD(FEATURE_RUNNING_LOCK, "callbackRef is null");
        napi_create_promise(env, &asyncInfo->deferred, &result);
    } else {
        POWER_HILOGD(FEATURE_RUNNING_LOCK, "callbackRef is not null");
        napi_get_undefined(env, &result);
    }
    CreateRunningLockCallBack(env, asyncInfo);
    return result;
}

napi_value RunningLockInterface::IsRunningLockTypeSupported(napi_env env, napi_callback_info info)
{
    size_t argc = ISRUNNINGLOCKTYPESUPPORTED_ARGC;
    napi_value argv[ISRUNNINGLOCKTYPESUPPORTED_ARGC] = {0};
    napi_value thisArg = nullptr;
    void* data = nullptr;

    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisArg, &data);
    NAPI_ASSERT(env, (status == napi_ok) && (argc >= 1), "Failed to get cb info");

    std::unique_ptr<RunningLockAsyncInfo> asyncInfo = std::make_unique<RunningLockAsyncInfo>();
    if (asyncInfo == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "asyncInfo is nullptr");
        return nullptr;
    }

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv[INDEX_0], &valueType);
    NAPI_ASSERT(env, valueType == napi_number, "The input parameter type is not number");
    int32_t type = static_cast<int32_t>(RunningLockType::RUNNINGLOCK_BUTT);
    napi_get_value_int32(env, argv[INDEX_0], &type);
    asyncInfo->type = static_cast<RunningLockType>(type);

    if (argc == ISRUNNINGLOCKTYPESUPPORTED_ARGC) {
        napi_typeof(env, argv[INDEX_1], &valueType);
        NAPI_ASSERT(env, valueType == napi_function, "The input parameter type is not function");
        napi_create_reference(env, argv[INDEX_1], 1, &asyncInfo->callbackRef);
    }

    napi_value result = nullptr;
    if (asyncInfo->callbackRef == nullptr) {
        POWER_HILOGD(FEATURE_RUNNING_LOCK, "callbackRef is null");
        napi_create_promise(env, &asyncInfo->deferred, &result);
    } else {
        POWER_HILOGD(FEATURE_RUNNING_LOCK, "callbackRef is not null");
        napi_get_undefined(env, &result);
    }
    IsRunningLockTypeSupportedCallBack(env, asyncInfo);
    return result;
}

napi_value RunningLockInterface::Lock(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {0};
    napi_value thisArg = nullptr;
    void* data = nullptr;

    napi_status status = napi_get_cb_info(env, info, &argc, args, &thisArg, &data);
    NAPI_ASSERT(env, (status == napi_ok) && (argc >= 1), "Failed to get cb info");
    napi_valuetype type = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[0], &type));
    NAPI_ASSERT(env, type == napi_number, "Wrong argument type. number expected.");

    int32_t timeOut;
    status = napi_get_value_int32(env, args[0], &timeOut);
    if (status != napi_ok) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "napi_get_value_uint32 failed");
        return nullptr;
    }
    RunningLockEntity* entity = nullptr;
    status = napi_unwrap(env, thisArg, (void**)&entity);
    if (status != napi_ok) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Cannot unwrap for pointer");
        return nullptr;
    }
    if (entity == nullptr || entity->runningLock == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Entity runningLock is nullptr");
        return nullptr;
    }
    entity->runningLock->Lock(timeOut);
    OHOS::HiviewDFX::ReportXPowerJsStackSysEvent(env, "RunningLockNapi::Lock");
    return nullptr;
}

napi_value RunningLockInterface::IsUsed(napi_env env, napi_callback_info info)
{
    napi_value thisArg = nullptr;
    napi_value result = nullptr;
    void* data = nullptr;

    napi_status status = napi_get_cb_info(env, info, nullptr, nullptr, &thisArg, &data);
    NAPI_ASSERT(env, (status == napi_ok), "Failed to get cb info");
    napi_get_boolean(env, false, &result);
    RunningLockEntity* entity = nullptr;
    status = napi_unwrap(env, thisArg, (void**)&entity);
    if (status != napi_ok) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Cannot unwrap for pointer");
        return result;
    }
    if (entity == nullptr || entity->runningLock == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Entity runningLock is nullptr");
        return result;
    }
    bool isUsed = entity->runningLock->IsUsed();
    napi_get_boolean(env, isUsed, &result);
    return result;
}

napi_value RunningLockInterface::Unlock(napi_env env, napi_callback_info info)
{
    napi_value thisArg = nullptr;
    void* data = nullptr;

    napi_status status = napi_get_cb_info(env, info, nullptr, nullptr, &thisArg, &data);
    NAPI_ASSERT(env, (status == napi_ok), "Unlock: failed to get cb info");

    RunningLockEntity* entity = nullptr;
    status = napi_unwrap(env, thisArg, (void**)&entity);
    if (status != napi_ok) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Cannot unwrap for pointer");
        return nullptr;
    }
    if (entity == nullptr || entity->runningLock == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Entity runningLock is nullptr");
        return nullptr;
    }
    entity->runningLock->UnLock();
    return nullptr;
}

napi_value RunningLockInterface::CreateInstanceForRunningLock(napi_env env, RunningLockAsyncInfo* asyncInfo)
{
    napi_value cons = nullptr;
    napi_value instance = nullptr;
    napi_status callBackStatus;
    RunningLockEntity* entity = nullptr;

    if (asyncInfo->runningLock == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "RunningLock is nullptr");
        return nullptr;
    }
    callBackStatus = napi_get_reference_value(env, asyncInfo->napiRunningLockIns, &cons);
    if (callBackStatus != napi_ok) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "NAPI failed to create a reference value");
        return nullptr;
    }
    callBackStatus = napi_new_instance(env, cons, 0, nullptr, &instance);
    if (callBackStatus != napi_ok || instance == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "NAPI failed to create a reference");
        return nullptr;
    }
    callBackStatus = napi_unwrap(env, instance, (void**)&entity);
    if (callBackStatus != napi_ok || entity == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Cannot unwrap entity from instance");
        return nullptr;
    }
    entity->runningLock = asyncInfo->runningLock;
    return instance;
}

void RunningLockInterface::CreateRunningLockCallBack(napi_env env, std::unique_ptr<RunningLockAsyncInfo>& asyncInfo)
{
    napi_value resource = nullptr;
    napi_create_string_utf8(env, "createRunningLock", NAPI_AUTO_LENGTH, &resource);
    napi_create_async_work(
        env, nullptr, resource,
        [](napi_env env, void* data) {
            auto* asyncInfo = reinterpret_cast<RunningLockAsyncInfo*>(data);
            if (!IsTypeSupported(asyncInfo->type)) {
                auto type = static_cast<uint32_t>(asyncInfo->type);
                POWER_HILOGW(FEATURE_RUNNING_LOCK, "type=%{public}u not supported", type);
                return;
            }
            asyncInfo->runningLock = g_powerMgrClient.CreateRunningLock(std::string(asyncInfo->name), asyncInfo->type);
        },
        [](napi_env env, napi_status status, void* data) {
            auto* asyncInfo = reinterpret_cast<RunningLockAsyncInfo*>(data);
            napi_value result[RESULT_SIZE] = {0};
            result[1] = CreateInstanceForRunningLock(env, asyncInfo);
            if (result[1] == nullptr) {
                napi_value message = nullptr;
                napi_create_string_utf8(env, "runningLock create failed", NAPI_AUTO_LENGTH, &message);
                napi_create_error(env, nullptr, message, &result[0]);
            }
            if (asyncInfo->deferred) {
                if (result[1] != nullptr) {
                    napi_resolve_deferred(env, asyncInfo->deferred, result[1]);
                } else {
                    napi_reject_deferred(env, asyncInfo->deferred, result[0]);
                }
            } else {
                napi_value tmp = nullptr;
                napi_value callback = nullptr;
                napi_get_undefined(env, &result[0]);
                napi_get_reference_value(env, asyncInfo->callbackRef, &callback);
                napi_call_function(env, nullptr, callback, RESULT_SIZE, result, &tmp);
                napi_delete_reference(env, asyncInfo->callbackRef);
            }
            napi_delete_async_work(env, asyncInfo->asyncWork);
            delete asyncInfo;
        },
        reinterpret_cast<void*>(asyncInfo.get()), &asyncInfo->asyncWork);
    if (napi_ok == napi_queue_async_work_with_qos(env, asyncInfo->asyncWork, napi_qos_utility)) {
        asyncInfo.release();
    }
}

void RunningLockInterface::IsRunningLockTypeSupportedCallBack(
    napi_env env, std::unique_ptr<RunningLockAsyncInfo>& asyncInfo)
{
    napi_value resource = nullptr;
    napi_create_string_utf8(env, "isRunningLockTypeSupported", NAPI_AUTO_LENGTH, &resource);
    napi_create_async_work(
        env, nullptr, resource,
        [](napi_env env, void* data) {
            auto* asyncInfo = reinterpret_cast<RunningLockAsyncInfo*>(data);
            asyncInfo->isSupported = IsTypeSupported(asyncInfo->type);
            POWER_HILOGD(FEATURE_RUNNING_LOCK, "type=%{public}u, isSupported=%{public}s",
                static_cast<uint32_t>(asyncInfo->type), asyncInfo->isSupported ? "true" : "false");
        },
        [](napi_env env, napi_status status, void* data) {
            auto* asyncInfo = reinterpret_cast<RunningLockAsyncInfo*>(data);
            napi_value result[RESULT_SIZE] = {0};
            napi_get_boolean(env, asyncInfo->isSupported, &result[1]);
            if (asyncInfo->deferred) {
                napi_resolve_deferred(env, asyncInfo->deferred, result[1]);
            } else {
                napi_value tmp = nullptr;
                napi_value callback = nullptr;
                napi_get_reference_value(env, asyncInfo->callbackRef, &callback);
                napi_get_undefined(env, &result[0]);
                napi_call_function(env, nullptr, callback, RESULT_SIZE, result, &tmp);
                napi_delete_reference(env, asyncInfo->callbackRef);
            }
            napi_delete_async_work(env, asyncInfo->asyncWork);
            delete asyncInfo;
        },
        reinterpret_cast<void*>(asyncInfo.get()), &asyncInfo->asyncWork);
    if (napi_ok == napi_queue_async_work_with_qos(env, asyncInfo->asyncWork, napi_qos_utility)) {
        asyncInfo.release();
    }
}

bool RunningLockInterface::IsTypeSupported(RunningLockType type)
{
    return type == RunningLockType::RUNNINGLOCK_BACKGROUND ||
        type == RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL;
}
} // namespace PowerMgr
} // namespace OHOS
