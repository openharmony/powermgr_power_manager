/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef POWERMGR_POWER_MGR_PowerSaveMode_H
#define POWERMGR_POWER_MGR_PowerSaveMode_H

#include <string>
#include <list>
#include <map>

#include "ipc_object_stub.h"
#include "libxml/tree.h"
#include "sp_singleton.h"
#include "system_ability.h"

#define RETURN_FLAG_FALSE (-1)
#define SLEEP_FILTER_VALUE 102

namespace OHOS {
namespace PowerMgr {
struct ModePolicy {
    uint32_t id;
    int32_t value;
    int32_t recover_flag;
};
enum ValueProp {
    value,
    recover
};

class PowerSaveMode : public RefBase {
public:
    PowerSaveMode();
    ~PowerSaveMode()=default;
    int32_t GetSleepTime(int32_t mode);
    std::map<int32_t, std::list<ModePolicy>> GetPolicyCache()
    {
        return policyCache_;
    }

private:
    std::map<int32_t, std::list<ModePolicy>> policyCache_;
    bool StartXMlParse(std::string path);
    std::string GetProp(const xmlNodePtr& nodePtr, const std::string& key);
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_POWER_MGR_PowerSaveMode_H
