/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef POWERMGR_RUNNING_LOCK_INFO_H
#define POWERMGR_RUNNING_LOCK_INFO_H

#include <list>
#include <string>

#include <parcel.h>

#include "work_trigger.h"

namespace OHOS {
namespace PowerMgr {
/**
 * Runninglock acquisition type
 */
enum class RunningLockType : uint32_t {
    /**
     * RunningLock type: used to keep screen on.
     */
    RUNNINGLOCK_SCREEN,
    /**
     * RunningLock type: used to keep cpu running.
     */
    RUNNINGLOCK_BACKGROUND,
    /**
     * RunningLock type: used to keep the screen on/off when the proximity sensor is active.
     */
    RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL,

    RUNNINGLOCK_BUTT
};

using WorkTriggerList = std::list<std::shared_ptr<WorkTrigger>>;

/**
 * Maintain runninglock information.
 */
struct RunningLockInfo : public Parcelable {
    /**
     * RunningLock name: used to identify user of the runninglock.
     * You are advised to set it to a combination of information,
     * such as package name and class name and a unique name.
     */
    std::string name;

    /**
     * RunningLock type: used to identify the type of RunningLock.
     */
    RunningLockType type;

    /**
     * WorkTriggerList: generally used to manintain the information about the application,
     * which attempts to acquire the RunningLock.
     */
    WorkTriggerList workTriggerlist;
    RunningLockInfo() = default;
    RunningLockInfo(const std::string& namestr, RunningLockType locktype) : name(namestr), type(locktype) {}
    bool ReadFromParcel(Parcel& parcel);
    bool Marshalling(Parcel& parcel) const override;
    static RunningLockInfo* Unmarshalling(Parcel& parcel);
    static bool MarshallingWorkTriggerList(Parcel& parcel, const WorkTriggerList& list);
    static bool ReadFromParcelWorkTriggerList(Parcel& parcel, WorkTriggerList& list);
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_RUNNING_LOCK_INFO_H
