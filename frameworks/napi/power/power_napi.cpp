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
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define SHUT_STAGE_FRAMEWORK_START 1
#define BOOT_DETECTOR_IOCTL_BASE 'B'
#define SET_SHUT_STAGE _IOW(BOOT_DETECTOR_IOCTL_BASE, 106, int)
#define SET_REBOOT _IOW(BOOT_DETECTOR_IOCTL_BASE, 109, int)

namespace OHOS {
namespace PowerMgr {
namespace {
constexpr uint32_t REBOOT_SHUTDOWN_MAX_ARGC = 1;
constexpr uint32_t WAKEUP_MAX_ARGC = 1;
constexpr uint32_t SET_MODE_CALLBACK_MAX_ARGC = 2;
constexpr uint32_t SET_MODE_PROMISE_MAX_ARGC = 1;
constexpr uint32_t SUSPEND_MAX_ARGC = 1;
constexpr uint32_t SET_SCREEN_OFFTIME_ARGC = 1;
constexpr uint32_t HIBERNATE_ARGC = 1;
constexpr int32_t INDEX_0 = 0;
constexpr int32_t INDEX_1 = 1;
constexpr int32_t RESTORE_DEFAULT_SCREENOFF_TIME = -1;
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
    size_t argc = SUSPEND_MAX_ARGC;
    napi_value argv[argc];
    NapiUtils::GetCallbackInfo(env, info, argc, argv);

    NapiErrors error;
    if (argc != SUSPEND_MAX_ARGC || !NapiUtils::CheckValueType(env, argv[INDEX_0], napi_boolean)) {
        if (!NapiUtils::CheckValueType(env, argv[INDEX_0], napi_undefined)) {
            std::string detail = NapiUtils::GetStringFromNapi(env, argv[INDEX_0]);
            if (!detail.empty()) {
                return error.ThrowError(env, PowerErrors::ERR_PARAM_INVALID);
            }
        }
    }
    bool isForce = false;
    napi_get_value_bool(env, argv[0], &isForce);

    if (isForce) {
        bool ret = g_powerMgrClient.ForceSuspendDevice();
        if (!ret) {
            POWER_HILOGE(FEATURE_WAKEUP, "Forcesuspend Device fail");
        }
    } else {
        PowerErrors code = g_powerMgrClient.SuspendDevice();
        if (code != PowerErrors::ERR_OK) {
            error.ThrowError(env, code);
        }
    }
    return nullptr;
}

napi_value PowerNapi::Hibernate(napi_env env, napi_callback_info info)
{
    size_t argc = HIBERNATE_ARGC;
    napi_value argv[argc];
    NapiUtils::GetCallbackInfo(env, info, argc, argv);

    NapiErrors error;
    if (argc != HIBERNATE_ARGC || !NapiUtils::CheckValueType(env, argv[INDEX_0], napi_boolean)) {
        if (!NapiUtils::CheckValueType(env, argv[INDEX_0], napi_undefined)) {
            std::string detail = NapiUtils::GetStringFromNapi(env, argv[INDEX_0]);
            if (!detail.empty()) {
                return error.ThrowError(env, PowerErrors::ERR_PARAM_INVALID);
            }
        }
    }
    bool clearMemory = false;
    napi_get_value_bool(env, argv[0], &clearMemory);

    bool ret = g_powerMgrClient.Hibernate(clearMemory);
    if (!ret) {
        POWER_HILOGE(FEATURE_WAKEUP, "Hibernate failed.");
        error.ThrowError(env, PowerErrors::ERR_FAILURE);
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

static void SetFrameworkBootStage(bool isReboot)
{
    int fd = open("/dev/bbox", O_WRONLY);
    if (fd < 0) {
        POWER_HILOGE(FEATURE_SHUTDOWN, "open /dev/bbox failed!");
        return;
    }
    int rebootFlag = isReboot ? 1 : 0;
    int ret = ioctl(fd, SET_REBOOT, &rebootFlag);
    if (ret < 0) {
        POWER_HILOGE(FEATURE_SHUTDOWN, "set reboot flag failed!");
        close(fd);
        return;
    }
    int stage = SHUT_STAGE_FRAMEWORK_START;
    ret = ioctl(fd, SET_SHUT_STAGE, &stage);
    if (ret < 0) {
        POWER_HILOGE(FEATURE_SHUTDOWN, "set shut stage failed!");
        close(fd);
    }
    return;
}

napi_value PowerNapi::RebootOrShutdown(napi_env env, napi_callback_info info, bool isReboot)
{
    size_t argc = REBOOT_SHUTDOWN_MAX_ARGC;
    napi_value argv[argc];
    SetFrameworkBootStage(isReboot);
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
            AsyncCallbackInfo* asyncInfo = reinterpret_cast<AsyncCallbackInfo*>(data);
            RETURN_IF(asyncInfo == nullptr);
            PowerErrors error = g_powerMgrClient.SetDeviceMode(asyncInfo->GetData().GetMode());
            asyncInfo->GetError().Error(error);
        },
        [](napi_env env, napi_status status, void* data) {
            AsyncCallbackInfo* asyncInfo = reinterpret_cast<AsyncCallbackInfo*>(data);
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
            AsyncCallbackInfo* asyncInfo = reinterpret_cast<AsyncCallbackInfo*>(data);
            RETURN_IF(asyncInfo == nullptr);
            PowerErrors error = g_powerMgrClient.SetDeviceMode(asyncInfo->GetData().GetMode());
            asyncInfo->GetError().Error(error);
        },
        [](napi_env env, napi_status status, void* data) {
            AsyncCallbackInfo* asyncInfo = reinterpret_cast<AsyncCallbackInfo*>(data);
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

void PowerNapi::AsyncWork(napi_env& env, std::unique_ptr<AsyncCallbackInfo>& asyncInfo, const std::string& resourceName,
    napi_async_execute_callback execute, napi_async_complete_callback complete)
{
    napi_value resource = nullptr;
    napi_create_string_utf8(env, resourceName.c_str(), NAPI_AUTO_LENGTH, &resource);
    napi_create_async_work(env, nullptr, resource, execute, complete,
        reinterpret_cast<void*>(asyncInfo.get()), &(asyncInfo->GetAsyncWork()));
    NAPI_CALL_RETURN_VOID(env, napi_queue_async_work_with_qos(env, asyncInfo->GetAsyncWork(), napi_qos_utility));
    asyncInfo.release();
}

napi_value PowerNapi::SetScreenOffTime(napi_env env, napi_callback_info info)
{
    size_t argc = SET_SCREEN_OFFTIME_ARGC;
    napi_value argv[argc];
    NapiUtils::GetCallbackInfo(env, info, argc, argv);

    NapiErrors error;
    if (argc != SET_SCREEN_OFFTIME_ARGC || !NapiUtils::CheckValueType(env, argv[INDEX_0], napi_number)) {
        POWER_HILOGE(FEATURE_WAKEUP, "check value type failed.");
        return error.ThrowError(env, PowerErrors::ERR_PARAM_INVALID);
    }

    int64_t timeout;
    if (napi_ok != napi_get_value_int64(env, argv[INDEX_0], &timeout)) {
        POWER_HILOGE(FEATURE_WAKEUP, "napi get int64 value failed.");
        return error.ThrowError(env, PowerErrors::ERR_PARAM_INVALID);
    }

    if (timeout == 0 || (timeout < 0 && timeout != RESTORE_DEFAULT_SCREENOFF_TIME)) {
        POWER_HILOGE(FEATURE_WAKEUP, "timeout is not right.");
        return error.ThrowError(env, PowerErrors::ERR_PARAM_INVALID);
    }

    bool ret;
    if (timeout == RESTORE_DEFAULT_SCREENOFF_TIME) {
        ret = g_powerMgrClient.RestoreScreenOffTime();
    } else {
        ret = g_powerMgrClient.OverrideScreenOffTime(timeout);
    }
    if (!ret) {
        POWER_HILOGE(FEATURE_WAKEUP, "SetScreenOffTime failed.");
        return error.ThrowError(env, PowerErrors::ERR_FAILURE);
    }
    return nullptr;
}

napi_value PowerNapi::IsStandby(napi_env env, napi_callback_info info)
{
    bool isStandby = false;
    PowerErrors code = g_powerMgrClient.IsStandby(isStandby);
    if (code == PowerErrors::ERR_OK) {
        napi_value napiValue;
        NAPI_CALL(env, napi_get_boolean(env, isStandby, &napiValue));
        return napiValue;
    }
    NapiErrors error;
    return error.ThrowError(env, code);
}
} // namespace PowerMgr
} // namespace OHOS
