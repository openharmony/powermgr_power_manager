/*
 * Copyright (c) 2024-2024 Huawei Device Co., Ltd.
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

#include "shutdown/takeover_info.h"
#include "power_common.h"
#include "string_ex.h"

namespace OHOS {
namespace PowerMgr {
bool TakeOverInfo::ReadFromParcel(Parcel& parcel)
{
    std::u16string readReason;
    bool readFlag = false;

    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(parcel, String16, readReason, false);
    reason_ = Str16ToStr8(readReason);
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(parcel, Bool, readFlag, false);
    intfParam_ = readFlag;
    return true;
}

bool TakeOverInfo::Marshalling(Parcel& parcel) const
{
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(parcel, String16, Str8ToStr16(reason_), false);
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(parcel, Bool, intfParam_, false);
    return true;
}

TakeOverInfo* TakeOverInfo::Unmarshalling(Parcel& parcel)
{
    TakeOverInfo* info = new TakeOverInfo();
    if (info == nullptr) {
        return nullptr;
    }
    if (!info->ReadFromParcel(parcel)) {
        delete info;
        return nullptr;
    }
    return info;
}
} // namespace PowerMgr
} // namespace OHOS