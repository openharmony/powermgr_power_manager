/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef POWERMGR_POWER_HITRACE_H
#define POWERMGR_POWER_HITRACE_H

#include "hitrace_meter.h"

namespace OHOS {
namespace PowerMgr {
class PowerHitrace {
public:
    explicit PowerHitrace(const std::string& trace) : trace_(trace)
    {
        StartTrace(HITRACE_TAG_POWER, trace_.c_str());
    }

    ~PowerHitrace()
    {
        FinishTrace(HITRACE_TAG_POWER);
    }
private:
    std::string trace_;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_POWER_HITRACE__H