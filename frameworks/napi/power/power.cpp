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

#include <cstdint>
#include <memory>
#include <string>

#include "napi/native_api.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"

#include "power.h"
#include "power_log.h"
#include "power_mgr_client.h"
#include "power_state_machine_info.h"

namespace OHOS {
namespace PowerMgr {
namespace {
constexpr int REASON_MAX = 512;
constexpr int RESULT_SIZE = 2;
static PowerMgrClient& g_powerMgrClient = PowerMgrClient::GetInstance();
}

napi_value Power::RebootOrShutdown(napi_env env, napi_callback_info info, bool isReboot)
{
    size_t argc = 1;
    napi_value args[1] = {0};
    napi_value jsthis;
    void* data = nullptr;

    napi_status status = napi_get_cb_info(env, info, &argc, args, &jsthis, &data);
    NAPI_ASSERT(env, (status == napi_ok) && (argc >= 1), "Failed to get cb info");
    napi_valuetype type = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[0], &type));
    NAPI_ASSERT(env, type == napi_string, "Wrong argument type. string expected.");

    char reason[REASON_MAX] = {0};
    size_t reasonLen = 0;
    status = napi_get_value_string_utf8(env, args[0], reason, REASON_MAX - 1, &reasonLen);
    if (status != napi_ok) {
        POWER_HILOGE(FEATURE_SHUTDOWN, "Get shutdown reason failed");
        return nullptr;
    }
    POWER_HILOGD(FEATURE_SHUTDOWN, "reboot: %{public}d, reason: %{public}s", isReboot, reason);
    if (isReboot) {
        g_powerMgrClient.RebootDeviceForDeprecated(std::string(reason));
    } else {
        g_powerMgrClient.ShutDownDevice(std::string(reason));
    }
    return nullptr;
}

napi_value Power::ShutdownDevice(napi_env env, napi_callback_info info)
{
    return RebootOrShutdown(env, info, false);
}

napi_value Power::RebootDevice(napi_env env, napi_callback_info info)
{
    return RebootOrShutdown(env, info, true);
}

void Power::IsScreenOnCallBack(napi_env env, std::unique_ptr<PowerAsyncCallbackInfo>& asCallbackInfoPtr)
{
    napi_value resource = nullptr;
    napi_create_string_utf8(env, "IsScreenOn", NAPI_AUTO_LENGTH, &resource);
    napi_create_async_work(
        env, nullptr, resource,
        [](napi_env env, void* data) {
            PowerAsyncCallbackInfo* asyncCallbackInfo = (PowerAsyncCallbackInfo*)data;
            asyncCallbackInfo->screenOn = g_powerMgrClient.IsScreenOn();
            POWER_HILOGD(COMP_FWK, "Screen is %{public}s ", asyncCallbackInfo->screenOn ? "ON" : "OFF");
        },
        [](napi_env env, napi_status status, void* data) {
            PowerAsyncCallbackInfo* asyncCallbackInfo = (PowerAsyncCallbackInfo*)data;
            napi_value result[RESULT_SIZE] = {0};
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
        (void*)asCallbackInfoPtr.get(), &asCallbackInfoPtr->asyncWork);
    if (napi_ok == napi_queue_async_work(env, asCallbackInfoPtr->asyncWork)) {
        asCallbackInfoPtr.release();
    }
}

napi_value Power::IsScreenOn(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {0};
    napi_value jsthis;
    void* data = nullptr;

    napi_status status = napi_get_cb_info(env, info, &argc, args, &jsthis, &data);
    NAPI_ASSERT(env, (status == napi_ok), "Failed to get cb info");

    auto asyncCallbackInfo = new PowerAsyncCallbackInfo();
    if (asyncCallbackInfo == nullptr) {
        POWER_HILOGE(COMP_FWK, "Failed to create asyncCallbackInfo");
        return nullptr;
    }
    asyncCallbackInfo->env = env;

    std::unique_ptr<PowerAsyncCallbackInfo> asCallbackInfoPtr(asyncCallbackInfo);
    napi_valuetype type;
    if (argc == 1) {
        NAPI_CALL(env, napi_typeof(env, args[0], &type));
        if (type != napi_function) {
            POWER_HILOGE(COMP_FWK, "Wrong argument type. napi_function expected");
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
    IsScreenOnCallBack(env, asCallbackInfoPtr);
    return result;
}

napi_value Power::WakeupDevice(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {0};
    napi_value jsthis;
    void* data = nullptr;

    napi_status status = napi_get_cb_info(env, info, &argc, args, &jsthis, &data);
    NAPI_ASSERT(env, (status == napi_ok) && (argc >= 1), "Failed to get cb info");
    napi_valuetype type = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[0], &type));
    NAPI_ASSERT(env, type == napi_string, "Wrong argument type. string expected.");

    char reason[REASON_MAX] = {0};
    size_t reasonLen = 0;
    status = napi_get_value_string_utf8(env, args[0], reason, REASON_MAX - 1, &reasonLen);
    if (status != napi_ok) {
        POWER_HILOGE(FEATURE_WAKEUP, "Get wakeup reason failed");
        return nullptr;
    }
    POWER_HILOGD(FEATURE_WAKEUP, "Wakeup type: APPLICATION, reason: %{public}s", reason);
    g_powerMgrClient.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_APPLICATION, std::string(reason));
    return nullptr;
}

napi_value Power::SuspendDevice(napi_env env, napi_callback_info info)
{
    napi_value thisArg = nullptr;
    void* data = nullptr;

    napi_status status = napi_get_cb_info(env, info, nullptr, nullptr, &thisArg, &data);
    NAPI_ASSERT(env, (status == napi_ok), "Failed to get cb info");

    POWER_HILOGD(FEATURE_SUSPEND, "Suspend type: APPLICATION");
    g_powerMgrClient.SuspendDevice(SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    return nullptr;
}
} // namespace PowerMgr
} // namespace OHOS
