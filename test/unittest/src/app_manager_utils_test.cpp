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

#include "app_manager_utils_test.h"

#include "app_manager_utils.h"
#include "power_log.h"

namespace OHOS {
namespace PowerMgr {
using namespace OHOS::AppExecFwk;
using namespace testing;
using namespace testing::ext;

/**
 * @tc.name: AppManagerUtilsTest001
 * @tc.desc: test GetForegroundApplications
 * @tc.type: FUNC
 */
HWTEST_F(AppManagerUtilsTest, AppManagerUtilsTest001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "AppManagerUtilsTest001 function start!");
    std::vector<AppStateData> testAppsData;
    AppManagerUtils::GetForegroundApplications(testAppsData);
    POWER_HILOGI(LABEL_TEST, "AppManagerUtilsTest001 function end!");
}

/**
 * @tc.name: AppManagerUtilsTest002
 * @tc.desc: test IsForegroundApplication
 * @tc.type: FUNC
 */
HWTEST_F(AppManagerUtilsTest, AppManagerUtilsTest002, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "AppManagerUtilsTest002 function start!");
    std::string unExistApp = "com.example.UNEXISTED_APP";
    bool res = AppManagerUtils::IsForegroundApplication(unExistApp);
    EXPECT_FALSE(res);
    POWER_HILOGI(LABEL_TEST, "AppManagerUtilsTest002 function end!");
}
} // PowerMgr
} // OHOS