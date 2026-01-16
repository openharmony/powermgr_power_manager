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

#include "death_recipient_manager_test.h"

#include <iremote_stub.h>
#include <iremote_proxy.h>

using namespace testing::ext;

namespace OHOS {
namespace PowerMgr {
namespace {
int32_t g_count = 0;
std::function<void(const sptr<IRemoteObject>&)> g_callback = [] (const sptr<IRemoteObject>& token) {
    EXPECT_TRUE(token != nullptr && token->IsProxyObject());
    g_count++;
};
} // namespace

void DeathRecipientManagerTest::SetUp()
{
    savedInfo_ = {
        {g_callback, "func1", 0, 1},
        {g_callback, "func1", 2, 3},
        {g_callback, "func10", 4, 5},
        {g_callback, "func10", 6, 7},
        {g_callback, "func100", 8, 9},
        {g_callback, "func100", 0, 0},
    };
}

void DeathRecipientManagerTest::TearDown()
{
    DeathRecipientManager::GetInstance().clientDeathRecipientMap_.clear();
    savedInfo_.clear();
    g_count = 0;
}

/**
 * @tc.name: DeathRecipientManagerTest001
 * @tc.desc: Test DeathRecipientManager
 * @tc.type: FUNC
 */
HWTEST_F(DeathRecipientManagerTest, DeathRecipientManagerTest001, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "DeathRecipientManagerTest001 function start!");
    DeathRecipientManager& instance1 = DeathRecipientManager::GetInstance();
    DeathRecipientManager& instance2 = DeathRecipientManager::GetInstance();
    EXPECT_EQ(&instance1, &instance2);
    POWER_HILOGI(LABEL_TEST, "DeathRecipientManagerTest001 function end!");
}

/**
 * @tc.name: DeathRecipientManagerTest002
 * @tc.desc: Test DeathRecipientManager
 * @tc.type: FUNC
 */
HWTEST_F(DeathRecipientManagerTest, DeathRecipientManagerTest002, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "DeathRecipientManagerTest002 function start!");
    sptr<IRemoteObject> obj = sptr<IPCObjectProxy>::MakeSptr(0, u"DeathRecipientManagerTest002.test1");
    DeathRecipientManager& drm = DeathRecipientManager::GetInstance();
    std::for_each(savedInfo_.begin(), savedInfo_.end(), [&obj](const DeathRecipientManager::CBInfo& info) {
        DeathRecipientManager::GetInstance().AddDeathRecipient(obj, info);
    });
    EXPECT_TRUE(drm.clientDeathRecipientMap_.size() == 1 &&
                drm.clientDeathRecipientMap_.count(obj) == 1 &&
                drm.clientDeathRecipientMap_[obj].size() == 3); // size:3 funcName: func1, func10, func100
    DeathRecipientManager::GetInstance().RemoveDeathRecipient(obj);
    EXPECT_TRUE(drm.clientDeathRecipientMap_.size() == 0);
    POWER_HILOGI(LABEL_TEST, "DeathRecipientManagerTest002 function end!");
}

/**
 * @tc.name: DeathRecipientManagerTest003
 * @tc.desc: Test DeathRecipientManager
 * @tc.type: FUNC
 */
HWTEST_F(DeathRecipientManagerTest, DeathRecipientManagerTest003, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "DeathRecipientManagerTest003 function start!");
    sptr<IRemoteObject> obj = sptr<IPCObjectProxy>::MakeSptr(0, u"DeathRecipientManagerTest003.test1");
    DeathRecipientManager& drm = DeathRecipientManager::GetInstance();
    sptr<TestDeathRecipientObject> drt = sptr<TestDeathRecipientObject>::MakeSptr();
    DeathRecipientManager::GetInstance().AddDeathRecipient(obj, drt);
    EXPECT_TRUE(drm.clientDeathRecipientMap_.size() == 1 &&
                drm.clientDeathRecipientMap_.count(obj) == 1 &&
                drm.clientDeathRecipientMap_[obj].size() == 0); // empty CBInfo set
    DeathRecipientManager::GetInstance().AddDeathRecipient(obj, drt);
    EXPECT_TRUE(drm.clientDeathRecipientMap_.size() == 1); // test repeat insertion
    std::for_each(savedInfo_.begin(), savedInfo_.end(), [&obj](const DeathRecipientManager::CBInfo& info) {
        DeathRecipientManager::GetInstance().AddDeathRecipient(obj, info);
    });
    EXPECT_TRUE(drm.clientDeathRecipientMap_.size() == 1 &&
                drm.clientDeathRecipientMap_.count(obj) == 1 &&
                drm.clientDeathRecipientMap_[obj].size() == 3); // size:3 funcName: func1, func10, func100
    wptr<IRemoteObject> objWeak = obj;
    drt->OnRemoteDied(objWeak);
    DeathRecipientManager::GetInstance().OnRemoteDied(objWeak);
    EXPECT_TRUE(drm.clientDeathRecipientMap_.size() == 0);
    EXPECT_TRUE(g_count == 3); // g_callback called three times
    POWER_HILOGI(LABEL_TEST, "DeathRecipientManagerTest003 function end!");
}

/**
 * @tc.name: DeathRecipientManagerTest004
 * @tc.desc: Test DeathRecipientManager
 * @tc.type: FUNC
 */
HWTEST_F(DeathRecipientManagerTest, DeathRecipientManagerTest004, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "DeathRecipientManagerTest004 function start!");
    sptr<IRemoteObject> obj1 = sptr<IPCObjectProxy>::MakeSptr(0, u"DeathRecipientManagerTest004.test1");
    DeathRecipientManager& drm = DeathRecipientManager::GetInstance();
    std::for_each(savedInfo_.begin(), savedInfo_.end(), [&obj1](const DeathRecipientManager::CBInfo& info) {
        DeathRecipientManager::GetInstance().AddDeathRecipient(obj1, info);
    });
    EXPECT_TRUE(drm.clientDeathRecipientMap_.size() == 1 &&
                drm.clientDeathRecipientMap_.count(obj1) == 1 &&
                drm.clientDeathRecipientMap_[obj1].size() == 3); // size:3 funcName: func1, func10, func100
    sptr<IRemoteObject> obj2 = sptr<IPCObjectProxy>::MakeSptr(0, u"DeathRecipientManagerTest004.test2");
    std::for_each(savedInfo_.begin(), savedInfo_.end(), [&obj2](const DeathRecipientManager::CBInfo& info) {
        DeathRecipientManager::GetInstance().AddDeathRecipient(obj2, info);
    });
    std::for_each(savedInfo_.begin(), savedInfo_.end(), [&obj2](const DeathRecipientManager::CBInfo& info) {
        DeathRecipientManager::GetInstance().AddDeathRecipient(obj2, info);
    });
    EXPECT_TRUE(drm.clientDeathRecipientMap_.size() == 2 &&
                drm.clientDeathRecipientMap_.count(obj1) == 1 &&
                drm.clientDeathRecipientMap_.count(obj2) == 1 &&
                drm.clientDeathRecipientMap_[obj2].size() == 3); // size:3 funcName: func1, func10, func100
    std::vector<std::string> checkVec {"func1", "func10", "func100"};
    auto it = drm.clientDeathRecipientMap_[obj2].begin();
    for (int i = 0; it != drm.clientDeathRecipientMap_[obj2].end() && i < 3; it++, i++) {
        EXPECT_EQ(it->funcName, checkVec[i]);
    }
    wptr<IRemoteObject> obj1Weak = obj1;
    wptr<IRemoteObject> obj2Weak = obj2;
    DeathRecipientManager::GetInstance().OnRemoteDied(obj1Weak);
    DeathRecipientManager::GetInstance().OnRemoteDied(obj2Weak);
    EXPECT_TRUE(g_count == 6); // g_callback called six times
    POWER_HILOGI(LABEL_TEST, "DeathRecipientManagerTest004 function end!");
}

/**
 * @tc.name: DeathRecipientManagerTest005
 * @tc.desc: Test DeathRecipientManager
 * @tc.type: FUNC
 */
HWTEST_F(DeathRecipientManagerTest, DeathRecipientManagerTest005, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "DeathRecipientManagerTest005 function start!");
    DeathRecipientManager& drm = DeathRecipientManager::GetInstance();
    sptr<IRemoteObject> obj = sptr<IPCObjectProxy>::MakeSptr(0, u"DeathRecipientManagerTest005.test1");
    DeathRecipientManager::GetInstance().OnRemoteDied(nullptr);
    DeathRecipientManager::GetInstance().RemoveDeathRecipient(nullptr);
    DeathRecipientManager::GetInstance().AddDeathRecipient(obj, {nullptr, "", 0, 0});
    EXPECT_TRUE(drm.clientDeathRecipientMap_.size() == 1);
    DeathRecipientManager::GetInstance().RemoveDeathRecipient(obj);
    EXPECT_TRUE(drm.clientDeathRecipientMap_.size() == 0);
    POWER_HILOGI(LABEL_TEST, "DeathRecipientManagerTest005 function end!");
}
} // namespace PowerMgr
} // namespace OHOS