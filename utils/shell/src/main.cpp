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

#include <iostream>
#include <sstream>

#include "power_shell_command.h"
#include "securec.h"

constexpr int MAX_PARAMETER_COUNT = 10;
constexpr int MAX_PARAMETER_LENGTH = 30;
void Loop(char* arg0)
{
    std::cout << "===================Power-Shell===================" << std::endl;
    std::cout << "You can run Power-Shell commands here without exiting the process(" << std::endl;
    std::cout << "help: to show help message" << std::endl;
    std::cout << "exit: to exit this programm" << std::endl;
    std::cout << "don't type \"power-shell\" again!" << std::endl;
    char* argv[MAX_PARAMETER_COUNT];
    for (int i = 0; i < MAX_PARAMETER_COUNT; ++i) {
        argv[i] = new char[MAX_PARAMETER_LENGTH];
    }
    argv[0] = arg0;
    std::string input;
    std::cout << "--------------------------------------------" << std::endl;
    std::cout << std::endl << "power-shell > ";
    std::getline(std::cin, input);
    while (input != "exit") {
        std::stringstream ss(input);
        int argc = 1;
        int ret = 0;
        for (; std::getline(ss, input, ' ') && argc < MAX_PARAMETER_COUNT; ++argc) {
            if (input == "exit") {
                return;
            }
            ret = strcpy_s(argv[argc], MAX_PARAMETER_LENGTH, input.c_str());
            if (ret != 0) {
                std::cout << "invalid input, the string is too long" << std::endl;
                break;
            }
        }
        if (ret == 0) {
            OHOS::PowerMgr::PowerShellCommand cmd(argc, argv);
            std::cout << cmd.ExecCommand();
        }
        std::cout << "--------------------------------------------" << std::endl;
        std::cout << std::endl << "power-shell > ";
        std::getline(std::cin, input);
    }
}
int main(int argc, char *argv[])
{
    if (argc <= 1) {
#ifndef POWER_SHELL_USER
        Loop(argv[0]);
#endif
    } else {
        OHOS::PowerMgr::PowerShellCommand cmd(argc, argv);
        std::cout << cmd.ExecCommand();
    }
    return 0;
}