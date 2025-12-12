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
#include "power_shutdown_callback.h"
#include "power_log.h"

namespace OHOS {
namespace PowerMgr {
void PowerShutdownCallback::CreateCallback(callback_view<void(bool)> shutdownCb)
{
    std::lock_guard lock(callbackMutex_);
    callback_ = optional<callback<void(bool)>>{std::in_place, shutdownCb};
}

void PowerShutdownCallback::OnAsyncShutdownOrReboot(bool isReboot)
{
    POWER_HILOGI(FEATURE_SHUTDOWN, "OnShutdownOrReboot, isReboot: %{public}d", static_cast<int32_t>(isReboot));
    std::lock_guard lock(callbackMutex_);
    if (callback_.has_value()) {
        callback_.value()(isReboot);
    }
}
} // namespace PowerMgr
} // namespace OHOS