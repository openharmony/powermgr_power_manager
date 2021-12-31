/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include <common_event_data.h>
#include <common_event_manager.h>
#include <common_event_publish_info.h>
#include <common_event_support.h>

#include "ipower_mode_callback.h"

#define FLAG_FALSE (-1)
#define LAST_MODE_FLAG 0
#define SETTINGS_PRIVIDER_VALUE_LCD_BRIGHTNESS 99
#define SETTINGS_PRIVIDER_VALUE_VIBRATION 1
#define SETTINGS_PRIVIDER_VALUE_ROTATION 1

namespace OHOS {
namespace PowerMgr {
class PowerModeModule {
public:
    enum {
        POWER_MODE_MIN = 600,
        DEFAULT_MODE = POWER_MODE_MIN,
        EXTREAM_MODE,
        NORMAL_MODE,
        SAVE_MODE,
        LOWPOWER_MODE,
        POWER_MODE_MAX = LOWPOWER_MODE
    };
    PowerModeModule();
    ~PowerModeModule() = default;
    void SetModeItem(uint32_t mode);
    uint32_t GetModeItem();
    void EnableMode(uint32_t mode);
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
        std::mutex mutex_;
        std::set<sptr<IRemoteObject>> callbacks_;
    };

    uint32_t mode_;
    uint32_t lastMode_;

    void Prepare();
    void PublishPowerModeEvent();

    CallbackManager callbackMgr_;
    void UpdateModepolicy();
    void RunAction();
    void SetDisplayOffTime();
    void SetSleepTime();
    void SetAutoAdjustBrightness();
    void SetLcdBrightness();
    void SetVibration();
    void OnOffRotation();

    std::atomic<bool> started_;
    std::map<uint32_t, int32_t> recoverValue;
    std::map<uint32_t, int32_t>::iterator recoverValueiter;
    std::mutex mutex_;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWER_MODE_THREAD_H
