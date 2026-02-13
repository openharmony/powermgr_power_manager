/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
    hibernateController_ = new HibernateController();
    g_callback = new PowerSyncHibernateTestCallback();
}

void HibernateControllerTest::TearDown()
{
    delete hibernateController_;
    hibernateController_ = nullptr;
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

    // 测试构造函数
    EXPECT_TRUE(hibernateController_ != nullptr);
}

/**
 * @tc.name: HibernateControllerTest002
 * @tc.desc: test register callback with null pointer
 * @tc.type: FUNC
 */
HWTEST_F(HibernateControllerTest, HibernateControllerTest002, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest002 function start!");

    // 测试空指针回调注册
    EXPECT_NO_THROW(hibernateController_->RegisterSyncHibernateCallback(nullptr));
}

/**
 * @tc.name: HibernateControllerTest003
 * @tc.desc: test unregister callback with null pointer
 * @tc.type: FUNC
 */
HWTEST_F(HibernateControllerTest, HibernateControllerTest003, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest003 function start!");

    // 测试空指针回调注销
    EXPECT_NO_THROW(hibernateController_->UnregisterSyncHibernateCallback(nullptr));
}

/**
 * @tc.name: HibernateControllerTest004
 * @tc.desc: test pre hibernate functionality
 * @tc.type: FUNC
 */
HWTEST_F(HibernateControllerTest, HibernateControllerTest004, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest004 function start!");

    // 测试PreHibernate功能
    EXPECT_NO_THROW(hibernateController_->PreHibernate());
}

/**
 * @tc.name: HibernateControllerTest005
 * @tc.desc: test post hibernate success functionality
 * @tc.type: FUNC
 */
HWTEST_F(HibernateControllerTest, HibernateControllerTest005, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest005 function start!");

    // 测试PostHibernate成功功能
    EXPECT_NO_THROW(hibernateController_->PostHibernate(true));
}

/**
 * @tc.name: HibernateControllerTest006
 * @tc.desc: test post hibernate failure functionality
 * @tc.type: FUNC
 */
HWTEST_F(HibernateControllerTest, HibernateControllerTest006, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest006 function start!");

    // 测试PostHibernate失败功能
    EXPECT_NO_THROW(hibernateController_->PostHibernate(false));
}

/**
 * @tc.name: HibernateControllerTest007
 * @tc.desc: test register and unregister callback sequence
 * @tc.type: FUNC
 */
HWTEST_F(HibernateControllerTest, HibernateControllerTest007, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest007 function start!");

    // 测试注册和注销序列
    EXPECT_NO_THROW(hibernateController_->RegisterSyncHibernateCallback(g_callback));
    EXPECT_NO_THROW(hibernateController_->UnregisterSyncHibernateCallback(g_callback));
}

/**
 * @tc.name: HibernateControllerTest008
 * @tc.desc: test pre and post hibernate sequence
 * @tc.type: FUNC
 */
HWTEST_F(HibernateControllerTest, HibernateControllerTest008, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest008 function start!");

    // 测试PreHibernate和PostHibernate序列
    EXPECT_NO_THROW(hibernateController_->PreHibernate());
    EXPECT_NO_THROW(hibernateController_->PostHibernate(true));
    EXPECT_NO_THROW(hibernateController_->PostHibernate(false));
}

/**
 * @tc.name: HibernateControllerTest009
 * @tc.desc: test pre and post hibernate sequence
 * @tc.type: FUNC
 */
HWTEST_F(HibernateControllerTest, HibernateControllerTest009, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest009 function start!");

    EXPECT_NO_THROW(hibernateController_->RegisterSyncHibernateCallback(g_callback));
    EXPECT_EQ(hibernateController_->callbacks_.size(), 1);
}

/**
 * @tc.name: HibernateControllerTest010
 * @tc.desc: test pre and post hibernate sequence
 * @tc.type: FUNC
 */
HWTEST_F(HibernateControllerTest, HibernateControllerTest010, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest010 function start!");

    // 测试UnregisterSyncHibernateCallback
    EXPECT_NO_THROW(hibernateController_->UnregisterSyncHibernateCallback(g_callback));
    EXPECT_EQ(hibernateController_->callbacks_.size(), 0);
}

} // namespace PowerMgr
} // namespace OHOS