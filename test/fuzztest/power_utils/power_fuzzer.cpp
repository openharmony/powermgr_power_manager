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

/* This files contains faultlog fuzzer test modules. */

#include "power_fuzzer.h"

#include <cstddef>
#include <cstdlib>

#include "actions/idevice_power_action.h"
#include "actions/idevice_state_action.h"
#include "message_parcel.h"
#include "running_lock_action.h"
#include "securec.h"
#include "shutdown/shutdown_client.h"

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

class FuzzShutdownAction : public ShutdownController {
public:
    FuzzShutdownAction() = default;
    virtual ~FuzzShutdownAction() = default;
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
    virtual void SetCoordinated([[maybe_unused]] bool coordinated) {};
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
const int32_t REWIND_READ_DATA = 0;
} // namespace

PowerFuzzerTest::PowerFuzzerTest()
{
    service_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    service_->OnStart();
    auto powerAction = new FuzzPowerAction();
    auto powerState = new FuzzStateAction();
    auto shutdownState = new FuzzStateAction();
    auto lockAction = new RunningLockAction();
    auto shutdownAction = new FuzzShutdownAction();
    service_->EnableMock(powerState, shutdownState, powerAction, lockAction);
    service_->EnableShutdownMock(shutdownAction);
}

PowerFuzzerTest::~PowerFuzzerTest()
{
    if (service_ != nullptr) {
        service_->OnStop();
        service_->Reset();
    }
    service_ = nullptr;
}

void PowerFuzzerTest::TestPowerServiceStub(const uint32_t code, const uint8_t* data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(PowerMgrService::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(REWIND_READ_DATA);
    MessageParcel reply;
    MessageOption option;
    service_->OnRemoteRequest(code, datas, reply, option);
}
