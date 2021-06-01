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

#include "power_mgr_dumper.h"

#include "power_common.h"
#include "power_mgr_service.h"

namespace OHOS {
namespace PowerMgr {
namespace {
const std::string ARGS_HELP = "-h";
const std::string ARGS_RUNNINGLOCK = "-runninglock";
}

bool PowerMgrDumper::Dump(const std::vector<std::string>& args, std::string& result)
{
    result.clear();
    auto argc = args.size();
    if ((argc == 0) || (args[0] == ARGS_HELP)) {
        ShowUsage(result);
        return true;
    }
    if (args[0] == ARGS_RUNNINGLOCK) {
        auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
        if (pms == nullptr) {
            return true;
        }
        auto runningLockMgr = pms->GetRunningLockMgr();
        if (runningLockMgr == nullptr) {
            return true;
        }
        runningLockMgr->DumpInfo(result);
    }
    return true;
}

void PowerMgrDumper::ShowUsage(std::string& result)
{
    result.append("Power manager dump options:\n")
        .append("  [-h] [-runninglock]\n")
        .append("  description of the cmd option:\n")
        .append("    -h: show this help.\n")
        .append("    -runninglock: show the all information of runninglock.\n");
}
} // namespace PowerMgr
} // namespace OHOS
