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

#include <gtest/gtest.h>
#include <sstream>
#include <string>
#include <cstdio>

#ifdef POWER_GTEST
#define private    public
#define protected public
#endif
#include <power_errors.h>
#include <power_mgr_client.h>
#include "power_cli_command.h"
#include "power_cli_enum_parser.h"

namespace OHOS {
namespace PowerMgr {

using namespace testing::ext;

static constexpr int ARGC_WITH_COMMAND = 2;
static constexpr int ARGC_WITH_ONE_ARG = 3;
static constexpr int ARGC_WITH_KEY_VALUE = 4;
static constexpr int ARGC_WITH_EXTRA = 5;
static constexpr int DETAIL_OVERFLOW_SIZE = 200;

static std::string g_lastCalled;

static PowerErrors g_suspendResult = PowerErrors::ERR_OK;
static PowerErrors g_wakeupResult = PowerErrors::ERR_OK;
static PowerErrors g_setModeResult = PowerErrors::ERR_OK;
static PowerErrors g_overrideResult = PowerErrors::ERR_OK;
static PowerErrors g_restoreResult = PowerErrors::ERR_OK;

static std::string CaptureStdout(std::function<void()> func)
{
    std::stringstream captured;
    auto* oldBuf = std::cout.rdbuf();
    std::cout.rdbuf(captured.rdbuf());
    func();
    std::cout.rdbuf(oldBuf);
    std::string result = captured.str();
    if (!result.empty() && result.back() == '\n') {
        result.pop_back();
    }
    return result;
}


class PowerCliTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp()
    {
        g_lastCalled.clear();
        g_suspendResult = PowerErrors::ERR_OK;
        g_wakeupResult = PowerErrors::ERR_OK;
        g_setModeResult = PowerErrors::ERR_OK;
        g_overrideResult = PowerErrors::ERR_OK;
        g_restoreResult = PowerErrors::ERR_OK;
    }
    void TearDown() {}
};

PowerErrors PowerMgrClient::SuspendDevice(
    SuspendDeviceType reason, bool suspendImmed, const std::string& apiVersion)
{
    g_lastCalled = "SuspendDevice";
    return g_suspendResult;
}

PowerErrors PowerMgrClient::WakeupDevice(
    WakeupDeviceType reason, const std::string& detail, const std::string& apiVersion)
{
    g_lastCalled = "WakeupDevice";
    return g_wakeupResult;
}

PowerErrors PowerMgrClient::SetDeviceMode(const PowerMode mode)
{
    g_lastCalled = "SetDeviceMode";
    return g_setModeResult;
}

PowerErrors PowerMgrClient::OverrideScreenOffTime(int64_t timeout, const std::string& apiVersion)
{
    g_lastCalled = "OverrideScreenOffTime";
    return g_overrideResult;
}

PowerErrors PowerMgrClient::RestoreScreenOffTime(const std::string& apiVersion)
{
    g_lastCalled = "RestoreScreenOffTime";
    return g_restoreResult;
}

// ============================================================================
// EnumParser Tests
// ============================================================================

namespace {

/**
 * @tc.name: PowerCliEnumParserTest001
 * @tc.desc: Test ParsePowerMode with valid values (normal and powerSave)
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliEnumParserTest001, TestSize.Level0)
{
    auto result = PowerCliEnumParser::ParsePowerMode("normal");
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.value, static_cast<int32_t>(PowerMode::NORMAL_MODE));

    result = PowerCliEnumParser::ParsePowerMode("powerSave");
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.value, static_cast<int32_t>(PowerMode::POWER_SAVE_MODE));
}

/**
 * @tc.name: PowerCliEnumParserTest002
 * @tc.desc: Test ParsePowerMode with invalid values
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliEnumParserTest002, TestSize.Level0)
{
    auto result = PowerCliEnumParser::ParsePowerMode("invalid-mode");
    EXPECT_FALSE(result.success);
    EXPECT_NE(result.error.find("invalid-mode"), std::string::npos);

    result = PowerCliEnumParser::ParsePowerMode("NORMAL");
    EXPECT_FALSE(result.success);

    result = PowerCliEnumParser::ParsePowerMode("power-save");
    EXPECT_FALSE(result.success);

    result = PowerCliEnumParser::ParsePowerMode("extreme");
    EXPECT_FALSE(result.success);

    result = PowerCliEnumParser::ParsePowerMode("custom");
    EXPECT_FALSE(result.success);
    EXPECT_NE(result.error.find("custom"), std::string::npos);

    result = PowerCliEnumParser::ParsePowerMode("");
    EXPECT_FALSE(result.success);
}

/**
 * @tc.name: PowerCliEnumParserTest003
 * @tc.desc: Test GetValidPowerModes helper function
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliEnumParserTest003, TestSize.Level0)
{
    auto powerModes = PowerCliEnumParser::GetValidPowerModes();
    EXPECT_NE(powerModes.find("normal"), std::string::npos);
    EXPECT_NE(powerModes.find("powerSave"), std::string::npos);
}

// ============================================================================
// Command Dispatch Tests
// ============================================================================

/**
 * @tc.name: PowerCliCommandTest001
 * @tc.desc: Test --help flag (output to stdout)
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliCommandTest001, TestSize.Level0)
{
    char name[] = "power_cli_test";
    char helpFlag[] = "--help";
    char* argv[] = {name, helpFlag};

    std::string stdoutOutput = CaptureStdout([&]() {
        PowerCliCommand cmd(ARGC_WITH_COMMAND, argv);
        cmd.Execute();
    });
    EXPECT_NE(stdoutOutput.find("suspend"), std::string::npos);
    EXPECT_NE(stdoutOutput.find("wakeup"), std::string::npos);
    EXPECT_NE(stdoutOutput.find("set-power-mode"), std::string::npos);
    EXPECT_NE(stdoutOutput.find("override-screen-off-time"), std::string::npos);
    EXPECT_NE(stdoutOutput.find("restore-screen-off-time"), std::string::npos);
}

/**
 * @tc.name: PowerCliCommandTest002
 * @tc.desc: Test no arguments (shows help to stdout)
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliCommandTest002, TestSize.Level0)
{
    char name[] = "power_cli_test";
    char* argv[] = {name};

    std::string stdoutOutput = CaptureStdout([&]() {
        PowerCliCommand cmd(1, argv);
        cmd.Execute();
    });
    EXPECT_NE(stdoutOutput.find("suspend"), std::string::npos);
}

/**
 * @tc.name: PowerCliCommandTest003
 * @tc.desc: Test unknown command outputs JSON error to stdout
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliCommandTest003, TestSize.Level0)
{
    char name[] = "power_cli_test";
    char unknownCmd[] = "unknown-command";
    char* argv[] = {name, unknownCmd};

    std::string output = CaptureStdout([&]() {
        PowerCliCommand cmd(ARGC_WITH_COMMAND, argv);
        cmd.Execute();
    });
    EXPECT_NE(output.find("\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(output.find("Unknown command"), std::string::npos);
}

/**
 * @tc.name: PowerCliCommandTest004
 * @tc.desc: Test that 'help' is NOT a valid subcommand (output JSON error)
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliCommandTest004, TestSize.Level0)
{
    char name[] = "power_cli_test";
    char helpCmd[] = "help";
    char* argv[] = {name, helpCmd};

    std::string output = CaptureStdout([&]() {
        PowerCliCommand cmd(ARGC_WITH_COMMAND, argv);
        cmd.Execute();
    });
    EXPECT_NE(output.find("\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(output.find("Unknown command"), std::string::npos);
}

/**
 * @tc.name: PowerCliCommandTest005
 * @tc.desc: Test subcommand --help outputs to stdout
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliCommandTest005, TestSize.Level0)
{
    char name[] = "power_cli_test";
    char command[] = "suspend";
    char helpFlag[] = "--help";
    char* argv[] = {name, command, helpFlag};

    std::string stdoutOutput = CaptureStdout([&]() {
        PowerCliCommand cmd(ARGC_WITH_ONE_ARG, argv);
        cmd.Execute();
    });
    EXPECT_NE(stdoutOutput.find("suspend"), std::string::npos);
    EXPECT_NE(stdoutOutput.find("--immediately"), std::string::npos);
}

/**
 * @tc.name: PowerCliCommandTest006
 * @tc.desc: Test wakeup --help outputs to stdout
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliCommandTest006, TestSize.Level0)
{
    char name[] = "power_cli_test";
    char command[] = "wakeup";
    char helpFlag[] = "--help";
    char* argv[] = {name, command, helpFlag};

    std::string stdoutOutput = CaptureStdout([&]() {
        PowerCliCommand cmd(ARGC_WITH_ONE_ARG, argv);
        cmd.Execute();
    });
    EXPECT_NE(stdoutOutput.find("wakeup"), std::string::npos);
    EXPECT_NE(stdoutOutput.find("--detail"), std::string::npos);
}

/**
 * @tc.name: PowerCliCommandTest007
 * @tc.desc: Test set-power-mode --help outputs to stdout
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliCommandTest007, TestSize.Level0)
{
    char name[] = "power_cli_test";
    char command[] = "set-power-mode";
    char helpFlag[] = "--help";
    char* argv[] = {name, command, helpFlag};

    std::string stdoutOutput = CaptureStdout([&]() {
        PowerCliCommand cmd(ARGC_WITH_ONE_ARG, argv);
        cmd.Execute();
    });
    EXPECT_NE(stdoutOutput.find("set-power-mode"), std::string::npos);
    EXPECT_NE(stdoutOutput.find("--mode"), std::string::npos);
}

/**
 * @tc.name: PowerCliCommandTest008
 * @tc.desc: Test override-screen-off-time --help outputs to stdout
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliCommandTest008, TestSize.Level0)
{
    char name[] = "power_cli_test";
    char command[] = "override-screen-off-time";
    char helpFlag[] = "--help";
    char* argv[] = {name, command, helpFlag};

    std::string stdoutOutput = CaptureStdout([&]() {
        PowerCliCommand cmd(ARGC_WITH_ONE_ARG, argv);
        cmd.Execute();
    });
    EXPECT_NE(stdoutOutput.find("override-screen-off-time"), std::string::npos);
    EXPECT_NE(stdoutOutput.find("--time"), std::string::npos);
}

/**
 * @tc.name: PowerCliCommandTest009
 * @tc.desc: Test restore-screen-off-time --help outputs to stdout
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliCommandTest009, TestSize.Level0)
{
    char name[] = "power_cli_test";
    char command[] = "restore-screen-off-time";
    char helpFlag[] = "--help";
    char* argv[] = {name, command, helpFlag};

    std::string stdoutOutput = CaptureStdout([&]() {
        PowerCliCommand cmd(ARGC_WITH_ONE_ARG, argv);
        cmd.Execute();
    });
    EXPECT_NE(stdoutOutput.find("restore-screen-off-time"), std::string::npos);
}

/**
 * @tc.name: PowerCliCommandTest010
 * @tc.desc: Test --help with extra argument returns error
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliCommandTest010, TestSize.Level0)
{
    char name[] = "power_cli_test";
    char helpFlag[] = "--help";
    char extra[] = "xxx";
    char* argv[] = {name, helpFlag, extra};

    std::string output = CaptureStdout([&]() {
        PowerCliCommand cmd(ARGC_WITH_ONE_ARG, argv);
        cmd.Execute();
    });
    EXPECT_NE(output.find("\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(output.find("Unexpected argument after --help"), std::string::npos);
    EXPECT_NE(output.find("xxx"), std::string::npos);
}

/**
 * @tc.name: PowerCliCommandTest011
 * @tc.desc: Test subcommand --help with extra argument returns error
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliCommandTest011, TestSize.Level0)
{
    char name[] = "power_cli_test";
    char command[] = "suspend";
    char helpFlag[] = "--help";
    char extra[] = "xxx";
    char* argv[] = {name, command, helpFlag, extra};

    std::string output = CaptureStdout([&]() {
        PowerCliCommand cmd(ARGC_WITH_KEY_VALUE, argv);
        cmd.Execute();
    });
    EXPECT_NE(output.find("\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(output.find("--help must be the only argument"), std::string::npos);
}

/**
 * @tc.name: PowerCliCommandTest012
 * @tc.desc: Test subcommand with --help mixed with other flags returns error
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliCommandTest012, TestSize.Level0)
{
    char name[] = "power_cli_test";
    char command[] = "suspend";
    char immFlag[] = "--immediately";
    char helpFlag[] = "--help";
    char* argv[] = {name, command, immFlag, helpFlag};

    std::string output = CaptureStdout([&]() {
        PowerCliCommand cmd(ARGC_WITH_KEY_VALUE, argv);
        cmd.Execute();
    });
    EXPECT_NE(output.find("\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(output.find("--help must be the only argument"), std::string::npos);
}

/**
 * @tc.name: PowerCliCommandTest013
 * @tc.desc: Test unknown command with --help returns Unknown command error
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliCommandTest013, TestSize.Level0)
{
    char name[] = "power_cli_test";
    char unknownCmd[] = "xxx";
    char helpFlag[] = "--help";
    char* argv[] = {name, unknownCmd, helpFlag};

    std::string output = CaptureStdout([&]() {
        PowerCliCommand cmd(ARGC_WITH_ONE_ARG, argv);
        cmd.Execute();
    });
    EXPECT_NE(output.find("\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(output.find("Unknown command"), std::string::npos);
    EXPECT_NE(output.find("xxx"), std::string::npos);
}

/**
 * @tc.name: PowerCliCommandTest014
 * @tc.desc: Test unknown command with --help mixed with other args returns error
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliCommandTest014, TestSize.Level0)
{
    char name[] = "power_cli_test";
    char unknownCmd[] = "xxx";
    char helpFlag[] = "--help";
    char extra[] = "yyy";
    char* argv[] = {name, unknownCmd, helpFlag, extra};

    std::string output = CaptureStdout([&]() {
        PowerCliCommand cmd(ARGC_WITH_KEY_VALUE, argv);
        cmd.Execute();
    });
    EXPECT_NE(output.find("\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(output.find("--help must be the only argument"), std::string::npos);
}

// ============================================================================
// Suspend Command Tests
// ============================================================================

/**
 * @tc.name: PowerCliSuspendTest001
 * @tc.desc: Test suspend with default parameters (no --immediately)
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliSuspendTest001, TestSize.Level0)
{
    char name[] = "power_cli_test";
    char command[] = "suspend";
    char* argv[] = {name, command};

    std::string output = CaptureStdout([&]() {
        PowerCliCommand cmd(ARGC_WITH_COMMAND, argv);
        cmd.Execute();
    });
    EXPECT_EQ(g_lastCalled, "SuspendDevice");
    EXPECT_NE(output.find("\"status\":\"success\""), std::string::npos);
    EXPECT_NE(output.find("\"command\":\"suspend\""), std::string::npos);
    EXPECT_NE(output.find("\"reason\":\"application\""), std::string::npos);
}

/**
 * @tc.name: PowerCliSuspendTest002
 * @tc.desc: Test suspend with --immediately flag
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliSuspendTest002, TestSize.Level0)
{
    char name[] = "power_cli_test";
    char command[] = "suspend";
    char immFlag[] = "--immediately";
    char* argv[] = {name, command, immFlag};

    std::string output = CaptureStdout([&]() {
        PowerCliCommand cmd(ARGC_WITH_ONE_ARG, argv);
        cmd.Execute();
    });
    EXPECT_EQ(g_lastCalled, "SuspendDevice");
    EXPECT_NE(output.find("\"status\":\"success\""), std::string::npos);
    EXPECT_NE(output.find("\"immediately\":true"), std::string::npos);
}

/**
 * @tc.name: PowerCliSuspendTest003
 * @tc.desc: Test suspend with service error (permission denied)
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliSuspendTest003, TestSize.Level0)
{
    g_suspendResult = PowerErrors::ERR_PERMISSION_DENIED;

    char name[] = "power_cli_test";
    char command[] = "suspend";
    char* argv[] = {name, command};

    std::string output = CaptureStdout([&]() {
        PowerCliCommand cmd(ARGC_WITH_COMMAND, argv);
        cmd.Execute();
    });
    EXPECT_EQ(g_lastCalled, "SuspendDevice");
    EXPECT_NE(output.find("\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(output.find("Permission denied"), std::string::npos);
}

/**
 * @tc.name: PowerCliSuspendTest004
 * @tc.desc: Test suspend with connection failure
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliSuspendTest004, TestSize.Level0)
{
    g_suspendResult = PowerErrors::ERR_CONNECTION_FAIL;

    char name[] = "power_cli_test";
    char command[] = "suspend";
    char* argv[] = {name, command};

    std::string output = CaptureStdout([&]() {
        PowerCliCommand cmd(ARGC_WITH_COMMAND, argv);
        cmd.Execute();
    });
    EXPECT_NE(output.find("\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(output.find("Connection"), std::string::npos);
    EXPECT_NE(output.find("ERR_CONNECTION_FAIL"), std::string::npos);
}

/**
 * @tc.name: PowerCliSuspendTest005
 * @tc.desc: Test suspend with unknown parameter
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliSuspendTest005, TestSize.Level0)
{
    char name[] = "power_cli_test";
    char command[] = "suspend";
    char unknownParam[] = "--unknown";
    char* argv[] = {name, command, unknownParam};

    std::string output = CaptureStdout([&]() {
        PowerCliCommand cmd(ARGC_WITH_ONE_ARG, argv);
        cmd.Execute();
    });
    EXPECT_NE(output.find("\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(output.find("Unknown parameter"), std::string::npos);
    EXPECT_NE(output.find("--unknown"), std::string::npos);
}

/**
 * @tc.name: PowerCliSuspendTest006
 * @tc.desc: Test suspend with positional argument
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliSuspendTest006, TestSize.Level0)
{
    char name[] = "power_cli_test";
    char command[] = "suspend";
    char positional[] = "extra";
    char* argv[] = {name, command, positional};

    std::string output = CaptureStdout([&]() {
        PowerCliCommand cmd(ARGC_WITH_ONE_ARG, argv);
        cmd.Execute();
    });
    EXPECT_NE(output.find("\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(output.find("Unknown parameter"), std::string::npos);
}

/**
 * @tc.name: PowerCliSuspendTest007
 * @tc.desc: Test suspend with ERR_SYSTEM_API_DENIED
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliSuspendTest007, TestSize.Level0)
{
    g_suspendResult = PowerErrors::ERR_SYSTEM_API_DENIED;

    char name[] = "power_cli_test";
    char command[] = "suspend";
    char* argv[] = {name, command};

    std::string output = CaptureStdout([&]() {
        PowerCliCommand cmd(ARGC_WITH_COMMAND, argv);
        cmd.Execute();
    });
    EXPECT_NE(output.find("\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(output.find("ERR_SYSTEM_API_DENIED"), std::string::npos);
    EXPECT_NE(output.find("System API denied"), std::string::npos);
}

/**
 * @tc.name: PowerCliSuspendTest008
 * @tc.desc: Test suspend with ERR_FAILURE
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliSuspendTest008, TestSize.Level0)
{
    g_suspendResult = PowerErrors::ERR_FAILURE;

    char name[] = "power_cli_test";
    char command[] = "suspend";
    char* argv[] = {name, command};

    std::string output = CaptureStdout([&]() {
        PowerCliCommand cmd(ARGC_WITH_COMMAND, argv);
        cmd.Execute();
    });
    EXPECT_NE(output.find("\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(output.find("ERR_FAILURE"), std::string::npos);
    EXPECT_NE(output.find("General failure"), std::string::npos);
}

/**
 * @tc.name: PowerCliSuspendTest009
 * @tc.desc: Test suspend with ERR_FREQUENT_FUNCTION_CALL
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliSuspendTest009, TestSize.Level0)
{
    g_suspendResult = PowerErrors::ERR_FREQUENT_FUNCTION_CALL;

    char name[] = "power_cli_test";
    char command[] = "suspend";
    char* argv[] = {name, command};

    std::string output = CaptureStdout([&]() {
        PowerCliCommand cmd(ARGC_WITH_COMMAND, argv);
        cmd.Execute();
    });
    EXPECT_NE(output.find("\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(output.find("ERR_FREQUENT_FUNCTION_CALL"), std::string::npos);
    EXPECT_NE(output.find("Too frequent"), std::string::npos);
}

/**
 * @tc.name: PowerCliSuspendTest010
 * @tc.desc: Test suspend with ERR_WRITE_OPERATION_FAILED
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliSuspendTest010, TestSize.Level0)
{
    g_suspendResult = PowerErrors::ERR_WRITE_OPERATION_FAILED;

    char name[] = "power_cli_test";
    char command[] = "suspend";
    char* argv[] = {name, command};

    std::string output = CaptureStdout([&]() {
        PowerCliCommand cmd(ARGC_WITH_COMMAND, argv);
        cmd.Execute();
    });
    EXPECT_NE(output.find("\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(output.find("ERR_WRITE_OPERATION_FAILED"), std::string::npos);
    EXPECT_NE(output.find("Write operation failed"), std::string::npos);
}

/**
 * @tc.name: PowerCliSuspendTest011
 * @tc.desc: Test suspend with ERR_USER_PARAM_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliSuspendTest011, TestSize.Level0)
{
    g_suspendResult = PowerErrors::ERR_USER_PARAM_INVALID;

    char name[] = "power_cli_test";
    char command[] = "suspend";
    char* argv[] = {name, command};

    std::string output = CaptureStdout([&]() {
        PowerCliCommand cmd(ARGC_WITH_COMMAND, argv);
        cmd.Execute();
    });
    EXPECT_NE(output.find("\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(output.find("ERR_USER_PARAM_INVALID"), std::string::npos);
    EXPECT_NE(output.find("User parameter value invalid"), std::string::npos);
}

/**
 * @tc.name: PowerCliSuspendTest012
 * @tc.desc: Test suspend with duplicate --immediately flag rejected
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliSuspendTest012, TestSize.Level0)
{
    char name[] = "power_cli_test";
    char command[] = "suspend";
    char flag1[] = "--immediately";
    char flag2[] = "--immediately";
    char* argv[] = {name, command, flag1, flag2};

    std::string output = CaptureStdout([&]() {
        PowerCliCommand cmd(ARGC_WITH_KEY_VALUE, argv);
        cmd.Execute();
    });
    EXPECT_NE(output.find("\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(output.find("Duplicate parameter"), std::string::npos);
    EXPECT_NE(output.find("--immediately"), std::string::npos);
}

// ============================================================================
// Wakeup Command Tests
// ============================================================================

/**
 * @tc.name: PowerCliWakeupTest001
 * @tc.desc: Test wakeup with default parameters
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliWakeupTest001, TestSize.Level0)
{
    char name[] = "power_cli_test";
    char command[] = "wakeup";
    char* argv[] = {name, command};

    std::string output = CaptureStdout([&]() {
        PowerCliCommand cmd(ARGC_WITH_COMMAND, argv);
        cmd.Execute();
    });
    EXPECT_EQ(g_lastCalled, "WakeupDevice");
    EXPECT_NE(output.find("\"status\":\"success\""), std::string::npos);
    EXPECT_NE(output.find("\"command\":\"wakeup\""), std::string::npos);
    EXPECT_NE(output.find("\"reason\":\"application\""), std::string::npos);
    EXPECT_NE(output.find("\"detail\":\"cli-call\""), std::string::npos);
}

/**
 * @tc.name: PowerCliWakeupTest002
 * @tc.desc: Test wakeup with --detail option
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliWakeupTest002, TestSize.Level0)
{
    char name[] = "power_cli_test";
    char command[] = "wakeup";
    char detailFlag[] = "--detail";
    char detailVal[] = "user pressed power key";
    char* argv[] = {name, command, detailFlag, detailVal};

    std::string output = CaptureStdout([&]() {
        PowerCliCommand cmd(ARGC_WITH_KEY_VALUE, argv);
        cmd.Execute();
    });
    EXPECT_EQ(g_lastCalled, "WakeupDevice");
    EXPECT_NE(output.find("\"status\":\"success\""), std::string::npos);
    EXPECT_NE(output.find("\"detail\":\"user pressed power key\""), std::string::npos);
}

/**
 * @tc.name: PowerCliWakeupTest003
 * @tc.desc: Test wakeup with detail exceeding max length
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliWakeupTest003, TestSize.Level0)
{
    char name[] = "power_cli_test";
    char command[] = "wakeup";
    char detailFlag[] = "--detail";
    char detailVal[DETAIL_OVERFLOW_SIZE];
    (void)memset_s(detailVal, sizeof(detailVal), 'x', DETAIL_OVERFLOW_SIZE - 1);
    detailVal[DETAIL_OVERFLOW_SIZE - 1] = '\0';
    char* argv[] = {name, command, detailFlag, detailVal};

    std::string output = CaptureStdout([&]() {
        PowerCliCommand cmd(ARGC_WITH_KEY_VALUE, argv);
        cmd.Execute();
    });
    EXPECT_NE(output.find("\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(output.find("exceeds maximum length"), std::string::npos);
}

/**
 * @tc.name: PowerCliWakeupTest004
 * @tc.desc: Test wakeup with --detail but missing value (uses default)
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliWakeupTest004, TestSize.Level0)
{
    char name[] = "power_cli_test";
    char command[] = "wakeup";
    char detailFlag[] = "--detail";
    char* argv[] = {name, command, detailFlag};

    std::string output = CaptureStdout([&]() {
        PowerCliCommand cmd(ARGC_WITH_ONE_ARG, argv);
        cmd.Execute();
    });
    EXPECT_EQ(g_lastCalled, "WakeupDevice");
    EXPECT_NE(output.find("\"status\":\"success\""), std::string::npos);
}

/**
 * @tc.name: PowerCliWakeupTest005
 * @tc.desc: Test wakeup with service error
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliWakeupTest005, TestSize.Level0)
{
    g_wakeupResult = PowerErrors::ERR_PERMISSION_DENIED;

    char name[] = "power_cli_test";
    char command[] = "wakeup";
    char* argv[] = {name, command};

    std::string output = CaptureStdout([&]() {
        PowerCliCommand cmd(ARGC_WITH_COMMAND, argv);
        cmd.Execute();
    });
    EXPECT_EQ(g_lastCalled, "WakeupDevice");
    EXPECT_NE(output.find("\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(output.find("Permission denied"), std::string::npos);
}

/**
 * @tc.name: PowerCliWakeupTest006
 * @tc.desc: Test wakeup with unknown parameter
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliWakeupTest006, TestSize.Level0)
{
    char name[] = "power_cli_test";
    char command[] = "wakeup";
    char unknownParam[] = "--unknown";
    char* argv[] = {name, command, unknownParam};

    std::string output = CaptureStdout([&]() {
        PowerCliCommand cmd(ARGC_WITH_ONE_ARG, argv);
        cmd.Execute();
    });
    EXPECT_NE(output.find("\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(output.find("Unknown parameter"), std::string::npos);
    EXPECT_NE(output.find("--unknown"), std::string::npos);
}

/**
 * @tc.name: PowerCliWakeupTest007
 * @tc.desc: Test wakeup with positional argument
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliWakeupTest007, TestSize.Level0)
{
    char name[] = "power_cli_test";
    char command[] = "wakeup";
    char positional[] = "extra";
    char* argv[] = {name, command, positional};

    std::string output = CaptureStdout([&]() {
        PowerCliCommand cmd(ARGC_WITH_ONE_ARG, argv);
        cmd.Execute();
    });
    EXPECT_NE(output.find("\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(output.find("Unknown parameter"), std::string::npos);
}

/**
 * @tc.name: PowerCliWakeupTest008
 * @tc.desc: Test wakeup with duplicate --detail flag rejected
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliWakeupTest008, TestSize.Level0)
{
    char name[] = "power_cli_test";
    char command[] = "wakeup";
    char flag1[] = "--detail";
    char val1[] = "first";
    char flag2[] = "--detail";
    char val2[] = "second";
    char* argv[] = {name, command, flag1, val1, flag2, val2};

    std::string output = CaptureStdout([&]() {
        PowerCliCommand cmd(6, argv);
        cmd.Execute();
    });
    EXPECT_NE(output.find("\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(output.find("Duplicate parameter"), std::string::npos);
    EXPECT_NE(output.find("--detail"), std::string::npos);
}

// ============================================================================
// SetPowerMode Command Tests
// ============================================================================

/**
 * @tc.name: PowerCliSetModeTest001
 * @tc.desc: Test set-power-mode --mode normal
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliSetModeTest001, TestSize.Level0)
{
    char name[] = "power_cli_test";
    char command[] = "set-power-mode";
    char modeFlag[] = "--mode";
    char modeVal[] = "normal";
    char* argv[] = {name, command, modeFlag, modeVal};

    std::string output = CaptureStdout([&]() {
        PowerCliCommand cmd(ARGC_WITH_KEY_VALUE, argv);
        cmd.Execute();
    });
    EXPECT_EQ(g_lastCalled, "SetDeviceMode");
    EXPECT_NE(output.find("\"status\":\"success\""), std::string::npos);
    EXPECT_NE(output.find("\"mode\":\"normal\""), std::string::npos);
}

/**
 * @tc.name: PowerCliSetModeTest002
 * @tc.desc: Test set-power-mode --mode powerSave
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliSetModeTest002, TestSize.Level0)
{
    char name[] = "power_cli_test";
    char command[] = "set-power-mode";
    char modeFlag[] = "--mode";
    char modeVal[] = "powerSave";
    char* argv[] = {name, command, modeFlag, modeVal};

    std::string output = CaptureStdout([&]() {
        PowerCliCommand cmd(ARGC_WITH_KEY_VALUE, argv);
        cmd.Execute();
    });
    EXPECT_EQ(g_lastCalled, "SetDeviceMode");
    EXPECT_NE(output.find("\"status\":\"success\""), std::string::npos);
    EXPECT_NE(output.find("\"mode\":\"powerSave\""), std::string::npos);
}

/**
 * @tc.name: PowerCliSetModeTest003
 * @tc.desc: Test set-power-mode with unsupported mode (power-save)
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliSetModeTest003, TestSize.Level0)
{
    char name[] = "power_cli_test";
    char command[] = "set-power-mode";
    char modeFlag[] = "--mode";
    char modeVal[] = "power-save";
    char* argv[] = {name, command, modeFlag, modeVal};

    std::string output = CaptureStdout([&]() {
        PowerCliCommand cmd(ARGC_WITH_KEY_VALUE, argv);
        cmd.Execute();
    });
    EXPECT_NE(output.find("\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(output.find("Invalid power mode"), std::string::npos);
}

/**
 * @tc.name: PowerCliSetModeTest004
 * @tc.desc: Test set-power-mode with unsupported mode (extreme)
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliSetModeTest004, TestSize.Level0)
{
    char name[] = "power_cli_test";
    char command[] = "set-power-mode";
    char modeFlag[] = "--mode";
    char modeVal[] = "extreme";
    char* argv[] = {name, command, modeFlag, modeVal};

    std::string output = CaptureStdout([&]() {
        PowerCliCommand cmd(ARGC_WITH_KEY_VALUE, argv);
        cmd.Execute();
    });
    EXPECT_NE(output.find("\"status\":\"failed\""), std::string::npos);
}

/**
 * @tc.name: PowerCliSetModeTest005
 * @tc.desc: Test set-power-mode missing --mode parameter
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliSetModeTest005, TestSize.Level0)
{
    char name[] = "power_cli_test";
    char command[] = "set-power-mode";
    char* argv[] = {name, command};

    std::string output = CaptureStdout([&]() {
        PowerCliCommand cmd(ARGC_WITH_COMMAND, argv);
        cmd.Execute();
    });
    EXPECT_NE(output.find("\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(output.find("Missing required parameter: --mode"), std::string::npos);
}

/**
 * @tc.name: PowerCliSetModeTest006
 * @tc.desc: Test set-power-mode with invalid mode string
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliSetModeTest006, TestSize.Level0)
{
    char name[] = "power_cli_test";
    char command[] = "set-power-mode";
    char modeFlag[] = "--mode";
    char modeVal[] = "invalid-mode";
    char* argv[] = {name, command, modeFlag, modeVal};

    std::string output = CaptureStdout([&]() {
        PowerCliCommand cmd(ARGC_WITH_KEY_VALUE, argv);
        cmd.Execute();
    });
    EXPECT_NE(output.find("\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(output.find("invalid-mode"), std::string::npos);
}

/**
 * @tc.name: PowerCliSetModeTest007
 * @tc.desc: Test set-power-mode with service error (mode transition failed)
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliSetModeTest007, TestSize.Level0)
{
    g_setModeResult = PowerErrors::ERR_POWER_MODE_TRANSIT_FAILED;

    char name[] = "power_cli_test";
    char command[] = "set-power-mode";
    char modeFlag[] = "--mode";
    char modeVal[] = "powerSave";
    char* argv[] = {name, command, modeFlag, modeVal};

    std::string output = CaptureStdout([&]() {
        PowerCliCommand cmd(ARGC_WITH_KEY_VALUE, argv);
        cmd.Execute();
    });
    EXPECT_EQ(g_lastCalled, "SetDeviceMode");
    EXPECT_NE(output.find("\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(output.find("mode transition failed"), std::string::npos);
    EXPECT_NE(output.find("ERR_POWER_MODE_TRANSIT_FAILED"), std::string::npos);
}

/**
 * @tc.name: PowerCliSetModeTest008
 * @tc.desc: Test set-power-mode with unknown parameter
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliSetModeTest008, TestSize.Level0)
{
    char name[] = "power_cli_test";
    char command[] = "set-power-mode";
    char modeFlag[] = "--mode";
    char modeVal[] = "normal";
    char unknownParam[] = "--unknown";
    char* argv[] = {name, command, modeFlag, modeVal, unknownParam};

    std::string output = CaptureStdout([&]() {
        PowerCliCommand cmd(ARGC_WITH_EXTRA, argv);
        cmd.Execute();
    });
    EXPECT_NE(output.find("\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(output.find("Unknown parameter"), std::string::npos);
    EXPECT_NE(output.find("--unknown"), std::string::npos);
}

/**
 * @tc.name: PowerCliSetModeTest009
 * @tc.desc: Test set-power-mode with positional argument
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliSetModeTest009, TestSize.Level0)
{
    char name[] = "power_cli_test";
    char command[] = "set-power-mode";
    char positional[] = "extra";
    char* argv[] = {name, command, positional};

    std::string output = CaptureStdout([&]() {
        PowerCliCommand cmd(ARGC_WITH_ONE_ARG, argv);
        cmd.Execute();
    });
    EXPECT_NE(output.find("\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(output.find("Unknown parameter"), std::string::npos);
}

/**
 * @tc.name: PowerCliSetModeTest010
 * @tc.desc: Test set-power-mode --mode as last arg (no value following)
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliSetModeTest010, TestSize.Level0)
{
    char name[] = "power_cli_test";
    char command[] = "set-power-mode";
    char modeFlag[] = "--mode";
    char* argv[] = {name, command, modeFlag};

    std::string output = CaptureStdout([&]() {
        PowerCliCommand cmd(ARGC_WITH_ONE_ARG, argv);
        cmd.Execute();
    });
    EXPECT_NE(output.find("\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(output.find("Missing required parameter: --mode"), std::string::npos);
}

/**
 * @tc.name: PowerCliSetModeTest011
 * @tc.desc: Test set-power-mode with duplicate --mode flag rejected
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliSetModeTest011, TestSize.Level0)
{
    char name[] = "power_cli_test";
    char command[] = "set-power-mode";
    char flag1[] = "--mode";
    char val1[] = "normal";
    char flag2[] = "--mode";
    char val2[] = "powerSave";
    char* argv[] = {name, command, flag1, val1, flag2, val2};

    std::string output = CaptureStdout([&]() {
        PowerCliCommand cmd(6, argv);
        cmd.Execute();
    });
    EXPECT_NE(output.find("\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(output.find("Duplicate parameter"), std::string::npos);
    EXPECT_NE(output.find("--mode"), std::string::npos);
}

// ============================================================================
// OverrideScreenOffTime Command Tests
// ============================================================================

/**
 * @tc.name: PowerCliOverrideTimeoutTest001
 * @tc.desc: Test override-screen-off-time --time 30000
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliOverrideTimeoutTest001, TestSize.Level0)
{
    char name[] = "power_cli_test";
    char command[] = "override-screen-off-time";
    char timeFlag[] = "--time";
    char timeoutVal[] = "30000";
    char* argv[] = {name, command, timeFlag, timeoutVal};

    std::string output = CaptureStdout([&]() {
        PowerCliCommand cmd(ARGC_WITH_KEY_VALUE, argv);
        cmd.Execute();
    });
    EXPECT_EQ(g_lastCalled, "OverrideScreenOffTime");
    EXPECT_NE(output.find("\"status\":\"success\""), std::string::npos);
    EXPECT_NE(output.find("\"timeout\":30000"), std::string::npos);
}

/**
 * @tc.name: PowerCliOverrideTimeoutTest002
 * @tc.desc: Test override-screen-off-time --time 1 (minimum valid)
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliOverrideTimeoutTest002, TestSize.Level0)
{
    char name[] = "power_cli_test";
    char command[] = "override-screen-off-time";
    char timeFlag[] = "--time";
    char timeoutVal[] = "1";
    char* argv[] = {name, command, timeFlag, timeoutVal};

    std::string output = CaptureStdout([&]() {
        PowerCliCommand cmd(ARGC_WITH_KEY_VALUE, argv);
        cmd.Execute();
    });
    EXPECT_EQ(g_lastCalled, "OverrideScreenOffTime");
    EXPECT_NE(output.find("\"status\":\"success\""), std::string::npos);
    EXPECT_NE(output.find("\"timeout\":1"), std::string::npos);
}

/**
 * @tc.name: PowerCliOverrideTimeoutTest003
 * @tc.desc: Test override-screen-off-time with large timeout
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliOverrideTimeoutTest003, TestSize.Level0)
{
    char name[] = "power_cli_test";
    char command[] = "override-screen-off-time";
    char timeFlag[] = "--time";
    char timeoutVal[] = "9223372036854775807";
    char* argv[] = {name, command, timeFlag, timeoutVal};

    std::string output = CaptureStdout([&]() {
        PowerCliCommand cmd(ARGC_WITH_KEY_VALUE, argv);
        cmd.Execute();
    });
    EXPECT_EQ(g_lastCalled, "OverrideScreenOffTime");
    EXPECT_NE(output.find("\"status\":\"success\""), std::string::npos);
}

/**
 * @tc.name: PowerCliOverrideTimeoutTest004
 * @tc.desc: Test override-screen-off-time missing --time parameter
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliOverrideTimeoutTest004, TestSize.Level0)
{
    char name[] = "power_cli_test";
    char command[] = "override-screen-off-time";
    char* argv[] = {name, command};

    std::string output = CaptureStdout([&]() {
        PowerCliCommand cmd(ARGC_WITH_COMMAND, argv);
        cmd.Execute();
    });
    EXPECT_NE(output.find("\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(output.find("Missing required parameter: --time"), std::string::npos);
}

/**
 * @tc.name: PowerCliOverrideTimeoutTest005
 * @tc.desc: Test override-screen-off-time --time 0 (zero timeout)
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliOverrideTimeoutTest005, TestSize.Level0)
{
    char name[] = "power_cli_test";
    char command[] = "override-screen-off-time";
    char timeFlag[] = "--time";
    char timeoutVal[] = "0";
    char* argv[] = {name, command, timeFlag, timeoutVal};

    std::string output = CaptureStdout([&]() {
        PowerCliCommand cmd(ARGC_WITH_KEY_VALUE, argv);
        cmd.Execute();
    });
    EXPECT_NE(output.find("\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(output.find("Invalid timeout"), std::string::npos);
}

/**
 * @tc.name: PowerCliOverrideTimeoutTest006
 * @tc.desc: Test override-screen-off-time --time -1000 (negative)
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliOverrideTimeoutTest006, TestSize.Level0)
{
    char name[] = "power_cli_test";
    char command[] = "override-screen-off-time";
    char timeFlag[] = "--time";
    char timeoutVal[] = "-1000";
    char* argv[] = {name, command, timeFlag, timeoutVal};

    std::string output = CaptureStdout([&]() {
        PowerCliCommand cmd(ARGC_WITH_KEY_VALUE, argv);
        cmd.Execute();
    });
    EXPECT_NE(output.find("\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(output.find("Invalid timeout"), std::string::npos);
}

/**
 * @tc.name: PowerCliOverrideTimeoutTest007
 * @tc.desc: Test override-screen-off-time --time abc (non-numeric)
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliOverrideTimeoutTest007, TestSize.Level0)
{
    char name[] = "power_cli_test";
    char command[] = "override-screen-off-time";
    char timeFlag[] = "--time";
    char timeoutVal[] = "abc";
    char* argv[] = {name, command, timeFlag, timeoutVal};

    std::string output = CaptureStdout([&]() {
        PowerCliCommand cmd(ARGC_WITH_KEY_VALUE, argv);
        cmd.Execute();
    });
    EXPECT_NE(output.find("\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(output.find("Invalid timeout"), std::string::npos);
}

/**
 * @tc.name: PowerCliOverrideTimeoutTest008
 * @tc.desc: Test override-screen-off-time --time 1000abc (mixed)
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliOverrideTimeoutTest008, TestSize.Level0)
{
    char name[] = "power_cli_test";
    char command[] = "override-screen-off-time";
    char timeFlag[] = "--time";
    char timeoutVal[] = "1000abc";
    char* argv[] = {name, command, timeFlag, timeoutVal};

    std::string output = CaptureStdout([&]() {
        PowerCliCommand cmd(ARGC_WITH_KEY_VALUE, argv);
        cmd.Execute();
    });
    EXPECT_NE(output.find("\"status\":\"failed\""), std::string::npos);
}

/**
 * @tc.name: PowerCliOverrideTimeoutTest009
 * @tc.desc: Test override-screen-off-time with service error
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliOverrideTimeoutTest009, TestSize.Level0)
{
    g_overrideResult = PowerErrors::ERR_PERMISSION_DENIED;

    char name[] = "power_cli_test";
    char command[] = "override-screen-off-time";
    char timeFlag[] = "--time";
    char timeoutVal[] = "30000";
    char* argv[] = {name, command, timeFlag, timeoutVal};

    std::string output = CaptureStdout([&]() {
        PowerCliCommand cmd(ARGC_WITH_KEY_VALUE, argv);
        cmd.Execute();
    });
    EXPECT_EQ(g_lastCalled, "OverrideScreenOffTime");
    EXPECT_NE(output.find("\"status\":\"failed\""), std::string::npos);
}

/**
 * @tc.name: PowerCliOverrideTimeoutTest010
 * @tc.desc: Test override-screen-off-time with unknown parameter
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliOverrideTimeoutTest010, TestSize.Level0)
{
    char name[] = "power_cli_test";
    char command[] = "override-screen-off-time";
    char timeFlag[] = "--time";
    char timeoutVal[] = "30000";
    char unknownParam[] = "--unknown";
    char* argv[] = {name, command, timeFlag, timeoutVal, unknownParam};

    std::string output = CaptureStdout([&]() {
        PowerCliCommand cmd(ARGC_WITH_EXTRA, argv);
        cmd.Execute();
    });
    EXPECT_NE(output.find("\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(output.find("Unknown parameter"), std::string::npos);
    EXPECT_NE(output.find("--unknown"), std::string::npos);
}

/**
 * @tc.name: PowerCliOverrideTimeoutTest011
 * @tc.desc: Test override-screen-off-time with positional argument
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliOverrideTimeoutTest011, TestSize.Level0)
{
    char name[] = "power_cli_test";
    char command[] = "override-screen-off-time";
    char positional[] = "extra";
    char* argv[] = {name, command, positional};

    std::string output = CaptureStdout([&]() {
        PowerCliCommand cmd(ARGC_WITH_ONE_ARG, argv);
        cmd.Execute();
    });
    EXPECT_NE(output.find("\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(output.find("Unknown parameter"), std::string::npos);
}

// ============================================================================
// RestoreScreenOffTime Command Tests
// ============================================================================

/**
 * @tc.name: PowerCliRestoreTimeoutTest001
 * @tc.desc: Test restore-screen-off-time success
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliRestoreTimeoutTest001, TestSize.Level0)
{
    char name[] = "power_cli_test";
    char command[] = "restore-screen-off-time";
    char* argv[] = {name, command};

    std::string output = CaptureStdout([&]() {
        PowerCliCommand cmd(ARGC_WITH_COMMAND, argv);
        cmd.Execute();
    });
    EXPECT_EQ(g_lastCalled, "RestoreScreenOffTime");
    EXPECT_NE(output.find("\"status\":\"success\""), std::string::npos);
    EXPECT_NE(output.find("\"command\":\"restore-screen-off-time\""), std::string::npos);
}

/**
 * @tc.name: PowerCliRestoreTimeoutTest002
 * @tc.desc: Test restore-screen-off-time with service error
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliRestoreTimeoutTest002, TestSize.Level0)
{
    g_restoreResult = PowerErrors::ERR_CONNECTION_FAIL;

    char name[] = "power_cli_test";
    char command[] = "restore-screen-off-time";
    char* argv[] = {name, command};

    std::string output = CaptureStdout([&]() {
        PowerCliCommand cmd(ARGC_WITH_COMMAND, argv);
        cmd.Execute();
    });
    EXPECT_EQ(g_lastCalled, "RestoreScreenOffTime");
    EXPECT_NE(output.find("\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(output.find("Connection"), std::string::npos);
}

/**
 * @tc.name: PowerCliRestoreTimeoutTest003
 * @tc.desc: Test restore-screen-off-time with unknown parameter
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliRestoreTimeoutTest003, TestSize.Level0)
{
    char name[] = "power_cli_test";
    char command[] = "restore-screen-off-time";
    char unknownParam[] = "--unknown";
    char* argv[] = {name, command, unknownParam};

    std::string output = CaptureStdout([&]() {
        PowerCliCommand cmd(ARGC_WITH_ONE_ARG, argv);
        cmd.Execute();
    });
    EXPECT_NE(output.find("\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(output.find("Unknown parameter"), std::string::npos);
    EXPECT_NE(output.find("--unknown"), std::string::npos);
}

/**
 * @tc.name: PowerCliRestoreTimeoutTest004
 * @tc.desc: Test restore-screen-off-time with positional argument
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliRestoreTimeoutTest004, TestSize.Level0)
{
    char name[] = "power_cli_test";
    char command[] = "restore-screen-off-time";
    char positional[] = "extra";
    char* argv[] = {name, command, positional};

    std::string output = CaptureStdout([&]() {
        PowerCliCommand cmd(ARGC_WITH_ONE_ARG, argv);
        cmd.Execute();
    });
    EXPECT_NE(output.find("\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(output.find("Unknown parameter"), std::string::npos);
}

/**
 * @tc.name: PowerCliOverrideTimeoutTest012
 * @tc.desc: Test override-screen-off-time --time as last arg (no value following)
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliOverrideTimeoutTest012, TestSize.Level0)
{
    char name[] = "power_cli_test";
    char command[] = "override-screen-off-time";
    char timeFlag[] = "--time";
    char* argv[] = {name, command, timeFlag};

    std::string output = CaptureStdout([&]() {
        PowerCliCommand cmd(ARGC_WITH_ONE_ARG, argv);
        cmd.Execute();
    });
    EXPECT_NE(output.find("\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(output.find("Missing required parameter: --time"), std::string::npos);
}

/**
 * @tc.name: PowerCliRestoreTimeoutTest005
 * @tc.desc: Test restore-screen-off-time with ERR_PERMISSION_DENIED
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliRestoreTimeoutTest005, TestSize.Level0)
{
    g_restoreResult = PowerErrors::ERR_PERMISSION_DENIED;

    char name[] = "power_cli_test";
    char command[] = "restore-screen-off-time";
    char* argv[] = {name, command};

    std::string output = CaptureStdout([&]() {
        PowerCliCommand cmd(ARGC_WITH_COMMAND, argv);
        cmd.Execute();
    });
    EXPECT_NE(output.find("\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(output.find("Permission denied"), std::string::npos);
}

/**
 * @tc.name: PowerCliOverrideTimeoutTest013
 * @tc.desc: Test override-screen-off-time with duplicate --time flag rejected
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliOverrideTimeoutTest013, TestSize.Level0)
{
    char name[] = "power_cli_test";
    char command[] = "override-screen-off-time";
    char flag1[] = "--time";
    char val1[] = "30000";
    char flag2[] = "--time";
    char val2[] = "60000";
    char* argv[] = {name, command, flag1, val1, flag2, val2};

    std::string output = CaptureStdout([&]() {
        PowerCliCommand cmd(6, argv);
        cmd.Execute();
    });
    EXPECT_NE(output.find("\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(output.find("Duplicate parameter"), std::string::npos);
    EXPECT_NE(output.find("--time"), std::string::npos);
}

// ============================================================================
// OutputFallbackError Tests
// ============================================================================

/**
 * @tc.name: PowerCliFallbackTest001
 * @tc.desc: Test OutputFallbackError outputs correct JSON format
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliFallbackTest001, TestSize.Level0)
{
    std::string output = CaptureStdout([&]() {
        PowerCliCommand::OutputFallbackError("test error message");
    });
    EXPECT_NE(output.find("\"type\":\"result\""), std::string::npos);
    EXPECT_NE(output.find("\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(output.find("\"errCode\":\"ERR_FAILURE\""), std::string::npos);
    EXPECT_NE(output.find("test error message"), std::string::npos);
    EXPECT_NE(output.find("Internal error"), std::string::npos);
}

/**
 * @tc.name: PowerCliFallbackTest002
 * @tc.desc: Test OutputFallbackError with empty message
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliFallbackTest002, TestSize.Level0)
{
    std::string output = CaptureStdout([&]() {
        PowerCliCommand::OutputFallbackError("");
    });
    EXPECT_NE(output.find("\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(output.find("\"errCode\":\"ERR_FAILURE\""), std::string::npos);
}

} // namespace
} // namespace PowerMgr
} // namespace OHOS
