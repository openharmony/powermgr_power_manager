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

#include "running_lock_info.h"

#include "new"
#include "power_log.h"
#include "power_common.h"
#include "string_ex.h"

namespace OHOS {
namespace PowerMgr {
bool RunningLockInfo::ReadFromParcelWorkTriggerList(Parcel& parcel, WorkTriggerList& list)
{
    uint32_t listSize = 0;
    if (!parcel.ReadUint32(listSize)) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "WriteUint32 is failed");
        return false;
    }
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "WorkTigger size: %{public}u", listSize);
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
    std::u16string u16Name;
    READ_PARCEL_WITH_RET(parcel, String16, u16Name, false);
    name = Str16ToStr8(u16Name);
    READ_PARCEL_WITH_RET(parcel, Uint32, readType, false);
    type = static_cast<RunningLockType>(readType);
    return ReadFromParcelWorkTriggerList(parcel, workTriggerlist);
}

RunningLockInfo* RunningLockInfo::Unmarshalling(Parcel& parcel)
{
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
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "WorkTigger size: %{public}u", listSize);
    if (!parcel.WriteUint32(listSize)) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "WriteUint32 is failed");
        return false;
    }

    for (const auto& templateVal : list) {
        if (templateVal == nullptr) {
            continue;
        }
        if (!parcel.WriteParcelable(templateVal.get())) {
            POWER_HILOGE(FEATURE_RUNNING_LOCK, "templateVal Marshalling failed");
            return false;
        }
    }
    return true;
}

bool RunningLockInfo::Marshalling(Parcel& parcel) const
{
    WRITE_PARCEL_WITH_RET(parcel, String16, Str8ToStr16(name), false);
    WRITE_PARCEL_WITH_RET(parcel, Uint32, static_cast<uint32_t>(type), false);

    return MarshallingWorkTriggerList(parcel, workTriggerlist);
}

bool WorkTrigger::ReadFromParcel(Parcel& parcel)
{
    READ_PARCEL_WITH_RET(parcel, String, name_, false);
    READ_PARCEL_WITH_RET(parcel, Int32, uid_, false);
    READ_PARCEL_WITH_RET(parcel, Int32, pid_, false);
    READ_PARCEL_WITH_RET(parcel, Int32, abilityId_, false);

    POWER_HILOGD(FEATURE_RUNNING_LOCK, "name_: %{public}s, uid_: %{public}d, \
        pid_: %{public}d, abilityId_: %{public}d", name_.c_str(), uid_, pid_, abilityId_);
    return true;
}

WorkTrigger* WorkTrigger::Unmarshalling(Parcel& parcel)
{
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
    WRITE_PARCEL_WITH_RET(parcel, Int32, uid_, false);
    WRITE_PARCEL_WITH_RET(parcel, Int32, pid_, false);
    WRITE_PARCEL_WITH_RET(parcel, Int32, abilityId_, false);

    POWER_HILOGD(FEATURE_RUNNING_LOCK, "name_: %{public}s, uid_: %{public}d, \
        pid_: %{public}d, abilityId_: %{public}d", name_.c_str(), uid_, pid_, abilityId_);
    return true;
}
} // namespace PowerMgr
} // namespace OHOS
