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

#include "running_lock_proxy.h"

#include <cmath>
#include "power_log.h"

namespace OHOS {
namespace PowerMgr {
void RunningLockProxy::AddRunningLock(pid_t pid, pid_t uid, const sptr<IRemoteObject>& remoteObj)
{
    std::string proxyKey = AssembleProxyKey(pid, uid);
    auto proxyIter = proxyMap_.find(proxyKey);
    if (proxyIter == proxyMap_.end()) {
        std::vector<sptr<IRemoteObject>> tmpLockList {};
        tmpLockList.push_back(remoteObj);
        std::tie(proxyIter, std::ignore) = proxyMap_.emplace(proxyKey, std::make_pair(tmpLockList, 0));
        POWER_HILOGD(FEATURE_RUNNING_LOCK, "Add runninglock proxy, proxyKey=%{public}s", proxyKey.c_str());
    } else {
        auto& remoteObjList = proxyIter->second.first;
        if (std::find(remoteObjList.begin(), remoteObjList.end(), remoteObj) != remoteObjList.end()) {
            POWER_HILOGD(FEATURE_RUNNING_LOCK, "Runninglock is existed, proxyKey=%{public}s", proxyKey.c_str());
            return;
        }
        remoteObjList.push_back(remoteObj);
        POWER_HILOGD(FEATURE_RUNNING_LOCK, "Insert runninglock, proxyKey=%{public}s", proxyKey.c_str());
    }
}

void RunningLockProxy::RemoveRunningLock(pid_t pid, pid_t uid, const sptr<IRemoteObject>& remoteObj)
{
    std::string proxyKey = AssembleProxyKey(pid, uid);
    auto proxyIter = proxyMap_.find(proxyKey);
    if (proxyIter == proxyMap_.end()) {
        POWER_HILOGI(FEATURE_RUNNING_LOCK,
            "Runninglock proxyKey is not existed, proxyKey=%{public}s", proxyKey.c_str());
        return;
    }
    auto& remoteObjList = proxyIter->second.first;
    auto remoteObjIter = std::find(remoteObjList.begin(), remoteObjList.end(), remoteObj);
    if (remoteObjIter == remoteObjList.end()) {
        POWER_HILOGD(FEATURE_RUNNING_LOCK, "Runninglock is not existed, proxyKey=%{public}s", proxyKey.c_str());
        return;
    }
    remoteObjList.erase(remoteObjIter);
    if (remoteObjList.empty()) {
        proxyMap_.erase(proxyKey);
        POWER_HILOGD(FEATURE_RUNNING_LOCK, "Runninglock list is empty, earse proxyKey=%{public}s", proxyKey.c_str());
    }
}

std::vector<sptr<IRemoteObject>> RunningLockProxy::GetRemoteObjectList(pid_t pid, pid_t uid)
{
    std::string proxyKey = AssembleProxyKey(pid, uid);
    auto proxyIter = proxyMap_.find(proxyKey);
    if (proxyIter != proxyMap_.end()) {
        return proxyIter->second.first;
    }
    return std::vector<sptr<IRemoteObject>>();
}

bool RunningLockProxy::IsProxied(pid_t pid, pid_t uid)
{
    std::string proxyKey = AssembleProxyKey(pid, uid);
    auto proxyIter = proxyMap_.find(proxyKey);
    if (proxyIter == proxyMap_.end()) {
        return false;
    }
    return proxyIter->second.second != 0;
}

bool RunningLockProxy::IncreaseProxyCnt(pid_t pid, pid_t uid, const std::function<void(void)>& proxyRunningLock)
{
    std::string proxyKey = AssembleProxyKey(pid, uid);
    auto proxyIter = proxyMap_.find(proxyKey);
    if (proxyIter == proxyMap_.end()) {
        std::tie(proxyIter, std::ignore) = proxyMap_.emplace(proxyKey,
            std::make_pair<std::vector<sptr<IRemoteObject>>, int32_t>({}, 0));
    }
    proxyIter->second.second++;
    POWER_HILOGI(FEATURE_RUNNING_LOCK, "IncreaseProxyCnt proxykey=%{public}s proxycnt=%{public}d",
        proxyKey.c_str(), proxyIter->second.second);
    if (proxyIter->second.second > 1) {
        return false;
    }
    proxyRunningLock();
    return true;
}

bool RunningLockProxy::DecreaseProxyCnt(pid_t pid, pid_t uid, const std::function<void(void)>& unProxyRunningLock)
{
    std::string proxyKey = AssembleProxyKey(pid, uid);
    auto proxyIter = proxyMap_.find(proxyKey);
    if (proxyIter == proxyMap_.end()) {
        return false;
    }
    proxyIter->second.second = std::max(0, proxyIter->second.second - 1);
    POWER_HILOGI(FEATURE_RUNNING_LOCK, "DecreaseProxyCnt proxykey=%{public}s proxycnt=%{public}d",
        proxyKey.c_str(), proxyIter->second.second);
    if (proxyIter->second.second > 0) {
        return false;
    }
    if (proxyIter->second.first.empty()) {
        proxyMap_.erase(proxyIter);
    }
    unProxyRunningLock();
    return true;
}

std::string RunningLockProxy::DumpProxyInfo()
{
    std::string result {""};
    int index = 0;
    for (const auto& [key, value] : proxyMap_) {
        index++;
        result.append("  index=").append(std::to_string(index))
            .append(" pid_uid=").append(key)
            .append(" lock_cnt=").append(std::to_string(value.first.size()))
            .append(" proxy_cnt=").append(std::to_string(value.second)).append("\n");
    }
    return result;
}

void RunningLockProxy::ResetRunningLocks()
{
    POWER_HILOGI(FEATURE_RUNNING_LOCK, "reset proxycnt");
    for (auto proxyIter = proxyMap_.begin(); proxyIter != proxyMap_.end();) {
        proxyIter->second.second = 0;
        if (proxyIter->second.first.empty()) {
            proxyMap_.erase(proxyIter++);
        } else {
            proxyIter++;
        }
    }
}

void RunningLockProxy::Clear()
{
    proxyMap_.clear();
}

std::string RunningLockProxy::AssembleProxyKey(pid_t pid, pid_t uid)
{
    return std::to_string(pid) + "_" + std::to_string(uid);
}
} // namespace PowerMgr
} // namespace OHOS

