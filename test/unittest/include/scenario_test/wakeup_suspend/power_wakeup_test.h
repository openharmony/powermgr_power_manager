/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef POWER_WAKEUP_TEST_H
#define POWER_WAKEUP_TEST_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "permission.h"
#include "power_mgr_service.h"
#include <mock_lock_action.h>
#include <mock_power_action.h>
#include <mock_state_action.h>

#ifdef POWERMGR_GTEST
#define private   public
#define protected public
#endif

namespace OHOS {
namespace PowerMgr {
class PowerWakeupTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp(void);
    void TearDown(void);
    testing::NiceMock<MockStateAction>* stateActionMock {nullptr};
    testing::NiceMock<MockStateAction>* shutdownStateActionMock {nullptr};
    testing::NiceMock<MockPowerAction>* powerActionMock {nullptr};
    testing::NiceMock<MockLockAction>* lockActionMock {nullptr};

    static inline sptr<PowerMgrService> stub_ {nullptr};
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWER_WAKEUP_TEST_H