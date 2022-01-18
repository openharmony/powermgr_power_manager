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

#include "power_mgr_service.h"

#include <datetime_ex.h>
#include <file_ex.h>
#include <hisysevent.h>
#include <if_system_ability_manager.h>
#include "input_manager.h"
#include <ipc_skeleton.h>
#include <iservice_registry.h>
#include <string_ex.h>
#include <system_ability_definition.h>
#include <unistd.h>

#include "permission.h"
#include "power_common.h"
#include "power_mgr_dumper.h"

namespace OHOS {
namespace PowerMgr {
namespace {
const std::string POWERMGR_SERVICE_NAME = "PowerMgrService";
const std::string TASK_RUNNINGLOCK_UNLOCK = "RunningLock_UnLock";
const std::string REASON_POWER_KEY = "power_key";
constexpr int APP_FIRST_UID = APP_FIRST_UID_VALUE;
auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
const bool G_REGISTER_RESULT = SystemAbility::MakeAndRegisterAbility(pms.GetRefPtr());
}

using namespace MMI;

PowerMgrService::PowerMgrService() : SystemAbility(POWER_MANAGER_SERVICE_ID, true) {}

PowerMgrService::~PowerMgrService() {}

void PowerMgrService::OnStart()
{
    POWER_HILOGI(MODULE_SERVICE, "OnStart enter.");
    if (ready_) {
        POWER_HILOGE(MODULE_SERVICE, "OnStart is ready, nothing to do.");
        return;
    }

    if (!Init()) {
        POWER_HILOGE(MODULE_SERVICE, "OnStart call init fail");
        return;
    }
    if (!Publish(DelayedSpSingleton<PowerMgrService>::GetInstance())) {
        POWER_HILOGE(MODULE_SERVICE, "OnStart register to system ability manager failed.");
        return;
    }
    ready_ = true;
    POWER_HILOGI(MODULE_SERVICE, "OnStart and add system ability success.");
}

bool PowerMgrService::Init()
{
    POWER_HILOGI(MODULE_SERVICE, "Init start");

    if (!eventRunner_) {
        eventRunner_ = AppExecFwk::EventRunner::Create(POWERMGR_SERVICE_NAME);
        if (eventRunner_ == nullptr) {
            POWER_HILOGE(MODULE_SERVICE, "Init failed due to create EventRunner");
            return false;
        }
    }

    if (!handler_) {
        handler_ = std::make_shared<PowermsEventHandler>(eventRunner_, pms);
    }

    if (!runningLockMgr_) {
        runningLockMgr_ = std::make_shared<RunningLockMgr>(pms);
    }
    if (!runningLockMgr_->Init()) {
        POWER_HILOGE(MODULE_SERVICE, "OnStart init fail");
        return false;
    }
    if (!PowerStateMachineInit()) {
        POWER_HILOGE(MODULE_SERVICE, "power state machine init fail!");
    }
    if (DelayedSpSingleton<PowerSaveMode>::GetInstance()) {
        powerModeModule_.SetModeItem(PowerModeModule::NORMAL_MODE);
    } else {
        POWER_HILOGE(MODULE_SERVICE, "power mode init fail!");
    }
    KeyMonitorInit();
    POWER_HILOGI(MODULE_SERVICE, "Init success");
    return true;
}

bool PowerMgrService::PowerStateMachineInit()
{
    if (powerStateMachine_ == nullptr) {
        powerStateMachine_ = std::make_shared<PowerStateMachine>(pms);
        if (!(powerStateMachine_->Init())) {
            POWER_HILOGE(MODULE_SERVICE, "power state machine start fail!");
            return false;
        }
    }
    if (powerMgrNotify_ == nullptr) {
        powerMgrNotify_ = std::make_shared<PowerMgrNotify>();
        powerMgrNotify_->RegisterPublishEvents();
    }
    return true;
}

class InputCallback : public IInputEventConsumer {
    virtual void OnInputEvent(std::shared_ptr<KeyEvent> keyEvent) const;
    virtual void OnInputEvent(std::shared_ptr<PointerEvent> pointerEvent) const;
    virtual void OnInputEvent(std::shared_ptr<AxisEvent> axisEvent) const;
};

void InputCallback::OnInputEvent(std::shared_ptr<KeyEvent> keyEvent) const
{
    POWER_HILOGE(MODULE_SERVICE, "OnInputEvent keyEvent");
    // Do noting. It's done by AddMonitor callback
}

void InputCallback::OnInputEvent(std::shared_ptr<PointerEvent> pointerEvent) const
{
    POWER_HILOGE(MODULE_SERVICE, "OnInputEvent pointerEvent");
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        return;
    }
    pms->HandlePointEvent();
}

void InputCallback::OnInputEvent(std::shared_ptr<AxisEvent> axisEvent) const
{
    POWER_HILOGE(MODULE_SERVICE, "OnInputEvent axisEvent");
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        return;
    }
    pms->HandlePointEvent();
}

void PowerMgrService::KeyMonitorInit()
{
    std::shared_ptr<OHOS::MMI::KeyOption> keyOption = std::make_shared<OHOS::MMI::KeyOption>();
    std::vector<int32_t> preKeys;

    keyOption->SetPreKeys(preKeys);
    keyOption->SetFinalKey(OHOS::MMI::KeyEvent::KEYCODE_POWER);
    keyOption->SetFinalKeyDown(true);
    keyOption->SetFinalKeyDownDuration(LONG_PRESS_TIME);
    int32_t id = InputManager::GetInstance()->SubscribeKeyEvent(keyOption,
        [this](std::shared_ptr<OHOS::MMI::KeyEvent> keyEvent) {
            POWER_HILOGI(MODULE_SERVICE, "Receive long press powerkey");
            this->ShutDownDevice(REASON_POWER_KEY);
    });

    keyOption.reset();
    keyOption = std::make_shared<OHOS::MMI::KeyOption>();
    keyOption->SetPreKeys(preKeys);
    keyOption->SetFinalKey(OHOS::MMI::KeyEvent::KEYCODE_POWER);
    keyOption->SetFinalKeyDown(true);
    keyOption->SetFinalKeyDownDuration(0);
    id = InputManager::GetInstance()->SubscribeKeyEvent(keyOption,
        [this](std::shared_ptr<OHOS::MMI::KeyEvent> keyEvent) {
            POWER_HILOGI(MODULE_SERVICE, "Receive short press powerkey");
            powerkeyPressed_ = true;
            handler_->SendEvent(PowermsEventHandler::POWER_KEY_TIMEOUT_MSG, 0, POWER_KEY_PRESS_TIME);
    });

    keyOption.reset();
    keyOption = std::make_shared<OHOS::MMI::KeyOption>();
    keyOption->SetPreKeys(preKeys);
    keyOption->SetFinalKey(OHOS::MMI::KeyEvent::KEYCODE_POWER);
    keyOption->SetFinalKeyDown(false);
    id = InputManager::GetInstance()->SubscribeKeyEvent(keyOption,
        [this](std::shared_ptr<OHOS::MMI::KeyEvent> keyEvent) {
            powerkeyPressed_ = false;
            handler_->RemoveEvent(PowermsEventHandler::POWER_KEY_TIMEOUT_MSG);
            this->HandlePowerKeyUp();
    });

    InputManager::GetInstance()->AddMonitor([this](std::shared_ptr<KeyEvent> event) {
        this->HandleKeyEvent(event->GetKeyCode());
    });

    std::shared_ptr<InputCallback> callback = std::make_shared<InputCallback>();
    InputManager::GetInstance()->AddMonitor(std::static_pointer_cast<IInputEventConsumer>(callback));
}

void PowerMgrService::HandlePowerKeyUp()
{
    POWER_HILOGI(MODULE_SERVICE, "Receive release powerkey");

    if (this->shutdownService_.IsShuttingDown()) {
        POWER_HILOGI(MODULE_SERVICE, "System is shutting down");
        return;
    }
    int64_t now = static_cast<int64_t>(time(0));
    if (this->IsScreenOn()) {
        this->SuspendDevice(now, SuspendDeviceType::SUSPEND_DEVICE_REASON_POWER_BUTTON, false);
    } else {
        this->WakeupDevice(now, WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON, REASON_POWER_KEY);
    }
}

void PowerMgrService::HandleKeyEvent(int32_t keyCode)
{
    POWER_HILOGI(MODULE_SERVICE, "HandleKeyEvent: %{public}d", keyCode);
    int64_t now = static_cast<int64_t>(time(0));
    if (IsScreenOn()) {
        this->RefreshActivity(now, UserActivityType::USER_ACTIVITY_TYPE_BUTTON, false);
    } else {
        if (keyCode >= KeyEvent::KEYCODE_0
            && keyCode <= KeyEvent::KEYCODE_NUMPAD_RIGHT_PAREN) {
            POWER_HILOGI(MODULE_SERVICE, "wakeup by keyboard");
            std::string reason = "keyboard:";
            reason.append(std::to_string(keyCode));
            this->WakeupDevice(now, WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON, reason);
        }
    }
}

void PowerMgrService::HandlePointEvent()
{
    POWER_HILOGI(MODULE_SERVICE, "HandlePointEvent");
    int64_t now = static_cast<int64_t>(time(0));
    this->RefreshActivity(now, UserActivityType::USER_ACTIVITY_TYPE_TOUCH, false);
}

void PowerMgrService::HandlePowerKeyTimeout()
{
    POWER_HILOGI(MODULE_SERVICE, "HandlePowerKeyTimeout");
    if (powerkeyPressed_) {
        const int logLevel = 2;
        const std::string tag = "TAG_POWER";
        HiviewDFX::HiSysEvent::Write(HiviewDFX::HiSysEvent::Domain::POWERMGR, "Service",
            HiviewDFX::HiSysEvent::EventType::FAULT,
            "LOG_LEVEL",
            logLevel,
            "TAG",
            tag,
            "MESSAGE",
            "POWER KEY PRESS TIMEOUT");
        POWER_HILOGI(MODULE_SERVICE, "PowerKey press Timeout");
        powerkeyPressed_ = false;
    }
}

void PowerMgrService::PowerMgrService::OnStop()
{
    POWER_HILOGI(MODULE_SERVICE, "stop service");
    if (!ready_) {
        return;
    }
    eventRunner_.reset();
    handler_.reset();
    ready_ = false;
}

int32_t PowerMgrService::Dump(int32_t fd, const std::vector<std::u16string>& args)
{
    std::lock_guard lock(mutex_);
    POWER_HILOGI(MODULE_SERVICE, "Dump service");
    std::vector<std::string> argsInStr;
    std::transform(args.begin(), args.end(), std::back_inserter(argsInStr),
        [](const std::u16string &arg) {
        std::string ret = Str16ToStr8(arg);
        POWER_HILOGI(MODULE_SERVICE, "arg: %{public}s", ret.c_str());
        return ret;
    });
    std::string result;
    PowerMgrDumper::Dump(argsInStr, result);
    if (!SaveStringToFd(fd, result)) {
        POWER_HILOGE(MODULE_SERVICE, "PowerMgrService::Dump failed, save to fd failed.");
        POWER_HILOGE(MODULE_SERVICE, "Dump Info:\n");
        POWER_HILOGE(MODULE_SERVICE, "%{public}s", result.c_str());
        return ERR_OK;
    }
    return ERR_OK;
}

void PowerMgrService::RebootDevice(const std::string& reason)
{
    std::lock_guard lock(mutex_);
    pid_t pid = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    if (reason.find("recovery") != std::string::npos) {
        if (!Permission::CheckCallingPermission("ohos.permission.REBOOT_RECOVERY")) {
            POWER_HILOGE(MODULE_SERVICE,
                "%{public}s Request failed, %{public}d permission check fail",
                __func__, pid);
            return;
        }
    } else {
        if ((uid >= APP_FIRST_UID)
            && !Permission::CheckCallingPermission("ohos.permission.REBOOT")) {
            POWER_HILOGE(MODULE_SERVICE,
                "%{public}s Request failed, %{public}d permission check fail",
                __func__, pid);
            return;
        }
    }
    POWER_HILOGI(MODULE_SERVICE, "Cancel auto sleep timer");
    powerStateMachine_->CancelDelayTimer(
        PowermsEventHandler::CHECK_USER_ACTIVITY_TIMEOUT_MSG);
    powerStateMachine_->CancelDelayTimer(
        PowermsEventHandler::CHECK_USER_ACTIVITY_OFF_TIMEOUT_MSG);
    powerStateMachine_->CancelDelayTimer(
        PowermsEventHandler::CHECK_USER_ACTIVITY_SLEEP_TIMEOUT_MSG);

    POWER_HILOGI(MODULE_SERVICE, "PID: %{public}d Call %{public}s !", pid, __func__);
    shutdownService_.Reboot(reason);
}

void PowerMgrService::ShutDownDevice(const std::string& reason)
{
    std::lock_guard lock(mutex_);
    pid_t pid  = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    if ((uid >= APP_FIRST_UID)
        && !Permission::CheckCallingPermission("ohos.permission.SHUTDOWN")) {
        POWER_HILOGE(MODULE_SERVICE,
            "%{public}s Request failed, %{public}d permission check fail",
            __func__, pid);
        return;
    }
    POWER_HILOGI(MODULE_SERVICE, "Cancel auto sleep timer");
    powerStateMachine_->CancelDelayTimer(
        PowermsEventHandler::CHECK_USER_ACTIVITY_TIMEOUT_MSG);
    powerStateMachine_->CancelDelayTimer(
        PowermsEventHandler::CHECK_USER_ACTIVITY_OFF_TIMEOUT_MSG);
    powerStateMachine_->CancelDelayTimer(
        PowermsEventHandler::CHECK_USER_ACTIVITY_SLEEP_TIMEOUT_MSG);

    POWER_HILOGI(MODULE_SERVICE, "PID: %{public}d Call %{public}s !", pid, __func__);
    shutdownService_.Shutdown(reason);
}

void PowerMgrService::SuspendDevice(int64_t callTimeMs,
    SuspendDeviceType reason,
    bool suspendImmed)
{
    std::lock_guard lock(mutex_);
    auto uid = IPCSkeleton::GetCallingUid();
    if (uid >= APP_FIRST_UID) {
        POWER_HILOGE(MODULE_SERVICE,
            "%{public}s Request failed, illegal calling uid %{public}d.",
            __func__, uid);
        return;
    }
    if (shutdownService_.IsShuttingDown()) {
        POWER_HILOGI(MODULE_SERVICE, "System is shutting down, can't suspend");
        return;
    }
    pid_t pid = IPCSkeleton::GetCallingPid();
    powerStateMachine_->SuspendDeviceInner(pid, callTimeMs, reason, suspendImmed);
}

void PowerMgrService::WakeupDevice(int64_t callTimeMs,
    WakeupDeviceType reason,
    const std::string& details)
{
    std::lock_guard lock(mutex_);
    auto uid = IPCSkeleton::GetCallingUid();
    if (uid >= APP_FIRST_UID) {
        POWER_HILOGE(MODULE_SERVICE,
            "%{public}s Request failed, illegal calling uid %{public}d.",
            __func__, uid);
        return;
    }
    pid_t pid = IPCSkeleton::GetCallingPid();
    powerStateMachine_->WakeupDeviceInner(pid, callTimeMs, reason, details, "OHOS");
}

void PowerMgrService::RefreshActivity(int64_t callTimeMs,
    UserActivityType type,
    bool needChangeBacklight)
{
    std::lock_guard lock(mutex_);
    auto uid = IPCSkeleton::GetCallingUid();
    if ((uid >= APP_FIRST_UID)
        || !Permission::CheckCallingPermission("ohos.permission.REFRESH_USER_ACTION")) {
        POWER_HILOGE(MODULE_SERVICE,
            "%{public}s Request failed, illegal calling uid %{public}d.",
            __func__, uid);
        return;
    }
    pid_t pid = IPCSkeleton::GetCallingPid();
    powerStateMachine_->RefreshActivityInner(pid, callTimeMs, type, needChangeBacklight);
}

PowerState PowerMgrService::GetState()
{
    std::lock_guard lock(mutex_);
    POWER_HILOGI(MODULE_SERVICE, "GetState");
    return powerStateMachine_->GetState();
}

bool PowerMgrService::IsScreenOn()
{
    std::lock_guard lock(mutex_);
    POWER_HILOGI(MODULE_SERVICE, "IsScreenOn");
    return powerStateMachine_->IsScreenOn();
}

bool PowerMgrService::ForceSuspendDevice(int64_t callTimeMs)
{
    std::lock_guard lock(mutex_);
    auto uid = IPCSkeleton::GetCallingUid();
    if (uid >= APP_FIRST_UID) {
        POWER_HILOGE(MODULE_SERVICE,
            "%{public}s Request failed, illegal calling uid %{public}d.",
            __func__, uid);
        return false;
    }
    if (shutdownService_.IsShuttingDown()) {
        POWER_HILOGI(MODULE_SERVICE, "System is shutting down, can't force suspend");
        return false;
    }
    pid_t pid = IPCSkeleton::GetCallingPid();
    return powerStateMachine_->ForceSuspendDeviceInner(pid, callTimeMs);
}

inline void PowerMgrService::FillUserIPCInfo(UserIPCInfo &userIPCinfo)
{
    userIPCinfo.pid = IPCSkeleton::GetCallingPid();
    userIPCinfo.uid = IPCSkeleton::GetCallingUid();
}

void PowerMgrService::CreateRunningLock(const sptr<IRemoteObject>& token,
    const RunningLockInfo& runningLockInfo)
{
    std::lock_guard lock(mutex_);
    auto uid = IPCSkeleton::GetCallingUid();
    if ((uid >= APP_FIRST_UID)
        && !Permission::CheckCallingPermission("ohos.permission.RUNNING_LOCK")) {
        POWER_HILOGE(MODULE_SERVICE,
            "%{public}s Request failed, %{public}d permission check fail",
            __func__, uid);
        return;
    }

    POWER_HILOGI(MODULE_SERVICE, "%{public}s :name = %s, type = %d", __func__,
        runningLockInfo.name.c_str(), runningLockInfo.type);

    UserIPCInfo userIPCInfo;
    FillUserIPCInfo(userIPCInfo);
    runningLockMgr_->CreateRunningLock(token, runningLockInfo, userIPCInfo);
}

void PowerMgrService::ReleaseRunningLock(const sptr<IRemoteObject>& token)
{
    std::lock_guard lock(mutex_);
    auto uid = IPCSkeleton::GetCallingUid();
    if ((uid >= APP_FIRST_UID)
        && !Permission::CheckCallingPermission("ohos.permission.RUNNING_LOCK")) {
        POWER_HILOGE(MODULE_SERVICE,
            "%{public}s Request failed, %{public}d permission check fail",
            __func__, uid);
        return;
    }

    POWER_HILOGI(MODULE_SERVICE, "%{public}s called.", __func__);
    runningLockMgr_->ReleaseLock(token);
}

bool PowerMgrService::IsRunningLockTypeSupported(uint32_t type)
{
    std::lock_guard lock(mutex_);
    if (type >= static_cast<uint32_t>(RunningLockType::RUNNINGLOCK_BUTT)) {
        return false;
    }
    return true;
}

void PowerMgrService::Lock(const sptr<IRemoteObject>& token,
    const RunningLockInfo& runningLockInfo,
    uint32_t timeOutMS)
{
    std::lock_guard lock(mutex_);
    auto uid = IPCSkeleton::GetCallingUid();
    if ((uid >= APP_FIRST_UID)
        && !Permission::CheckCallingPermission("ohos.permission.RUNNING_LOCK")) {
        POWER_HILOGE(MODULE_SERVICE,
            "%{public}s Request failed, %{public}d permission check fail",
            __func__, uid);
        return;
    }

    POWER_HILOGI(MODULE_SERVICE,
        "%{public}s :timeOutMS = %{public}d, name = %{public}s, type = %{public}d",
        __func__,
        timeOutMS,
        runningLockInfo.name.c_str(),
        runningLockInfo.type);

    UserIPCInfo userIPCInfo;
    FillUserIPCInfo(userIPCInfo);
    runningLockMgr_->Lock(token, runningLockInfo, userIPCInfo, timeOutMS);
}

void PowerMgrService::UnLock(const sptr<IRemoteObject>& token)
{
    std::lock_guard lock(mutex_);
    auto uid = IPCSkeleton::GetCallingUid();
    if ((uid >= APP_FIRST_UID)
        && !Permission::CheckCallingPermission("ohos.permission.RUNNING_LOCK")) {
        POWER_HILOGE(MODULE_SERVICE,
            "%{public}s Request failed, %{public}d permission check fail",
            __func__, uid);
        return;
    }

    POWER_HILOGI(MODULE_SERVICE, "%{public}s called.", __func__);
    runningLockMgr_->UnLock(token);
}

void PowerMgrService::ForceUnLock(const sptr<IRemoteObject>& token)
{
    std::lock_guard lock(mutex_);
    POWER_HILOGI(MODULE_SERVICE, "%{public}s called.", __func__);
    runningLockMgr_->UnLock(token);
    runningLockMgr_->ReleaseLock(token);
}

bool PowerMgrService::IsUsed(const sptr<IRemoteObject>& token)
{
    std::lock_guard lock(mutex_);
    POWER_HILOGI(MODULE_SERVICE, "%{public}s called.", __func__);
    return runningLockMgr_->IsUsed(token);
}

void PowerMgrService::NotifyRunningLockChanged(bool isUnLock)
{
    if (isUnLock) {
        // When unlock we try to suspend
        if (!runningLockMgr_->ExistValidRunningLock()
            && !powerStateMachine_->IsScreenOn()) {
            // runninglock is empty and Screen is off,
            // so we try to suspend device from Z side.
            POWER_HILOGI(MODULE_SERVICE,
                "%{public}s :RunningLock is empty, try to suspend from Z Side!",
                __func__);
            powerStateMachine_->SuspendDeviceInner(getpid(), GetTickCount(),
                SuspendDeviceType::SUSPEND_DEVICE_REASON_MIN, true, true);
        }
    }
}

void PowerMgrService::SetWorkTriggerList(const sptr<IRemoteObject>& token,
    const WorkTriggerList& workTriggerList)
{
    std::lock_guard lock(mutex_);
    auto uid = IPCSkeleton::GetCallingUid();
    if ((uid >= APP_FIRST_UID)
        && !Permission::CheckCallingPermission("ohos.permission.RUNNING_LOCK")) {
        POWER_HILOGE(MODULE_SERVICE,
            "%{public}s Request failed, %{public}d permission check fail",
            __func__, uid);
        return;
    }

    POWER_HILOGI(MODULE_SERVICE, "%{public}s called.", __func__);
    runningLockMgr_->SetWorkTriggerList(token, workTriggerList);
}

void PowerMgrService::ProxyRunningLock(bool proxyLock, pid_t uid, pid_t pid)
{
    std::lock_guard lock(mutex_);
    auto calllingUid = IPCSkeleton::GetCallingUid();
    if (calllingUid >= APP_FIRST_UID) {
        POWER_HILOGE(MODULE_SERVICE,
            "%{public}s Request failed, illegal calling uid %{public}d.",
            __func__,
            calllingUid);
        return;
    }
    runningLockMgr_->ProxyRunningLock(proxyLock, uid, pid);
}

void PowerMgrService::RegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback)
{
    std::lock_guard lock(mutex_);
    auto uid = IPCSkeleton::GetCallingUid();
    if ((uid >= APP_FIRST_UID)
        && !Permission::CheckCallingPermission("ohos.permission.POWER_MANAGER")) {
        POWER_HILOGE(MODULE_SERVICE,
            "%{public}s Request failed, %{public}d permission check fail",
            __func__, uid);
        return;
    }
    powerStateMachine_->RegisterPowerStateCallback(callback);
}

void PowerMgrService::UnRegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback)
{
    std::lock_guard lock(mutex_);
    auto uid = IPCSkeleton::GetCallingUid();
    if ((uid >= APP_FIRST_UID)
        && !Permission::CheckCallingPermission("ohos.permission.POWER_MANAGER")) {
        POWER_HILOGE(MODULE_SERVICE,
            "%{public}s Request failed, %{public}d permission check fail",
            __func__, uid);
        return;
    }
    powerStateMachine_->UnRegisterPowerStateCallback(callback);
}

void PowerMgrService::RegisterShutdownCallback(IShutdownCallback::ShutdownPriority priority,
    const sptr<IShutdownCallback>& callback)
{
    std::lock_guard lock(mutex_);
    auto uid = IPCSkeleton::GetCallingUid();
    if (uid >= APP_FIRST_UID) {
        POWER_HILOGE(MODULE_SERVICE, "Register failed, %{public}d fail", uid);
        return;
    }
    POWER_HILOGE(MODULE_SERVICE, "Register shutdown callback: %{public}d", uid);
    shutdownService_.AddShutdownCallback(priority, callback);
}

void PowerMgrService::UnRegisterShutdownCallback(const sptr<IShutdownCallback>& callback)
{
    std::lock_guard lock(mutex_);
    auto uid = IPCSkeleton::GetCallingUid();
    if (uid >= APP_FIRST_UID) {
        POWER_HILOGE(MODULE_SERVICE, "UnRegister failed, %{public}d fail", uid);
        return;
    }
    POWER_HILOGE(MODULE_SERVICE, "UnRegister shutdown callback: %{public}d", uid);
    shutdownService_.DelShutdownCallback(callback);
}

void PowerMgrService::RegisterPowerModeCallback(const sptr<IPowerModeCallback>& callback)
{
    std::lock_guard lock(mutex_);
    auto uid = IPCSkeleton::GetCallingUid();
    if (uid >= APP_FIRST_UID) {
        POWER_HILOGE(MODULE_SERVICE, "Register failed, %{public}d fail", uid);
        return;
    }
    POWER_HILOGE(MODULE_SERVICE, "Register power mode callback: %{public}d", uid);
    powerModeModule_.AddPowerModeCallback(callback);
}

void PowerMgrService::UnRegisterPowerModeCallback(const sptr<IPowerModeCallback>& callback)
{
    std::lock_guard lock(mutex_);
    auto uid = IPCSkeleton::GetCallingUid();
    if (uid >= APP_FIRST_UID) {
        POWER_HILOGE(MODULE_SERVICE, "UnRegister failed, %{public}d fail", uid);
        return;
    }
    POWER_HILOGE(MODULE_SERVICE, "UnRegister power mode callback: %{public}d", uid);
    powerModeModule_.DelPowerModeCallback(callback);
}

void PowerMgrService::SetDisplaySuspend(bool enable)
{
    std::lock_guard lock(mutex_);
    auto uid = IPCSkeleton::GetCallingUid();
    if (uid >= APP_FIRST_UID) {
        POWER_HILOGE(MODULE_SERVICE, "SetDisplaySuspend failed, %{public}d fail", uid);
        return;
    }
    powerStateMachine_->SetDisplaySuspend(enable);
}

void PowerMgrService::SetDeviceMode(const uint32_t& mode)
{
    std::lock_guard lock(mutex_);
    pid_t pid = IPCSkeleton::GetCallingPid();
    POWER_HILOGI(MODULE_SERVICE, "PID: %{public}d Call %{public}s !", pid, __func__);
    powerModeModule_.SetModeItem(mode);
}

uint32_t PowerMgrService::GetDeviceMode()
{
    std::lock_guard lock(mutex_);
    pid_t pid = IPCSkeleton::GetCallingPid();
    POWER_HILOGI(MODULE_SERVICE, "PID: %{public}d Call %{public}s !", pid, __func__);
    return powerModeModule_.GetModeItem();
}
} // namespace PowerMgr
} // namespace OHOS
