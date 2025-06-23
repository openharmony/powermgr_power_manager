/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef POWERMGR_CLIENT_CALLBACK_TEST_H
#define POWERMGR_CLIENT_CALLBACK_TEST_H

#include <gtest/gtest.h>

#include "power_mgr_service.h"
#include "power_runninglock_callback_stub.h"
#include "screen_off_pre_callback_stub.h"
#include "sync_hibernate_callback_stub.h"
#include "sync_sleep_callback_stub.h"

namespace OHOS {
namespace PowerMgr {
class PowerMgrClientCallbackTest : public testing::Test {
public:

    class PowerRunningLockTest1Callback : public IRemoteStub<IPowerRunninglockCallback> {
    public:
        PowerRunningLockTest1Callback() {};
        virtual ~PowerRunningLockTest1Callback() {};
        virtual void HandleRunningLockMessage(std::string message) override;
    };

    class ScreenOffPreTest1Callback : public IRemoteStub<IScreenOffPreCallback> {
    public:
        ScreenOffPreTest1Callback() {};
        virtual ~ScreenOffPreTest1Callback() {};
        virtual void OnScreenStateChanged(uint32_t state) override;
    };

    class PowerSyncHibernateTest1Callback : public IRemoteStub<ISyncHibernateCallback> {
    public:
        PowerSyncHibernateTest1Callback() {};
        virtual ~PowerSyncHibernateTest1Callback() {};
        virtual void OnSyncHibernate() override;
        virtual void OnSyncWakeup(bool hibernateResult = false) override;
    };

    class PowerSyncSleepTest1Callback : public IRemoteStub<ISyncSleepCallback> {
    public:
        PowerSyncSleepTest1Callback() {};
        virtual ~PowerSyncSleepTest1Callback() {};
        virtual void OnSyncSleep(bool onForceSleep) override;
        virtual void OnSyncWakeup(bool onForceSleep) override;
    };
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_CLIENT_CALLBACK_TEST_H