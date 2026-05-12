/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef POWER_CLI_COMMAND_H
#define POWER_CLI_COMMAND_H

#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <cJSON.h>
#include "power_errors.h"

namespace OHOS {
namespace PowerMgr {

class PowerCliCommand {
public:
    explicit PowerCliCommand(int argc, char* argv[]);
    int Execute();

private:
    int argc_;
    char** argv_;
    std::string command_;
    std::vector<std::string> args_;

    static const std::unordered_map<std::string,
        std::function<int(const std::vector<std::string>&)>> COMMAND_TABLE;
    static const std::unordered_map<std::string, std::function<void()>> COMMAND_HELP_TABLE;

    static int RunAsSuspendCommand(const std::vector<std::string>& args);
    static int RunAsWakeupCommand(const std::vector<std::string>& args);
    static int RunAsSetPowerModeCommand(const std::vector<std::string>& args);
    static int RunAsOverrideScreenOffTimeCommand(const std::vector<std::string>& args);
    static int RunAsRestoreScreenOffTimeCommand(const std::vector<std::string>& args);

    static void ShowGlobalHelp();
    static void ShowSuspendHelp();
    static void ShowWakeupHelp();
    static void ShowSetPowerModeHelp();
    static void ShowOverrideScreenOffTimeHelp();
    static void ShowRestoreScreenOffTimeHelp();

    static void OutputSuccess(cJSON* data);
    static void OutputError(int32_t code, const std::string& errCode, const std::string& message,
        const std::string& suggestion);
    static void OutputFallbackError(const std::string& message);
    static std::string PowerErrorsToErrCode(PowerErrors err);
    static std::string PowerErrorsToString(PowerErrors err);
    static std::string GetSuggestion(PowerErrors err);

    static bool HasHelpFlag(const std::vector<std::string>& args);

    void ParseArgs();
};

} // namespace PowerMgr
} // namespace OHOS

#endif // POWER_CLI_COMMAND_H
