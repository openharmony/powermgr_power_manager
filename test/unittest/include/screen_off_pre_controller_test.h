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

#ifndef SCREEN_OFF_PRE_CONTROLLER_TEST_H
#define SCREEN_OFF_PRE_CONTROLLER_TEST_H

#ifdef POWER_GTEST
#define private    public
#define protected  public
#endif

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include "power_log.h"

#include "screen_off_pre_controller.h"
#include "power_state_machine.h"

namespace OHOS {
namespace PowerMgr {
class ScreenOffPreControllerTest : public testing::Test {
public:
    void SetUp() override;
    void TearDown() override;
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}

    std::shared_ptr<PowerStateMachine> stateMachine_;
    std::unique_ptr<ScreenOffPreController> controller_;
};
} // namespace PowerMgr
} // namespace OHOS

#endif // SCREEN_OFF_PRE_CONTROLLER_TEST_H