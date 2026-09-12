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

#ifndef POWERMGR_POWER_MANAGER_PARAM_CACHER_H
#define POWERMGR_POWER_MANAGER_PARAM_CACHER_H

#include <parameters.h>

namespace OHOS {
namespace PowerMgr {
class ParamCacher {
public:
    static ParamCacher& Instance()
    {
        static ParamCacher instance;
        return instance;
    }

    bool IsNeverSleepDimEnabled() const
    {
        return neverSleepDimEnabled_;
    }

    int64_t GetActiveTimeBeforeLongTimeDim() const
    {
        return activeTimeBeforeLongTimeDim_;
    }

private:
    ParamCacher()
    {
        Init();
    }

    // Reads system parameters into the cached members. All subsequent const.* parameter
    // initialization should be added here so the logic stays in one place.
    void Init()
    {
        neverSleepDimEnabled_ = system::GetBoolParameter("const.power.never_sleep_dim_enabled", false);
        activeTimeBeforeLongTimeDim_ =
            static_cast<int64_t>(system::GetIntParameter("const.power.active_time_before_long_time_dim", -1));
    }

    ParamCacher(const ParamCacher&) = delete;
    ParamCacher& operator=(const ParamCacher&) = delete;

    // Combination of the two parameters below (only takes effect when auto screen-off is
    // disabled, i.e. displayOffTime < 0):
    //  - neverSleepDimEnabled_ == true:  stay AWAKE for activeTimeBeforeLongTimeDim_, then enter
    //    DIM and remain in DIM (never screen off).
    //  - neverSleepDimEnabled_ == false: stay AWAKE indefinitely (neither DIM nor OFF).
    // activeTimeBeforeLongTimeDim_ also independently defines the "long time dim" threshold when
    // displayOffTime > 0 (see PowerStateMachine::GetDimTime): if displayOffTime exceeds it, the
    // DIM period starts earlier (displayOffTime - activeTimeBeforeLongTimeDim_).
    bool neverSleepDimEnabled_ {false};
    int64_t activeTimeBeforeLongTimeDim_ {-1};
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_POWER_MANAGER_PARAM_CACHER_H
