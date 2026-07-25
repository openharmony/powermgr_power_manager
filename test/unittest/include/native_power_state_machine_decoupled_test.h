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

#ifndef NATIVE_POWERMGR_STATE_MACHINE_DECOUPLED_TEST_H
#define NATIVE_POWERMGR_STATE_MACHINE_DECOUPLED_TEST_H

#ifdef POWER_GTEST
#define private    public
#define protected  public
#endif

#include <memory>

#include <gtest/gtest.h>
#include <display_manager_lite.h>

#include "power_mgr_client.h"
#include "power_mgr_service.h"

namespace OHOS {
namespace PowerMgr {
// Screen Decoupling power state machine white-box test fixture. Same style as native_power_state_machine_test:
// SetUpTestCase is empty; each case builds its own PowerStateMachine and injects a MockStateAction.
class NativePowerStateMachineDecoupledTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};
} // namespace PowerMgr
} // namespace OHOS
#endif // NATIVE_POWERMGR_STATE_MACHINE_DECOUPLED_TEST_H
