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

#include <gtest/gtest.h>

#ifdef POWER_GTEST
#define private    public
#define protected  public
#endif
#include <parameters.h>
#include <power_log.h>
#include <power_mgr_client.h>
#include <power_shell_command.h>
#ifdef HAS_DISPLAY_MANAGER_PART
#include "display_power_mgr_client.h"
#endif

namespace {
std::string g_lastCalled {};
}

namespace OHOS {
namespace system {
bool GetBoolParameter(const std::string&, bool)
{
    return true;
}
} // namespace system

namespace DisplayPowerMgr {
#ifdef HAS_DISPLAY_MANAGER_PART
bool DisplayPowerMgrClient::OverrideBrightness(uint32_t value, uint32_t displayId, uint32_t duration)
{
    g_lastCalled = "OverrideBrightness";
    if (value == 2333) {
        return false;
    }
    return true;
}

bool DisplayPowerMgrClient::RestoreBrightness(uint32_t displayId, uint32_t duration)
{
    if (g_lastCalled != "RestoreBrightness") {
        g_lastCalled = "RestoreBrightness";
        return false;
    }
    g_lastCalled = "RestoreBrightness";
    return true;
}

bool DisplayPowerMgrClient::BoostBrightness(int32_t timeoutMs, uint32_t displayId)
{
    g_lastCalled = "BoostBrightness";
    if (timeoutMs == 2333) {
        return false;
    }
    return true;
}

bool DisplayPowerMgrClient::CancelBoostBrightness(uint32_t displayId)
{
    if (g_lastCalled != "CancelBoostBrightness") {
        g_lastCalled = "CancelBoostBrightness";
        return false;
    }
    g_lastCalled = "CancelBoostBrightness";
    return true;
}

bool DisplayPowerMgrClient::SetBrightness(uint32_t value, uint32_t displayId, bool continuous)
{
    g_lastCalled = "SetBrightness";
    if (value == 2333) {
        return false;
    }
    return true;
}

bool DisplayPowerMgrClient::DiscountBrightness(double discount, uint32_t displayId)
{
    g_lastCalled = "DiscountBrightness";
    if (discount == 0.2) {
        return false;
    }
    return true;
}
#endif
} // namespace DisplayPowerMgr

namespace PowerMgr {
using namespace testing::ext;

PowerErrors PowerMgrClient::LockScreenAfterTimingOut(bool, bool, bool)
{
    g_lastCalled = "LockScreenAfterTimingOut";
    return PowerErrors::ERR_OK;
}

PowerErrors PowerMgrClient::LockScreenAfterTimingOutWithAppid(pid_t, bool)
{
    g_lastCalled = "LockScreenAfterTimingOutWithAppid";
    return PowerErrors::ERR_OK;
}

PowerErrors PowerMgrClient::SetForceTimingOut(bool enabled)
{
    g_lastCalled = "SetForceTimingOut";
    return PowerErrors::ERR_OK;
}

PowerErrors PowerMgrClient::WakeupDevice(
    WakeupDeviceType reason, const std::string& detail, const std::string& apiVersion)
{
    g_lastCalled = "WakeupDevice";
    return PowerErrors::ERR_OK;
}

PowerErrors PowerMgrClient::Hibernate(bool clearMemory, const std::string& reason, const std::string& apiVersion)
{
    g_lastCalled = "Hibernate";
    return PowerErrors::ERR_OK;
}

bool PowerMgrClient::QueryRunningLockLists(std::map<std::string, RunningLockInfo>& runningLockLists)
{
    runningLockLists.clear();
    std::string lockid1 = "lockid1";
    RunningLockInfo runningLockInfo1("locktest1", RunningLockType::RUNNINGLOCK_SCREEN);
    runningLockInfo1.bundleName = "bundle1";
    runningLockInfo1.pid = 1;
    runningLockInfo1.uid = 1;
    std::string lockid2 = "lockid2";
    RunningLockInfo runningLockInfo2("locktest2", RunningLockType::RUNNINGLOCK_BACKGROUND);
    runningLockInfo2.bundleName = "bundle2";
    runningLockInfo2.pid = 2;
    runningLockInfo2.uid = 2;
    std::string lockid3 = "lockid3";
    RunningLockInfo runningLockInfo3("locktest3", RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL);
    runningLockInfo3.bundleName = "bundle3";
    runningLockInfo3.pid = 3;
    runningLockInfo3.uid = 3;
    std::string lockid4 = "lockid4";
    RunningLockInfo runningLockInfo4("locktest4", RunningLockType::RUNNINGLOCK_BACKGROUND_PHONE);
    runningLockInfo4.bundleName = "bundle4";
    runningLockInfo4.pid = 4;
    runningLockInfo4.uid = 4;
    std::string lockid5 = "lockid5";
    RunningLockInfo runningLockInfo5("locktest5", RunningLockType::RUNNINGLOCK_BACKGROUND_NOTIFICATION);
    runningLockInfo5.bundleName = "bundle5";
    runningLockInfo5.pid = 5;
    runningLockInfo5.uid = 5;
    std::string lockid6 = "lockid6";
    RunningLockInfo runningLockInfo6("locktest6", RunningLockType::RUNNINGLOCK_BACKGROUND_AUDIO);
    runningLockInfo6.bundleName = "bundle6";
    runningLockInfo6.pid = 6;
    runningLockInfo6.uid = 6;
    std::string lockid7 = "lockid7";
    RunningLockInfo runningLockInfo7("locktest7", RunningLockType::RUNNINGLOCK_BACKGROUND_SPORT);
    runningLockInfo7.bundleName = "bundle7";
    runningLockInfo7.pid = 7;
    runningLockInfo7.uid = 7;
    std::string lockid8 = "lockid8";
    RunningLockInfo runningLockInfo8("locktest8", RunningLockType::RUNNINGLOCK_BACKGROUND_NAVIGATION);
    runningLockInfo8.bundleName = "bundle8";
    runningLockInfo8.pid = 8;
    runningLockInfo8.uid = 8;
    std::string lockid9 = "lockid9";
    RunningLockInfo runningLockInfo9("locktest9", RunningLockType::RUNNINGLOCK_BACKGROUND_TASK);
    runningLockInfo9.bundleName = "bundle9";
    runningLockInfo9.pid = 9;
    runningLockInfo9.uid = 9;
    std::string lockid10 = "lockid10";
    RunningLockInfo runningLockInfo10("locktest10", RunningLockType::RUNNINGLOCK_BUTT);
    runningLockInfo10.bundleName = "bundle10";
    runningLockInfo10.pid = 10;
    runningLockInfo10.uid = 10;
    std::string lockid11 = "lockid11";
    RunningLockInfo runningLockInfo11("locktest11", static_cast<RunningLockType>(-1));
    runningLockInfo11.bundleName = "bundle11";
    runningLockInfo11.pid = 11;
    runningLockInfo11.uid = 11;
    runningLockLists.insert(std::pair<std::string, RunningLockInfo>(lockid1, runningLockInfo1));
    runningLockLists.insert(std::pair<std::string, RunningLockInfo>(lockid2, runningLockInfo2));
    runningLockLists.insert(std::pair<std::string, RunningLockInfo>(lockid3, runningLockInfo3));
    runningLockLists.insert(std::pair<std::string, RunningLockInfo>(lockid4, runningLockInfo4));
    runningLockLists.insert(std::pair<std::string, RunningLockInfo>(lockid5, runningLockInfo5));
    runningLockLists.insert(std::pair<std::string, RunningLockInfo>(lockid6, runningLockInfo6));
    runningLockLists.insert(std::pair<std::string, RunningLockInfo>(lockid7, runningLockInfo7));
    runningLockLists.insert(std::pair<std::string, RunningLockInfo>(lockid8, runningLockInfo8));
    runningLockLists.insert(std::pair<std::string, RunningLockInfo>(lockid9, runningLockInfo9));
    runningLockLists.insert(std::pair<std::string, RunningLockInfo>(lockid10, runningLockInfo10));
    runningLockLists.insert(std::pair<std::string, RunningLockInfo>(lockid11, runningLockInfo11));
    g_lastCalled = "QueryRunningLockLists";
    return true;
}

bool PowerMgrClient::ProxyRunningLock(bool isProxied, pid_t pid, pid_t uid)
{
    g_lastCalled = "ProxyRunningLock";
    return false;
}

std::string PowerMgrClient::Dump(const std::vector<std::string>& args)
{
    g_lastCalled = "Dump";
    return g_lastCalled;
}

PowerErrors PowerMgrClient::RestoreScreenOffTime(const std::string& apiVersion)
{
    if (g_lastCalled == "") {
        g_lastCalled = "RestoreScreenOffTime";
        return PowerErrors::ERR_FAILURE;
    }
    g_lastCalled = "RestoreScreenOffTime";
    return PowerErrors::ERR_OK;
}

class PowerShellTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown()
    {
        g_lastCalled.clear();
    }
};

namespace {
/**
 * @tc.name: PowerShellTest001
 * @tc.desc: Test PowerShellCommand RunAsTimeOutScreenLockCommand
 * @tc.type: FUNC
 */
HWTEST_F(PowerShellTest, PowerShellTest001, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerShellTest001 start");
    char name[] = "power_shell_test";
    char command[] = "timeoutscreenlock";
    char paramAppid[] = "11";
    char paramBool[] = "1";
    char* argvTwoParamsWithId[] = {name, command, paramAppid, paramBool};
    char* argvTwoParamsWithoutId[] = {name, command, paramBool, paramBool};
    char* argvThreeParamsWithId[] = {name, command, paramAppid, paramBool, paramBool};
    char* argvThreeParamWithoutId[] = {name, command, paramBool, paramBool, paramBool};

    PowerShellCommand cmd1{sizeof(argvTwoParamsWithId) / sizeof(char*), argvTwoParamsWithId};
    cmd1.ExecCommand();
    EXPECT_EQ(g_lastCalled, "LockScreenAfterTimingOutWithAppid");
    g_lastCalled.clear();

    PowerShellCommand cmd2{sizeof(argvTwoParamsWithoutId) / sizeof(char*), argvTwoParamsWithoutId};
    cmd2.ExecCommand();
    EXPECT_EQ(g_lastCalled, "LockScreenAfterTimingOut");
    g_lastCalled.clear();

    PowerShellCommand cmd3{sizeof(argvThreeParamsWithId) / sizeof(char*), argvThreeParamsWithId};
    cmd3.ExecCommand();
    EXPECT_EQ(g_lastCalled, "LockScreenAfterTimingOut");
    g_lastCalled.clear();

    PowerShellCommand cmd4{sizeof(argvThreeParamWithoutId) / sizeof(char*), argvThreeParamWithoutId};
    cmd4.ExecCommand();
    EXPECT_EQ(g_lastCalled, "LockScreenAfterTimingOut");
    g_lastCalled.clear();

    POWER_HILOGI(LABEL_TEST, "PowerShellTest001 end");
}

/**
 * @tc.name: PowerShellTest002
 * @tc.desc: Test PowerShellCommand RunAsForceTimeOutCommand
 * @tc.type: FUNC
 */
HWTEST_F(PowerShellTest, PowerShellTest002, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerShellTest002 start");
    char name[] = "power_shell_test";
    char command[] = "forcetimeout";
    char paramBool[] = "1";
    char* argv[] = {name, command};
    char* argvOneParams[] = {name, command, paramBool};

    g_lastCalled.clear();
    PowerShellCommand cmd1{sizeof(argv) / sizeof(char*), argv};
    cmd1.ExecCommand();
    std::string expectedCode = "too few arguments \n";
    EXPECT_EQ(cmd1.resultReceiver_, expectedCode);

    PowerShellCommand cmd2{sizeof(argvOneParams) / sizeof(char*), argvOneParams};
    cmd2.ExecCommand();
    EXPECT_EQ(g_lastCalled, "SetForceTimingOut");

    POWER_HILOGI(LABEL_TEST, "PowerShellTest002 end");
}

/**
 * @tc.name: PowerShellTest003
 * @tc.desc: Test PowerShellCommand RunAsHelpCommand
 * @tc.type: FUNC
 */
HWTEST_F(PowerShellTest, PowerShellTest003, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerShellTest003 start");
    char name[] = "power_shell_test";
    char command[] = "help";
    char paramBool[] = "1";
    char* argv[] = {name, command};
    char* argvOneParams[] = {name, command, paramBool};

    PowerShellCommand cmd1{sizeof(argv) / sizeof(char*), argv};
    cmd1.ExecCommand();
    std::string expectedCode =
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
    EXPECT_EQ(cmd1.resultReceiver_, expectedCode);

    PowerShellCommand cmd2{sizeof(argvOneParams) / sizeof(char*), argvOneParams};
    cmd2.ExecCommand();
    EXPECT_EQ(cmd2.resultReceiver_, expectedCode);

    POWER_HILOGI(LABEL_TEST, "PowerShellTest003 end");
}

/**
 * @tc.name: PowerShellTest004
 * @tc.desc: Test PowerShellCommand RunAsWakeupCommand
 * @tc.type: FUNC
 */
HWTEST_F(PowerShellTest, PowerShellTest004, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerShellTest004 start");
    char name[] = "power_shell_test";
    char command[] = "wakeup";
    char paramOption[] = "";
    char paramOptionA[] = "-a";
    char paramOptionB[] = "-b";
    char paramOptionC[] = "-c";
    char paramOptionD[] = "-d";
    char paramOptionE[] = "-e";
    char* argv[] = {name, command};
    char* argvOneParams[] = {name, command, paramOption};
    char* argvOneParamsA[] = {name, command, paramOptionA};
    char* argvOneParamsB[] = {name, command, paramOptionB};
    char* argvOneParamsC[] = {name, command, paramOptionC};
    char* argvOneParamsD[] = {name, command, paramOptionD};
    char* argvOneParamsE[] = {name, command, paramOptionE};

    g_lastCalled.clear();
    PowerShellCommand cmd1{sizeof(argv) / sizeof(char*), argv};
    cmd1.ExecCommand();
    std::string expectedCode = "WakeupDevice is called\n";
    EXPECT_EQ(cmd1.resultReceiver_, expectedCode);
    EXPECT_EQ(g_lastCalled, "WakeupDevice");

    PowerShellCommand cmd2{sizeof(argvOneParams) / sizeof(char*), argvOneParams};
    cmd2.ExecCommand();
    expectedCode = "WakeupDevice is called\n";
    EXPECT_EQ(cmd2.resultReceiver_, expectedCode);
    EXPECT_EQ(g_lastCalled, "WakeupDevice");

    PowerShellCommand cmd3{sizeof(argvOneParamsA) / sizeof(char*), argvOneParamsA};
    cmd3.ExecCommand();
    expectedCode =
        "pre_bright is called\n"
        "WakeupDevice is called\n";
    EXPECT_EQ(cmd3.resultReceiver_, expectedCode);
    EXPECT_EQ(g_lastCalled, "WakeupDevice");

    PowerShellCommand cmd4{sizeof(argvOneParamsB) / sizeof(char*), argvOneParamsB};
    cmd4.ExecCommand();
    expectedCode =
        "pre_bright_auth_success is called\n"
        "WakeupDevice is called\n";
    EXPECT_EQ(cmd4.resultReceiver_, expectedCode);
    EXPECT_EQ(g_lastCalled, "WakeupDevice");

    PowerShellCommand cmd5{sizeof(argvOneParamsC) / sizeof(char*), argvOneParamsC};
    cmd5.ExecCommand();
    expectedCode = "WakeupDevice is called\n";
    EXPECT_EQ(cmd5.resultReceiver_, expectedCode);
    EXPECT_EQ(g_lastCalled, "WakeupDevice");

    PowerShellCommand cmd6{sizeof(argvOneParamsD) / sizeof(char*), argvOneParamsD};
    cmd6.ExecCommand();
    expectedCode = "WakeupDevice is called\n";
    EXPECT_EQ(cmd6.resultReceiver_, expectedCode);
    EXPECT_EQ(g_lastCalled, "WakeupDevice");

    PowerShellCommand cmd7{sizeof(argvOneParamsE) / sizeof(char*), argvOneParamsE};
    cmd7.ExecCommand();
    expectedCode = "WakeupDevice is called\n";
    EXPECT_EQ(cmd7.resultReceiver_, expectedCode);
    EXPECT_EQ(g_lastCalled, "WakeupDevice");

    POWER_HILOGI(LABEL_TEST, "PowerShellTest004 end");
}

#ifndef POWER_SHELL_USER
/**
 * @tc.name: PowerShellTest005
 * @tc.desc: Test PowerShellCommand RunAsHibernateCommand
 * @tc.type: FUNC
 */
HWTEST_F(PowerShellTest, PowerShellTest005, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerShellTest005 start");
    char name[] = "power_shell_test";
    char command[] = "hibernate";
    char paramOption[] = "";
    char paramOptionH[] = "-h";
    char paramOptionF[] = "false";
    char paramOptionT[] = "true";
    char paramOptionU[] = "-u";
    char* argv[] = {name, command};
    char* argvOneParams[] = {name, command, paramOption};
    char* argvOneParamsH[] = {name, command, paramOptionH};
    char* argvOneParamsF[] = {name, command, paramOptionF};
    char* argvOneParamsT[] = {name, command, paramOptionT};
    char* argvOneParamsU[] = {name, command, paramOptionU};

    g_lastCalled.clear();
    PowerShellCommand cmd1{sizeof(argv) / sizeof(char*), argv};
    cmd1.ExecCommand();
    std::string expectedCode = "Hibernate false is called\n";
    EXPECT_EQ(cmd1.resultReceiver_, expectedCode);
    EXPECT_EQ(g_lastCalled, "Hibernate");

    PowerShellCommand cmd2{sizeof(argvOneParams) / sizeof(char*), argvOneParams};
    cmd2.ExecCommand();
    expectedCode =
        "Error! please input your option value. \n"
        "usage: power-shell hibernate [<options>]\n"
        "  hibernate <options are as below> \n"
        "  the default option is false\n"
        "  true  :  clear memory before hibernate\n"
        "  false :  skip clearing memory before hibernate\n";
    EXPECT_EQ(cmd2.resultReceiver_, expectedCode);
    EXPECT_EQ(g_lastCalled, "Hibernate");

    PowerShellCommand cmd3{sizeof(argvOneParamsH) / sizeof(char*), argvOneParamsH};
    cmd3.ExecCommand();
    expectedCode =
        "Error! please input your option value. \n"
        "usage: power-shell hibernate [<options>]\n"
        "  hibernate <options are as below> \n"
        "  the default option is false\n"
        "  true  :  clear memory before hibernate\n"
        "  false :  skip clearing memory before hibernate\n";
    EXPECT_EQ(cmd3.resultReceiver_, expectedCode);

    PowerShellCommand cmd4{sizeof(argvOneParamsF) / sizeof(char*), argvOneParamsF};
    cmd4.ExecCommand();
    expectedCode = "Hibernate false is called\n";
    EXPECT_EQ(cmd4.resultReceiver_, expectedCode);
    EXPECT_EQ(g_lastCalled, "Hibernate");

    PowerShellCommand cmd5{sizeof(argvOneParamsT) / sizeof(char*), argvOneParamsT};
    cmd5.ExecCommand();
    expectedCode = "Hibernate true is called\n";
    EXPECT_EQ(cmd5.resultReceiver_, expectedCode);
    EXPECT_EQ(g_lastCalled, "Hibernate");

    PowerShellCommand cmd6{sizeof(argvOneParamsU) / sizeof(char*), argvOneParamsU};
    cmd6.ExecCommand();
    expectedCode =
        "Error! please input your option value. \n"
        "usage: power-shell hibernate [<options>]\n"
        "  hibernate <options are as below> \n"
        "  the default option is false\n"
        "  true  :  clear memory before hibernate\n"
        "  false :  skip clearing memory before hibernate\n";
    EXPECT_EQ(cmd6.resultReceiver_, expectedCode);

    POWER_HILOGI(LABEL_TEST, "PowerShellTest005 end");
}

/**
 * @tc.name: PowerShellTest006
 * @tc.desc: Test PowerShellCommand RunAsQueryLockCommand
 * @tc.type: FUNC
 */
HWTEST_F(PowerShellTest, PowerShellTest006, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerShellTest006 start");
    char name[] = "power_shell_test";
    char command[] = "lock";
    char paramOption[] = "";
    char paramOptionU[] = "-u";
    char* argv[] = {name, command};
    char* argvOneParams[] = {name, command, paramOption};
    char* argvOneParamsU[] = {name, command, paramOptionU};

    g_lastCalled.clear();
    PowerShellCommand cmd1{sizeof(argv) / sizeof(char*), argv};
    cmd1.ExecCommand();
    std::string expectedCode =
        "The locking application information is as follows:\n"
        "The nums of holding lock by bundle app is 11.\n"
        "1. bundleName=bundle1 name=locktest1 type=SCREEN pid=1 uid=1.\n"
        "2. bundleName=bundle10 name=locktest10 type=BUTT pid=10 uid=10.\n"
        "3. bundleName=bundle11 name=locktest11 type=UNKNOWN pid=11 uid=11.\n"
        "4. bundleName=bundle2 name=locktest2 type=BACKGROUND pid=2 uid=2.\n"
        "5. bundleName=bundle3 name=locktest3 type=PROXIMITY_SCREEN_CONTROL pid=3 uid=3.\n"
        "6. bundleName=bundle4 name=locktest4 type=BACKGROUND_PHONE pid=4 uid=4.\n"
        "7. bundleName=bundle5 name=locktest5 type=BACKGROUND_NOTIFICATION pid=5 uid=5.\n"
        "8. bundleName=bundle6 name=locktest6 type=BACKGROUND_AUDIO pid=6 uid=6.\n"
        "9. bundleName=bundle7 name=locktest7 type=BACKGROUND_SPORT pid=7 uid=7.\n"
        "10. bundleName=bundle8 name=locktest8 type=BACKGROUND_NAVIGATION pid=8 uid=8.\n"
        "11. bundleName=bundle9 name=locktest9 type=BACKGROUND_TASK pid=9 uid=9.\n";
    EXPECT_EQ(cmd1.resultReceiver_, expectedCode);
    EXPECT_EQ(g_lastCalled, "QueryRunningLockLists");

    PowerShellCommand cmd2{sizeof(argvOneParams) / sizeof(char*), argvOneParams};
    cmd2.ExecCommand();
    expectedCode =
        "The locking application information is as follows:\n"
        "The nums of holding lock by bundle app is 11.\n"
        "1. bundleName=bundle1 name=locktest1 type=SCREEN pid=1 uid=1.\n"
        "2. bundleName=bundle10 name=locktest10 type=BUTT pid=10 uid=10.\n"
        "3. bundleName=bundle11 name=locktest11 type=UNKNOWN pid=11 uid=11.\n"
        "4. bundleName=bundle2 name=locktest2 type=BACKGROUND pid=2 uid=2.\n"
        "5. bundleName=bundle3 name=locktest3 type=PROXIMITY_SCREEN_CONTROL pid=3 uid=3.\n"
        "6. bundleName=bundle4 name=locktest4 type=BACKGROUND_PHONE pid=4 uid=4.\n"
        "7. bundleName=bundle5 name=locktest5 type=BACKGROUND_NOTIFICATION pid=5 uid=5.\n"
        "8. bundleName=bundle6 name=locktest6 type=BACKGROUND_AUDIO pid=6 uid=6.\n"
        "9. bundleName=bundle7 name=locktest7 type=BACKGROUND_SPORT pid=7 uid=7.\n"
        "10. bundleName=bundle8 name=locktest8 type=BACKGROUND_NAVIGATION pid=8 uid=8.\n"
        "11. bundleName=bundle9 name=locktest9 type=BACKGROUND_TASK pid=9 uid=9.\n";
    EXPECT_EQ(cmd2.resultReceiver_, expectedCode);
    EXPECT_EQ(g_lastCalled, "QueryRunningLockLists");

    PowerShellCommand cmd3{sizeof(argvOneParamsU) / sizeof(char*), argvOneParamsU};
    cmd3.ExecCommand();
    expectedCode =
        "The locking application information is as follows:\n"
        "The nums of holding lock by bundle app is 11.\n"
        "1. bundleName=bundle1 name=locktest1 type=SCREEN pid=1 uid=1.\n"
        "2. bundleName=bundle10 name=locktest10 type=BUTT pid=10 uid=10.\n"
        "3. bundleName=bundle11 name=locktest11 type=UNKNOWN pid=11 uid=11.\n"
        "4. bundleName=bundle2 name=locktest2 type=BACKGROUND pid=2 uid=2.\n"
        "5. bundleName=bundle3 name=locktest3 type=PROXIMITY_SCREEN_CONTROL pid=3 uid=3.\n"
        "6. bundleName=bundle4 name=locktest4 type=BACKGROUND_PHONE pid=4 uid=4.\n"
        "7. bundleName=bundle5 name=locktest5 type=BACKGROUND_NOTIFICATION pid=5 uid=5.\n"
        "8. bundleName=bundle6 name=locktest6 type=BACKGROUND_AUDIO pid=6 uid=6.\n"
        "9. bundleName=bundle7 name=locktest7 type=BACKGROUND_SPORT pid=7 uid=7.\n"
        "10. bundleName=bundle8 name=locktest8 type=BACKGROUND_NAVIGATION pid=8 uid=8.\n"
        "11. bundleName=bundle9 name=locktest9 type=BACKGROUND_TASK pid=9 uid=9.\n";
    EXPECT_EQ(cmd3.resultReceiver_, expectedCode);
    EXPECT_EQ(g_lastCalled, "QueryRunningLockLists");

    POWER_HILOGI(LABEL_TEST, "PowerShellTest006 end");
}

/**
 * @tc.name: PowerShellTest007
 * @tc.desc: Test PowerShellCommand RunAsProxyLockCommand
 * @tc.type: FUNC
 */
HWTEST_F(PowerShellTest, PowerShellTest007, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerShellTest007 start");
    char name[] = "power_shell_test";
    char command[] = "proxylock";
    char paramOptionH[] = "-h";
    char paramOptionP[] = "-p";
    char paramOptionPV[] = "12";
    char paramOptionU[] = "-u 92";
    char paramOptionF[] = "-f 923";
    char* argv[] = {name, command};
    char* argvOneParamsH[] = {name, command, paramOptionH};
    char* argvOneParamsP[] = {name, command, paramOptionP, paramOptionPV};
    char* argvOneParamsU[] = {name, command, paramOptionU};
    char* argvOneParamsF[] = {name, command, paramOptionF};

    g_lastCalled.clear();
    PowerShellCommand cmd1{sizeof(argv) / sizeof(char*), argv};
    cmd1.ExecCommand();
    std::string expectedCode =
        "Error! please input your app uid.\n"
        "usage: power-shell proxylock [<options>] 20020041\n"
        "proxylock <options are as below> \n"
        "  -p  :  proxy runninglock\n"
        "  -u  :  unproxy runninglock\n";
    EXPECT_EQ(cmd1.resultReceiver_, expectedCode);

    PowerShellCommand cmd2{sizeof(argvOneParamsH) / sizeof(char*), argvOneParamsH};
    cmd2.ExecCommand();
    expectedCode =
        "Error! please input your app uid.\n"
        "usage: power-shell proxylock [<options>] 20020041\n"
        "proxylock <options are as below> \n"
        "  -p  :  proxy runninglock\n"
        "  -u  :  unproxy runninglock\n";
    EXPECT_EQ(cmd2.resultReceiver_, expectedCode);

    PowerShellCommand cmd3{sizeof(argvOneParamsF) / sizeof(char*), argvOneParamsF};
    cmd3.ExecCommand();

    POWER_HILOGI(LABEL_TEST, "PowerShellTest007 end");
}

/**
 * @tc.name: PowerShellTest008
 * @tc.desc: Test PowerShellCommand RunAsDumpCommand
 * @tc.type: FUNC
 */
HWTEST_F(PowerShellTest, PowerShellTest008, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerShellTest008 start");
    char name[] = "power_shell_test";
    char command[] = "dump";
    char* argv[] = {name, command};

    g_lastCalled.clear();
    PowerShellCommand cmd1{sizeof(argv) / sizeof(char*), argv};
    cmd1.ExecCommand();
    std::string expectedCode =
        "Power Dump result: \n"
        "Dump";
    EXPECT_EQ(cmd1.resultReceiver_, expectedCode);

    POWER_HILOGI(LABEL_TEST, "PowerShellTest008 end");
}

#ifdef HAS_DISPLAY_MANAGER_PART
/**
 * @tc.name: PowerShellTest009
 * @tc.desc: Test PowerShellCommand RunAsDisplayCommand RunAsDisplayCommandHelp
 * @tc.type: FUNC
 */
HWTEST_F(PowerShellTest, PowerShellTest009, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerShellTest009 start");
    char name[] = "power_shell_test";
    char command[] = "display";
    char paramOptionH[] = "-h";
    char* argvOneParamsH[] = {name, command, paramOptionH};

    g_lastCalled.clear();
    PowerShellCommand cmd1{sizeof(argvOneParamsH) / sizeof(char*), argvOneParamsH};
    cmd1.ExecCommand();
    std::string expectedCode =
        "usage: power-shell display [<options>] 100\n"
        "display <options are as below> \n"
        "  -h  :  display help\n"
        "  -r  :  retore brightness\n"
        "  -s  :  set brightness\n"
        "  -o  :  override brightness\n"
        "  -b  :  timing maximum brightness\n"
        "  -c  :  cancel the timing maximum brightness\n"
        "  -d  :  discount brightness\n";
    EXPECT_EQ(cmd1.resultReceiver_, expectedCode);

    POWER_HILOGI(LABEL_TEST, "PowerShellTest009 end");
}

/**
 * @tc.name: PowerShellTest010
 * @tc.desc: Test PowerShellCommand RunAsDisplayCommand RunAsDisplayCommandOverride
 * @tc.type: FUNC
 */
HWTEST_F(PowerShellTest, PowerShellTest010, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerShellTest010 start");
    char name[] = "power_shell_test";
    char command[] = "display";
    char paramOptionO[] = "-o";
    char paramBrightness[] = "100";
    char paramBrightness2[] = "2333";
    char* argvOneParamsO[] = {name, command, paramOptionO};
    char* argvOneParamsOB[] = {name, command, paramOptionO, paramBrightness};
    char* argvOneParamsOB2[] = {name, command, paramOptionO, paramBrightness2};

    g_lastCalled.clear();
    PowerShellCommand cmd1{sizeof(argvOneParamsO) / sizeof(char*), argvOneParamsO};
    cmd1.ExecCommand();
    std::string expectedCode =
        "usage: power-shell display [<options>] 100\n"
        "display <options are as below> \n"
        "  -h  :  display help\n"
        "  -r  :  retore brightness\n"
        "  -s  :  set brightness\n"
        "  -o  :  override brightness\n"
        "  -b  :  timing maximum brightness\n"
        "  -c  :  cancel the timing maximum brightness\n"
        "  -d  :  discount brightness\n";
    EXPECT_EQ(cmd1.resultReceiver_, expectedCode);

    PowerShellCommand cmd2{sizeof(argvOneParamsOB) / sizeof(char*), argvOneParamsOB};
    cmd2.ExecCommand();
    expectedCode =
        "usage: power-shell display [<options>] 100\n"
        "display <options are as below> \n"
        "  -h  :  display help\n"
        "  -r  :  retore brightness\n"
        "  -s  :  set brightness\n"
        "  -o  :  override brightness\n"
        "  -b  :  timing maximum brightness\n"
        "  -c  :  cancel the timing maximum brightness\n"
        "  -d  :  discount brightness\n";
    EXPECT_EQ(cmd2.resultReceiver_, expectedCode);
    EXPECT_EQ(g_lastCalled, "");

    POWER_HILOGI(LABEL_TEST, "PowerShellTest010 end");
}

/**
 * @tc.name: PowerShellTest011
 * @tc.desc: Test PowerShellCommand RunAsDisplayCommand RunAsDisplayCommandRestore
 * @tc.type: FUNC
 */
HWTEST_F(PowerShellTest, PowerShellTest011, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerShellTest011 start");
    char name[] = "power_shell_test";
    char command[] = "display";
    char paramOptionR[] = "-r";
    char paramBrightness[] = "100";
    char* argvOneParamsR[] = {name, command, paramOptionR};
    char* argvOneParamsRB[] = {name, command, paramOptionR, paramBrightness};

    g_lastCalled.clear();
    PowerShellCommand cmd1{sizeof(argvOneParamsRB) / sizeof(char*), argvOneParamsRB};
    cmd1.RunAsDisplayCommandRestore();
    std::string expectedCode = "Restore brightness failed\n";
    EXPECT_EQ(cmd1.resultReceiver_, expectedCode);
    EXPECT_EQ(g_lastCalled, "RestoreBrightness");

    PowerShellCommand cmd2{sizeof(argvOneParamsR) / sizeof(char*), argvOneParamsR};
    cmd2.RunAsDisplayCommandRestore();
    expectedCode = "Restore brightness\n";
    EXPECT_EQ(cmd2.resultReceiver_, expectedCode);
    EXPECT_EQ(g_lastCalled, "RestoreBrightness");

    POWER_HILOGI(LABEL_TEST, "PowerShellTest011 end");
}

/**
 * @tc.name: PowerShellTest012
 * @tc.desc: Test PowerShellCommand RunAsDisplayCommand RunAsDisplayCommandBoost
 * @tc.type: FUNC
 */
HWTEST_F(PowerShellTest, PowerShellTest012, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerShellTest012 start");
    char name[] = "power_shell_test";
    char command[] = "display";
    char paramOptionB[] = "-b";
    char paramValue[] = "100";
    char paramValue2[] = "2333";
    char* argvOneParamsB[] = {name, command, paramOptionB};
    char* argvOneParamsBV[] = {name, command, paramOptionB, paramValue};
    char* argvOneParamsBV2[] = {name, command, paramOptionB, paramValue2};

    g_lastCalled.clear();
    PowerShellCommand cmd1{sizeof(argvOneParamsB) / sizeof(char*), argvOneParamsB};
    cmd1.ExecCommand();
    std::string expectedCode =
        "usage: power-shell display [<options>] 100\n"
        "display <options are as below> \n"
        "  -h  :  display help\n"
        "  -r  :  retore brightness\n"
        "  -s  :  set brightness\n"
        "  -o  :  override brightness\n"
        "  -b  :  timing maximum brightness\n"
        "  -c  :  cancel the timing maximum brightness\n"
        "  -d  :  discount brightness\n";
    EXPECT_EQ(cmd1.resultReceiver_, expectedCode);

    POWER_HILOGI(LABEL_TEST, "PowerShellTest012 end");
}

/**
 * @tc.name: PowerShellTest013
 * @tc.desc: Test PowerShellCommand RunAsDisplayCommand CancelBoostBrightness
 * @tc.type: FUNC
 */
HWTEST_F(PowerShellTest, PowerShellTest013, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerShellTest013 start");
    char name[] = "power_shell_test";
    char command[] = "display";
    char paramOptionB[] = "-c";
    char* argvOneParamsB[] = {name, command, paramOptionB};

    g_lastCalled.clear();
    PowerShellCommand cmd1{sizeof(argvOneParamsB) / sizeof(char*), argvOneParamsB};
    cmd1.ExecCommand();
    std::string expectedCode =
        "usage: power-shell display [<options>] 100\n"
        "display <options are as below> \n"
        "  -h  :  display help\n"
        "  -r  :  retore brightness\n"
        "  -s  :  set brightness\n"
        "  -o  :  override brightness\n"
        "  -b  :  timing maximum brightness\n"
        "  -c  :  cancel the timing maximum brightness\n"
        "  -d  :  discount brightness\n";
    EXPECT_EQ(cmd1.resultReceiver_, expectedCode);
    EXPECT_EQ(g_lastCalled, "");

    PowerShellCommand cmd2{sizeof(argvOneParamsB) / sizeof(char*), argvOneParamsB};
    optind = 0;
    cmd2.ExecCommand();
    expectedCode = "Cancel boost brightness failed\n";
    EXPECT_EQ(cmd2.resultReceiver_, expectedCode);
    EXPECT_EQ(g_lastCalled, "CancelBoostBrightness");

    POWER_HILOGI(LABEL_TEST, "PowerShellTest013 end");
}

/**
 * @tc.name: PowerShellTest014
 * @tc.desc: Test PowerShellCommand RunAsDisplayCommand RunAsDisplayCommandSetValue
 * @tc.type: FUNC
 */
HWTEST_F(PowerShellTest, PowerShellTest014, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerShellTest014 start");
    char name[] = "power_shell_test";
    char command[] = "display";
    char paramOptionS[] = "-s";
    char paramValue[] = "100";
    char paramValue2[] = "2333";
    char* argvOneParamsS[] = {name, command, paramOptionS};
    char* argvOneParamsSV[] = {name, command, paramOptionS, paramValue};
    char* argvOneParamsSV2[] = {name, command, paramOptionS, paramValue2};

    g_lastCalled.clear();
    PowerShellCommand cmd1{sizeof(argvOneParamsS) / sizeof(char*), argvOneParamsS};
    cmd1.ExecCommand();
    std::string expectedCode =
        "usage: power-shell display [<options>] 100\n"
        "display <options are as below> \n"
        "  -h  :  display help\n"
        "  -r  :  retore brightness\n"
        "  -s  :  set brightness\n"
        "  -o  :  override brightness\n"
        "  -b  :  timing maximum brightness\n"
        "  -c  :  cancel the timing maximum brightness\n"
        "  -d  :  discount brightness\n";
    EXPECT_EQ(cmd1.resultReceiver_, expectedCode);

    POWER_HILOGI(LABEL_TEST, "PowerShellTest014 end");
}

/**
 * @tc.name: PowerShellTest015
 * @tc.desc: Test PowerShellCommand RunAsDisplayCommand CancelBoostBrightness
 * @tc.type: FUNC
 */
HWTEST_F(PowerShellTest, PowerShellTest015, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerShellTest015 start");
    char name[] = "power_shell_test";
    char command[] = "display";
    char paramOptionS[] = "-d";
    char paramValue[] = "0.5";
    char paramValue2[] = "0.2";
    char* argvOneParamsS[] = {name, command, paramOptionS};
    char* argvOneParamsSV[] = {name, command, paramOptionS, paramValue};
    char* argvOneParamsSV2[] = {name, command, paramOptionS, paramValue2};

    g_lastCalled.clear();
    PowerShellCommand cmd1{sizeof(argvOneParamsS) / sizeof(char*), argvOneParamsS};
    cmd1.ExecCommand();
    std::string expectedCode =
        "usage: power-shell display [<options>] 100\n"
        "display <options are as below> \n"
        "  -h  :  display help\n"
        "  -r  :  retore brightness\n"
        "  -s  :  set brightness\n"
        "  -o  :  override brightness\n"
        "  -b  :  timing maximum brightness\n"
        "  -c  :  cancel the timing maximum brightness\n"
        "  -d  :  discount brightness\n";
    EXPECT_EQ(cmd1.resultReceiver_, expectedCode);

    POWER_HILOGI(LABEL_TEST, "PowerShellTest015 end");
}

/**
 * @tc.name: PowerShellTest016
 * @tc.desc: Test PowerShellCommand RunAsDisplayCommand
 * @tc.type: FUNC
 */
HWTEST_F(PowerShellTest, PowerShellTest016, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerShellTest016 start");
    char name[] = "power_shell_test";
    char command[] = "display";
    char paramOptionH[] = "-h";
    char paramOptionR[] = "-r";
    char paramOptionC[] = "-c";
    char paramOptionS[] = "-s";
    char paramValue[] = "100";
    char paramOptionO[] = "-o";
    char paramBrightness[] = "923";
    char paramOptionB[] = "-b";
    char paramValue1[] = "100";
    char paramOptionD[] = "-d";
    char paramValue2[] = "0.5";

    char* argvOneParams[] = {name, command, paramOptionH, paramOptionR, paramOptionC, paramOptionS, paramValue,
        paramOptionO, paramBrightness, paramOptionB, paramValue1, paramOptionS, paramValue2};

    g_lastCalled.clear();
    PowerShellCommand cmd1{sizeof(argvOneParams) / sizeof(char*), argvOneParams};
    cmd1.ExecCommand();
    EXPECT_NE(cmd1.resultReceiver_, "");

    POWER_HILOGI(LABEL_TEST, "PowerShellTest016 end");
}
#endif

/**
 * @tc.name: PowerShellTest017
 * @tc.desc: Test PowerShellCommand RunAsTimeOutCommand
 * @tc.type: FUNC
 */
HWTEST_F(PowerShellTest, PowerShellTest017, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerShellTest017 start");
    char name[] = "power_shell_test";
    char command[] = "timeout";
    char paramOptionH[] = "-h";
    char paramOptionR[] = "-r";
    char paramOptionO[] = "-o:100";
    char paramOptionO2[] = "-o:923";

    char* argv[] = {name, command};
    char* argvOneParamsH[] = {name, command, paramOptionH};
    char* argvOneParamsR[] = {name, command, paramOptionR};
    char* argvOneParamsO[] = {name, command, paramOptionO};
    char* argvOneParamsO2[] = {name, command, paramOptionO2};

    g_lastCalled.clear();
    PowerShellCommand cmd1{sizeof(argv) / sizeof(char*), argv};
    cmd1.ExecCommand();
    std::string expectedCode =
        "Error! please input your screen off time.\n"
        "usage: power-shell timeout [<options>] 1000\n"
        "timeout <options are as below> \n"
        "  -o  :  override screen off time\n"
        "  -r  :  restore screen off time\n";
    EXPECT_EQ(cmd1.resultReceiver_, expectedCode);

    POWER_HILOGI(LABEL_TEST, "PowerShellTest017 end");
}
#endif
} // namespace
} // namespace PowerMgr
} // namespace OHOS