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

#ifndef DEVICE_STATE_ACTION_NATIVE_TEST_H
#define DEVICE_STATE_ACTION_NATIVE_TEST_H

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
#include "power_state_callback_stub.h"
#include "display/device_state_action.h"

namespace OHOS {
namespace PowerMgr {
constexpr uint32_t DISPLAYID = 3;
constexpr uint32_t DISPLAYID_A = 0;
constexpr uint32_t UNDISPLAYSTATE = 7;

class DeviceStateActionNativeTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};
} // namespace PowerMgr
} // namespace OHOS
#endif // DEVICE_STATE_ACTION_NATIVE_TEST_H
