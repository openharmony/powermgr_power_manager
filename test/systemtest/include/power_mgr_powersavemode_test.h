/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef POWERMGR_POWERSAVEMODE_TEST_H
#define POWERMGR_POWERSAVEMODE_TEST_H

#include <gtest/gtest.h>

#include "power_mgr_client.h"
#include "power_mgr_service.h"
#include "power_mode_callback_stub.h"
#include "common_event_subscriber.h"
#include "common_event_support.h"


namespace OHOS {
namespace PowerMgr {
constexpr int SLEEP_WAIT_TIME_S = 6;
constexpr int REFRESHACTIVITY_WAIT_TIME_S = 8;
constexpr int SCREEN_OFF_WAIT_TIME_S = 15;
constexpr int SET_DISPLAY_OFF_TIME_MS = 8000;

class PowerMgrPowerSavemodeTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    class PowerModeTest1Callback : public PowerModeCallbackStub {
    public:
        PowerModeTest1Callback() {};
        virtual ~PowerModeTest1Callback() {};
        virtual void OnPowerModeChanged(PowerMode mode) override;
    };
    class CommonEventServiCesSystemTest : public EventFwk::CommonEventSubscriber {
    public:
        CommonEventServiCesSystemTest(const EventFwk::CommonEventSubscribeInfo &subscriberInfo);
        virtual ~CommonEventServiCesSystemTest() {};
        virtual void OnReceiveEvent(const EventFwk::CommonEventData &data);
};
};
} // namespace PowerMgr
} // namespace OHOS
#endif
