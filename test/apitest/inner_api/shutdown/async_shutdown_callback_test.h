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

#ifndef POWERMGR_POWER_MANAGER_ASYNC_SHUTDOWN_CALLBACK_TEST_H
#define POWERMGR_POWER_MANAGER_ASYNC_SHUTDOWN_CALLBACK_TEST_H

#include <gtest/gtest.h>

#include "shutdown/async_shutdown_callback_stub.h"

namespace OHOS {
namespace PowerMgr {
namespace UnitTest {
class AsyncShutdownCallbackTest : public testing::Test {
public:
    class AsyncShutdownCallback : public AsyncShutdownCallbackStub {
    public:
        ~AsyncShutdownCallback() override = default;
        void OnAsyncShutdown() override;
    };

    class HighPriorityAsyncShutdownCallback : public AsyncShutdownCallbackStub {
    public:
        ~HighPriorityAsyncShutdownCallback() override = default;
        void OnAsyncShutdown() override;
    };

    class LowPriorityAsyncShutdownCallback : public AsyncShutdownCallbackStub {
    public:
        ~LowPriorityAsyncShutdownCallback() override = default;
        void OnAsyncShutdown() override;
    };

    class NotAsyncShutdownCallback : public AsyncShutdownCallbackStub {
        ~NotAsyncShutdownCallback() override = default;
        void OnAsyncShutdown() override;
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

#endif // POWERMGR_POWER_MANAGER_ASYNC_SHUTDOWN_CALLBACK_TEST_H
