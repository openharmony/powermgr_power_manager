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
    hibernateController_ = std::make_shared<HibernateController>();
    g_callback = new PowerSyncHibernateTestCallback();
}

void HibernateControllerTest::TearDown()
{
    hibernateController_ = nullptr;
}

void PowerSyncHibernateTestCallback::OnSyncHibernate()
{
    POWER_HILOGI(LABEL_TEST, "PowerSyncHibernateTestCallback::OnSyncHibernate.");
}

void PowerSyncHibernateTestCallback::OnSyncWakeup(bool hibernateResult)
{
    POWER_HILOGI(LABEL_TEST, "PowerSyncHibernateTestCallback::OnSyncWakeup, hibernateResult = %{public}d.",
        hibernateResult);
}

size_t GetTotalCallbackCount(HibernateController& ctrl)
{
    return ctrl.highPriorityCallbacks_.size() + ctrl.defaultPriorityCallbacks_.size()
        + ctrl.lowPriorityCallbacks_.size();
}

HWTEST_F(HibernateControllerTest, HibernateControllerTest001, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest001 start!");
    EXPECT_TRUE(hibernateController_ != nullptr);
}

HWTEST_F(HibernateControllerTest, HibernateControllerTest002, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest002 start!");
    size_t before = GetTotalCallbackCount(*hibernateController_);
    hibernateController_->RegisterSyncHibernateCallback(nullptr, HibernateCallbackPriority::DEFAULT);
    size_t after = GetTotalCallbackCount(*hibernateController_);
    EXPECT_EQ(after, before);
}

HWTEST_F(HibernateControllerTest, HibernateControllerTest003, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest003 start!");
    sptr<ISyncHibernateCallback> cb = new PowerSyncHibernateTestCallback();
    hibernateController_->RegisterSyncHibernateCallback(cb, HibernateCallbackPriority::DEFAULT);
    EXPECT_EQ(GetTotalCallbackCount(*hibernateController_), 1);
    hibernateController_->UnregisterSyncHibernateCallback(cb);
    EXPECT_EQ(GetTotalCallbackCount(*hibernateController_), 0);
}

HWTEST_F(HibernateControllerTest, HibernateControllerTest004, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest004 start!");
    hibernateController_->PreHibernate();
    EXPECT_TRUE(hibernateController_->prepared_);
}

HWTEST_F(HibernateControllerTest, HibernateControllerTest005, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest005 start!");
    hibernateController_->PreHibernate();
    hibernateController_->PostHibernate(true);
    EXPECT_FALSE(hibernateController_->prepared_);
}

HWTEST_F(HibernateControllerTest, HibernateControllerTest006, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest006 start!");
    hibernateController_->PreHibernate();
    hibernateController_->PostHibernate(false);
    EXPECT_FALSE(hibernateController_->prepared_);
}

HWTEST_F(HibernateControllerTest, HibernateControllerTest007, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest007 start!");
    EXPECT_EQ(GetTotalCallbackCount(*hibernateController_), 0);
    hibernateController_->RegisterSyncHibernateCallback(g_callback, HibernateCallbackPriority::DEFAULT);
    EXPECT_EQ(GetTotalCallbackCount(*hibernateController_), 1);
    hibernateController_->UnregisterSyncHibernateCallback(g_callback);
    EXPECT_EQ(GetTotalCallbackCount(*hibernateController_), 0);
}

HWTEST_F(HibernateControllerTest, HibernateControllerTest008, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest008 start!");
    hibernateController_->PreHibernate();
    EXPECT_TRUE(hibernateController_->prepared_);
    hibernateController_->PostHibernate(true);
    EXPECT_FALSE(hibernateController_->prepared_);
    hibernateController_->PreHibernate();
    EXPECT_TRUE(hibernateController_->prepared_);
    hibernateController_->PostHibernate(false);
    EXPECT_FALSE(hibernateController_->prepared_);
}

HWTEST_F(HibernateControllerTest, HibernateControllerTest009, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest009 start!");
    hibernateController_->RegisterSyncHibernateCallback(g_callback, HibernateCallbackPriority::DEFAULT);
    EXPECT_EQ(hibernateController_->defaultPriorityCallbacks_.size(), 1);
    hibernateController_->UnregisterSyncHibernateCallback(g_callback);
    EXPECT_EQ(GetTotalCallbackCount(*hibernateController_), 0);
}

HWTEST_F(HibernateControllerTest, HibernateControllerTest010, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest010 start!");
    hibernateController_->UnregisterSyncHibernateCallback(g_callback);
    EXPECT_EQ(GetTotalCallbackCount(*hibernateController_), 0);
}

HWTEST_F(HibernateControllerTest, HibernateControllerTest011, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest011 start!");
    hibernateController_->PostHibernate(true);
    EXPECT_FALSE(hibernateController_->prepared_);
}

HWTEST_F(HibernateControllerTest, HibernateControllerTest012, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest012 start!");
    hibernateController_->RegisterSyncHibernateCallback(g_callback, HibernateCallbackPriority::DEFAULT);
    hibernateController_->RegisterSyncHibernateCallback(g_callback, HibernateCallbackPriority::HIGH);
    EXPECT_EQ(GetTotalCallbackCount(*hibernateController_), 2);
    EXPECT_EQ(hibernateController_->highPriorityCallbacks_.size(), 1);
    EXPECT_EQ(hibernateController_->defaultPriorityCallbacks_.size(), 1);
    hibernateController_->UnregisterSyncHibernateCallback(g_callback);
    EXPECT_EQ(GetTotalCallbackCount(*hibernateController_), 0);
}

HWTEST_F(HibernateControllerTest, HibernateControllerTest013, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest013 start!");
    hibernateController_->PreHibernate();
    EXPECT_TRUE(hibernateController_->prepared_);
}

HWTEST_F(HibernateControllerTest, HibernateControllerTest014, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest014 start!");
    hibernateController_->PreHibernate();
    EXPECT_TRUE(hibernateController_->prepared_);
    hibernateController_->PostHibernate(true);
    EXPECT_FALSE(hibernateController_->prepared_);
}

HWTEST_F(HibernateControllerTest, HibernateControllerTest015, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest015 start!");
    hibernateController_->RegisterSyncHibernateCallback(g_callback, HibernateCallbackPriority::DEFAULT);
    EXPECT_EQ(hibernateController_->cachedRegister_.size(), 1);
    hibernateController_->UnregisterSyncHibernateCallback(g_callback);
    EXPECT_EQ(hibernateController_->cachedRegister_.size(), 0);
}

HWTEST_F(HibernateControllerTest, HibernateControllerTest016, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest016 start!");
    hibernateController_->RegisterSyncHibernateCallback(g_callback, HibernateCallbackPriority::DEFAULT);
    EXPECT_EQ(GetTotalCallbackCount(*hibernateController_), 1);
    EXPECT_EQ(hibernateController_->cachedRegister_.size(), 1);
    wptr<IRemoteObject> remote = g_callback->AsObject();
    hibernateController_->deathRecipient_->OnRemoteDied(remote);
    EXPECT_EQ(GetTotalCallbackCount(*hibernateController_), 0);
    EXPECT_EQ(hibernateController_->cachedRegister_.size(), 0);
}

HWTEST_F(HibernateControllerTest, HibernateControllerTest017, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest017 start!");
    hibernateController_->RegisterSyncHibernateCallback(g_callback, HibernateCallbackPriority::DEFAULT);
    EXPECT_EQ(GetTotalCallbackCount(*hibernateController_), 1);
    wptr<IRemoteObject> remote = nullptr;
    hibernateController_->deathRecipient_->OnRemoteDied(remote);
    EXPECT_EQ(GetTotalCallbackCount(*hibernateController_), 1);
    hibernateController_->UnregisterSyncHibernateCallback(g_callback);
}

HWTEST_F(HibernateControllerTest, HibernateControllerTest018, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest018 start!");
    hibernateController_->RegisterSyncHibernateCallback(g_callback, HibernateCallbackPriority::DEFAULT);
    EXPECT_EQ(GetTotalCallbackCount(*hibernateController_), 1);
    hibernateController_->UnregisterSyncHibernateCallback(nullptr);
    EXPECT_EQ(GetTotalCallbackCount(*hibernateController_), 1);
    hibernateController_->UnregisterSyncHibernateCallback(g_callback);
    EXPECT_EQ(GetTotalCallbackCount(*hibernateController_), 0);
}

HWTEST_F(HibernateControllerTest, HibernateControllerTest019, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest019 start!");
    hibernateController_->RegisterSyncHibernateCallback(g_callback, HibernateCallbackPriority::DEFAULT);
    hibernateController_->PreHibernate();
    EXPECT_TRUE(hibernateController_->prepared_);
    hibernateController_->PostHibernate(true);
    EXPECT_FALSE(hibernateController_->prepared_);
    hibernateController_->UnregisterSyncHibernateCallback(g_callback);
}

HWTEST_F(HibernateControllerTest, HibernateControllerTest020, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest020 start!");
    sptr<ISyncHibernateCallback> cb1 = new PowerSyncHibernateTestCallback();
    sptr<ISyncHibernateCallback> cb2 = new PowerSyncHibernateTestCallback();
    hibernateController_->RegisterSyncHibernateCallback(cb1, HibernateCallbackPriority::DEFAULT);
    hibernateController_->RegisterSyncHibernateCallback(cb2, HibernateCallbackPriority::DEFAULT);
    EXPECT_EQ(GetTotalCallbackCount(*hibernateController_), 2);
    hibernateController_->PreHibernate();
    EXPECT_TRUE(hibernateController_->prepared_);
    hibernateController_->PostHibernate(false);
    EXPECT_FALSE(hibernateController_->prepared_);
    hibernateController_->UnregisterSyncHibernateCallback(cb1);
    hibernateController_->UnregisterSyncHibernateCallback(cb2);
    EXPECT_EQ(GetTotalCallbackCount(*hibernateController_), 0);
}

HWTEST_F(HibernateControllerTest, HibernateControllerTest021, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest021 start!");
    sptr<ISyncHibernateCallback> cbLow = new PowerSyncHibernateTestCallback();
    sptr<ISyncHibernateCallback> cbDefault = new PowerSyncHibernateTestCallback();
    sptr<ISyncHibernateCallback> cbHigh = new PowerSyncHibernateTestCallback();
    hibernateController_->RegisterSyncHibernateCallback(cbLow, HibernateCallbackPriority::LOW);
    hibernateController_->RegisterSyncHibernateCallback(cbDefault, HibernateCallbackPriority::DEFAULT);
    hibernateController_->RegisterSyncHibernateCallback(cbHigh, HibernateCallbackPriority::HIGH);
    EXPECT_EQ(hibernateController_->highPriorityCallbacks_.size(), 1);
    EXPECT_EQ(hibernateController_->defaultPriorityCallbacks_.size(), 1);
    EXPECT_EQ(hibernateController_->lowPriorityCallbacks_.size(), 1);
    EXPECT_EQ(GetTotalCallbackCount(*hibernateController_), 3);
    hibernateController_->UnregisterSyncHibernateCallback(cbLow);
    hibernateController_->UnregisterSyncHibernateCallback(cbDefault);
    hibernateController_->UnregisterSyncHibernateCallback(cbHigh);
    EXPECT_EQ(GetTotalCallbackCount(*hibernateController_), 0);
}

HWTEST_F(HibernateControllerTest, HibernateControllerTest022, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest022 start!");
    sptr<ISyncHibernateCallback> cbLow = new PowerSyncHibernateTestCallback();
    hibernateController_->RegisterSyncHibernateCallback(cbLow, HibernateCallbackPriority::LOW);
    EXPECT_EQ(hibernateController_->lowPriorityCallbacks_.size(), 1);
    hibernateController_->UnregisterSyncHibernateCallback(cbLow);
    EXPECT_EQ(GetTotalCallbackCount(*hibernateController_), 0);
}

HWTEST_F(HibernateControllerTest, HibernateControllerTest023, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest023 start!");
    sptr<ISyncHibernateCallback> cb = new PowerSyncHibernateTestCallback();
    hibernateController_->RegisterSyncHibernateCallback(cb, HibernateCallbackPriority::HIGH);
    EXPECT_EQ(hibernateController_->highPriorityCallbacks_.size(), 1);
    hibernateController_->UnregisterSyncHibernateCallback(cb);
    EXPECT_EQ(hibernateController_->highPriorityCallbacks_.size(), 0);
}

HWTEST_F(HibernateControllerTest, HibernateControllerTest024, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "HibernateControllerTest024 start!");
    hibernateController_->RegisterSyncHibernateCallback(g_callback, HibernateCallbackPriority::HIGH);
    hibernateController_->RegisterSyncHibernateCallback(g_callback, HibernateCallbackPriority::LOW);
    EXPECT_EQ(GetTotalCallbackCount(*hibernateController_), 2);
    EXPECT_EQ(hibernateController_->highPriorityCallbacks_.size(), 1);
    EXPECT_EQ(hibernateController_->defaultPriorityCallbacks_.size(), 0);
    EXPECT_EQ(hibernateController_->lowPriorityCallbacks_.size(), 1);
    hibernateController_->UnregisterSyncHibernateCallback(g_callback);
    EXPECT_EQ(GetTotalCallbackCount(*hibernateController_), 0);
}

} // namespace PowerMgr
} // namespace OHOS
