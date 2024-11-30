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

#include "power_ext_intf_wrapper.h"
#include "power_log.h"

namespace OHOS {
namespace PowerMgr {
namespace {
const char* POWER_MANAGER_EXT_PATH = "libpower_manager_ext.z.so";
const std::vector<std::string> ALL_POWER_EXT_INTF_SYMBOL = {
    "GetRebootCommand",
};
} // namespace

PowerExtIntfWrapper& PowerExtIntfWrapper::Instance()
{
    static PowerExtIntfWrapper instance(POWER_MANAGER_EXT_PATH, ALL_POWER_EXT_INTF_SYMBOL);
    return instance;
}

PowerExtIntfWrapper::ErrCode PowerExtIntfWrapper::GetRebootCommand(
    const std::string& rebootReason, std::string& rebootCmd) const
{
    POWER_HILOGE(COMP_SVC, "Enter GetRebootCommand wrapper");
    void* funcPtr = intfLoader_.QueryInterface("GetRebootCommand");
    if (funcPtr == nullptr) {
        return PowerExtIntfWrapper::ErrCode::ERR_NOT_FOUND;
    }
    auto getRebootCommandFunc = reinterpret_cast<const char* (*)(const std::string&)>(funcPtr);
    rebootCmd = getRebootCommandFunc(rebootReason);
    return PowerExtIntfWrapper::ErrCode::ERR_OK;
}

} // namespace PowerMgr
} // namespace OHOS