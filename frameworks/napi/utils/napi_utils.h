/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef POWER_NAPI_UTILS_H
#define POWER_NAPI_UTILS_H

#include <string>
#include <condition_variable>

#include "napi/native_api.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace PowerMgr {
class NapiUtils {
public:
    static napi_value GetCallbackInfo(napi_env& env, napi_callback_info& info, size_t& argc, napi_value argv[]);
    static std::string GetStringFromNapi(napi_env& env, napi_value& napiStr);
    static napi_ref CreateReference(napi_env& env, napi_value& value);
    static void ReleaseReference(napi_env& env, napi_ref& ref);
    static bool CheckValueType(napi_env& env, napi_value& value, napi_valuetype checkType);
};

class SyncContext {
public:
    void NotifyOne()
    {
        {
            std::unique_lock<std::mutex> lck(syncMutex_);
            isReady_ = true;
        }
        cv_.notify_one();
    }

    bool WaitFor(int32_t timeoutMs)
    {
        std::unique_lock<std::mutex> lck(syncMutex_);
        bool result = cv_.wait_for(lck, std::chrono::milliseconds(timeoutMs), [this] { return isReady_; });
        isReady_ = false;
        return result;
    }
private:
    bool isReady_ { false };
    std::mutex syncMutex_;
    std::condition_variable cv_;
};
} // namespace PowerMgr
} // namespace OHOS

#endif // POWER_NAPI_UTILS_H
