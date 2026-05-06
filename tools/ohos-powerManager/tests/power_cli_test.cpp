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

static std::string CaptureStderr(std::function<void()> func)
{
    std::stringstream captured;
    auto* oldBuf = std::cerr.rdbuf();
    std::cerr.rdbuf(captured.rdbuf());
    func();
    std::cerr.rdbuf(oldBuf);
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
 * @tc.desc: Test ParsePowerMode with valid values (normal and performance)
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliEnumParserTest001, TestSize.Level0)
{
    auto result = PowerCliEnumParser::ParsePowerMode("normal");
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.value, static_cast<int32_t>(PowerMode::NORMAL_MODE));

    result = PowerCliEnumParser::ParsePowerMode("performance");
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.value, static_cast<int32_t>(PowerMode::PERFORMANCE_MODE));
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
    EXPECT_NE(powerModes.find("performance"), std::string::npos);
}

// ============================================================================
// Command Dispatch Tests
// ============================================================================

/**
 * @tc.name: PowerCliCommandTest001
 * @tc.desc: Test --help flag (output to stderr)
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliCommandTest001, TestSize.Level0)
{
    char name[] = "power_cli_test";
    char helpFlag[] = "--help";
    char* argv[] = {name, helpFlag};

    std::string stderrOutput = CaptureStderr([&]() {
        PowerCliCommand cmd(2, argv);
        cmd.Execute();
    });
    EXPECT_NE(stderrOutput.find("suspend"), std::string::npos);
    EXPECT_NE(stderrOutput.find("wakeup"), std::string::npos);
    EXPECT_NE(stderrOutput.find("set-power-mode"), std::string::npos);
    EXPECT_NE(stderrOutput.find("override-screen-off-time"), std::string::npos);
    EXPECT_NE(stderrOutput.find("restore-screen-off-time"), std::string::npos);
}

/**
 * @tc.name: PowerCliCommandTest002
 * @tc.desc: Test no arguments (shows help to stderr)
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliCommandTest002, TestSize.Level0)
{
    char name[] = "power_cli_test";
    char* argv[] = {name};

    std::string stderrOutput = CaptureStderr([&]() {
        PowerCliCommand cmd(1, argv);
        cmd.Execute();
    });
    EXPECT_NE(stderrOutput.find("suspend"), std::string::npos);
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
        PowerCliCommand cmd(2, argv);
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
        PowerCliCommand cmd(2, argv);
        cmd.Execute();
    });
    EXPECT_NE(output.find("\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(output.find("Unknown command"), std::string::npos);
}

/**
 * @tc.name: PowerCliCommandTest005
 * @tc.desc: Test subcommand --help outputs to stderr
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliCommandTest005, TestSize.Level0)
{
    char name[] = "power_cli_test";
    char command[] = "suspend";
    char helpFlag[] = "--help";
    char* argv[] = {name, command, helpFlag};

    std::string stderrOutput = CaptureStderr([&]() {
        PowerCliCommand cmd(3, argv);
        cmd.Execute();
    });
    EXPECT_NE(stderrOutput.find("suspend"), std::string::npos);
    EXPECT_NE(stderrOutput.find("--immediately"), std::string::npos);
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
        PowerCliCommand cmd(2, argv);
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
        PowerCliCommand cmd(3, argv);
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
        PowerCliCommand cmd(2, argv);
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
        PowerCliCommand cmd(2, argv);
        cmd.Execute();
    });
    EXPECT_NE(output.find("\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(output.find("Connection"), std::string::npos);
    EXPECT_NE(output.find("ERR_CONNECTION_FAIL"), std::string::npos);
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
        PowerCliCommand cmd(2, argv);
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
        PowerCliCommand cmd(4, argv);
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
    char detailVal[200];
    memset(detailVal, 'x', 199);
    detailVal[199] = '\0';
    char* argv[] = {name, command, detailFlag, detailVal};

    std::string output = CaptureStdout([&]() {
        PowerCliCommand cmd(4, argv);
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
        PowerCliCommand cmd(3, argv);
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
        PowerCliCommand cmd(2, argv);
        cmd.Execute();
    });
    EXPECT_EQ(g_lastCalled, "WakeupDevice");
    EXPECT_NE(output.find("\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(output.find("Permission denied"), std::string::npos);
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
        PowerCliCommand cmd(4, argv);
        cmd.Execute();
    });
    EXPECT_EQ(g_lastCalled, "SetDeviceMode");
    EXPECT_NE(output.find("\"status\":\"success\""), std::string::npos);
    EXPECT_NE(output.find("\"mode\":\"normal\""), std::string::npos);
}

/**
 * @tc.name: PowerCliSetModeTest002
 * @tc.desc: Test set-power-mode --mode performance
 * @tc.type: FUNC
 */
HWTEST_F(PowerCliTest, PowerCliSetModeTest002, TestSize.Level0)
{
    char name[] = "power_cli_test";
    char command[] = "set-power-mode";
    char modeFlag[] = "--mode";
    char modeVal[] = "performance";
    char* argv[] = {name, command, modeFlag, modeVal};

    std::string output = CaptureStdout([&]() {
        PowerCliCommand cmd(4, argv);
        cmd.Execute();
    });
    EXPECT_EQ(g_lastCalled, "SetDeviceMode");
    EXPECT_NE(output.find("\"status\":\"success\""), std::string::npos);
    EXPECT_NE(output.find("\"mode\":\"performance\""), std::string::npos);
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
        PowerCliCommand cmd(4, argv);
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
        PowerCliCommand cmd(4, argv);
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
        PowerCliCommand cmd(2, argv);
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
        PowerCliCommand cmd(4, argv);
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
    char modeVal[] = "performance";
    char* argv[] = {name, command, modeFlag, modeVal};

    std::string output = CaptureStdout([&]() {
        PowerCliCommand cmd(4, argv);
        cmd.Execute();
    });
    EXPECT_EQ(g_lastCalled, "SetDeviceMode");
    EXPECT_NE(output.find("\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(output.find("mode transition failed"), std::string::npos);
    EXPECT_NE(output.find("ERR_POWER_MODE_TRANSIT_FAILED"), std::string::npos);
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
        PowerCliCommand cmd(4, argv);
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
        PowerCliCommand cmd(4, argv);
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
        PowerCliCommand cmd(4, argv);
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
        PowerCliCommand cmd(2, argv);
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
        PowerCliCommand cmd(4, argv);
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
        PowerCliCommand cmd(4, argv);
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
        PowerCliCommand cmd(4, argv);
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
        PowerCliCommand cmd(4, argv);
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
        PowerCliCommand cmd(4, argv);
        cmd.Execute();
    });
    EXPECT_EQ(g_lastCalled, "OverrideScreenOffTime");
    EXPECT_NE(output.find("\"status\":\"failed\""), std::string::npos);
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
        PowerCliCommand cmd(2, argv);
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
        PowerCliCommand cmd(2, argv);
        cmd.Execute();
    });
    EXPECT_EQ(g_lastCalled, "RestoreScreenOffTime");
    EXPECT_NE(output.find("\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(output.find("Connection"), std::string::npos);
}

} // namespace
} // namespace PowerMgr
} // namespace OHOS
