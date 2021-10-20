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

#include "power_hdf_client.h"

#include "iservmgr_hdi.h"
#include "power_common.h"

namespace OHOS {
namespace PowerMgr {
namespace {
const std::string POWER_HDF_SERVICE = "power_hdf";
}

using OHOS::HDI::ServiceManager::V1_0::IServiceManager;

ErrCode PowerHdfClient::Suspend()
{
    POWER_HILOGW(MODULE_SERVICE, "PowerHdfClient::Suspend: fun is start");
    MessageParcel data;
    MessageParcel reply;
    return Dispatch(CMD_SUSPEND, data, reply);
}

ErrCode PowerHdfClient::ReadWakeCount(std::string& count)
{
    POWER_HILOGW(MODULE_SERVICE, "PowerHdfClient::ReadWakeCount: fun is start");
    MessageParcel data;
    MessageParcel reply;
    ErrCode ret = Dispatch(CMD_READ_WAKE_COUNT, data, reply);
    if (ret != ERR_OK) {
        return ret;
    }
    count = reply.ReadString();
    POWER_HILOGW(MODULE_SERVICE, "PowerHdfClient::ReadWakeCount: %{public}s", count.c_str());
    return ERR_OK;
}

ErrCode PowerHdfClient::WriteWakeCount(const std::string& count)
{
    POWER_HILOGW(MODULE_SERVICE, "PowerHdfClient::WriteWakeCount: fun is start");
    MessageParcel data;
    MessageParcel reply;
    data.WriteString(count);
    ErrCode ret = Dispatch(CMD_WRITE_WAKE_COUNT, data, reply);
    if (ret != ERR_OK) {
        return ret;
    }
    return ERR_OK;
}

ErrCode PowerHdfClient::WakeLock(const std::string& name)
{
    POWER_HILOGW(MODULE_SERVICE, "PowerHdfClient::WakeLock: fun is start");
    MessageParcel data;
    MessageParcel reply;
    data.WriteString(name);
    return Dispatch(CMD_WAKE_LOCK, data, reply);
}

ErrCode PowerHdfClient::WakeUnlock(const std::string& name)
{
    POWER_HILOGW(MODULE_SERVICE, "PowerHdfClient::WakeUnlock: fun is start");
    MessageParcel data;
    MessageParcel reply;
    data.WriteString(name);
    return Dispatch(CMD_WAKE_UNLOCK, data, reply);
}

sptr<IRemoteObject> PowerHdfClient::GetService()
{
    POWER_HILOGW(MODULE_SERVICE, "PowerHdfClient::GetService: fun is start");
    auto serviceManager = IServiceManager::Get();
    if (serviceManager == nullptr) {
        POWER_HILOGW(MODULE_SERVICE, "service manager is nullptr");
        return nullptr;
    }
    auto service = serviceManager->GetService(POWER_HDF_SERVICE.c_str());
    if (service == nullptr) {
        POWER_HILOGW(MODULE_SERVICE, "power_hdf service is nullptr");
        return nullptr;
    }
    return service;
}

ErrCode PowerHdfClient::Dump(std::string& info)
{
    POWER_HILOGW(MODULE_SERVICE, "PowerHdfClient::Dump: fun is start");
    MessageParcel data;
    MessageParcel reply;
    ErrCode ret = Dispatch(CMD_DUMP, data, reply);
    if (ret != ERR_OK) {
        return ret;
    }
    info = reply.ReadString();
    return ERR_OK;
}

ErrCode PowerHdfClient::Dispatch(uint32_t cmd, MessageParcel &data, MessageParcel &reply)
{
    POWER_HILOGD(MODULE_SERVICE, "Start to dispatch cmd: %{public}d", cmd);
    auto service = GetService();
    POWER_HILOGD(MODULE_SERVICE, "PowerHdfClient::Dispatch: GetService");
    if (service == nullptr) {
        POWER_HILOGD(MODULE_SERVICE, "PowerHdfClient::Dispatch: service nullptr)");
        return ERR_NO_INIT;
    }
    POWER_HILOGD(MODULE_SERVICE, "PowerHdfClient::Dispatch: service not nullptr)");

    MessageOption option;
    POWER_HILOGD(MODULE_SERVICE, "PowerHdfClient::Dispatch: option)");
    auto ret = service->SendRequest(cmd, data, reply, option);
    POWER_HILOGD(MODULE_SERVICE, "PowerHdfClient::Dispatch: auto ret)");
    if (ret != ERR_OK) {
        POWER_HILOGE(MODULE_SERVICE, "failed to send request, cmd: %{public}d, ret: %{public}d", cmd, ret);
        return ret;
    } else {
        ret = reply.ReadInt32();
    }

    return ret;
}
} // namespace PowerMgr
} // namespace OHOS
