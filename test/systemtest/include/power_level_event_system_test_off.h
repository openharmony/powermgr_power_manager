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

#ifndef POWERMGR_LEVEL_EVENT_SYSTEM_TEST_OFF_H
#define POWERMGR_LEVEL_EVENT_SYSTEM_TEST_OFF_H

#include <gtest/gtest.h>

#include "common_event_subscriber.h"
#include "common_event_support.h"
#include "power_mgr_client.h"
#include "power_mgr_service.h"
#include "power_mode_callback_stub.h"

namespace OHOS {
namespace PowerMgr {

class PowerLevelEventSystemTestOff : public testing::Test {
public:
    static void SetUpTestCase(void);
};
} // namespace PowerMgr
} // namespace OHOS
#endif
