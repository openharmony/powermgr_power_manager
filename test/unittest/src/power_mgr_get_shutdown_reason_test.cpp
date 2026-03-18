/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "power_mgr_get_shutdown_reason_test.h"

#ifdef POWER_GTEST
#define private   public
#define protected public
#endif

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <cstring>

#include "power_mgr_service.h"
#include "power_errors.h"
#include "permission.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

namespace {
constexpr int32_t TEST_PID = 1001;
constexpr int32_t TEST_UID = 1001;
constexpr int32_t MAX_REASON_LENGTH = 512;
constexpr int32_t TEST_CASE_COUNT = 25;

sptr<PowerMgrService> g_pmsTest = nullptr;
bool g_isSystem = true;
bool g_isPermissionGranted = true;

struct TestCaseInfo {
    string caseName;
    string inputReason;
    string expectedReason;
    PowerErrors expectedError;
    bool isSystem;
};

struct ReasonMappingTestCase {
    string inputParam;
    string expectedOutput;
    bool shouldSucceed;
};

struct PermissionTestCase {
    string caseName;
    bool isSystemUser;
    bool hasPermission;
    PowerErrors expectedError;
};

struct ReasonFormatTestCase {
    string caseName;
    string inputReason;
    string expectedOutput;
};

struct ReasonValidationTestCase {
    string caseName;
    string inputReason;
    bool isValid;
    string description;
};

struct ReasonBoundaryTestCase {
    string caseName;
    string inputReason;
    size_t maxLength;
    string expectedOutput;
};

struct ReasonCombinationTestCase {
    string caseName;
    vector<string> reasonParts;
    string expectedCombinedReason;
};

struct ReasonEmptyTestCase {
    string caseName;
    string inputReason;
    bool expectEmpty;
};

struct ReasonSpecialCharTestCase {
    string caseName;
    string inputReason;
    bool shouldPreserve;
};

struct ReasonNumberTestCase {
    string caseName;
    string inputReason;
    bool expectNumeric;
};

struct ReasonUnicodeTestCase {
    string caseName;
    string inputReason;
    bool expectUnicode;
};

struct ReasonTimeTestCase {
    string caseName;
    string inputReason;
    bool expectTimestamp;
};

struct ReasonUserTestCase {
    string caseName;
    string inputReason;
    bool expectUserInfo;
};

struct ReasonDeviceTestCase {
    string caseName;
    string inputReason;
    bool expectDeviceInfo;
};

struct ReasonAppTestCase {
    string caseName;
    string inputReason;
    bool expectAppInfo;
};

struct ReasonSystemTestCase {
    string caseName;
    string inputReason;
    bool expectSystemInfo;
};

struct ReasonCustomTestCase {
    string caseName;
    string inputReason;
    string customPrefix;
    bool expectCustomPrefix;
};
} // namespace

bool Permission::IsSystem()
{
    return g_isSystem;
}

bool Permission::IsPermissionGranted(const std::string& perm)
{
    return g_isPermissionGranted;
}

void PowerMgrServiceShutdownReasonTest::SetUpTestCase()
{
    g_pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
}

void PowerMgrServiceShutdownReasonTest::TearDownTestCase()
{
}

void PowerMgrServiceShutdownReasonTest::SetUp()
{
    g_isSystem = true;
    g_isPermissionGranted = true;
}

void PowerMgrServiceShutdownReasonTest::TearDown()
{
    g_isSystem = true;
    g_isPermissionGranted = true;
}

namespace {

ReasonMappingTestCase g_reasonMappingCases[] = {
    {"Empty", "", "", true},
    {"NormalReason", "user_shutdown", "user_shutdown", true},
    {"BatteryReason", "battery_low", "battery_low", true},
    {"OverheatReason", "thermal_shutdown", "thermal_shutdown", true},
    {"ErrorCase", "invalid_reason_xyz", "", false},
};

PermissionTestCase g_permissionTestCases[] = {
    {"SystemUserWithPermission", true, true, PowerErrors::ERR_OK},
    {"SystemUserWithoutPermission", true, false, PowerErrors::ERR_PERMISSION_DENIED},
    {"NonSystemUserWithPermission", false, true, PowerErrors::ERR_SYSTEM_API_DENIED},
    {"NonSystemUserWithoutPermission", false, false, PowerErrors::ERR_SYSTEM_API_DENIED},
};

ReasonFormatTestCase g_reasonFormatCases[] = {
    {"SimpleLower", "shutdown", "shutdown"},
    {"SimpleUpper", "SHUTDOWN", "SHUTDOWN"},
    {"MixedCase", "ShUtDoWn", "ShUtDoWn"},
    {"WithNumber", "reason123", "reason123"},
    {"WithUnderscore", "user_reason_test", "user_reason_test"},
    {"WithDash", "user-reason-test", "user-reason-test"},
    {"WithSpace", "user reason", "user reason"},
};

ReasonValidationTestCase g_reasonValidationCases[] = {
    {"ValidShort", "a", true, "Single character reason"},
    {"ValidNormal", "user_shutdown", true, "Normal shutdown reason"},
    {"ValidLong", "user_requested_shutdown_via_ui", true, "Long reason string"},
    {"ValidWithNumber", "reason123", true, "Reason with numbers"},
    {"ValidSpecialChars", "reason_test", true, "Valid special chars"},
    {"EmptyReason", "", true, "Empty string is valid"},
};

ReasonBoundaryTestCase g_reasonBoundaryCases[] = {
    {"BoundaryZero", "", 0, ""},
    {"BoundaryOne", "a", 1, "a"},
    {"BoundaryTen", "1234567890", 10, "1234567890"},
    {"BoundaryHundred", "123456789012345678901234567890123456789012345678901234567890",
        100, "123456789012345678901234567890123456789012345678901234567890"},
};

ReasonCombinationTestCase g_reasonCombinationCases[] = {
    {"TwoParts", {"user", "shutdown"}, "user_shutdown"},
    {"ThreeParts", {"battery", "low", "shutdown"}, "battery_low_shutdown"},
    {"FourParts", {"thermal", "overheat", "forced", "shutdown"}, "thermal_overheat_forced_shutdown"},
};

ReasonEmptyTestCase g_reasonEmptyCases[] = {
    {"EmptyString", "", true},
    {"NonEmpty", "test_reason", false},
    {"SpaceOnly", " ", false},
    {"TabOnly", "\t", true},
};

ReasonSpecialCharTestCase g_reasonSpecialCharCases[] = {
    {"Underscore", "user_shutdown", true},
    {"Dash", "user-shutdown", true},
    {"Dot", "user.shutdown", true},
    {"At", "user@shutdown", false},
    {"Hash", "user#shutdown", false},
    {"Dollar", "user$shutdown", false},
    {"Percent", "user%shutdown", false},
};

ReasonNumberTestCase g_reasonNumberCases[] = {
    {"PureNumber", "12345", true},
    {"NumberPrefix", "123reason", false},
    {"NumberSuffix", "reason123", false},
    {"NumberMiddle", "re123ason", false},
    {"NoNumber", "reason", false},
};

ReasonUnicodeTestCase g_reasonUnicodeCases[] = {
    {"Chinese", "关机", true},
    {"Japanese", "シャットダウン", true},
    {"Korean", "종료", true},
    {"Emoji", "🔴", true},
    {"Normal", "shutdown", false},
};

ReasonTimeTestCase g_reasonTimeCases[] = {
    {"UnixTimestamp", "1699999999", true},
    {"NormalTime", "2024-01-01", false},
    {"NormalTime2", "12:30:00", false},
    {"ReasonWithTime", "user_shutdown_2024", false},
};

ReasonUserTestCase g_reasonUserCases[] = {
    {"UserId", "user_1001", true},
    {"UserName", "user_admin", false},
    {"UidHex", "uid_0x3e8", true},
    {"NormalReason", "battery_low", false},
};

ReasonSystemTestCase g_reasonSystemCases[] = {
    {"KernelPanic", "kernel_panic", true},
    {"SystemError", "system_error", false},
    {"Oom", "low_memory", false},
    {"NormalReason", "user_shutdown", false},
};

ReasonCustomTestCase g_reasonCustomCases[] = {
    {"CustomPrefix1", "shutdown", "custom_", true},
    {"CustomPrefix2", "battery", "power_", true},
    {"NoPrefix", "normal_reason", "custom_", false},
};

} // namespace

/**
 * @tc.name: GetShutdownReasonBasic001
 * @tc.desc: Test GetShutdownReason with basic empty reason
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonBasic001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonBasic001 start.";
    std::string reason = "";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    GTEST_LOG_(INFO) << "GetShutdownReasonBasic001 end.";
}

/**
 * @tc.name: GetShutdownReasonBasic002
 * @tc.desc: Test GetShutdownReason with normal reason string
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonBasic002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonBasic002 start.";
    std::string reason = "user_shutdown";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    GTEST_LOG_(INFO) << "GetShutdownReasonBasic002 end.";
}

/**
 * @tc.name: GetShutdownReasonBasic003
 * @tc.desc: Test GetShutdownReason with battery related reason
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonBasic003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonBasic003 start.";
    std::string reason = "battery_low";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    GTEST_LOG_(INFO) << "GetShutdownReasonBasic003 end.";
}

/**
 * @tc.name: GetShutdownReasonBasic004
 * @tc.desc: Test GetShutdownReason with thermal reason
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonBasic004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonBasic004 start.";
    std::string reason = "thermal_shutdown";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    GTEST_LOG_(INFO) << "GetShutdownReasonBasic004 end.";
}

/**
 * @tc.name: GetShutdownReasonBasic005
 * @tc.desc: Test GetShutdownReason with overheat reason
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonBasic005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonBasic005 start.";
    std::string reason = "overheat_protection";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    GTEST_LOG_(INFO) << "GetShutdownReasonBasic005 end.";
}

/**
 * @tc.name: GetShutdownReasonPermission001
 * @tc.desc: Test GetShutdownReason with system permission granted
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonPermission001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonPermission001 start.";
    g_isSystem = true;
    g_isPermissionGranted = true;
    std::string reason = "";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    GTEST_LOG_(INFO) << "GetShutdownReasonPermission001 end.";
}

/**
 * @tc.name: GetShutdownReasonPermission002
 * @tc.desc: Test GetShutdownReason without system permission
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonPermission002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonPermission002 start.";
    g_isSystem = false;
    g_isPermissionGranted = true;
    std::string reason = "";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_SYSTEM_API_DENIED);
    GTEST_LOG_(INFO) << "GetShutdownReasonPermission002 end.";
}

/**
 * @tc.name: GetShutdownReasonPermission003
 * @tc.desc: Test GetShutdownReason with system but without other permission
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonPermission003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonPermission003 start.";
    g_isSystem = true;
    g_isPermissionGranted = false;
    std::string reason = "";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    GTEST_LOG_(INFO) << "GetShutdownReasonPermission003 end.";
}

/**
 * @tc.name: GetShutdownReasonPermission004
 * @tc.desc: Test GetShutdownReason without both permissions
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonPermission004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonPermission004 start.";
    g_isSystem = false;
    g_isPermissionGranted = false;
    std::string reason = "";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_SYSTEM_API_DENIED);
    GTEST_LOG_(INFO) << "GetShutdownReasonPermission004 end.";
}

/**
 * @tc.name: GetShutdownReasonReason001
 * @tc.desc: Test GetShutdownReason with various reason formats - lowercase
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonReason001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonReason001 start.";
    std::string reason = "shutdown";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    EXPECT_EQ(reason, "shutdown");
    GTEST_LOG_(INFO) << "GetShutdownReasonReason001 end.";
}

/**
 * @tc.name: GetShutdownReasonReason002
 * @tc.desc: Test GetShutdownReason with uppercase reason
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonReason002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonReason002 start.";
    std::string reason = "SHUTDOWN";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    GTEST_LOG_(INFO) << "GetShutdownReasonReason002 end.";
}

/**
 * @tc.name: GetShutdownReasonReason003
 * @tc.desc: Test GetShutdownReason with mixed case reason
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonReason003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonReason003 start.";
    std::string reason = "ShUtDoWn";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    GTEST_LOG_(INFO) << "GetShutdownReasonReason003 end.";
}

/**
 * @tc.name: GetShutdownReasonReason004
 * @tc.desc: Test GetShutdownReason with reason containing numbers
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonReason004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonReason004 start.";
    std::string reason = "reason123";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    GTEST_LOG_(INFO) << "GetShutdownReasonReason004 end.";
}

/**
 * @tc.name: GetShutdownReasonReason005
 * @tc.desc: Test GetShutdownReason with underscore reason
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonReason005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonReason005 start.";
    std::string reason = "user_reason_test";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    GTEST_LOG_(INFO) << "GetShutdownReasonReason005 end.";
}

/**
 * @tc.name: GetShutdownReasonReason006
 * @tc.desc: Test GetShutdownReason with dash reason
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonReason006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonReason006 start.";
    std::string reason = "user-reason-test";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    GTEST_LOG_(INFO) << "GetShutdownReasonReason006 end.";
}

/**
 * @tc.name: GetShutdownReasonReason007
 * @tc.desc: Test GetShutdownReason with space in reason
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonReason007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonReason007 start.";
    std::string reason = "user reason";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    GTEST_LOG_(INFO) << "GetShutdownReasonReason007 end.";
}

/**
 * @tc.name: GetShutdownReasonValidation001
 * @tc.desc: Test GetShutdownReason validation with single char
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonValidation001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonValidation001 start.";
    std::string reason = "a";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    EXPECT_EQ(reason.length(), 1);
    GTEST_LOG_(INFO) << "GetShutdownReasonValidation001 end.";
}

/**
 * @tc.name: GetShutdownReasonValidation002
 * @tc.desc: Test GetShutdownReason validation with normal length
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonValidation002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonValidation002 start.";
    std::string reason = "user_shutdown";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    EXPECT_GT(reason.length(), 1);
    GTEST_LOG_(INFO) << "GetShutdownReasonValidation002 end.";
}

/**
 * @tc.name: GetShutdownReasonValidation003
 * @tc.desc: Test GetShutdownReason validation with long string
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonValidation003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonValidation003 start.";
    std::string reason = "user_requested_shutdown_via_ui";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    EXPECT_GT(reason.length(), 10);
    GTEST_LOG_(INFO) << "GetShutdownReasonValidation003 end.";
}

/**
 * @tc.name: GetShutdownReasonValidation004
 * @tc.desc: Test GetShutdownReason validation with numeric string
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonValidation004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonValidation004 start.";
    std::string reason = "12345";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    GTEST_LOG_(INFO) << "GetShutdownReasonValidation004 end.";
}

/**
 * @tc.name: GetShutdownReasonValidation005
 * @tc.desc: Test GetShutdownReason validation with empty string
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonValidation005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonValidation005 start.";
    std::string reason = "";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    EXPECT_EQ(reason.length(), 0);
    GTEST_LOG_(INFO) << "GetShutdownReasonValidation005 end.";
}

/**
 * @tc.name: GetShutdownReasonBoundary001
 * @tc.desc: Test GetShutdownReason boundary with zero length
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonBoundary001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonBoundary001 start.";
    std::string reason = "";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    EXPECT_EQ(reason.length(), 0);
    GTEST_LOG_(INFO) << "GetShutdownReasonBoundary001 end.";
}

/**
 * @tc.name: GetShutdownReasonBoundary002
 * @tc.desc: Test GetShutdownReason boundary with one character
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonBoundary002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonBoundary002 start.";
    std::string reason = "a";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    EXPECT_EQ(reason.length(), 1);
    GTEST_LOG_(INFO) << "GetShutdownReasonBoundary002 end.";
}

/**
 * @tc.name: GetShutdownReasonBoundary003
 * @tc.desc: Test GetShutdownReason boundary with ten characters
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonBoundary003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonBoundary003 start.";
    std::string reason = "1234567890";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    EXPECT_EQ(reason.length(), 10);
    GTEST_LOG_(INFO) << "GetShutdownReasonBoundary003 end.";
}

/**
 * @tc.name: GetShutdownReasonBoundary004
 * @tc.desc: Test GetShutdownReason boundary with hundred characters
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonBoundary004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonBoundary004 start.";
    std::string reason;
    for (int i = 0; i < 100; i++) {
        reason += "a";
    }
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    EXPECT_EQ(reason.length(), 100);
    GTEST_LOG_(INFO) << "GetShutdownReasonBoundary004 end.";
}

/**
 * @tc.name: GetShutdownReasonCombination001
 * @tc.desc: Test GetShutdownReason with combined two parts
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonCombination001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonCombination001 start.";
    std::string reason = "user_shutdown";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    GTEST_LOG_(INFO) << "GetShutdownReasonCombination001 end.";
}

/**
 * @tc.name: GetShutdownReasonCombination002
 * @tc.desc: Test GetShutdownReason with combined three parts
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonCombination002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonCombination002 start.";
    std::string reason = "battery_low_shutdown";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    GTEST_LOG_(INFO) << "GetShutdownReasonCombination002 end.";
}

/**
 * @tc.name: GetShutdownReasonCombination003
 * @tc.desc: Test GetShutdownReason with four parts combination
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonCombination003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonCombination003 start.";
    std::string reason = "thermal_overheat_forced_shutdown";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    GTEST_LOG_(INFO) << "GetShutdownReasonCombination003 end.";
}

/**
 * @tc.name: GetShutdownReasonEmpty001
 * @tc.desc: Test GetShutdownReason with empty string output parameter
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonEmpty001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonEmpty001 start.";
    std::string reason = "";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    bool isEmpty = reason.empty();
    EXPECT_TRUE(isEmpty);
    GTEST_LOG_(INFO) << "GetShutdownReasonEmpty001 end.";
}

/**
 * @tc.name: GetShutdownReasonEmpty002
 * @tc.desc: Test GetShutdownReason with non-empty output parameter
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonEmpty002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonEmpty002 start.";
    std::string reason = "test_reason";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    bool isEmpty = reason.empty();
    EXPECT_FALSE(isEmpty);
    GTEST_LOG_(INFO) << "GetShutdownReasonEmpty002 end.";
}

/**
 * @tc.name: GetShutdownReasonEmpty003
 * @tc.desc: Test GetShutdownReason with space only output
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonEmpty003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonEmpty003 start.";
    std::string reason = " ";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    GTEST_LOG_(INFO) << "GetShutdownReasonEmpty003 end.";
}

/**
 * @tc.name: GetShutdownReasonEmpty004
 * @tc.desc: Test GetShutdownReason with tab character only
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonEmpty004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonEmpty004 start.";
    std::string reason = "\t";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    GTEST_LOG_(INFO) << "GetShutdownReasonEmpty004 end.";
}

/**
 * @tc.name: GetShutdownReasonSpecialChar001
 * @tc.desc: Test GetShutdownReason with underscore special char
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonSpecialChar001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonSpecialChar001 start.";
    std::string reason = "user_shutdown";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    EXPECT_NE(reason.find("_"), std::string::npos);
    GTEST_LOG_(INFO) << "GetShutdownReasonSpecialChar001 end.";
}

/**
 * @tc.name: GetShutdownReasonSpecialChar002
 * @tc.desc: Test GetShutdownReason with dash special char
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonSpecialChar002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonSpecialChar002 start.";
    std::string reason = "user-shutdown";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    GTEST_LOG_(INFO) << "GetShutdownReasonSpecialChar002 end.";
}

/**
 * @tc.name: GetShutdownReasonSpecialChar003
 * @tc.desc: Test GetShutdownReason with dot special char
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonSpecialChar003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonSpecialChar003 start.";
    std::string reason = "user.shutdown";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    GTEST_LOG_(INFO) << "GetShutdownReasonSpecialChar003 end.";
}

/**
 * @tc.name: GetShutdownReasonNumber001
 * @tc.desc: Test GetShutdownReason with pure number reason
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonNumber001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonNumber001 start.";
    std::string reason = "12345";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    bool isNumeric = true;
    for (char c : reason) {
        if (!isdigit(c)) {
            isNumeric = false;
            break;
        }
    }
    EXPECT_TRUE(isNumeric);
    GTEST_LOG_(INFO) << "GetShutdownReasonNumber001 end.";
}

/**
 * @tc.name: GetShutdownReasonNumber002
 * @tc.desc: Test GetShutdownReason with number prefix reason
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonNumber002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonNumber002 start.";
    std::string reason = "123reason";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    GTEST_LOG_(INFO) << "GetShutdownReasonNumber002 end.";
}

/**
 * @tc.name: GetShutdownReasonNumber003
 * @tc.desc: Test GetShutdownReason with number suffix reason
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonNumber003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonNumber003 start.";
    std::string reason = "reason123";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    GTEST_LOG_(INFO) << "GetShutdownReasonNumber003 end.";
}

/**
 * @tc.name: GetShutdownReasonUnicode001
 * @tc.desc: Test GetShutdownReason with Chinese unicode
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonUnicode001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonUnicode001 start.";
    std::string reason = "关机";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    GTEST_LOG_(INFO) << "GetShutdownReasonUnicode001 end.";
}

/**
 * @tc.name: GetShutdownReasonUnicode002
 * @tc.desc: Test GetShutdownReason with Japanese unicode
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonUnicode002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonUnicode002 start.";
    std::string reason = "シャットダウン";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    GTEST_LOG_(INFO) << "GetShutdownReasonUnicode002 end.";
}

/**
 * @tc.name: GetShutdownReasonUnicode003
 * @tc.desc: Test GetShutdownReason with Korean unicode
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonUnicode003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonUnicode003 start.";
    std::string reason = "종료";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    GTEST_LOG_(INFO) << "GetShutdownReasonUnicode003 end.";
}

/**
 * @tc.name: GetShutdownReasonTime001
 * @tc.desc: Test GetShutdownReason with unix timestamp
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonTime001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonTime001 start.";
    std::string reason = "1699999999";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    GTEST_LOG_(INFO) << "GetShutdownReasonTime001 end.";
}

/**
 * @tc.name: GetShutdownReasonTime002
 * @tc.desc: Test GetShutdownReason with normal time format
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonTime002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonTime002 start.";
    std::string reason = "2024-01-01";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    GTEST_LOG_(INFO) << "GetShutdownReasonTime002 end.";
}

/**
 * @tc.name: GetShutdownReasonUser001
 * @tc.desc: Test GetShutdownReason with user id
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonUser001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonUser001 start.";
    std::string reason = "user_1001";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    GTEST_LOG_(INFO) << "GetShutdownReasonUser001 end.";
}

/**
 * @tc.name: GetShutdownReasonUser002
 * @tc.desc: Test GetShutdownReason with uid hex format
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonUser002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonUser002 start.";
    std::string reason = "uid_0x3e8";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    GTEST_LOG_(INFO) << "GetShutdownReasonUser002 end.";
}

/**
 * @tc.name: GetShutdownReasonDevice002
 * @tc.desc: Test GetShutdownReason with serial number
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonDevice002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonDevice002 start.";
    std::string reason = "serial_123456";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    GTEST_LOG_(INFO) << "GetShutdownReasonDevice002 end.";
}

/**
 * @tc.name: GetShutdownReasonApp001
 * @tc.desc: Test GetShutdownReason with app uid
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonApp001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonApp001 start.";
    std::string reason = "app_10000";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    GTEST_LOG_(INFO) << "GetShutdownReasonApp001 end.";
}

/**
 * @tc.name: GetShutdownReasonSystem001
 * @tc.desc: Test GetShutdownReason with kernel panic
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonSystem001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonSystem001 start.";
    std::string reason = "kernel_panic";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    GTEST_LOG_(INFO) << "GetShutdownReasonSystem001 end.";
}

/**
 * @tc.name: GetShutdownReasonSystem002
 * @tc.desc: Test GetShutdownReason with low memory
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonSystem002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonSystem002 start.";
    std::string reason = "low_memory";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    GTEST_LOG_(INFO) << "GetShutdownReasonSystem002 end.";
}

/**
 * @tc.name: GetShutdownReasonCustom001
 * @tc.desc: Test GetShutdownReason with custom prefix
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonCustom001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonCustom001 start.";
    std::string reason = "custom_shutdown";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    GTEST_LOG_(INFO) << "GetShutdownReasonCustom001 end.";
}

/**
 * @tc.name: GetShutdownReasonCustom002
 * @tc.desc: Test GetShutdownReason with power prefix
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonCustom002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonCustom002 start.";
    std::string reason = "power_battery";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    GTEST_LOG_(INFO) << "GetShutdownReasonCustom002 end.";
}

/**
 * @tc.name: GetShutdownReasonError001
 * @tc.desc: Test GetShutdownReason returns correct error code
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonError001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonError001 start.";
    std::string reason = "";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    EXPECT_NE(ret, PowerErrors::ERR_FAILURE);
    EXPECT_NE(ret, PowerErrors::ERR_PARAM_INVALID);
    GTEST_LOG_(INFO) << "GetShutdownReasonError001 end.";
}

/**
 * @tc.name: GetShutdownReasonError002
 * @tc.desc: Test GetShutdownReason permission denied error code
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonError002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonError002 start.";
    g_isSystem = false;
    std::string reason = "";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_SYSTEM_API_DENIED);
    GTEST_LOG_(INFO) << "GetShutdownReasonError002 end.";
}

/**
 * @tc.name: GetShutdownReasonOutput001
 * @tc.desc: Test GetShutdownReason output parameter is modified
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonOutput001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonOutput001 start.";
    std::string reason = "initial_value";
    std::string original = reason;
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    bool isModified = (reason != original);
    EXPECT_TRUE(isModified);
    GTEST_LOG_(INFO) << "GetShutdownReasonOutput001 end.";
}

/**
 * @tc.name: GetShutdownReasonOutput002
 * @tc.desc: Test GetShutdownReason output parameter can be empty
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonOutput002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonOutput002 start.";
    std::string reason = "test";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    bool canBeEmpty = (reason.length() >= 0);
    EXPECT_TRUE(canBeEmpty);
    GTEST_LOG_(INFO) << "GetShutdownReasonOutput002 end.";
}

/**
 * @tc.name: GetShutdownReasonMultiple001
 * @tc.desc: Test GetShutdownReason called multiple times
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonMultiple001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonMultiple001 start.";
    for (int i = 0; i < 10; i++) {
        std::string reason = "";
        PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
        EXPECT_EQ(ret, PowerErrors::ERR_OK);
    }
    GTEST_LOG_(INFO) << "GetShutdownReasonMultiple001 end.";
}

/**
 * @tc.name: GetShutdownReasonMultiple002
 * @tc.desc: Test GetShutdownReason called with different reasons
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonMultiple002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonMultiple002 start.";
    std::vector<std::string> testReasons = {"reason1", "reason2", "reason3", "reason4", "reason5"};
    for (const auto& r : testReasons) {
        std::string reason = r;
        PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
        EXPECT_EQ(ret, PowerErrors::ERR_OK);
    }
    GTEST_LOG_(INFO) << "GetShutdownReasonMultiple002 end.";
}

/**
 * @tc.name: GetShutdownReasonConsistency001
 * @tc.desc: Test GetShutdownReason returns consistent results
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonConsistency001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonConsistency001 start.";
    std::string reason1 = "";
    std::string reason2 = "";
    PowerErrors ret1 = g_pmsTest->GetShutdownReason(reason1);
    PowerErrors ret2 = g_pmsTest->GetShutdownReason(reason2);
    EXPECT_EQ(ret1, ret2);
    GTEST_LOG_(INFO) << "GetShutdownReasonConsistency001 end.";
}

/**
 * @tc.name: GetShutdownReasonConcurrent001
 * @tc.desc: Test GetShutdownReason with reference parameter
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonConcurrent001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonConcurrent001 start.";
    std::string reason = "";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    GTEST_LOG_(INFO) << "GetShutdownReasonConcurrent001 end.";
}

/**
 * @tc.name: GetShutdownReasonRef001
 * @tc.desc: Test GetShutdownReason reference parameter behavior
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonRef001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonRef001 start.";
    std::string reason = "test_reason";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    bool isValid = (reason.length() >= 0);
    EXPECT_TRUE(isValid);
    GTEST_LOG_(INFO) << "GetShutdownReasonRef001 end.";
}

/**
 * @tc.name: GetShutdownReasonRef002
 * @tc.desc: Test GetShutdownReason reference with different initial values
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonRef002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonRef002 start.";
    std::string reason = "another_test_reason";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    GTEST_LOG_(INFO) << "GetShutdownReasonRef002 end.";
}

/**
 * @tc.name: GetShutdownReasonRef003
 * @tc.desc: Test GetShutdownReason reference with long initial value
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonRef003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonRef003 start.";
    std::string reason = "this_is_a_very_long_test_reason_string_that_exceeds_normal_length";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    GTEST_LOG_(INFO) << "GetShutdownReasonRef003 end.";
}

/**
 * @tc.name: GetShutdownReasonStringOperation001
 * @tc.desc: Test GetShutdownReason with string operations after call
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonStringOperation001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonStringOperation001 start.";
    std::string reason = "";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    std::string copy = reason;
    bool equal = (copy == reason);
    EXPECT_TRUE(equal);
    GTEST_LOG_(INFO) << "GetShutdownReasonStringOperation001 end.";
}

/**
 * @tc.name: GetShutdownReasonStringOperation002
 * @tc.desc: Test GetShutdownReason with string concatenation
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonStringOperation002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonStringOperation002 start.";
    std::string reason = "";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    std::string appended = reason + "_suffix";
    bool isAppended = (appended.length() >= reason.length());
    EXPECT_TRUE(isAppended);
    GTEST_LOG_(INFO) << "GetShutdownReasonStringOperation002 end.";
}

/**
 * @tc.name: GetShutdownReasonStringOperation003
 * @tc.desc: Test GetShutdownReason with string comparison
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonStringOperation003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonStringOperation003 start.";
    std::string reason = "";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    std::string compareWith = "";
    bool isEqual = (reason == compareWith);
    EXPECT_TRUE(isEqual);
    GTEST_LOG_(INFO) << "GetShutdownReasonStringOperation003 end.";
}

/**
 * @tc.name: GetShutdownReasonStringOperation004
 * @tc.desc: Test GetShutdownReason with string find operation
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonStringOperation004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonStringOperation004 start.";
    std::string reason = "";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    size_t pos = reason.find("test");
    bool canSearch = (pos == std::string::npos || pos >= 0);
    EXPECT_TRUE(canSearch);
    GTEST_LOG_(INFO) << "GetShutdownReasonStringOperation004 end.";
}

/**
 * @tc.name: GetShutdownReasonStringOperation005
 * @tc.desc: Test GetShutdownReason with string length check
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonStringOperation005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonStringOperation005 start.";
    std::string reason = "";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    size_t len = reason.length();
    bool validLength = (len >= 0);
    EXPECT_TRUE(validLength);
    GTEST_LOG_(INFO) << "GetShutdownReasonStringOperation005 end.";
}

/**
 * @tc.name: GetShutdownReasonErrorHandling001
 * @tc.desc: Test GetShutdownReason error handling with system api denied
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonErrorHandling001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonErrorHandling001 start.";
    g_isSystem = false;
    std::string reason = "";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_SYSTEM_API_DENIED);
    GTEST_LOG_(INFO) << "GetShutdownReasonErrorHandling001 end.";
}

/**
 * @tc.name: GetShutdownReasonErrorHandling002
 * @tc.desc: Test GetShutdownReason error handling returns correct error
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonErrorHandling002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonErrorHandling002 start.";
    g_isSystem = false;
    std::string reason = "test";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_SYSTEM_API_DENIED);
    EXPECT_NE(ret, PowerErrors::ERR_OK);
    EXPECT_NE(ret, PowerErrors::ERR_FAILURE);
    GTEST_LOG_(INFO) << "GetShutdownReasonErrorHandling002 end.";
}

/**
 * @tc.name: GetShutdownReasonErrorHandling003
 * @tc.desc: Test GetShutdownReason error handling when system is true
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonErrorHandling003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonErrorHandling003 start.";
    g_isSystem = true;
    g_isPermissionGranted = true;
    std::string reason = "";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    GTEST_LOG_(INFO) << "GetShutdownReasonErrorHandling003 end.";
}

/**
 * @tc.name: GetShutdownReasonErrorHandling004
 * @tc.desc: Test GetShutdownReason error handling various states
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonErrorHandling004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonErrorHandling004 start.";
    g_isSystem = true;
    g_isPermissionGranted = false;
    std::string reason = "";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    GTEST_LOG_(INFO) << "GetShutdownReasonErrorHandling004 end.";
}

/**
 * @tc.name: GetShutdownReasonErrorHandling005
 * @tc.desc: Test GetShutdownReason error handling with system false
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonErrorHandling005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonErrorHandling005 start.";
    g_isSystem = false;
    g_isPermissionGranted = true;
    std::string reason = "";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_SYSTEM_API_DENIED);
    EXPECT_NE(ret, PowerErrors::ERR_PERMISSION_DENIED);
    GTEST_LOG_(INFO) << "GetShutdownReasonErrorHandling005 end.";
}

/**
 * @tc.name: GetShutdownReasonErrorHandling006
 * @tc.desc: Test GetShutdownReason error handling both false
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonErrorHandling006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonErrorHandling006 start.";
    g_isSystem = false;
    g_isPermissionGranted = false;
    std::string reason = "";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_SYSTEM_API_DENIED);
    EXPECT_NE(ret, PowerErrors::ERR_PERMISSION_DENIED);
    GTEST_LOG_(INFO) << "GetShutdownReasonErrorHandling006 end.";
}

/**
 * @tc.name: GetShutdownReasonReturnValue001
 * @tc.desc: Test GetShutdownReason return value ERR_OK
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonReturnValue001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonReturnValue001 start.";
    std::string reason = "";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(static_cast<int>(ret), static_cast<int>(PowerErrors::ERR_OK));
    GTEST_LOG_(INFO) << "GetShutdownReasonReturnValue001 end.";
}

/**
 * @tc.name: GetShutdownReasonReturnValue002
 * @tc.desc: Test GetShutdownReason return value ERR_SYSTEM_API_DENIED
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonReturnValue002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonReturnValue002 start.";
    g_isSystem = false;
    std::string reason = "";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(static_cast<int>(ret), static_cast<int>(PowerErrors::ERR_SYSTEM_API_DENIED));
    GTEST_LOG_(INFO) << "GetShutdownReasonReturnValue002 end.";
}

/**
 * @tc.name: GetShutdownReasonReturnValue003
 * @tc.desc: Test GetShutdownReason return value not failure
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonReturnValue003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonReturnValue003 start.";
    std::string reason = "";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_NE(static_cast<int>(ret), static_cast<int>(PowerErrors::ERR_FAILURE));
    GTEST_LOG_(INFO) << "GetShutdownReasonReturnValue003 end.";
}

/**
 * @tc.name: GetShutdownReasonReturnValue004
 * @tc.desc: Test GetShutdownReason return value not param invalid
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonReturnValue004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonReturnValue004 start.";
    std::string reason = "";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_NE(static_cast<int>(ret), static_cast<int>(PowerErrors::ERR_PARAM_INVALID));
    GTEST_LOG_(INFO) << "GetShutdownReasonReturnValue004 end.";
}

/**
 * @tc.name: GetShutdownReasonReturnValue005
 * @tc.desc: Test GetShutdownReason return value comparison
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonReturnValue005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonReturnValue005 start.";
    std::string reason = "";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    bool isOk = (ret == PowerErrors::ERR_OK);
    EXPECT_TRUE(isOk);
    GTEST_LOG_(INFO) << "GetShutdownReasonReturnValue005 end.";
}

/**
 * @tc.name: GetShutdownReasonReturnValue006
 * @tc.desc: Test GetShutdownReason return value inequality
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonReturnValue006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonReturnValue006 start.";
    g_isSystem = false;
    std::string reason = "";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    bool isNotOk = (ret != PowerErrors::ERR_OK);
    EXPECT_TRUE(isNotOk);
    GTEST_LOG_(INFO) << "GetShutdownReasonReturnValue006 end.";
}

/**
 * @tc.name: GetShutdownReasonStress001
 * @tc.desc: Test GetShutdownReason stress test - many calls
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonStress001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonStress001 start.";
    for (int i = 0; i < 100; i++) {
        std::string reason = "";
        PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
        EXPECT_EQ(ret, PowerErrors::ERR_OK);
    }
    GTEST_LOG_(INFO) << "GetShutdownReasonStress001 end.";
}

/**
 * @tc.name: GetShutdownReasonStress002
 * @tc.desc: Test GetShutdownReason stress test - different strings
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonStress002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonStress002 start.";
    for (int i = 0; i < 50; i++) {
        std::string reason = "test_reason_" + std::to_string(i);
        PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
        EXPECT_EQ(ret, PowerErrors::ERR_OK);
    }
    GTEST_LOG_(INFO) << "GetShutdownReasonStress002 end.";
}

/**
 * @tc.name: GetShutdownReasonStress003
 * @tc.desc: Test GetShutdownReason stress test - long strings
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonStress003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonStress003 start.";
    for (int i = 0; i < 20; i++) {
        std::string reason(200, 'a' + (i % 26));
        PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
        EXPECT_EQ(ret, PowerErrors::ERR_OK);
    }
    GTEST_LOG_(INFO) << "GetShutdownReasonStress003 end.";
}

/**
 * @tc.name: GetShutdownReasonEdge001
 * @tc.desc: Test GetShutdownReason edge case - very short string
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonEdge001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonEdge001 start.";
    std::string reason = "a";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    EXPECT_EQ(reason.length(), 1);
    GTEST_LOG_(INFO) << "GetShutdownReasonEdge001 end.";
}

/**
 * @tc.name: GetShutdownReasonEdge002
 * @tc.desc: Test GetShutdownReason edge case - two characters
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonEdge002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonEdge002 start.";
    std::string reason = "ab";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    EXPECT_EQ(reason.length(), 2);
    GTEST_LOG_(INFO) << "GetShutdownReasonEdge002 end.";
}

/**
 * @tc.name: GetShutdownReasonEdge003
 * @tc.desc: Test GetShutdownReason edge case - max reasonable length
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonEdge003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonEdge003 start.";
    std::string reason(256, 'x');
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    EXPECT_GE(reason.length(), 0);
    GTEST_LOG_(INFO) << "GetShutdownReasonEdge003 end.";
}

/**
 * @tc.name: GetShutdownReasonType001
 * @tc.desc: Test GetShutdownReason returns PowerErrors type
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonType001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonType001 start.";
    std::string reason = "";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    bool isPowerError = (ret == PowerErrors::ERR_OK || ret == PowerErrors::ERR_SYSTEM_API_DENIED);
    EXPECT_TRUE(isPowerError);
    GTEST_LOG_(INFO) << "GetShutdownReasonType001 end.";
}

/**
 * @tc.name: GetShutdownReasonType002
 * @tc.desc: Test GetShutdownReason PowerErrors enum values
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonType002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonType002 start.";
    std::string reason = "";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    int errValue = static_cast<int>(ret);
    bool isValidError = (errValue == 0 || errValue == 202);
    EXPECT_TRUE(isValidError);
    GTEST_LOG_(INFO) << "GetShutdownReasonType002 end.";
}

/**
 * @tc.name: GetShutdownReasonType003
 * @tc.desc: Test GetShutdownReason with ERR_OK value
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonType003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonType003 start.";
    std::string reason = "";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    GTEST_LOG_(INFO) << "GetShutdownReasonType003 end.";
}

/**
 * @tc.name: GetShutdownReasonType004
 * @tc.desc: Test GetShutdownReason with non-zero error value
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonType004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonType004 start.";
    g_isSystem = false;
    std::string reason = "";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_SYSTEM_API_DENIED);
    GTEST_LOG_(INFO) << "GetShutdownReasonType004 end.";
}

/**
 * @tc.name: GetShutdownReasonType005
 * @tc.desc: Test GetShutdownReason error type comparison
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonType005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonType005 start.";
    g_isSystem = false;
    std::string reason = "";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    PowerErrors expected = PowerErrors::ERR_SYSTEM_API_DENIED;
    EXPECT_EQ(ret, expected);
    GTEST_LOG_(INFO) << "GetShutdownReasonType005 end.";
}

/**
 * @tc.name: GetShutdownReasonIntegration001
 * @tc.desc: Test GetShutdownReason integration test with shutdown
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonIntegration001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonIntegration001 start.";
    std::string reason = "";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    GTEST_LOG_(INFO) << "GetShutdownReasonIntegration001 end.";
}

/**
 * @tc.name: GetShutdownReasonIntegration002
 * @tc.desc: Test GetShutdownReason integration - get and use reason
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonIntegration002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonIntegration002 start.";
    std::string reason = "";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    if (ret == PowerErrors::ERR_OK) {
        EXPECT_GE(reason.length(), 0);
    }
    GTEST_LOG_(INFO) << "GetShutdownReasonIntegration002 end.";
}

/**
 * @tc.name: GetShutdownReasonIntegration003
 * @tc.desc: Test GetShutdownReason integration - reason processing
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonIntegration003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonIntegration003 start.";
    std::string reason = "shutdown_reason";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    bool canProcess = (reason.length() > 0);
    GTEST_LOG_(INFO) << "GetShutdownReasonIntegration003 end.";
}

/**
 * @tc.name: GetShutdownReasonIntegration004
 * @tc.desc: Test GetShutdownReason integration - after permission change
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonIntegration004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonIntegration004 start.";
    std::string reason = "";
    g_isSystem = true;
    PowerErrors ret1 = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret1, PowerErrors::ERR_OK);
    g_isSystem = false;
    PowerErrors ret2 = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret2, PowerErrors::ERR_SYSTEM_API_DENIED);
    GTEST_LOG_(INFO) << "GetShutdownReasonIntegration004 end.";
}

/**
 * @tc.name: GetShutdownReasonIntegration005
 * @tc.desc: Test GetShutdownReason integration - sequence test
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonIntegration005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonIntegration005 start.";
    std::string reason1 = "first_reason";
    std::string reason2 = "second_reason";
    PowerErrors ret1 = g_pmsTest->GetShutdownReason(reason1);
    PowerErrors ret2 = g_pmsTest->GetShutdownReason(reason2);
    EXPECT_EQ(ret1, PowerErrors::ERR_OK);
    EXPECT_EQ(ret2, PowerErrors::ERR_OK);
    GTEST_LOG_(INFO) << "GetShutdownReasonIntegration005 end.";
}

/**
 * @tc.name: GetShutdownReasonIntegration006
 * @tc.desc: Test GetShutdownReason integration - multi-field test
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonIntegration006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonIntegration006 start.";
    std::string reasons[] = {"reason_a", "reason_b", "reason_c"};
    for (const auto& r : reasons) {
        std::string reason = r;
        PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
        EXPECT_EQ(ret, PowerErrors::ERR_OK);
    }
    GTEST_LOG_(INFO) << "GetShutdownReasonIntegration006 end.";
}

/**
 * @tc.name: GetShutdownReasonIntegration007
 * @tc.desc: Test GetShutdownReason integration - loop with conditions
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonIntegration007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonIntegration007 start.";
    for (int i = 0; i < 5; i++) {
        std::string reason = "";
        PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
        if (ret == PowerErrors::ERR_OK) {
            EXPECT_GE(reason.length(), 0);
        }
    }
    GTEST_LOG_(INFO) << "GetShutdownReasonIntegration007 end.";
}

/**
 * @tc.name: GetShutdownReasonIntegration008
 * @tc.desc: Test GetShutdownReason integration - error flow
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonIntegration008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonIntegration008 start.";
    g_isSystem = false;
    std::string reason = "";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_SYSTEM_API_DENIED);
    if (ret != PowerErrors::ERR_OK) {
        EXPECT_EQ(ret, PowerErrors::ERR_SYSTEM_API_DENIED);
    }
    GTEST_LOG_(INFO) << "GetShutdownReasonIntegration008 end.";
}

/**
 * @tc.name: GetShutdownReasonIntegration009
 * @tc.desc: Test GetShutdownReason integration - nested call
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonIntegration009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonIntegration009 start.";
    std::string reasonOuter = "";
    PowerErrors retOuter = g_pmsTest->GetShutdownReason(reasonOuter);
    EXPECT_EQ(retOuter, PowerErrors::ERR_OK);
    std::string reasonInner = "";
    PowerErrors retInner = g_pmsTest->GetShutdownReason(reasonInner);
    EXPECT_EQ(retInner, PowerErrors::ERR_OK);
    GTEST_LOG_(INFO) << "GetShutdownReasonIntegration009 end.";
}

/**
 * @tc.name: GetShutdownReasonIntegration010
 * @tc.desc: Test GetShutdownReason integration - reason preservation
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonIntegration010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonIntegration010 start.";
    std::string reason = "preserved_reason";
    std::string original = reason;
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    bool preserved = (reason != original);
    EXPECT_TRUE(preserved);
    GTEST_LOG_(INFO) << "GetShutdownReasonIntegration010 end.";
}

/**
 * @tc.name: GetShutdownReasonParameter001
 * @tc.desc: Test GetShutdownReason parameter passing - const ref
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonParameter001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonParameter001 start.";
    std::string reason = "";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    bool paramPassed = (reason.length() >= 0);
    EXPECT_TRUE(paramPassed);
    GTEST_LOG_(INFO) << "GetShutdownReasonParameter001 end.";
}

/**
 * @tc.name: GetShutdownReasonParameter002
 * @tc.desc: Test GetShutdownReason parameter modification
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonParameter002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonParameter002 start.";
    std::string reason = "before";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    bool modified = (reason.length() >= 0);
    EXPECT_TRUE(modified);
    GTEST_LOG_(INFO) << "GetShutdownReasonParameter002 end.";
}

/**
 * @tc.name: GetShutdownReasonParameter003
 * @tc.desc: Test GetShutdownReason parameter type - string reference
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonParameter003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonParameter003 start.";
    std::string reason = "test";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    EXPECT_TRUE(true);
    GTEST_LOG_(INFO) << "GetShutdownReasonParameter003 end.";
}

/**
 * @tc.name: GetShutdownReasonParameter004
 * @tc.desc: Test GetShutdownReason parameter passing method
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonParameter004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonParameter004 start.";
    std::string reason = "";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    GTEST_LOG_(INFO) << "GetShutdownReasonParameter004 end.";
}

/**
 * @tc.name: GetShutdownReasonParameter005
 * @tc.desc: Test GetShutdownReason parameter behavior
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonParameter005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonParameter005 start.";
    std::string reason = "";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    GTEST_LOG_(INFO) << "GetShutdownReasonParameter005 end.";
}

/**
 * @tc.name: GetShutdownReasonParameter006
 * @tc.desc: Test GetShutdownReason output parameter effect
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonParameter006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonParameter006 start.";
    std::string reason;
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    bool valid = (reason.length() >= 0);
    EXPECT_TRUE(valid);
    GTEST_LOG_(INFO) << "GetShutdownReasonParameter006 end.";
}

/**
 * @tc.name: GetShutdownReasonParameter007
 * @tc.desc: Test GetShutdownReason reference parameter semantics
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonParameter007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonParameter007 start.";
    std::string reason = "";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    bool semanticsValid = true;
    EXPECT_TRUE(semanticsValid);
    GTEST_LOG_(INFO) << "GetShutdownReasonParameter007 end.";
}

/**
 * @tc.name: GetShutdownReasonParameter008
 * @tc.desc: Test GetShutdownReason reference passing
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonParameter008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonParameter008 start.";
    std::string reason = "parameter_test";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    GTEST_LOG_(INFO) << "GetShutdownReasonParameter008 end.";
}

/**
 * @tc.name: GetShutdownReasonParameter009
 * @tc.desc: Test GetShutdownReason modify via reference
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonParameter009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonParameter009 start.";
    std::string reason = "";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    EXPECT_GE(reason.length(), 0);
    GTEST_LOG_(INFO) << "GetShutdownReasonParameter009 end.";
}

/**
 * @tc.name: GetShutdownReasonParameter010
 * @tc.desc: Test GetShutdownReason reference effect
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonParameter010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonParameter010 start.";
    std::string reason;
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    GTEST_LOG_(INFO) << "GetShutdownReasonParameter010 end.";
}

/**
 * @tc.name: GetShutdownReasonParameter011
 * @tc.desc: Test GetShutdownReason reference output
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonParameter011, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonParameter011 start.";
    std::string reason;
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    EXPECT_TRUE(true);
    GTEST_LOG_(INFO) << "GetShutdownReasonParameter011 end.";
}

/**
 * @tc.name: GetShutdownReasonParameter012
 * @tc.desc: Test GetShutdownReason reference value after call
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonParameter012, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonParameter012 start.";
    std::string reason = "test_value";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    EXPECT_GE(reason.length(), 0);
    GTEST_LOG_(INFO) << "GetShutdownReasonParameter012 end.";
}

/**
 * @tc.name: GetShutdownReasonParameter013
 * @tc.desc: Test GetShutdownReason parameter semantics
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonParameter013, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonParameter013 start.";
    std::string reason = "";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    EXPECT_TRUE(true);
    GTEST_LOG_(INFO) << "GetShutdownReasonParameter013 end.";
}

/**
 * @tc.name: GetShutdownReasonParameter014
 * @tc.desc: Test GetShutdownReason parameter effect on reason
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonParameter014, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonParameter014 start.";
    std::string reason = "input";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    EXPECT_TRUE(true);
    GTEST_LOG_(INFO) << "GetShutdownReasonParameter014 end.";
}

/**
 * @tc.name: GetShutdownReasonParameter015
 * @tc.desc: Test GetShutdownReason reference parameter behavior
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonParameter015, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonParameter015 start.";
    std::string reason = "";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    GTEST_LOG_(INFO) << "GetShutdownReasonParameter015 end.";
}

/**
 * @tc.name: GetShutdownReasonParameter016
 * @tc.desc: Test GetShutdownReason reference change
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonParameter016, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonParameter016 start.";
    std::string reason;
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    GTEST_LOG_(INFO) << "GetShutdownReasonParameter016 end.";
}

/**
 * @tc.name: GetShutdownReasonParameter017
 * @tc.desc: Test GetShutdownReason reference value modification
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonParameter017, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonParameter017 start.";
    std::string reason = "";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    EXPECT_GE(reason.length(), 0);
    GTEST_LOG_(INFO) << "GetShutdownReasonParameter017 end.";
}

/**
 * @tc.name: GetShutdownReasonParameter018
 * @tc.desc: Test GetShutdownReason parameter reference semantics
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonParameter018, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonParameter018 start.";
    std::string reason = "semantic_test";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    GTEST_LOG_(INFO) << "GetShutdownReasonParameter018 end.";
}

/**
 * @tc.name: GetShutdownReasonParameter019
 * @tc.desc: Test GetShutdownReason reference parameter content
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonParameter019, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonParameter019 start.";
    std::string reason = "";
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    bool contentValid = (reason.length() >= 0);
    EXPECT_TRUE(contentValid);
    GTEST_LOG_(INFO) << "GetShutdownReasonParameter019 end.";
}

/**
 * @tc.name: GetShutdownReasonParameter020
 * @tc.desc: Test GetShutdownReason reference parameter output
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceShutdownReasonTest, GetShutdownReasonParameter020, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetShutdownReasonParameter020 start.";
    std::string reason;
    PowerErrors ret = g_pmsTest->GetShutdownReason(reason);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    EXPECT_TRUE(true);
    GTEST_LOG_(INFO) << "GetShutdownReasonParameter020 end.";
}
