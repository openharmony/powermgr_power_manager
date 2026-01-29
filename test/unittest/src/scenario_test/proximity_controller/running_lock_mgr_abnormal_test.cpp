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

#include "power_log.h"
#define private public
#include "power_mgr_service.h"
#include "death_recipient_manager.h"
#undef private
#include "power_hookmgr.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

namespace {
constexpr uint32_t AWAY_PROXIMITY_ACTION = 0;
constexpr uint32_t CLOSE_PROXIMITY_ACTION = 1;
constexpr uint32_t INVALID_PROXIMITY_ACTION = 2;
sptr<PowerMgrService> g_pmsTest {nullptr};
std::shared_ptr<IProximityController> g_controller {nullptr};
std::function<void(uint32_t, bool)> g_action {nullptr};
bool g_isRealInstance = false;
void SetInstanceReal(bool real = true)
{
    g_isRealInstance = real;
}
} // namespace

#ifdef POWER_MANAGER_PROXIMITY_CONTROLLER_OVERRIDE
int HookMgrExecute(HOOK_MGR* hookMgr, int stage, void* context, const HOOK_EXEC_OPTIONS* options)
{
    if (static_cast<PowerHookStage>(stage) == PowerHookStage::POWER_PROXIMITY_CONTROLLER_INIT) {
        ProximityControllerContext* proximityControllerContext = static_cast<ProximityControllerContext*>(context);
        proximityControllerContext->controllerPtr = g_controller;
        g_action = proximityControllerContext->action;
    }
    POWER_HILOGI(LABEL_TEST, "Mock HookMgrExecute stage is %{public}d", stage);
    return 0;
}
#endif

namespace OHOS {
namespace PowerMgr {
template<> sptr<PowerMgrService> DelayedSpSingleton<PowerMgrService>::instance_;

template<> sptr<PowerMgrService> DelayedSpSingleton<PowerMgrService>::GetInstance()
{
    if (g_isRealInstance) {
        if (!instance_) {
            std::lock_guard<std::mutex> lock(mutex_);
            if (instance_ == nullptr) {
                instance_ = new PowerMgrService();
            }
        }
        POWER_HILOGI(LABEL_TEST, "Power service 1");
        return instance_;
    } else {
        POWER_HILOGI(LABEL_TEST, "Power service 0");
        return nullptr;
    }
}

template<> void DelayedSpSingleton<PowerMgrService>::DestroyInstance()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (instance_) {
        instance_.clear();
        instance_ = nullptr;
    }
}

class RunningLockMgrAbnormalTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void RunningLockMgrAbnormalTest::SetUpTestCase()
{
    g_pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    ASSERT_EQ(g_pmsTest, nullptr);
}

void RunningLockMgrAbnormalTest::TearDownTestCase()
{
}

void RunningLockMgrAbnormalTest::SetUp()
{
}

void RunningLockMgrAbnormalTest::TearDown()
{
    DelayedSpSingleton<PowerMgrService>::DestroyInstance();
    g_pmsTest = nullptr;
    g_isRealInstance = false;
    g_controller = nullptr;
    g_action = nullptr;
}
} // namespace PowerMgr
} // namespace OHOS

namespace {
/**
 * @tc.name: RunningLockMgrAbnormalTest001
 * @tc.desc: Test proximity running lock action HandleProximityAwayEvent abnormal
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
HWTEST_F(RunningLockMgrAbnormalTest, RunningLockMgrAbnormalTest001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockMgrAbnormalTest001 function start!");
    std::shared_ptr<RunningLockMgr> lockMgr = std::make_shared<RunningLockMgr>(nullptr);
    lockMgr->HandleProximityAwayEvent();
    EXPECT_EQ(g_pmsTest, nullptr);

    SetInstanceReal();
    g_pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    lockMgr = std::make_shared<RunningLockMgr>(nullptr);
    lockMgr->HandleProximityAwayEvent();
    EXPECT_NE(g_pmsTest, nullptr);

    g_pmsTest->powerStateMachine_ = std::make_shared<PowerStateMachine>(nullptr);
    lockMgr->HandleProximityAwayEvent();
    EXPECT_NE(g_pmsTest->GetPowerStateMachine(), nullptr);
    POWER_HILOGI(LABEL_TEST, "RunningLockMgrAbnormalTest001 function end!");
}

/**
 * @tc.name: RunningLockMgrAbnormalTest002
 * @tc.desc: Test proximity running lock action HandleProximityCloseEvent abnormal
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
HWTEST_F(RunningLockMgrAbnormalTest, RunningLockMgrAbnormalTest002, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockMgrAbnormalTest002 function start!");
    std::shared_ptr<RunningLockMgr> lockMgr = std::make_shared<RunningLockMgr>(nullptr);
    lockMgr->HandleProximityCloseEvent();
    EXPECT_EQ(g_pmsTest, nullptr);

    SetInstanceReal();
    g_pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    lockMgr = std::make_shared<RunningLockMgr>(nullptr);
    lockMgr->HandleProximityCloseEvent();
    EXPECT_NE(g_pmsTest, nullptr);
    EXPECT_EQ(g_pmsTest->powerStateMachine_, nullptr);

    g_pmsTest->powerStateMachine_ = std::make_shared<PowerStateMachine>(nullptr);
    lockMgr->HandleProximityCloseEvent();
    EXPECT_NE(g_pmsTest->GetPowerStateMachine(), nullptr);
    POWER_HILOGI(LABEL_TEST, "RunningLockMgrAbnormalTest002 function end!");
}

/**
 * @tc.name: RunningLockMgrAbnormalTest003
 * @tc.desc: Test proximity running lock action InitProximityController
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
HWTEST_F(RunningLockMgrAbnormalTest, RunningLockMgrAbnormalTest003, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockMgrAbnormalTest003 function start!");
    std::shared_ptr<RunningLockMgr> lockMgr = std::make_shared<RunningLockMgr>(nullptr);
    bool ret = lockMgr->InitProximityController();
    EXPECT_TRUE(lockMgr->proximityController_ != nullptr);
    
    lockMgr = std::make_shared<RunningLockMgr>(nullptr);
    g_controller = std::make_shared<RunningLockMgr::ProximityController>();
    ret = lockMgr->InitProximityController();
    EXPECT_TRUE(lockMgr->proximityController_ != nullptr);

    ret = lockMgr->InitProximityController();
    EXPECT_TRUE(ret);
    POWER_HILOGI(LABEL_TEST, "RunningLockMgrAbnormalTest003 function end!");
}

/**
 * @tc.name: RunningLockMgrAbnormalTest004
 * @tc.desc: Test proximity running lock action ProximityController OnAway abnormal
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
HWTEST_F(RunningLockMgrAbnormalTest, RunningLockMgrAbnormalTest004, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockMgrAbnormalTest004 function start!");
    std::shared_ptr<RunningLockMgr> lockMgr = std::make_shared<RunningLockMgr>(nullptr);
    lockMgr->InitProximityController();
    lockMgr->proximityController_->SetEnabled(false);
    lockMgr->proximityController_->OnAway();
    EXPECT_FALSE(lockMgr->proximityController_->IsClose());

    lockMgr->proximityController_->SetEnabled(true);
    lockMgr->proximityController_->SetClose(false);
    lockMgr->proximityController_->OnAway();
    EXPECT_FALSE(lockMgr->proximityController_->IsClose());

    lockMgr->proximityController_->SetClose(true);
    lockMgr->proximityController_->OnAway();
    EXPECT_TRUE(g_pmsTest == nullptr);
    EXPECT_FALSE(lockMgr->proximityController_->IsClose());

    lockMgr->proximityController_->SetClose(true);
    SetInstanceReal();
    g_pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    lockMgr->proximityController_->OnAway();
    EXPECT_TRUE(g_pmsTest != nullptr);
    EXPECT_FALSE(lockMgr->proximityController_->IsClose());

    lockMgr->proximityController_->SetClose(true);
    // for HandleProximityAwayEvent branch (powerStateMachine_ != nullptr)
    g_pmsTest->powerStateMachine_ = std::make_shared<PowerStateMachine>(nullptr);
    lockMgr->proximityController_->OnAway();
    EXPECT_FALSE(lockMgr->proximityController_->IsClose());
    POWER_HILOGI(LABEL_TEST, "RunningLockMgrAbnormalTest004 function end!");
}

/**
 * @tc.name: RunningLockMgrAbnormalTest005
 * @tc.desc: Test proximity running lock action ProximityController OnClose abnormal
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
HWTEST_F(RunningLockMgrAbnormalTest, RunningLockMgrAbnormalTest005, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockMgrAbnormalTest005 function start!");
    std::shared_ptr<RunningLockMgr> lockMgr = std::make_shared<RunningLockMgr>(nullptr);
    lockMgr->InitProximityController();
    lockMgr->proximityController_->SetEnabled(false);
    lockMgr->proximityController_->OnClose();
    EXPECT_FALSE(lockMgr->proximityController_->IsClose());

    lockMgr->proximityController_->SetEnabled(true);
    lockMgr->proximityController_->SetClose(true);
    lockMgr->proximityController_->OnClose();
    EXPECT_TRUE(lockMgr->proximityController_->IsClose());

    lockMgr->proximityController_->SetClose(false);
    lockMgr->proximityController_->OnClose();
    EXPECT_TRUE(g_pmsTest == nullptr);
    EXPECT_TRUE(lockMgr->proximityController_->IsClose());

    lockMgr->proximityController_->SetClose(false);
    SetInstanceReal();
    g_pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    lockMgr->proximityController_->OnClose();
    EXPECT_TRUE(g_pmsTest != nullptr);
    EXPECT_TRUE(lockMgr->proximityController_->IsClose());

    lockMgr->proximityController_->SetClose(false);
    // for HandleProximityCloseEvent branch (powerStateMachine_ != nullptr)
    g_pmsTest->powerStateMachine_ = std::make_shared<PowerStateMachine>(nullptr);
    lockMgr->proximityController_->OnClose();
    EXPECT_TRUE(lockMgr->proximityController_->IsClose());
    POWER_HILOGI(LABEL_TEST, "RunningLockMgrAbnormalTest005 function end!");
}

/**
 * @tc.name: RunningLockMgrAbnormalTest006
 * @tc.desc: Test proximity running lock action ProximityController RecordSensorCallback abnormal(pms == nullptr)
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
HWTEST_F(RunningLockMgrAbnormalTest, RunningLockMgrAbnormalTest006, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockMgrAbnormalTest006 function start!");
    SensorEvent event = {.sensorTypeId = SENSOR_TYPE_ID_PROXIMITY};
    RunningLockMgr::ProximityController::RecordSensorCallback(&event);
    EXPECT_TRUE(g_pmsTest == nullptr);

    SetInstanceReal();
    g_pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    RunningLockMgr::ProximityController::RecordSensorCallback(&event);
    EXPECT_TRUE(g_pmsTest->GetRunningLockMgr() == nullptr);
    POWER_HILOGI(LABEL_TEST, "RunningLockMgrAbnormalTest006 function end!");
}

/**
 * @tc.name: RunningLockMgrAbnormalTest007
 * @tc.desc: Test proximity running lock action InitLocksTypeProximity abnormal(activate_ lambda)
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
HWTEST_F(RunningLockMgrAbnormalTest, RunningLockMgrAbnormalTest007, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockMgrAbnormalTest007 function start!");
    std::shared_ptr<RunningLockMgr> lockMgr = std::make_shared<RunningLockMgr>(nullptr);
    lockMgr->InitLocksTypeProximity();
    RunningLockParam lockParam;
    auto iterator = lockMgr->lockCounters_.find(RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL);
    if (iterator != lockMgr->lockCounters_.end()) {
        iterator->second->activate_(false, lockParam);
        SetInstanceReal();
        g_pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
        iterator->second->activate_(false, lockParam);
        EXPECT_FALSE(lockMgr->proximityController_->IsEnabled());
    } else {
        GTEST_FAIL();
    }
    POWER_HILOGI(LABEL_TEST, "RunningLockMgrAbnormalTest007 function end!");
}

/**
 * @tc.name: RunningLockMgrAbnormalTest008
 * @tc.desc: Test proximity running lock action Lambda action
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
#ifdef POWER_MANAGER_PROXIMITY_CONTROLLER_OVERRIDE
HWTEST_F(RunningLockMgrAbnormalTest, RunningLockMgrAbnormalTest008, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockMgrAbnormalTest008 function start!");
    g_controller = std::make_shared<RunningLockMgr::ProximityController>();
    std::shared_ptr<RunningLockMgr> lockMgr = std::make_shared<RunningLockMgr>(nullptr);
    lockMgr->InitLocksTypeProximity();
    ASSERT_TRUE(g_action != nullptr) << "g_action == nullptr";

    g_action(INVALID_PROXIMITY_ACTION, true);
    EXPECT_TRUE(g_pmsTest == nullptr);

    SetInstanceReal();
    g_pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    EXPECT_TRUE(g_pmsTest != nullptr);
    g_action(INVALID_PROXIMITY_ACTION, true);

    g_pmsTest->runningLockMgr_ = lockMgr;
    EXPECT_TRUE(g_pmsTest->GetRunningLockMgr() != nullptr);
    g_action(INVALID_PROXIMITY_ACTION, true);

    g_action(CLOSE_PROXIMITY_ACTION, true);

    g_action(AWAY_PROXIMITY_ACTION, true);
    EXPECT_FALSE(lockMgr->IsProximityClose());
    POWER_HILOGI(LABEL_TEST, "RunningLockMgrAbnormalTest008 function end!");
}
#endif

/**
 * @tc.name: RunningLockMgrAbnormalTest010
 * @tc.desc: Test IsVoiceAppForeground
 * @tc.type: FUNC
 * @tc.require: ICGV1M
 */
HWTEST_F(RunningLockMgrAbnormalTest, RunningLockMgrAbnormalTest010, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockMgrAbnormalTest010 function start!");
    std::shared_ptr<RunningLockMgr> lockMgr = std::make_shared<RunningLockMgr>(nullptr);
    bool ret = lockMgr->IsVoiceAppForeground();
    EXPECT_FALSE(ret);
    POWER_HILOGI(LABEL_TEST, "RunningLockMgrAbnormalTest010 function end!");
}

/**
 * @tc.name: RunningLockMgrAbnormalTest011
 * @tc.desc: Test PowerStateMachine HandleProximityClose abnormal
 * @tc.type: FUNC
 * @tc.require: issues#1567
 */
HWTEST_F(RunningLockMgrAbnormalTest, RunningLockMgrAbnormalTest011, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockMgrAbnormalTest011 function start!");
    g_pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    auto stateMachine = std::make_shared<PowerStateMachine>(g_pmsTest);
    stateMachine->HandleProximityClose();
    EXPECT_TRUE(g_pmsTest == nullptr);
    SetInstanceReal();
    g_pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    stateMachine = std::make_shared<PowerStateMachine>(g_pmsTest);
    stateMachine->HandleProximityClose();
    EXPECT_TRUE(g_pmsTest != nullptr && g_pmsTest->GetSuspendController() == nullptr);
    POWER_HILOGI(LABEL_TEST, "RunningLockMgrAbnormalTest011 function end!");
}

/**
 * @tc.name: PowerNullPointerTest001
 * @tc.desc: Test SetProxFilteringStrategy pms == nullptr \ stateMachine == nullptr
 * @tc.type: FUNC
 */
HWTEST_F(RunningLockMgrAbnormalTest, PowerNullPointerTest001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerNullPointerTest001 function start!");
    SetInstanceReal();
    g_pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    EXPECT_TRUE(g_pmsTest != nullptr && g_pmsTest->PowerStateMachineInit());
    sptr<IRemoteObject> obj = sptr<IPCObjectProxy>::MakeSptr(0, u"PowerNullPointerTest001");
    g_pmsTest->SetProxFilteringStrategy(ProxFilteringStrategy::NOT_FILTERING, obj);
    auto it = DeathRecipientManager::GetInstance().clientDeathRecipientMap_.find(obj);
    EXPECT_TRUE(it != DeathRecipientManager::GetInstance().clientDeathRecipientMap_.end() && !it->second.empty());
    auto callbackFunc = (it->second.begin())->func;
    auto funcName = (it->second.begin())->funcName;
    EXPECT_TRUE(callbackFunc != nullptr && funcName == "SetProxFilteringStrategy");
    SetInstanceReal(false);
    callbackFunc(obj);
    EXPECT_TRUE(DelayedSpSingleton<PowerMgrService>::GetInstance() == nullptr);
    SetInstanceReal();
    g_pmsTest->powerStateMachine_.reset();
    callbackFunc(obj);
    EXPECT_TRUE(DelayedSpSingleton<PowerMgrService>::GetInstance() != nullptr &&
                DelayedSpSingleton<PowerMgrService>::GetInstance()->GetPowerStateMachine() == nullptr);
    POWER_HILOGI(LABEL_TEST, "PowerNullPointerTest001 function end!");
}
} // namespace