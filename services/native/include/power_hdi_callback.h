/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#ifndef POWERMGR_POWER_MANAGER_POWER_HDI_CALLBACK_H
#define POWERMGR_POWER_MANAGER_POWER_HDI_CALLBACK_H

#include <cstdint>
#include "v1_2/ipower_hdi_callback.h"
#include "v1_3/ipower_hdi_callback_ext.h"

namespace OHOS {
namespace PowerMgr {
using namespace OHOS::HDI::Power;
class PowerHdiCallback : public V1_2::IPowerHdiCallback {
public:
    virtual ~PowerHdiCallback() {}

    int32_t OnSuspend() override;

    int32_t OnWakeup() override;
};

class PowerHdiCallbackExt : public V1_3::IPowerHdiCallbackExt {
public:
    virtual ~PowerHdiCallbackExt() {}

    int32_t OnSuspendWithTag(const std::string& tag) override;

    int32_t OnWakeupWithTag(const std::string& tag) override;
};
} // OHOS
} // PowerMgr
#endif // POWERMGR_POWER_MANAGER_POWER_HDI_CALLBACK_H
