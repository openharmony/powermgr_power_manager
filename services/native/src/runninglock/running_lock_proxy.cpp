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
    std::map<int32_t, bool> wksMap;
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
    std::map<int32_t, bool> workSourcesState)
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
    int32_t proxyCount = 0;
    for (const auto& wks : workSourceMap) {
        if (workSourcesState.find(wks.first) != workSourcesState.end() && wks.second == true) {
            workSourcesState[wks.first] = true;
            proxyCount++;
        }
    }
    bool isProxyed = tokenWksMapIter->second.second != 0 &&
        tokenWksMapIter->second.second == static_cast<int32_t>(workSourceMap.size());
    if (isProxyed && (workSourcesState.size() == 0 ||
        static_cast<int32_t>(workSourcesState.size()) > proxyCount)) {
        ProxyInner(remoteObj, false);
        tokenWksMapIter->second.second = proxyCount;
    }
    tokenWksMapIter->second.first = std::move(workSourcesState);
    return true;
}

void RunningLockProxy::ProxyInner(const sptr<IRemoteObject>& remoteObj, bool isProxy)
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
    if (isProxy) {
        rlmgr->UnlockInnerByProxy(remoteObj, lockInner);
    } else {
        rlmgr->LockInnerByProxy(remoteObj, lockInner);
    }
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
                wks.second = true;
            }
            tokenWksItem.second.second = tokenWksItem.second.first.size();
            ProxyInner(tokenWksItem.first, true);
        }
        return true;
    }
    for (auto& proxyItem : proxyMap_) {
        auto& tokenWksMap = proxyItem.second;
        for (auto& tokenWksItem : tokenWksMap) {
            auto& wksMap = tokenWksItem.second.first;
            if (wksMap.find(uid) != wksMap.end() && !wksMap[uid]) {
                wksMap[uid] = true;
                tokenWksItem.second.second++;
            } else {
                continue;
            }
            if (tokenWksItem.second.second == static_cast<int32_t>(wksMap.size())) {
                ProxyInner(tokenWksItem.first, true);
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
                wks.second = false;
            }
            tokenWksItem.second.second = 0;
            ProxyInner(tokenWksItem.first, false);
        }
        return true;
    }
    for (auto& proxyItem : proxyMap_) {
        auto& tokenWksMap = proxyItem.second;
        for (auto& tokenWksItem : tokenWksMap) {
            auto& wksMap = tokenWksItem.second.first;
            if (wksMap.find(uid) != wksMap.end() && wksMap[uid]) {
                wksMap[uid] = false;
                tokenWksItem.second.second = std::max(0, tokenWksItem.second.second - 1);
            } else {
                continue;
            }
            if (tokenWksItem.second.second == 0) {
                ProxyInner(tokenWksItem.first, false);
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
                result.append("********workSourceIndex=").append(std::to_string(appIndex))
                    .append("********appuid=").append(std::to_string(wks.first))
                    .append("********proxyState=").append(std::to_string(wks.second))
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
                wks.second = false;
            }
            tokenWksItem.second.second = 0;
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

