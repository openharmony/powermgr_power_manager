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

#include "native_power_state_machine_decoupled_test.h"

#include <ipc_skeleton.h>
#include <datetime_ex.h>

#include <common_event_data.h>
#include <common_event_manager.h>
#include <common_event_subscriber.h>
#include <common_event_support.h>
#include <common_event_publish_info.h>
#include "power_mgr_client.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

#ifdef POWER_MANAGER_ENABLE_SCREEN_DECOUPLING

namespace {
constexpr int32_t DECOUPLED_WAIT_EVENT_MS = 400;
constexpr int32_t DECOUPLED_US_PER_MS = 1000;
bool g_decoupledScreenOn = false;
bool g_decoupledScreenOff = false;

class DecoupledScreenEventSubscriber : public OHOS::EventFwk::CommonEventSubscriber {
public:
    explicit DecoupledScreenEventSubscriber(const OHOS::EventFwk::CommonEventSubscribeInfo& info)
        : OHOS::EventFwk::CommonEventSubscriber(info) {}
    void OnReceiveEvent(const OHOS::EventFwk::CommonEventData& data) override
    {
        std::string action = data.GetWant().GetAction();
        if (action == OHOS::EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON) {
            g_decoupledScreenOn = true;
        } else if (action == OHOS::EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF) {
            g_decoupledScreenOff = true;
        }
    }
    static std::shared_ptr<DecoupledScreenEventSubscriber> Register()
    {
        OHOS::EventFwk::MatchingSkills skills;
        skills.AddEvent(OHOS::EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON);
        skills.AddEvent(OHOS::EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF);
        OHOS::EventFwk::CommonEventSubscribeInfo info(skills);
        auto subscriber = std::make_shared<DecoupledScreenEventSubscriber>(info);
        if (OHOS::EventFwk::CommonEventManager::SubscribeCommonEvent(subscriber)) {
            return subscriber;
        }
        return nullptr;
    }
};

// In-process service instance for PowerMgrClient proxy override.
sptr<PowerMgrService> g_decoupledPms = nullptr;
} // namespace

// PowerMgrClient proxy override: connect to the in-process service (same as coordination_lock_test).
sptr<IPowerMgr> PowerMgrClient::GetPowerMgrProxy()
{
    return g_decoupledPms;
}

namespace {
// Test constants.
constexpr int64_t CALLTIMEMS = 1;
constexpr pid_t PID = 1;
} // namespace

void NativePowerStateMachineDecoupledTest::SetUpTestCase()
{
    g_decoupledPms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    g_decoupledPms->OnStart();
}

/**
 * @tc.name: NativePowerStateMachineDecoupled001
 * @tc.desc: test decoupling init state is always AWAKE regardless of screen state
 * @tc.type: FUNC
 */
HWTEST_F(NativePowerStateMachineDecoupledTest, NativePowerStateMachineDecoupled001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativePowerStateMachineDecoupled001: decoupling init to AWAKE start.";
    POWER_HILOGI(LABEL_TEST, "NativePowerStateMachineDecoupled001 function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    pmsTest->OnStart();
    auto stateMachine = std::make_shared<PowerStateMachine>(pmsTest);
    EXPECT_TRUE(stateMachine->Init());
    // InitState always switches to AWAKE on decoupling.
    stateMachine->InitState();
    EXPECT_TRUE(stateMachine->GetState() == PowerState::AWAKE);
    POWER_HILOGI(LABEL_TEST, "NativePowerStateMachineDecoupled001 function end!");
    GTEST_LOG_(INFO) << "NativePowerStateMachineDecoupled001: decoupling init to AWAKE end.";
}

/**
 * @tc.name: NativePowerStateMachineDecoupled002
 * @tc.desc: test baseline SetState to AWAKE succeeds on screen decoupling
 * @tc.type: FUNC
 */
HWTEST_F(NativePowerStateMachineDecoupledTest, NativePowerStateMachineDecoupled002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativePowerStateMachineDecoupled002: SetState AWAKE decoupled start.";
    POWER_HILOGI(LABEL_TEST, "NativePowerStateMachineDecoupled002 function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    pmsTest->OnStart();
    pmsTest->SuspendControllerInit();
    pmsTest->WakeupControllerInit();
    auto stateMachine = pmsTest->GetPowerStateMachine();
    EXPECT_TRUE(stateMachine->Init());
    stateMachine->InitState(); // to AWAKE first
    // Screen decoupling: baseline SetState switches the power state to AWAKE; the screen is cockpit-managed
    // (stateAction display methods are no-op, exercised by decoupled_device_state_action_test).
    stateMachine->currentState_ = PowerState::INACTIVE;
    bool ret =
        stateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_APPLICATION, true);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(stateMachine->GetState() == PowerState::AWAKE);
    POWER_HILOGI(LABEL_TEST, "NativePowerStateMachineDecoupled002 function end!");
    GTEST_LOG_(INFO) << "NativePowerStateMachineDecoupled002: SetState AWAKE decoupled end.";
}

/**
 * @tc.name: NativePowerStateMachineDecoupled003
 * @tc.desc: test baseline SetState to INACTIVE succeeds on screen decoupling
 * @tc.type: FUNC
 */
HWTEST_F(NativePowerStateMachineDecoupledTest, NativePowerStateMachineDecoupled003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativePowerStateMachineDecoupled003: SetState INACTIVE decoupled start.";
    POWER_HILOGI(LABEL_TEST, "NativePowerStateMachineDecoupled003 function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    pmsTest->OnStart();
    pmsTest->SuspendControllerInit();
    pmsTest->WakeupControllerInit();
    auto stateMachine = pmsTest->GetPowerStateMachine();
    EXPECT_TRUE(stateMachine->Init());
    stateMachine->InitState();
    // Screen decoupling: baseline SetState switches the power state to INACTIVE; the screen is cockpit-managed
    // (stateAction display methods are no-op, exercised by decoupled_device_state_action_test).
    bool ret =
        stateMachine->SetState(PowerState::INACTIVE, StateChangeReason::STATE_CHANGE_REASON_APPLICATION, true);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(stateMachine->GetState() == PowerState::INACTIVE);
    POWER_HILOGI(LABEL_TEST, "NativePowerStateMachineDecoupled003 function end!");
    GTEST_LOG_(INFO) << "NativePowerStateMachineDecoupled003: SetState INACTIVE decoupled end.";
}

/**
 * @tc.name: NativePowerStateMachineDecoupled004
 * @tc.desc: test baseline SetState keeps forbidMap gating from AWAKE: AWAKE->DIM (IVI mask) and AWAKE->SLEEP forbidden
 * @tc.type: FUNC
 */
HWTEST_F(NativePowerStateMachineDecoupledTest, NativePowerStateMachineDecoupled004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativePowerStateMachineDecoupled004: forbidMap from AWAKE start.";
    POWER_HILOGI(LABEL_TEST, "NativePowerStateMachineDecoupled004 function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    pmsTest->OnStart();
    auto stateMachine = std::make_shared<PowerStateMachine>(pmsTest);
    EXPECT_TRUE(stateMachine->Init());
    stateMachine->InitState();
    EXPECT_TRUE(stateMachine->GetState() == PowerState::AWAKE);
    // AWAKE->DIM is forbidden (IVI masks DIM via forbidMap_).
    EXPECT_FALSE(stateMachine->CanTransitTo(PowerState::AWAKE, PowerState::DIM,
        StateChangeReason::STATE_CHANGE_REASON_TIMEOUT));
    EXPECT_FALSE(stateMachine->SetState(PowerState::DIM, StateChangeReason::STATE_CHANGE_REASON_TIMEOUT, true));
    EXPECT_TRUE(stateMachine->GetState() == PowerState::AWAKE);
    // AWAKE->SLEEP is also forbidden by forbidMap_ (SLEEP must be reached via INACTIVE).
    EXPECT_FALSE(stateMachine->SetState(PowerState::SLEEP, StateChangeReason::STATE_CHANGE_REASON_TIMEOUT, true));
    EXPECT_TRUE(stateMachine->GetState() == PowerState::AWAKE);
    POWER_HILOGI(LABEL_TEST, "NativePowerStateMachineDecoupled004 function end!");
    GTEST_LOG_(INFO) << "NativePowerStateMachineDecoupled004: forbidMap from AWAKE end.";
}

/**
 * @tc.name: NativePowerStateMachineDecoupled005
 * @tc.desc: test baseline SetState is idempotent: repeated AWAKE returns success
 * @tc.type: FUNC
 */
HWTEST_F(NativePowerStateMachineDecoupledTest, NativePowerStateMachineDecoupled005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativePowerStateMachineDecoupled005: idempotent start.";
    POWER_HILOGI(LABEL_TEST, "NativePowerStateMachineDecoupled005 function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    pmsTest->OnStart();
    auto stateMachine = std::make_shared<PowerStateMachine>(pmsTest);
    EXPECT_TRUE(stateMachine->Init());
    stateMachine->InitState();
    EXPECT_TRUE(stateMachine->GetState() == PowerState::AWAKE);
    // Already AWAKE; switching to AWAKE again should be idempotent.
    bool ret =
        stateMachine->SetState(PowerState::AWAKE, StateChangeReason::STATE_CHANGE_REASON_APPLICATION, true);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(stateMachine->GetState() == PowerState::AWAKE);
    POWER_HILOGI(LABEL_TEST, "NativePowerStateMachineDecoupled005 function end!");
    GTEST_LOG_(INFO) << "NativePowerStateMachineDecoupled005: idempotent end.";
}

/**
 * @tc.name: NativePowerStateMachineDecoupled006
 * @tc.desc: test SLEEP is reachable via baseline SetState on decoupling (CPU sleep path retained)
 * @tc.type: FUNC
 */
HWTEST_F(NativePowerStateMachineDecoupledTest, NativePowerStateMachineDecoupled006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativePowerStateMachineDecoupled006: SLEEP via SetState start.";
    POWER_HILOGI(LABEL_TEST, "NativePowerStateMachineDecoupled006 function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    pmsTest->OnStart();
    auto stateMachine = std::make_shared<PowerStateMachine>(pmsTest);
    EXPECT_TRUE(stateMachine->Init());
    stateMachine->InitState(); // AWAKE
    // On decoupling SLEEP is no longer a special "unhandled" case: it goes through baseline SetState like
    // the normal CPU-suspend path. AWAKE->SLEEP is forbidden, so go via INACTIVE first.
    EXPECT_TRUE(stateMachine->SetState(PowerState::INACTIVE, StateChangeReason::STATE_CHANGE_REASON_TIMEOUT, true));
    ASSERT_TRUE(stateMachine->GetState() == PowerState::INACTIVE);
    EXPECT_TRUE(stateMachine->SetState(PowerState::SLEEP, StateChangeReason::STATE_CHANGE_REASON_TIMEOUT, true));
    EXPECT_TRUE(stateMachine->GetState() == PowerState::SLEEP);
    POWER_HILOGI(LABEL_TEST, "NativePowerStateMachineDecoupled006 function end!");
    GTEST_LOG_(INFO) << "NativePowerStateMachineDecoupled006: SLEEP via SetState end.";
}

/**
 * @tc.name: NativePowerStateMachineDecoupled007
 * @tc.desc: test IsScreenOn follows power state on decoupling: AWAKE=true, INACTIVE=false
 * @tc.type: FUNC
 */
HWTEST_F(NativePowerStateMachineDecoupledTest, NativePowerStateMachineDecoupled007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativePowerStateMachineDecoupled007: IsScreenOn power state start.";
    POWER_HILOGI(LABEL_TEST, "NativePowerStateMachineDecoupled007 function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    pmsTest->OnStart();
    auto stateMachine = std::make_shared<PowerStateMachine>(pmsTest);
    EXPECT_TRUE(stateMachine->Init());
    stateMachine->InitState();
    // On decoupling IsScreenOn follows the power state: true when AWAKE.
    EXPECT_TRUE(stateMachine->GetState() == PowerState::AWAKE);
    EXPECT_TRUE(stateMachine->IsScreenOn());
    // After switching to INACTIVE, IsScreenOn becomes false.
    stateMachine->SetState(PowerState::INACTIVE, StateChangeReason::STATE_CHANGE_REASON_TIMEOUT, true);
    EXPECT_FALSE(stateMachine->IsScreenOn());
    POWER_HILOGI(LABEL_TEST, "NativePowerStateMachineDecoupled007 function end!");
    GTEST_LOG_(INFO) << "NativePowerStateMachineDecoupled007: IsScreenOn power state end.";
}

/**
 * @tc.name: NativePowerStateMachineDecoupled008
 * @tc.desc: test ResetInactiveTimer returns early on decoupling without starting timers
 * @tc.type: FUNC
 */
HWTEST_F(NativePowerStateMachineDecoupledTest, NativePowerStateMachineDecoupled008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativePowerStateMachineDecoupled008: ResetInactiveTimer early return start.";
    POWER_HILOGI(LABEL_TEST, "NativePowerStateMachineDecoupled008 function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    pmsTest->OnStart();
    auto stateMachine = std::make_shared<PowerStateMachine>(pmsTest);
    EXPECT_TRUE(stateMachine->Init());
    stateMachine->InitState();
    PowerState before = stateMachine->GetState();
    stateMachine->ResetInactiveTimer();
    EXPECT_TRUE(stateMachine->GetState() == before);
    POWER_HILOGI(LABEL_TEST, "NativePowerStateMachineDecoupled008 function end!");
    GTEST_LOG_(INFO) << "NativePowerStateMachineDecoupled008: ResetInactiveTimer early return end.";
}

/**
 * @tc.name: NativePowerStateMachineDecoupled009
 * @tc.desc: test SuspendDeviceInner routes to SetState(INACTIVE) on screen decoupling
 * @tc.type: FUNC
 */
HWTEST_F(NativePowerStateMachineDecoupledTest, NativePowerStateMachineDecoupled009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativePowerStateMachineDecoupled009: SuspendDeviceInner decoupled entry start.";
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    pmsTest->OnStart();
    pmsTest->SuspendControllerInit();
    pmsTest->WakeupControllerInit();
    auto stateMachine = pmsTest->GetPowerStateMachine();
    ASSERT_TRUE(stateMachine != nullptr);
    EXPECT_TRUE(stateMachine->Init());
    stateMachine->InitState(); // AWAKE
    ASSERT_TRUE(stateMachine->GetState() == PowerState::AWAKE);
    // On decoupling, SuspendDeviceInner routes to SetState(INACTIVE) (baseline).
    stateMachine->SuspendDeviceInner(IPCSkeleton::GetCallingPid(), GetTickCount(),
        SuspendDeviceType::SUSPEND_DEVICE_REASON_TIMEOUT, false, true);
    EXPECT_TRUE(stateMachine->GetState() == PowerState::INACTIVE);
    GTEST_LOG_(INFO) << "NativePowerStateMachineDecoupled009: SuspendDeviceInner decoupled entry end.";
}

/**
 * @tc.name: NativePowerStateMachineDecoupled010
 * @tc.desc: test transitioning to AWAKE does not publish SCREEN_ON on decoupling
 * @tc.type: FUNC
 */
HWTEST_F(NativePowerStateMachineDecoupledTest, NativePowerStateMachineDecoupled010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativePowerStateMachineDecoupled010: no SCREEN_ON broadcast start.";
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    pmsTest->OnStart();
    auto stateMachine = std::make_shared<PowerStateMachine>(pmsTest);
    EXPECT_TRUE(stateMachine->Init());
    stateMachine->InitState(); // AWAKE

    g_decoupledScreenOn = false;
    g_decoupledScreenOff = false;
    auto subscriber = DecoupledScreenEventSubscriber::Register();
    ASSERT_FALSE(subscriber == nullptr);

    // Switch to INACTIVE, then back to AWAKE (baseline publishes SCREEN_ON here).
    stateMachine->SetState(PowerState::INACTIVE,
        StateChangeReason::STATE_CHANGE_REASON_APPLICATION, true);
    usleep(DECOUPLED_WAIT_EVENT_MS * DECOUPLED_US_PER_MS);
    stateMachine->SetState(PowerState::AWAKE,
        StateChangeReason::STATE_CHANGE_REASON_APPLICATION, true);
    usleep(DECOUPLED_WAIT_EVENT_MS * DECOUPLED_US_PER_MS);
    EXPECT_EQ(stateMachine->GetState(), PowerState::AWAKE);

    // Screen decoupling: SCREEN_ON is not published.
    EXPECT_FALSE(g_decoupledScreenOn);

    OHOS::EventFwk::CommonEventManager::UnSubscribeCommonEvent(subscriber);
    GTEST_LOG_(INFO) << "NativePowerStateMachineDecoupled010: no SCREEN_ON broadcast end.";
}

/**
 * @tc.name: NativePowerStateMachineDecoupled011
 * @tc.desc: test transitioning to INACTIVE does not publish SCREEN_OFF on decoupling
 * @tc.type: FUNC
 */
HWTEST_F(NativePowerStateMachineDecoupledTest, NativePowerStateMachineDecoupled011, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativePowerStateMachineDecoupled011: no SCREEN_OFF broadcast start.";
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    pmsTest->OnStart();
    auto stateMachine = std::make_shared<PowerStateMachine>(pmsTest);
    EXPECT_TRUE(stateMachine->Init());
    stateMachine->InitState(); // AWAKE

    g_decoupledScreenOn = false;
    g_decoupledScreenOff = false;
    auto subscriber = DecoupledScreenEventSubscriber::Register();
    ASSERT_FALSE(subscriber == nullptr);

    // Switch from AWAKE to INACTIVE (baseline publishes SCREEN_OFF here).
    stateMachine->SetState(PowerState::INACTIVE,
        StateChangeReason::STATE_CHANGE_REASON_APPLICATION, true);
    usleep(DECOUPLED_WAIT_EVENT_MS * DECOUPLED_US_PER_MS);
    EXPECT_EQ(stateMachine->GetState(), PowerState::INACTIVE);

    // Screen decoupling: SCREEN_OFF is not published.
    EXPECT_FALSE(g_decoupledScreenOff);

    OHOS::EventFwk::CommonEventManager::UnSubscribeCommonEvent(subscriber);
    GTEST_LOG_(INFO) << "NativePowerStateMachineDecoupled011: no SCREEN_OFF broadcast end.";
}

TransitResult MockTransitCallback(StateChangeReason trigger)
{
    return TransitResult::ALREADY_IN_STATE;
}

/**
 * @tc.name: NativePowerStateMachineDecoupled012
 * @tc.desc: test TransitTo returns OTHER_ERR when owner is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(NativePowerStateMachineDecoupledTest, NativePowerStateMachineDecoupled012, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativePowerStateMachineDecoupled012: TransitTo OTHER_ERR start.";
    auto stateMachineController = std::make_shared<PowerStateMachine::StateController>(
        PowerState::INACTIVE, nullptr, MockTransitCallback);
    StateChangeReason reason = StateChangeReason::STATE_CHANGE_REASON_BATTERY;
    EXPECT_TRUE(stateMachineController->TransitTo(reason, false) == TransitResult::OTHER_ERR);
    GTEST_LOG_(INFO) << "NativePowerStateMachineDecoupled012: TransitTo OTHER_ERR end.";
}

/**
 * @tc.name: NativePowerStateMachineDecoupled013
 * @tc.desc: test SetState returns false when running lock blocks transit (LOCKING)
 * @tc.type: FUNC
 */
HWTEST_F(NativePowerStateMachineDecoupledTest, NativePowerStateMachineDecoupled013, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativePowerStateMachineDecoupled013: SetState LOCKING start.";
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    pmsTest->OnStart();
    auto stateMachine = pmsTest->GetPowerStateMachine();
    ASSERT_TRUE(stateMachine != nullptr);
    EXPECT_TRUE(stateMachine->Init());
    stateMachine->InitState();
    stateMachine->currentState_ = PowerState::AWAKE;

    // Acquire a RUNNINGLOCK_SCREEN lock to block the INACTIVE transition.
    auto runningLockMgr = pmsTest->GetRunningLockMgr();
    ASSERT_TRUE(runningLockMgr != nullptr);
    sptr<IRemoteObject> token = new RunningLockTokenStub();
    int32_t pid = IPCSkeleton::GetCallingPid();
    int32_t uid = IPCSkeleton::GetCallingUid();
    RunningLockParam param {0, "decoupledLockTest", "", RunningLockType::RUNNINGLOCK_SCREEN, 10000, pid, uid};
    ASSERT_TRUE(runningLockMgr->CreateRunningLock(token, param) != nullptr);
    runningLockMgr->Lock(token);

    // baseline SetState(INACTIVE) is blocked by the running lock -> returns false, state unchanged.
    EXPECT_FALSE(stateMachine->SetState(PowerState::INACTIVE, StateChangeReason::STATE_CHANGE_REASON_INIT, false));
    EXPECT_TRUE(stateMachine->GetState() == PowerState::AWAKE);

    // Cleanup: unlock to avoid side effects on other tests
    runningLockMgr->UnLock(token);
    GTEST_LOG_(INFO) << "NativePowerStateMachineDecoupled013: SetState LOCKING end.";
}

#endif // POWER_MANAGER_ENABLE_SCREEN_DECOUPLING
