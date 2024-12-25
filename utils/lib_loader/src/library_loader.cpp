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

#include "library_loader.h"
#include "power_log.h"

namespace OHOS {
namespace PowerMgr {
static constexpr uint32_t FAILURE_RETRY_TIMES = 3;

LibraryLoader::LibraryLoader(const std::string& libPath, int32_t flags) : libPath_(libPath)
{
    for (uint32_t i = 0; i < FAILURE_RETRY_TIMES; ++i) {
        libHandle_ = dlopen(libPath.c_str(), flags);
        if (libHandle_ != nullptr) {
            return;
        }
        POWER_HILOGE(COMP_SVC, "Failed to dlopen %{public}s, reason: %{public}s, try again(%{public}u)",
            libPath_.c_str(), dlerror(), i + 1);
    }
}

LibraryLoader::~LibraryLoader() noexcept
{
    if (libHandle_ == nullptr) {
        return;
    }
    for (uint32_t i = 0; i < FAILURE_RETRY_TIMES; ++i) {
        int32_t ret = dlclose(libHandle_);
        if (ret == 0) {
            break;
        }
        POWER_HILOGE(COMP_SVC, "Failed to dlclose %{public}s, reason: %{public}s, try again(%{public}u)",
            libPath_.c_str(), dlerror(), i + 1);
    }
    libHandle_ = nullptr;
}

void* LibraryLoader::LoadInterface(const char* symbolName)
{
    if (libHandle_ == nullptr || symbolName == nullptr) {
        POWER_HILOGE(COMP_SVC, "library handle or symbol name is invalid");
        return nullptr;
    }
    for (uint32_t i = 0; i < FAILURE_RETRY_TIMES; ++i) {
        void* funcPtr = dlsym(libHandle_, symbolName);
        if (funcPtr != nullptr) {
            return funcPtr;
        }
        POWER_HILOGE(COMP_SVC, "Failed to dlsym %{public}s, reason: %{public}s, try again(%{public}u)", symbolName,
            dlerror(), i + 1);
    }
    return nullptr;
}

} // namespace PowerMgr
} // namespace OHOS