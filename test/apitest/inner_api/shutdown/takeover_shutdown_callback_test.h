/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef POWERMGR_POWER_MANAGER_TAKEOVER_SHUTDOWN_CALLBACK_TEST_H
#define POWERMGR_POWER_MANAGER_TAKEOVER_SHUTDOWN_CALLBACK_TEST_H

#include <gtest/gtest.h>

#include "shutdown/takeover_shutdown_callback_stub.h"

namespace OHOS {
namespace PowerMgr {
namespace UnitTest {
class TakeOverShutdownCallbackTest : public testing::Test {
public:
    class TakeOverShutdownCallback : public TakeOverShutdownCallbackStub {
    public:
        ~TakeOverShutdownCallback() override = default;
        bool OnTakeOverShutdown(bool isReboot) override;
    };

    class HighPriorityTakeOverShutdownCallback : public TakeOverShutdownCallbackStub {
    public:
        ~HighPriorityTakeOverShutdownCallback() override = default;
        bool OnTakeOverShutdown(bool isReboot) override;
    };

    class LowPriorityTakeOverShutdownCallback : public TakeOverShutdownCallbackStub {
    public:
        ~LowPriorityTakeOverShutdownCallback() override = default;
        bool OnTakeOverShutdown(bool isReboot) override;
    };

    class NotTakeOverShutdownCallback : public TakeOverShutdownCallbackStub {
        ~NotTakeOverShutdownCallback() override = default;
        bool OnTakeOverShutdown(bool isReboot) override;
    };

protected:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};
} // namespace UnitTest
} // namespace PowerMgr
} // namespace OHOS

#endif // POWERMGR_POWER_MANAGER_TAKEOVER_SHUTDOWN_CALLBACK_TEST_H
