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

#include "multi_invoker_helper.h"
#include <ipc_skeleton.h>
#include <power_log.h>
namespace OHOS::PowerMgr {
using namespace std;
// the user i.e MultiInvokerHelper should provide mutex
std::bitset<MAX_PARAM_NUMBER> MultiInvokerHelper::Invoker::SetValue(
    pid_t appid, const std::bitset<MAX_PARAM_NUMBER>& input)
{
    const auto iter = entries_.find(appid);
    std::bitset<MAX_PARAM_NUMBER> previous = (iter == entries_.end() ? 0 : iter->second);
    // update counter according to current entries and counter
    // an add for each parameter + extra space for saving counter VS a bitwise-or for each entry.
    // I would prefer just doing bitwise-or but well it doesn't matter much.
    for (size_t index = 0; index < paramCount_; index++) {
        if (previous[index] && !input[index]) {
            result_[index] = static_cast<bool>(--sum_[index]);
        } else if (!previous[index] && input[index]) {
            result_[index] = static_cast<bool>(++sum_[index]);
        }
    }
    // update entry
    if (input != 0) {
        entries_[appid] = input;
    } else if (iter != entries_.end()) {
        entries_.erase(iter);
    }
    return result_;
}

std::bitset<MAX_PARAM_NUMBER> MultiInvokerHelper::Invoker::GetResult() const
{
    return result_;
}

std::vector<uint64_t> MultiInvokerHelper::Invoker::GetSum() const
{
    return sum_;
}

std::string MultiInvokerHelper::Invoker::Dump() const
{
    constexpr int lastCharsToRemove = 2;
    std::string ret {};
    ret += "sums:[";
    // print sums in reverse order to match the bitset appearance
    for (auto iter = sum_.crbegin(); iter != sum_.crend(); iter++) {
        ret += to_string(*iter);
        if (std::next(iter) != sum_.crend()) {
            ret += ", ";
        } else {
            ret += "] ";
        }
    }
    for (const auto& item : entries_) {
        // to_string(integral type) and std::bitset::to_string()
        ret += to_string(item.first) + ": " + item.second.to_string() + ", ";
    }
    if (!ret.empty()) {
        ret.erase(ret.size() - lastCharsToRemove);
    }
    return ret;
}

std::string MultiInvokerHelper::DumpInner() const
{
    std::string ret {};
    ret += "sums:[";
    // print sums in reverse order to match the bitset appearance
    for (auto iter = sum_.crbegin(); iter != sum_.crend(); iter++) {
        ret += to_string(*iter);
        if (std::next(iter) != sum_.crend()) {
            ret += ", ";
        } else {
            ret += "] ";
        }
    }
    for (auto iter = invokers_.cbegin(); iter != invokers_.cend(); iter++) {
        ret += to_string(iter->first) + ": {" + iter->second.Dump() + "}";
        if (std::next(iter) != invokers_.cend()) {
            ret += ", ";
        }
    }
    return ret;
}

std::string MultiInvokerHelper::Dump()
{
    std::lock_guard lock(mutex_);
    return DumpInner();
}

std::bitset<MAX_PARAM_NUMBER> MultiInvokerHelper::GetResult()
{
    std::lock_guard lock(mutex_);
    return result_;
}

bool MultiInvokerHelper::RemoveInvoker(pid_t pid)
{
    std::lock_guard lock(mutex_);
    const auto iter = invokers_.find(pid);
    if (iter == invokers_.cend()) {
        return false;
    }
    for (size_t index = 0; index < paramCount_; index++) {
        result_[index] = static_cast<bool>(sum_[index] -= iter->second.GetResult()[index]);
    }
    invokers_.erase(iter);
    return true;
}

// For now our client is a singleton, there is 1-1 correspondence between IPCObjectProxy and pid.
// If it is not the case, the pid can't be used to uniquely identify IPCObjectProxy.
// I am not be able to use handle or descriptor as key since other ppl dont like it.
void MultiInvokerHelper::Set(
    const sptr<IRemoteObject>& remoteObj, pid_t invokerPid, pid_t appid, std::bitset<MAX_PARAM_NUMBER>& input)
{
    if (!remoteObj) {
        POWER_HILOGE(FEATURE_POWER_STATE, "%{public}s: remoteObj is nullptr", __func__);
        return;
    }
    std::bitset<MAX_PARAM_NUMBER> deltaInput = input ^ defaultParam_;
    std::lock_guard lock(mutex_);
    auto [iter, _] = invokers_.try_emplace(invokerPid, Invoker {paramCount_, remoteObj});
    std::bitset<MAX_PARAM_NUMBER> previous = iter->second.GetResult();
    pid_t inputKey = appid != -1 ? appid : invokerPid;
    std::bitset<MAX_PARAM_NUMBER> result = iter->second.SetValue(inputKey, deltaInput);

    for (size_t index = 0; index < paramCount_; index++) {
        if (previous[index] && !result[index]) {
            result_[index] = static_cast<bool>(--sum_[index]);
        } else if (!previous[index] && result[index]) {
            result_[index] = static_cast<bool>(++sum_[index]);
        }
    }

    POWER_HILOGI(FEATURE_POWER_STATE, "previous: %{public}s, result: %{public}s", previous.to_string().c_str(),
        result.to_string().c_str());
    if (previous == 0 && result != 0) {
        // implicitly cast this to IRemoteObject::DeathRecipient.
        // Thus the instance should only be created by using MakeSptr.
        remoteObj->AddDeathRecipient(this);
    } else if (previous != 0 && result == 0) {
        remoteObj->RemoveDeathRecipient(this);
    }
    std::string dumpStr = DumpInner();
    POWER_HILOGI(FEATURE_POWER_STATE, "final result may have changed after calling interface.");
    POWER_HILOGI(FEATURE_POWER_STATE, "current invokers: %{public}s", dumpStr.c_str());
    onChange_(result_ ^ defaultParam_);
}

void MultiInvokerHelper::OnRemoteDied(const wptr<IRemoteObject>& object)
{
    pid_t pid = IPCSkeleton::GetCallingPid();
    POWER_HILOGI(FEATURE_POWER_STATE, "remote died, calling pid: %{public}d", pid);
    auto strongRef = object.promote();
    if (!strongRef) {
        POWER_HILOGW(FEATURE_POWER_STATE, "remote died, but IRemoteObject invalid");
        return;
    }
    bool removed = RemoveInvoker(pid);
    if (removed) {
        std::string dumpStr = DumpInner();
        POWER_HILOGI(FEATURE_POWER_STATE, "final result may have changed after remote dying");
        POWER_HILOGI(FEATURE_POWER_STATE, "current invokers: %{public}s", dumpStr.c_str());
        onChange_(result_ ^ defaultParam_);
    }
    strongRef->RemoveDeathRecipient(this);
}
} // namespace OHOS::PowerMgr