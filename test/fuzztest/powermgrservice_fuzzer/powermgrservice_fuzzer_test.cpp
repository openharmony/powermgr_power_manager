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

/* This files contains faultlog fuzzer test modules. */

#define FUZZ_PROJECT_NAME "powermgrservice_fuzzer"

#include "power_fuzzer.h"
#include "ipower_mgr.h"
#include "powermgrservice_fuzzer_test.h"

using namespace OHOS::PowerMgr;

namespace OHOS {
void AsyncShutdownCallback::OnAsyncShutdown()
{
    return;
}

void AsyncShutdownCallback::OnAsyncShutdownOrReboot(bool isReboot)
{
    return;
}

void SyncShutdownCallback::OnSyncShutdown()
{
    return;
}

void SyncShutdownCallback::OnSyncShutdownOrReboot(bool isReboot)
{
    return;
}

bool TakeOverShutdownCallback::OnTakeOverHibernate(const TakeOverInfo& info)
{
    return true;
}

bool TakeOverShutdownCallback::OnTakeOverShutdown(const TakeOverInfo& info)
{
    return true;
}


void PowerServiceFuzzTest()
{
    auto g_service = DelayedSpSingleton<PowerMgrService>::GetInstance();
    constexpr int32_t DISPLAY_POWER_MANAGER_ID = 3308;
    const std::string TEST_DEVICE_ID = "test_device_id";
    g_service->OnStart();

    bool clearMemory = false;
    bool isUsed = false;
    bool result = false;
    bool enabled = false;
    bool enabledLockScreen = false;
    bool checkLock = false;
    bool sendScreenOffEvent = false;
    int64_t callTimeMs  = 0;
    int32_t timeOutMs  = 0;
    int32_t powerError  = 0;
    int32_t lockType  = 0;
    int32_t reasonValue  = 0;
    const std::string apiVersion = "-1";
    sptr<PowerMgrStubAsync> asyncCallback = new PowerMgrStubAsync();
    sptr<IPowerMgrAsync> powerProxy = iface_cast<IPowerMgrAsync>(asyncCallback);
    const sptr<IRemoteObject> remoteObj = new RunningLockTokenStub();
    const sptr<IRemoteObject> token = new RunningLockTokenStub();
    const std::vector<int32_t> workSources;
    const RunningLockInfo runningLockInfo("fuzztest", RunningLockType::RUNNINGLOCK_SCREEN);

    g_service->WakeupDeviceAsyncIpc(callTimeMs, reasonValue, "", apiVersion);

    g_service->ForceSuspendDeviceIpc(callTimeMs, apiVersion, powerProxy);
    g_service->HibernateIpc(clearMemory, "", apiVersion, powerProxy);
    g_service->CreateRunningLockIpc(remoteObj, runningLockInfo, powerError);
    g_service->ReleaseRunningLockIpc(remoteObj, "");

    g_service->UpdateWorkSourceIpc(remoteObj, workSources);
    g_service->LockIpc(remoteObj, timeOutMs, powerError);
    g_service->UnLockIpc(remoteObj, "", powerError);

    g_service->IsUsedIpc(remoteObj, isUsed);

    g_service->SetForceTimingOutIpc(enabled, token, powerError);
    g_service->LockScreenAfterTimingOutIpc(
        enabledLockScreen, checkLock, sendScreenOffEvent, token, powerError);
    g_service->IsRunningLockEnabledIpc(lockType, result, powerError);
}

void PowerServiceCallbackFuzzTest()
{
    auto g_service = DelayedSpSingleton<PowerMgrService>::GetInstance();
    constexpr int32_t DISPLAY_POWER_MANAGER_ID = 3308;
    const std::string TEST_DEVICE_ID = "test_device_id";
    g_service->OnStart();

    bool isSync = false;
    int32_t remainTime  = 0;
    int32_t priorityValue  = 0;
    const sptr<ITakeOverShutdownCallback> takeOverShutdownCallback = new TakeOverShutdownCallback();
    const sptr<IAsyncShutdownCallback> asyncShutdownCallback = new AsyncShutdownCallback();
    const sptr<ISyncShutdownCallback> syncShutdownCallback = new SyncShutdownCallback();
    const sptr<IPowerRunninglockCallback> runningLockCallback = new PowerRunningLockTestCallback();
    sptr<IPowerStateCallback> stateCallback = new PowerStateTestCallback();
    const sptr<ISyncSleepCallback> sleepCallback = new PowerSyncSleepTest1Callback();
    const sptr<ISyncHibernateCallback> hibernateCallback = new PowerSyncHibernateTest1Callback();
    sptr<IPowerModeCallback> modeCallback = new PowerModeTestCallback();
    const sptr<IScreenOffPreCallback> screenOffPreCallback = new ScreenOffPreTestCallback();

    g_service->RegisterShutdownCallbackIpc(takeOverShutdownCallback, priorityValue);
    g_service->UnRegisterShutdownCallbackIpc(takeOverShutdownCallback);
    g_service->RegisterShutdownCallbackIpc(asyncShutdownCallback, priorityValue);
    g_service->UnRegisterShutdownCallbackIpc(asyncShutdownCallback);
    g_service->RegisterShutdownCallbackIpc(syncShutdownCallback, priorityValue);
    g_service->UnRegisterShutdownCallbackIpc(syncShutdownCallback);

    g_service->RegisterPowerStateCallbackIpc(stateCallback, isSync);
    g_service->UnRegisterPowerStateCallbackIpc(stateCallback);
    g_service->RegisterSyncSleepCallbackIpc(sleepCallback, priorityValue);
    g_service->UnRegisterSyncSleepCallbackIpc(sleepCallback);
    g_service->RegisterSyncHibernateCallbackIpc(hibernateCallback);
    g_service->UnRegisterSyncHibernateCallbackIpc(hibernateCallback);
    g_service->RegisterPowerModeCallbackIpc(modeCallback);
    g_service->UnRegisterPowerModeCallbackIpc(modeCallback);
    g_service->RegisterScreenStateCallbackIpc(remainTime, screenOffPreCallback);
    g_service->UnRegisterScreenStateCallbackIpc(screenOffPreCallback);

    g_service->RegisterRunningLockCallbackIpc(runningLockCallback);
    g_service->UnRegisterRunningLockCallbackIpc(runningLockCallback);
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::PowerServiceFuzzTest();
    OHOS::PowerServiceCallbackFuzzTest();
    PowerFuzzerTest g_serviceTest;
    g_serviceTest.TestPowerServiceStub(static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_SHELL_DUMP_IPC), data, size);
    return 0;
}

