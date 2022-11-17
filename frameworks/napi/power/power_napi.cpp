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

#include "power_napi.h"

#include "napi_errors.h"
#include "napi_utils.h"
#include "power_common.h"
#include "power_log.h"
#include "power_mgr_client.h"

namespace OHOS {
namespace PowerMgr {
namespace {
constexpr uint32_t REBOOT_SHUTDOWN_MAX_ARGC = 1;
constexpr uint32_t WAKEUP_MAX_ARGC = 1;
constexpr uint32_t SET_MODE_CALLBACK_MAX_ARGC = 2;
constexpr uint32_t SET_MODE_PROMISE_MAX_ARGC = 1;
constexpr int32_t INDEX_0 = 0;
constexpr int32_t INDEX_1 = 1;
static PowerMgrClient& g_powerMgrClient = PowerMgrClient::GetInstance();
} // namespace
napi_value PowerNapi::Shutdown(napi_env env, napi_callback_info info)
{
    return RebootOrShutdown(env, info, false);
}

napi_value PowerNapi::Reboot(napi_env env, napi_callback_info info)
{
    return RebootOrShutdown(env, info, true);
}

napi_value PowerNapi::IsActive(napi_env env, napi_callback_info info)
{
    bool isScreen = g_powerMgrClient.IsScreenOn();
    napi_value napiValue;
    NAPI_CALL(env, napi_get_boolean(env, isScreen, &napiValue));
    return napiValue;
}

napi_value PowerNapi::Wakeup(napi_env env, napi_callback_info info)
{
    size_t argc = WAKEUP_MAX_ARGC;
    napi_value argv[argc];
    NapiUtils::GetCallbackInfo(env, info, argc, argv);

    NapiErrors error;
    if (argc != WAKEUP_MAX_ARGC || !NapiUtils::CheckValueType(env, argv[INDEX_0], napi_string)) {
        return error.ThrowError(env, PowerErrors::ERR_PARAM_INVALID);
    }

    std::string detail = NapiUtils::GetStringFromNapi(env, argv[INDEX_0]);
    POWER_HILOGD(FEATURE_WAKEUP, "Wakeup type: APPLICATION, reason: %{public}s", detail.c_str());
    PowerErrors code = g_powerMgrClient.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_APPLICATION, detail);
    if (code != PowerErrors::ERR_OK) {
        error.ThrowError(env, code);
    }
    return nullptr;
}

napi_value PowerNapi::Suspend(napi_env env, napi_callback_info info)
{
    NapiErrors error;
    PowerErrors code = g_powerMgrClient.SuspendDevice();
    if (code != PowerErrors::ERR_OK) {
        error.ThrowError(env, code);
    }
    return nullptr;
}

napi_value PowerNapi::SetPowerMode(napi_env env, napi_callback_info info)
{
    size_t argc = SET_MODE_CALLBACK_MAX_ARGC;
    napi_value argv[argc];
    NapiUtils::GetCallbackInfo(env, info, argc, argv);

    NapiErrors error;
    if (argc != SET_MODE_CALLBACK_MAX_ARGC && argc != SET_MODE_PROMISE_MAX_ARGC) {
        return error.ThrowError(env, PowerErrors::ERR_PARAM_INVALID);
    }

    std::unique_ptr<AsyncCallbackInfo> asyncInfo = std::make_unique<AsyncCallbackInfo>();
    RETURN_IF_WITH_RET(asyncInfo == nullptr, nullptr);
    // callback
    if (argc == SET_MODE_CALLBACK_MAX_ARGC) {
        POWER_HILOGD(FEATURE_POWER_MODE, "Call setPowerMode callback");
        return SetPowerModeCallback(env, argv, asyncInfo);
    }

    // promise
    POWER_HILOGD(FEATURE_POWER_MODE, "Call setPowerMode promise");
    return SetPowerModePromise(env, argv, asyncInfo);
}

napi_value PowerNapi::GetPowerMode(napi_env env, napi_callback_info info)
{
    PowerMode mode = g_powerMgrClient.GetDeviceMode();
    napi_value napiValue;
    NAPI_CALL(env, napi_create_uint32(env, static_cast<uint32_t>(mode), &napiValue));
    return napiValue;
}

napi_value PowerNapi::RebootOrShutdown(napi_env env, napi_callback_info info, bool isReboot)
{
    size_t argc = REBOOT_SHUTDOWN_MAX_ARGC;
    napi_value argv[argc];
    NapiUtils::GetCallbackInfo(env, info, argc, argv);

    NapiErrors error;
    if (argc != REBOOT_SHUTDOWN_MAX_ARGC || !NapiUtils::CheckValueType(env, argv[INDEX_0], napi_string)) {
        return error.ThrowError(env, PowerErrors::ERR_PARAM_INVALID);
    }

    std::string reason = NapiUtils::GetStringFromNapi(env, argv[INDEX_0]);
    POWER_HILOGD(FEATURE_SHUTDOWN, "reboot: %{public}d, reason: %{public}s", isReboot, reason.c_str());

    PowerErrors code;
    if (isReboot) {
        code = g_powerMgrClient.RebootDevice(reason);
    } else {
        code = g_powerMgrClient.ShutDownDevice(reason);
    }
    if (code != PowerErrors::ERR_OK) {
        error.ThrowError(env, code);
    }

    return nullptr;
}

napi_value PowerNapi::SetPowerModeCallback(
    napi_env& env, napi_value argv[], std::unique_ptr<AsyncCallbackInfo>& asyncInfo)
{
    bool isNum = NapiUtils::CheckValueType(env, argv[INDEX_0], napi_number);
    bool isFunc = NapiUtils::CheckValueType(env, argv[INDEX_1], napi_function);
    if (!isNum || !isFunc) {
        POWER_HILOGW(FEATURE_POWER_MODE, "isNum: %{public}d, isFunc: %{public}d", isNum, isFunc);
        return asyncInfo->GetError().ThrowError(env, PowerErrors::ERR_PARAM_INVALID);
    }

    asyncInfo->GetData().SetMode(env, argv[INDEX_0]);
    asyncInfo->CreateCallback(env, argv[INDEX_1]);

    AsyncWork(
        env, asyncInfo, "SetPowerModeCallback",
        [](napi_env env, void* data) {
            AsyncCallbackInfo* asyncInfo = (AsyncCallbackInfo*)data;
            RETURN_IF(asyncInfo == nullptr);
            PowerErrors error = g_powerMgrClient.SetDeviceMode(asyncInfo->GetData().GetMode());
            asyncInfo->GetError().Error(error);
        },
        [](napi_env env, napi_status status, void* data) {
            AsyncCallbackInfo* asyncInfo = (AsyncCallbackInfo*)data;
            RETURN_IF(asyncInfo == nullptr);
            asyncInfo->CallFunction(env, nullptr);
            asyncInfo->Release(env);
            delete asyncInfo;
        });
    return nullptr;
}

napi_value PowerNapi::SetPowerModePromise(
    napi_env& env, napi_value argv[], std::unique_ptr<AsyncCallbackInfo>& asyncInfo)
{
    bool isNum = NapiUtils::CheckValueType(env, argv[INDEX_0], napi_number);
    if (!isNum) {
        POWER_HILOGW(FEATURE_POWER_MODE, "isNum: %{public}d", isNum);
        return asyncInfo->GetError().ThrowError(env, PowerErrors::ERR_PARAM_INVALID);
    }
    napi_value promise;
    asyncInfo->CreatePromise(env, promise);
    RETURN_IF_WITH_RET(promise == nullptr, nullptr);
    asyncInfo->GetData().SetMode(env, argv[INDEX_0]);

    AsyncWork(
        env, asyncInfo, "SetPowerModePromise",
        [](napi_env env, void* data) {
            AsyncCallbackInfo* asyncInfo = (AsyncCallbackInfo*)data;
            RETURN_IF(asyncInfo == nullptr);
            PowerErrors error = g_powerMgrClient.SetDeviceMode(asyncInfo->GetData().GetMode());
            asyncInfo->GetError().Error(error);
        },
        [](napi_env env, napi_status status, void* data) {
            AsyncCallbackInfo* asyncInfo = (AsyncCallbackInfo*)data;
            RETURN_IF(asyncInfo == nullptr);
            if (asyncInfo->GetError().IsError()) {
                napi_reject_deferred(env, asyncInfo->GetDeferred(), asyncInfo->GetError().GetNapiError(env));
            } else {
                napi_value undefined;
                napi_get_undefined(env, &undefined);
                napi_resolve_deferred(env, asyncInfo->GetDeferred(), undefined);
            }
            asyncInfo->Release(env);
            delete asyncInfo;
        });
    return promise;
}

void PowerNapi::AsyncWork(napi_env& env, std::unique_ptr<AsyncCallbackInfo>& asyncInfo, const std::string resourceName,
    napi_async_execute_callback execute, napi_async_complete_callback complete)
{
    napi_value resource = nullptr;
    napi_create_string_utf8(env, resourceName.c_str(), NAPI_AUTO_LENGTH, &resource);
    napi_create_async_work(
        env, nullptr, resource, execute, complete, (void*)asyncInfo.get(), &(asyncInfo->GetAsyncWork()));
    NAPI_CALL_RETURN_VOID(env, napi_queue_async_work(env, asyncInfo->GetAsyncWork()));
    asyncInfo.release();
}
} // namespace PowerMgr
} // namespace OHOS
