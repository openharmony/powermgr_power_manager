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

#include "ulsr_callback_proxy_test.h"

#include <memory>
#include "gtest/gtest.h"
#include "power_common.h"
#include "power_log.h"
#include "ulsr/ulsr_callback_holder.h"
#include "ulsr/ulsr_callback_ipc_interface_code.h"
#include "ulsr_callback_proxy.h"
#include "message_option.h"
#include "message_parcel.h"
#include "mock_power_remote_object.h"

namespace OHOS {
namespace PowerMgr {

using namespace testing::ext;

void UlsrCallbackProxyTest::SetUpTestCase() {}
void UlsrCallbackProxyTest::TearDownTestCase() {}
void UlsrCallbackProxyTest::SetUp() {}
void UlsrCallbackProxyTest::TearDown() {}

/**
 * @tc.name: UlsrCallbackProxyTest001
 * @tc.desc: Test UlsrCallbackProxy::OnSyncUlsr - WriteInterfaceToken failed branch
 * @tc.type: FUNC
 */
HWTEST_F(UlsrCallbackProxyTest, UlsrCallbackProxyTest001, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "UlsrCallbackProxyTest::UlsrCallbackProxyTest001 start!");

    sptr<MockPowerRemoteObject> mockRemote = new MockPowerRemoteObject();
    EXPECT_TRUE(mockRemote != nullptr);

    UlsrCallbackProxy proxy(mockRemote);

    // WriteInterfaceToken returns false when MOCK_WRITE_INTERFACE_TOKEN_RETURN_TRUE is not defined
    // This triggers the if (!data.WriteInterfaceToken(...)) branch
    proxy.OnSyncUlsr();

    POWER_HILOGI(LABEL_TEST, "UlsrCallbackProxyTest::UlsrCallbackProxyTest001 end!");
}

/**
 * @tc.name: UlsrCallbackProxyTest002
 * @tc.desc: Test UlsrCallbackProxy::OnAsyncWakeup - WriteInterfaceToken failed branch
 * @tc.type: FUNC
 */
HWTEST_F(UlsrCallbackProxyTest, UlsrCallbackProxyTest002, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "UlsrCallbackProxyTest::UlsrCallbackProxyTest002 start!");

    sptr<MockPowerRemoteObject> mockRemote = new MockPowerRemoteObject();
    EXPECT_TRUE(mockRemote != nullptr);

    UlsrCallbackProxy proxy(mockRemote);

    // WriteInterfaceToken returns false when MOCK_WRITE_INTERFACE_TOKEN_RETURN_TRUE is not defined
    // This triggers the if (!data.WriteInterfaceToken(...)) branch
    proxy.OnAsyncWakeup();

    POWER_HILOGI(LABEL_TEST, "UlsrCallbackProxyTest::UlsrCallbackProxyTest002 end!");
}

} // namespace PowerMgr
} // namespace OHOS