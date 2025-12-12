/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef POWER_SHUTDOWN_CALLBACK_H
#define POWER_SHUTDOWN_CALLBACK_H

#include "ohos.power.proj.hpp"
#include "ohos.power.impl.hpp"
#include "taihe/runtime.hpp"
#include "shutdown/async_shutdown_callback_stub.h"

namespace OHOS {
namespace PowerMgr {
using namespace taihe;
class PowerShutdownCallback : public AsyncShutdownCallbackStub {
public:
    PowerShutdownCallback() = default;
    virtual ~PowerShutdownCallback() = default;
    void CreateCallback(callback_view<void(bool)> shutdownCb);
    void OnAsyncShutdownOrReboot(bool isReboot) override;

private:
    optional<callback<void(bool)>> callback_;
    std::mutex callbackMutex_;
};
} // namespace PowerMgr
} // namespace OHOS

#endif // POWER_SHUTDOWN_CALLBACK_H