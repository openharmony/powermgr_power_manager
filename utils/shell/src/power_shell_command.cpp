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

#include "power_shell_command.h"

#include <cerrno>
#include <fcntl.h>
#include <getopt.h>
#include <iostream>
#include <string_ex.h>
#include <unistd.h>

#include "power_mgr_client.h"
#include "display_power_mgr_client.h"
#include "iservice_registry.h"
#include "singleton.h"
#include "system_ability_definition.h"

extern char *optarg;

namespace OHOS {
namespace PowerMgr {
using namespace OHOS::DisplayPowerMgr;

static const struct option SET_MODE_OPTIONS[] = {
    {"help", no_argument, nullptr, 'h'},
};

static const struct option DISPLAY_OPTIONS[] = {
    {"help", no_argument, nullptr, 'h'},
    {"restore", no_argument, nullptr, 'r'},
    {"update", required_argument, nullptr, 'u'},
    {"override", required_argument, nullptr, 'o'},
};

static const std::string HELP_MSG =
    "usage: power-shell\n"
    "command list:\n"
    "  setmode :    Set power mode. \n"
    "  wakeup  :    Wakeup system and turn screen on. \n"
    "  suspend :    Suspend system and turn screen off. \n"
    "  display :    Update or Override display brightness. \n"
    "  dump    :    Dump power info. \n"
    "  help    :    Show this help menu. \n";

static const std::string SETMODE_HELP_MSG =
    "usage: power-shell setmode [<options>]\n"
    "setmode <power mode: (value is as below)> \n"
    "  600  :  normal mode\n"
    "  601  :  power save mode\n"
    "  602  :  extreme mode\n";

static const std::string DISPLAY_HELP_MSG =
    "usage: power-shell display [<options>] 100\n"
    "display <options is as below> \n"
    "  -u  :  update brightness\n"
    "  -o  :  override brightness\n";

PowerShellCommand::PowerShellCommand(int argc, char *argv[]) : ShellCommand(argc, argv, "power-shell")
{}

ErrCode PowerShellCommand::CreateCommandMap()
{
    commandMap_ = {
        {"help", std::bind(&PowerShellCommand::RunAsHelpCommand, this)},
        {"setmode", std::bind(&PowerShellCommand::RunAsSetModeCommand, this)},
        {"wakeup", std::bind(&PowerShellCommand::RunAsWakeupCommand, this)},
        {"suspend", std::bind(&PowerShellCommand::RunAsSuspendCommand, this)},
        {"display", std::bind(&PowerShellCommand::RunAsDisplayCommand, this)},
        {"dump", std::bind(&PowerShellCommand::RunAsDumpCommand, this)},
    };

    return ERR_OK;
}

ErrCode PowerShellCommand::CreateMessageMap()
{
    messageMap_ = {};

    return ERR_OK;
}

ErrCode PowerShellCommand::init()
{
    return OHOS::ERR_OK;
}

ErrCode PowerShellCommand::RunAsHelpCommand()
{
    resultReceiver_.clear();
    resultReceiver_.append(HELP_MSG);
    return ERR_OK;
}

ErrCode PowerShellCommand::RunAsSetModeCommand()
{
    int ind = 0;
    int option = getopt_long(argc_, argv_, "h", SET_MODE_OPTIONS, &ind);
    resultReceiver_.clear();
    if (option == 'h') {
        resultReceiver_.append(SETMODE_HELP_MSG);
        return ERR_OK;
    }
    if (argList_.empty()) {
        resultReceiver_.append("Error! please input your mode value. \n");
        resultReceiver_.append(SETMODE_HELP_MSG);
        return ERR_OK;
    }

    uint32_t mode = static_cast<uint32_t>(atoi(argList_[0].c_str()));
    resultReceiver_.append("Set Mode: ");
    resultReceiver_.append(argList_[0]);
    resultReceiver_.append("\n");
    PowerMgrClient &client = PowerMgrClient::GetInstance();
    client.SetDeviceMode(mode);
    uint32_t result = client.GetDeviceMode();
    if (result == mode) {
        resultReceiver_.append("Set Mode Success!");
    } else {
        resultReceiver_.append("Set Mode Failed, current mode is: ");
        resultReceiver_.append(std::to_string(result));
    }

    return ERR_OK;
}

ErrCode PowerShellCommand::RunAsWakeupCommand()
{
    PowerMgrClient &client = PowerMgrClient::GetInstance();
    std::string detail = "shell";
    client.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON, detail);
    resultReceiver_.append("WakeupDevice is called");
    return ERR_OK;
}

ErrCode PowerShellCommand::RunAsSuspendCommand()
{
    PowerMgrClient &client = PowerMgrClient::GetInstance();
    std::string detail = "shell";
    client.SuspendDevice(SuspendDeviceType::SUSPEND_DEVICE_REASON_POWER_BUTTON);
    resultReceiver_.append("SuspendDevice is called");
    return ERR_OK;
}

extern "C" void PrintDumpFileError(std::string& receiver, const char* path)
{
    receiver.append("Open Dump file (");
    receiver.append(path);
    receiver.append(") failed: ");
    receiver.append(std::to_string(errno));
    receiver.append("\n");
}

ErrCode PowerShellCommand::RunAsDumpCommand()
{
    resultReceiver_.clear();

    PowerMgrClient &client = PowerMgrClient::GetInstance();
    std::string ret = client.Dump(argList_);
    resultReceiver_.append("Power Dump result: \n");
    resultReceiver_.append(ret);

    return ERR_OK;
}

ErrCode PowerShellCommand::RunAsDisplayCommand()
{
    int ind = 0;
    int option = getopt_long(argc_, argv_, "hru:o:", DISPLAY_OPTIONS, &ind);
    resultReceiver_.clear();
    if (option == 'h') {
        resultReceiver_.append(DISPLAY_HELP_MSG);
        return ERR_OK;
    }
    if (option == 'r') {
        bool ret = DisplayPowerMgrClient::GetInstance().RestoreBrightness();
        resultReceiver_.append("Restore brightness");
        if (!ret) {
            resultReceiver_.append(" failed");
        }
        resultReceiver_.append("\n");
        return ERR_OK;
    }
    if (!optarg) {
        resultReceiver_.append("Error! please input your brightness value.\n");
        resultReceiver_.append(DISPLAY_HELP_MSG);
        return ERR_OK;
    }
    auto value = static_cast<uint32_t>(atoi(optarg));
    if (option == 'u') {
        bool ret = DisplayPowerMgrClient::GetInstance().SetBrightness(value);
        resultReceiver_.append("Update brightness to ");
        resultReceiver_.append(std::to_string(value));
        if (!ret) {
            resultReceiver_.append(" failed");
        }
        resultReceiver_.append("\n");
        return ERR_OK;
    }
    if (option == 'o') {
        bool ret = DisplayPowerMgrClient::GetInstance().OverrideBrightness(value);
        resultReceiver_.append("Override brightness to ");
        resultReceiver_.append(std::to_string(value));
        if (!ret) {
            resultReceiver_.append(" failed");
        }
        resultReceiver_.append("\n");
        return ERR_OK;
    }
    return ERR_OK;
}
}
}  // namespace OHOS
