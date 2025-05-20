/*
 * Copyright (c) 2024-2024 Huawei Device Co., Ltd.
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

#include "power_ext_intf_wrapper.h"
#include "power_log.h"

namespace OHOS {
namespace PowerMgr {
namespace {
const char* POWER_MANAGER_EXT_PATH = "libpower_manager_ext.z.so";
const std::vector<std::string> ALL_POWER_EXT_INTF_SYMBOL = {
    "GetRebootCommand",
#ifdef POWER_MANAGER_POWER_ENABLE_S4
    "SubscribeScreenLockCommonEvent",
    "UnSubscribeScreenLockCommonEvent",
    "BlockHibernateUntilScrLckReady",
    "OnHibernateEnd",
#endif
#ifdef POWER_MANAGER_ENABLE_WATCH_CUSTOMIZED_SCREEN_COMMON_EVENT_RULES
    "SetScreenOnEventRules",
    "PublishCustomizedScreenEvent",
    "NotifyScreenOnEventAgain",
    "NotifyOperateEventAfterScreenOn",
#endif
};
} // namespace

PowerExtIntfWrapper& PowerExtIntfWrapper::Instance()
{
    static PowerExtIntfWrapper instance(POWER_MANAGER_EXT_PATH, ALL_POWER_EXT_INTF_SYMBOL);
    return instance;
}

PowerExtIntfWrapper::ErrCode PowerExtIntfWrapper::GetRebootCommand(
    const std::string& rebootReason, std::string& rebootCmd) const
{
    POWER_HILOGE(COMP_SVC, "Enter GetRebootCommand wrapper");
    void* funcPtr = intfLoader_.QueryInterface("GetRebootCommand");
    if (funcPtr == nullptr) {
        return PowerExtIntfWrapper::ErrCode::ERR_NOT_FOUND;
    }
    auto getRebootCommandFunc = reinterpret_cast<const char* (*)(const std::string&)>(funcPtr);
    rebootCmd = getRebootCommandFunc(rebootReason);
    return PowerExtIntfWrapper::ErrCode::ERR_OK;
}

PowerExtIntfWrapper::ErrCode PowerExtIntfWrapper::SubscribeScreenLockCommonEvent() const
{
    void* funcPtr = intfLoader_.QueryInterface("SubscribeScreenLockCommonEvent");
    if (funcPtr == nullptr) {
        return PowerExtIntfWrapper::ErrCode::ERR_NOT_FOUND;
    }
    auto subscribeScrLockEventFunc = reinterpret_cast<void (*)(void)>(funcPtr);
    subscribeScrLockEventFunc();
    return PowerExtIntfWrapper::ErrCode::ERR_OK;
}

PowerExtIntfWrapper::ErrCode PowerExtIntfWrapper::UnSubscribeScreenLockCommonEvent() const
{
    void* funcPtr = intfLoader_.QueryInterface("UnSubscribeScreenLockCommonEvent");
    if (funcPtr == nullptr) {
        return PowerExtIntfWrapper::ErrCode::ERR_NOT_FOUND;
    }
    auto unSubscribeScrLockEventFunc = reinterpret_cast<void (*)(void)>(funcPtr);
    unSubscribeScrLockEventFunc();
    return PowerExtIntfWrapper::ErrCode::ERR_OK;
}

PowerExtIntfWrapper::ErrCode PowerExtIntfWrapper::BlockHibernateUntilScrLckReady() const
{
    void* funcPtr = intfLoader_.QueryInterface("BlockHibernateUntilScrLckReady");
    if (funcPtr == nullptr) {
        return PowerExtIntfWrapper::ErrCode::ERR_NOT_FOUND;
    }
    auto blockHibernateFunc = reinterpret_cast<void (*)(void)>(funcPtr);
    blockHibernateFunc();
    return PowerExtIntfWrapper::ErrCode::ERR_OK;
}

void PowerExtIntfWrapper::OnHibernateEnd(bool hibernateResult)
{
    POWER_HILOGI(COMP_SVC, "Enter OnHibernateEnd wrapper");
    void *funcPtr = intfLoader_.QueryInterface("OnHibernateEnd");
    if (funcPtr == nullptr) {
        return;
    }
    auto OnHibernateEndFunc = reinterpret_cast<void (*)(bool)>(funcPtr);
    OnHibernateEndFunc(hibernateResult);
}

#ifdef POWER_MANAGER_ENABLE_WATCH_CUSTOMIZED_SCREEN_COMMON_EVENT_RULES
PowerExtIntfWrapper::ErrCode PowerExtIntfWrapper::SetScreenOnEventRules(StateChangeReason reason,
    const std::vector<StateChangeReason>& stateChangeReason, const std::vector<WakeupDeviceType>& wakeupDeviceTypes)
{
    void *funcPtr = intfLoader_.QueryInterface("SetScreenOnEventRules");
    if (funcPtr == nullptr) {
        return PowerExtIntfWrapper::ErrCode::ERR_NOT_FOUND;
    }
    auto setScreenOnEventRulesFunc = reinterpret_cast<void (*)(
        StateChangeReason, const std::vector<StateChangeReason>&, const std::vector<WakeupDeviceType>&)>(funcPtr);
    setScreenOnEventRulesFunc(reason, stateChangeReason, wakeupDeviceTypes);
    return PowerExtIntfWrapper::ErrCode::ERR_OK;
}

void PowerExtIntfWrapper::PublishCustomizedScreenEvent(PowerState state, std::vector<std::string> bundleNames)
{
    void *funcPtr = intfLoader_.QueryInterface("PublishCustomizedScreenEvent");
    if (funcPtr == nullptr) {
        return;
    }
    auto publishCustomizedScreenEventFunc =
        reinterpret_cast<void (*)(PowerState, std::vector<std::string>)>(funcPtr);
    publishCustomizedScreenEventFunc(state, bundleNames);
}

bool PowerExtIntfWrapper::NotifyScreenOnEventAgain(WakeupDeviceType reason, std::vector<std::string> bundleNames)
{
    void *funcPtr = intfLoader_.QueryInterface("NotifyScreenOnEventAgain");
    if (funcPtr == nullptr) {
        return false;
    }
    auto notifyScreenOnEventAgain =
        reinterpret_cast<bool (*)(WakeupDeviceType, std::vector<std::string>)>(funcPtr);
    return notifyScreenOnEventAgain(reason, bundleNames);
}

void PowerExtIntfWrapper::NotifyOperateEventAfterScreenOn(std::vector<std::string> bundleNames)
{
    void *funcPtr = intfLoader_.QueryInterface("NotifyOperateEventAfterScreenOn");
    if (funcPtr == nullptr) {
        return;
    }
    auto notifyOperateEventAfterScreenOnFunc = reinterpret_cast<void (*)(std::vector<std::string>)>(funcPtr);
    notifyOperateEventAfterScreenOnFunc(bundleNames);
}
#endif

} // namespace PowerMgr
} // namespace OHOS