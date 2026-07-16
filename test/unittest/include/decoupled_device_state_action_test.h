/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef DECOUPLED_DEVICE_STATE_ACTION_TEST_H
#define DECOUPLED_DEVICE_STATE_ACTION_TEST_H

#include <gtest/gtest.h>

namespace OHOS {
namespace PowerMgr {
// DecoupledDeviceStateAction test fixture. Directly constructs DecoupledDeviceStateAction to test its display
// methods (same style as device_state_action_native_test: HDI-delegated paths are not covered).
class DecoupledDeviceStateActionTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};
} // namespace PowerMgr
} // namespace OHOS
#endif // DECOUPLED_DEVICE_STATE_ACTION_TEST_H
