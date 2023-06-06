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

#ifndef POWERMGR_POWERMGR_ST_SUSPEND_TEST_H
#define POWERMGR_POWERMGR_ST_SUSPEND_TEST_H

#include "power_common.h"
#include "power_mgr_service.h"
#include <gtest/gtest.h>

namespace OHOS {
namespace PowerMgr {
constexpr int SLEEP_WAIT_TIME_S = 5;
constexpr int SET_DISPLAY_OFF_TIME = 8;
constexpr int REFRESHACTIVITY_WAIT_TIME_S = 8;
constexpr int DOUBLE_TIMES = 2;
constexpr int TEST_RATE = 3;
constexpr int ONE_SECOND = 1;

class PowerMgrSTSuspendTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
};
} // namespace PowerMgr
} // namespace OHOS

#endif // POWERMGR_POWERMGR_MOCK_TEST_H