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

#include <gtest/gtest.h>
#include <unistd.h>

#define private   public
#define protected public
#include "shutdown_dialog.h"
#undef private
#undef protected

#include "ability_manager_client.h"
#include "iremote_object.h"
#include "mock_power_remote_object.h"
#include "power_mgr_service.h"

using namespace std;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AAFwk;
using namespace OHOS::PowerMgr;

namespace OHOS {
namespace PowerMgr {
class PowerMgrPowerDialog : public testing::Test {
public:
    static void SetUpTestCase()
    {
        auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
        pms->OnStart();
    }
    static void TearDownTestCase()
    {
        auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
        pms->OnStop();
    }
};
} // namespace PowerMgr
} // namespace OHOS

namespace {
constexpr int32_t RESULT_CODE = -1;
constexpr int32_t SLEEP_WAIT_TIME_S = 1;

/**
 * @tc.name: LongPressKeyMonitorInitTest
 * @tc.desc: test KeyMonitorInit and KeyMonitorCancel
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrPowerDialog, LongPressKeyMonitorInitTest, TestSize.Level2)
{
    ShutdownDialog shutdownDialog;
    shutdownDialog.KeyMonitorInit();
    EXPECT_TRUE(shutdownDialog.longPressId_ >= OHOS::ERR_OK);
    shutdownDialog.KeyMonitorCancel();
    EXPECT_TRUE(shutdownDialog.longPressId_ == OHOS::ERR_OK);
}

/**
 * @tc.name: ConnectSystemUiDialogShowTest
 * @tc.desc: test power dialog has been show
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrPowerDialog, ConnectSystemUiDialogShowTest, TestSize.Level2)
{
    ShutdownDialog shutdownDialog;
    sptr<IRemoteObject> sptrRemoteObj = new MockPowerRemoteObject();
    MockPowerRemoteObject::SetRequestValue(ERR_OK);
    AppExecFwk::ElementName element;
    shutdownDialog.dialogConnectionCallback_->OnAbilityConnectDone(element, sptrRemoteObj, RESULT_CODE);
    sleep(SLEEP_WAIT_TIME_S);
    EXPECT_TRUE(shutdownDialog.ConnectSystemUi());
}

/**
 * @tc.name: OnAbilityConnectDoneTestFail
 * @tc.desc: test OnAbilityConnectDone SendRequest falied
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrPowerDialog, OnAbilityConnectDoneTestFail, TestSize.Level2)
{
    ShutdownDialog shutdownDialog;
    shutdownDialog.ResetLongPressFlag();
    AppExecFwk::ElementName element;
    sptr<IRemoteObject> sptrRemoteObj = new MockPowerRemoteObject();
    MockPowerRemoteObject::SetRequestValue(ERR_NO_INIT);
    shutdownDialog.dialogConnectionCallback_->OnAbilityConnectDone(element, sptrRemoteObj, RESULT_CODE);
    sleep(SLEEP_WAIT_TIME_S);
    EXPECT_FALSE(shutdownDialog.IsLongPress());
}

/**
 * @tc.name: OnAbilityConnectDoneTestRemoteNull
 * @tc.desc: test OnAbilityConnectDone remoteObj is null
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrPowerDialog, OnAbilityConnectDoneTestRemoteNull, TestSize.Level2)
{
    ShutdownDialog shutdownDialog;
    shutdownDialog.ResetLongPressFlag();
    AppExecFwk::ElementName element;
    sptr<IRemoteObject> sptrRemoteObj = nullptr;
    shutdownDialog.dialogConnectionCallback_->OnAbilityConnectDone(element, sptrRemoteObj, RESULT_CODE);
    sleep(SLEEP_WAIT_TIME_S);
    EXPECT_FALSE(shutdownDialog.IsLongPress());
}

/**
 * @tc.name: OnAbilityConnectDoneTest
 * @tc.desc: test OnAbilityConnectDone succ
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrPowerDialog, OnAbilityConnectDoneTest, TestSize.Level2)
{
    ShutdownDialog shutdownDialog;
    shutdownDialog.ResetLongPressFlag();
    AppExecFwk::ElementName element;
    sptr<IRemoteObject> sptrRemoteObj = new MockPowerRemoteObject();
    MockPowerRemoteObject::SetRequestValue(ERR_OK);
    shutdownDialog.dialogConnectionCallback_->OnAbilityConnectDone(element, sptrRemoteObj, RESULT_CODE);
    sleep(SLEEP_WAIT_TIME_S);
    EXPECT_TRUE(shutdownDialog.IsLongPress());
}

/**
 * @tc.name: OnAbilityDisconnectDoneTest
 * @tc.desc: test OnAbilityDisconnectDone succ
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrPowerDialog, OnAbilityDisconnectDoneTest, TestSize.Level2)
{
    ShutdownDialog shutdownDialog;
    AppExecFwk::ElementName element;
    shutdownDialog.dialogConnectionCallback_->OnAbilityDisconnectDone(element, RESULT_CODE);
    sleep(SLEEP_WAIT_TIME_S);
    EXPECT_FALSE(shutdownDialog.IsLongPress());
}
} // namespace
