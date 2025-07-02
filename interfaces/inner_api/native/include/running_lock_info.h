/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

namespace OHOS {
namespace PowerMgr {
/**
 * Runninglock acquisition type
 */
enum class RunningLockType : uint32_t {
    /**
     * RunningLock type: used to keep screen on.
     */
    RUNNINGLOCK_SCREEN = 0,
    /**
     * RunningLock type: used to keep cpu running.
     */
    RUNNINGLOCK_BACKGROUND = 1,
    /**
     * RunningLock type: used to keep the screen on/off when the proximity sensor is active.
     */
    RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL = 2,
    /**
     * RunningLock type: keeping image transmission when the current screen is off.
     */
    RUNNINGLOCK_COORDINATION = 4,
    /**
     * Background runningLock type, keeping phone background task active.
     */
    RUNNINGLOCK_BACKGROUND_PHONE = RUNNINGLOCK_BACKGROUND | 1 << 1,  // 0b00000011
    /**
     * Background runningLock type, keeping notification background task active.
     */
    RUNNINGLOCK_BACKGROUND_NOTIFICATION =  RUNNINGLOCK_BACKGROUND | 1 << 2, // 0b00000101
    /**
     * Background runningLock type, keeping audio background task active.
     */
    RUNNINGLOCK_BACKGROUND_AUDIO =   RUNNINGLOCK_BACKGROUND | 1 << 3, // 0b00001001
    /**
     * Background runningLock type, keeping sport background task active.
     */
    RUNNINGLOCK_BACKGROUND_SPORT =  RUNNINGLOCK_BACKGROUND | 1 << 4, // 0b00010001
    /**
     * Background runningLock type, keeping navigation background task active.
     */
    RUNNINGLOCK_BACKGROUND_NAVIGATION =  RUNNINGLOCK_BACKGROUND | 1 << 5, // 0b00100001
    /**
     * Background runningLock type, keeping common background task active.
     */
    RUNNINGLOCK_BACKGROUND_TASK =  RUNNINGLOCK_BACKGROUND | 1 << 6, // 0b01000001
    /**
     * RunningLock reserved type.
     */
    RUNNINGLOCK_BUTT
};

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

    std::string bundleName;
    int32_t pid = 0;
    int32_t uid = 0;
    RunningLockInfo() = default;
    RunningLockInfo(const std::string& namestr, RunningLockType locktype) : name(namestr), type(locktype) {}
    bool ReadFromParcel(Parcel& parcel);
    bool Marshalling(Parcel& parcel) const override;
    static RunningLockInfo* Unmarshalling(Parcel& parcel);
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_RUNNING_LOCK_INFO_H
