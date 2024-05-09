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

#include "power_coordination_lock_test.h"

#include <common_event_data.h>
#include <common_event_manager.h>
#include <common_event_publish_info.h>
#include <common_event_subscriber.h>
#include <common_event_support.h>

#ifdef HAS_MULTIMODALINPUT_INPUT_PART
#include "input_manager.h"
#include "pointer_event.h"
#endif
#include "power_log.h"
#include "power_mgr_client.h"
#include "power_mgr_service.h"
#include "power_state_callback_stub.h"

using namespace OHOS;
using namespace OHOS::EventFwk;
using namespace OHOS::PowerMgr;
using namespace std;
using namespace testing::ext;

namespace {
constexpr uint32_t SCREEN_OFF_TIME_OVERRIDE_COORDINATION_MS = 10000;
constexpr int32_t US_PER_MS = 1000;
constexpr uint32_t AUTO_SLEEP_DELAY_MS = 5000;
constexpr uint32_t WAIT_AUTO_SUSPEND_SLEEP_TIME_MS = AUTO_SLEEP_DELAY_MS + 2000;
constexpr int32_t WAIT_EVENT_TIME_MS = 400;
constexpr int32_t RETRY_WAIT_TIME_MS = 100;
constexpr int32_t WAIT_STATE_TIME_MS = 500;
constexpr int32_t OVER_TIME_SCREEN_OFF_TIME_MS = 2000;
constexpr int32_t OVER_TIME_SCREEN_OFF_TIME_TEST_MS = 2000 + 2000;
constexpr int32_t WAIT_SUSPEND_TIME_MS = 2000;

bool g_screenOnEvent = false;
bool g_screenOffEvent = false;
bool g_awakeCallback = false;
bool g_inactiveCallback = false;
PowerMode g_modeBeforeTest = PowerMode::NORMAL_MODE;

#ifdef HAS_MULTIMODALINPUT_INPUT_PART
std::shared_ptr<KeyEvent> CreateKeyEvent()
{
    std::shared_ptr<MMI::KeyEvent> keyEvent = MMI::KeyEvent::Create();
    keyEvent->SetKeyAction(MMI::KeyEvent::KEY_ACTION_DOWN);
    keyEvent->SetKeyCode(MMI::KeyEvent::KEYCODE_0);
    keyEvent->AddFlag(MMI::InputEvent::EVENT_FLAG_SIMULATE);
    return keyEvent;
}

std::shared_ptr<PointerEvent> CreatePointerEvent()
{
    constexpr const int32_t ARBITRARY_NON_MAGIC_NUMBER_SIX = 6;
    constexpr const int32_t ARBITRARY_NON_MAGIC_NUMBER_EIGHT = 8;
    constexpr const int32_t ARBITRARY_NON_MAGIC_NUMBER_TEN = 10;
    auto pointerEvent = PointerEvent::Create();

    PointerEvent::PointerItem item;
    item.SetPointerId(0);
    item.SetDisplayX(ARBITRARY_NON_MAGIC_NUMBER_SIX);
    item.SetDisplayY(ARBITRARY_NON_MAGIC_NUMBER_SIX);
    item.SetPressure(ARBITRARY_NON_MAGIC_NUMBER_SIX);
    item.SetDeviceId(1);
    pointerEvent->AddPointerItem(item);

    item.SetPointerId(1);
    item.SetDisplayX(ARBITRARY_NON_MAGIC_NUMBER_SIX);
    item.SetDisplayY(ARBITRARY_NON_MAGIC_NUMBER_TEN);
    item.SetPressure(ARBITRARY_NON_MAGIC_NUMBER_EIGHT);
    item.SetDeviceId(1);
    pointerEvent->AddPointerItem(item);

    pointerEvent->SetPointerAction(PointerEvent::POINTER_ACTION_DOWN);
    pointerEvent->SetPointerId(1);
    pointerEvent->SetSourceType(PointerEvent::SOURCE_TYPE_TOUCHSCREEN);

    pointerEvent->AddFlag(MMI::InputEvent::EVENT_FLAG_SIMULATE);

    return pointerEvent;
}
#endif

void ResetTriggeredFlag()
{
    g_screenOnEvent = false;
    g_screenOffEvent = false;
    g_awakeCallback = false;
    g_inactiveCallback = false;
}

void MatchCommonEventTriggered(std::string event)
{
    if (event == CommonEventSupport::COMMON_EVENT_SCREEN_ON) {
        g_screenOnEvent = true;
    } else if (event == CommonEventSupport::COMMON_EVENT_SCREEN_OFF) {
        g_screenOffEvent = true;
    }
}

void MatchPowerStateTriggered(PowerState state)
{
    switch (state) {
        case PowerState::AWAKE:
            g_awakeCallback =  true;
            break;
        case PowerState::INACTIVE:
            g_inactiveCallback = true;
            break;
        default:
            break;
    }
}

class PowerStateCommonEventSubscriber : public CommonEventSubscriber {
public:
    explicit PowerStateCommonEventSubscriber(const CommonEventSubscribeInfo& subscribeInfo)
        : CommonEventSubscriber(subscribeInfo) {}
    virtual ~PowerStateCommonEventSubscriber() {}
    void OnReceiveEvent(const CommonEventData &data) override
    {
        std::string action = data.GetWant().GetAction();
        POWER_HILOGI(LABEL_TEST, "On receive common event=%{public}s", action.c_str());
        MatchCommonEventTriggered(action);
    }
    static shared_ptr<PowerStateCommonEventSubscriber> RegisterEvent();
};

shared_ptr<PowerStateCommonEventSubscriber> PowerStateCommonEventSubscriber::RegisterEvent()
{
    POWER_HILOGI(LABEL_TEST, "Regist subscriber screen off event");
    int32_t retryTimes = 2;
    bool succeed = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_SCREEN_ON);
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_SCREEN_OFF);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<PowerStateCommonEventSubscriber>(subscribeInfo);
    for (int32_t tryTimes = 0; tryTimes < retryTimes; tryTimes++) {
        succeed = CommonEventManager::SubscribeCommonEvent(subscriberPtr);
        if (succeed) {
            break;
        }
        usleep(RETRY_WAIT_TIME_MS * US_PER_MS);
    }
    if (!succeed) {
        POWER_HILOGI(LABEL_TEST, "Failed to register subscriber");
        return nullptr;
    }
    return subscriberPtr;
}

class PowerStateTestCallback : public PowerStateCallbackStub {
public:
    PowerStateTestCallback() = default;
    virtual ~PowerStateTestCallback() = default;
    void OnPowerStateChanged(PowerState state) override
    {
        POWER_HILOGI(LABEL_TEST, "On power state=%{public}d changed callback", state);
        MatchPowerStateTriggered(state);
    }
};
}

void PowerCoordinationLockTest::SetUpTestCase(void)
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    g_modeBeforeTest = powerMgrClient.GetDeviceMode();
    EXPECT_EQ(powerMgrClient.SetDeviceMode(PowerMode::NORMAL_MODE), PowerErrors::ERR_OK);
}

void PowerCoordinationLockTest::TearDownTestCase(void)
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.SetDeviceMode(g_modeBeforeTest);
}

void PowerCoordinationLockTest::TearDown(void)
{
    ResetTriggeredFlag();
}

namespace {
/**
 * @tc.name: PowerCoordinationLockTest_001
 * @tc.desc: test coordination runninglock func when power state is awake
 * @tc.type: FUNC
 * @tc.require: issueI8JBT4
 */
HWTEST_F (PowerCoordinationLockTest, PowerCoordinationLockTest_001, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerCoordinationLockTest_001 start");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    EXPECT_TRUE(powerMgrClient.IsRunningLockTypeSupported(RunningLockType::RUNNINGLOCK_COORDINATION));
    auto runninglock =
        powerMgrClient.CreateRunningLock("CoordinationRunninglock001", RunningLockType::RUNNINGLOCK_COORDINATION);
    ASSERT_NE(runninglock, nullptr);
    EXPECT_FALSE(runninglock->IsUsed());

    powerMgrClient.WakeupDevice();
    EXPECT_TRUE(powerMgrClient.IsScreenOn());
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);

    runninglock->Lock();
    EXPECT_TRUE(runninglock->IsUsed());
    runninglock->UnLock();
    EXPECT_FALSE(runninglock->IsUsed());

    int32_t timeoutMs = 500;
    runninglock->Lock(timeoutMs);
    EXPECT_TRUE(runninglock->IsUsed());
    usleep(timeoutMs * US_PER_MS);
    usleep(WAIT_EVENT_TIME_MS * US_PER_MS);
    EXPECT_FALSE(runninglock->IsUsed());
    POWER_HILOGI(LABEL_TEST, "PowerCoordinationLockTest_001 end");
}

/**
 * @tc.name: PowerCoordinationLockTest_002
 * @tc.desc: test coordination runninglock proxy func when power state is awake
 * @tc.type: FUNC
 * @tc.require: issueI8JBT4
 */
HWTEST_F (PowerCoordinationLockTest, PowerCoordinationLockTest_002, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerCoordinationLockTest_002 start");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    auto runninglock =
        powerMgrClient.CreateRunningLock("CoordinationRunninglock002", RunningLockType::RUNNINGLOCK_COORDINATION);
    ASSERT_NE(runninglock, nullptr);

    powerMgrClient.WakeupDevice();
    EXPECT_TRUE(powerMgrClient.IsScreenOn());
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);

    pid_t curUid = getuid();
    pid_t curPid = getpid();

    EXPECT_TRUE(powerMgrClient.ProxyRunningLock(true, curPid, curUid));
    runninglock->Lock();
    EXPECT_FALSE(runninglock->IsUsed());
    EXPECT_TRUE(powerMgrClient.ProxyRunningLock(false, curPid, curUid));

    runninglock->Lock();
    EXPECT_TRUE(runninglock->IsUsed());
    EXPECT_TRUE(powerMgrClient.ProxyRunningLock(true, curPid, curUid));
    EXPECT_FALSE(runninglock->IsUsed());
    EXPECT_TRUE(powerMgrClient.ProxyRunningLock(false, curPid, curUid));
    EXPECT_TRUE(runninglock->IsUsed());
    POWER_HILOGI(LABEL_TEST, "PowerCoordinationLockTest_002 end");
}


/**
 * @tc.name: PowerCoordinationLockTest_003
 * @tc.desc: test coordination runninglock proxy func when power state is awake
 * @tc.type: FUNC
 * @tc.require: issueI8JBT4
 */
HWTEST_F (PowerCoordinationLockTest, PowerCoordinationLockTest_003, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerCoordinationLockTest_003 start");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    auto runninglock =
        powerMgrClient.CreateRunningLock("CoordinationRunninglock003", RunningLockType::RUNNINGLOCK_COORDINATION);
    ASSERT_NE(runninglock, nullptr);

    powerMgrClient.WakeupDevice();
    EXPECT_TRUE(powerMgrClient.IsScreenOn());
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);

    pid_t curUid = getuid();
    pid_t curPid = getpid();

    int32_t timeoutMs = 500;
    runninglock->Lock(timeoutMs);
    EXPECT_TRUE(runninglock->IsUsed());

    EXPECT_TRUE(powerMgrClient.ProxyRunningLock(true, curPid, curUid));
    EXPECT_FALSE(runninglock->IsUsed());
    EXPECT_TRUE(powerMgrClient.ProxyRunningLock(false, curPid, curUid));
    EXPECT_TRUE(runninglock->IsUsed());
    usleep(timeoutMs * US_PER_MS);
    usleep(WAIT_EVENT_TIME_MS * US_PER_MS);
    EXPECT_FALSE(runninglock->IsUsed());
    POWER_HILOGI(LABEL_TEST, "PowerCoordinationLockTest_003 end");
}

/**
 * @tc.name: PowerCoordinationLockTest_004
 * @tc.desc: test coordination runninglock proxy func when power state is awake
 * @tc.type: FUNC
 * @tc.require: issueI8JBT4
 */
HWTEST_F (PowerCoordinationLockTest, PowerCoordinationLockTest_004, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerCoordinationLockTest_004 start");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    powerMgrClient.WakeupDevice();
    EXPECT_TRUE(powerMgrClient.IsScreenOn());
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);

    pid_t curUid = getuid();
    pid_t curPid = getpid();
    EXPECT_TRUE(powerMgrClient.ProxyRunningLock(true, curPid, curUid));

    auto runninglock =
        powerMgrClient.CreateRunningLock("CoordinationRunninglock004", RunningLockType::RUNNINGLOCK_COORDINATION);
    ASSERT_NE(runninglock, nullptr);
    runninglock->Lock();
    EXPECT_FALSE(runninglock->IsUsed());
    EXPECT_TRUE(powerMgrClient.ProxyRunningLock(false, curPid, curUid));
    EXPECT_FALSE(runninglock->IsUsed());
    POWER_HILOGI(LABEL_TEST, "PowerCoordinationLockTest_004 end");
}

/**
 * @tc.name: PowerCoordinationLockTest_005
 * @tc.desc: test coordination runninglock proxy func when power state is sleep
 * @tc.type: FUNC
 * @tc.require: issueI8JBT4
 */
HWTEST_F (PowerCoordinationLockTest, PowerCoordinationLockTest_005, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerCoordinationLockTest_005 start");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    auto runninglock =
        powerMgrClient.CreateRunningLock("PowerCoordinationLockTest_005", RunningLockType::RUNNINGLOCK_COORDINATION);
    ASSERT_NE(runninglock, nullptr);

    powerMgrClient.SuspendDevice();
    usleep(WAIT_AUTO_SUSPEND_SLEEP_TIME_MS * US_PER_MS);

    EXPECT_FALSE(powerMgrClient.IsScreenOn());
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::SLEEP);

    runninglock->Lock();
    EXPECT_FALSE(runninglock->IsUsed());
    runninglock->UnLock();
    EXPECT_FALSE(runninglock->IsUsed());
    POWER_HILOGI(LABEL_TEST, "PowerCoordinationLockTest_005 end");
}

/**
 * @tc.name: PowerCoordinationLockTest_006
 * @tc.desc: test coordination runninglock is locked, not notify event and callback when inactive(suspend)
 * @tc.type: FUNC
 * @tc.require: issueI8JBT4
 */
HWTEST_F (PowerCoordinationLockTest, PowerCoordinationLockTest_006, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerCoordinationLockTest_006 start");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    auto runninglock =
        powerMgrClient.CreateRunningLock("PowerCoordinationLockTest_006", RunningLockType::RUNNINGLOCK_COORDINATION);
    ASSERT_NE(runninglock, nullptr);
    EXPECT_FALSE(runninglock->IsUsed());
    powerMgrClient.WakeupDevice();
    EXPECT_TRUE(powerMgrClient.IsScreenOn());
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);

    runninglock->Lock();
    EXPECT_TRUE(runninglock->IsUsed());

    shared_ptr<PowerStateCommonEventSubscriber> subscriber = PowerStateCommonEventSubscriber::RegisterEvent();
    EXPECT_FALSE(subscriber == nullptr);
    const sptr<IPowerStateCallback> stateCallback = new PowerStateTestCallback();
    powerMgrClient.RegisterPowerStateCallback(stateCallback);

    powerMgrClient.SuspendDevice();
    usleep(WAIT_EVENT_TIME_MS * US_PER_MS);

    EXPECT_FALSE(g_screenOffEvent);
    EXPECT_FALSE(g_inactiveCallback);
    EXPECT_FALSE(powerMgrClient.IsScreenOn());
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::INACTIVE);

    usleep(WAIT_AUTO_SUSPEND_SLEEP_TIME_MS * US_PER_MS);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::INACTIVE);

    runninglock->UnLock();
    EXPECT_FALSE(runninglock->IsUsed());
    usleep(WAIT_EVENT_TIME_MS * US_PER_MS);

    EXPECT_FALSE(g_screenOffEvent);
    EXPECT_FALSE(g_inactiveCallback);
    EXPECT_TRUE(powerMgrClient.IsScreenOn());
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);

    powerMgrClient.SuspendDevice();
    usleep(WAIT_AUTO_SUSPEND_SLEEP_TIME_MS * US_PER_MS);

    EXPECT_TRUE(g_screenOffEvent);
    EXPECT_TRUE(g_inactiveCallback);
    EXPECT_FALSE(powerMgrClient.IsScreenOn());
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::SLEEP);

    CommonEventManager::UnSubscribeCommonEvent(subscriber);
    powerMgrClient.UnRegisterPowerStateCallback(stateCallback);
    POWER_HILOGI(LABEL_TEST, "PowerCoordinationLockTest_006 end");
}

/**
 * @tc.name: PowerCoordinationLockTest_007
 * @tc.desc: test coordination runninglock is locked, not notify event and callback when inactive(over time)
 * @tc.type: FUNC
 * @tc.require: issueI8JBT4
 */
HWTEST_F (PowerCoordinationLockTest, PowerCoordinationLockTest_007, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerCoordinationLockTest_007 start");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.OverrideScreenOffTime(OVER_TIME_SCREEN_OFF_TIME_MS);
    auto runninglock =
        powerMgrClient.CreateRunningLock("PowerCoordinationLockTest_007", RunningLockType::RUNNINGLOCK_COORDINATION);
    ASSERT_NE(runninglock, nullptr);
    EXPECT_FALSE(runninglock->IsUsed());
    powerMgrClient.WakeupDevice();
    EXPECT_TRUE(powerMgrClient.IsScreenOn());
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);

    runninglock->Lock();
    EXPECT_TRUE(runninglock->IsUsed());

    shared_ptr<PowerStateCommonEventSubscriber> subscriber = PowerStateCommonEventSubscriber::RegisterEvent();
    EXPECT_FALSE(subscriber == nullptr);
    const sptr<IPowerStateCallback> stateCallback = new PowerStateTestCallback();
    powerMgrClient.RegisterPowerStateCallback(stateCallback);

    usleep(OVER_TIME_SCREEN_OFF_TIME_TEST_MS * US_PER_MS);

    EXPECT_FALSE(g_screenOffEvent);
    EXPECT_FALSE(g_inactiveCallback);
    EXPECT_FALSE(powerMgrClient.IsScreenOn());
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::INACTIVE);

    usleep(WAIT_AUTO_SUSPEND_SLEEP_TIME_MS * US_PER_MS);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::INACTIVE);

    runninglock->UnLock();
    EXPECT_FALSE(runninglock->IsUsed());
    usleep(WAIT_EVENT_TIME_MS * US_PER_MS);

    EXPECT_FALSE(g_screenOffEvent);
    EXPECT_FALSE(g_inactiveCallback);
    EXPECT_TRUE(powerMgrClient.IsScreenOn());
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);

    CommonEventManager::UnSubscribeCommonEvent(subscriber);
    powerMgrClient.UnRegisterPowerStateCallback(stateCallback);
    powerMgrClient.RestoreScreenOffTime();
    POWER_HILOGI(LABEL_TEST, "PowerCoordinationLockTest_007 end");
}

/**
 * @tc.name: PowerCoordinationLockTest_008
 * @tc.desc: test coordination runninglock function, when the power state transitions
 * @tc.type: FUNC
 * @tc.require: issueI8JBT4
 */
HWTEST_F (PowerCoordinationLockTest, PowerCoordinationLockTest_008, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerCoordinationLockTest_008 start");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    auto runninglock =
        powerMgrClient.CreateRunningLock("PowerCoordinationLockTest_008", RunningLockType::RUNNINGLOCK_COORDINATION);
    ASSERT_NE(runninglock, nullptr);
    EXPECT_FALSE(runninglock->IsUsed());
    powerMgrClient.WakeupDevice();
    EXPECT_TRUE(powerMgrClient.IsScreenOn());
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);

    runninglock->Lock();
    EXPECT_TRUE(runninglock->IsUsed());

    shared_ptr<PowerStateCommonEventSubscriber> subscriber = PowerStateCommonEventSubscriber::RegisterEvent();
    EXPECT_FALSE(subscriber == nullptr);
    const sptr<IPowerStateCallback> stateCallback = new PowerStateTestCallback();
    powerMgrClient.RegisterPowerStateCallback(stateCallback);

    powerMgrClient.SuspendDevice();
    usleep(WAIT_EVENT_TIME_MS * US_PER_MS);

    EXPECT_FALSE(g_screenOffEvent);
    EXPECT_FALSE(g_inactiveCallback);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::INACTIVE);

    ResetTriggeredFlag();
    powerMgrClient.WakeupDevice();
    EXPECT_TRUE(powerMgrClient.IsScreenOn());
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);
    usleep(WAIT_EVENT_TIME_MS * US_PER_MS);
    EXPECT_TRUE(g_screenOnEvent);
    EXPECT_TRUE(g_awakeCallback);

    runninglock->UnLock();
    EXPECT_FALSE(runninglock->IsUsed());

    EXPECT_FALSE(g_screenOffEvent);
    EXPECT_FALSE(g_inactiveCallback);

    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);

    CommonEventManager::UnSubscribeCommonEvent(subscriber);
    powerMgrClient.UnRegisterPowerStateCallback(stateCallback);
    POWER_HILOGI(LABEL_TEST, "PowerCoordinationLockTest_008 end");
}

/**
 * @tc.name: PowerCoordinationLockTest_009
 * @tc.desc: test coordination runninglock function, when the power state transitions
 * @tc.type: FUNC
 * @tc.require: issueI8JBT4
 */
HWTEST_F (PowerCoordinationLockTest, PowerCoordinationLockTest_009, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerCoordinationLockTest_009 start");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    auto runninglockOne =
        powerMgrClient.CreateRunningLock("PowerCoordinationLockTest_009_1", RunningLockType::RUNNINGLOCK_COORDINATION);
    auto runninglockTwo =
        powerMgrClient.CreateRunningLock("PowerCoordinationLockTest_009_2", RunningLockType::RUNNINGLOCK_COORDINATION);
    ASSERT_NE(runninglockOne, nullptr);
    ASSERT_NE(runninglockTwo, nullptr);
    EXPECT_FALSE(runninglockOne->IsUsed());
    EXPECT_FALSE(runninglockTwo->IsUsed());

    powerMgrClient.WakeupDevice();

    runninglockOne->Lock();
    runninglockTwo->Lock();
    EXPECT_TRUE(runninglockOne->IsUsed());
    EXPECT_TRUE(runninglockTwo->IsUsed());

    shared_ptr<PowerStateCommonEventSubscriber> subscriber = PowerStateCommonEventSubscriber::RegisterEvent();
    EXPECT_FALSE(subscriber == nullptr);
    const sptr<IPowerStateCallback> stateCallback = new PowerStateTestCallback();
    powerMgrClient.RegisterPowerStateCallback(stateCallback);

    runninglockOne->UnLock();
    EXPECT_FALSE(runninglockOne->IsUsed());

    powerMgrClient.SuspendDevice();
    usleep(WAIT_EVENT_TIME_MS * US_PER_MS);

    EXPECT_FALSE(g_screenOffEvent);
    EXPECT_FALSE(g_inactiveCallback);
    EXPECT_FALSE(powerMgrClient.IsScreenOn());
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::INACTIVE);

    runninglockTwo->UnLock();
    EXPECT_FALSE(runninglockTwo->IsUsed());
    usleep(WAIT_EVENT_TIME_MS * US_PER_MS);

    EXPECT_FALSE(g_screenOffEvent);
    EXPECT_FALSE(g_inactiveCallback);
    EXPECT_TRUE(powerMgrClient.IsScreenOn());
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);

    CommonEventManager::UnSubscribeCommonEvent(subscriber);
    powerMgrClient.UnRegisterPowerStateCallback(stateCallback);
    POWER_HILOGI(LABEL_TEST, "PowerCoordinationLockTest_009 end");
}

/**
 * @tc.name: PowerCoordinationLockTest_010
 * @tc.desc: test coordination runninglock lock, screen keep off when touching the screen
 * @tc.type: FUNC
 * @tc.require: issueI8JBT4
 */
HWTEST_F (PowerCoordinationLockTest, PowerCoordinationLockTest_010, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerCoordinationLockTest_010 start");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    auto runninglock =
        powerMgrClient.CreateRunningLock("PowerCoordinationLockTest_010", RunningLockType::RUNNINGLOCK_COORDINATION);
    ASSERT_NE(runninglock, nullptr);
    EXPECT_FALSE(runninglock->IsUsed());
    powerMgrClient.WakeupDevice();
    EXPECT_TRUE(powerMgrClient.IsScreenOn());
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);

    runninglock->Lock();
    EXPECT_TRUE(runninglock->IsUsed());

    shared_ptr<PowerStateCommonEventSubscriber> subscriber = PowerStateCommonEventSubscriber::RegisterEvent();
    EXPECT_FALSE(subscriber == nullptr);
    const sptr<IPowerStateCallback> stateCallback = new PowerStateTestCallback();
    powerMgrClient.RegisterPowerStateCallback(stateCallback);

    powerMgrClient.SuspendDevice();
    usleep(WAIT_EVENT_TIME_MS * US_PER_MS);

    EXPECT_FALSE(g_screenOffEvent);
    EXPECT_FALSE(g_inactiveCallback);
    EXPECT_FALSE(powerMgrClient.IsScreenOn());
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::INACTIVE);

    powerMgrClient.RefreshActivity(UserActivityType::USER_ACTIVITY_TYPE_TOUCH);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::INACTIVE);

    runninglock->UnLock();
    EXPECT_FALSE(runninglock->IsUsed());
    usleep(WAIT_EVENT_TIME_MS * US_PER_MS);

    EXPECT_FALSE(g_screenOffEvent);
    EXPECT_FALSE(g_inactiveCallback);
    EXPECT_TRUE(powerMgrClient.IsScreenOn());
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);

    CommonEventManager::UnSubscribeCommonEvent(subscriber);
    powerMgrClient.UnRegisterPowerStateCallback(stateCallback);
    POWER_HILOGI(LABEL_TEST, "PowerCoordinationLockTest_010 end");
}

#ifdef HAS_MULTIMODALINPUT_INPUT_PART
/**
 * @tc.name: PowerCoordinationLockTest_011
 * @tc.desc: test entering DIM state while coordination
 * @tc.type: FUNC
 * @tc.require: issueI8JBT4
 */
HWTEST_F (PowerCoordinationLockTest, PowerCoordinationLockTest_011, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "PowerCoordinationLockTest_011 start");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    shared_ptr<PowerStateCommonEventSubscriber> subscriber = PowerStateCommonEventSubscriber::RegisterEvent();
    EXPECT_FALSE(subscriber == nullptr);
    auto runninglock =
        powerMgrClient.CreateRunningLock("PowerCoordinationLockTest_010", RunningLockType::RUNNINGLOCK_COORDINATION);
    ASSERT_NE(runninglock, nullptr);
    EXPECT_FALSE(runninglock->IsUsed());
    powerMgrClient.WakeupDevice();
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);
    powerMgrClient.OverrideScreenOffTime(OVER_TIME_SCREEN_OFF_TIME_MS);
    runninglock->Lock();
    EXPECT_TRUE(runninglock->IsUsed());

    auto inputManager = MMI::InputManager::GetInstance();

    std::shared_ptr<MMI::KeyEvent> keyEvent = CreateKeyEvent();
    inputManager->SimulateInputEvent(keyEvent);
    usleep((WAIT_EVENT_TIME_MS + WAIT_STATE_TIME_MS) * US_PER_MS);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::DIM);
    usleep(OVER_TIME_SCREEN_OFF_TIME_TEST_MS * US_PER_MS);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::DIM);
    // already in DIM, not resetting 10s timer
    inputManager->SimulateInputEvent(keyEvent);
    usleep(SCREEN_OFF_TIME_OVERRIDE_COORDINATION_MS / 2 * US_PER_MS);
    // already in DIM, not resetting 10s timer
    inputManager->SimulateInputEvent(keyEvent);
    usleep(SCREEN_OFF_TIME_OVERRIDE_COORDINATION_MS / 2 * US_PER_MS);
    // screen should be off now
    EXPECT_FALSE(powerMgrClient.IsScreenOn());

    powerMgrClient.WakeupDevice();
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);
    inputManager->SimulateInputEvent(keyEvent);
    usleep(WAIT_EVENT_TIME_MS * US_PER_MS);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::DIM);
    ResetTriggeredFlag();
    powerMgrClient.WakeupDevice();
    // DIM to AWAKE, no event
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);
    EXPECT_FALSE(g_screenOnEvent);
    // AWAKE to AWAKE, no event
    powerMgrClient.WakeupDevice();
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::AWAKE);
    EXPECT_FALSE(g_screenOnEvent);

    // test pointer event
    std::shared_ptr<PointerEvent> pointerEvent = CreatePointerEvent();
    powerMgrClient.WakeupDevice();
    inputManager->SimulateInputEvent(pointerEvent);
    usleep((WAIT_EVENT_TIME_MS + WAIT_STATE_TIME_MS) * US_PER_MS);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::DIM);
    usleep(OVER_TIME_SCREEN_OFF_TIME_TEST_MS * US_PER_MS);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::DIM);
    usleep(SCREEN_OFF_TIME_OVERRIDE_COORDINATION_MS * US_PER_MS);
    EXPECT_EQ(powerMgrClient.GetState(), PowerState::INACTIVE);

    powerMgrClient.RestoreScreenOffTime();
    CommonEventManager::UnSubscribeCommonEvent(subscriber);
}
#endif
}
