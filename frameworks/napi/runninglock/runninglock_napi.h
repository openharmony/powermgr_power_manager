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

#ifndef POWERMGR_POWER_MANAGER_RUNNINGLOCK_NAPI_H
#define POWERMGR_POWER_MANAGER_RUNNINGLOCK_NAPI_H

#include <memory>

#include "napi/native_api.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"

#include "async_callback_info.h"
#include "running_lock.h"

namespace OHOS {
namespace PowerMgr {
class RunningLockNapi {
public:
    static napi_value Create(napi_env& env, napi_callback_info& info, napi_ref& napiRunningLockIns);
    static napi_value IsSupported(napi_env env, napi_callback_info info);
    static napi_value Hold(napi_env env, napi_callback_info info);
    static napi_value IsHolding(napi_env env, napi_callback_info info);
    static napi_value UnHold(napi_env env, napi_callback_info info);

private:
    static napi_value CreateAsyncCallback(
        napi_env& env, napi_value argv[], std::unique_ptr<AsyncCallbackInfo>& asyncInfo);
    static napi_value CreatePromise(napi_env& env, napi_value argv[], std::unique_ptr<AsyncCallbackInfo>& asyncInfo);

    static void AsyncWork(napi_env& env, std::unique_ptr<AsyncCallbackInfo>& asyncInfo, const std::string resourceName,
        napi_async_execute_callback execute, napi_async_complete_callback complete);
    static std::shared_ptr<RunningLock> UnwrapRunningLock(napi_env& env, napi_value& thisArg);
};
} // namespace PowerMgr
} // namespace OHOS

#endif // POWERMGR_POWER_MANAGER_RUNNINGLOCK_NAPI_H
