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
constexpr int INVALID_PID = -1;
constexpr int INVALID_UID = -1;

class WorkTrigger : public Parcelable {
public:
    WorkTrigger() = default;
    WorkTrigger(uint32_t uid, const std::string& name = "", uint32_t pid = 0)
        : uid_(uid), name_(name), pid_(pid) {}
    ~WorkTrigger() = default;

    void SetAbilityId(int abilityId)
    {
        abilityId_ = abilityId;
    }

    void SetPid(int pid)
    {
        pid_ = pid;
    }

    int GetAbilityId() const
    {
        return abilityId_;
    }

    int GetPid() const
    {
        return pid_;
    }

    int GetUid() const
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
    uint32_t uid_{INVALID_UID};
    std::string name_;
    uint32_t pid_{INVALID_PID};
    uint32_t abilityId_ {0};
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_WORKTRIGGER_H
