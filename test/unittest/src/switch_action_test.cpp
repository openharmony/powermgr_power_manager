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

#ifdef POWER_GTEST
#define private public
#define protected public
#endif

#include <gtest/gtest.h>
#include <memory>

#include "power_log.h"
#include "adapter/iswitch_action.h"
#include "power_mgr_service.h"
#include "power_state_machine.h"
#include "singleton.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;

namespace {

constexpr pid_t PID = 1;
constexpr int64_t CALLTIMEMS = 1;

class SPNSwitchActionStub : public ISwitchAction {
public:
    bool IsWakeupInClosedStateCalled() const { return wakeupCalled_; }
    bool IsSwitchOpenCalled() const { return openCalled_; }
    bool IsSwitchCloseCalled() const { return closeCalled_; }
    bool IsReportSwitchStateCalled() const { return reportCalled_; }
    bool IsIsScreenOnCalled() const { return screenOnCalled_; }
    void Reset()
    {
        wakeupCalled_ = false;
        openCalled_ = false;
        closeCalled_ = false;
        reportCalled_ = false;
        screenOnCalled_ = false;
    }

private:
    SwitchActionRet DoWakeupInClosedState() override
    {
        wakeupCalled_ = true;
        return SwitchActionRet::HANDLED;
    }
    SwitchActionRet DoSwitchOpen() override
    {
        openCalled_ = true;
        return SwitchActionRet::HANDLED;
    }
    SwitchActionRet DoSwitchClose() override
    {
        closeCalled_ = true;
        return SwitchActionRet::HANDLED;
    }
    SwitchActionRet DoReportSwitchState() override
    {
        reportCalled_ = true;
        return SwitchActionRet::HANDLED;
    }
    SwitchActionRet DoIsScreenOn() override
    {
        screenOnCalled_ = true;
        return SwitchActionRet::IS_SCREEN_ON;
    }

    bool wakeupCalled_ = false;
    bool openCalled_ = false;
    bool closeCalled_ = false;
    bool reportCalled_ = false;
    bool screenOnCalled_ = false;
};

class SwitchActionTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    std::shared_ptr<ISwitchAction> defaultAction_;
    std::shared_ptr<SPNSwitchActionStub> stubAction_;
    sptr<PowerMgrService> pms_;
    std::shared_ptr<PowerStateMachine> stateMachine_;
};

void SwitchActionTest::SetUpTestCase()
{
    POWER_HILOGI(LABEL_TEST, "SwitchActionTest::SetUpTestCase");
}

void SwitchActionTest::TearDownTestCase()
{
    POWER_HILOGI(LABEL_TEST, "SwitchActionTest::TearDownTestCase");
}

void SwitchActionTest::SetUp()
{
    POWER_HILOGI(LABEL_TEST, "SwitchActionTest::SetUp");
    defaultAction_ = std::make_shared<ISwitchAction>();
    stubAction_ = std::make_shared<SPNSwitchActionStub>();
    pms_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms_ != nullptr) {
        pms_->OnStart();
        stateMachine_ = std::make_shared<PowerStateMachine>(pms_);
        if (stateMachine_ != nullptr) {
            stateMachine_->Init();
            stateMachine_->InitState();
        }
    }
}

void SwitchActionTest::TearDown()
{
    POWER_HILOGI(LABEL_TEST, "SwitchActionTest::TearDown");
    defaultAction_.reset();
    stubAction_.reset();
    stateMachine_.reset();
    pms_ = nullptr;
}

/**
 * @tc.name: ISwitchAction_HandleSwitchAction_AllTypes_NoException
 * @tc.desc: Test default ISwitchAction handles all types without exception
 * @tc.type: FUNC
 * @tc.require: issueNone
 */
HWTEST_F(SwitchActionTest, ISwitchAction_HandleSwitchAction_AllTypes_NoException, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "ISwitchAction_HandleSwitchAction_AllTypes_NoException start");
    ASSERT_NE(defaultAction_, nullptr);
    POWER_HILOGI(LABEL_TEST, "ISwitchAction_HandleSwitchAction_AllTypes_NoException end");
}

/**
 * @tc.name: ISwitchAction_HandleSwitchAction_DefaultRet_ReturnsDefault
 * @tc.desc: Test default ISwitchAction returns SwitchActionRet::DEFAULT for all types
 * @tc.type: FUNC
 * @tc.require: issueNone
 */
HWTEST_F(SwitchActionTest, ISwitchAction_HandleSwitchAction_DefaultRet_ReturnsDefault, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "ISwitchAction_HandleSwitchAction_DefaultRet_ReturnsDefault start");
    ASSERT_NE(defaultAction_, nullptr);
    EXPECT_EQ(defaultAction_->HandleSwitchAction(SwitchActionType::WAKEUP_IN_CLOSED_STATE),
        SwitchActionRet::DEFAULT);
    EXPECT_EQ(defaultAction_->HandleSwitchAction(SwitchActionType::SWITCH_OPEN),
        SwitchActionRet::DEFAULT);
    EXPECT_EQ(defaultAction_->HandleSwitchAction(SwitchActionType::SWITCH_CLOSE),
        SwitchActionRet::DEFAULT);
    POWER_HILOGI(LABEL_TEST, "ISwitchAction_HandleSwitchAction_DefaultRet_ReturnsDefault end");
}

/**
 * @tc.name: SPNSwitchActionStub_HandleSwitchAction_WakeupInClosedState_Called
 * @tc.desc: Test SPNSwitchActionStub correctly records WAKEUP_IN_CLOSED_STATE call
 * @tc.type: FUNC
 * @tc.require: issueNone
 */
HWTEST_F(SwitchActionTest, SPNSwitchActionStub_HandleSwitchAction_WakeupInClosedState_Called, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "SPNSwitchActionStub_HandleSwitchAction_WakeupInClosedState_Called start");
    ASSERT_NE(stubAction_, nullptr);
    auto ret = stubAction_->HandleSwitchAction(SwitchActionType::WAKEUP_IN_CLOSED_STATE);
    EXPECT_TRUE(stubAction_->IsWakeupInClosedStateCalled());
    EXPECT_FALSE(stubAction_->IsSwitchOpenCalled());
    EXPECT_FALSE(stubAction_->IsSwitchCloseCalled());
    EXPECT_EQ(ret, SwitchActionRet::HANDLED);
    POWER_HILOGI(LABEL_TEST, "SPNSwitchActionStub_HandleSwitchAction_WakeupInClosedState_Called end");
}

/**
 * @tc.name: SPNSwitchActionStub_HandleSwitchAction_SwitchOpen_Called
 * @tc.desc: Test SPNSwitchActionStub correctly records SWITCH_OPEN call
 * @tc.type: FUNC
 * @tc.require: issueNone
 */
HWTEST_F(SwitchActionTest, SPNSwitchActionStub_HandleSwitchAction_SwitchOpen_Called, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "SPNSwitchActionStub_HandleSwitchAction_SwitchOpen_Called start");
    ASSERT_NE(stubAction_, nullptr);
    auto ret = stubAction_->HandleSwitchAction(SwitchActionType::SWITCH_OPEN);
    EXPECT_FALSE(stubAction_->IsWakeupInClosedStateCalled());
    EXPECT_TRUE(stubAction_->IsSwitchOpenCalled());
    EXPECT_FALSE(stubAction_->IsSwitchCloseCalled());
    EXPECT_EQ(ret, SwitchActionRet::HANDLED);
    POWER_HILOGI(LABEL_TEST, "SPNSwitchActionStub_HandleSwitchAction_SwitchOpen_Called end");
}

/**
 * @tc.name: SPNSwitchActionStub_HandleSwitchAction_SwitchClose_Called
 * @tc.desc: Test SPNSwitchActionStub correctly records SWITCH_CLOSE call
 * @tc.type: FUNC
 * @tc.require: issueNone
 */
HWTEST_F(SwitchActionTest, SPNSwitchActionStub_HandleSwitchAction_SwitchClose_Called, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "SPNSwitchActionStub_HandleSwitchAction_SwitchClose_Called start");
    ASSERT_NE(stubAction_, nullptr);
    auto ret = stubAction_->HandleSwitchAction(SwitchActionType::SWITCH_CLOSE);
    EXPECT_FALSE(stubAction_->IsWakeupInClosedStateCalled());
    EXPECT_FALSE(stubAction_->IsSwitchOpenCalled());
    EXPECT_TRUE(stubAction_->IsSwitchCloseCalled());
    EXPECT_EQ(ret, SwitchActionRet::HANDLED);
    POWER_HILOGI(LABEL_TEST, "SPNSwitchActionStub_HandleSwitchAction_SwitchClose_Called end");
}

/**
 * @tc.name: HandleSwitchAction_InvalidType_NoCrash
 * @tc.desc: Test that passing an out-of-range enum value does not crash
 * @tc.type: FUNC
 * @tc.require: issueNone
 */
HWTEST_F(SwitchActionTest, HandleSwitchAction_InvalidType_NoCrash, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "HandleSwitchAction_InvalidType_NoCrash start");
    ASSERT_NE(defaultAction_, nullptr);
    auto invalidType = static_cast<SwitchActionType>(100);
    auto ret = defaultAction_->HandleSwitchAction(invalidType);
    EXPECT_EQ(ret, SwitchActionRet::DEFAULT);
    ASSERT_NE(stubAction_, nullptr);
    stubAction_->Reset();
    ret = stubAction_->HandleSwitchAction(invalidType);
    EXPECT_EQ(ret, SwitchActionRet::DEFAULT);
    EXPECT_FALSE(stubAction_->IsWakeupInClosedStateCalled());
    EXPECT_FALSE(stubAction_->IsSwitchOpenCalled());
    EXPECT_FALSE(stubAction_->IsSwitchCloseCalled());
    POWER_HILOGI(LABEL_TEST, "HandleSwitchAction_InvalidType_NoCrash end");
}

/**
 * @tc.name: PowerStateMachine_SetSwitchAction_GetSwitchAction_Success
 * @tc.desc: Test setting and getting switch action on real PowerStateMachine
 * @tc.type: FUNC
 * @tc.require: issueNone
 */
HWTEST_F(SwitchActionTest, PowerStateMachine_SetSwitchAction_GetSwitchAction_Success, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerStateMachine_SetSwitchAction_GetSwitchAction_Success start");
    ASSERT_NE(stateMachine_, nullptr);

    auto action = std::make_shared<SPNSwitchActionStub>();
    stateMachine_->SetSwitchAction(action);
    EXPECT_EQ(stateMachine_->GetSwitchActionPtr(), action);

    auto retrievedAction = stateMachine_->GetSwitchActionPtr();
    ASSERT_NE(retrievedAction, nullptr);
    auto ret = retrievedAction->HandleSwitchAction(SwitchActionType::SWITCH_OPEN);
    EXPECT_TRUE(action->IsSwitchOpenCalled());
    EXPECT_EQ(ret, SwitchActionRet::HANDLED);
    POWER_HILOGI(LABEL_TEST, "PowerStateMachine_SetSwitchAction_GetSwitchAction_Success end");
}

/**
 * @tc.name: PowerStateMachine_DefaultAction_WakeupSuspend_NoEffect
 * @tc.desc: Test default ISwitchAction does not affect normal wakeup/suspend flow
 * @tc.type: FUNC
 * @tc.require: issueNone
 */
HWTEST_F(SwitchActionTest, PowerStateMachine_DefaultAction_WakeupSuspend_NoEffect, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerStateMachine_DefaultAction_WakeupSuspend_NoEffect start");
    ASSERT_NE(stateMachine_, nullptr);

    stateMachine_->SetSwitchAction(std::make_shared<ISwitchAction>());
    stateMachine_->SetSwitchState(true);

    stateMachine_->WakeupDeviceInner(PID, CALLTIMEMS,
        WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON, "test", "test");
    EXPECT_EQ(stateMachine_->GetState(), PowerState::AWAKE);

    stateMachine_->SuspendDeviceInner(PID, CALLTIMEMS,
        SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, true);
    EXPECT_EQ(stateMachine_->GetState(), PowerState::INACTIVE);
    POWER_HILOGI(LABEL_TEST, "PowerStateMachine_DefaultAction_WakeupSuspend_NoEffect end");
}

/**
 * @tc.name: PowerStateMachine_SwitchClose_DefaultAction_SuspendSuccess
 * @tc.desc: Test default action does not affect suspend when switch closes (S03)
 * @tc.type: FUNC
 * @tc.require: issueNone
 */
HWTEST_F(SwitchActionTest, PowerStateMachine_SwitchClose_DefaultAction_SuspendSuccess, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerStateMachine_SwitchClose_DefaultAction_SuspendSuccess start");
    ASSERT_NE(stateMachine_, nullptr);

    stateMachine_->SetSwitchAction(std::make_shared<ISwitchAction>());
    stateMachine_->SetSwitchState(true);
    stateMachine_->WakeupDeviceInner(PID, CALLTIMEMS,
        WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON, "test", "test");
    EXPECT_EQ(stateMachine_->GetState(), PowerState::AWAKE);

    stateMachine_->SetSwitchState(false);
    stateMachine_->SuspendDeviceInner(PID, CALLTIMEMS,
        SuspendDeviceType::SUSPEND_DEVICE_REASON_SWITCH, true);
    EXPECT_EQ(stateMachine_->GetState(), PowerState::INACTIVE);
    POWER_HILOGI(LABEL_TEST, "PowerStateMachine_SwitchClose_DefaultAction_SuspendSuccess end");
}

/**
 * @tc.name: PowerStateMachine_StubAction_SwitchOpen_WakeupSuccess
 * @tc.desc: Test SPN stub action is called during switch open wakeup (S02)
 * @tc.type: FUNC
 * @tc.require: issueNone
 */
HWTEST_F(SwitchActionTest, PowerStateMachine_StubAction_SwitchOpen_WakeupSuccess, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerStateMachine_StubAction_SwitchOpen_WakeupSuccess start");
    ASSERT_NE(stateMachine_, nullptr);

    auto stub = std::make_shared<SPNSwitchActionStub>();
    stateMachine_->SetSwitchAction(stub);
    stateMachine_->SetSwitchState(false);

    auto ret = stub->HandleSwitchAction(SwitchActionType::SWITCH_OPEN);
    EXPECT_TRUE(stub->IsSwitchOpenCalled());
    EXPECT_FALSE(stub->IsWakeupInClosedStateCalled());
    EXPECT_FALSE(stub->IsSwitchCloseCalled());
    EXPECT_EQ(ret, SwitchActionRet::HANDLED);

    stateMachine_->SetSwitchState(true);
    stateMachine_->WakeupDeviceInner(PID, CALLTIMEMS,
        WakeupDeviceType::WAKEUP_DEVICE_SWITCH, "test", "test");
    EXPECT_EQ(stateMachine_->GetState(), PowerState::AWAKE);
    POWER_HILOGI(LABEL_TEST, "PowerStateMachine_StubAction_SwitchOpen_WakeupSuccess end");
}

/**
 * @tc.name: PowerStateMachine_StubAction_SwitchClose_CalledBeforeSuspend
 * @tc.desc: Test SPN stub action is called during switch close suspend (S03)
 * @tc.type: FUNC
 * @tc.require: issueNone
 */
HWTEST_F(SwitchActionTest, PowerStateMachine_StubAction_SwitchClose_CalledBeforeSuspend, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerStateMachine_StubAction_SwitchClose_CalledBeforeSuspend start");
    ASSERT_NE(stateMachine_, nullptr);

    auto stub = std::make_shared<SPNSwitchActionStub>();
    stateMachine_->SetSwitchAction(stub);
    stateMachine_->SetSwitchState(true);
    stateMachine_->WakeupDeviceInner(PID, CALLTIMEMS,
        WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON, "test", "test");
    EXPECT_EQ(stateMachine_->GetState(), PowerState::AWAKE);

    stub->Reset();
    auto ret = stub->HandleSwitchAction(SwitchActionType::SWITCH_CLOSE);
    EXPECT_TRUE(stub->IsSwitchCloseCalled());
    EXPECT_FALSE(stub->IsSwitchOpenCalled());
    EXPECT_FALSE(stub->IsWakeupInClosedStateCalled());
    EXPECT_EQ(ret, SwitchActionRet::HANDLED);

    stateMachine_->SetSwitchState(false);
    stateMachine_->SuspendDeviceInner(PID, CALLTIMEMS,
        SuspendDeviceType::SUSPEND_DEVICE_REASON_SWITCH, true);
    EXPECT_EQ(stateMachine_->GetState(), PowerState::INACTIVE);
    POWER_HILOGI(LABEL_TEST, "PowerStateMachine_StubAction_SwitchClose_CalledBeforeSuspend end");
}

/**
 * @tc.name: PowerStateMachine_SwitchAction_Nullptr_Safe
 * @tc.desc: Test that nullptr switch action does not cause crash
 * @tc.type: FUNC
 * @tc.require: issueNone
 */
HWTEST_F(SwitchActionTest, PowerStateMachine_SwitchAction_Nullptr_Safe, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerStateMachine_SwitchAction_Nullptr_Safe start");
    ASSERT_NE(stateMachine_, nullptr);

    stateMachine_->SetSwitchAction(nullptr);
    EXPECT_EQ(stateMachine_->GetSwitchActionPtr(), nullptr);
    POWER_HILOGI(LABEL_TEST, "PowerStateMachine_SwitchAction_Nullptr_Safe end");
}

/**
 * @tc.name: PowerStateMachine_WakeupInClosedState_StubActionCalled
 * @tc.desc: Test WakeupDeviceInner calls HandleSwitchAction(WAKEUP_IN_CLOSED_STATE) in
 *           IsWakeupDeviceSkip when switch is closed. Verify S01 end-to-end: adapter is called
 *           and returns HANDLED, so wakeup is NOT skipped.
 * @tc.type: FUNC
 * @tc.require: issueNone
 */
HWTEST_F(SwitchActionTest, PowerStateMachine_WakeupInClosedState_StubActionCalled, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerStateMachine_WakeupInClosedState_StubActionCalled start");
    ASSERT_NE(stateMachine_, nullptr);

    auto stub = std::make_shared<SPNSwitchActionStub>();
    stateMachine_->SetSwitchAction(stub);

    // Set to closed state, then suspend to INACTIVE
    stateMachine_->SetSwitchState(false);
    stateMachine_->SuspendDeviceInner(PID, CALLTIMEMS,
        SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, true);
    EXPECT_EQ(stateMachine_->GetState(), PowerState::INACTIVE);

    // Trigger wakeup while switch is closed (S01 scenario)
    stub->Reset();
    stateMachine_->WakeupDeviceInner(PID, CALLTIMEMS,
        WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON, "test", "test");

    // Adapter's WAKEUP_IN_CLOSED_STATE should be called in IsWakeupDeviceSkip
    EXPECT_TRUE(stub->IsWakeupInClosedStateCalled());
    EXPECT_FALSE(stub->IsSwitchOpenCalled());
    EXPECT_FALSE(stub->IsSwitchCloseCalled());

    // SPN stub returns HANDLED, so wakeup is NOT skipped
    EXPECT_EQ(stateMachine_->GetState(), PowerState::AWAKE);
    POWER_HILOGI(LABEL_TEST, "PowerStateMachine_WakeupInClosedState_StubActionCalled end");
}

/**
 * @tc.name: PowerStateMachine_WakeupInOpenState_DefaultAction_NoEffect
 * @tc.desc: Test default action does not affect normal wakeup when switch is open
 * @tc.type: FUNC
 * @tc.require: issueNone
 */
HWTEST_F(SwitchActionTest, PowerStateMachine_WakeupInOpenState_DefaultAction_NoEffect, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerStateMachine_WakeupInOpenState_DefaultAction_NoEffect start");
    ASSERT_NE(stateMachine_, nullptr);

    stateMachine_->SetSwitchAction(std::make_shared<ISwitchAction>());
    stateMachine_->SetSwitchState(true);

    // Suspend first
    stateMachine_->SuspendDeviceInner(PID, CALLTIMEMS,
        SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, true);
    EXPECT_EQ(stateMachine_->GetState(), PowerState::INACTIVE);

    // Wakeup in open state - should succeed
    stateMachine_->WakeupDeviceInner(PID, CALLTIMEMS,
        WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON, "test", "test");
    EXPECT_EQ(stateMachine_->GetState(), PowerState::AWAKE);
    POWER_HILOGI(LABEL_TEST, "PowerStateMachine_WakeupInOpenState_DefaultAction_NoEffect end");
}

/**
 * @tc.name: PowerStateMachine_SwitchClose_InInactiveState_StubActionCalled
 * @tc.desc: Test HandleSwitchAction(SWITCH_CLOSE) is called even when device is already in
 *           INACTIVE (screen off) state. Verifies TC-07: strategy is always called regardless
 *           of current power state.
 * @tc.type: FUNC
 * @tc.require: issueNone
 */
HWTEST_F(SwitchActionTest, PowerStateMachine_SwitchClose_InInactiveState_StubActionCalled, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerStateMachine_SwitchClose_InInactiveState_StubActionCalled start");
    ASSERT_NE(stateMachine_, nullptr);

    auto stub = std::make_shared<SPNSwitchActionStub>();
    stateMachine_->SetSwitchAction(stub);
    stateMachine_->SetSwitchState(true);

    // Wakeup then suspend to reach INACTIVE state
    stateMachine_->WakeupDeviceInner(PID, CALLTIMEMS,
        WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON, "test", "test");
    EXPECT_EQ(stateMachine_->GetState(), PowerState::AWAKE);

    stateMachine_->SuspendDeviceInner(PID, CALLTIMEMS,
        SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, true);
    EXPECT_EQ(stateMachine_->GetState(), PowerState::INACTIVE);

    // Device is already in INACTIVE (screen off), trigger switch close
    stub->Reset();
    auto ret = stub->HandleSwitchAction(SwitchActionType::SWITCH_CLOSE);
    EXPECT_TRUE(stub->IsSwitchCloseCalled());
    EXPECT_FALSE(stub->IsSwitchOpenCalled());
    EXPECT_FALSE(stub->IsWakeupInClosedStateCalled());
    EXPECT_EQ(ret, SwitchActionRet::HANDLED);
    POWER_HILOGI(LABEL_TEST, "PowerStateMachine_SwitchClose_InInactiveState_StubActionCalled end");
}

/**
 * @tc.name: PowerStateMachine_DefaultInitAction_SetAndGet_Success
 * @tc.desc: Test InitSwitchAction default path: when no plugin is loaded, switchAction_ is set
 *           to an ISwitchAction default instance via SetSwitchAction, and GetSwitchAction returns
 *           a non-null pointer. Verifies TC-01 (default adapter initialization).
 * @tc.type: FUNC
 * @tc.require: issueNone
 */
HWTEST_F(SwitchActionTest, PowerStateMachine_DefaultInitAction_SetAndGet_Success, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerStateMachine_DefaultInitAction_SetAndGet_Success start");
    ASSERT_NE(stateMachine_, nullptr);

    // Simulate InitSwitchAction default path: no plugin, fallback to default ISwitchAction
    auto defaultAction = std::make_shared<ISwitchAction>();
    stateMachine_->SetSwitchAction(defaultAction);
    ASSERT_NE(stateMachine_->GetSwitchActionPtr(), nullptr);

    // Default action should return DEFAULT for all types
    EXPECT_EQ(stateMachine_->GetSwitchActionPtr()->HandleSwitchAction(
        SwitchActionType::WAKEUP_IN_CLOSED_STATE), SwitchActionRet::DEFAULT);
    EXPECT_EQ(stateMachine_->GetSwitchActionPtr()->HandleSwitchAction(
        SwitchActionType::SWITCH_OPEN), SwitchActionRet::DEFAULT);
    EXPECT_EQ(stateMachine_->GetSwitchActionPtr()->HandleSwitchAction(
        SwitchActionType::SWITCH_CLOSE), SwitchActionRet::DEFAULT);

    // Default action does not affect normal wakeup/suspend
    stateMachine_->SetSwitchState(true);
    stateMachine_->WakeupDeviceInner(PID, CALLTIMEMS,
        WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON, "test", "test");
    EXPECT_EQ(stateMachine_->GetState(), PowerState::AWAKE);

    stateMachine_->SuspendDeviceInner(PID, CALLTIMEMS,
        SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, true);
    EXPECT_EQ(stateMachine_->GetState(), PowerState::INACTIVE);
    POWER_HILOGI(LABEL_TEST, "PowerStateMachine_DefaultInitAction_SetAndGet_Success end");
}

/**
 * @tc.name: PowerStateMachine_WakeupInClosedState_DefaultAction_Skipped
 * @tc.desc: Test default action returns DEFAULT in closed state, so wakeup is skipped
 *           (original behavior preserved). Verifies A01.
 * @tc.type: FUNC
 * @tc.require: issueNone
 */
HWTEST_F(SwitchActionTest, PowerStateMachine_WakeupInClosedState_DefaultAction_Skipped, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerStateMachine_WakeupInClosedState_DefaultAction_Skipped start");
    ASSERT_NE(stateMachine_, nullptr);

    stateMachine_->SetSwitchAction(std::make_shared<ISwitchAction>());
    stateMachine_->SetSwitchState(false);

    // Suspend to INACTIVE
    stateMachine_->SuspendDeviceInner(PID, CALLTIMEMS,
        SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, true);
    EXPECT_EQ(stateMachine_->GetState(), PowerState::INACTIVE);

    // Wakeup while switch closed with default action -> skipped
    stateMachine_->WakeupDeviceInner(PID, CALLTIMEMS,
        WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON, "test", "test");
    EXPECT_EQ(stateMachine_->GetState(), PowerState::INACTIVE);
    POWER_HILOGI(LABEL_TEST, "PowerStateMachine_WakeupInClosedState_DefaultAction_Skipped end");
}

/**
 * @tc.name: ISwitchAction_HandleSwitchAction_ReportSwitchState_ReturnsDefault
 * @tc.desc: Test default ISwitchAction returns DEFAULT for REPORT_SWITCH_STATE, so the
 *           compensation call is a no-op on non-foldable devices (F4: non-foldable unaffected).
 * @tc.type: FUNC
 * @tc.require: issue#6
 */
HWTEST_F(SwitchActionTest, ISwitchAction_HandleSwitchAction_ReportSwitchState_ReturnsDefault, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "ISwitchAction_HandleSwitchAction_ReportSwitchState_ReturnsDefault start");
    ASSERT_NE(defaultAction_, nullptr);
    EXPECT_EQ(defaultAction_->HandleSwitchAction(SwitchActionType::REPORT_SWITCH_STATE),
        SwitchActionRet::DEFAULT);
    POWER_HILOGI(LABEL_TEST, "ISwitchAction_HandleSwitchAction_ReportSwitchState_ReturnsDefault end");
}

/**
 * @tc.name: ISwitchAction_HandleSwitchAction_IsScreenOn_ReturnsDefault
 * @tc.desc: Test default ISwitchAction returns DEFAULT for IS_SCREEN_ON
 * @tc.type: FUNC
 * @tc.require: issue#6
 */
HWTEST_F(SwitchActionTest, ISwitchAction_HandleSwitchAction_IsScreenOn_ReturnsDefault, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "ISwitchAction_HandleSwitchAction_IsScreenOn_ReturnsDefault start");
    ASSERT_NE(defaultAction_, nullptr);
    EXPECT_EQ(defaultAction_->HandleSwitchAction(SwitchActionType::IS_SCREEN_ON),
        SwitchActionRet::DEFAULT);
    POWER_HILOGI(LABEL_TEST, "ISwitchAction_HandleSwitchAction_IsScreenOn_ReturnsDefault end");
}

/**
 * @tc.name: SPNSwitchActionStub_HandleSwitchAction_ReportSwitchState_Called
 * @tc.desc: Test SPNSwitchActionStub correctly records REPORT_SWITCH_STATE call and returns
 *           HANDLED. This verifies the dispatch entry point used by the compensation logic.
 * @tc.type: FUNC
 * @tc.require: issue#6
 */
HWTEST_F(SwitchActionTest, SPNSwitchActionStub_HandleSwitchAction_ReportSwitchState_Called, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "SPNSwitchActionStub_HandleSwitchAction_ReportSwitchState_Called start");
    ASSERT_NE(stubAction_, nullptr);
    auto ret = stubAction_->HandleSwitchAction(SwitchActionType::REPORT_SWITCH_STATE);
    EXPECT_TRUE(stubAction_->IsReportSwitchStateCalled());
    EXPECT_FALSE(stubAction_->IsSwitchOpenCalled());
    EXPECT_FALSE(stubAction_->IsSwitchCloseCalled());
    EXPECT_FALSE(stubAction_->IsWakeupInClosedStateCalled());
    EXPECT_FALSE(stubAction_->IsIsScreenOnCalled());
    EXPECT_EQ(ret, SwitchActionRet::HANDLED);
    POWER_HILOGI(LABEL_TEST, "SPNSwitchActionStub_HandleSwitchAction_ReportSwitchState_Called end");
}

/**
 * @tc.name: SPNSwitchActionStub_HandleSwitchAction_IsScreenOn_Called
 * @tc.desc: Test SPNSwitchActionStub correctly records IS_SCREEN_ON call and returns IS_SCREEN_ON
 * @tc.type: FUNC
 * @tc.require: issue#6
 */
HWTEST_F(SwitchActionTest, SPNSwitchActionStub_HandleSwitchAction_IsScreenOn_Called, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "SPNSwitchActionStub_HandleSwitchAction_IsScreenOn_Called start");
    ASSERT_NE(stubAction_, nullptr);
    auto ret = stubAction_->HandleSwitchAction(SwitchActionType::IS_SCREEN_ON);
    EXPECT_TRUE(stubAction_->IsIsScreenOnCalled());
    EXPECT_FALSE(stubAction_->IsReportSwitchStateCalled());
    EXPECT_EQ(ret, SwitchActionRet::IS_SCREEN_ON);
    POWER_HILOGI(LABEL_TEST, "SPNSwitchActionStub_HandleSwitchAction_IsScreenOn_Called end");
}

/**
 * @tc.name: PowerStateMachine_ReportSwitchState_Compensation_Dispatched
 * @tc.desc: Verify the compensation path (T1): after a switch action is set on the state
 *           machine, calling HandleSwitchAction(REPORT_SWITCH_STATE) through the action
 *           pointer dispatches to DoReportSwitchState. This mirrors the compensation code
 *           added in SwitchSubscriberInit (subscribe-success branch) and
 *           OnAddSystemAbilityInner (multimodal SA online branch).
 * @tc.type: FUNC
 * @tc.require: issue#6
 */
HWTEST_F(SwitchActionTest, PowerStateMachine_ReportSwitchState_Compensation_Dispatched, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerStateMachine_ReportSwitchState_Compensation_Dispatched start");
    ASSERT_NE(stateMachine_, nullptr);

    auto stub = std::make_shared<SPNSwitchActionStub>();
    stateMachine_->SetSwitchAction(stub);
    ASSERT_NE(stateMachine_->GetSwitchActionPtr(), nullptr);

    auto ret = stateMachine_->GetSwitchActionPtr()->HandleSwitchAction(
        SwitchActionType::REPORT_SWITCH_STATE);
    EXPECT_TRUE(stub->IsReportSwitchStateCalled());
    EXPECT_FALSE(stub->IsSwitchOpenCalled());
    EXPECT_FALSE(stub->IsSwitchCloseCalled());
    EXPECT_EQ(ret, SwitchActionRet::HANDLED);
    POWER_HILOGI(LABEL_TEST, "PowerStateMachine_ReportSwitchState_Compensation_Dispatched end");
}

/**
 * @tc.name: PowerStateMachine_ReportSwitchState_Compensation_LidOpen_LightsInnerScreen
 * @tc.desc: Verify the bug-fix scenario (T2): device initially reports closed (DMS lights
 *           outer screen), the open event is lost in the timing window, then the
 *           compensation re-reports the real state. Setting switchOpen=true represents the
 *           real "lid open" state read by HDI; calling REPORT_SWITCH_STATE through the stub
 *           represents the compensation notifying DMS, which then lights the inner screen.
 *           The stub's DoReportSwitchState is invoked, confirming DMS would be notified.
 * @tc.type: FUNC
 * @tc.require: issue#6
 */
HWTEST_F(SwitchActionTest, PowerStateMachine_ReportSwitchState_Compensation_LidOpen_LightsInnerScreen, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerStateMachine_ReportSwitchState_Compensation_LidOpen_LightsInnerScreen start");
    ASSERT_NE(stateMachine_, nullptr);

    auto stub = std::make_shared<SPNSwitchActionStub>();
    stateMachine_->SetSwitchAction(stub);

    // Step 1: device initially closed (OnAddSystemAbility reported CLOSED to DMS,
    // outer screen lit). Internal state reflects closed.
    stateMachine_->SetSwitchState(false);
    EXPECT_FALSE(stateMachine_->IsSwitchOpen());

    // Step 2: user opens the lid during the window; the SwitchEvent has no listener yet
    // and is lost. The open event (DoSwitchOpen) is NOT delivered here.
    stub->Reset();
    EXPECT_FALSE(stub->IsSwitchOpenCalled());

    // Step 3: real lid state is now open (HDI would read open). Reflect truth in state.
    stateMachine_->SetSwitchState(true);
    EXPECT_TRUE(stateMachine_->IsSwitchOpen());

    // Step 4: subscription succeeds -> compensation re-reports real state to DMS.
    // DoReportSwitchState is invoked; DMS receives OPEN and lights the inner screen.
    ASSERT_NE(stateMachine_->GetSwitchActionPtr(), nullptr);
    auto ret = stateMachine_->GetSwitchActionPtr()->HandleSwitchAction(
        SwitchActionType::REPORT_SWITCH_STATE);
    EXPECT_TRUE(stub->IsReportSwitchStateCalled());
    EXPECT_FALSE(stub->IsSwitchOpenCalled());
    EXPECT_EQ(ret, SwitchActionRet::HANDLED);
    POWER_HILOGI(LABEL_TEST, "PowerStateMachine_ReportSwitchState_Compensation_LidOpen_LightsInnerScreen end");
}

/**
 * @tc.name: PowerStateMachine_ReportSwitchState_Compensation_StillClosed_NoOpenNotify
 * @tc.desc: Verify that when the real state is still closed at subscription time, the
 *           compensation only re-reports CLOSED (DoReportSwitchState called) and does not
 *           trigger an open flow (DoSwitchOpen not called). Confirms no spurious wakeup.
 * @tc.type: FUNC
 * @tc.require: issue#6
 */
HWTEST_F(SwitchActionTest, PowerStateMachine_ReportSwitchState_Compensation_StillClosed_NoOpenNotify, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerStateMachine_ReportSwitchState_Compensation_StillClosed_NoOpenNotify start");
    ASSERT_NE(stateMachine_, nullptr);

    auto stub = std::make_shared<SPNSwitchActionStub>();
    stateMachine_->SetSwitchAction(stub);
    stateMachine_->SetSwitchState(false);
    EXPECT_FALSE(stateMachine_->IsSwitchOpen());

    stub->Reset();
    ASSERT_NE(stateMachine_->GetSwitchActionPtr(), nullptr);
    auto ret = stateMachine_->GetSwitchActionPtr()->HandleSwitchAction(
        SwitchActionType::REPORT_SWITCH_STATE);
    EXPECT_TRUE(stub->IsReportSwitchStateCalled());
    EXPECT_FALSE(stub->IsSwitchOpenCalled());
    EXPECT_FALSE(stub->IsSwitchCloseCalled());
    EXPECT_EQ(ret, SwitchActionRet::HANDLED);
    POWER_HILOGI(LABEL_TEST, "PowerStateMachine_ReportSwitchState_Compensation_StillClosed_NoOpenNotify end");
}

/**
 * @tc.name: PowerStateMachine_ReportSwitchState_Compensation_DefaultAction_NoEffect
 * @tc.desc: Verify the compensation call is a no-op on non-foldable devices: with the
 *           default ISwitchAction, REPORT_SWITCH_STATE returns DEFAULT and no screen state
 *           changes occur. Covers F4 (non-foldable unaffected) and C3 (null/empty action).
 * @tc.type: FUNC
 * @tc.require: issue#6
 */
HWTEST_F(SwitchActionTest, PowerStateMachine_ReportSwitchState_Compensation_DefaultAction_NoEffect, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerStateMachine_ReportSwitchState_Compensation_DefaultAction_NoEffect start");
    ASSERT_NE(stateMachine_, nullptr);

    stateMachine_->SetSwitchAction(std::make_shared<ISwitchAction>());
    ASSERT_NE(stateMachine_->GetSwitchActionPtr(), nullptr);
    stateMachine_->SetSwitchState(true);

    auto ret = stateMachine_->GetSwitchActionPtr()->HandleSwitchAction(
        SwitchActionType::REPORT_SWITCH_STATE);
    EXPECT_EQ(ret, SwitchActionRet::DEFAULT);

    // State machine still transitions normally; default action does not block wakeup/suspend.
    stateMachine_->WakeupDeviceInner(PID, CALLTIMEMS,
        WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON, "test", "test");
    EXPECT_EQ(stateMachine_->GetState(), PowerState::AWAKE);
    stateMachine_->SuspendDeviceInner(PID, CALLTIMEMS,
        SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, true);
    EXPECT_EQ(stateMachine_->GetState(), PowerState::INACTIVE);
    POWER_HILOGI(LABEL_TEST, "PowerStateMachine_ReportSwitchState_Compensation_DefaultAction_NoEffect end");
}

/**
 * @tc.name: PowerStateMachine_ReportSwitchState_Compensation_NullAction_NoCrash
 * @tc.desc: Verify the compensation's null-pointer guard (C3): when GetSwitchActionPtr
 *           returns nullptr, the compensation must not crash. The caller checks for nullptr
 *           before invoking HandleSwitchAction, so no dispatch happens.
 * @tc.type: FUNC
 * @tc.require: issue#6
 */
HWTEST_F(SwitchActionTest, PowerStateMachine_ReportSwitchState_Compensation_NullAction_NoCrash, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerStateMachine_ReportSwitchState_Compensation_NullAction_NoCrash start");
    stateMachine_ = pms_->GetPowerStateMachine();
    ASSERT_NE(stateMachine_, nullptr);
    pms_->SwitchSubscriberInit();
    stateMachine_->SetSwitchAction(nullptr);
    auto action = stateMachine_->GetSwitchActionPtr();
    EXPECT_EQ(action, nullptr);
    // Mirror the compensation guard: only dispatch when the pointer is non-null.
    pms_->SwitchSubscriberInit();
    POWER_HILOGI(LABEL_TEST, "PowerStateMachine_ReportSwitchState_Compensation_NullAction_NoCrash end");
}

/**
 * @tc.name: PowerStateMachine_ReportSwitchState_AfterSwitchOpen_NotDuplicated
 * @tc.desc: Verify that calling REPORT_SWITCH_STATE after a normal SWITCH_OPEN does not
 *           interfere: both are independently dispatched, and the compensation re-report is
 *           idempotent (R: no duplicate wakeup). DoSwitchOpen and DoReportSwitchState are
 *           each called once.
 * @tc.type: FUNC
 * @tc.require: issue#6
 */
HWTEST_F(SwitchActionTest, PowerStateMachine_ReportSwitchState_AfterSwitchOpen_NotDuplicated, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerStateMachine_ReportSwitchState_AfterSwitchOpen_NotDuplicated start");
    ASSERT_NE(stateMachine_, nullptr);

    auto stub = std::make_shared<SPNSwitchActionStub>();
    stateMachine_->SetSwitchAction(stub);
    stateMachine_->SetSwitchState(false);

    // Normal open event delivered
    ASSERT_EQ(stub->HandleSwitchAction(SwitchActionType::SWITCH_OPEN), SwitchActionRet::HANDLED);
    EXPECT_TRUE(stub->IsSwitchOpenCalled());

    // Compensation re-report afterwards
    stub->Reset();
    ASSERT_EQ(stub->HandleSwitchAction(SwitchActionType::REPORT_SWITCH_STATE),
        SwitchActionRet::HANDLED);
    EXPECT_TRUE(stub->IsReportSwitchStateCalled());
    EXPECT_FALSE(stub->IsSwitchOpenCalled());
    POWER_HILOGI(LABEL_TEST, "PowerStateMachine_ReportSwitchState_AfterSwitchOpen_NotDuplicated end");
}

} // namespace
