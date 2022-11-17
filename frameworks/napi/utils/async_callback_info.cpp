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

#include "async_callback_info.h"
#include "napi_utils.h"
#include "power_log.h"
#include "power_mgr_client.h"
#include "runninglock_entity.h"

namespace OHOS {
namespace PowerMgr {
void AsyncCallbackInfo::CallFunction(napi_env& env, napi_value results)
{
    napi_value callback = nullptr;
    if (napi_ok != napi_get_reference_value(env, callbackRef_, &callback)) {
        POWER_HILOGW(COMP_FWK, "Failed to get a callback reference");
        return;
    }
    const int32_t maxArgc = 2;
    napi_value argv[] = {error_.GetNapiError(env), results};
    napi_value result;
    if (napi_ok != napi_call_function(env, nullptr, callback, maxArgc, argv, &result)) {
        POWER_HILOGW(COMP_FWK, "Failed to call the callback function");
    }
}

void AsyncCallbackInfo::Release(napi_env& env)
{
    NapiUtils::ReleaseReference(env, callbackRef_);
    if (asyncWork_ != nullptr) {
        napi_delete_async_work(env, asyncWork_);
        asyncWork_ = nullptr;
    }
    deferred_ = nullptr;
}

void AsyncCallbackInfo::CreatePromise(napi_env& env, napi_value& promise)
{
    if (napi_ok != napi_create_promise(env, &deferred_, &promise)) {
        POWER_HILOGW(COMP_FWK, "napi_create_promise failure");
    }
}

void AsyncCallbackInfo::CreateCallback(napi_env& env, napi_value& callback)
{
    callbackRef_ = NapiUtils::CreateReference(env, callback);
}

void AsyncCallbackInfo::AsyncData::SetMode(napi_env& env, napi_value& mode)
{
    uint32_t result;
    napi_get_value_uint32(env, mode, &result);
    powerMode_ = static_cast<PowerMode>(result);
}

void AsyncCallbackInfo::AsyncData::SetType(napi_env& env, napi_value& type)
{
    int32_t result;
    napi_get_value_int32(env, type, &result);
    type_ = static_cast<RunningLockType>(result);
}

void AsyncCallbackInfo::AsyncData::SetName(napi_env& env, napi_value& name)
{
    name_ = NapiUtils::GetStringFromNapi(env, name);
}

PowerErrors AsyncCallbackInfo::AsyncData::CreateRunningLock()
{
    runningLock_ = PowerMgrClient::GetInstance().CreateRunningLock(name_, type_);
    return PowerMgrClient::GetInstance().GetError();
}

napi_value AsyncCallbackInfo::AsyncData::CreateInstanceForRunningLock(napi_env& env)
{
    if (runningLock_ == nullptr || napiRunningLockIns_ == nullptr) {
        POWER_HILOGW(FEATURE_RUNNING_LOCK, "RunningLock is nullptr");
        return nullptr;
    }
    napi_value cons = nullptr;
    napi_status status = napi_get_reference_value(env, napiRunningLockIns_, &cons);
    if (status != napi_ok) {
        POWER_HILOGW(FEATURE_RUNNING_LOCK, "NAPI failed to create a reference value");
        return nullptr;
    }
    napi_value instance = nullptr;
    status = napi_new_instance(env, cons, 0, nullptr, &instance);
    if (status != napi_ok) {
        POWER_HILOGW(FEATURE_RUNNING_LOCK, "NAPI failed to create a reference");
        return nullptr;
    }

    RunningLockEntity* entity = nullptr;
    status = napi_unwrap(env, instance, (void**)&entity);
    if (status != napi_ok || entity == nullptr) {
        POWER_HILOGW(FEATURE_RUNNING_LOCK, "Cannot unwrap entity from instance");
        return nullptr;
    }
    entity->runningLock = runningLock_;
    return instance;
}
} // namespace PowerMgr
} // namespace OHOS