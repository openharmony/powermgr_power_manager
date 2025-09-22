/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include <gtest/gtest.h>
#include <ipc_object_proxy.h>
#include <ipc_skeleton.h>
#include <multi_invoker_helper/multi_invoker_helper.h>
#include <power_log.h>

namespace OHOS {
namespace PowerMgr {
using namespace testing;
using namespace ext;
class MultiInvokerHelperTest : public Test {
public:
    void SetUp() {}
    void TearDown() {}
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
};

namespace {
/**
 * @tc.name: MultiInvokerHelperTest001
 * @tc.desc: class unittest, cover abnormal branches
 * @tc.type: FUNC
 */
HWTEST_F(MultiInvokerHelperTest, MultiInvokerHelperTest001, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "MultiInvokerHelperTest001 function start!");
    constexpr size_t paramCount = 8;
    auto testHelper =
        sptr<MultiInvokerHelper>::MakeSptr(paramCount, 0, std::function<void(std::bitset<paramCount>)> {});
    sptr<IPCObjectProxy> testProxy = sptr<IPCObjectProxy>::MakeSptr(0, std::u16string {u"test"});
    sptr<IPCObjectProxy> testProxyInvalid = sptr<IPCObjectProxy>::MakeSptr(0, std::u16string {u"nottest"});
    constexpr pid_t testPid = 1;
    std::bitset<paramCount> input = 0b10101010;
    testHelper->Set(nullptr, testPid, -1, input);
    EXPECT_EQ(testHelper->GetResult(), 0);
    EXPECT_FALSE(testHelper->RemoveInvoker(testProxy->GetObjectDescriptor()));
    POWER_HILOGI(LABEL_TEST, "phase 1 dump: %{public}s", testHelper->Dump().c_str());

    testHelper->Set(testProxy, testPid, -1, input);
    EXPECT_EQ(testHelper->GetResult(), input);
    POWER_HILOGI(LABEL_TEST, "phase 2 dump: %{public}s", testHelper->Dump().c_str());

    testHelper->OnRemoteDied(nullptr);
    EXPECT_EQ(testHelper->GetResult(), input);
    POWER_HILOGI(LABEL_TEST, "phase 3 dump: %{public}s", testHelper->Dump().c_str());

    testHelper->OnRemoteDied(testProxyInvalid);
    EXPECT_EQ(testHelper->GetResult(), input);
    POWER_HILOGI(LABEL_TEST, "phase 4 dump: %{public}s", testHelper->Dump().c_str());

    testHelper->OnRemoteDied(testProxy);
    EXPECT_EQ(testHelper->GetResult(), 0);

    POWER_HILOGI(LABEL_TEST, "phase 5 dump: %{public}s", testHelper->Dump().c_str());
    POWER_HILOGI(LABEL_TEST, "MultiInvokerHelperTest001 function end!");
}
} // namespace
} // namespace PowerMgr
} // namespace OHOS
