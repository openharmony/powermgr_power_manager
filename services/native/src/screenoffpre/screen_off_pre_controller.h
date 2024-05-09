/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef SCREEN_OFF_PRE_CONTROLLER_H
#define SCREEN_OFF_PRE_CONTROLLER_H

#include <mutex>
#include <set>

#include "iscreen_off_pre_callback.h"
#include "ffrt_utils.h"
#include "power_state_machine.h"

namespace OHOS {
namespace PowerMgr {

class ScreenOffPreController {
public:
    explicit ScreenOffPreController(std::shared_ptr<PowerStateMachine>& stateMachine);
    ~ScreenOffPreController() = default;
    void Init();
    bool IsRegistered();
    void SchedulEyeDetectTimeout(int64_t nextTimeOut, int64_t now);
    void AddScreenStateCallback(int32_t remainTime, const sptr<IScreenOffPreCallback>& callback);
    void DelScreenStateCallback(const sptr<IScreenOffPreCallback>& callback);
    void Reset();

private:
    class CallbackMgr : public IRemoteObject::DeathRecipient {
    public:
        void OnRemoteDied(const wptr<IRemoteObject>& remote) override;
        void AddCallback(const sptr<IScreenOffPreCallback>& callback);
        void RemoveCallback(const sptr<IScreenOffPreCallback>& callback);
        void Notify();

    private:
        std::mutex mutex_;
        std::set<sptr<IRemoteObject>> callbackSet_;
    };

    int32_t remainTime_ = 0;
    bool isRegistered_ = false;
    sptr<CallbackMgr> callbackMgr_;
    std::mutex mutexLock_;
    std::mutex ffrtMutex_;
    std::shared_ptr<PowerStateMachine> powerStateMachine_;
    std::shared_ptr<FFRTQueue> queue_;
    FFRTHandle eyeDetectTimeOutHandle_ {nullptr};
    void TriggerCallback();
    bool NeedEyeDetectLocked(int64_t nextEyeDetectTime);
    void HandleEyeDetectTimeout(int64_t delayTime);
    void CancelEyeDetectTimeout();
};

} // namespace PowerMgr
} // namespace OHOS

#endif // SCREEN_OFF_PRE_CONTROLLER_H