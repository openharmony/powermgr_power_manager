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
#include "power_mgr_client.h"
#include "running_lock.h"
#include "running_lock_info.h"

using namespace OHOS::FFI;

namespace OHOS {
namespace CJSystemapi {
namespace RunningLockFfi {
using namespace OHOS::PowerMgr;
extern "C" {
void FfiOHOSRunningLockHold(int64_t id, int32_t timeout, int32_t *ret)
{
    auto runningLock = FFI::FFIData::GetData<CJRunningLock>(id)->GetRunningLock();
    if (!runningLock) {
        *ret = static_cast<int32_t>(PowerErrors::ERR_PARAM_INVALID);
        return;
    }
    runningLock->Lock(timeout);
}

bool FfiOHOSRunningLockIsHolding(int64_t id, int32_t *ret)
{
    auto runningLock = FFI::FFIData::GetData<CJRunningLock>(id)->GetRunningLock();
    if (!runningLock) {
        *ret = static_cast<int32_t>(PowerErrors::ERR_PARAM_INVALID);
        return false;
    }
    bool result = runningLock->IsUsed();
    return result;
}

void FfiOHOSRunningLockUnhold(int64_t id, int32_t *ret)
{
    auto runningLock = FFI::FFIData::GetData<CJRunningLock>(id)->GetRunningLock();
    if (!runningLock) {
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
    int64_t id = native->GetID();
    if (native == nullptr) {
        *ret = static_cast<int32_t>(FFI::FFIData::GetData<CJRunningLock>(id)->GetError());
        return 0;
    }
    return id;
}
}

} // namespace RunningLockFfi
} // namespace CJSystemapi
} // namespace OHOS