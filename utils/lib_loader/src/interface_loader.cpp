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

#include "interface_loader.h"
#include "power_log.h"

namespace OHOS {
namespace PowerMgr {
static constexpr uint32_t MAX_INTERFACE_COUNT = 128;

InterfaceLoader::InterfaceLoader(const std::string& libPath, const std::vector<std::string>& symbolArr)
    : LibraryLoader(libPath)
{
    if (symbolArr.size() > MAX_INTERFACE_COUNT) {
        POWER_HILOGE(COMP_SVC, "Symbol count is invalid");
        return;
    }
    for (const std::string& s : symbolArr) {
        POWER_HILOGE(COMP_SVC, "InterfaceLoader add symbol: %{public}s", s.c_str());
        interfaces_.insert(std::make_pair(s, nullptr));
    }
}

InterfaceLoader::~InterfaceLoader() noexcept
{
    POWER_HILOGI(COMP_SVC, "InterfaceLoader instance destroyed");
}

bool InterfaceLoader::Init()
{
    std::unique_lock lock(shMutex_);
    if (isInited_) {
        POWER_HILOGW(COMP_SVC, "Interface already loaded");
        return true;
    }
    bool ret = LoadAllInterfaces();
    POWER_HILOGI(COMP_SVC, "Interface loading result: %{public}u", static_cast<uint32_t>(ret));
    if (ret) {
        isInited_ = true;
    }
    return ret;
}

void InterfaceLoader::DeInit()
{
    std::unique_lock lock(shMutex_);
    if (isInited_) {
        isInited_ = false;
        interfaces_.clear();
    }
}

void* InterfaceLoader::QueryInterface(const std::string& symbol) const
{
    std::shared_lock lock(shMutex_);
    if (!isInited_) {
        POWER_HILOGE(COMP_SVC, "Interface not loading or loading failed");
        return nullptr;
    }
    auto iter = interfaces_.find(symbol);
    if (iter == interfaces_.end() || iter->second == nullptr) {
        POWER_HILOGE(COMP_SVC, "%{public}s symbol not found", symbol.c_str());
        return nullptr;
    }
    return iter->second;
}

bool InterfaceLoader::LoadAllInterfaces()
{
    void* curFunc = nullptr;
    for (auto iter = interfaces_.begin(); iter != interfaces_.end(); ++iter) {
        curFunc = LoadInterface(iter->first.c_str());
        if (curFunc != nullptr) {
            iter->second = curFunc;
        }
    }
    return true;
}

} // namespace PowerMgr
} // namespace OHOS