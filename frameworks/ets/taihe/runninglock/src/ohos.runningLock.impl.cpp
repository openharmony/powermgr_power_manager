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

#include <map>
#include "ohos.runningLock.proj.hpp"
#include "taihe/runtime.hpp"
#include "stdexcept"
#include "power_mgr_client.h"
#include "power_log.h"
#include "power_mgr_errors.h"
#include "power_errors.h"
#include "ohos.runningLock.impl.hpp"

using namespace taihe;
using namespace ohos::runningLock;
using namespace OHOS::PowerMgr;

namespace {
std::map<PowerErrors, std::string> errorTable = {
    {PowerErrors::ERR_CONNECTION_FAIL,   "Failed to connect to the service."},
    {PowerErrors::ERR_PERMISSION_DENIED, "Permission is denied"             },
    {PowerErrors::ERR_SYSTEM_API_DENIED, "System permission is denied"      },
    {PowerErrors::ERR_PARAM_INVALID,     "Invalid input parameter."         }
};

class RunningLockImpl {
public:
    RunningLockImpl() {}

    explicit RunningLockImpl(std::shared_ptr<OHOS::PowerMgr::RunningLock> runLock)
    {
        runningLock_ = runLock;
    }

    void Hold(int32_t timeout)
    {
        POWER_HILOGD(FEATURE_RUNNING_LOCK, "ets Hold interface");
        if (runningLock_ == nullptr) {
            POWER_HILOGE(FEATURE_RUNNING_LOCK, "runningLock_ is nullptr");
            return;
        }
        OHOS::ErrCode code = runningLock_->Lock(timeout);
        if (code == E_PERMISSION_DENIED) {
            taihe::set_business_error(static_cast<int32_t>(PowerErrors::ERR_PERMISSION_DENIED),
                errorTable[PowerErrors::ERR_PERMISSION_DENIED]);
        }
    }

    bool IsHolding()
    {
        POWER_HILOGD(FEATURE_RUNNING_LOCK, "ets IsHolding interface");
        if (runningLock_ == nullptr) {
            POWER_HILOGE(FEATURE_RUNNING_LOCK, "runningLock_ is nullptr");
            return false;
        }
        bool isUsed = runningLock_->IsUsed();
        return isUsed;
    }

    void Unhold()
    {
        POWER_HILOGD(FEATURE_RUNNING_LOCK, "ets Unhold interface");
        if (runningLock_ == nullptr) {
            POWER_HILOGE(FEATURE_RUNNING_LOCK, "runningLock_ is nullptr");
            return;
        }
        OHOS::ErrCode code = runningLock_->UnLock();
        if (code == E_PERMISSION_DENIED) {
            taihe::set_business_error(static_cast<int32_t>(PowerErrors::ERR_PERMISSION_DENIED),
                errorTable[PowerErrors::ERR_PERMISSION_DENIED]);
        }
    }

private:
    std::shared_ptr<OHOS::PowerMgr::RunningLock> runningLock_ = nullptr;
};

ohos::runningLock::RunningLock CreateSync(string_view name, ohos::runningLock::RunningLockType type)
{
    std::shared_ptr<OHOS::PowerMgr::RunningLock> runLock = nullptr;
    OHOS::PowerMgr::RunningLockType tp = static_cast<OHOS::PowerMgr::RunningLockType>(type.get_value());
    runLock = PowerMgrClient::GetInstance().CreateRunningLock(std::string(name), tp);
    PowerErrors code = PowerMgrClient::GetInstance().GetError();
    if (code != PowerErrors::ERR_OK && errorTable.find(code) != errorTable.end()) {
        taihe::set_business_error(static_cast<int32_t>(code), errorTable[code]);
    }
    return make_holder<RunningLockImpl, ohos::runningLock::RunningLock>(runLock);
}

bool IsSupported(ohos::runningLock::RunningLockType type)
{
    OHOS::PowerMgr::RunningLockType tp = static_cast<OHOS::PowerMgr::RunningLockType>(type.get_value());
    return tp == OHOS::PowerMgr::RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL;
}
}  // namespace

// Since these macros are auto-generate, lint will cause false positive
// NOLINTBEGIN
TH_EXPORT_CPP_API_CreateSync(CreateSync);
TH_EXPORT_CPP_API_IsSupported(IsSupported);
// NOLINTEND