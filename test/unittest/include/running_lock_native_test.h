/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef POWERMGR_RUNNING_LOCK_NATIVE_TEST_H
#define POWERMGR_RUNNING_LOCK_NATIVE_TEST_H

#ifdef POWER_GTEST
#define private    public
#define protected  public
#endif

#include <map>
#include <memory>
#include <stdlib.h>

#include <gtest/gtest.h>

#include "power_mgr_client.h"
#include "power_mgr_service.h"
#include "running_lock_action.h"
#include "sensor_agent.h"
#include "power_mgr_factory.h"
#include "power_runninglock_callback_stub.h"

namespace OHOS {
namespace PowerMgr {
class RunningLockNativeTest : public testing::Test {};
class PowerRunningLockTestCallback : public PowerRunningLockCallbackStub {
public:
    PowerRunningLockTestCallback() {};
    virtual ~PowerRunningLockTestCallback() {};
    virtual void HandleRunningLockMessage(std::string message) override;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_RUNNING_LOCK_NATIVE_TEST_H
