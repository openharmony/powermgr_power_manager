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

#ifndef POWERMGR_POWER_MANAGER_POWER_HDI_CALLBACK_H
#define POWERMGR_POWER_MANAGER_POWER_HDI_CALLBACK_H

#include <cstdint>
#include "v1_0/ipower_hdi_callback.h"

namespace OHOS {
namespace PowerMgr {
using namespace OHOS::HDI::Power::V1_0;
class PowerHdiCallback : public IPowerHdiCallback {
public:
    virtual ~PowerHdiCallback() {}

    int32_t OnSuspend() override;

    int32_t OnWakeup() override;
};
} // OHOS
} // PowerMgr
#endif // POWERMGR_POWER_MANAGER_POWER_HDI_CALLBACK_H
