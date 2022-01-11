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
#include "iservice_registry.h"
#include "singleton.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace PowerMgr {
static const struct option SET_MODE_OPTIONS[] = {
    {"help", no_argument, nullptr, 'h'},
};

static const std::string HELP_MSG =
    "usage: power-shell\n"
    "command list:\n"
    "  setmode :    Set power mode. \n"
    "  dump    :    Dump power info. \n"
    "  help    :    Show this help menu. \n";

static const std::string SETMODE_HELP_MSG =
    "usage: power-shell setmode [<options>]\n"
    "setmode <power mode: (value is as below)> \n"
    "  600  :  normal mode\n"
    "  601  :  power save mode\n"
    "  602  :  extreme mode\n";

PowerShellCommand::PowerShellCommand(int argc, char *argv[]) : ShellCommand(argc, argv, "power-shell")
{}

ErrCode PowerShellCommand::CreateCommandMap()
{
    commandMap_ = {
        {"help", std::bind(&PowerShellCommand::RunAsHelpCommand, this)},
        {"setmode", std::bind(&PowerShellCommand::RunAsSetModeCommand, this)},
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

    int mode = atoi(argList_[0].c_str());
    resultReceiver_.append("Set Mode: ");
    resultReceiver_.append(argList_[0]);
    resultReceiver_.append("\n");
    PowerMgrClient &client = PowerMgrClient::GetInstance();
    client.SetDeviceMode(mode);
    int result = client.GetDeviceMode();
    if (result == mode) {
        resultReceiver_.append("Set Mode Success!");
    } else {
        resultReceiver_.append("Set Mode Failed, current mode is: ");
        resultReceiver_.append(std::to_string(result));
    }

    return ERR_OK;
}

extern "C" void PrintDumpFileError(std::string& receiver, const char* path)
{
    receiver.append("Open Dump file (");
    receiver.append(path);
    receiver.append(") failed: ");
    receiver.append(std::to_string(errno));
}

ErrCode PowerShellCommand::RunAsDumpCommand()
{
    resultReceiver_.clear();
    int fd = open(TEMP_DUMP_LOG_PATH,
        O_RDWR | O_APPEND | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
    if (fd < 0) {
        PrintDumpFileError(resultReceiver_, TEMP_DUMP_LOG_PATH);
        return ERR_OK;
    }
    std::vector<std::u16string> args;
    for (int i = 2; i < argc_; i++) {
        std::string str(argv_[i]);
        args.push_back(Str8ToStr16(str));
    }
    PowerMgrClient &client = PowerMgrClient::GetInstance();
    int ret = client.Dump(fd, args);
    resultReceiver_.append("Power Dump result: ");
    resultReceiver_.append(std::to_string(ret));
    resultReceiver_.append("\n");
    char buff[DUMP_BUFF_SIZE];
    while (read(fd, buff, sizeof(buff) - 1) > 0) {
        resultReceiver_.append(buff);
    }
    close(fd);
    remove(TEMP_DUMP_LOG_PATH);
    return ERR_OK;
}
}
}  // namespace OHOS
