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

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <cstdlib>

#include <cJSON.h>

#include "power_cli_command.h"
#include "power_cli_enum_parser.h"
#include "power_mgr_client.h"
#include "power_state_machine_info.h"
#include "hilog/log.h"

#undef LOG_DOMAIN
#undef LOG_TAG
#define LOG_DOMAIN 0xD00D100
#define LOG_TAG "POWER_CLI"

namespace OHOS {
namespace PowerMgr {

static constexpr const char* API_VERSION = "23";
static constexpr const char* WAKEUP_DETAIL_DEFAULT = "cli-call";
static constexpr int MAX_DETAIL_LENGTH = 128;
static constexpr int MIN_COMMAND_ARGC = 2;

PowerCliCommand::PowerCliCommand(int argc, char* argv[]) : argc_(argc), argv_(argv)
{
    ParseArgs();
}

void PowerCliCommand::ParseArgs()
{
    if (argc_ >= MIN_COMMAND_ARGC) {
        command_ = argv_[1];
    }
    for (int i = MIN_COMMAND_ARGC; i < argc_; ++i) {
        args_.emplace_back(argv_[i]);
    }
}

bool PowerCliCommand::HasHelpFlag(const std::vector<std::string>& args)
{
    for (const auto& arg : args) {
        if (arg == "--help") {
            return true;
        }
    }
    return false;
}

int PowerCliCommand::Execute()
{
    if (command_.empty()) {
        OutputError(0,
            "ERR_NO_COMMAND",
            "No command specified",
            "Run 'ohos-powerManager --help' for available commands");
        return 0;
    }

    if (command_ == "--help") {
        if (!args_.empty()) {
            OutputError(static_cast<int32_t>(PowerErrors::ERR_PARAM_INVALID),
                "ERR_PARAM_INVALID",
                "Unexpected argument after --help: " + args_[0],
                "Run 'ohos-powerManager --help' for available commands");
            return static_cast<int32_t>(PowerErrors::ERR_PARAM_INVALID);
        }
        ShowGlobalHelp();
        return 0;
    }

    if (args_.size() == 1 && args_[0] == "--help") {
        auto it = COMMAND_TABLE.find(command_);
        if (it != COMMAND_TABLE.end()) {
            COMMAND_HELP_TABLE.at(command_)();
        } else {
            OutputError(static_cast<int32_t>(PowerErrors::ERR_PARAM_INVALID),
                "ERR_PARAM_INVALID",
                "Unknown command: " + command_,
                "Run 'ohos-powerManager --help' for available commands");
            return static_cast<int32_t>(PowerErrors::ERR_PARAM_INVALID);
        }
        return 0;
    }

    if (HasHelpFlag(args_)) {
        OutputError(static_cast<int32_t>(PowerErrors::ERR_PARAM_INVALID),
            "ERR_PARAM_INVALID",
            "--help must be the only argument",
            "Run 'ohos-powerManager " + command_ + " --help' for usage");
        return static_cast<int32_t>(PowerErrors::ERR_PARAM_INVALID);
    }

    auto it = COMMAND_TABLE.find(command_);
    if (it != COMMAND_TABLE.end()) {
        return it->second(args_);
    }

    OutputError(static_cast<int32_t>(PowerErrors::ERR_PARAM_INVALID),
        "ERR_PARAM_INVALID",
        "Unknown command: " + command_,
        "Run 'ohos-powerManager --help' for available commands");
    return static_cast<int32_t>(PowerErrors::ERR_PARAM_INVALID);
}

const std::unordered_map<std::string,
    std::function<int(const std::vector<std::string>&)>> PowerCliCommand::COMMAND_TABLE = {
    {"suspend",                  RunAsSuspendCommand},
    {"wakeup",                   RunAsWakeupCommand},
    {"set-power-mode",           RunAsSetPowerModeCommand},
    {"override-screen-off-time", RunAsOverrideScreenOffTimeCommand},
    {"restore-screen-off-time",  RunAsRestoreScreenOffTimeCommand},
};

const std::unordered_map<std::string, std::function<void()>> PowerCliCommand::COMMAND_HELP_TABLE = {
    {"suspend",                  ShowSuspendHelp},
    {"wakeup",                   ShowWakeupHelp},
    {"set-power-mode",           ShowSetPowerModeHelp},
    {"override-screen-off-time", ShowOverrideScreenOffTimeHelp},
    {"restore-screen-off-time",  ShowRestoreScreenOffTimeHelp},
};

void PowerCliCommand::OutputFallbackError(const std::string& message)
{
    std::cout << "{\"type\":\"result\",\"status\":\"failed\",\"errCode\":\"ERR_FAILURE\","
              << "\"errMsg\":\"" << message << "\","
              << "\"suggestion\":\"Internal error, try again later\"}" << std::endl;
}

void PowerCliCommand::OutputSuccess(cJSON* data)
{
    cJSON* output = cJSON_CreateObject();
    if (output == nullptr) {
        cJSON_Delete(data);
        OutputFallbackError("Failed to create JSON output object");
        return;
    }
    if (cJSON_AddStringToObject(output, "type", "result") == nullptr ||
        cJSON_AddStringToObject(output, "status", "success") == nullptr ||
        !cJSON_AddItemToObject(output, "data", data)) {
        cJSON_Delete(data);
        cJSON_Delete(output);
        OutputFallbackError("Failed to build JSON output");
        return;
    }
    char* jsonStr = cJSON_PrintUnformatted(output);
    if (jsonStr != nullptr) {
        std::cout << jsonStr << std::endl;
        cJSON_free(jsonStr);
    }
    cJSON_Delete(output);
}

void PowerCliCommand::OutputError(int32_t code, const std::string& errCode, const std::string& message,
    const std::string& suggestion)
{
    cJSON* output = cJSON_CreateObject();
    if (output == nullptr) {
        OutputFallbackError("Failed to create JSON output object");
        return;
    }
    if (cJSON_AddStringToObject(output, "type", "result") == nullptr ||
        cJSON_AddStringToObject(output, "status", "failed") == nullptr ||
        cJSON_AddStringToObject(output, "errCode", errCode.c_str()) == nullptr ||
        cJSON_AddStringToObject(output, "errMsg", message.c_str()) == nullptr ||
        cJSON_AddStringToObject(output, "suggestion", suggestion.c_str()) == nullptr) {
        cJSON_Delete(output);
        OutputFallbackError("Failed to build JSON output");
        return;
    }
    char* jsonStr = cJSON_PrintUnformatted(output);
    if (jsonStr != nullptr) {
        std::cout << jsonStr << std::endl;
        cJSON_free(jsonStr);
    }
    cJSON_Delete(output);
}

std::string PowerCliCommand::PowerErrorsToErrCode(PowerErrors err)
{
    switch (err) {
        case PowerErrors::ERR_OK:
            return "ERR_OK";
        case PowerErrors::ERR_FAILURE:
            return "ERR_FAILURE";
        case PowerErrors::ERR_PERMISSION_DENIED:
            return "ERR_PERMISSION_DENIED";
        case PowerErrors::ERR_SYSTEM_API_DENIED:
            return "ERR_SYSTEM_API_DENIED";
        case PowerErrors::ERR_PARAM_INVALID:
            return "ERR_PARAM_INVALID";
        case PowerErrors::ERR_CONNECTION_FAIL:
            return "ERR_CONNECTION_FAIL";
        case PowerErrors::ERR_FREQUENT_FUNCTION_CALL:
            return "ERR_FREQUENT_FUNCTION_CALL";
        case PowerErrors::ERR_POWER_MODE_TRANSIT_FAILED:
            return "ERR_POWER_MODE_TRANSIT_FAILED";
        case PowerErrors::ERR_SKIP_FUNCTION_CALL:
            return "ERR_SKIP_FUNCTION_CALL";
        case PowerErrors::ERR_READ_OPERATION_FAILED:
            return "ERR_READ_OPERATION_FAILED";
        case PowerErrors::ERR_USER_PARAM_INVALID:
            return "ERR_USER_PARAM_INVALID";
        case PowerErrors::ERR_WRITE_OPERATION_FAILED:
            return "ERR_WRITE_OPERATION_FAILED";
        default:
            return "ERR_UNKNOWN";
    }
}

std::string PowerCliCommand::PowerErrorsToString(PowerErrors err)
{
    switch (err) {
        case PowerErrors::ERR_OK:
            return "Operation succeeded";
        case PowerErrors::ERR_FAILURE:
            return "General failure";
        case PowerErrors::ERR_PERMISSION_DENIED:
            return "Permission denied";
        case PowerErrors::ERR_SYSTEM_API_DENIED:
            return "System API denied";
        case PowerErrors::ERR_PARAM_INVALID:
            return "Invalid parameter";
        case PowerErrors::ERR_CONNECTION_FAIL:
            return "Connection to power service failed";
        case PowerErrors::ERR_FREQUENT_FUNCTION_CALL:
            return "Too frequent function calls";
        case PowerErrors::ERR_POWER_MODE_TRANSIT_FAILED:
            return "Power mode transition failed";
        case PowerErrors::ERR_SKIP_FUNCTION_CALL:
            return "Function call skipped";
        case PowerErrors::ERR_READ_OPERATION_FAILED:
            return "Read operation failed";
        case PowerErrors::ERR_USER_PARAM_INVALID:
            return "User parameter value invalid";
        case PowerErrors::ERR_WRITE_OPERATION_FAILED:
            return "Write operation failed";
        default:
            return "Unknown error";
    }
}

std::string PowerCliCommand::GetSuggestion(PowerErrors err)
{
    switch (err) {
        case PowerErrors::ERR_PERMISSION_DENIED:
            return "Ensure the caller has the required permission "
                   "(ohos.permission.POWER_MANAGER or ohos.permission.POWER_OPTIMIZATION)";
        case PowerErrors::ERR_SYSTEM_API_DENIED:
            return "This API is restricted to system applications";
        case PowerErrors::ERR_CONNECTION_FAIL:
            return "Check if the power service (powermgr) is running: 'hidumper -s 3301'";
        case PowerErrors::ERR_FREQUENT_FUNCTION_CALL:
            return "Reduce the call frequency and try again later";
        case PowerErrors::ERR_POWER_MODE_TRANSIT_FAILED:
            return "Check if the target mode is supported and the device state allows transition";
        case PowerErrors::ERR_PARAM_INVALID:
            return "Check the input parameters and refer to 'ohos-powerManager --help'";
        case PowerErrors::ERR_USER_PARAM_INVALID:
            return "Ensure parameter values are within the expected range";
        default:
            return "Run 'ohos-powerManager --help' for usage information";
    }
}

// ---- Help functions (output to stdout) ----

void PowerCliCommand::ShowGlobalHelp()
{
    const char* helpText =
        "ohos-powerManager - Power management CLI tool for HarmonyOS/OHOS\n"
        "\n"
        "Usage:\n"
        "  ohos-powerManager <command> [options]\n"
        "\n"
        "Commands:\n"
        "  suspend                      Suspend device (turn screen off)\n"
        "  wakeup                       Wake up device (turn screen on)\n"
        "  set-power-mode               Set device power mode\n"
        "  override-screen-off-time     Override screen off timeout\n"
        "  restore-screen-off-time      Restore screen off timeout to default\n"
        "\n"
        "Options:\n"
        "  --help                       Show this help message\n"
        "\n"
        "Use 'ohos-powerManager <command> --help' for more information on a command.\n";

    std::cout << helpText;
}

void PowerCliCommand::ShowSuspendHelp()
{
    const char* helpText =
        "suspend - Suspend device and turn screen off\n"
        "\n"
        "Usage:\n"
        "  ohos-powerManager suspend [--immediately]\n"
        "\n"
        "Parameters:\n"
        "  --immediately                Suspend immediately without delay (optional, default: false)\n"
        "  --help                       Display this help message\n"
        "\n"
        "Examples:\n"
        "  ohos-powerManager suspend\n"
        "  ohos-powerManager suspend --immediately\n"
        "\n"
        "Permission: ohos.permission.POWER_MANAGER\n";

    std::cout << helpText;
}

void PowerCliCommand::ShowWakeupHelp()
{
    const char* helpText =
        "wakeup - Wake up device and turn screen on\n"
        "\n"
        "Usage:\n"
        "  ohos-powerManager wakeup [--detail <string>]\n"
        "\n"
        "Parameters:\n"
        "  --detail <string>            Wakeup detail string (optional, max 128 chars, default: \"cli-call\")\n"
        "  --help                       Display this help message\n"
        "\n"
        "Examples:\n"
        "  ohos-powerManager wakeup\n"
        "  ohos-powerManager wakeup --detail \"user pressed power key\"\n"
        "\n"
        "Permission: ohos.permission.POWER_MANAGER\n";

    std::cout << helpText;
}

void PowerCliCommand::ShowSetPowerModeHelp()
{
    const char* helpText =
        "set-power-mode - Set device power mode\n"
        "\n"
        "Usage:\n"
        "  ohos-powerManager set-power-mode --mode <mode>\n"
        "\n"
        "Parameters:\n"
        "  --mode <mode>                Power mode to set (required, values: [normal, powerSave])\n"
        "  --help                       Display this help message\n"
        "\n"
        "Examples:\n"
        "  ohos-powerManager set-power-mode --mode normal\n"
        "  ohos-powerManager set-power-mode --mode powerSave\n"
        "\n"
        "Permission: ohos.permission.POWER_OPTIMIZATION\n";

    std::cout << helpText;
}

void PowerCliCommand::ShowOverrideScreenOffTimeHelp()
{
    const char* helpText =
        "override-screen-off-time - Override screen off timeout\n"
        "\n"
        "Usage:\n"
        "  ohos-powerManager override-screen-off-time --time <timeout_ms>\n"
        "\n"
        "Parameters:\n"
        "  --time <timeout_ms>          Screen off timeout in milliseconds (required, must be positive integer)\n"
        "  --help                       Display this help message\n"
        "\n"
        "Examples:\n"
        "  ohos-powerManager override-screen-off-time --time 30000\n"
        "  ohos-powerManager override-screen-off-time --time 60000\n"
        "\n"
        "Permission: ohos.permission.POWER_MANAGER\n";

    std::cout << helpText;
}

void PowerCliCommand::ShowRestoreScreenOffTimeHelp()
{
    const char* helpText =
        "restore-screen-off-time - Restore screen off timeout to system default\n"
        "\n"
        "Usage:\n"
        "  ohos-powerManager restore-screen-off-time\n"
        "\n"
        "Parameters:\n"
        "  --help                       Display this help message\n"
        "\n"
        "Examples:\n"
        "  ohos-powerManager restore-screen-off-time\n"
        "\n"
        "Permission: ohos.permission.POWER_MANAGER\n";

    std::cout << helpText;
}

// ---- Command handlers ----

int PowerCliCommand::RunAsSuspendCommand(const std::vector<std::string>& args)
{
    bool immediately = false;
    bool hasImmediately = false;

    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "--immediately") {
            if (hasImmediately) {
                OutputError(static_cast<int32_t>(PowerErrors::ERR_PARAM_INVALID),
                    "ERR_PARAM_INVALID",
                    "Duplicate parameter: --immediately",
                    "Each parameter may only be specified once. Run 'ohos-powerManager suspend --help' for usage");
                return static_cast<int32_t>(PowerErrors::ERR_PARAM_INVALID);
            }
            immediately = true;
            hasImmediately = true;
        } else {
            OutputError(static_cast<int32_t>(PowerErrors::ERR_PARAM_INVALID),
                "ERR_PARAM_INVALID",
                "Unknown parameter: " + args[i],
                "Run 'ohos-powerManager suspend --help' for usage");
            return static_cast<int32_t>(PowerErrors::ERR_PARAM_INVALID);
        }
    }

    PowerMgrClient& client = PowerMgrClient::GetInstance();
    PowerErrors ret = client.SuspendDevice(
        SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, immediately, API_VERSION);

    if (ret == PowerErrors::ERR_OK) {
        cJSON* data = cJSON_CreateObject();
        if (data == nullptr) {
            OutputFallbackError("Failed to create JSON data object");
            return 0;
        }
        if (cJSON_AddStringToObject(data, "command", "suspend") == nullptr ||
            cJSON_AddStringToObject(data, "reason", "application") == nullptr ||
            cJSON_AddBoolToObject(data, "immediately", immediately) == nullptr) {
            cJSON_Delete(data);
            OutputFallbackError("Failed to build JSON data");
            return 0;
        }
        OutputSuccess(data);
        return 0;
    }

    int32_t errCode = static_cast<int32_t>(ret);
    OutputError(errCode, PowerErrorsToErrCode(ret), PowerErrorsToString(ret), GetSuggestion(ret));
    return errCode;
}

int PowerCliCommand::RunAsWakeupCommand(const std::vector<std::string>& args)
{
    std::string detail = WAKEUP_DETAIL_DEFAULT;
    bool hasDetail = false;

    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "--detail") {
            if (hasDetail) {
                OutputError(static_cast<int32_t>(PowerErrors::ERR_PARAM_INVALID),
                    "ERR_PARAM_INVALID",
                    "Duplicate parameter: --detail",
                    "Each parameter may only be specified once. Run 'ohos-powerManager wakeup --help' for usage");
                return static_cast<int32_t>(PowerErrors::ERR_PARAM_INVALID);
            }
            if (i + 1 < args.size()) {
                detail = args[i + 1];
                i++;
            }
            hasDetail = true;
        } else {
            OutputError(static_cast<int32_t>(PowerErrors::ERR_PARAM_INVALID),
                "ERR_PARAM_INVALID",
                "Unknown parameter: " + args[i],
                "Run 'ohos-powerManager wakeup --help' for usage");
            return static_cast<int32_t>(PowerErrors::ERR_PARAM_INVALID);
        }
    }

    if (static_cast<int>(detail.size()) > MAX_DETAIL_LENGTH) {
        OutputError(static_cast<int32_t>(PowerErrors::ERR_PARAM_INVALID),
            "ERR_PARAM_INVALID",
            "Detail string exceeds maximum length of " + std::to_string(MAX_DETAIL_LENGTH) + " characters",
            "Use a shorter detail string");
        return static_cast<int32_t>(PowerErrors::ERR_PARAM_INVALID);
    }

    PowerMgrClient& client = PowerMgrClient::GetInstance();
    PowerErrors ret = client.WakeupDevice(
        WakeupDeviceType::WAKEUP_DEVICE_APPLICATION, detail, API_VERSION);

    if (ret == PowerErrors::ERR_OK) {
        cJSON* data = cJSON_CreateObject();
        if (data == nullptr) {
            OutputFallbackError("Failed to create JSON data object");
            return 0;
        }
        if (cJSON_AddStringToObject(data, "command", "wakeup") == nullptr ||
            cJSON_AddStringToObject(data, "reason", "application") == nullptr ||
            cJSON_AddStringToObject(data, "detail", detail.c_str()) == nullptr) {
            cJSON_Delete(data);
            OutputFallbackError("Failed to build JSON data");
            return 0;
        }
        OutputSuccess(data);
        return 0;
    }

    int32_t errCode = static_cast<int32_t>(ret);
    OutputError(errCode, PowerErrorsToErrCode(ret), PowerErrorsToString(ret), GetSuggestion(ret));
    return errCode;
}

int PowerCliCommand::RunAsSetPowerModeCommand(const std::vector<std::string>& args)
{
    std::string modeStr;
    bool hasMode = false;

    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "--mode") {
            if (hasMode) {
                OutputError(static_cast<int32_t>(PowerErrors::ERR_PARAM_INVALID),
                    "ERR_PARAM_INVALID",
                    "Duplicate parameter: --mode",
                    "Each parameter may only be specified once. "
                    "Run 'ohos-powerManager set-power-mode --help' for usage");
                return static_cast<int32_t>(PowerErrors::ERR_PARAM_INVALID);
            }
            if (i + 1 < args.size()) {
                modeStr = args[i + 1];
                i++;
            }
            hasMode = true;
        } else {
            OutputError(static_cast<int32_t>(PowerErrors::ERR_PARAM_INVALID),
                "ERR_PARAM_INVALID",
                "Unknown parameter: " + args[i],
                "Run 'ohos-powerManager set-power-mode --help' for usage");
            return static_cast<int32_t>(PowerErrors::ERR_PARAM_INVALID);
        }
    }

    if (modeStr.empty()) {
        OutputError(static_cast<int32_t>(PowerErrors::ERR_PARAM_INVALID),
            "ERR_PARAM_INVALID",
            "Missing required parameter: --mode",
            "Usage: ohos-powerManager set-power-mode --mode <mode>. Valid modes: "
                + PowerCliEnumParser::GetValidPowerModes());
        return static_cast<int32_t>(PowerErrors::ERR_PARAM_INVALID);
    }

    EnumParseResult parseResult = PowerCliEnumParser::ParsePowerMode(modeStr);
    if (!parseResult.success) {
        OutputError(static_cast<int32_t>(PowerErrors::ERR_PARAM_INVALID),
            "ERR_PARAM_INVALID",
            parseResult.error,
            "Valid modes: " + PowerCliEnumParser::GetValidPowerModes());
        return static_cast<int32_t>(PowerErrors::ERR_PARAM_INVALID);
    }

    PowerMode mode = static_cast<PowerMode>(parseResult.value);
    PowerMgrClient& client = PowerMgrClient::GetInstance();
    PowerErrors ret = client.SetDeviceMode(mode);

    if (ret == PowerErrors::ERR_OK) {
        cJSON* data = cJSON_CreateObject();
        if (data == nullptr) {
            OutputFallbackError("Failed to create JSON data object");
            return 0;
        }
        if (cJSON_AddStringToObject(data, "command", "set-power-mode") == nullptr ||
            cJSON_AddStringToObject(data, "mode", modeStr.c_str()) == nullptr) {
            cJSON_Delete(data);
            OutputFallbackError("Failed to build JSON data");
            return 0;
        }
        OutputSuccess(data);
        return 0;
    }

    int32_t errCode = static_cast<int32_t>(ret);
    OutputError(errCode, PowerErrorsToErrCode(ret), PowerErrorsToString(ret), GetSuggestion(ret));
    return errCode;
}

int PowerCliCommand::RunAsOverrideScreenOffTimeCommand(const std::vector<std::string>& args)
{
    std::string timeoutStr;
    bool hasTime = false;

    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "--time") {
            if (hasTime) {
                OutputError(static_cast<int32_t>(PowerErrors::ERR_PARAM_INVALID),
                    "ERR_PARAM_INVALID",
                    "Duplicate parameter: --time",
                    "Each parameter may only be specified once. "
                    "Run 'ohos-powerManager override-screen-off-time --help' for usage");
                return static_cast<int32_t>(PowerErrors::ERR_PARAM_INVALID);
            }
            if (i + 1 < args.size()) {
                timeoutStr = args[i + 1];
                i++;
            }
            hasTime = true;
        } else {
            OutputError(static_cast<int32_t>(PowerErrors::ERR_PARAM_INVALID),
                "ERR_PARAM_INVALID",
                "Unknown parameter: " + args[i],
                "Run 'ohos-powerManager override-screen-off-time --help' for usage");
            return static_cast<int32_t>(PowerErrors::ERR_PARAM_INVALID);
        }
    }

    if (timeoutStr.empty()) {
        OutputError(static_cast<int32_t>(PowerErrors::ERR_PARAM_INVALID),
            "ERR_PARAM_INVALID",
            "Missing required parameter: --time",
            "Usage: ohos-powerManager override-screen-off-time --time <timeout_ms>");
        return static_cast<int32_t>(PowerErrors::ERR_PARAM_INVALID);
    }

    char* endPtr = nullptr;
    int64_t timeout = std::strtoll(timeoutStr.c_str(), &endPtr, 10);
    if (endPtr == timeoutStr.c_str() || *endPtr != '\0' || timeout <= 0) {
        OutputError(static_cast<int32_t>(PowerErrors::ERR_PARAM_INVALID),
            "ERR_PARAM_INVALID",
            "Invalid timeout value: '" + timeoutStr + "'. Must be a positive integer (milliseconds)",
            "Example: ohos-powerManager override-screen-off-time --time 30000");
        return static_cast<int32_t>(PowerErrors::ERR_PARAM_INVALID);
    }

    PowerMgrClient& client = PowerMgrClient::GetInstance();
    PowerErrors ret = client.OverrideScreenOffTime(timeout, API_VERSION);

    if (ret == PowerErrors::ERR_OK) {
        cJSON* data = cJSON_CreateObject();
        if (data == nullptr) {
            OutputFallbackError("Failed to create JSON data object");
            return 0;
        }
        if (cJSON_AddStringToObject(data, "command", "override-screen-off-time") == nullptr ||
            cJSON_AddNumberToObject(data, "timeout", static_cast<double>(timeout)) == nullptr) {
            cJSON_Delete(data);
            OutputFallbackError("Failed to build JSON data");
            return 0;
        }
        OutputSuccess(data);
        return 0;
    }

    int32_t errCode = static_cast<int32_t>(ret);
    OutputError(errCode, PowerErrorsToErrCode(ret), PowerErrorsToString(ret), GetSuggestion(ret));
    return errCode;
}

int PowerCliCommand::RunAsRestoreScreenOffTimeCommand(const std::vector<std::string>& args)
{
    if (!args.empty()) {
        OutputError(static_cast<int32_t>(PowerErrors::ERR_PARAM_INVALID),
            "ERR_PARAM_INVALID",
            "Unknown parameter: " + args[0],
            "restore-screen-off-time takes no parameters. "
            "Run 'ohos-powerManager restore-screen-off-time --help' for usage");
        return static_cast<int32_t>(PowerErrors::ERR_PARAM_INVALID);
    }

    PowerMgrClient& client = PowerMgrClient::GetInstance();
    PowerErrors ret = client.RestoreScreenOffTime(API_VERSION);

    if (ret == PowerErrors::ERR_OK) {
        cJSON* data = cJSON_CreateObject();
        if (data == nullptr) {
            OutputFallbackError("Failed to create JSON data object");
            return 0;
        }
        if (cJSON_AddStringToObject(data, "command", "restore-screen-off-time") == nullptr) {
            cJSON_Delete(data);
            OutputFallbackError("Failed to build JSON data");
            return 0;
        }
        OutputSuccess(data);
        return 0;
    }

    int32_t errCode = static_cast<int32_t>(ret);
    OutputError(errCode, PowerErrorsToErrCode(ret), PowerErrorsToString(ret), GetSuggestion(ret));
    return errCode;
}

} // namespace PowerMgr
} // namespace OHOS
