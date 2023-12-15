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

#ifndef POWERMGR_POWER_MANAGER_SHUTDOWN_DIALOG_H
#define POWERMGR_POWER_MANAGER_SHUTDOWN_DIALOG_H

#include <mutex>

#include "ability_connect_callback_interface.h"
#include "ability_connect_callback_stub.h"
#include "ffrt_utils.h"

namespace OHOS {
namespace PowerMgr {
class ShutdownDialog {
public:
    ShutdownDialog();
    ~ShutdownDialog();
    void KeyMonitorInit();
    void KeyMonitorCancel();
    bool ConnectSystemUi();
    bool IsLongPress() const;
    void ResetLongPressFlag();
    void StartVibrator();
    void LoadDialogConfig();
    static std::string GetBundleName()
    {
        return bundleName_;
    }

    static std::string GetAbilityName()
    {
        return abilityName_;
    }

    static std::string GetUiExtensionType()
    {
        return uiExtensionType_;
    }
private:
    class DialogAbilityConnection : public OHOS::AAFwk::AbilityConnectionStub {
    public:
        void OnAbilityConnectDone(
            const AppExecFwk::ElementName& element, const sptr<IRemoteObject>& remoteObject, int resultCode) override;
        void OnAbilityDisconnectDone(const AppExecFwk::ElementName& element, int resultCode) override;

    private:
        std::mutex mutex_;
    };

    int32_t longPressId_ {0};
    sptr<OHOS::AAFwk::IAbilityConnection> dialogConnectionCallback_ {nullptr};
    FFRTQueue queue_ {"shutdown_dialog"};
    static std::string bundleName_;
    static std::string abilityName_;
    static std::string uiExtensionType_;
};
} // namespace PowerMgr
} // namespace OHOS

#endif // POWERMGR_POWER_MANAGER_SHUTDOWN_DIALOG_H
