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

/* This files contains faultlog fuzzer test modules. */

#include "power_fuzzer.h"

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <random>
#include <cstddef>
#include "securec.h"

#include "power_mgr_client.h"

using namespace OHOS::PowerMgr;
using namespace std;

namespace {
auto& g_powerMgrClient = PowerMgrClient::GetInstance();
constexpr int32_t SIZE = 1;
constexpr int32_t INDEX_0 = 0;
}

static void SuspendDevice(const uint8_t* data)
{
    int32_t type[1];
    int32_t idSize = 4;
    if ((memcpy_s(type, sizeof(type), data, idSize)) != EOK) {
        return;
    }

    g_powerMgrClient.SuspendDevice(static_cast<SuspendDeviceType>(type[0]));
}

static void WakeupDevice(const uint8_t* data)
{
    int32_t type[1];
    int32_t idSize = 4;
    if ((memcpy_s(type, sizeof(type), data, idSize)) != EOK) {
        return;
    }

    g_powerMgrClient.WakeupDevice(static_cast<WakeupDeviceType>(type[0]));
}

static void RefreshActivity(const uint8_t* data)
{
    int32_t type[1];
    int32_t idSize = 4;
    if ((memcpy_s(type, sizeof(type), data, idSize)) != EOK) {
        return;
    }

    g_powerMgrClient.RefreshActivity(static_cast<UserActivityType>(type[0]));
}

static void IsScreenOn(const uint8_t* data)
{
    int32_t type[1];
    int32_t idSize = 4;
    if ((memcpy_s(type, sizeof(type), data, idSize)) != EOK) {
        return;
    }

    g_powerMgrClient.IsScreenOn();
}

static void GetState(const uint8_t* data)
{
    int32_t type[1];
    int32_t idSize = 4;
    if ((memcpy_s(type, sizeof(type), data, idSize)) != EOK) {
        return;
    }

    g_powerMgrClient.GetState();
}

static void SetDeviceMode(const uint8_t* data)
{
    int32_t type[1];
    int32_t idSize = 4;
    if ((memcpy_s(type, sizeof(type), data, idSize)) != EOK) {
        return;
    }

    g_powerMgrClient.SetDeviceMode(static_cast<PowerMode>(type[0]));
}

static void GetDeviceMode(const uint8_t* data)
{
    int32_t type[1];
    int32_t idSize = 4;
    if ((memcpy_s(type, sizeof(type), data, idSize)) != EOK) {
        return;
    }

    g_powerMgrClient.GetDeviceMode();
}

static void CreateRunningLock(const uint8_t* data)
{
    int32_t type[1];
    int32_t idSize = 4;
    if ((memcpy_s(type, sizeof(type), data, idSize)) != EOK) {
        return;
    }

    g_powerMgrClient.CreateRunningLock("FUZZ", static_cast <RunningLockType>(type[0]));
}

static void RegisterPowerStateCallback(const uint8_t* data)
{
    static OHOS::sptr<IPowerStateCallback> callback;
    int32_t idSize = 4;
    if ((memcpy_s(callback, sizeof(callback), data, idSize)) != EOK) {
        return;
    }

    g_powerMgrClient.RegisterPowerStateCallback(callback);
    g_powerMgrClient.UnRegisterPowerStateCallback(callback);
}

static void RegisterPowerModeCallback(const uint8_t* data)
{
    static OHOS::sptr<IPowerModeCallback> callback;
    int32_t idSize = 4;
    if ((memcpy_s(callback, sizeof(callback), data, idSize)) != EOK) {
        return;
    }

    g_powerMgrClient.RegisterPowerModeCallback(callback);
    g_powerMgrClient.UnRegisterPowerModeCallback(callback);
}

static void RegisterShutdownCallback(const uint8_t* data)
{
    static OHOS::sptr<IShutdownCallback> callback;
    int32_t idSize = 4;
    if ((memcpy_s(callback, sizeof(callback), data, idSize)) != EOK) {
        return;
    }
    uint32_t intPriority[SIZE];
    if (memcpy_s(intPriority, sizeof(intPriority), data, idSize) != EOK) {
        return;
    }
    IShutdownCallback::ShutdownPriority priority =
        static_cast<IShutdownCallback::ShutdownPriority>(intPriority[INDEX_0]);
    g_powerMgrClient.RegisterShutdownCallback(callback, priority);
    g_powerMgrClient.UnRegisterShutdownCallback(callback);
}

static void Lock(const uint8_t* data)
{
    int32_t type[1];
    int32_t idSize = 4;
    if ((memcpy_s(type, sizeof(type), data, idSize)) != EOK) {
        return;
    }

    auto fuzzLock = g_powerMgrClient.CreateRunningLock("FUZZ", static_cast <RunningLockType>(type[0]));
    fuzzLock -> Lock();
}

static void UnLock(const uint8_t* data)
{
    int32_t type[1];
    int32_t idSize = 4;
    if ((memcpy_s(type, sizeof(type), data, idSize)) != EOK) {
        return;
    }

    auto fuzzLock = g_powerMgrClient.CreateRunningLock("FUZZ", static_cast <RunningLockType>(type[0]));
    fuzzLock -> Lock();
    fuzzLock -> UnLock();
}

static void OverrideScreenOffTime(const uint8_t* data)
{
    int64_t type[1];
    int32_t idSize = 4;
    if ((memcpy_s(type, sizeof(type), data, idSize)) != EOK) {
        return;
    }

    g_powerMgrClient.OverrideScreenOffTime(type[0]);
}

static void RestoreScreenOffTime(const uint8_t* data)
{
    int32_t type[1];
    int32_t idSize = 4;
    if ((memcpy_s(type, sizeof(type), data, idSize)) != EOK) {
        return;
    }

    g_powerMgrClient.RestoreScreenOffTime();
}

static void IsRunningLockTypeSupported(const uint8_t* data)
{
    uint32_t type[SIZE];
    int32_t idSize = 4;
    if ((memcpy_s(type, sizeof(type), data, idSize)) != EOK) {
        return;
    }

    g_powerMgrClient.IsRunningLockTypeSupported(type[INDEX_0]);
}

static void SetDisplaySuspend(const uint8_t* data)
{
    char type[SIZE];
    int32_t idSize = 1;
    if ((memcpy_s(type, sizeof(type), data, idSize)) != EOK) {
        return;
    }

    g_powerMgrClient.SetDisplaySuspend(static_cast<bool>(type[INDEX_0]));
}

static int32_t randNum()
{
    std::random_device rd;
    std::default_random_engine engine(rd());
    std::uniform_int_distribution<int32_t> randomNum(static_cast<int32_t>(ApiNumber::NUM_ZERO),
        static_cast<int32_t>(ApiNumber::NUM_END) - 1);
    return randomNum(engine);
}

namespace OHOS {
bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    int32_t idSize = 4;
    int32_t cond[1];
    if (static_cast<int32_t>(size) > idSize) {
        if ((memcpy_s(cond, sizeof(cond), data, idSize)) != EOK) {
            return false;
        }
        int32_t number = randNum();

        switch (static_cast<ApiNumber>(number)) {
            case ApiNumber::NUM_ZERO:
                SuspendDevice(data);
                break;
            case ApiNumber::NUM_ONE:
                WakeupDevice(data);
                break;
            case ApiNumber::NUM_TWO:
                RefreshActivity(data);
                break;
            case ApiNumber::NUM_THREE:
                IsScreenOn(data);
                break;
            case ApiNumber::NUM_FOUR:
                GetState(data);
                break;
            case ApiNumber::NUM_EIGHT:
                SetDeviceMode(data);
                break;
            case ApiNumber::NUM_NINE:
                GetDeviceMode(data);
                break;
            case ApiNumber::NUM_TEN:
                CreateRunningLock(data);
                break;
            case ApiNumber::NUM_ELEVEN:
                RegisterPowerStateCallback(data);
                break;
            case ApiNumber::NUM_THIRTEEN:
                RegisterPowerModeCallback(data);
                break;
            case ApiNumber::NUM_FIFTEEN:
                Lock(data);
                break;
            case ApiNumber::NUM_SIXTEEN:
                UnLock(data);
                break;
            case ApiNumber::NUM_SEVENTEEN:
                OverrideScreenOffTime(data);
                break;
            case ApiNumber::NUM_EIGHTEEN:
                RestoreScreenOffTime(data);
                break;
            case ApiNumber::NUM_NINETEEN:
                IsRunningLockTypeSupported(data);
                break;
            case ApiNumber::NUM_TWENTY:
                SetDisplaySuspend(data);
                break;
            case ApiNumber::NUM_TWENTY_ONE:
                RegisterShutdownCallback(data);
                break;
            default:
                break;
        }
    }

    return true;
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}

