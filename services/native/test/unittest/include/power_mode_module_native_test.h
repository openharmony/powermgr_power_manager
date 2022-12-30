/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef POWERMGR_MODE_MODULE_NATIVE_TEST_H
#define POWERMGR_MODE_MODULE_NATIVE_TEST_H

#ifdef POWER_GTEST
#define private    public
#define protected  public
#endif

#include <gtest/gtest.h>

#include "power_mgr_service.h"
#include "power_mode_policy.h"

namespace OHOS {
namespace PowerMgr {
constexpr uint32_t POWERMODE = 607;
constexpr int32_t GETI = 0;
constexpr uint32_t READODE = 600;
constexpr uint32_t MODEITEM = 7;

class PowerModeModuleNativeTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

class PowerModeTestCallback : public IRemoteStub<IPowerModeCallback> {
public:
    PowerModeTestCallback() {};
    virtual ~PowerModeTestCallback() {};
    virtual void OnPowerModeChanged(PowerMode mode) override;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_MODE_MODULE_NATIVE_TEST_H
