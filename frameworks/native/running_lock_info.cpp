/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "running_lock_info.h"

#include <string_ex.h>

#include "power_common.h"

namespace OHOS {
namespace PowerMgr {
bool RunningLockInfo::ReadFromParcelWorkTriggerList(Parcel& parcel, WorkTriggerList& list)
{
    uint32_t listSize = 0;
    if (!parcel.ReadUint32(listSize)) {
        return false;
    }
    POWER_HILOGD(MODULE_SERVICE, "RunningLockInfo::%{public}s listSize = %u", __func__, listSize);
    while (listSize > 0) {
        std::shared_ptr<WorkTrigger> workTrigger(parcel.ReadParcelable<WorkTrigger>());
        if (workTrigger == nullptr) {
            return false;
        }
        list.emplace_back(workTrigger);
        listSize--;
    }
    return true;
}

bool RunningLockInfo::ReadFromParcel(Parcel& parcel)
{
    uint32_t readType;
    READ_PARCEL_WITH_RET(parcel, String, name, false);
    READ_PARCEL_WITH_RET(parcel, Uint32, readType, false);
    type = static_cast<RunningLockType>(readType);
    return ReadFromParcelWorkTriggerList(parcel, workTriggerlist);
}

RunningLockInfo* RunningLockInfo::Unmarshalling(Parcel& parcel)
{
    POWER_HILOGD(MODULE_SERVICE, "RunningLockInfo::%{public}s enter.", __func__);
    RunningLockInfo* info = new RunningLockInfo();
    if (info == nullptr) {
        return nullptr;
    }
    if (!info->ReadFromParcel(parcel)) {
        delete info;
        return nullptr;
    }
    return info;
}

bool RunningLockInfo::MarshallingWorkTriggerList(Parcel& parcel, const WorkTriggerList& list)
{
    uint32_t listSize = static_cast<uint32_t>(list.size());
    if (!parcel.WriteUint32(listSize)) {
        POWER_HILOGE(MODULE_INNERKIT, "RunningLockInfo::%{public}s listSize = %u.", __func__, listSize);
        return false;
    }
    POWER_HILOGD(MODULE_INNERKIT, "RunningLockInfo::%{public}s listSize = %u.", __func__, listSize);

    for (const auto& templateVal : list) {
        if (templateVal == nullptr) {
            continue;
        }
        if (!parcel.WriteParcelable(templateVal.get())) {
            POWER_HILOGE(MODULE_INNERKIT, "RunningLockInfo::%{public}s templateVal->Marshalling failed.", __func__);
            return false;
        }
    }
    return true;
}

bool RunningLockInfo::Marshalling(Parcel& parcel) const
{
    WRITE_PARCEL_WITH_RET(parcel, String, name, false);
    WRITE_PARCEL_WITH_RET(parcel, Uint32, static_cast<uint32_t>(type), false);

    return MarshallingWorkTriggerList(parcel, workTriggerlist);
}

bool WorkTrigger::ReadFromParcel(Parcel& parcel)
{
    READ_PARCEL_WITH_RET(parcel, String, name_, false);
    READ_PARCEL_WITH_RET(parcel, Uint32, uid_, false);
    READ_PARCEL_WITH_RET(parcel, Uint32, pid_, false);
    READ_PARCEL_WITH_RET(parcel, Uint32, abilityId_, false);

    POWER_HILOGD(MODULE_SERVICE, "WorkTrigger::%{public}s name_ = %s, uid_ = %d, pid_ = %d, abilityId_ = %d.", __func__,
        name_.c_str(), uid_, pid_, abilityId_);
    return true;
}

WorkTrigger* WorkTrigger::Unmarshalling(Parcel& parcel)
{
    POWER_HILOGD(MODULE_SERVICE, "WorkTrigger::%{public}s enter.", __func__);
    WorkTrigger* workTrigger = new WorkTrigger();
    if (workTrigger == nullptr) {
        return nullptr;
    }
    if (!workTrigger->ReadFromParcel(parcel)) {
        delete workTrigger;
        return nullptr;
    }
    return workTrigger;
}

bool WorkTrigger::Marshalling(Parcel& parcel) const
{
    WRITE_PARCEL_WITH_RET(parcel, String, name_, false);
    WRITE_PARCEL_WITH_RET(parcel, Uint32, uid_, false);
    WRITE_PARCEL_WITH_RET(parcel, Uint32, pid_, false);
    WRITE_PARCEL_WITH_RET(parcel, Uint32, abilityId_, false);

    POWER_HILOGD(MODULE_INNERKIT, "WorkTrigger::%{public}s name_ = %s, uid_ = %d, pid_ = %d, abilityId_ = %d.",
        __func__, name_.c_str(), uid_, pid_, abilityId_);
    return true;
}
} // namespace PowerMgr
} // namespace OHOS
