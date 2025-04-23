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

#ifndef POWERMGR_UTILS_LIB_LOADER_LIBRARY_LOADER_H
#define POWERMGR_UTILS_LIB_LOADER_LIBRARY_LOADER_H

#include <dlfcn.h>
#include <string>
#include <nocopyable.h>

namespace OHOS {
namespace PowerMgr {
class LibraryLoader : public NoCopyable {
public:
    explicit LibraryLoader(const std::string& libPath, int32_t flags = RTLD_LAZY | RTLD_NODELETE);
    virtual ~LibraryLoader() noexcept;
    void* LoadInterface(const char* symbolName);

private:
    const std::string libPath_;
    void* libHandle_ {nullptr};
};

} // namespace PowerMgr
} // namespace OHOS

#endif // POWERMGR_UTILS_LIB_LOADER_LIBRARY_LOADER_H