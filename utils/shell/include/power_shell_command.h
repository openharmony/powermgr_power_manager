/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef POWER_SHELL_COMMAND_H
#define POWER_SHELL_COMMAND_H

#include <functional>
#include <getopt.h>
#include <map>
#include "shell_command.h"

#include "power_mgr_client.h"

namespace OHOS {
namespace PowerMgr {
class PowerShellCommand : public OHOS::AAFwk::ShellCommand {
public:
    PowerShellCommand(int argc, char *argv[]);
    ~PowerShellCommand() override {};

private:
    static bool IsDeveloperMode();
    ErrCode CreateCommandMap() override;
    ErrCode CreateMessageMap() override;
    ErrCode init() override;
    ErrCode RunAsHelpCommand();
    ErrCode RunAsDumpCommand();
    ErrCode RunAsSetModeCommand();
    ErrCode RunAsWakeupCommand();
    ErrCode RunAsSuspendCommand();
    ErrCode RunAsHibernateCommand();
    ErrCode RunAsQueryLockCommand();
    ErrCode RunAsProxyLockCommand();
    ErrCode RunAsForceTimeOutCommand();
    ErrCode RunAsTimeOutScreenLockCommand();
#ifdef HAS_DISPLAY_MANAGER_PART
    ErrCode RunAsDisplayCommand();
    bool DisplayOptargEmpty();
    ErrCode RunAsDisplayCommandHelp();
    ErrCode RunAsDisplayCommandOverride();
    ErrCode RunAsDisplayCommandRestore();
    ErrCode RunAsDisplayCommandBoost();
    ErrCode RunAsDisplayCommandCancelBoost();
    ErrCode RunAsDisplayCommandSetValue();
    ErrCode RunAsDisplayCommandDiscount();
    std::map<char, std::function<int32_t()>> commandDisplay_;
#endif
    ErrCode RunAsTimeOutCommand();
};
}  // namespace PowerMgr
}  // namespace OHOS
#endif  // POWER_SHELL_COMMAND_H
