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

#ifndef POWER_HDF_SERVICE_H
#define POWER_HDF_SERVICE_H

#include <chrono>
#include <mutex>
#include <stdlib.h>
#include <thread>
#include "errors.h"
#include "hdf_device_desc.h"
#include "hdf_remote_service.h"
#include "ipower_hdf_callback.h"
#include "unique_fd.h"

namespace OHOS {
namespace PowerMgr {
class PowerHdfService {
public:
    PowerHdfService() = default;
    ~PowerHdfService() = default;
    static int32_t Bind(struct HdfDeviceObject *device);
    static int32_t Init(struct HdfDeviceObject *device);
    static void Release(struct HdfDeviceObject *device);

    static int32_t Dispatch(struct HdfDeviceIoClient *client,
        int cmdId, struct HdfSBuf *data, struct HdfSBuf *reply);
    static int32_t RegisterCallback(struct HdfSBuf *data);
    static int32_t StartSuspend(struct HdfSBuf *data);
    static int32_t StopSuspend(struct HdfSBuf *data);
    static int32_t ForceSuspend(struct HdfSBuf *data);
    static int32_t SuspendBlock(struct HdfSBuf *data);
    static int32_t SuspendUnblock(struct HdfSBuf *data);
    static int32_t Dump(struct HdfSBuf *data);

    struct IDeviceIoService ioService;
    struct HdfDeviceObject *device;
private:
    static constexpr const char * const SUSPEND_STATE = "mem";
    static constexpr const char * const SUSPEND_STATE_PATH = "/sys/power/state";
    static constexpr const char * const LOCK_PATH = "/sys/power/wake_lock";
    static constexpr const char * const UNLOCK_PATH = "/sys/power/wake_unlock";
    static constexpr const char * const WAKEUP_COUNT_PATH = "/sys/power/wakeup_count";
    static std::chrono::milliseconds waitTime_; // {100ms};
    static std::mutex mutex_;
    static std::unique_ptr<std::thread> daemon_;
    static bool suspending_;
    static struct HdfRemoteService *callback_;
    static UniqueFd wakeupCountFd;
    static void AutoSuspendLoop();
    static int32_t DoSuspend();
    static std::string ReadWakeCount();
    static bool WriteWakeCount(const std::string& count);
    static void NotifyCallback(int code);
};
} // namespace PowerMgr
} // namespace OHOS

#endif // POWER_HDF_SERVICE_H