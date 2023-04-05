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

#ifndef POWERMGR_RUNNING_LOCK_HUB_NATIVE_TEST_H
#define POWERMGR_RUNNING_LOCK_HUB_NATIVE_TEST_H

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
#include "running_lock_hub.h"
#include "suspend_controller.h"

namespace OHOS {
namespace PowerMgr {
constexpr int32_t UNFD = -1;
constexpr int32_t FD = 1;

class RunningLockHubNativeTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_RUNNING_LOCK_HUB_NATIVE_TEST_H
