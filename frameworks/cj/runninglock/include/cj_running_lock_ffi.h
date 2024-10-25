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

#ifndef RUNNING_LOCK_FFI_H
#define RUNNING_LOCK_FFI_H

#include "native/ffi_remote_data.h"
#include "power_mgr_client.h"
#include "running_lock.h"
#include "running_lock_info.h"

extern "C" {
    FFI_EXPORT void FfiOHOSRunningLockHold(int64_t id, int32_t timeout, int32_t *ret);
    FFI_EXPORT bool FfiOHOSRunningLockIsHolding(int64_t id, int32_t *ret);
    FFI_EXPORT void FfiOHOSRunningLockUnhold(int64_t id, int32_t *ret);
    FFI_EXPORT bool FfiOHOSRunningLockIsSupported(int32_t num_type, int32_t *ret);
    FFI_EXPORT int64_t FfiOHOSRunningLockCreate(std::string name, int32_t num_type, int32_t *ret);
}

#endif // RUNNING_LOCK_FFI_H