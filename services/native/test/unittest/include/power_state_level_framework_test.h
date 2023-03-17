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

#ifndef POWERMGR_STATE_LEVEL_FRAMEWORK_TEST_H
#define POWERMGR_STATE_LEVEL_FRAMEWORK_TEST_H

#include <gtest/gtest.h>

namespace OHOS {
namespace PowerMgr {
constexpr uint32_t AWAKE = 0;
constexpr uint32_t FREEZE = 1;
constexpr uint32_t INACTIVE = 2;
constexpr uint32_t STAND_BY = 3;
constexpr uint32_t DOZE = 4;
constexpr uint32_t SLEEP = 5;
constexpr uint32_t HIBERNATE = 6;
constexpr uint32_t SHUTDOWN = 7;

class PowerStateLevelFrameworkTest : public testing::Test {
public:
    static void SetUpTestCase();
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_STATE_LEVEL_FRAMEWORK_TEST_H