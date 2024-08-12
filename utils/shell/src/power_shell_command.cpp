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

#include "power_shell_command.h"

#include <cerrno>
#include <string_ex.h>
#include <sstream>
#include "parameters.h"
#include "power_mgr_client.h"
#ifdef HAS_DISPLAY_MANAGER_PART
#include "display_power_mgr_client.h"
#endif

namespace OHOS {
namespace PowerMgr {

bool PowerShellCommand::IsDeveloperMode()
{
    return OHOS::system::GetBoolParameter("const.security.developermode.state", true);
}

static const struct option SET_MODE_OPTIONS[] = {
    {"help", no_argument, nullptr, 'h'},
};

#ifndef POWER_SHELL_USER
#ifdef HAS_DISPLAY_MANAGER_PART
static const struct option DISPLAY_OPTIONS[] = {
    {"help", no_argument, nullptr, 'h'},
    {"restore", no_argument, nullptr, 'r'},
    {"set", required_argument, nullptr, 's'},
    {"override", required_argument, nullptr, 'o'},
    {"boost", required_argument, nullptr, 'b'},
    {"cancel", no_argument, nullptr, 'c'},
    {"discount", required_argument, nullptr, 'd'},
};
#endif

static const struct option PROXYLOCK_OPTIONS[] = {
    {"help", no_argument, nullptr, 'h'},
    {"proxy", required_argument, nullptr, 'p'},
    {"uproxy", required_argument, nullptr, 'u'},
};

static const struct option HIBERNATE_OPTIONS[] = {
    {"help", no_argument, nullptr, 'h'},
};
#endif

static const struct option TIME_OUT_OPTIONS[] = {
    {"help", no_argument, nullptr, 'h'},
    {"restore", no_argument, nullptr, 'r'},
    {"override", required_argument, nullptr, 'o'},
};

static const struct option WAKE_UP_TYPES[] = {
    {"pre_bright", no_argument, nullptr, 'a'},
    {"pre_bright_auth_success", no_argument, nullptr, 'b'},
    {"pre_bright_auth_fail_screen_on", no_argument, nullptr, 'c'},
    {"pre_bright_auth_fail_screen_off", no_argument, nullptr, 'd'},
    {"default", no_argument, nullptr, 'e'},
};

static const std::string HELP_MSG =
    "usage: power-shell\n"
    "command list:\n"
    "  setmode :    Set power mode. \n"
    "  wakeup  :    Wakeup system and turn screen on. \n"
    "  suspend :    Suspend system and turn screen off. \n"
#ifndef POWER_SHELL_USER
    "  lock    :    Query running lock lists by bundle app. \n"
    "  proxylock :    Proxy running lock by app uid. \n"
    "  hibernate :    hibernate the device. \n"
#ifdef HAS_DISPLAY_MANAGER_PART
    "  display :    Update or Override display brightness. \n"
#endif
    "  dump    :    Dump power info. \n"
#endif
    "  timeout :    Override or Restore screen off time. \n"
    "  help    :    Show this help menu. \n";

static const std::string SETMODE_HELP_MSG =
    "usage: power-shell setmode [<options>]\n"
    "setmode <power mode: (value is as below)> \n"
    "  600  :  normal mode\n"
    "  601  :  power save mode\n"
    "  602  :  performance mode\n"
    "  603  :  extreme power save mode\n";

#ifndef POWER_SHELL_USER
#ifdef HAS_DISPLAY_MANAGER_PART
static const std::string DISPLAY_HELP_MSG =
    "usage: power-shell display [<options>] 100\n"
    "display <options are as below> \n"
    "  -h  :  display help\n"
    "  -r  :  retore brightness\n"
    "  -s  :  set brightness\n"
    "  -o  :  override brightness\n"
    "  -b  :  timing maximum brightness\n"
    "  -c  :  cancel the timing maximum brightness\n"
    "  -d  :  discount brightness\n";
#endif

static const std::string PROXYLOCK_HELP_MSG =
    "usage: power-shell proxylock [<options>] 20020041\n"
    "proxylock <options are as below> \n"
    "  -p  :  proxy runninglock\n"
    "  -u  :  unproxy runninglock\n";

static const std::string HIBERNATE_HELP_MSG =
    "usage: power-shell hibernate [<options>]\n"
    "  hibernate <options are as below> \n"
    "  the default option is false\n"
    "  true  :  clear memory before hibernate\n"
    "  false :  skip clearing memory before hibernate\n";
#endif

static const std::string TIME_OUT_HELP_MSG =
    "usage: power-shell timeout [<options>] 1000\n"
    "timeout <options are as below> \n"
    "  -o  :  override screen off time\n"
    "  -r  :  restore screen off time\n";

PowerShellCommand::PowerShellCommand(int argc, char *argv[]) : ShellCommand(argc, argv, "power-shell")
{}

ErrCode PowerShellCommand::CreateCommandMap()
{
    commandMap_ = {
        {"help", [this]() -> ErrCode { return this->RunAsHelpCommand(); }},
        {"setmode", [this]() -> ErrCode { return this->RunAsSetModeCommand(); }},
        {"wakeup", [this]() -> ErrCode { return this->RunAsWakeupCommand(); }},
        {"suspend", [this]() -> ErrCode { return this->RunAsSuspendCommand(); }},
#ifndef POWER_SHELL_USER
        {"hibernate", [this]() -> ErrCode { return this->RunAsHibernateCommand(); }},
        {"lock", [this]() -> ErrCode { return this->RunAsQueryLockCommand(); }},
        {"proxylock", [this]() -> ErrCode { return this->RunAsProxyLockCommand(); }},
#ifdef HAS_DISPLAY_MANAGER_PART
        {"display", [this]() -> ErrCode { return this->RunAsDisplayCommand(); }},
#endif
        {"dump", [this]() -> ErrCode { return this->RunAsDumpCommand(); }},
#endif
        {"timeout", [this]() -> ErrCode { return this->RunAsTimeOutCommand(); }},
        {"forcetimeout", [this]() -> ErrCode { return this->RunAsForceTimeOutCommand(); }},
        {"timeoutscreenlock", [this]() -> ErrCode { return this->RunAsTimeOutScreenLockCommand(); }},
    };

#ifndef POWER_SHELL_USER
#ifdef HAS_DISPLAY_MANAGER_PART
    commandDisplay_ = {
        {'h', [this]() -> ErrCode { return this->RunAsDisplayCommandHelp(); }},
        {'r', [this]() -> ErrCode { return this->RunAsDisplayCommandRestore(); }},
        {'s', [this]() -> ErrCode { return this->RunAsDisplayCommandSetValue(); }},
        {'o', [this]() -> ErrCode { return this->RunAsDisplayCommandOverride(); }},
        {'b', [this]() -> ErrCode { return this->RunAsDisplayCommandBoost(); }},
        {'c', [this]() -> ErrCode { return this->RunAsDisplayCommandCancelBoost(); }},
        {'d', [this]() -> ErrCode { return this->RunAsDisplayCommandDiscount(); }},
    };
#endif
#endif

    return ERR_OK;
}

ErrCode PowerShellCommand::RunAsForceTimeOutCommand()
{
    if (!IsDeveloperMode()) {
        return ERR_PERMISSION_DENIED;
    }
    bool enabled = argList_[0][0] - '0';
    PowerMgrClient& client = PowerMgrClient::GetInstance();
    client.SetForceTimingOut(enabled);
    return ERR_OK;
}

ErrCode PowerShellCommand::RunAsTimeOutScreenLockCommand()
{
    if (!IsDeveloperMode()) {
        return ERR_PERMISSION_DENIED;
    }
    resultReceiver_.clear();
    auto parameterCount = argList_.size();
    if (parameterCount < 2) {
        resultReceiver_.append("too few arguments \n");
        return ERR_OK;
    }
    PowerMgrClient& client = PowerMgrClient::GetInstance();
    bool enableLockScreen = argList_[0][0] - '0';
    bool checkScreenOnLock = argList_[1][0] - '0';
    if (parameterCount == 2) {
        client.LockScreenAfterTimingOut(enableLockScreen, checkScreenOnLock);
        return ERR_OK;
    }
    bool sendScreenOffEvent = argList_[2][0] - '0';
    client.LockScreenAfterTimingOut(enableLockScreen, checkScreenOnLock, sendScreenOffEvent);
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
    if (!IsDeveloperMode()) {
        return ERR_PERMISSION_DENIED;
    }
    resultReceiver_.clear();
    resultReceiver_.append(HELP_MSG);
    return ERR_OK;
}

ErrCode PowerShellCommand::RunAsSetModeCommand()
{
    if (!IsDeveloperMode()) {
        return ERR_PERMISSION_DENIED;
    }
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

    auto mode = static_cast<uint32_t>(strtol(argList_[0].c_str(), nullptr, 0));
    resultReceiver_.append("Set Mode: ");
    resultReceiver_.append(argList_[0]);
    resultReceiver_.append("\n");
    PowerMgrClient& client = PowerMgrClient::GetInstance();
    client.SetDeviceMode(static_cast<PowerMode>(mode));
    uint32_t result = static_cast<uint32_t>(client.GetDeviceMode());
    if (result == mode) {
        resultReceiver_.append("Set Mode Success!\n");
    } else {
        resultReceiver_.append("Set Mode Failed, current mode is: ");
        resultReceiver_.append(std::to_string(result));
        resultReceiver_.append("\n");
    }

    return ERR_OK;
}

ErrCode PowerShellCommand::RunAsWakeupCommand()
{
    if (!IsDeveloperMode()) {
        return ERR_PERMISSION_DENIED;
    }
    int ind = 0;
    int option = getopt_long(argc_, argv_, "abcde", WAKE_UP_TYPES, &ind);
    resultReceiver_.clear();
    PowerMgrClient& client = PowerMgrClient::GetInstance();
    std::string detail = "shell";
    if (option == 'a') {
        detail = "pre_bright";
        resultReceiver_.append("pre_bright is called\n");
    }
    if (option == 'b') {
        detail = "pre_bright_auth_success";
        resultReceiver_.append("pre_bright_auth_success is called\n");
    }
    if (option == 'c') {
        detail = "pre_bright_auth_fail_screen_on";
        resultReceiver_.append("pre_bright_auth_fail_screen_on is called\n");
    }
    if (option == 'd') {
        detail = "pre_bright_auth_fail_screen_off";
        resultReceiver_.append("pre_bright_auth_fail_screen_off is called\n");
    }
    if (option == 'e') {
        resultReceiver_.append("default is called\n");
        detail = "shell";
    }
    client.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_APPLICATION, detail);
    resultReceiver_.append("WakeupDevice is called\n");
    return ERR_OK;
}

ErrCode PowerShellCommand::RunAsSuspendCommand()
{
    if (!IsDeveloperMode()) {
        return ERR_PERMISSION_DENIED;
    }
    PowerMgrClient& client = PowerMgrClient::GetInstance();
    client.SuspendDevice(SuspendDeviceType::SUSPEND_DEVICE_REASON_POWER_KEY);
    resultReceiver_.append("SuspendDevice is called\n");
    return ERR_OK;
}

#ifndef POWER_SHELL_USER
ErrCode PowerShellCommand::RunAsHibernateCommand()
{
    if (!IsDeveloperMode()) {
        return ERR_PERMISSION_DENIED;
    }
    int ind = 0;
    int option = getopt_long(argc_, argv_, "h", HIBERNATE_OPTIONS, &ind);
    resultReceiver_.clear();
    if (option == 'h') {
        resultReceiver_.append(HIBERNATE_HELP_MSG);
        return ERR_OK;
    }
    bool clearMemory = false;
    if (!argList_.empty()) {
        if (strcmp(argList_[0].c_str(), "false") == 0) {
            clearMemory = false;
        } else if (strcmp(argList_[0].c_str(), "true") == 0) {
            clearMemory = true;
        } else {
            resultReceiver_.append("Error! please input your option value. \n");
            resultReceiver_.append(HIBERNATE_HELP_MSG);
            return ERR_OK;
        }
    }

    PowerMgrClient& client = PowerMgrClient::GetInstance();
    client.Hibernate(clearMemory);
    if (clearMemory) {
        resultReceiver_.append("Hibernate true is called\n");
    } else {
        resultReceiver_.append("Hibernate false is called\n");
    }
    return ERR_OK;
}

static const std::string GetBundleRunningLockTypeString(RunningLockType type)
{
    switch (type) {
        case RunningLockType::RUNNINGLOCK_SCREEN:
            return "SCREEN";
        case RunningLockType::RUNNINGLOCK_BACKGROUND:
            return "BACKGROUND";
        case RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL:
            return "PROXIMITY_SCREEN_CONTROL";
        case RunningLockType::RUNNINGLOCK_BACKGROUND_PHONE:
            return "BACKGROUND_PHONE";
        case RunningLockType::RUNNINGLOCK_BACKGROUND_NOTIFICATION:
            return "BACKGROUND_NOTIFICATION";
        case RunningLockType::RUNNINGLOCK_BACKGROUND_AUDIO:
            return "BACKGROUND_AUDIO";
        case RunningLockType::RUNNINGLOCK_BACKGROUND_SPORT:
            return "BACKGROUND_SPORT";
        case RunningLockType::RUNNINGLOCK_BACKGROUND_NAVIGATION:
            return "BACKGROUND_NAVIGATION";
        case RunningLockType::RUNNINGLOCK_BACKGROUND_TASK:
            return "BACKGROUND_TASK";
        case RunningLockType::RUNNINGLOCK_BUTT:
            return "BUTT";
        default:
            break;
    }

    return "UNKNOWN";
}

ErrCode PowerShellCommand::RunAsQueryLockCommand()
{
    if (!IsDeveloperMode()) {
        return ERR_PERMISSION_DENIED;
    }
    PowerMgrClient& client = PowerMgrClient::GetInstance();
    std::map<std::string, RunningLockInfo> runningLockLists;
    bool ret = client.QueryRunningLockLists(runningLockLists);
    if (!ret) {
        resultReceiver_.append("failed.\n");
        return ERR_OK;
    }
    resultReceiver_.append("The locking application information is as follows:\n");
    uint32_t mapSize = static_cast<uint32_t>(runningLockLists.size());
    resultReceiver_.append("The nums of holding lock by bundle app is ");
    resultReceiver_.append(std::to_string(mapSize));
    resultReceiver_.append(".\n");
    int counter = 0;
    for (auto it : runningLockLists) {
        counter++;
        resultReceiver_.append(std::to_string(counter));
        resultReceiver_.append(". bundleName=");
        resultReceiver_.append(it.second.bundleName);
        resultReceiver_.append(" name=");
        resultReceiver_.append(it.second.name);
        resultReceiver_.append(" type=");
        resultReceiver_.append(GetBundleRunningLockTypeString(it.second.type));
        resultReceiver_.append(" pid=");
        resultReceiver_.append(std::to_string(it.second.pid));
        resultReceiver_.append(" uid=");
        resultReceiver_.append(std::to_string(it.second.uid));
        resultReceiver_.append(".\n");
    }
    return ERR_OK;
}

ErrCode PowerShellCommand::RunAsProxyLockCommand()
{
    if (!IsDeveloperMode()) {
        return ERR_PERMISSION_DENIED;
    }
    int ind = 0;
    int option = getopt_long(argc_, argv_, "hp:u:", PROXYLOCK_OPTIONS, &ind);
    resultReceiver_.clear();
    if (option == 'h') {
        resultReceiver_.append(PROXYLOCK_HELP_MSG);
        return ERR_OK;
    }
    if (!optarg) {
        resultReceiver_.append("Error! please input your app uid.\n");
        resultReceiver_.append(PROXYLOCK_HELP_MSG);
        return ERR_OK;
    }
    int32_t uid = 0;
    StrToInt(optarg, uid);
    if (option == 'p') {
        bool ret = PowerMgrClient::GetInstance().ProxyRunningLock(true, INT32_MAX, uid);
        resultReceiver_.append("proxy runninglock for");
        resultReceiver_.append(std::to_string(uid));
        if (!ret) {
            resultReceiver_.append(" failed");
        }
        resultReceiver_.append("\n");
        return ERR_OK;
    }
    if (option == 'u') {
        bool ret = PowerMgrClient::GetInstance().ProxyRunningLock(false, INT32_MAX, uid);
        resultReceiver_.append("unproxy runninglock for");
        resultReceiver_.append(std::to_string(uid));
        if (!ret) {
            resultReceiver_.append(" failed");
        }
        resultReceiver_.append("\n");
        return ERR_OK;
    }
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
    if (!IsDeveloperMode()) {
        return ERR_PERMISSION_DENIED;
    }
    resultReceiver_.clear();

    PowerMgrClient& client = PowerMgrClient::GetInstance();
    std::string ret = client.Dump(argList_);
    resultReceiver_.append("Power Dump result: \n");
    resultReceiver_.append(ret);

    return ERR_OK;
}

#ifdef HAS_DISPLAY_MANAGER_PART
using namespace OHOS::DisplayPowerMgr;
bool PowerShellCommand::DisplayOptargEmpty()
{
    if (!optarg) {
        resultReceiver_.append("Error! please input your brightness value.\n");
        resultReceiver_.append(DISPLAY_HELP_MSG);
        return true;
    }
    return false;
}

ErrCode PowerShellCommand::RunAsDisplayCommandHelp()
{
    if (!IsDeveloperMode()) {
        return ERR_PERMISSION_DENIED;
    }
    resultReceiver_.append(DISPLAY_HELP_MSG);
    return ERR_OK;
}

ErrCode PowerShellCommand::RunAsDisplayCommandOverride()
{
    if (!IsDeveloperMode()) {
        return ERR_PERMISSION_DENIED;
    }
    if (DisplayOptargEmpty()) {
        return ERR_OK;
    }
    int32_t value = 0;
    StrToInt(optarg, value);
    bool ret = DisplayPowerMgrClient::GetInstance().OverrideBrightness(static_cast<uint32_t>(value));
    resultReceiver_.append("Override brightness to ");
    resultReceiver_.append(std::to_string(value));
    if (!ret) {
        resultReceiver_.append(" failed");
    }
    resultReceiver_.append("\n");
    return ERR_OK;
}

ErrCode PowerShellCommand::RunAsDisplayCommandRestore()
{
    if (!IsDeveloperMode()) {
        return ERR_PERMISSION_DENIED;
    }
    bool ret = DisplayPowerMgrClient::GetInstance().RestoreBrightness();
    resultReceiver_.append("Restore brightness");
    if (!ret) {
        resultReceiver_.append(" failed");
    }
    resultReceiver_.append("\n");
    return ERR_OK;
}

ErrCode PowerShellCommand::RunAsDisplayCommandBoost()
{
    if (!IsDeveloperMode()) {
        return ERR_PERMISSION_DENIED;
    }
    if (DisplayOptargEmpty()) {
        return ERR_OK;
    }
    int32_t value = 0;
    StrToInt(optarg, value);
    bool ret = DisplayPowerMgrClient::GetInstance().BoostBrightness(static_cast<uint32_t>(value));
    resultReceiver_.append("Boost brightness timeout ");
    resultReceiver_.append(std::to_string(value)).append("ms");
    if (!ret) {
        resultReceiver_.append(" failed");
    }
    resultReceiver_.append("\n");
    return ERR_OK;
}

ErrCode PowerShellCommand::RunAsDisplayCommandCancelBoost()
{
    if (!IsDeveloperMode()) {
        return ERR_PERMISSION_DENIED;
    }
    bool ret = DisplayPowerMgrClient::GetInstance().CancelBoostBrightness();
    resultReceiver_.append("Cancel boost brightness");
    if (!ret) {
        resultReceiver_.append(" failed");
    }
    resultReceiver_.append("\n");
    return ERR_OK;
}

ErrCode PowerShellCommand::RunAsDisplayCommandSetValue()
{
    if (!IsDeveloperMode()) {
        return ERR_PERMISSION_DENIED;
    }
    if (DisplayOptargEmpty()) {
        return ERR_OK;
    }
    int32_t value = 0;
    StrToInt(optarg, value);
    bool ret = DisplayPowerMgrClient::GetInstance().SetBrightness(static_cast<uint32_t>(value));
    resultReceiver_.append("Set brightness to ");
    resultReceiver_.append(std::to_string(value));
    if (!ret) {
        resultReceiver_.append(" failed");
    }
    resultReceiver_.append("\n");
    return ERR_OK;
}

ErrCode PowerShellCommand::RunAsDisplayCommandDiscount()
{
    if (!IsDeveloperMode()) {
        return ERR_PERMISSION_DENIED;
    }
    if (DisplayOptargEmpty()) {
        return ERR_OK;
    }
    std::stringstream fstr(optarg);
    double discount = 0;
    fstr >> discount;
    bool ret = DisplayPowerMgrClient::GetInstance().DiscountBrightness(discount);
    resultReceiver_.append("Set brightness discount to ");
    resultReceiver_.append(std::to_string(discount));
    if (!ret) {
        resultReceiver_.append(" failed");
    }
    resultReceiver_.append("\n");
    return ERR_OK;
}

ErrCode PowerShellCommand::RunAsDisplayCommand()
{
    if (!IsDeveloperMode()) {
        return ERR_PERMISSION_DENIED;
    }
    int ind = 0;
    int option = getopt_long(argc_, argv_, "hrcs:o:b:d:", DISPLAY_OPTIONS, &ind);
    resultReceiver_.clear();
    auto item = commandDisplay_.find(option);
    if (item != commandDisplay_.end()) {
        return item->second();
    }
    resultReceiver_.append(DISPLAY_HELP_MSG);
    return ERR_OK;
}
#endif
#endif

ErrCode PowerShellCommand::RunAsTimeOutCommand()
{
    if (!IsDeveloperMode()) {
        return ERR_PERMISSION_DENIED;
    }
    int ind = 0;
    int option = getopt_long(argc_, argv_, "hro:", TIME_OUT_OPTIONS, &ind);
    resultReceiver_.clear();
    if (option == 'h') {
        resultReceiver_.append(TIME_OUT_HELP_MSG);
        return ERR_OK;
    }
    if (option == 'r') {
        int ret = (int)PowerMgrClient::GetInstance().RestoreScreenOffTime();
        resultReceiver_.append("Restore screen off time");
        if (ret != ERR_OK) {
            resultReceiver_.append(" failed");
        }
        resultReceiver_.append("\n");
        return ERR_OK;
    }
    if (!optarg) {
        resultReceiver_.append("Error! please input your screen off time.\n");
        resultReceiver_.append(TIME_OUT_HELP_MSG);
        return ERR_OK;
    }
    if (option == 'o') {
        int32_t timeout = 0;
        StrToInt(optarg, timeout);
        int ret = (int)PowerMgrClient::GetInstance().OverrideScreenOffTime(static_cast<int64_t>(timeout));
        resultReceiver_.append("Override screen off time to ");
        resultReceiver_.append(std::to_string(timeout));
        if (ret != ERR_OK) {
            resultReceiver_.append(" failed");
        }
        resultReceiver_.append("\n");
        return ERR_OK;
    }
    return ERR_OK;
}
} // namespace PowerMgr
} // namespace OHOS
