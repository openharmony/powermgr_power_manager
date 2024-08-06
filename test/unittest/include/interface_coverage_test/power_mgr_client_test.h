/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef POWERMGR_POWERMGR_CLIENT_TEST_H
#define POWERMGR_POWERMGR_CLIENT_TEST_H

#include <gtest/gtest.h>
#include <ipc_skeleton.h>

#include "mock_lock_action.h"
#include "mock_power_action.h"
#include "mock_state_action.h"
#include "power_common.h"
#include "power_mgr_client.h"
#include "power_mgr_service.h"
#include "power_state_machine_info.h"

namespace OHOS {
namespace PowerMgr {
constexpr uint32_t NEXT_WAIT_TIME_S = 1;
constexpr uint32_t ASYNC_WAIT_TIME_S = 3;
constexpr uint32_t REFRESHACTIVITY_WAIT_TIME_S = 8;
constexpr uint32_t SCREEN_OFF_WAIT_TIME_MS = 2000;
constexpr uint32_t SCREEN_OFF_WAIT_TIME_S = 2;
constexpr uint32_t DIM_STATE_WAIT_TIME_MS = 300;
constexpr uint32_t WAIT_SUSPEND_TIME_MS = 2000;
constexpr uint32_t SLEEP_WAIT_TIME_S = 1;
constexpr uint32_t SLEEP_WAIT_TIME_MS = 100;
constexpr uint32_t SET_DISPLAY_OFF_TIME_MS = 1500;
constexpr uint32_t TRANSFER_NS_TO_MS = 1000000;
constexpr uint32_t TRANSFER_MS_TO_S = 1000;
constexpr float DIM_RATE = 0.8;
constexpr uint32_t DOUBLE_TIMES = 2;

class PowerMgrClientTest : public testing::Test {
};
} // namespace PowerMgr
} // namespace OHOS

#endif // POWERMGR_POWERMGR_CLIENT_TEST_H
