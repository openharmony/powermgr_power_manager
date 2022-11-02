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

#ifndef POWERMGR_POWERMGR_ST_MOCK_TEST_H
#define POWERMGR_POWERMGR_ST_MOCK_TEST_H

#include <gtest/gtest.h>
#include <ipc_skeleton.h>

#include "mock_lock_action.h"
#include "mock_power_action.h"
#include "mock_state_action.h"
#include "power_common.h"
#include "power_mgr_service.h"

namespace OHOS {
namespace PowerMgr {
constexpr int NEXT_WAIT_TIME_S = 1;
constexpr int ASYNC_WAIT_TIME_S = 3;
constexpr int REFRESHACTIVITY_WAIT_TIME_S = 8;
constexpr int SCREEN_OFF_WAIT_TIME_S = (DEFAULT_DISPLAY_OFF_TIME / 1000);
constexpr int SCREEN_DIM_WAIT_TIME_S = (SCREEN_OFF_WAIT_TIME_S / 2);
constexpr int SLEEP_WAIT_TIME_S = (DEFAULT_SLEEP_TIME / 1000);
constexpr int SET_DISPLAY_OFF_TIME_MS = 8000;

class PowerMgrMockSystemTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};
} // namespace PowerMgr
} // namespace OHOS

#endif // POWERMGR_POWERMGR_MOCK_TEST_H
