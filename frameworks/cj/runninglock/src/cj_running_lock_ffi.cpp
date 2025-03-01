/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cinttypes>
#include "cj_running_lock_ffi.h"
#include "cj_running_lock_impl.h"
#include "power_log.h"
#include "power_mgr_client.h"
#include "running_lock.h"
#include "running_lock_info.h"

using namespace OHOS::FFI;

namespace OHOS {
namespace CJSystemapi {
namespace RunningLockFfi {
using namespace OHOS::PowerMgr;

namespace {
const constexpr int32_t ERR_OTHER = -1;
const constexpr int64_t INVALID_FFIDATA_ID = 0;
}

extern "C" {
void FfiOHOSRunningLockHold(int64_t id, int32_t timeout, int32_t *ret)
{
    auto runningLock = FFI::FFIData::GetData<CJRunningLock>(id)->GetRunningLock();
    if (runningLock == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "runningLock is nullptr, id=%{public}" PRId64, id);
        *ret = static_cast<int32_t>(PowerErrors::ERR_PARAM_INVALID);
        return;
    }
    runningLock->Lock(timeout);
}

bool FfiOHOSRunningLockIsHolding(int64_t id, int32_t *ret)
{
    auto runningLock = FFI::FFIData::GetData<CJRunningLock>(id)->GetRunningLock();
    if (runningLock == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "runningLock is nullptr, id=%{public}" PRId64, id);
        *ret = static_cast<int32_t>(PowerErrors::ERR_PARAM_INVALID);
        return false;
    }
    return runningLock->IsUsed();
}

void FfiOHOSRunningLockUnhold(int64_t id, int32_t *ret)
{
    auto runningLock = FFI::FFIData::GetData<CJRunningLock>(id)->GetRunningLock();
    if (runningLock == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "runningLock is nullptr, id=%{public}" PRId64, id);
        *ret = static_cast<int32_t>(PowerErrors::ERR_PARAM_INVALID);
        return;
    }
    runningLock->UnLock();
}

bool FfiOHOSRunningLockIsSupported(int32_t num_type, int32_t *ret)
{
    RunningLockType type = RunningLockType(num_type);
    return (type == RunningLockType::RUNNINGLOCK_BACKGROUND) ||
           (type == RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL);
}

int64_t FfiOHOSRunningLockCreate(char *name, int32_t num_type, int32_t *ret)
{
    auto native = FFI::FFIData::Create<CJRunningLock>(name, num_type);
    if (native == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "create ffidata failed, name=%{public}s", name);
        *ret = ERR_OTHER;
        return INVALID_FFIDATA_ID;
    }
    *ret = static_cast<int32_t>(native->GetError());
    return native->GetID();
}
}

} // namespace RunningLockFfi
} // namespace CJSystemapi
} // namespace OHOS