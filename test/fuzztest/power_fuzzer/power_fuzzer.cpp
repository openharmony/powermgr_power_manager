/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "power_fuzzer.h"

#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <random>

#include "actions/idevice_power_action.h"
#include "actions/idevice_state_action.h"
#include "message_parcel.h"
#include "power_mgr_client.h"
#include "power_mgr_service.h"
#include "power_state_machine_info.h"
#include "running_lock_action.h"
#include "securec.h"

using namespace OHOS;
using namespace OHOS::PowerMgr;
using namespace std;

namespace OHOS {
namespace PowerMgr {
class FuzzPowerAction : public IDevicePowerAction {
public:
    FuzzPowerAction() = default;
    virtual ~FuzzPowerAction() = default;
    virtual void Reboot([[maybe_unused]] const std::string& reason) {};
    virtual void Shutdown([[maybe_unused]] const std::string& reason) {};
};

class FuzzStateAction : public IDeviceStateAction {
public:
    FuzzStateAction() = default;
    virtual ~FuzzStateAction() = default;
    virtual void Suspend([[maybe_unused]] int64_t callTimeMs, [[maybe_unused]] SuspendDeviceType type,
        [[maybe_unused]] uint32_t flags) {};
    virtual void ForceSuspend() {};
    virtual void Wakeup([[maybe_unused]] int64_t callTimeMs, [[maybe_unused]] WakeupDeviceType type,
        [[maybe_unused]] const std::string& details, [[maybe_unused]] const std::string& pkgName) {};
    virtual void RefreshActivity([[maybe_unused]] int64_t callTimeMs, [[maybe_unused]] UserActivityType type,
        [[maybe_unused]] uint32_t flags) {};
    virtual DisplayState GetDisplayState()
    {
        return DisplayState::DISPLAY_OFF;
    };
    virtual uint32_t SetDisplayState([[maybe_unused]] DisplayState state,
        [[maybe_unused]] StateChangeReason reason = StateChangeReason::STATE_CHANGE_REASON_UNKNOWN)
    {
        return 0;
    }
    virtual uint32_t GoToSleep([[maybe_unused]] const std::function<void()> onSuspend,
        [[maybe_unused]] const std::function<void()> onWakeup, [[maybe_unused]] bool force)
    {
        return 0;
    }
    virtual void RegisterCallback([[maybe_unused]] std::function<void(uint32_t)>& callback) {};
};
} // namespace PowerMgr
} // namespace OHOS

namespace {
auto& g_powerMgrClient = PowerMgrClient::GetInstance();
sptr<PowerMgrService> g_service = nullptr;
const int32_t REWIND_READ_DATA = 0;
} // namespace

static int32_t GetInt32(const uint8_t* data, size_t size)
{
    int32_t value = 0;
    if (size < sizeof(value)) {
        return value;
    }
    if (memcpy_s(&value, sizeof(value), data, sizeof(value)) != EOK) {
        return value;
    }
    return value;
}

static void SuspendDevice(const uint8_t* data, size_t size)
{
    int32_t reason = GetInt32(data, size);
    g_powerMgrClient.SuspendDevice(static_cast<SuspendDeviceType>(reason), static_cast<bool>(reason));
}

static void WakeupDevice(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return;
    }
    int32_t reason = GetInt32(data, size);
    std::string detail(reinterpret_cast<const char*>(data), size);
    g_powerMgrClient.WakeupDevice(static_cast<WakeupDeviceType>(reason), detail);
}

static void RefreshActivity(const uint8_t* data, size_t size)
{
    int32_t type = GetInt32(data, size);
    g_powerMgrClient.RefreshActivity(static_cast<UserActivityType>(type));
}

static void IsScreenOn([[maybe_unused]] const uint8_t* data, [[maybe_unused]] size_t size)
{
    g_powerMgrClient.IsScreenOn();
}

static void GetState([[maybe_unused]] const uint8_t* data, [[maybe_unused]] size_t size)
{
    g_powerMgrClient.GetState();
}

static void SetDeviceMode(const uint8_t* data, size_t size)
{
    int32_t mode = GetInt32(data, size);
    g_powerMgrClient.SetDeviceMode(static_cast<PowerMode>(mode));
}

static void GetDeviceMode([[maybe_unused]] const uint8_t* data, [[maybe_unused]] size_t size)
{
    g_powerMgrClient.GetDeviceMode();
}

static void CreateRunningLock(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return;
    }
    int32_t type = GetInt32(data, size);
    std::string name(reinterpret_cast<const char*>(data), size);
    g_powerMgrClient.CreateRunningLock(name, static_cast<RunningLockType>(type));
}

static void RegisterPowerStateCallback(const uint8_t* data, size_t size)
{
    OHOS::sptr<IPowerStateCallback> callback;
    if ((memcpy_s(callback, sizeof(callback), data, sizeof(callback))) != EOK) {
        return;
    }
    g_powerMgrClient.RegisterPowerStateCallback(callback);
    g_powerMgrClient.UnRegisterPowerStateCallback(callback);
}

static void RegisterPowerModeCallback(const uint8_t* data, size_t size)
{
    OHOS::sptr<IPowerModeCallback> callback;
    if ((memcpy_s(callback, sizeof(callback), data, sizeof(callback))) != EOK) {
        return;
    }
    g_powerMgrClient.RegisterPowerModeCallback(callback);
    g_powerMgrClient.UnRegisterPowerModeCallback(callback);
}

static void RegisterShutdownCallback(const uint8_t* data, size_t size)
{
    OHOS::sptr<IShutdownCallback> callback;
    if ((memcpy_s(callback, sizeof(callback), data, sizeof(callback))) != EOK) {
        return;
    }
    int32_t type = GetInt32(data, size);
    IShutdownCallback::ShutdownPriority priority = static_cast<IShutdownCallback::ShutdownPriority>(type);
    g_powerMgrClient.RegisterShutdownCallback(callback, priority);
    g_powerMgrClient.UnRegisterShutdownCallback(callback);
}

static void ProxyRunningLock(const uint8_t* data, size_t size)
{
    int32_t type = GetInt32(data, size);
    bool isProxied = static_cast<bool>(type);
    pid_t pid = static_cast<pid_t>(type);
    pid_t uid = static_cast<pid_t>(type);
    g_powerMgrClient.ProxyRunningLock(isProxied, pid, uid);
}

static void Lock(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return;
    }
    int32_t type = GetInt32(data, size);
    std::string name(reinterpret_cast<const char*>(data), size);
    auto fuzzLock = g_powerMgrClient.CreateRunningLock(name, static_cast<RunningLockType>(type));
    if (fuzzLock == nullptr) {
        return;
    }
    int32_t timeout = GetInt32(data, size);
    fuzzLock->Lock(timeout);
}

static void UnLock(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return;
    }
    int32_t type = GetInt32(data, size);
    std::string name(reinterpret_cast<const char*>(data), size);
    auto fuzzLock = g_powerMgrClient.CreateRunningLock(name, static_cast<RunningLockType>(type));
    if (fuzzLock == nullptr) {
        return;
    }
    fuzzLock->UnLock();
}

static void OverrideScreenOffTime(const uint8_t* data, size_t size)
{
    int32_t timeout = GetInt32(data, size);
    g_powerMgrClient.OverrideScreenOffTime(timeout);
}

static void RestoreScreenOffTime([[maybe_unused]] const uint8_t* data, [[maybe_unused]] size_t size)
{
    g_powerMgrClient.RestoreScreenOffTime();
}

static void IsRunningLockTypeSupported(const uint8_t* data, size_t size)
{
    int32_t type = GetInt32(data, size);
    g_powerMgrClient.IsRunningLockTypeSupported(static_cast<RunningLockType>(type));
}

static void SetDisplaySuspend(const uint8_t* data, size_t size)
{
    int32_t suspend = GetInt32(data, size);
    g_powerMgrClient.SetDisplaySuspend(static_cast<bool>(suspend));
}

static void TestPowerServiceStub(const uint8_t* data, size_t size)
{
    uint32_t code;
    if (size < sizeof(code)) {
        return;
    }
    if (memcpy_s(&code, sizeof(code), data, sizeof(code)) != EOK) {
        return;
    }

    MessageParcel datas;
    datas.WriteInterfaceToken(PowerMgrService::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(REWIND_READ_DATA);
    MessageParcel reply;
    MessageOption option;
    if (g_service == nullptr) {
        g_service = DelayedSpSingleton<PowerMgrService>::GetInstance();
        g_service->OnStart();
        auto powerAction = new FuzzPowerAction();
        auto powerState = new FuzzStateAction();
        auto shutdownState = new FuzzStateAction();
        auto lockAction = new RunningLockAction();
        g_service->EnableMock(powerState, shutdownState, powerAction, lockAction);
    }
    g_service->OnRemoteRequest(code, datas, reply, option);
}

static std::vector<std::function<void(const uint8_t*, size_t)>> fuzzFunc = {
    &SuspendDevice,
    &WakeupDevice,
    &RefreshActivity,
    &IsScreenOn,
    &GetState,
    &SetDeviceMode,
    &GetDeviceMode,
    &CreateRunningLock,
    &RegisterPowerStateCallback,
    &RegisterPowerModeCallback,
    &RegisterShutdownCallback,
    &ProxyRunningLock,
    &Lock,
    &UnLock,
    &OverrideScreenOffTime,
    &RestoreScreenOffTime,
    &IsRunningLockTypeSupported,
    &SetDisplaySuspend
};

namespace OHOS {
bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    std::random_device rd;
    std::default_random_engine engine(rd());
    std::uniform_int_distribution<int32_t> randomNum(0, fuzzFunc.size() - 1);
    int32_t number = randomNum(engine);
    fuzzFunc[number](data, size);
    TestPowerServiceStub(data, size);
    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
