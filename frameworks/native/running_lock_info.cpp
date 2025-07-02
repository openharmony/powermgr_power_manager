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
bool RunningLockInfo::ReadFromParcel(Parcel& parcel)
{
    uint32_t readType;
    std::u16string u16Name;
    std::u16string u16BundleName;
    int32_t readPid;
    int32_t readUid;
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(parcel, String16, u16Name, false);
    name = Str16ToStr8(u16Name);
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(parcel, Uint32, readType, false);
    type = static_cast<RunningLockType>(readType);
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(parcel, String16, u16BundleName, false);
    bundleName = Str16ToStr8(u16BundleName);
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(parcel, Int32, readPid, false);
    pid = readPid;
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(parcel, Int32, readUid, false);
    uid = readUid;
    return true;
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

bool RunningLockInfo::Marshalling(Parcel& parcel) const
{
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(parcel, String16, Str8ToStr16(name), false);
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(parcel, Uint32, static_cast<uint32_t>(type), false);
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(parcel, String16, Str8ToStr16(bundleName), false);
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(parcel, Int32, static_cast<int32_t>(pid), false);
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(parcel, Int32, static_cast<int32_t>(uid), false);
    return true;
}
} // namespace PowerMgr
} // namespace OHOS
