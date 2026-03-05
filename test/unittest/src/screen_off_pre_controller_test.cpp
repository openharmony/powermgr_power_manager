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

#include "screen_off_pre_controller_test.h"
#include "power_mgr_service.h"

namespace OHOS {
namespace PowerMgr {

using namespace testing;
using namespace testing::ext;
using namespace std;

void ScreenOffPreControllerTest::SetUp()
{
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    stateMachine_ = std::make_shared<PowerStateMachine>(pms);
    controller_ = new ScreenOffPreController(stateMachine_);
}

void ScreenOffPreControllerTest::TearDown()
{
    delete controller_;
    controller_ = nullptr;
    stateMachine_.reset();
}

/**
 * @tc.name: ScreenOffPreControllerTest001
 * @tc.desc: test constructor and IsRegistered
 * @tc.type: FUNC
 */
HWTEST_F(ScreenOffPreControllerTest, ScreenOffPreControllerTest001, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "ScreenOffPreControllerTest001 function start!");

    EXPECT_TRUE(controller_ != nullptr);
    EXPECT_FALSE(controller_->IsRegistered());
    POWER_HILOGI(LABEL_TEST, "ScreenOffPreControllerTest001 function end!");
}

/**
 * @tc.name: ScreenOffPreControllerTest002
 * @tc.desc: test Init function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenOffPreControllerTest, ScreenOffPreControllerTest002, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "ScreenOffPreControllerTest002 function start!");

    EXPECT_NO_THROW(controller_->Init());
    POWER_HILOGI(LABEL_TEST, "ScreenOffPreControllerTest002 function end!");
}

/**
 * @tc.name: ScreenOffPreControllerTest003
 * @tc.desc: test Reset function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenOffPreControllerTest, ScreenOffPreControllerTest003, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "ScreenOffPreControllerTest003 function start!");
    EXPECT_NO_THROW(controller_->Reset());
    POWER_HILOGI(LABEL_TEST, "ScreenOffPreControllerTest003 function end!");
}

/**
 * @tc.name: ScreenOffPreControllerTest004
 * @tc.desc: test AddScreenStateCallback with null callback
 * @tc.type: FUNC
 */
HWTEST_F(ScreenOffPreControllerTest, ScreenOffPreControllerTest004, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "ScreenOffPreControllerTest004 function start!");
    EXPECT_NO_THROW(controller_->AddScreenStateCallback(0, nullptr));
    POWER_HILOGI(LABEL_TEST, "ScreenOffPreControllerTest004 function end!");
}

/**
 * @tc.name: ScreenOffPreControllerTest005
 * @tc.desc: test DelScreenStateCallback with null callback
 * @tc.type: FUNC
 */
HWTEST_F(ScreenOffPreControllerTest, ScreenOffPreControllerTest005, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "ScreenOffPreControllerTest005 function start!");
    EXPECT_NO_THROW(controller_->DelScreenStateCallback(nullptr));
    POWER_HILOGI(LABEL_TEST, "ScreenOffPreControllerTest005 function end!");
}

/**
 * @tc.name: ScreenOffPreControllerTest006
 * @tc.desc: test SchedulEyeDetectTimeout with invalid time
 * @tc.type: FUNC
 */
HWTEST_F(ScreenOffPreControllerTest, ScreenOffPreControllerTest006, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "ScreenOffPreControllerTest006 function start!");
    EXPECT_NO_THROW(controller_->SchedulEyeDetectTimeout(-1, 0));
    POWER_HILOGI(LABEL_TEST, "ScreenOffPreControllerTest006 function end!");
}

/**
 * @tc.name: ScreenOffPreControllerTest007
 * @tc.desc: test NeedEyeDetectLocked with negative time
 * @tc.type: FUNC
 */
HWTEST_F(ScreenOffPreControllerTest, ScreenOffPreControllerTest007, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "ScreenOffPreControllerTest007 function start!");
    EXPECT_FALSE(controller_->NeedEyeDetectLocked(-1));
    POWER_HILOGI(LABEL_TEST, "ScreenOffPreControllerTest007 function end!");
}

/**
 * @tc.name: ScreenOffPreControllerTest008
 * @tc.desc: test NeedEyeDetectLocked with positive time
 * @tc.type: FUNC
 */
HWTEST_F(ScreenOffPreControllerTest, ScreenOffPreControllerTest008, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "ScreenOffPreControllerTest008 function start!");
    EXPECT_TRUE(controller_->NeedEyeDetectLocked(100));
    POWER_HILOGI(LABEL_TEST, "ScreenOffPreControllerTest008 function end!");
}

/**
 * @tc.name: ScreenOffPreControllerTest009
 * @tc.desc: test CancelEyeDetectTimeout function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenOffPreControllerTest, ScreenOffPreControllerTest009, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "ScreenOffPreControllerTest009 function start!");
    EXPECT_NO_THROW(controller_->CancelEyeDetectTimeout());
    POWER_HILOGI(LABEL_TEST, "ScreenOffPreControllerTest009 function end!");
}

/**
 * @tc.name: ScreenOffPreControllerTest010
 * @tc.desc: test TriggerCallback function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenOffPreControllerTest, ScreenOffPreControllerTest010, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "ScreenOffPreControllerTest010 function start!");
    EXPECT_NO_THROW(controller_->TriggerCallback());
    POWER_HILOGI(LABEL_TEST, "ScreenOffPreControllerTest010 function end!");
}

} // namespace PowerMgr
} // namespace OHOS