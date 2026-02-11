/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef POWER_PLUG_STATUS_MANAGER_H
#define POWER_PLUG_STATUS_MANAGER_H

#include <atomic>
#include <cstdint>

namespace OHOS {
namespace PowerMgr {

class PowerPlugStatusManager {
public:
    enum class PowerPlugStatus : int32_t {
        POWER_PLUG_IN_INVALID = -1,
        POWER_PLUG_IN_AC = 0,
        POWER_PLUG_IN_DC = 1,
    };

    void Init();
    void SetPowerPlugStatus(PowerPlugStatus status);
    bool IsPowerPluged();

private:
    std::atomic<PowerPlugStatus> powerPlugStatus_ {PowerPlugStatus::POWER_PLUG_IN_INVALID};
};

} //namespace PowerMgr
} //namespace OHOS
#endif // POWER_PLUG_STATUS_MANAGER_H