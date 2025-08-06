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

#ifndef POWERMGR_SERVICE_FUZZ_TEST_H
#define POWERMGR_SERVICE_FUZZ_TEST_H

#include "power_mgr_service.h"
#include "power_mgr_async_reply_stub.h"
#include "screen_off_pre_callback_stub.h"
#include "sync_hibernate_callback_stub.h"
#include "sync_sleep_callback_stub.h"
#include "power_state_callback_stub.h"
#include "power_mode_callback_stub.h"
#include "power_runninglock_callback_stub.h"
#include "shutdown/async_shutdown_callback_stub.h"
#include "shutdown/sync_shutdown_callback_stub.h"
#include "shutdown/takeover_shutdown_callback_stub.h"

namespace OHOS {
namespace PowerMgr {
class PowerRunningLockTestCallback : public PowerRunningLockCallbackStub {
public:
    PowerRunningLockTestCallback() {};
    virtual ~PowerRunningLockTestCallback() {};
    virtual void HandleRunningLockMessage(std::string message) {};
};

class PowerStateTestCallback : public PowerStateCallbackStub {
public:
    PowerStateTestCallback() {};
    virtual ~PowerStateTestCallback() {};
    virtual void OnPowerStateChanged(PowerState state) {};
};

class PowerSyncSleepTest1Callback : public IRemoteStub<ISyncSleepCallback> {
public:
    PowerSyncSleepTest1Callback() {};
    virtual ~PowerSyncSleepTest1Callback() {};
    virtual void OnSyncSleep(bool onForceSleep) {};
    virtual void OnSyncWakeup(bool onForceSleep) {};
};

class PowerSyncHibernateTest1Callback : public IRemoteStub<ISyncHibernateCallback> {
public:
    PowerSyncHibernateTest1Callback() {};
    virtual ~PowerSyncHibernateTest1Callback() {};
    virtual void OnSyncHibernate() {};
    virtual void OnSyncWakeup(bool hibernateResult = false) {};
};

class PowerModeTestCallback : public PowerModeCallbackStub {
public:
    PowerModeTestCallback() {};
    virtual ~PowerModeTestCallback() {};
    virtual void OnPowerModeChanged(PowerMode mode) {};
};

class ScreenOffPreTestCallback : public IRemoteStub<IScreenOffPreCallback> {
public:
    ScreenOffPreTestCallback() {};
    virtual ~ScreenOffPreTestCallback() {};
    virtual void OnScreenStateChanged(uint32_t state) {};
};

class TakeOverShutdownCallback : public TakeOverShutdownCallbackStub {
public:
    ~TakeOverShutdownCallback() override = default;
    bool OnTakeOverShutdown(const TakeOverInfo& info) override;
    bool OnTakeOverHibernate(const TakeOverInfo& info) override;
};

class AsyncShutdownCallback : public AsyncShutdownCallbackStub {
public:
    ~AsyncShutdownCallback() override = default;
    void OnAsyncShutdown() override;
    void OnAsyncShutdownOrReboot(bool isReboot) override;
};

class SyncShutdownCallback : public SyncShutdownCallbackStub {
public:
    ~SyncShutdownCallback() override = default;
    void OnSyncShutdown() override;
    void OnSyncShutdownOrReboot(bool isReboot) override;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_SERVICE_FUZZ_TEST_H