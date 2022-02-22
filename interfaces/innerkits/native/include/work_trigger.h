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

#ifndef POWERMGR_WORKTRIGGER_H
#define POWERMGR_WORKTRIGGER_H

#include <string>
#include <list>

#include <parcel.h>

namespace OHOS {
namespace PowerMgr {
constexpr int32_t INVALID_PID = -1;
constexpr int32_t INVALID_UID = -1;

class WorkTrigger : public Parcelable {
public:
    WorkTrigger() = default;
    WorkTrigger(int32_t uid, const std::string& name = "", int32_t pid = 0)
        : uid_(uid), name_(name), pid_(pid) {}
    ~WorkTrigger() = default;

    void SetAbilityId(int32_t abilityId)
    {
        abilityId_ = abilityId;
    }

    void SetPid(int32_t pid)
    {
        pid_ = pid;
    }

    int32_t GetAbilityId() const
    {
        return abilityId_;
    }

    int32_t GetPid() const
    {
        return pid_;
    }

    int32_t GetUid() const
    {
        return uid_;
    }

    const std::string& GetName() const
    {
        return name_;
    }

    bool operator==(const WorkTrigger& other) const
    {
        return (uid_ == other.uid_) && (pid_ == other.pid_) && (abilityId_ == other.abilityId_) &&
            (name_.compare(other.name_) == 0);
    }

    bool ReadFromParcel(Parcel& parcel);
    bool Marshalling(Parcel& parcel) const override;
    static WorkTrigger* Unmarshalling(Parcel& parcel);

private:
    int32_t uid_{INVALID_UID};
    std::string name_;
    int32_t pid_{INVALID_PID};
    int32_t abilityId_ {0};
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_WORKTRIGGER_H
