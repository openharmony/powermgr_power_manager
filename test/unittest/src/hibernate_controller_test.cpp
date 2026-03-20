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

#include "hibernate_controller_test.h"

namespace OHOS {
namespace PowerMgr {

using namespace testing;
using namespace testing::ext;
using namespace std;

sptr<ISyncHibernateCallback> g_callback;

void HibernateControllerTest::SetUp()
{
    hibernateController_ = std::make_unique<HibernateController>();
    g_callback = new PowerSyncHibernateTestCallback();
}

void HibernateControllerTest::TearDown()
{
    hibernateController_.reset();
}

void PowerSyncHibernateTestCallback::OnSyncHibernate()
{
    POWER_HILOGI(LABEL_TEST, "PowerSyncHibernateTest1Callback::OnSyncHibernate.");
}

void PowerSyncHibernateTestCallback::OnSyncWakeup(bool hibernateResult)
{
    POWER_HILOGI(LABEL_TEST, "PowerSyncHibernateTest1Callback::OnSyncWakeup, hibernateResult = %{public}d.",
        hibernateResult);
}


/**
 * @tc.name: HibernateControllerTest001
 * @tc.desc: test hibernate controller constructor
 * @tc.type: FUNC
 */
HWTEST_F(HibernateControllerTest, HibernateControllerTest001, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest001 function start!");
    EXPECT_TRUE(hibernateController_ != nullptr);
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest001 function end!");
}

/**
 * @tc.name: HibernateControllerTest002
 * @tc.desc: test register callback with null pointer
 * @tc.type: FUNC
 */
HWTEST_F(HibernateControllerTest, HibernateControllerTest002, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest002 function start!");
    size_t beforeSize = hibernateController_->callbacks_.size();
    hibernateController_->RegisterSyncHibernateCallback(nullptr);
    size_t afterSize = hibernateController_->callbacks_.size();
    EXPECT_EQ(afterSize, beforeSize + 1);
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest002 function end!");
}

/**
 * @tc.name: HibernateControllerTest003
 * @tc.desc: test unregister callback with null pointer
 * @tc.type: FUNC
 */
HWTEST_F(HibernateControllerTest, HibernateControllerTest003, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest003 function start!");
    sptr<ISyncHibernateCallback> callback = new PowerSyncHibernateTestCallback();
    hibernateController_->RegisterSyncHibernateCallback(callback);
    size_t beforeSize = hibernateController_->callbacks_.size();
    EXPECT_EQ(beforeSize, 1);
    hibernateController_->UnregisterSyncHibernateCallback(callback);
    size_t afterSize = hibernateController_->callbacks_.size();
    EXPECT_EQ(afterSize, 0);
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest003 function end!");
}

/**
 * @tc.name: HibernateControllerTest004
 * @tc.desc: test pre hibernate functionality
 * @tc.type: FUNC
 */
HWTEST_F(HibernateControllerTest, HibernateControllerTest004, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest004 function start!");
    hibernateController_->PreHibernate();
    EXPECT_TRUE(hibernateController_->prepared_);
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest004 function end!");
}

/**
 * @tc.name: HibernateControllerTest005
 * @tc.desc: test post hibernate success functionality
 * @tc.type: FUNC
 */
HWTEST_F(HibernateControllerTest, HibernateControllerTest005, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest005 function start!");
    hibernateController_->PreHibernate();
    hibernateController_->PostHibernate(true);
    EXPECT_FALSE(hibernateController_->prepared_);
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest005 function end!");
}

/**
 * @tc.name: HibernateControllerTest006
 * @tc.desc: test post hibernate failure functionality
 * @tc.type: FUNC
 */
HWTEST_F(HibernateControllerTest, HibernateControllerTest006, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest006 function start!");
    hibernateController_->PreHibernate();
    hibernateController_->PostHibernate(false);
    EXPECT_FALSE(hibernateController_->prepared_);
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest006 function end!");
}

/**
 * @tc.name: HibernateControllerTest007
 * @tc.desc: test register and unregister callback sequence
 * @tc.type: FUNC
 */
HWTEST_F(HibernateControllerTest, HibernateControllerTest007, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest007 function start!");
    EXPECT_EQ(hibernateController_->callbacks_.size(), 0);
    hibernateController_->RegisterSyncHibernateCallback(g_callback);
    EXPECT_EQ(hibernateController_->callbacks_.size(), 1);
    hibernateController_->UnregisterSyncHibernateCallback(g_callback);
    EXPECT_EQ(hibernateController_->callbacks_.size(), 0);
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest007 function end!");
}

/**
 * @tc.name: HibernateControllerTest008
 * @tc.desc: test pre and post hibernate sequence
 * @tc.type: FUNC
 */
HWTEST_F(HibernateControllerTest, HibernateControllerTest008, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest008 function start!");
    hibernateController_->PreHibernate();
    EXPECT_TRUE(hibernateController_->prepared_);
    hibernateController_->PostHibernate(true);
    EXPECT_FALSE(hibernateController_->prepared_);
    hibernateController_->PreHibernate();
    EXPECT_TRUE(hibernateController_->prepared_);
    hibernateController_->PostHibernate(false);
    EXPECT_FALSE(hibernateController_->prepared_);
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest008 function end!");
}

/**
 * @tc.name: HibernateControllerTest009
 * @tc.desc: test pre and post hibernate sequence
 * @tc.type: FUNC
 */
HWTEST_F(HibernateControllerTest, HibernateControllerTest009, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest009 function start!");
    hibernateController_->RegisterSyncHibernateCallback(g_callback);
    EXPECT_EQ(hibernateController_->callbacks_.size(), 1);
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest009 function end!");
}

/**
 * @tc.name: HibernateControllerTest010
 * @tc.desc: test pre and post hibernate sequence
 * @tc.type: FUNC
 */
HWTEST_F(HibernateControllerTest, HibernateControllerTest010, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest010 function start!");
    hibernateController_->UnregisterSyncHibernateCallback(g_callback);
    EXPECT_EQ(hibernateController_->callbacks_.size(), 0);
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest010 function end!");
}

/**
 * @tc.name: HibernateControllerTest011
 * @tc.desc: test PostHibernate without PreHibernate
 * @tc.type: FUNC
 */
HWTEST_F(HibernateControllerTest, HibernateControllerTest011, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest011 function start!");
    hibernateController_->PostHibernate(true);
    EXPECT_FALSE(hibernateController_->prepared_);
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest011 function end!");
}

/**
 * @tc.name: HibernateControllerTest012
 * @tc.desc: test multiple register same callback
 * @tc.type: FUNC
 */
HWTEST_F(HibernateControllerTest, HibernateControllerTest012, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest012 function start!");
    hibernateController_->RegisterSyncHibernateCallback(g_callback);
    hibernateController_->RegisterSyncHibernateCallback(g_callback);
    EXPECT_EQ(hibernateController_->callbacks_.size(), 1);
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest012 function end!");
}

/**
 * @tc.name: HibernateControllerTest013
 * @tc.desc: test PreHibernate sets prepared_ to true
 * @tc.type: FUNC
 */
HWTEST_F(HibernateControllerTest, HibernateControllerTest013, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest013 function start!");
    hibernateController_->PreHibernate();
    EXPECT_TRUE(hibernateController_->prepared_);
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest013 function end!");
}

/**
 * @tc.name: HibernateControllerTest014
 * @tc.desc: test PreHibernate and PostHibernate reset prepared_
 * @tc.type: FUNC
 */
HWTEST_F(HibernateControllerTest, HibernateControllerTest014, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest014 function start!");
    hibernateController_->PreHibernate();
    EXPECT_TRUE(hibernateController_->prepared_);
    hibernateController_->PostHibernate(true);
    EXPECT_FALSE(hibernateController_->prepared_);
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest014 function end!");
}

/**
 * @tc.name: HibernateControllerTest015
 * @tc.desc: test cachedRegister_ after register callback
 * @tc.type: FUNC
 */
HWTEST_F(HibernateControllerTest, HibernateControllerTest015, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest015 function start!");
    hibernateController_->RegisterSyncHibernateCallback(g_callback);
    EXPECT_EQ(hibernateController_->cachedRegister_.size(), 1);
    hibernateController_->UnregisterSyncHibernateCallback(g_callback);
    EXPECT_EQ(hibernateController_->cachedRegister_.size(), 0);
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest015 function end!");
}

} // namespace PowerMgr
} // namespace OHOS