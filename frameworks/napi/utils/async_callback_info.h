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

#ifndef POWERMGR_POWER_ASYNC_CALLBACK_INFO_H
#define POWERMGR_POWER_ASYNC_CALLBACK_INFO_H

#include "napi/native_api.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"

#include "napi_errors.h"
#include "power_mode_info.h"
#include "running_lock.h"
#include "running_lock_info.h"

namespace OHOS {
namespace PowerMgr {
class AsyncCallbackInfo {
public:
    void CallFunction(napi_env& env, napi_value results);
    void Release(napi_env& env);
    void CreatePromise(napi_env& env, napi_value& promise);
    void CreateCallback(napi_env& env, napi_value& callback);
    napi_async_work& GetAsyncWork()
    {
        return asyncWork_;
    }
    napi_deferred& GetDeferred()
    {
        return deferred_;
    }
    inline NapiErrors& GetError()
    {
        return error_;
    }
    class AsyncData {
    public:
        void SetMode(napi_env& env, napi_value& type);
        inline PowerMode GetMode() const
        {
            return powerMode_;
        }
        // runninglock
        void SetType(napi_env& env, napi_value& type);
        void SetName(napi_env& env, napi_value& name);
        PowerErrors CreateRunningLock();
        napi_value CreateInstanceForRunningLock(napi_env& env);
        inline void SetRunningLockInstance(napi_ref& instance)
        {
            napiRunningLockIns_ = instance;
        }

    private:
        // power
        PowerMode powerMode_ = PowerMode::NORMAL_MODE;
        // runninglock
        RunningLockType type_ = RunningLockType::RUNNINGLOCK_BUTT;
        std::string name_;
        std::shared_ptr<RunningLock> runningLock_ = nullptr;
        napi_ref napiRunningLockIns_ = nullptr;
    };
    inline AsyncData& GetData()
    {
        return data_;
    }

private:
    napi_ref callbackRef_ = nullptr;
    napi_async_work asyncWork_ = nullptr;
    napi_deferred deferred_ = nullptr;
    NapiErrors error_;
    AsyncData data_;
};
} // namespace PowerMgr
} // namespace OHOS

#endif // POWERMGR_POWER_ASYNC_CALLBACK_INFO_H