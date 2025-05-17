/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "power_hookmgr.h"
#include "power_log.h"

namespace OHOS {
namespace PowerMgr {
static const char* POWER_HOOKMGR_NAME = "power_hookmgr";

HOOK_MGR* GetPowerHookMgr()
{
    static HOOK_MGR* powerHookMgr = HookMgrCreate(POWER_HOOKMGR_NAME);
    return powerHookMgr;
}
} // namespace PowerMgr
} // namespace OHOS