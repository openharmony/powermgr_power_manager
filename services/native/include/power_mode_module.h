/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef POWER_MODE_THREAD_H
#define POWER_MODE_THREAD_H

#include <map>
#include <set>
#include "ipc_skeleton.h"
#include <common_event_data.h>
#include <common_event_manager.h>
#include <common_event_publish_info.h>
#include <common_event_support.h>

#include "ipower_mode_callback.h"
#include "power_errors.h"

#define FLAG_FALSE (-1)
#define LAST_MODE_FLAG 0
#define SETTINGS_PRIVIDER_VALUE_LCD_BRIGHTNESS 99
#define SETTINGS_PRIVIDER_VALUE_VIBRATION 1
#define SETTINGS_PRIVIDER_VALUE_ROTATION 1

namespace OHOS {
namespace PowerMgr {
class PowerModeModule {
public:
    enum SocPerformance {
        STANDARD = 0,
        HIGH = 1,
    };
    PowerModeModule();
    ~PowerModeModule() = default;
    void InitPowerMode();
    PowerErrors SetModeItem(PowerMode mode);
    PowerMode GetModeItem();
    void EnableMode(PowerMode mode, bool isBoot = false);
    void AddPowerModeCallback(const sptr<IPowerModeCallback>& callback);
    void DelPowerModeCallback(const sptr<IPowerModeCallback>& callback);

private:
    using IntentWant = OHOS::AAFwk::Want;

    class CallbackManager : public IRemoteObject::DeathRecipient {
    public:
        void OnRemoteDied(const wptr<IRemoteObject>& remote) override;
        void AddCallback(const sptr<IPowerModeCallback>& callback);
        void RemoveCallback(const sptr<IPowerModeCallback>& callback);
        void WaitingCallback();

    private:
        void AddCallbackPidUid(const sptr<IRemoteObject>& callback);
        void RemoveCallbackPidUid(const sptr<IRemoteObject>& callback);
        std::pair<int32_t, int32_t> FindCallbackPidUid(const sptr<IRemoteObject>& callback);
        std::mutex mutex_;
        std::set<sptr<IRemoteObject>> callbacks_;
        std::map<sptr<IRemoteObject>, std::pair<int32_t, int32_t>> cachedRegister_;
    };

    PowerMode mode_;
    uint32_t lastMode_;
    bool observerRegisted_ = false;
    std::map<PowerMode, std::set<PowerMode>> forbidMap_;

    void Prepare();
    void PublishPowerModeEvent();
    void UnregisterSaveModeObserver();
    void RegisterSaveModeObserver();
    void RegisterAutoAdjustBrightnessObserver();
    void RegisterAutoWindowRotationObserver();
    void RegisterVibrateStateObserver();
    void RegisterIntellVoiceObserver();
    void InitPowerModeTransitMap();
    bool IsBootNeedActions();
    bool CanTransitTo(PowerMode from, PowerMode to);

    sptr<CallbackManager> callbackMgr_;
    void UpdateModepolicy();
    void RunAction(bool isBoot);
    static void SetDisplayOffTime(bool isBoot);
    static void SetSleepTime([[maybe_unused]] bool isBoot);
    static void SetAutoAdjustBrightness(bool isBoot);
    static void SetLcdBrightness(bool isBoot);
    static void SetVibration(bool isBoot);
    static void SetWindowRotation(bool isBoot);
    static void SetIntellVoiceState(bool isBoot);
    static void SetSocPerfState([[maybe_unused]]bool isBoot);

    std::atomic<bool> started_;
    std::mutex mutex_;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWER_MODE_THREAD_H
