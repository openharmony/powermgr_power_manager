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

#include "power_hdf_service.h"

#include <file_ex.h>
#include <sys/eventfd.h>
#include "errors.h"
#include "hdf_sbuf.h"
#include "power_hdf_info.h"
#include "pubdef.h"
#include "unique_fd.h"
#include "utils/hdf_log.h"

namespace OHOS {
namespace PowerMgr {
std::mutex PowerHdfService::mutex_;

int32_t PowerHdfService::Bind(struct HdfDeviceObject *device)
{
    HDF_LOGD("%{public}s enter", __func__);
    if (device == NULL) {
        HDF_LOGE("%{public}s device is NULL", __func__);
        return HDF_ERR_INVALID_OBJECT;
    }
    PowerHdfService* service = new PowerHdfService();
    service->device = device;
    service->ioService.Open = nullptr;
    service->ioService.Dispatch = PowerHdfService::Dispatch;
    service->ioService.Release = nullptr;

    device->service = &(service->ioService);
    device->priv = reinterpret_cast<void*>(service);

    return HDF_SUCCESS;
}

int32_t PowerHdfService::Init(struct HdfDeviceObject *device)
{
    HDF_LOGD("%{public}s enter", __func__);
    if (device == NULL) {
        HDF_LOGE("%{public}s device is NULL", __func__);
        return HDF_ERR_INVALID_OBJECT;
    }

    PowerHdfService* service = reinterpret_cast<PowerHdfService*>(device->priv);
    if (service == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }

    return HDF_SUCCESS;
}

void PowerHdfService::Release(struct HdfDeviceObject *device)
{
    HDF_LOGD("%{public}s enter", __func__);
    if (device == NULL) {
        return;
    }
    PowerHdfService* service = reinterpret_cast<PowerHdfService*>(device->priv);
    if (service == NULL) {
        return;
    }
    delete service;
}

int32_t PowerHdfService::Dispatch(struct HdfDeviceIoClient *client,
    int cmdId, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    HDF_LOGD("%{public}s enter", __func__);
    if (client == NULL || client->device == NULL) {
        HDF_LOGE("%{public}s: client or client->device is NULL", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    int ret = HDF_ERR_NOT_SUPPORT;
    switch (cmdId) {
        case CMD_SUSPEND: {
            ret = PowerHdfService::Suspend();
            break;
        }
        case CMD_READ_WAKE_COUNT: {
            ret = PowerHdfService::ReadWakeCount(reply);
            break;
        }
        case CMD_WRITE_WAKE_COUNT: {
            ret = PowerHdfService::WriteWakeCount(data);
            break;
        }
        case CMD_WAKE_LOCK: {
            ret = PowerHdfService::WakeLock(data);
            break;
        }
        case CMD_WAKE_UNLOCK: {
            ret = PowerHdfService::WakeUnlock(data);
            break;
        }
        case CMD_DUMP: {
            ret = PowerHdfService::Dump(reply);
            break;
        }
        default:
            break;
    }
    if (ret == HDF_SUCCESS) {
        HdfSbufWriteInt32(reply, ERR_OK);
    } else if (ret == HDF_FAILURE) {
        HdfSbufWriteInt32(reply, ERR_INVALID_OPERATION);
    }
    return ret;
}

int32_t PowerHdfService::Suspend()
{
    HDF_LOGD("%{public}s enter", __func__);
    std::lock_guard<std::mutex> lock(mutex_);
    UniqueFd suspendStateFd(TEMP_FAILURE_RETRY(open(SUSPEND_STATE_PATH, O_RDWR | O_CLOEXEC)));
    if (suspendStateFd < 0) {
        HDF_LOGD("Failed to open the suspending state fd!");
        return HDF_FAILURE;
    }
    bool ret = SaveStringToFd(suspendStateFd, SUSPEND_STATE);
    if (!ret) {
        HDF_LOGE("Failed to write the suspend state!");
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t PowerHdfService::ReadWakeCount(struct HdfSBuf *reply)
{
    HDF_LOGD("%{public}s enter", __func__);
    std::lock_guard<std::mutex> lock(mutex_);
    UniqueFd fd(TEMP_FAILURE_RETRY(open(WAKEUP_COUNT_PATH, O_RDWR | O_CLOEXEC)));
    if (fd < 0) {
        HDF_LOGD("Failed to open the suspending state fd!");
        return HDF_FAILURE;
    }
    std::string count;
    bool ret = LoadStringFromFd(fd, count);
    if (!ret) {
        HDF_LOGE("Failed to read wake count from kernel!");
        return HDF_FAILURE;
    }
    HdfSbufWriteString(reply, count.c_str());
    return HDF_SUCCESS;
}

int32_t PowerHdfService::WriteWakeCount(struct HdfSBuf *data)
{
    HDF_LOGD("%{public}s enter", __func__);
    std::lock_guard<std::mutex> lock(mutex_);
    if (data == nullptr) {
        return HDF_ERR_INVALID_PARAM;
    }
    const char* count = HdfSbufReadString(data);
    if (count == nullptr) {
        return HDF_ERR_INVALID_PARAM;
    }
    UniqueFd fd(TEMP_FAILURE_RETRY(open(WAKEUP_COUNT_PATH, O_RDWR | O_CLOEXEC)));
    bool ret = SaveStringToFd(fd, count);
    if (!ret) {
        HDF_LOGE("Failed to write the lock to kernel!");
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t PowerHdfService::WakeLock(struct HdfSBuf *data)
{
    HDF_LOGD("%{public}s enter", __func__);
    std::lock_guard<std::mutex> lock(mutex_);
    if (data == nullptr) {
        return HDF_ERR_INVALID_PARAM;
    }
    const char* name = HdfSbufReadString(data);
    if (name == nullptr) {
        return HDF_ERR_INVALID_PARAM;
    }
    UniqueFd fd(TEMP_FAILURE_RETRY(open(LOCK_PATH, O_RDWR | O_CLOEXEC)));
    bool ret = SaveStringToFd(fd, name);
    if (!ret) {
        HDF_LOGE("Failed to write the lock to kernel!");
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t PowerHdfService::WakeUnlock(struct HdfSBuf *data)
{
    HDF_LOGD("%{public}s enter", __func__);
    std::lock_guard<std::mutex> lock(mutex_);
    if (data == nullptr) {
        return HDF_ERR_INVALID_PARAM;
    }
    const char* name = HdfSbufReadString(data);
    if (name == nullptr) {
        return HDF_ERR_INVALID_PARAM;
    }
    UniqueFd fd(TEMP_FAILURE_RETRY(open(UNLOCK_PATH, O_RDWR | O_CLOEXEC)));
    bool ret = SaveStringToFd(fd, name);
    if (!ret) {
        HDF_LOGE("Failed to write the lock to kernel!");
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

static void loadSystemInfo(const char* const path, std::string& info)
{
    UniqueFd fd(TEMP_FAILURE_RETRY(open(path, O_RDWR | O_CLOEXEC)));
    std::string str;
    if (fd >= 0) {
        bool ret = LoadStringFromFd(fd, str);
        if (!ret) {
            str = "# Failed to read";
        }
    } else {
        str = "# Failed to open";
    }
    info.append(path);
    info.append(": " + str + "\n");
}

int32_t PowerHdfService::Dump(struct HdfSBuf *reply)
{
    HDF_LOGD("%{public}s enter", __func__);
    std::string dumpInfo("");
    loadSystemInfo(SUSPEND_STATE_PATH, dumpInfo);
    loadSystemInfo(WAKEUP_COUNT_PATH, dumpInfo);
    loadSystemInfo(LOCK_PATH, dumpInfo);
    loadSystemInfo(UNLOCK_PATH, dumpInfo);

    HdfSbufWriteString(reply, dumpInfo.c_str());
    return HDF_SUCCESS;
}

struct HdfDriverEntry g_powerHdfEntry = {
    .moduleVersion = 1,
    .moduleName = "power_hdf",
    .Bind = PowerHdfService::Bind,
    .Init = PowerHdfService::Init,
    .Release = PowerHdfService::Release,
};
} // namespace PowerMgr
} // namespace OHOS

#ifndef __cplusplus
extern "C" {
#endif

HDF_INIT(OHOS::PowerMgr::g_powerHdfEntry);

#ifndef __cplusplus
}
#endif

