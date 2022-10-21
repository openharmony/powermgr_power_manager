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

#ifndef POWERMGR_POWER_MANAGER_RUNNINGLOCK_INTERFACE_H
#define POWERMGR_POWER_MANAGER_RUNNINGLOCK_INTERFACE_H

#include <string>

#include "napi/native_api.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"

#include "running_lock.h"
#include "running_lock_info.h"

namespace OHOS {
namespace PowerMgr {
struct RunningLockAsyncInfo {
    napi_async_work asyncWork = nullptr;
    napi_ref callbackRef = nullptr;
    napi_deferred deferred = nullptr;
    std::string name;
    size_t nameLen = 0;
    RunningLockType type = RunningLockType::RUNNINGLOCK_BUTT;
    bool isSupported = false;
    std::shared_ptr<RunningLock> runningLock = nullptr;
    napi_ref napiRunningLockIns = nullptr;
};

class RunningLockInterface {
public:
    static napi_value CreateRunningLock(napi_env env, napi_callback_info info, napi_ref& napiRunningLock);
    static napi_value IsRunningLockTypeSupported(napi_env env, napi_callback_info info);
    static napi_value Lock(napi_env env, napi_callback_info info);
    static napi_value IsUsed(napi_env env, napi_callback_info info);
    static napi_value Unlock(napi_env env, napi_callback_info info);

private:
    static napi_value CreateInstanceForRunningLock(napi_env env, RunningLockAsyncInfo* asyncInfo);
    static void CreateRunningLockCallBack(napi_env env, RunningLockAsyncInfo* asyncInfo);
    static void IsRunningLockTypeSupportedCallBack(napi_env env, RunningLockAsyncInfo* asyncInfo);
};
} // namespace PowerMgr
} // namespace OHOS

#endif // POWERMGR_POWER_MANAGER_RUNNINGLOCK_INTERFACE_H
