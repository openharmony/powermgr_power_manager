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
#include "power_mgr_service.h"

#include <cmath>
#include "power_log.h"

namespace OHOS {
namespace PowerMgr {
void RunningLockProxy::AddRunningLock(pid_t pid, pid_t uid, const sptr<IRemoteObject>& remoteObj)
{
    std::string proxyKey = AssembleProxyKey(pid, uid);
    auto proxyIter = proxyMap_.find(proxyKey);
    WksMap wksMap;
    if (proxyIter == proxyMap_.end()) {
        TokenWorkSourceMap tokenWksMap;
        tokenWksMap.emplace(remoteObj, std::make_pair(wksMap, 0));
        proxyMap_.emplace(proxyKey, tokenWksMap);
        POWER_HILOGD(FEATURE_RUNNING_LOCK, "Add runninglock proxy, proxyKey=%{public}s", proxyKey.c_str());
    } else {
        auto& tokenWksMap = proxyIter->second;
        auto tokenIter = tokenWksMap.find(remoteObj);
        if (tokenIter == tokenWksMap.end()) {
            tokenWksMap.emplace(remoteObj, std::make_pair(wksMap, 0));
            POWER_HILOGD(FEATURE_RUNNING_LOCK, "Insert runninglock, proxyKey=%{public}s", proxyKey.c_str());
        } else {
            POWER_HILOGD(FEATURE_RUNNING_LOCK, "Already add runninglock, proxyKey=%{public}s", proxyKey.c_str());
        }
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
    TokenWorkSourceMap& tokenWksMap = proxyIter->second;
    auto tokenIter = tokenWksMap.find(remoteObj);
    if (tokenIter == tokenWksMap.end()) {
        POWER_HILOGD(FEATURE_RUNNING_LOCK, "Runninglock is not existed, proxyKey=%{public}s", proxyKey.c_str());
        return;
    } else {
        tokenWksMap.erase(tokenIter);
        POWER_HILOGD(FEATURE_RUNNING_LOCK, "Runninglocklist empty, earse proxyKey=%{public}s", proxyKey.c_str());
    }
    if (proxyIter->second.empty()) {
        proxyMap_.erase(proxyIter);
        POWER_HILOGD(FEATURE_RUNNING_LOCK, "Runninglocklist empty, earse proxyKey=%{public}s", proxyKey.c_str());
    }
}

bool RunningLockProxy::UpdateWorkSource(pid_t pid, pid_t uid, const sptr<IRemoteObject>& remoteObj,
    const std::map<int32_t, std::string>& workSources)
{
    std::string proxyKey = AssembleProxyKey(pid, uid);
    auto proxyIter = proxyMap_.find(proxyKey);
    if (proxyIter == proxyMap_.end()) {
        POWER_HILOGW(FEATURE_RUNNING_LOCK, "UpdateWorkSource failed, proxyKey=%{public}s not existed",
            proxyKey.c_str());
        return false;
    }
    auto& tokenWksMap = proxyIter->second;
    auto tokenWksMapIter = tokenWksMap.find(remoteObj);
    if (tokenWksMapIter == tokenWksMap.end()) {
        POWER_HILOGW(FEATURE_RUNNING_LOCK, "UpdateWorkSource failed, runninglock not existed");
            return false;
    }
    auto& workSourceMap = tokenWksMapIter->second.first;
    WksMap workSourcesState;
    std::string bundleName;
    int32_t proxyCount = 0;
    for (const auto& wks : workSources) {
        auto iter = workSourceMap.find(wks.first);
        if (iter != workSourceMap.end()) {
            workSourcesState.insert({ iter->first, { iter->second.first, iter->second.second } });
            if (iter->second.second) {
                proxyCount++;
            } else {
                bundleName.append(iter->second.first).append(" ");
            }
        } else {
            workSourcesState.insert({ wks.first, { wks.second, false } });
            bundleName.append(wks.second).append(" ");
        }
    }
    if (!bundleName.empty()) {
        bundleName.pop_back();
    }
    bool isProxyed = tokenWksMapIter->second.second != 0 &&
        tokenWksMapIter->second.second == static_cast<int32_t>(workSourceMap.size());
    int32_t wksCount = static_cast<int32_t>(workSourcesState.size());
    if (isProxyed && wksCount > proxyCount) {
        ProxyInner(remoteObj, bundleName, RunningLockEvent::RUNNINGLOCK_UNPROXY);
    } else if (!isProxyed && wksCount > 0 && wksCount == proxyCount) {
        ProxyInner(remoteObj, bundleName, RunningLockEvent::RUNNINGLOCK_PROXY);
    } else {
        ProxyInner(remoteObj, bundleName, RunningLockEvent::RUNNINGLOCK_UPDATE);
    }
    tokenWksMapIter->second.first = std::move(workSourcesState);
    tokenWksMapIter->second.second = proxyCount;
    return true;
}

void RunningLockProxy::ProxyInner(const sptr<IRemoteObject>& remoteObj,
    const std::string& bundleNames, RunningLockEvent event)
{
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        POWER_HILOGW(FEATURE_RUNNING_LOCK, "Power service is nullptr");
        return;
    }
    auto rlmgr = pms->GetRunningLockMgr();
    if (rlmgr == nullptr) {
        POWER_HILOGW(FEATURE_RUNNING_LOCK, "RunninglockMgr is nullptr");
        return;
    }
    auto lockInner = rlmgr->GetRunningLockInner(remoteObj);
    if (lockInner == nullptr) {
        POWER_HILOGW(FEATURE_RUNNING_LOCK, "RunninglockMgr is nullptr");
        return;
    }
    lockInner->SetBundleName(bundleNames);
    switch (event) {
        case RunningLockEvent::RUNNINGLOCK_UPDATE:
            rlmgr->NotifyRunningLockChanged(lockInner->GetParam(), "DUBAI_TAG_RUNNINGLOCK_UPDATE");
            break;
        case RunningLockEvent::RUNNINGLOCK_PROXY:
            rlmgr->NotifyRunningLockChanged(lockInner->GetParam(), "DUBAI_TAG_RUNNINGLOCK_UPDATE");
            rlmgr->UnlockInnerByProxy(remoteObj, lockInner);
            break;
        case RunningLockEvent::RUNNINGLOCK_UNPROXY:
            rlmgr->LockInnerByProxy(remoteObj, lockInner);
            rlmgr->NotifyRunningLockChanged(lockInner->GetParam(), "DUBAI_TAG_RUNNINGLOCK_UPDATE");
            break;
        default:
            break;
    }
}

std::string RunningLockProxy::MergeBundleName(const WksMap& wksMap)
{
    std::string bundleName;
    for (const auto& wks : wksMap) {
        if (wks.second.second == false) {
            bundleName.append(wks.second.first).append(" ");
        }
    }
    if (!bundleName.empty()) {
        bundleName.pop_back();
    }
    return bundleName;
}

bool RunningLockProxy::IncreaseProxyCnt(pid_t pid, pid_t uid)
{
    std::string proxyKey = AssembleProxyKey(pid, uid);
    POWER_HILOGI(FEATURE_RUNNING_LOCK, "IncreaseProxyCnt proxykey=%{public}s", proxyKey.c_str());
    auto proxyIter = proxyMap_.find(proxyKey);
    if (proxyIter != proxyMap_.end()) {
        auto& tokenWksMap = proxyIter->second;
        for (auto& tokenWksItem : tokenWksMap) {
            for (auto& wks : tokenWksItem.second.first) {
                wks.second.second = true;
            }
            tokenWksItem.second.second = static_cast<int32_t>(tokenWksItem.second.first.size());
            ProxyInner(tokenWksItem.first, "", RunningLockEvent::RUNNINGLOCK_PROXY);
        }
    }
    for (auto& proxyItem : proxyMap_) {
        auto& tokenWksMap = proxyItem.second;
        for (auto& tokenWksItem : tokenWksMap) {
            auto& wksMap = tokenWksItem.second.first;
            if (wksMap.find(uid) != wksMap.end() && !wksMap[uid].second) {
                wksMap[uid].second = true;
                tokenWksItem.second.second++;
            } else {
                continue;
            }
            if (tokenWksItem.second.second == static_cast<int32_t>(wksMap.size())) {
                ProxyInner(tokenWksItem.first, "", RunningLockEvent::RUNNINGLOCK_PROXY);
            } else {
                ProxyInner(tokenWksItem.first, MergeBundleName(wksMap), RunningLockEvent::RUNNINGLOCK_UPDATE);
            }
        }
    }
    return true;
}

bool RunningLockProxy::DecreaseProxyCnt(pid_t pid, pid_t uid)
{
    std::string proxyKey = AssembleProxyKey(pid, uid);
    POWER_HILOGI(FEATURE_RUNNING_LOCK, "DecreaseProxyCnt proxykey=%{public}s", proxyKey.c_str());
    auto proxyIter = proxyMap_.find(proxyKey);
    if (proxyIter != proxyMap_.end()) {
        auto& tokenWksMap = proxyIter->second;
        for (auto& tokenWksItem : tokenWksMap) {
            for (auto& wks : tokenWksItem.second.first) {
                wks.second.second = false;
            }
            tokenWksItem.second.second = 0;
            ProxyInner(tokenWksItem.first, MergeBundleName(tokenWksItem.second.first),
                RunningLockEvent::RUNNINGLOCK_UNPROXY);
        }
    }
    for (auto& proxyItem : proxyMap_) {
        auto& tokenWksMap = proxyItem.second;
        for (auto& tokenWksItem : tokenWksMap) {
            auto& wksMap = tokenWksItem.second.first;
            if (wksMap.find(uid) != wksMap.end() && wksMap[uid].second) {
                wksMap[uid].second = false;
                tokenWksItem.second.second = std::max(0, tokenWksItem.second.second - 1);
            } else {
                continue;
            }
            if (tokenWksItem.second.second == static_cast<int32_t>(wksMap.size()) - 1) {
                ProxyInner(tokenWksItem.first, MergeBundleName(wksMap), RunningLockEvent::RUNNINGLOCK_UNPROXY);
            } else {
                ProxyInner(tokenWksItem.first, MergeBundleName(wksMap), RunningLockEvent::RUNNINGLOCK_UPDATE);
            }
        }
    }
    return true;
}

std::string RunningLockProxy::DumpProxyInfo()
{
    std::string result {""};
    int index = 0;
    for (const auto& [key, value] : proxyMap_) {
        index++;
        result.append("  ProcessIndex=").append(std::to_string(index))
            .append(" pid_uid=").append(key)
            .append(" lock_cnt=").append(std::to_string(value.size())).append("\n");
        int lockIndex = 0;
        for (const auto& [token, tokenWksMap] : value) {
            lockIndex++;
            result.append("****lockIndex=").append(std::to_string(lockIndex))
                .append("****workSourceSize=").append(std::to_string(tokenWksMap.first.size()))
                .append("****proxyCount=").append(std::to_string(tokenWksMap.second))
                .append("\n");
            int appIndex = 0;
            for (const auto& wks : tokenWksMap.first) {
                appIndex++;
                result.append("********workSourceIndex=").append(std::to_string(appIndex))
                    .append("********appuid=").append(std::to_string(wks.first))
                    .append("********bundleName=").append(wks.second.first)
                    .append("********proxyState=").append(std::to_string(wks.second.second))
                    .append("\n");
            }
        }
    }
    return result;
}

void RunningLockProxy::ResetRunningLocks()
{
    POWER_HILOGI(FEATURE_RUNNING_LOCK, "reset proxycnt");
    for (auto &proxyItem : proxyMap_) {
        auto& tokenWksMap = proxyItem.second;
        for (auto &tokenWksItem : tokenWksMap) {
            for (auto &wks : tokenWksItem.second.first) {
                wks.second.second = false;
            }
            ProxyInner(tokenWksItem.first, MergeBundleName(tokenWksItem.second.first),
                RunningLockEvent::RUNNINGLOCK_UNPROXY);
            tokenWksItem.second.second = 0;
        }
    }
}

bool RunningLockProxy::UpdateProxyState(pid_t pid, pid_t uid, const sptr<IRemoteObject>& remoteObj,
    bool state)
{
    std::string proxyKey = AssembleProxyKey(pid, uid);
    auto proxyIter = proxyMap_.find(proxyKey);
    if (proxyIter == proxyMap_.end()) {
        POWER_HILOGW(FEATURE_RUNNING_LOCK, "UpdateProxyState failed, proxyKey=%{public}s not existed",
            proxyKey.c_str());
        return false;
    }
    auto& tokenWksMap = proxyIter->second;
    auto tokenWksMapIter = tokenWksMap.find(remoteObj);
    if (tokenWksMapIter == tokenWksMap.end()) {
        POWER_HILOGW(FEATURE_RUNNING_LOCK, "UpdateProxyState failed, runninglock not existed");
        return false;
    }
    auto& workSourceMap = tokenWksMapIter->second.first;
    for (auto &wks : workSourceMap) {
        wks.second.second = false;
    }
    tokenWksMapIter->second.second = 0;
    return true;
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

