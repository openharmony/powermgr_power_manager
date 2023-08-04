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
#include "power_service_mock_test.h"
#include <fstream>
#include <thread>
#include <unistd.h>

#ifdef POWERMGR_GTEST
#define private   public
#define protected public
#endif

#include <datetime_ex.h>
#include <input_manager.h>
#include <securec.h>

#include "permission.h"
#include "power_mgr_service.h"
#include "power_state_callback_stub.h"
#include "power_state_machine.h"
#include "setting_helper.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;
static constexpr int SLEEP_WAIT_TIME_S = 2;

class InputCallbackMock : public IInputEventConsumer {
public:
    virtual void OnInputEvent(std::shared_ptr<KeyEvent> keyEvent) const;
    virtual void OnInputEvent(std::shared_ptr<PointerEvent> pointerEvent) const;
    virtual void OnInputEvent(std::shared_ptr<AxisEvent> axisEvent) const;
};

template <>
sptr<PowerMgrService> DelayedSpSingleton<PowerMgrService>::GetInstance()
{
    instance_ = nullptr;
    return instance_;
}

bool Permission::IsSystem()
{
    return true;
}

bool PowerKeySuspendMonitor::Init()
{
    if (powerkeyReleaseId_ >= 0) {
        return true;
    }
    std::shared_ptr<OHOS::MMI::KeyOption> keyOption = std::make_shared<OHOS::MMI::KeyOption>();
    std::set<int32_t> preKeys;

    keyOption.reset();
    keyOption = std::make_shared<OHOS::MMI::KeyOption>();
    keyOption->SetPreKeys(preKeys);
    keyOption->SetFinalKey(OHOS::MMI::KeyEvent::KEYCODE_POWER);
    keyOption->SetFinalKeyDown(false);
    keyOption->SetFinalKeyDownDuration(0);
    powerkeyReleaseId_ = InputManager::GetInstance()->SubscribeKeyEvent(
        keyOption, [this](std::shared_ptr<OHOS::MMI::KeyEvent> keyEvent) {
            POWER_HILOGI(FEATURE_SUSPEND, "Receive key on notify");
            auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
            if (pms == nullptr) {
                return;
            }
            std::shared_ptr<SuspendController> suspendController = pms->GetSuspendController();
            if (suspendController->GetPowerkeyDownWhenScreenOff()) {
                POWER_HILOGI(FEATURE_SUSPEND, "no action suspend");
                return;
            }
            Notify();
        });
    POWER_HILOGI(FEATURE_SUSPEND, "powerkeyReleaseId_=%{public}d", powerkeyReleaseId_);
    return powerkeyReleaseId_ >= 0 ? true : false;
}

namespace {
MMI::PointerEvent::PointerItem CreatePointerItem(
    int32_t pointerId, int32_t deviceId, const std::pair<int32_t, int32_t>& displayLocation, bool isPressed)
{
    MMI::PointerEvent::PointerItem item;
    item.SetPointerId(pointerId);
    item.SetDeviceId(deviceId);
    item.SetDisplayX(displayLocation.first);
    item.SetDisplayY(displayLocation.second);
    item.SetPressed(isPressed);
    return item;
}
/**
 * @tc.name: PowerServiceMockTest001
 * @tc.desc: test GetInstance(exception)
 * @tc.type: FUNC
 * @tc.require: issueI7G6OY
 */
HWTEST_F(PowerServiceMockTest, PowerServiceMockTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerServiceMockTest001: start";
    std::shared_ptr<PowerStateMachine> stateMachine = nullptr;
    std::shared_ptr<WakeupController> wakeupController_ = std::make_shared<WakeupController>(stateMachine);
    wakeupController_->Wakeup();
    auto pmsTest1 = DelayedSpSingleton<PowerMgrService>::GetInstance();
    EXPECT_TRUE(pmsTest1 == nullptr);

    wakeupController_->ControlListener(WakeupDeviceType::WAKEUP_DEVICE_APPLICATION);
    auto pmsTest2 = DelayedSpSingleton<PowerMgrService>::GetInstance();
    EXPECT_TRUE(pmsTest2 == nullptr);

    InputCallback* callback = new InputCallback();
    InputCallbackMock* callback_mock = reinterpret_cast<InputCallbackMock*>(callback);
    std::shared_ptr<OHOS::MMI::KeyEvent> keyEvent = OHOS::MMI::KeyEvent::Create();
    keyEvent->SetKeyAction(MMI::KeyEvent::KEY_ACTION_DOWN);
    keyEvent->SetKeyCode(OHOS::MMI::KeyEvent::KEYCODE_F1);
    callback_mock->OnInputEvent(keyEvent);
    auto pmsTest3 = DelayedSpSingleton<PowerMgrService>::GetInstance();
    EXPECT_TRUE(pmsTest3 == nullptr);

    constexpr int32_t DRAG_DST_X {500};
    constexpr int32_t DRAG_DST_Y {500};
    int32_t deviceMouseId {0};
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    MMI::PointerEvent::PointerItem curPointerItem;
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_DOWN);
    pointerEvent->SetPointerId(0);
    pointerEvent->SetSourceType(PointerEvent::SOURCE_TYPE_MOUSE);
    curPointerItem = CreatePointerItem(0, deviceMouseId, {DRAG_DST_X, DRAG_DST_Y}, true);
    pointerEvent->AddPointerItem(curPointerItem);
    callback_mock->OnInputEvent(pointerEvent);
    delete callback;
    auto pmsTest4 = DelayedSpSingleton<PowerMgrService>::GetInstance();
    EXPECT_TRUE(pmsTest4 == nullptr);

    GTEST_LOG_(INFO) << "PowerServiceMockTest001:  end";
}

/**
 * @tc.name: PowerServiceMockTest002
 * @tc.desc: test ControlListener(exception)
 * @tc.type: FUNC
 * @tc.require: issueI7G6OY
 */
HWTEST_F(PowerServiceMockTest, PowerServiceMockTest002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PowerServiceMockTest002: start";
    sleep(SLEEP_WAIT_TIME_S);
    std::shared_ptr<ShutdownController> shutdownController = nullptr;
    std::shared_ptr<PowerStateMachine> stateMachine = nullptr;
    std::shared_ptr<SuspendController> suspendController_ =
        std::make_shared<SuspendController>(shutdownController, stateMachine);
    suspendController_->ControlListener(SuspendDeviceType::SUSPEND_DEVICE_REASON_POWER_KEY, 1, 0);
    auto pmsTest1 = DelayedSpSingleton<PowerMgrService>::GetInstance();
    EXPECT_TRUE(pmsTest1 == nullptr);

    GTEST_LOG_(INFO) << "PowerServiceMockTest002:  end";
}
} // namespace