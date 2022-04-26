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

#ifndef HDI_SERVICE_STATUS_LISTENER_H
#define HDI_SERVICE_STATUS_LISTENER_H

#include <functional>

#include <iservmgr_hdi.h>
#include "hdf_io_service_if.h"
#include "hdf_service_status.h"

namespace OHOS {
namespace PowerMgr {
class HdiServiceStatusListener : public OHOS::HDI::ServiceManager::V1_0::ServStatListenerStub {
public:
    using StatusCallback = std::function<void(const OHOS::HDI::ServiceManager::V1_0::ServiceStatus &)>;
    explicit HdiServiceStatusListener(StatusCallback callback) : callback_(std::move(callback)) {}
    ~HdiServiceStatusListener() = default;
    void OnReceive(const OHOS::HDI::ServiceManager::V1_0::ServiceStatus &status) override
    {
        callback_(status);
    }
private:
    StatusCallback callback_;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // HDI_SERVICE_STATUS_LISTENER_H