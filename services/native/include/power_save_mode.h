/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include <system_ability.h>
#include "sp_singleton.h"
#include "ipc_object_stub.h"

#define RETURN_FLAG_FALSE (-1)
#define SLEEP_FILTER_VALUE 124

namespace OHOS {
namespace PowerMgr {
struct ModePolicy {
    int32_t id;
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
    bool GetValuePolicy(std::list<ModePolicy> &openPolicy, int32_t mode);
    bool GetRecoverPolicy(std::list<ModePolicy> &recoverPolicy, int32_t mode);
    bool GetFilterPolicy(std::list<ModePolicy> &policy, int32_t mode, int32_t value);
    int32_t GetSleepTime(int32_t mode);
    std::list<ModePolicy> GetLastMode();
    std::list<ModePolicy> SetLastMode(std::list<ModePolicy> &policy);

private:
    std::map<int32_t, std::list<ModePolicy>> policyCache_;
    bool StartXMlParse(std::string path);
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_POWER_MGR_PowerSaveMode_H
