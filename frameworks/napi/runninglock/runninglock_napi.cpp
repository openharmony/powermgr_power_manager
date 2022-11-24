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

#include "runninglock_napi.h"

#include <memory>

#include "napi_errors.h"
#include "napi_utils.h"
#include "power_common.h"
#include "power_log.h"
#include "runninglock_entity.h"

namespace OHOS {
namespace PowerMgr {
namespace {
constexpr uint32_t CREATE_PROMISE_MAX_ARGC = 2;
constexpr uint32_t CREATE_CALLBACK_MAX_ARGC = 3;
constexpr uint32_t ISSUPPORTED_MAX_ARGC = 1;
constexpr uint32_t HOLD_MAX_ARGC = 1;
constexpr int32_t INDEX_0 = 0;
constexpr int32_t INDEX_1 = 1;
constexpr int32_t INDEX_2 = 2;
}

napi_value RunningLockNapi::Create(napi_env& env, napi_callback_info& info, napi_ref& napiRunningLockIns)
{
    size_t argc = CREATE_CALLBACK_MAX_ARGC;
    napi_value argv[argc];
    NapiUtils::GetCallbackInfo(env, info, argc, argv);

    NapiErrors error;
    if (argc != CREATE_CALLBACK_MAX_ARGC && argc != CREATE_PROMISE_MAX_ARGC) {
        return error.ThrowError(env, PowerErrors::ERR_PARAM_INVALID);
    }

    std::unique_ptr<AsyncCallbackInfo> asyncInfo = std::make_unique<AsyncCallbackInfo>();
    RETURN_IF_WITH_RET(asyncInfo == nullptr, nullptr);
    asyncInfo->GetData().SetRunningLockInstance(napiRunningLockIns);
    // callback
    if (argc == CREATE_CALLBACK_MAX_ARGC) {
        return CreateAsyncCallback(env, argv, asyncInfo);
    }

    // promise
    return CreatePromise(env, argv, asyncInfo);
}

napi_value RunningLockNapi::IsSupported(napi_env env, napi_callback_info info)
{
    size_t argc = ISSUPPORTED_MAX_ARGC;
    napi_value argv[argc];
    NapiUtils::GetCallbackInfo(env, info, argc, argv);

    NapiErrors error;
    if (argc != ISSUPPORTED_MAX_ARGC || !NapiUtils::CheckValueType(env, argv[INDEX_0], napi_number)) {
        return error.ThrowError(env, PowerErrors::ERR_PARAM_INVALID);
    }

    int32_t numType;
    napi_get_value_int32(env, argv[INDEX_0], &numType);
    RunningLockType type = static_cast<RunningLockType>(numType);

    bool isSupported = (type == RunningLockType::RUNNINGLOCK_BACKGROUND) ||
        (type == RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL);

    napi_value result;
    napi_get_boolean(env, isSupported, &result);
    return result;
}

napi_value RunningLockNapi::Hold(napi_env env, napi_callback_info info)
{
    size_t argc = HOLD_MAX_ARGC;
    napi_value argv[argc];
    napi_value thisArg = NapiUtils::GetCallbackInfo(env, info, argc, argv);
    NapiErrors error;
    if (argc != HOLD_MAX_ARGC || !NapiUtils::CheckValueType(env, argv[INDEX_0], napi_number)) {
        return error.ThrowError(env, PowerErrors::ERR_PARAM_INVALID);
    }

    uint32_t timeOut;
    if (napi_ok != napi_get_value_uint32(env, argv[INDEX_0], &timeOut)) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "napi_get_value_uint32 failed");
        return nullptr;
    }
    auto runningLock = UnwrapRunningLock(env, thisArg);
    RETURN_IF_WITH_RET(runningLock == nullptr, nullptr);
    runningLock->Lock(timeOut);

    return nullptr;
}

napi_value RunningLockNapi::IsHolding(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value thisArg = NapiUtils::GetCallbackInfo(env, info, argc, nullptr);
    auto runningLock = UnwrapRunningLock(env, thisArg);
    RETURN_IF_WITH_RET(runningLock == nullptr, nullptr);
    bool isUsed = runningLock->IsUsed();
    napi_value result;
    napi_get_boolean(env, isUsed, &result);
    return result;
}

napi_value RunningLockNapi::UnHold(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value thisArg = NapiUtils::GetCallbackInfo(env, info, argc, nullptr);
    auto runningLock = UnwrapRunningLock(env, thisArg);
    RETURN_IF_WITH_RET(runningLock == nullptr, nullptr);
    runningLock->UnLock();
    return nullptr;
}

napi_value RunningLockNapi::CreateAsyncCallback(
    napi_env& env, napi_value argv[], std::unique_ptr<AsyncCallbackInfo>& asyncInfo)
{
    bool isStr = NapiUtils::CheckValueType(env, argv[INDEX_0], napi_string);
    bool isNum = NapiUtils::CheckValueType(env, argv[INDEX_1], napi_number);
    bool isFunc = NapiUtils::CheckValueType(env, argv[INDEX_2], napi_function);
    if (!isStr || !isNum || !isFunc) {
        POWER_HILOGD(
            FEATURE_RUNNING_LOCK, "isStr: %{public}d, isNum: %{public}d, isFunc: %{public}d", isStr, isNum, isFunc);
        return asyncInfo->GetError().ThrowError(env, PowerErrors::ERR_PARAM_INVALID);
    }
    asyncInfo->GetData().SetName(env, argv[INDEX_0]);
    asyncInfo->GetData().SetType(env, argv[INDEX_1]);
    asyncInfo->CreateCallback(env, argv[INDEX_2]);

    AsyncWork(
        env, asyncInfo, "CreateAsyncCallback",
        [](napi_env env, void* data) {
            AsyncCallbackInfo* asyncInfo = (AsyncCallbackInfo*)data;
            RETURN_IF(asyncInfo == nullptr);
            auto error = asyncInfo->GetData().CreateRunningLock();
            asyncInfo->GetError().Error(error);
        },
        [](napi_env env, napi_status status, void* data) {
            AsyncCallbackInfo* asyncInfo = (AsyncCallbackInfo*)data;
            RETURN_IF(asyncInfo == nullptr);
            napi_value result = asyncInfo->GetData().CreateInstanceForRunningLock(env);
            asyncInfo->CallFunction(env, result);
            asyncInfo->Release(env);
            delete asyncInfo;
        });
    return nullptr;
}

napi_value RunningLockNapi::CreatePromise(
    napi_env& env, napi_value argv[], std::unique_ptr<AsyncCallbackInfo>& asyncInfo)
{
    bool isStr = NapiUtils::CheckValueType(env, argv[INDEX_0], napi_string);
    bool isNum = NapiUtils::CheckValueType(env, argv[INDEX_1], napi_number);
    if (!isStr || !isNum) {
        POWER_HILOGW(FEATURE_RUNNING_LOCK, "isStr: %{public}d, isNum: %{public}d", isStr, isNum);
        return asyncInfo->GetError().ThrowError(env, PowerErrors::ERR_PARAM_INVALID);
    }

    napi_value promise;
    asyncInfo->CreatePromise(env, promise);
    RETURN_IF_WITH_RET(promise == nullptr, nullptr);
    asyncInfo->GetData().SetName(env, argv[INDEX_0]);
    asyncInfo->GetData().SetType(env, argv[INDEX_1]);

    AsyncWork(
        env, asyncInfo, "CreatePromise",
        [](napi_env env, void* data) {
            AsyncCallbackInfo* asyncInfo = (AsyncCallbackInfo*)data;
            RETURN_IF(asyncInfo == nullptr);
            auto error = asyncInfo->GetData().CreateRunningLock();
            asyncInfo->GetError().Error(error);
        },
        [](napi_env env, napi_status status, void* data) {
            AsyncCallbackInfo* asyncInfo = (AsyncCallbackInfo*)data;
            RETURN_IF(asyncInfo == nullptr);
            if (asyncInfo->GetError().IsError()) {
                napi_reject_deferred(env, asyncInfo->GetDeferred(), asyncInfo->GetError().GetNapiError(env));
            } else {
                napi_value result = asyncInfo->GetData().CreateInstanceForRunningLock(env);
                napi_resolve_deferred(env, asyncInfo->GetDeferred(), result);
            }
            asyncInfo->Release(env);
            delete asyncInfo;
        });
    return promise;
}

void RunningLockNapi::AsyncWork(napi_env& env, std::unique_ptr<AsyncCallbackInfo>& asyncInfo,
    const std::string resourceName, napi_async_execute_callback execute, napi_async_complete_callback complete)
{
    napi_value resource = nullptr;
    napi_create_string_utf8(env, resourceName.c_str(), NAPI_AUTO_LENGTH, &resource);
    napi_create_async_work(
        env, nullptr, resource, execute, complete, (void*)asyncInfo.get(), &asyncInfo->GetAsyncWork());
    NAPI_CALL_RETURN_VOID(env, napi_queue_async_work(env, asyncInfo->GetAsyncWork()));
    asyncInfo.release();
}

std::shared_ptr<RunningLock> RunningLockNapi::UnwrapRunningLock(napi_env& env, napi_value& thisArg)
{
    RunningLockEntity* entity = nullptr;
    if (napi_ok != napi_unwrap(env, thisArg, (void**)&entity)) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Cannot unwrap for pointer");
        return nullptr;
    }
    if (entity == nullptr || entity->runningLock == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Entity runningLock is nullptr");
        return nullptr;
    }
    return entity->runningLock;
}
} // namespace PowerMgr
} // namespace OHOS
