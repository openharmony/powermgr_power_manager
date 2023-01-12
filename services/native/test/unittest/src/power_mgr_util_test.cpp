/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "accesstoken_kit.h"
#include "mock_accesstoken_kit.h"
#include "permission.h"
#include "tokenid_kit.h"

using namespace OHOS::Security::AccessToken;
using namespace OHOS::PowerMgr;
using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace PowerMgr {
class PowerMgrUtilTest : public testing::Test {};
} // namespace PowerMgr
} // namespace OHOS

namespace {
/**
 * @tc.name: PermissionIsSystemNative
 * @tc.desc: The IsSystem and IsPermissionGranted functions are granted by default as TOKEN_NATIVE or TOKEN_SHELL types
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrUtilTest, PermissionIsSystemNative, TestSize.Level0)
{
    MockAccesstokenKit::MockSetTokenTypeFlag(ATokenTypeEnum::TOKEN_NATIVE);
    EXPECT_TRUE(Permission::IsSystem());
    EXPECT_TRUE(Permission::IsPermissionGranted("REBOOT"));

    MockAccesstokenKit::MockSetTokenTypeFlag(ATokenTypeEnum::TOKEN_SHELL);
    EXPECT_TRUE(Permission::IsSystem());
    EXPECT_TRUE(Permission::IsPermissionGranted("REBOOT"));
}

/**
 * @tc.name: PermissionIsSystemHap
 * @tc.desc: The function IsSystem and IsPermissionGranted in the test TOKEN_HAP
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrUtilTest, PermissionIsSystemHap, TestSize.Level0)
{
    MockAccesstokenKit::MockSetTokenTypeFlag(ATokenTypeEnum::TOKEN_HAP);
    MockAccesstokenKit::MockSetSystemApp(false);
    EXPECT_FALSE(Permission::IsSystem());

    MockAccesstokenKit::MockSetSystemApp(true);
    EXPECT_TRUE(Permission::IsSystem());
    MockAccesstokenKit::MockSetPermissionState(PermissionState::PERMISSION_GRANTED);
    EXPECT_TRUE(Permission::IsPermissionGranted("REBOOT"));
}

/**
 * @tc.name: PermissionIsSystemInvalid
 * @tc.desc: The IsSystem and IsPermissionGranted do not have permissions on TOKEN_INVALID or TOKEN_TYPE_BUTT types
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrUtilTest, PermissionIsSystemInvalid, TestSize.Level0)
{
    MockAccesstokenKit::MockSetTokenTypeFlag(ATokenTypeEnum::TOKEN_INVALID);
    EXPECT_FALSE(Permission::IsSystem());
    EXPECT_FALSE(Permission::IsPermissionGranted("REBOOT"));

    MockAccesstokenKit::MockSetTokenTypeFlag(ATokenTypeEnum::TOKEN_TYPE_BUTT);
    EXPECT_FALSE(Permission::IsSystem());
    EXPECT_FALSE(Permission::IsPermissionGranted("REBOOT"));
}

/**
 * @tc.name: PermissionIsPermissionGrantedHap
 * @tc.desc: Test Permission function IsPermissionGranted is TOKEN_HAP
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrUtilTest, PermissionIsPermissionGrantedHap, TestSize.Level0)
{
    MockAccesstokenKit::MockSetTokenTypeFlag(ATokenTypeEnum::TOKEN_HAP);
    MockAccesstokenKit::MockSetPermissionState(PermissionState::PERMISSION_DENIED);
    EXPECT_FALSE(Permission::IsPermissionGranted("REBOOT"));

    MockAccesstokenKit::MockSetPermissionState(PermissionState::PERMISSION_GRANTED);
    EXPECT_TRUE(Permission::IsPermissionGranted("REBOOT"));
}

/**
 * @tc.name: PermissionIsPermissionGrantedNative
 * @tc.desc: Test function IsPermissionGranted is TOKEN_NATIVE or TOKEN_SHELL with permissions by default
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrUtilTest, PermissionIsPermissionGrantedNative, TestSize.Level0)
{
    MockAccesstokenKit::MockSetTokenTypeFlag(ATokenTypeEnum::TOKEN_NATIVE);
    EXPECT_TRUE(Permission::IsPermissionGranted("REBOOT"));

    MockAccesstokenKit::MockSetTokenTypeFlag(ATokenTypeEnum::TOKEN_SHELL);
    EXPECT_TRUE(Permission::IsPermissionGranted("REBOOT"));
}

/**
 * @tc.name: PermissionIsPermissionGrantedInvalid
 * @tc.desc: Test Permission function IsSystem is TOKEN_INVALID or TOKEN_TYPE_BUTT without permission
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrUtilTest, PermissionIsPermissionGrantedInvalid, TestSize.Level0)
{
    MockAccesstokenKit::MockSetTokenTypeFlag(ATokenTypeEnum::TOKEN_INVALID);
    EXPECT_FALSE(Permission::IsPermissionGranted("REBOOT"));
    MockAccesstokenKit::MockSetTokenTypeFlag(ATokenTypeEnum::TOKEN_TYPE_BUTT);
    EXPECT_FALSE(Permission::IsPermissionGranted("REBOOT"));
}
} // namespace
