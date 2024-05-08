/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef RUNNINGLOCK_TIMERHANDKER_H
#define RUNNINGLOCK_TIMERHANDKER_H

#include <cstdint>
#include <map>
#include <timer.h>
#include <mutex>

#include <iremote_object.h>

namespace OHOS {
namespace PowerMgr {
class RunningLockTimerHandler {
public:
    RunningLockTimerHandler() = default;
    ~RunningLockTimerHandler();

    static RunningLockTimerHandler& GetInstance()
    {
        static RunningLockTimerHandler timerHandler;
        return timerHandler;
    }

    bool RegisterRunningLockTimer(const sptr<IRemoteObject> &token, const std::function<void()> &callback,
        int32_t timeoutMs);
    bool UnregisterRunningLockTimer(const sptr<IRemoteObject> &token);

private:
    uint32_t GetRunningLockTimerId(const sptr<IRemoteObject> &token);
    void AddRunningLockTimerMap(const sptr<IRemoteObject> &token, uint32_t timerId);
    void RemoveRunningLockTimerMap(const sptr<IRemoteObject> &token);
    void UnregisterTimer(uint32_t timerId);
    void CleanTimer();
    std::unique_ptr<OHOS::Utils::Timer> handlerTimer_;
    std::mutex mutex_;
    std::map<const sptr<IRemoteObject>, uint32_t> runninglockTimerMap_;
};
} // namespace PowerMgr
} // namespace OHOS

#endif // RUNNINGLOCK_TIMERHANDKER_H
