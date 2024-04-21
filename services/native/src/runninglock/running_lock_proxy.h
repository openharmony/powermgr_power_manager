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

#ifndef POWERMGR_RUNNING_LOCK_PROXY_H
#define POWERMGR_RUNNING_LOCK_PROXY_H

#include <map>
#include <vector>

#include <iremote_object.h>
#include <functional>

namespace OHOS {
namespace PowerMgr {
using RunningLockProxyMap = std::map<std::string, std::pair<std::vector<sptr<IRemoteObject>>, int32_t>>;
class RunningLockProxy {
public:
    RunningLockProxy() = default;
    ~RunningLockProxy() = default;

    void AddRunningLock(pid_t pid, pid_t uid, const sptr<IRemoteObject>& remoteObj);
    void RemoveRunningLock(pid_t pid, pid_t uid, const sptr<IRemoteObject>& remoteObj);
    std::vector<sptr<IRemoteObject>> GetRemoteObjectList(pid_t pid, pid_t uid);
    bool IsProxied(pid_t pid, pid_t uid);
    bool IncreaseProxyCnt(pid_t pid, pid_t uid, const std::function<void(void)>& proxyRunningLock);
    bool DecreaseProxyCnt(pid_t pid, pid_t uid, const std::function<void(void)>& unProxyRunningLock);
    void Clear();
    std::string DumpProxyInfo();
    void ResetRunningLocks();
private:
    std::string AssembleProxyKey(pid_t pid, pid_t uid);
    RunningLockProxyMap proxyMap_;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_RUNNING_LOCK_PROXY_H

