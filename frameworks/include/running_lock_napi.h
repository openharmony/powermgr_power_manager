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

#ifndef POWERMGR_RUNNING_LOCK_NAPI_H
#define POWERMGR_RUNNING_LOCK_NAPI_H

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "power_mgr_client.h"
#include "power_napi_context.h"

namespace OHOS {
namespace PowerMgr {
class RunningLockWrapper {
public:
    RunningLockWrapper(std::shared_ptr<RunningLock> lock) : lock_ptr(lock) {}
    ~RunningLockWrapper() = default;
    std::shared_ptr<RunningLock> lock_ptr;
};

class RunningLockNapi {
public:
    static napi_value InitRunningLockClass(napi_env env, napi_value exports);
    static bool CreateRunningLock(napi_env env, napi_callback_info info,
        napi_value* result, napi_value* error);
private:
    RunningLockNapi() = default;
    ~RunningLockNapi() = default;
    static napi_value Lock(napi_env env, napi_callback_info info);
    static napi_value IsUsed(napi_env env, napi_callback_info info);
    static napi_value Unlock(napi_env env, napi_callback_info info);

    static void RemoveRunningLock(napi_env env, void* finalize_data, void* finalize_hint);
    static napi_value ConstructRunningLockObject(napi_env env, napi_callback_info info);
    static bool GetNativeLock(napi_env env, napi_value object, RunningLockWrapper** lock);

    static napi_ref runningLockConstructor_;
};
} // namespace PowerMgr
} // namespace OHOS

#endif // POWERMGR_RUNNING_LOCK_NAPI_H
